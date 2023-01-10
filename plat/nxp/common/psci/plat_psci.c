/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>

#include <plat_gic.h>
#include <plat_common.h>
#include <plat_psci.h>
#ifdef NXP_WARM_BOOT
#include <plat_warm_rst.h>
#endif

#include <platform_def.h>

#if (SOC_CORE_OFF || SOC_CORE_PWR_DWN)
static void __dead2 _no_return_wfi(void)
{
_bl31_dead_wfi:
	wfi();
	goto _bl31_dead_wfi;
}
#endif

#if (SOC_CORE_RELEASE || SOC_CORE_PWR_DWN)
 /* the entry for core warm boot */
static uintptr_t warmboot_entry = (uintptr_t) NULL;
#endif

#if (SOC_CORE_RELEASE)
static int _pwr_domain_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos(mpidr);
	int rc = PSCI_E_INVALID_PARAMS;
	u_register_t core_mask;

	if (core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT) {

		_soc_set_start_addr(warmboot_entry);

		dsb();
		isb();

		core_mask = (1 << core_pos);
		rc = _psci_cpu_on(core_mask);
	}

	return (rc);
}
#endif

#if (SOC_CORE_OFF)
static void _pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state = _getCoreState(core_mask);

	 /* set core state in internal data */
	core_state = CORE_OFF_PENDING;
	_setCoreState(core_mask, core_state);

	_psci_cpu_prep_off(core_mask);
}
#endif

#if (SOC_CORE_OFF || SOC_CORE_PWR_DWN)
static void __dead2 _pwr_down_wfi(const psci_power_state_t *target_state)
{
	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state = _getCoreState(core_mask);

	switch (core_state) {
#if (SOC_CORE_OFF)
	case CORE_OFF_PENDING:
		/* set core state in internal data */
		core_state = CORE_OFF;
		_setCoreState(core_mask, core_state);

		 /* turn the core off */
		_psci_cpu_off_wfi(core_mask, warmboot_entry);
	break;
#endif
#if (SOC_CORE_PWR_DWN)
	case CORE_PWR_DOWN:
		 /* power-down the core */
		_psci_cpu_pwrdn_wfi(core_mask, warmboot_entry);
		break;
#endif
#if (SOC_SYSTEM_PWR_DWN)
	case SYS_OFF_PENDING:
		/* set core state in internal data */
		core_state = SYS_OFF;
		_setCoreState(core_mask, core_state);

		/* power-down the system */
		_psci_sys_pwrdn_wfi(core_mask, warmboot_entry);
		break;
#endif
	default:
		_no_return_wfi();
	break;
	}
}
#endif

#if (SOC_CORE_RELEASE || SOC_CORE_RESTART)
static void _pwr_domain_wakeup(const psci_power_state_t *target_state)
{
	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state = _getCoreState(core_mask);

	switch (core_state) {
	case CORE_PENDING: /* this core is coming out of reset */

		 /* soc per cpu setup */
		soc_init_percpu();

		 /* gic per cpu setup */
		plat_gic_pcpu_init();

		 /* set core state in internal data */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
		break;

#if (SOC_CORE_RESTART)
	case CORE_WAKEUP:

		 /* this core is waking up from OFF */
		_psci_wakeup(core_mask);

		 /* set core state in internal data */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);

	break;
#endif
	}
}
#endif

#if (SOC_CORE_STANDBY)
static void _pwr_cpu_standby(plat_local_state_t  cpu_state)
{
	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state;

	if (cpu_state == PLAT_MAX_RET_STATE) {

		/* set core state to standby */
		core_state = CORE_STANDBY;
		_setCoreState(core_mask, core_state);

		_psci_core_entr_stdby(core_mask);

		/* when we are here, the core is waking up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
	}
}
#endif

#if (SOC_CORE_PWR_DWN)
static void _pwr_suspend(const psci_power_state_t *state)
{

	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state;

	if (state->pwr_domain_state[PLAT_MAX_LVL] == PLAT_MAX_OFF_STATE) {
#if (SOC_SYSTEM_PWR_DWN)
		_psci_sys_prep_pwrdn(core_mask);

		 /* set core state */
		core_state = SYS_OFF_PENDING;
		_setCoreState(core_mask, core_state);
#endif
	} else if (state->pwr_domain_state[PLAT_MAX_LVL]
				== PLAT_MAX_RET_STATE) {
#if (SOC_SYSTEM_STANDBY)
		_psci_sys_prep_stdby(core_mask);

		 /* set core state */
		core_state = CORE_STANDBY;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CLSTR_LVL] ==
					PLAT_MAX_OFF_STATE) {
#if (SOC_CLUSTER_PWR_DWN)
		_psci_clstr_prep_pwrdn(core_mask);

		 /* set core state */
		core_state = CORE_PWR_DOWN;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CLSTR_LVL] ==
					PLAT_MAX_RET_STATE) {
#if (SOC_CLUSTER_STANDBY)
		_psci_clstr_prep_stdby(core_mask);

		 /* set core state */
		core_state = CORE_STANDBY;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CORE_LVL] == PLAT_MAX_OFF_STATE) {
#if (SOC_CORE_PWR_DWN)
		 /* prep the core for power-down */
		_psci_core_prep_pwrdn(core_mask);

		 /* set core state */
		core_state = CORE_PWR_DOWN;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CORE_LVL] == PLAT_MAX_RET_STATE) {
#if (SOC_CORE_STANDBY)
		_psci_core_prep_stdby(core_mask);

		 /* set core state */
		core_state = CORE_STANDBY;
		_setCoreState(core_mask, core_state);
#endif
	}

}
#endif

#if (SOC_CORE_PWR_DWN)
static void _pwr_suspend_finish(const psci_power_state_t *state)
{

	u_register_t core_mask  = plat_my_core_mask();
	u_register_t core_state;


	if (state->pwr_domain_state[PLAT_MAX_LVL] == PLAT_MAX_OFF_STATE) {
#if (SOC_SYSTEM_PWR_DWN)
		_psci_sys_exit_pwrdn(core_mask);

		/* when we are here, the core is back up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	} else if (state->pwr_domain_state[PLAT_MAX_LVL]
				== PLAT_MAX_RET_STATE) {
#if (SOC_SYSTEM_STANDBY)
		_psci_sys_exit_stdby(core_mask);

		/* when we are here, the core is waking up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CLSTR_LVL] ==
						PLAT_MAX_OFF_STATE) {
#if (SOC_CLUSTER_PWR_DWN)
		_psci_clstr_exit_pwrdn(core_mask);

		/* when we are here, the core is waking up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CLSTR_LVL] ==
						PLAT_MAX_RET_STATE) {
#if (SOC_CLUSTER_STANDBY)
		_psci_clstr_exit_stdby(core_mask);

		/* when we are here, the core is waking up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CORE_LVL] == PLAT_MAX_OFF_STATE) {
#if (SOC_CORE_PWR_DWN)
		_psci_core_exit_pwrdn(core_mask);

		/* when we are here, the core is back up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	}

	else if (state->pwr_domain_state[PLAT_CORE_LVL] == PLAT_MAX_RET_STATE) {
#if (SOC_CORE_STANDBY)
		_psci_core_exit_stdby(core_mask);

		/* when we are here, the core is waking up
		 * set core state to released
		 */
		core_state = CORE_RELEASED;
		_setCoreState(core_mask, core_state);
#endif
	}

}
#endif

#if (SOC_CORE_STANDBY || SOC_CORE_PWR_DWN)

#define PWR_STATE_TYPE_MASK    0x00010000
#define PWR_STATE_TYPE_STNDBY  0x0
#define PWR_STATE_TYPE_PWRDWN  0x00010000
#define PWR_STATE_LVL_MASK     0x03000000
#define PWR_STATE_LVL_CORE     0x0
#define PWR_STATE_LVL_CLSTR    0x01000000
#define PWR_STATE_LVL_SYS      0x02000000
#define PWR_STATE_LVL_MAX      0x03000000

 /* turns a requested power state into a target power state
  * based on SoC capabilities
  */
static int _pwr_state_validate(uint32_t pwr_state,
				    psci_power_state_t *state)
{
	int stat   = PSCI_E_INVALID_PARAMS;
	int pwrdn  = (pwr_state & PWR_STATE_TYPE_MASK);
	int lvl    = (pwr_state & PWR_STATE_LVL_MASK);

	switch (lvl) {
	case PWR_STATE_LVL_MAX:
		if (pwrdn && SOC_SYSTEM_PWR_DWN)
			state->pwr_domain_state[PLAT_MAX_LVL] =
				PLAT_MAX_OFF_STATE;
		else if (SOC_SYSTEM_STANDBY)
			state->pwr_domain_state[PLAT_MAX_LVL] =
				PLAT_MAX_RET_STATE;
		 /* fallthrough */
	case PWR_STATE_LVL_SYS:
		if (pwrdn && SOC_SYSTEM_PWR_DWN)
			state->pwr_domain_state[PLAT_SYS_LVL] =
				PLAT_MAX_OFF_STATE;
		else if (SOC_SYSTEM_STANDBY)
			state->pwr_domain_state[PLAT_SYS_LVL] =
				PLAT_MAX_RET_STATE;
		 /* fallthrough */
	case PWR_STATE_LVL_CLSTR:
		if (pwrdn && SOC_CLUSTER_PWR_DWN)
			state->pwr_domain_state[PLAT_CLSTR_LVL] =
				PLAT_MAX_OFF_STATE;
		else if (SOC_CLUSTER_STANDBY)
			state->pwr_domain_state[PLAT_CLSTR_LVL] =
				PLAT_MAX_RET_STATE;
		 /* fallthrough */
	case PWR_STATE_LVL_CORE:
		stat = PSCI_E_SUCCESS;

		if (pwrdn && SOC_CORE_PWR_DWN)
			state->pwr_domain_state[PLAT_CORE_LVL] =
				PLAT_MAX_OFF_STATE;
		else if (SOC_CORE_STANDBY)
			state->pwr_domain_state[PLAT_CORE_LVL] =
				PLAT_MAX_RET_STATE;
		break;
	}
	return (stat);
}

#endif

#if (SOC_SYSTEM_PWR_DWN)
static void _pwr_state_sys_suspend(psci_power_state_t *req_state)
{

	/* if we need to have per-SoC settings, then we need to
	 * extend this by calling into psci_utils.S and from there
	 * on down to the SoC.S files
	 */

	req_state->pwr_domain_state[PLAT_MAX_LVL]   = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[PLAT_SYS_LVL]   = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[PLAT_CLSTR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[PLAT_CORE_LVL]  = PLAT_MAX_OFF_STATE;

}
#endif

#if defined(NXP_WARM_BOOT) && (SOC_SYSTEM_RESET2)
static int psci_system_reset2(int is_vendor,
			      int reset_type,
			      u_register_t cookie)
{
	int ret = 0;

	INFO("Executing the sequence of warm reset.\n");
	ret = prep_n_execute_warm_reset();

	return ret;
}
#endif

static plat_psci_ops_t _psci_pm_ops = {
#if (SOC_SYSTEM_OFF)
	.system_off = _psci_system_off,
#endif
#if (SOC_SYSTEM_RESET)
	.system_reset = _psci_system_reset,
#endif
#if defined(NXP_WARM_BOOT) && (SOC_SYSTEM_RESET2)
	.system_reset2 = psci_system_reset2,
#endif
#if (SOC_CORE_RELEASE || SOC_CORE_RESTART)
	 /* core released or restarted */
	.pwr_domain_on_finish = _pwr_domain_wakeup,
#endif
#if (SOC_CORE_OFF)
	 /* core shutting down */
	.pwr_domain_off	= _pwr_domain_off,
#endif
#if (SOC_CORE_OFF || SOC_CORE_PWR_DWN)
	.pwr_domain_pwr_down_wfi = _pwr_down_wfi,
#endif
#if (SOC_CORE_STANDBY || SOC_CORE_PWR_DWN)
	 /* cpu_suspend */
	.validate_power_state = _pwr_state_validate,
#if (SOC_CORE_STANDBY)
	.cpu_standby = _pwr_cpu_standby,
#endif
#if (SOC_CORE_PWR_DWN)
	.pwr_domain_suspend        = _pwr_suspend,
	.pwr_domain_suspend_finish = _pwr_suspend_finish,
#endif
#endif
#if (SOC_SYSTEM_PWR_DWN)
	.get_sys_suspend_power_state = _pwr_state_sys_suspend,
#endif
#if (SOC_CORE_RELEASE)
	 /* core executing psci_cpu_on */
	.pwr_domain_on	= _pwr_domain_on
#endif
};

#if (SOC_CORE_RELEASE  || SOC_CORE_PWR_DWN)
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	warmboot_entry = sec_entrypoint;
	*psci_ops = &_psci_pm_ops;
	return 0;
}

#else

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &_psci_pm_ops;
	return 0;
}
#endif

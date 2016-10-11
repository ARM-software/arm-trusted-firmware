/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <errno.h>
#include <debug.h>
#include <psci.h>
#include <delay_timer.h>
#include <platform_def.h>
#include <plat_private.h>

/* Macros to read the rk power domain state */
#define RK_CORE_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define RK_CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define RK_SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

static uintptr_t rockchip_sec_entrypoint;

static struct rockchip_pm_ops_cb *rockchip_ops;

/*******************************************************************************
 * Rockchip standard platform handler called to check the validity of the power
 * state parameter.
 ******************************************************************************/
int rockchip_validate_power_state(unsigned int power_state,
				  psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's probably to enter standby only on power level 0
		 * ignore any other power level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] =
					PLAT_MAX_RET_STATE;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_OFF_STATE;

		for (i = (pwr_lvl + 1); i <= PLAT_MAX_PWR_LVL; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_RET_STATE;
	}

	/* We expect the 'state id' to be zero */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

void rockchip_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * RockChip handler called when a CPU is about to enter standby.
 ******************************************************************************/
void rockchip_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	assert(cpu_state == PLAT_MAX_RET_STATE);

	scr = read_scr_el3();
	/* Enable PhysicalIRQ bit for NS world to wake the CPU */
	write_scr_el3(scr | SCR_IRQ_BIT);
	isb();
	dsb();
	wfi();

	/*
	 * Restore SCR to the original value, synchronisation of scr_el3 is
	 * done by eret while el3_exit to save some execution cycles.
	 */
	write_scr_el3(scr);
}

/*******************************************************************************
 * RockChip handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
int rockchip_pwr_domain_on(u_register_t mpidr)
{
	if (rockchip_ops && rockchip_ops->cores_pwr_dm_on)
		rockchip_ops->cores_pwr_dm_on(mpidr, rockchip_sec_entrypoint);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * RockChip handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void rockchip_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint32_t lvl;
	plat_local_state_t lvl_state;

	assert(RK_CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE);

	plat_rockchip_gic_cpuif_disable();

	if (RK_CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		plat_cci_disable();

	if (!rockchip_ops || !rockchip_ops->cores_pwr_dm_off)
		return;

	rockchip_ops->cores_pwr_dm_off();

	if (!rockchip_ops->hlvl_pwr_dm_off)
		return;

	for (lvl = MPIDR_AFFLVL1; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		lvl_state = target_state->pwr_domain_state[lvl];
		rockchip_ops->hlvl_pwr_dm_off(lvl, lvl_state);
	}
}

/*******************************************************************************
 * RockChip handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void rockchip_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint32_t lvl;
	plat_local_state_t lvl_state;

	if (RK_CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	if (rockchip_ops) {
		if (RK_SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE &&
		    rockchip_ops->sys_pwr_dm_suspend) {
			rockchip_ops->sys_pwr_dm_suspend();
		} else if (rockchip_ops->cores_pwr_dm_suspend) {
			rockchip_ops->cores_pwr_dm_suspend();
		}
	}

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_rockchip_gic_cpuif_disable();

	/* Perform the common cluster specific operations */
	if (RK_CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		plat_cci_disable();

	if (RK_SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		return;

	if (!rockchip_ops || !rockchip_ops->hlvl_pwr_dm_suspend)
		return;

	for (lvl = MPIDR_AFFLVL1; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		lvl_state = target_state->pwr_domain_state[lvl];
		rockchip_ops->hlvl_pwr_dm_suspend(lvl, lvl_state);
	}
}

/*******************************************************************************
 * RockChip handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void rockchip_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint32_t lvl;
	plat_local_state_t lvl_state;

	assert(RK_CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE);

	if (!rockchip_ops)
		goto comm_finish;

	if (rockchip_ops->hlvl_pwr_dm_on_finish) {
		for (lvl = MPIDR_AFFLVL1; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
			lvl_state = target_state->pwr_domain_state[lvl];
			rockchip_ops->hlvl_pwr_dm_on_finish(lvl, lvl_state);
		}
	}

	if (rockchip_ops->cores_pwr_dm_on_finish)
		rockchip_ops->cores_pwr_dm_on_finish();
comm_finish:

	/* Perform the common cluster specific operations */
	if (RK_CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		/* Enable coherency if this cluster was off */
		plat_cci_enable();
	}

	/* Enable the gic cpu interface */
	plat_rockchip_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_rockchip_gic_cpuif_enable();
}

/*******************************************************************************
 * RockChip handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
void rockchip_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint32_t lvl;
	plat_local_state_t lvl_state;

	/* Nothing to be done on waking up from retention from CPU level */
	if (RK_CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	/* Perform system domain restore if woken up from system suspend */
	if (!rockchip_ops)
		goto comm_finish;

	if (RK_SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		if (rockchip_ops->sys_pwr_dm_resume)
			rockchip_ops->sys_pwr_dm_resume();
		goto comm_finish;
	}

	if (rockchip_ops->hlvl_pwr_dm_resume) {
		for (lvl = MPIDR_AFFLVL1; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
			lvl_state = target_state->pwr_domain_state[lvl];
			rockchip_ops->hlvl_pwr_dm_resume(lvl, lvl_state);
		}
	}

	if (rockchip_ops->cores_pwr_dm_resume)
		rockchip_ops->cores_pwr_dm_resume();
	/*
	 * Program the gic per-cpu distributor or re-distributor interface.
	 * For sys power domain operation, resuming of the gic needs to operate
	 * in rockchip_ops->sys_pwr_dm_resume, according to the sys power mode
	 * implements.
	 */
	plat_rockchip_gic_cpuif_enable();

comm_finish:
	/* Perform the common cluster specific operations */
	if (RK_CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		/* Enable coherency if this cluster was off */
		plat_cci_enable();
	}
}

/*******************************************************************************
 * RockChip handlers to reboot the system
 ******************************************************************************/
static void __dead2 rockchip_system_reset(void)
{
	assert(rockchip_ops && rockchip_ops->sys_gbl_soft_reset);

	rockchip_ops->sys_gbl_soft_reset();
}

/*******************************************************************************
 * RockChip handlers to power off the system
 ******************************************************************************/
static void __dead2 rockchip_system_poweroff(void)
{
	assert(rockchip_ops && rockchip_ops->system_off);

	rockchip_ops->system_off();
}

/*******************************************************************************
 * Export the platform handlers via plat_rockchip_psci_pm_ops. The rockchip
 * standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t plat_rockchip_psci_pm_ops = {
	.cpu_standby = rockchip_cpu_standby,
	.pwr_domain_on = rockchip_pwr_domain_on,
	.pwr_domain_off = rockchip_pwr_domain_off,
	.pwr_domain_suspend = rockchip_pwr_domain_suspend,
	.pwr_domain_on_finish = rockchip_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = rockchip_pwr_domain_suspend_finish,
	.system_reset = rockchip_system_reset,
	.system_off = rockchip_system_poweroff,
	.validate_power_state = rockchip_validate_power_state,
	.get_sys_suspend_power_state = rockchip_get_sys_suspend_power_state
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_rockchip_psci_pm_ops;
	rockchip_sec_entrypoint = sec_entrypoint;
	return 0;
}

uintptr_t plat_get_sec_entrypoint(void)
{
	assert(rockchip_sec_entrypoint);
	return rockchip_sec_entrypoint;
}

void plat_setup_rockchip_pm_ops(struct rockchip_pm_ops_cb *ops)
{
	rockchip_ops = ops;
}

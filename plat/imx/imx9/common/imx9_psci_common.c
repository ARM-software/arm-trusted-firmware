/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/gicv3.h>
#include <lib/psci/psci.h>
#include <scmi_imx9.h>

#include <imx9_psci_common.h>
#include <imx9_sys_sleep.h>
#include <imx_scmi_client.h>
#include <plat_imx8.h>

#if SYS_PWR_FULL_CTRL == 1
#define PLAT_SCMI_SYS_PWR_SHUTDOWN	IMX9_SCMI_SYS_PWR_FULL_SHUTDOWN
#define PLAT_SCMI_SYS_PWR_COLD_RESET	IMX9_SCMI_SYS_PWR_FULL_RESET
#else
#define PLAT_SCMI_SYS_PWR_SHUTDOWN	SCMI_SYS_PWR_SHUTDOWN
#define PLAT_SCMI_SYS_PWR_COLD_RESET	SCMI_SYS_PWR_COLD_RESET
#endif

/* platform secure warm boot entry */
uintptr_t secure_entrypoint;

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(uint32_t power_state,
			     psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(uint32_t core_id, uint64_t boot_entry,
			    uint32_t flag)
{
	scmi_core_set_reset_addr(imx9_scmi_handle,
				 boot_entry,
				 SCMI_CPU_A55_ID(core_id),
				 flag);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t mask = DEBUG_WAKEUP_MASK | EVENT_WAKEUP_MASK;

	imx_set_cpu_boot_entry(core_id,
			       secure_entrypoint,
			       SCMI_CPU_VEC_FLAGS_BOOT);

	scmi_core_start(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id));

	/*
	 * Set NON-IRQ wakeup mask to Disable wakeup on DEBUG_WAKEUP
	 */
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id), 0U, 1U, mask);

	/* Set the default LPM state for cpuidle. */
	struct scmi_lpm_config cpu_lpm_cfg = {
		SCMI_CPU_A55_PD(core_id),
		SCMI_CPU_PD_LPM_ON_RUN,
		0U
	};

	scmi_core_lpm_mode_set(imx9_scmi_handle,
			       SCMI_CPU_A55_ID(core_id),
			       1U, &cpu_lpm_cfg);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);

	scmi_core_set_sleep_mode(imx9_scmi_handle,
				 SCMI_CPU_A55_ID(core_id),
				 SCMI_GIC_WAKEUP,
				 SCMI_CPU_SLEEP_WAIT);
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* Ensure the cluster can be powered off. */
	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/* Configure core LPM state for hotplug. */
	struct scmi_lpm_config cpu_lpm_cfg = {
		SCMI_CPU_A55_PD(core_id),
		SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
		0U
	};
	/* Set the default LPM state for cpuidle */
	scmi_core_lpm_mode_set(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id),
				1U, &cpu_lpm_cfg);

	/*
	 * Mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * so we need to use SW_WAKEUP for hotplug purpose
	 */
	scmi_core_Irq_wake_set(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id), 0U,
			       IMR_NUM, mask_all);

	scmi_core_set_sleep_mode(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id),
				 SCMI_GPC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t sys_mode;

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint, SCMI_CPU_VEC_FLAGS_RESUME);
	}

	/* config DSU for cluster power down */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* L3 retention */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
		} else {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);
		}
	}

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		imx9_sys_sleep_prepare(core_id);

		/* switch to GPC wakeup source, config the target mode to SUSPEND */
		scmi_core_set_sleep_mode(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id),
					 SCMI_GPC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);

		scmi_core_set_sleep_mode(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
					 SCMI_GPC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);

		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				SCMI_PWR_MIX_SLICE_IDX_A55P,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				0U
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_NOC,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				BIT_32(SCMI_PWR_MEM_SLICE_IDX_NOC_OCRAM)
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
				keep_wakeupmix_on ? SCMI_CPU_PD_LPM_ON_ALWAYS :
					SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				0U
			}
		};

		/* Set the default LPM state for suspend */
		scmi_core_lpm_mode_set(imx9_scmi_handle,
				       IMX9_SCMI_CPU_A55P,
				       ARRAY_SIZE(cpu_lpm_cfg),
				       cpu_lpm_cfg);

		/* Set the system sleep config */
		sys_mode = SCMI_IMX_SYS_POWER_STATE_MODE_MASK;
		if (has_netc_irq) {
			sys_mode |= SYS_SLEEP_MODE_H(SM_PERF_LVL_LOW);
			scmi_sys_pwr_state_set(imx9_scmi_handle,
					       SCMI_SYS_PWR_FORCEFUL_REQ,
					       sys_mode);
		}
	}

}
void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t sys_mode;

	/* system level */
	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		imx9_sys_sleep_unprepare(core_id);

		sys_mode = SCMI_IMX_SYS_POWER_STATE_MODE_MASK;
		if (has_netc_irq) {
			scmi_sys_pwr_state_set(imx9_scmi_handle,
					       SCMI_SYS_PWR_FORCEFUL_REQ,
					       sys_mode);
		}

		/* switch to GIC wakeup source, config the target mode to WAIT */
		scmi_core_set_sleep_mode(imx9_scmi_handle, SCMI_CPU_A55_ID(core_id),
					 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

		scmi_core_set_sleep_mode(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
					 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				SCMI_PWR_MIX_SLICE_IDX_A55P,
				SCMI_CPU_PD_LPM_ON_RUN,
				BIT_32(SCMI_PWR_MEM_SLICE_IDX_A55L3)
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_NOC,
				SCMI_CPU_PD_LPM_ON_ALWAYS,
				0U
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
				SCMI_CPU_PD_LPM_ON_ALWAYS,
				0U
			}
		};

		/* Set the default LPM state for RUN MODE */
		scmi_core_lpm_mode_set(imx9_scmi_handle,
				       IMX9_SCMI_CPU_A55P,
				       ARRAY_SIZE(cpu_lpm_cfg),
				       cpu_lpm_cfg);
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint32_t i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

void imx_pwr_domain_pwr_down(const psci_power_state_t *target_state)
{
}

void imx_system_off(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
				     SCMI_SYS_PWR_FORCEFUL_REQ,
				     PLAT_SCMI_SYS_PWR_SHUTDOWN);
	if (ret) {
		NOTICE("%s failed: %d\n", __func__, ret);
	}
}

void imx_system_reset(void)
{
	int ret;

	/* TODO: temp workaround for GIC to let reset done */
	gicd_clr_ctlr(PLAT_GICD_BASE,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Force: work, Gracefull: not work */
	ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
				     SCMI_SYS_PWR_FORCEFUL_REQ,
				     PLAT_SCMI_SYS_PWR_COLD_RESET);
	if (ret) {
		VERBOSE("%s failed: %d\n", __func__, ret);
	}
}

/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <lib/cassert.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>

/* Allow CSS platforms to override `plat_arm_psci_pm_ops` */
#pragma weak plat_arm_psci_pm_ops

#if ARM_RECOM_STATE_ID_ENC
/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
const unsigned int arm_pm_idle_states[] = {
	/* State-id - 0x001 */
	arm_make_pwrstate_lvl2(ARM_LOCAL_STATE_RUN, ARM_LOCAL_STATE_RUN,
		ARM_LOCAL_STATE_RET, ARM_PWR_LVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x002 */
	arm_make_pwrstate_lvl2(ARM_LOCAL_STATE_RUN, ARM_LOCAL_STATE_RUN,
		ARM_LOCAL_STATE_OFF, ARM_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x022 */
	arm_make_pwrstate_lvl2(ARM_LOCAL_STATE_RUN, ARM_LOCAL_STATE_OFF,
		ARM_LOCAL_STATE_OFF, ARM_PWR_LVL1, PSTATE_TYPE_POWERDOWN),
#if PLAT_MAX_PWR_LVL > ARM_PWR_LVL1
	/* State-id - 0x222 */
	arm_make_pwrstate_lvl2(ARM_LOCAL_STATE_OFF, ARM_LOCAL_STATE_OFF,
		ARM_LOCAL_STATE_OFF, ARM_PWR_LVL2, PSTATE_TYPE_POWERDOWN),
#endif
	0,
};
#endif /* __ARM_RECOM_STATE_ID_ENC__ */

/*
 * All the power management helpers in this file assume at least cluster power
 * level is supported.
 */
CASSERT(PLAT_MAX_PWR_LVL >= ARM_PWR_LVL1,
		assert_max_pwr_lvl_supported_mismatch);

/*
 * Ensure that the PLAT_MAX_PWR_LVL is not greater than CSS_SYSTEM_PWR_DMN_LVL
 * assumed by the CSS layer.
 */
CASSERT(PLAT_MAX_PWR_LVL <= CSS_SYSTEM_PWR_DMN_LVL,
		assert_max_pwr_lvl_higher_than_css_sys_lvl);

/*******************************************************************************
 * Handler called when a power domain is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int css_pwr_domain_on(u_register_t mpidr)
{
	css_scp_on(mpidr);

	return PSCI_E_SUCCESS;
}

static void css_pwr_domain_on_finisher_common(
		const psci_power_state_t *target_state)
{
	assert(CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF);

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (CSS_CLUSTER_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF)
		plat_arm_interconnect_enter_coherency();
}

/*******************************************************************************
 * Handler called when a power level has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from. This handler would never be invoked with
 * the system power domain uninitialized as either the primary would have taken
 * care of it as part of cold boot or the first core awakened from system
 * suspend would have already initialized it.
 ******************************************************************************/
void css_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* Assert that the system power domain need not be initialized */
	assert(css_system_pwr_state(target_state) == ARM_LOCAL_STATE_RUN);

	css_pwr_domain_on_finisher_common(target_state);
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on and the cpu
 * and its cluster are fully participating in coherent transaction on the
 * interconnect. Data cache must be enabled for CPU at this point.
 ******************************************************************************/
void css_pwr_domain_on_finish_late(const psci_power_state_t *target_state)
{
	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_arm_gic_pcpu_init();

	/* Enable the gic cpu interface */
	plat_arm_gic_cpuif_enable();
}

/*******************************************************************************
 * Common function called while turning a cpu off or suspending it. It is called
 * from css_off() or css_suspend() when these functions in turn are called for
 * power domain at the highest power level which will be powered down. It
 * performs the actions common to the OFF and SUSPEND calls.
 ******************************************************************************/
static void css_power_down_common(const psci_power_state_t *target_state)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	plat_arm_gic_cpuif_disable();

	/* Cluster is to be turned off, so disable coherency */
	if (CSS_CLUSTER_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF) {
		plat_arm_interconnect_exit_coherency();

#if HW_ASSISTED_COHERENCY
		uint32_t reg;

		/*
		 * If we have determined this core to be the last man standing and we
		 * intend to power down the cluster proactively, we provide a hint to
		 * the power controller that cluster power is not required when all
		 * cores are powered down.
		 * Note that this is only an advisory to power controller and is supported
		 * by SoCs with DynamIQ Shared Units only.
		 */
		reg = read_clusterpwrdn();

		/* Clear and set bit 0 : Cluster power not required */
		reg &= ~DSU_CLUSTER_PWR_MASK;
		reg |= DSU_CLUSTER_PWR_OFF;
		write_clusterpwrdn(reg);
#endif
	}
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void css_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF);
	css_power_down_common(target_state);
	css_scp_off(target_state);
}

/*******************************************************************************
 * Handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void css_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/*
	 * CSS currently supports retention only at cpu level. Just return
	 * as nothing is to be done for retention.
	 */
	if (CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_RET)
		return;


	assert(CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF);
	css_power_down_common(target_state);

	/* Perform system domain state saving if issuing system suspend */
	if (css_system_pwr_state(target_state) == ARM_LOCAL_STATE_OFF) {
		arm_system_pwr_domain_save();

		/* Power off the Redistributor after having saved its context */
		plat_arm_gic_redistif_off();
	}

	css_scp_suspend(target_state);
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
void css_pwr_domain_suspend_finish(
				const psci_power_state_t *target_state)
{
	/* Return as nothing is to be done on waking up from retention. */
	if (CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_RET)
		return;

	/* Perform system domain restore if woken up from system suspend */
	if (css_system_pwr_state(target_state) == ARM_LOCAL_STATE_OFF)
		/*
		 * At this point, the Distributor must be powered on to be ready
		 * to have its state restored. The Redistributor will be powered
		 * on as part of gicv3_rdistif_init_restore.
		 */
		arm_system_pwr_domain_resume();

	css_pwr_domain_on_finisher_common(target_state);

	/* Enable the gic cpu interface */
	plat_arm_gic_cpuif_enable();
}

/*******************************************************************************
 * Handlers to shutdown/reboot the system
 ******************************************************************************/
void __dead2 css_system_off(void)
{
	css_scp_sys_shutdown();
}

void __dead2 css_system_reset(void)
{
	css_scp_sys_reboot();
}

/*******************************************************************************
 * Handler called when the CPU power domain is about to enter standby.
 ******************************************************************************/
void css_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	assert(cpu_state == ARM_LOCAL_STATE_RET);

	scr = read_scr_el3();
	/*
	 * Enable the Non secure interrupt to wake the CPU.
	 * In GICv3 affinity routing mode, the non secure group1 interrupts use
	 * the PhysicalFIQ at EL3 whereas in GICv2, it uses the PhysicalIRQ.
	 * Enabling both the bits works for both GICv2 mode and GICv3 affinity
	 * routing mode.
	 */
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
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
 * Handler called to return the 'req_state' for system suspend.
 ******************************************************************************/
void css_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/*
	 * System Suspend is supported only if the system power domain node
	 * is implemented.
	 */
	assert(PLAT_MAX_PWR_LVL == CSS_SYSTEM_PWR_DMN_LVL);

	for (i = ARM_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = ARM_LOCAL_STATE_OFF;
}

/*******************************************************************************
 * Handler to query CPU/cluster power states from SCP
 ******************************************************************************/
int css_node_hw_state(u_register_t mpidr, unsigned int power_level)
{
	return css_scp_get_power_state(mpidr, power_level);
}

/*
 * The system power domain suspend is only supported only via
 * PSCI SYSTEM_SUSPEND API. PSCI CPU_SUSPEND request to system power domain
 * will be downgraded to the lower level.
 */
static int css_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int rc;
	rc = arm_validate_power_state(power_state, req_state);

	/*
	 * Ensure that we don't overrun the pwr_domain_state array in the case
	 * where the platform supported max power level is less than the system
	 * power level
	 */

#if (PLAT_MAX_PWR_LVL == CSS_SYSTEM_PWR_DMN_LVL)

	/*
	 * Ensure that the system power domain level is never suspended
	 * via PSCI CPU SUSPEND API. Currently system suspend is only
	 * supported via PSCI SYSTEM SUSPEND API.
	 */

	req_state->pwr_domain_state[CSS_SYSTEM_PWR_DMN_LVL] =
							ARM_LOCAL_STATE_RUN;
#endif

	return rc;
}

/*
 * Custom `translate_power_state_by_mpidr` handler for CSS. Unlike in the
 * `css_validate_power_state`, we do not downgrade the system power
 * domain level request in `power_state` as it will be used to query the
 * PSCI_STAT_COUNT/RESIDENCY at the system power domain level.
 */
static int css_translate_power_state_by_mpidr(u_register_t mpidr,
		unsigned int power_state,
		psci_power_state_t *output_state)
{
	return arm_validate_power_state(power_state, output_state);
}

/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t plat_arm_psci_pm_ops = {
	.pwr_domain_on		= css_pwr_domain_on,
	.pwr_domain_on_finish	= css_pwr_domain_on_finish,
	.pwr_domain_on_finish_late = css_pwr_domain_on_finish_late,
	.pwr_domain_off		= css_pwr_domain_off,
	.cpu_standby		= css_cpu_standby,
	.pwr_domain_suspend	= css_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= css_pwr_domain_suspend_finish,
	.system_off		= css_system_off,
	.system_reset		= css_system_reset,
	.validate_power_state	= css_validate_power_state,
	.validate_ns_entrypoint = arm_validate_psci_entrypoint,
	.translate_power_state_by_mpidr = css_translate_power_state_by_mpidr,
	.get_node_hw_state	= css_node_hw_state,
	.get_sys_suspend_power_state = css_get_sys_suspend_power_state,

#if defined(PLAT_ARM_MEM_PROT_ADDR)
	.mem_protect_chk	= arm_psci_mem_protect_chk,
	.read_mem_protect	= arm_psci_read_mem_protect,
	.write_mem_protect	= arm_nor_psci_write_mem_protect,
#endif
#if CSS_USE_SCMI_SDS_DRIVER
	.system_reset2		= css_system_reset2,
#endif
};

/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/arm/fvp/fvp_pwrc.h>
#include <lib/extensions/spe.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#include "fvp_private.h"
#include "../drivers/arm/gic/v3/gicv3_private.h"


#if ARM_RECOM_STATE_ID_ENC
/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
const unsigned int arm_pm_idle_states[] = {
	/* State-id - 0x01 */
	arm_make_pwrstate_lvl1(ARM_LOCAL_STATE_RUN, ARM_LOCAL_STATE_RET,
			ARM_PWR_LVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x02 */
	arm_make_pwrstate_lvl1(ARM_LOCAL_STATE_RUN, ARM_LOCAL_STATE_OFF,
			ARM_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x22 */
	arm_make_pwrstate_lvl1(ARM_LOCAL_STATE_OFF, ARM_LOCAL_STATE_OFF,
			ARM_PWR_LVL1, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x222 */
	arm_make_pwrstate_lvl2(ARM_LOCAL_STATE_OFF, ARM_LOCAL_STATE_OFF,
		ARM_LOCAL_STATE_OFF, ARM_PWR_LVL2, PSTATE_TYPE_POWERDOWN),
	0,
};
#endif

/*******************************************************************************
 * Function which implements the common FVP specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void fvp_cluster_pwrdwn_common(void)
{
	uint64_t mpidr = read_mpidr_el1();

#if ENABLE_SPE_FOR_LOWER_ELS
	/*
	 * On power down we need to disable statistical profiling extensions
	 * before exiting coherency.
	 */
	spe_disable();
#endif

	/* Disable coherency if this cluster is to be turned off */
	fvp_interconnect_disable();

	/* Program the power controller to turn the cluster off */
	fvp_pwrc_write_pcoffr(mpidr);
}

/*
 * Empty implementation of these hooks avoid setting the GICR_WAKER.Sleep bit
 * on ARM GICv3 implementations on FVP. This is required, because FVP does not
 * support SYSTEM_SUSPEND and it is `faked` in firmware. Hence, for wake up
 * from `fake` system suspend the GIC must not be powered off.
 */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{}

void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{}

static void fvp_power_domain_on_finish_common(const psci_power_state_t *target_state)
{
	unsigned long mpidr;

	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
					ARM_LOCAL_STATE_OFF);

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/* Perform the common cluster specific operations */
	if (target_state->pwr_domain_state[ARM_PWR_LVL1] ==
					ARM_LOCAL_STATE_OFF) {
		/*
		 * This CPU might have woken up whilst the cluster was
		 * attempting to power down. In this case the FVP power
		 * controller will have a pending cluster power off request
		 * which needs to be cleared by writing to the PPONR register.
		 * This prevents the power controller from interpreting a
		 * subsequent entry of this cpu into a simple wfi as a power
		 * down request.
		 */
		fvp_pwrc_write_pponr(mpidr);

		/* Enable coherency if this cluster was off */
		fvp_interconnect_enable();
	}
	/* Perform the common system specific operations */
	if (target_state->pwr_domain_state[ARM_PWR_LVL2] ==
						ARM_LOCAL_STATE_OFF)
		arm_system_pwr_domain_resume();

	/*
	 * Clear PWKUPR.WEN bit to ensure interrupts do not interfere
	 * with a cpu power down unless the bit is set again
	 */
	fvp_pwrc_clr_wen(mpidr);
}


/*******************************************************************************
 * FVP handler called when a CPU is about to enter standby.
 ******************************************************************************/
static void fvp_cpu_standby(plat_local_state_t cpu_state)
{

	assert(cpu_state == ARM_LOCAL_STATE_RET);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * FVP handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int fvp_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int psysr;

	/*
	 * Ensure that we do not cancel an inflight power off request for the
	 * target cpu. That would leave it in a zombie wfi. Wait for it to power
	 * off and then program the power controller to turn that CPU on.
	 */
	do {
		psysr = fvp_pwrc_read_psysr(mpidr);
	} while ((psysr & PSYSR_AFF_L0) != 0U);

	fvp_pwrc_write_pponr(mpidr);
	return rc;
}

/*******************************************************************************
 * FVP handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void fvp_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
					ARM_LOCAL_STATE_OFF);

	/*
	 * If execution reaches this stage then this power domain will be
	 * suspended. Perform at least the cpu specific actions followed
	 * by the cluster specific operations if applicable.
	 */

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_arm_gic_cpuif_disable();

	/* Turn redistributor off */
	plat_arm_gic_redistif_off();

	/* Program the power controller to power off this cpu. */
	fvp_pwrc_write_ppoffr(read_mpidr_el1());

	if (target_state->pwr_domain_state[ARM_PWR_LVL1] ==
					ARM_LOCAL_STATE_OFF)
		fvp_cluster_pwrdwn_common();

}

/*******************************************************************************
 * FVP handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void fvp_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned long mpidr;

	/*
	 * FVP has retention only at cpu level. Just return
	 * as nothing is to be done for retention.
	 */
	if (target_state->pwr_domain_state[ARM_PWR_LVL0] ==
					ARM_LOCAL_STATE_RET)
		return;

	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
					ARM_LOCAL_STATE_OFF);

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/* Program the power controller to enable wakeup interrupts. */
	fvp_pwrc_set_wen(mpidr);

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_arm_gic_cpuif_disable();

	/*
	 * The Redistributor is not powered off as it can potentially prevent
	 * wake up events reaching the CPUIF and/or might lead to losing
	 * register context.
	 */

	/* Perform the common cluster specific operations */
	if (target_state->pwr_domain_state[ARM_PWR_LVL1] ==
					ARM_LOCAL_STATE_OFF)
		fvp_cluster_pwrdwn_common();

	/* Perform the common system specific operations */
	if (target_state->pwr_domain_state[ARM_PWR_LVL2] ==
						ARM_LOCAL_STATE_OFF)
		arm_system_pwr_domain_save();

	/* Program the power controller to power off this cpu. */
	fvp_pwrc_write_ppoffr(read_mpidr_el1());
}

/*******************************************************************************
 * FVP handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
static void fvp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	fvp_power_domain_on_finish_common(target_state);

}

/*******************************************************************************
 * FVP handler called when a power domain has just been powered on and the cpu
 * and its cluster are fully participating in coherent transaction on the
 * interconnect. Data cache must be enabled for CPU at this point.
 ******************************************************************************/
static void fvp_pwr_domain_on_finish_late(const psci_power_state_t *target_state)
{
	/* Program GIC per-cpu distributor or re-distributor interface */
	plat_arm_gic_pcpu_init();

	/* Enable GIC CPU interface */
	plat_arm_gic_cpuif_enable();
}

/*******************************************************************************
 * FVP handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
static void fvp_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	/*
	 * Nothing to be done on waking up from retention from CPU level.
	 */
	if (target_state->pwr_domain_state[ARM_PWR_LVL0] ==
					ARM_LOCAL_STATE_RET)
		return;

	fvp_power_domain_on_finish_common(target_state);

	/* Enable GIC CPU interface */
	plat_arm_gic_cpuif_enable();
}

/*******************************************************************************
 * FVP handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 fvp_system_off(void)
{
	/* Write the System Configuration Control Register */
	mmio_write_32(V2M_SYSREGS_BASE + V2M_SYS_CFGCTRL,
		V2M_CFGCTRL_START |
		V2M_CFGCTRL_RW |
		V2M_CFGCTRL_FUNC(V2M_FUNC_SHUTDOWN));
	wfi();
	ERROR("FVP System Off: operation not handled.\n");
	panic();
}

static void __dead2 fvp_system_reset(void)
{
	/* Write the System Configuration Control Register */
	mmio_write_32(V2M_SYSREGS_BASE + V2M_SYS_CFGCTRL,
		V2M_CFGCTRL_START |
		V2M_CFGCTRL_RW |
		V2M_CFGCTRL_FUNC(V2M_FUNC_REBOOT));
	wfi();
	ERROR("FVP System Reset: operation not handled.\n");
	panic();
}

static int fvp_node_hw_state(u_register_t target_cpu,
			     unsigned int power_level)
{
	unsigned int psysr;
	int ret;

	/*
	 * The format of 'power_level' is implementation-defined, but 0 must
	 * mean a CPU. We also allow 1 to denote the cluster
	 */
	if ((power_level != ARM_PWR_LVL0) && (power_level != ARM_PWR_LVL1))
		return PSCI_E_INVALID_PARAMS;

	/*
	 * Read the status of the given MPDIR from FVP power controller. The
	 * power controller only gives us on/off status, so map that to expected
	 * return values of the PSCI call
	 */
	psysr = fvp_pwrc_read_psysr(target_cpu);
	if (psysr == PSYSR_INVALID)
		return PSCI_E_INVALID_PARAMS;

	if (power_level == ARM_PWR_LVL0) {
		ret = ((psysr & PSYSR_AFF_L0) != 0U) ? HW_ON : HW_OFF;
	} else {
		/* power_level == ARM_PWR_LVL1 */
		ret = ((psysr & PSYSR_AFF_L1) != 0U) ? HW_ON : HW_OFF;
	}

	return ret;
}

/*
 * The FVP doesn't truly support power management at SYSTEM power domain. The
 * SYSTEM_SUSPEND will be down-graded to the cluster level within the platform
 * layer. The `fake` SYSTEM_SUSPEND allows us to validate some of the driver
 * save and restore sequences on FVP.
 */
#if !ARM_BL31_IN_DRAM
static void fvp_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = ARM_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = ARM_LOCAL_STATE_OFF;
}
#endif

/*******************************************************************************
 * Handler to filter PSCI requests.
 ******************************************************************************/
/*
 * The system power domain suspend is only supported only via
 * PSCI SYSTEM_SUSPEND API. PSCI CPU_SUSPEND request to system power domain
 * will be downgraded to the lower level.
 */
static int fvp_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int rc;
	rc = arm_validate_power_state(power_state, req_state);

	/*
	 * Ensure that the system power domain level is never suspended
	 * via PSCI CPU SUSPEND API. Currently system suspend is only
	 * supported via PSCI SYSTEM SUSPEND API.
	 */
	req_state->pwr_domain_state[ARM_PWR_LVL2] = ARM_LOCAL_STATE_RUN;
	return rc;
}

/*
 * Custom `translate_power_state_by_mpidr` handler for FVP. Unlike in the
 * `fvp_validate_power_state`, we do not downgrade the system power
 * domain level request in `power_state` as it will be used to query the
 * PSCI_STAT_COUNT/RESIDENCY at the system power domain level.
 */
static int fvp_translate_power_state_by_mpidr(u_register_t mpidr,
		unsigned int power_state,
		psci_power_state_t *output_state)
{
	return arm_validate_power_state(power_state, output_state);
}

/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t plat_arm_psci_pm_ops = {
	.cpu_standby = fvp_cpu_standby,
	.pwr_domain_on = fvp_pwr_domain_on,
	.pwr_domain_off = fvp_pwr_domain_off,
	.pwr_domain_suspend = fvp_pwr_domain_suspend,
	.pwr_domain_on_finish = fvp_pwr_domain_on_finish,
	.pwr_domain_on_finish_late = fvp_pwr_domain_on_finish_late,
	.pwr_domain_suspend_finish = fvp_pwr_domain_suspend_finish,
	.system_off = fvp_system_off,
	.system_reset = fvp_system_reset,
	.validate_power_state = fvp_validate_power_state,
	.validate_ns_entrypoint = arm_validate_psci_entrypoint,
	.translate_power_state_by_mpidr = fvp_translate_power_state_by_mpidr,
	.get_node_hw_state = fvp_node_hw_state,
#if !ARM_BL31_IN_DRAM
	/*
	 * The TrustZone Controller is set up during the warmboot sequence after
	 * resuming the CPU from a SYSTEM_SUSPEND. If BL31 is located in SRAM
	 * this is  not a problem but, if it is in TZC-secured DRAM, it tries to
	 * reconfigure the same memory it is running on, causing an exception.
	 */
	.get_sys_suspend_power_state = fvp_get_sys_suspend_power_state,
#endif
	.mem_protect_chk	= arm_psci_mem_protect_chk,
	.read_mem_protect	= arm_psci_read_mem_protect,
	.write_mem_protect	= arm_nor_psci_write_mem_protect,
};

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return ops;
}

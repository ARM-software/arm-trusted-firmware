/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include <thunder_pwrc.h>
#include <thunder_private.h>
#undef GICD_SETSPI_NSR
#undef GICD_CLRSPI_NSR
#undef GICD_SETSPI_SR
#undef GICD_CLRSPI_SR
#undef GICD_TYPER
#undef GICD_IIDR
#include <gicv3.h>

/*******************************************************************************
 * Function which implements the common FVP specific operations to power down a
 * cpu in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void thunder_cpu_pwrdwn_common(void)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	gicv3_cpuif_disable(plat_my_core_pos());

	/* Program the power controller to power off this cpu. */
	thunder_pwrc_write_ppoffr(read_mpidr_el1());
}

/*******************************************************************************
 * Function which implements the common FVP specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void thunder_cluster_pwrdwn_common(void)
{
	uint64_t mpidr = read_mpidr_el1();

	/* Disable coherency if this cluster is to be turned off */

	/* Program the power controller to turn the cluster off */
	thunder_pwrc_write_pcoffr(mpidr);
}

static void thunder_power_domain_on_finish_common(const psci_power_state_t *target_state)
{
	unsigned long mpidr;

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					THUNDER_STATE_OFF);

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/*
	 * Clear PWKUPR.WEN bit to ensure interrupts do not interfere
	 * with a cpu power down unless the bit is set again
	 */
	thunder_pwrc_clr_wen(mpidr);
}


/*******************************************************************************
 * FVP handler called when a CPU is about to enter standby.
 ******************************************************************************/
void thunder_cpu_standby(plat_local_state_t cpu_state)
{

	assert(cpu_state == THUNDER_STATE_RET);

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
int thunder_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int psysr;

	/*
	 * Ensure that we do not cancel an inflight power off request for the
	 * target cpu. That would leave it in a zombie wfi. Wait for it to power
	 * off and then program the power controller to turn that CPU on.
	 */
	do {
		psysr = thunder_pwrc_read_psysr(mpidr);
	} while (psysr & PSYSR_AFF_L0);

	thunder_pwrc_write_pponr(mpidr);
	return rc;
}

/*******************************************************************************
 * FVP handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void thunder_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					THUNDER_STATE_OFF);

	/*
	 * If execution reaches this stage then this power domain will be
	 * suspended. Perform at least the cpu specific actions followed
	 * by the cluster specific operations if applicable.
	 */
	thunder_cpu_pwrdwn_common();

	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
					THUNDER_STATE_OFF)
		thunder_cluster_pwrdwn_common();

}

/*******************************************************************************
 * FVP handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void thunder_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned long mpidr;

	/*
	 * FVP has retention only at cpu level. Just return
	 * as nothing is to be done for retention.
	 */
	if (target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					THUNDER_STATE_RET)
		return;

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					THUNDER_STATE_OFF);

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/* Program the power controller to enable wakeup interrupts. */
	thunder_pwrc_set_wen(mpidr);

	/* Perform the common cpu specific operations */
	thunder_cpu_pwrdwn_common();

	/* Perform the common cluster specific operations */
	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
					THUNDER_STATE_OFF)
		thunder_cluster_pwrdwn_common();
}

/*******************************************************************************
 * FVP handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void thunder_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	thunder_power_domain_on_finish_common(target_state);

	/* Enable the gic cpu interface */
	gicv3_rdistif_init(plat_my_core_pos());

	/* Program the gic per-cpu distributor or re-distributor interface */
	gicv3_cpuif_enable(plat_my_core_pos());

	thunder_cpu_setup();
}

/*******************************************************************************
 * FVP handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
void thunder_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	/*
	 * Nothing to be done on waking up from retention from CPU level.
	 */
	if (target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					THUNDER_STATE_RET)
		return;

	thunder_power_domain_on_finish_common(target_state);

	/* Enable the gic cpu interface */
	gicv3_cpuif_enable(plat_my_core_pos());
}

/*******************************************************************************
 * FVP handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 thunder_system_off(void)
{
	ERROR("Thunder System Off: operation not handled.\n");
	panic();
}

static void __dead2 thunder_system_reset(void)
{
	unsigned long node = cavm_numa_local();
	union cavm_rst_soft_rst rst_soft_rst;
	union cavm_rst_ocx rst_ocx;

	rst_ocx.u = 0;
	CSR_WRITE_PA(node, CAVM_RST_OCX, rst_ocx.u);

	rst_ocx.u = CSR_READ_PA(node, CAVM_RST_OCX);
	rst_soft_rst.u = 0;
	rst_soft_rst.s.soft_rst = 1;
	CSR_WRITE_PA(node, CAVM_RST_SOFT_RST, rst_soft_rst.u);

	ERROR("Thunder System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power state
 * parameter.
 ******************************************************************************/
int thunder_validate_power_state(unsigned int power_state,
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
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] =
					THUNDER_STATE_RET;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					THUNDER_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * ARM standard platform handler called to check the validity of the non secure
 * entrypoint.
 ******************************************************************************/
int thunder_validate_ns_entrypoint(uintptr_t entrypoint)
{
	int i;
	unsigned node_count = thunder_get_node_count();
	uint64_t dram_end = 0;

	for (i = 0; i < node_count; i++)
		dram_end += thunder_dram_size_node(i);

	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= NS_IMAGE_BASE) && (entrypoint < (dram_end - 1)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

/*******************************************************************************
 * Export the platform handlers via plat_thunder_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t plat_thunder_psci_pm_ops = {
	.cpu_standby = thunder_cpu_standby,
	.pwr_domain_on = thunder_pwr_domain_on,
	.pwr_domain_off = thunder_pwr_domain_off,
	.pwr_domain_suspend = thunder_pwr_domain_suspend,
	.pwr_domain_on_finish = thunder_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = thunder_pwr_domain_suspend_finish,
	.system_off = thunder_system_off,
	.system_reset = thunder_system_reset,
	.validate_power_state = thunder_validate_power_state,
	.validate_ns_entrypoint = thunder_validate_ns_entrypoint
};

extern uint64_t thunder_sec_entry_point;

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{

	*psci_ops = &plat_thunder_psci_pm_ops;

	/* Setup mailbox with entry point. */
	thunder_sec_entry_point = sec_entrypoint;

	return 0;
}

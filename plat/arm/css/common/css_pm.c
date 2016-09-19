/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <cassert.h>
#include <css_pm.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>
#include "css_scpi.h"

/* Macros to read the CSS power domain state */
#define CSS_CORE_PWR_STATE(state)	(state)->pwr_domain_state[ARM_PWR_LVL0]
#define CSS_CLUSTER_PWR_STATE(state)	(state)->pwr_domain_state[ARM_PWR_LVL1]
#define CSS_SYSTEM_PWR_STATE(state)	((PLAT_MAX_PWR_LVL > ARM_PWR_LVL1) ?\
				(state)->pwr_domain_state[ARM_PWR_LVL2] : 0)

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

/*******************************************************************************
 * Handler called when a power domain is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int css_pwr_domain_on(u_register_t mpidr)
{
	/*
	 * SCP takes care of powering up parent power domains so we
	 * only need to care about level 0
	 */
	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);

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
	assert(CSS_SYSTEM_PWR_STATE(target_state) == ARM_LOCAL_STATE_RUN);

	css_pwr_domain_on_finisher_common(target_state);

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
	uint32_t cluster_state = scpi_power_on;
	uint32_t system_state = scpi_power_on;

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_arm_gic_cpuif_disable();

	/* Check if power down at system power domain level is requested */
	if (CSS_SYSTEM_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF)
			system_state = scpi_power_retention;

	/* Cluster is to be turned off, so disable coherency */
	if (CSS_CLUSTER_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF) {
		plat_arm_interconnect_exit_coherency();
		cluster_state = scpi_power_off;
	}

	/*
	 * Ask the SCP to power down the appropriate components depending upon
	 * their state.
	 */
	scpi_set_css_power_state(read_mpidr_el1(),
				 scpi_power_off,
				 cluster_state,
				 system_state);
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void css_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(CSS_CORE_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF);
	css_power_down_common(target_state);
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
	if (CSS_SYSTEM_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF)
		arm_system_pwr_domain_resume();
	else
		/* Enable the gic cpu interface */
		plat_arm_gic_cpuif_enable();

	css_pwr_domain_on_finisher_common(target_state);
}

/*******************************************************************************
 * Handlers to shutdown/reboot the system
 ******************************************************************************/
void __dead2 css_system_off(void)
{
	uint32_t response;

	/* Send the power down request to the SCP */
	response = scpi_sys_power_state(scpi_system_shutdown);

	if (response != SCP_OK) {
		ERROR("CSS System Off: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("CSS System Off: operation not handled.\n");
	panic();
}

void __dead2 css_system_reset(void)
{
	uint32_t response;

	/* Send the system reset request to the SCP */
	response = scpi_sys_power_state(scpi_system_reboot);

	if (response != SCP_OK) {
		ERROR("CSS System Reset: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("CSS System Reset: operation not handled.\n");
	panic();
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
	assert(PLAT_MAX_PWR_LVL >= ARM_PWR_LVL2);

	for (i = ARM_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = ARM_LOCAL_STATE_OFF;
}

/*******************************************************************************
 * Handler to query CPU/cluster power states from SCP
 ******************************************************************************/
int css_node_hw_state(u_register_t mpidr, unsigned int power_level)
{
	int rc, element;
	unsigned int cpu_state, cluster_state;

	/*
	 * The format of 'power_level' is implementation-defined, but 0 must
	 * mean a CPU. We also allow 1 to denote the cluster
	 */
	if (power_level != ARM_PWR_LVL0 && power_level != ARM_PWR_LVL1)
		return PSCI_E_INVALID_PARAMS;

	/* Query SCP */
	rc = scpi_get_css_power_state(mpidr, &cpu_state, &cluster_state);
	if (rc != 0)
		return PSCI_E_INVALID_PARAMS;

	/* Map power states of CPU and cluster to expected PSCI return codes */
	if (power_level == ARM_PWR_LVL0) {
		/*
		 * The CPU state returned by SCP is an 8-bit bit mask
		 * corresponding to each CPU in the cluster
		 */
		element = mpidr & MPIDR_AFFLVL_MASK;
		return CSS_CPU_PWR_STATE(cpu_state, element) ==
			CSS_CPU_PWR_STATE_ON ? HW_ON : HW_OFF;
	} else {
		assert(cluster_state == CSS_CLUSTER_PWR_STATE_ON ||
				cluster_state == CSS_CLUSTER_PWR_STATE_OFF);
		return cluster_state == CSS_CLUSTER_PWR_STATE_ON ? HW_ON :
			HW_OFF;
	}
}

/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t plat_arm_psci_pm_ops = {
	.pwr_domain_on		= css_pwr_domain_on,
	.pwr_domain_on_finish	= css_pwr_domain_on_finish,
	.pwr_domain_off		= css_pwr_domain_off,
	.cpu_standby		= css_cpu_standby,
	.pwr_domain_suspend	= css_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= css_pwr_domain_suspend_finish,
	.system_off		= css_system_off,
	.system_reset		= css_system_reset,
	.validate_power_state	= arm_validate_power_state,
	.validate_ns_entrypoint = arm_validate_ns_entrypoint,
	.get_node_hw_state	= css_node_hw_state
};

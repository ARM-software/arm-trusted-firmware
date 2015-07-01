/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <arch_helpers.h>
#include <arm_gic.h>
#include <cci.h>
#include <css_def.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include "css_scpi.h"

unsigned long wakeup_address;

/*******************************************************************************
 * Private function to program the mailbox for a cpu before it is released
 * from reset.
 ******************************************************************************/
static void css_program_mailbox(uint64_t mpidr, uint64_t address)
{
	uint64_t linear_id;
	uint64_t mbox;

	linear_id = plat_arm_calc_core_pos(mpidr);
	mbox = TRUSTED_MAILBOXES_BASE +	(linear_id << TRUSTED_MAILBOX_SHIFT);
	*((uint64_t *) mbox) = address;
	flush_dcache_range(mbox, sizeof(mbox));
}

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

	/*
	 * Setup mailbox with address for CPU entrypoint when it next powers up
	 */
	css_program_mailbox(mpidr, wakeup_address);

	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when a power level has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void css_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_OFF);

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (target_state->pwr_domain_state[ARM_PWR_LVL1] ==
						ARM_LOCAL_STATE_OFF)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	/* Enable the gic cpu interface */
	arm_gic_cpuif_setup();

	/* todo: Is this setup only needed after a cold boot? */
	arm_gic_pcpu_distif_setup();

	/* Clear the mailbox for this cpu. */
	css_program_mailbox(read_mpidr_el1(), 0);
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

	/* Prevent interrupts from spuriously waking up this cpu */
	arm_gic_cpuif_deactivate();

	/* Cluster is to be turned off, so disable coherency */
	if (target_state->pwr_domain_state[ARM_PWR_LVL1] ==
						ARM_LOCAL_STATE_OFF) {
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));
		cluster_state = scpi_power_off;
	}

	/*
	 * Ask the SCP to power down the appropriate components depending upon
	 * their state.
	 */
	scpi_set_css_power_state(read_mpidr_el1(),
				 scpi_power_off,
				 cluster_state,
				 scpi_power_on);
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void css_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_OFF);

	css_power_down_common(target_state);
}

/*******************************************************************************
 * Handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void css_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/*
	 * Juno has retention only at cpu level. Just return
	 * as nothing is to be done for retention.
	 */
	if (target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_RET)
		return;

	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_OFF);

	/*
	 * Setup mailbox with address for CPU entrypoint when it next powers up.
	 */
	css_program_mailbox(read_mpidr_el1(), wakeup_address);

	css_power_down_common(target_state);
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
static void css_pwr_domain_suspend_finish(
				const psci_power_state_t *target_state)
{
	/*
	 * Return as nothing is to be done on waking up from retention.
	 */
	if (target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_RET)
		return;

	css_pwr_domain_on_finish(target_state);
}

/*******************************************************************************
 * Handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 css_system_off(void)
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

static void __dead2 css_system_reset(void)
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
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_psci_ops_t css_ops = {
	.pwr_domain_on		= css_pwr_domain_on,
	.pwr_domain_on_finish	= css_pwr_domain_on_finish,
	.pwr_domain_off		= css_pwr_domain_off,
	.cpu_standby		= css_cpu_standby,
	.pwr_domain_suspend	= css_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= css_pwr_domain_suspend_finish,
	.system_off		= css_system_off,
	.system_reset		= css_system_reset,
	.validate_power_state	= arm_validate_power_state
};

/*******************************************************************************
 * Export the platform specific psci ops.
 ******************************************************************************/
int platform_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &css_ops;

	wakeup_address = sec_entrypoint;
	flush_dcache_range((unsigned long)&wakeup_address,
				sizeof(wakeup_address));
	return 0;
}

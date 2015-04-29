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

/*******************************************************************************
 * Private function to program the mailbox for a cpu before it is released
 * from reset.
 ******************************************************************************/
static void css_program_mailbox(uint64_t mpidr, uint64_t address)
{
	uint64_t linear_id;
	uint64_t mbox;

	linear_id = platform_get_core_pos(mpidr);
	mbox = TRUSTED_MAILBOXES_BASE +	(linear_id << TRUSTED_MAILBOX_SHIFT);
	*((uint64_t *) mbox) = address;
	flush_dcache_range(mbox, sizeof(mbox));
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int32_t css_affinst_on(uint64_t mpidr,
			uint64_t sec_entrypoint,
			uint32_t afflvl,
			uint32_t state)
{
	/*
	 * SCP takes care of powering up higher affinity levels so we
	 * only need to care about level 0
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return PSCI_E_SUCCESS;

	/*
	 * Setup mailbox with address for CPU entrypoint when it next powers up
	 */
	css_program_mailbox(mpidr, sec_entrypoint);

	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when an affinity instance has just been powered on after
 * being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
void css_affinst_on_finish(uint32_t afflvl, uint32_t state)
{
	unsigned long mpidr;

	/* Determine if any platform actions need to be executed. */
	if (arm_do_affinst_actions(afflvl, state) == -EAGAIN)
		return;

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	/* Enable the gic cpu interface */
	arm_gic_cpuif_setup();

	/* todo: Is this setup only needed after a cold boot? */
	arm_gic_pcpu_distif_setup();

	/* Clear the mailbox for this cpu. */
	css_program_mailbox(mpidr, 0);
}

/*******************************************************************************
 * Common function called while turning a cpu off or suspending it. It is called
 * from css_off() or css_suspend() when these functions in turn are called for
 * the highest affinity level which will be powered down. It performs the
 * actions common to the OFF and SUSPEND calls.
 ******************************************************************************/
static void css_power_down_common(uint32_t afflvl)
{
	uint32_t cluster_state = scpi_power_on;

	/* Prevent interrupts from spuriously waking up this cpu */
	arm_gic_cpuif_deactivate();

	/* Cluster is to be turned off, so disable coherency */
	if (afflvl > MPIDR_AFFLVL0) {
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
 * Handler called when an affinity instance is about to be turned off. The
 * level and mpidr determine the affinity instance. The 'state' arg. allows the
 * platform to decide whether the cluster is being turned off and take
 * appropriate actions.
 *
 * CAUTION: There is no guarantee that caches will remain turned on across calls
 * to this function as each affinity level is dealt with. So do not write & read
 * global variables across calls. It will be wise to do flush a write to the
 * global to prevent unpredictable results.
 ******************************************************************************/
static void css_affinst_off(uint32_t afflvl, uint32_t state)
{
	/* Determine if any platform actions need to be executed */
	if (arm_do_affinst_actions(afflvl, state) == -EAGAIN)
		return;

	css_power_down_common(afflvl);
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be suspended. The
 * level and mpidr determine the affinity instance. The 'state' arg. allows the
 * platform to decide whether the cluster is being turned off and take apt
 * actions. The 'sec_entrypoint' determines the address in BL3-1 from where
 * execution should resume.
 *
 * CAUTION: There is no guarantee that caches will remain turned on across calls
 * to this function as each affinity level is dealt with. So do not write & read
 * global variables across calls. It will be wise to do flush a write to the
 * global to prevent unpredictable results.
 ******************************************************************************/
static void css_affinst_suspend(uint64_t sec_entrypoint,
				    uint32_t afflvl,
				    uint32_t state)
{
	/* Determine if any platform actions need to be executed */
	if (arm_do_affinst_actions(afflvl, state) == -EAGAIN)
		return;

	/*
	 * Setup mailbox with address for CPU entrypoint when it next powers up.
	 */
	css_program_mailbox(read_mpidr_el1(), sec_entrypoint);

	css_power_down_common(afflvl);
}

/*******************************************************************************
 * Handler called when an affinity instance has just been powered on after
 * having been suspended earlier. The level and mpidr determine the affinity
 * instance.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
static void css_affinst_suspend_finish(uint32_t afflvl,
					   uint32_t state)
{
	css_affinst_on_finish(afflvl, state);
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
 * Handler called when an affinity instance is about to enter standby.
 ******************************************************************************/
void css_affinst_standby(unsigned int power_state)
{
	unsigned int scr;

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
static const plat_pm_ops_t css_ops = {
	.affinst_on		= css_affinst_on,
	.affinst_on_finish	= css_affinst_on_finish,
	.affinst_off		= css_affinst_off,
	.affinst_standby	= css_affinst_standby,
	.affinst_suspend	= css_affinst_suspend,
	.affinst_suspend_finish	= css_affinst_suspend_finish,
	.system_off		= css_system_off,
	.system_reset		= css_system_reset,
	.validate_power_state	= arm_validate_power_state
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &css_ops;
	return 0;
}

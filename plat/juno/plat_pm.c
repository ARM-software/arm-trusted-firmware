/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <cci400.h>
#include <errno.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include "juno_def.h"
#include "juno_private.h"
#include "scpi.h"

/*******************************************************************************
 * Private Juno function to program the mailbox for a cpu before it is released
 * from reset.
 ******************************************************************************/
static void juno_program_mailbox(uint64_t mpidr, uint64_t address)
{
	uint64_t linear_id;
	uint64_t mbox;

	linear_id = platform_get_core_pos(mpidr);
	mbox = TRUSTED_MAILBOXES_BASE +	(linear_id << TRUSTED_MAILBOX_SHIFT);
	*((uint64_t *) mbox) = address;
	flush_dcache_range(mbox, sizeof(mbox));
}

/*******************************************************************************
 * Private Juno function which is used to determine if any platform actions
 * should be performed for the specified affinity instance given its
 * state. Nothing needs to be done if the 'state' is not off or if this is not
 * the highest affinity level which will enter the 'state'.
 ******************************************************************************/
static int32_t juno_do_plat_actions(uint32_t afflvl, uint32_t state)
{
	uint32_t max_phys_off_afflvl;

	assert(afflvl <= MPIDR_AFFLVL1);

	if (state != PSCI_STATE_OFF)
		return -EAGAIN;

	/*
	 * Find the highest affinity level which will be suspended and postpone
	 * all the platform specific actions until that level is hit.
	 */
	max_phys_off_afflvl = psci_get_max_phys_off_afflvl();
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);
	assert(psci_get_suspend_afflvl() >= max_phys_off_afflvl);
	if (afflvl != max_phys_off_afflvl)
		return -EAGAIN;

	return 0;
}

/*******************************************************************************
 * Juno handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int32_t juno_affinst_on(uint64_t mpidr,
			uint64_t sec_entrypoint,
			uint64_t ns_entrypoint,
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
	juno_program_mailbox(mpidr, sec_entrypoint);

	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Juno handler called when an affinity instance has just been powered on after
 * being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
int32_t juno_affinst_on_finish(uint64_t mpidr, uint32_t afflvl, uint32_t state)
{
	/* Determine if any platform actions need to be executed. */
	if (juno_do_plat_actions(afflvl, state) == -EAGAIN)
		return PSCI_E_SUCCESS;

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		cci_enable_cluster_coherency(mpidr);


	/* Enable the gic cpu interface */
	gic_cpuif_setup(GICC_BASE);

	/* Juno todo: Is this setup only needed after a cold boot? */
	gic_pcpu_distif_setup(GICD_BASE);

	/* Clear the mailbox for this cpu. */
	juno_program_mailbox(mpidr, 0);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Common function called while turning a cpu off or suspending it. It is called
 * from juno_off() or juno_suspend() when these functions in turn are called for
 * the highest affinity level which will be powered down. It performs the
 * actions common to the OFF and SUSPEND calls.
 ******************************************************************************/
static int32_t juno_power_down_common(uint32_t afflvl)
{
	uint32_t cluster_state = scpi_power_on;

	/* Prevent interrupts from spuriously waking up this cpu */
	gic_cpuif_deactivate(GICC_BASE);

	/* Cluster is to be turned off, so disable coherency */
	if (afflvl > MPIDR_AFFLVL0) {
		cci_disable_cluster_coherency(read_mpidr_el1());
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

	return PSCI_E_SUCCESS;
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
static int32_t juno_affinst_off(uint64_t mpidr, uint32_t afflvl, uint32_t state)
{
	/* Determine if any platform actions need to be executed */
	if (juno_do_plat_actions(afflvl, state) == -EAGAIN)
		return PSCI_E_SUCCESS;

	return juno_power_down_common(afflvl);
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
static int32_t juno_affinst_suspend(uint64_t mpidr,
				    uint64_t sec_entrypoint,
				    uint64_t ns_entrypoint,
				    uint32_t afflvl,
				    uint32_t state)
{
	/* Determine if any platform actions need to be executed */
	if (juno_do_plat_actions(afflvl, state) == -EAGAIN)
		return PSCI_E_SUCCESS;

	/*
	 * Setup mailbox with address for CPU entrypoint when it next powers up.
	 */
	juno_program_mailbox(mpidr, sec_entrypoint);

	return juno_power_down_common(afflvl);
}

/*******************************************************************************
 * Juno handler called when an affinity instance has just been powered on after
 * having been suspended earlier. The level and mpidr determine the affinity
 * instance.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
static int32_t juno_affinst_suspend_finish(uint64_t mpidr,
					   uint32_t afflvl,
					   uint32_t state)
{
	return juno_affinst_on_finish(mpidr, afflvl, state);
}

/*******************************************************************************
 * Juno handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 juno_system_off(void)
{
	uint32_t response;

	/* Send the power down request to the SCP */
	response = scpi_sys_power_state(scpi_system_shutdown);

	if (response != SCP_OK) {
		ERROR("Juno System Off: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("Juno System Off: operation not handled.\n");
	panic();
}

static void __dead2 juno_system_reset(void)
{
	uint32_t response;

	/* Send the system reset request to the SCP */
	response = scpi_sys_power_state(scpi_system_reboot);

	if (response != SCP_OK) {
		ERROR("Juno System Reset: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("Juno System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t juno_ops = {
	.affinst_on		= juno_affinst_on,
	.affinst_on_finish	= juno_affinst_on_finish,
	.affinst_off		= juno_affinst_off,
	.affinst_suspend	= juno_affinst_suspend,
	.affinst_suspend_finish	= juno_affinst_suspend_finish,
	.system_off		= juno_system_off,
	.system_reset		= juno_system_reset
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &juno_ops;
	return 0;
}

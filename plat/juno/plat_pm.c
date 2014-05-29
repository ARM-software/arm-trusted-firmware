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

#include <arch_helpers.h>
#include <cci400.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include "juno_def.h"
#include "juno_private.h"
#include "scpi.h"

int pm_on(unsigned long mpidr,
		   unsigned long sec_entrypoint,
		   unsigned long ns_entrypoint,
		   unsigned int afflvl,
		   unsigned int state)
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
	unsigned long *mbox = (unsigned long *)(unsigned long)(
			TRUSTED_MAILBOXES_BASE +
			(platform_get_core_pos(mpidr) << TRUSTED_MAILBOX_SHIFT)
			);
	*mbox = sec_entrypoint;
	flush_dcache_range((unsigned long)mbox, sizeof(*mbox));

	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);

	return PSCI_E_SUCCESS;
}

int pm_on_finish(unsigned long mpidr, unsigned int afflvl, unsigned int state)
{
	switch (afflvl) {

	case MPIDR_AFFLVL1:
		/* Enable coherency if this cluster was off */
		if (state == PSCI_STATE_OFF)
			cci_enable_coherency(mpidr);
		break;

	case MPIDR_AFFLVL0:
		/*
		 * Ignore the state passed for a cpu. It could only have
		 * been off if we are here.
		 */

		/* Turn on intra-cluster coherency. */
		write_cpuectlr(read_cpuectlr() | CPUECTLR_SMP_BIT);

		/* Enable the gic cpu interface */
		gic_cpuif_setup(GICC_BASE);
		/* Juno todo: Is this setup only needed after a cold boot? */
		gic_pcpu_distif_setup(GICD_BASE);

		break;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned off. The
 * level and mpidr determine the affinity instance. The 'state' arg. allows the
 * platform to decide whether the cluster is being turned off and take apt
 * actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
int pm_off(unsigned long mpidr, unsigned int afflvl, unsigned int state)
{
	 /* We're only interested in power off states */
	if (state != PSCI_STATE_OFF)
		return PSCI_E_SUCCESS;

	switch (afflvl) {
	case MPIDR_AFFLVL1:
		/* Cluster is to be turned off, so disable coherency */
		cci_disable_coherency(mpidr);

		break;

	case MPIDR_AFFLVL0:
		/* Turn off intra-cluster coherency */
		write_cpuectlr(read_cpuectlr() & ~CPUECTLR_SMP_BIT);

		/* Prevent interrupts from spuriously waking up this cpu */
		gic_cpuif_deactivate(GICC_BASE);

		/*
		 * Ask SCP to power down CPU.
		 *
		 * Note, we also ask for cluster power down as well because we
		 * know the SCP will only actually do that if this is the last
		 * CPU going down, and also, that final power down won't happen
		 * until this CPU executes the WFI instruction after the PSCI
		 * framework has done it's thing.
		 */
		scpi_set_css_power_state(mpidr, scpi_power_off, scpi_power_off,
						scpi_power_retention);
		break;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t pm_ops = {
	.affinst_on		= pm_on,
	.affinst_on_finish	= pm_on_finish,
	.affinst_off		= pm_off
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the fvp power controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &pm_ops;
	return 0;
}

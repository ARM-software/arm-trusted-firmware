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
#include <psci.h>
#include <scpi.h>

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
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static plat_pm_ops pm_ops = {
	.affinst_on		= pm_on,
	.affinst_on_finish	= pm_on_finish
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the fvp power controller
 ******************************************************************************/
int platform_setup_pm(plat_pm_ops **plat_ops)
{
	*plat_ops = &pm_ops;
	return 0;
}

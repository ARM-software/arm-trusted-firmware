/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <bakery_lock.h>
#include <cci400.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include "drivers/pwrc/fvp_pwrc.h"
#include "fvp_def.h"
#include "fvp_private.h"

/*******************************************************************************
 * FVP handler called when an affinity instance is about to enter standby.
 ******************************************************************************/
int fvp_affinst_standby(unsigned int power_state)
{
	unsigned int target_afflvl;

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);

	/*
	 * It's possible to enter standby only on affinity level 0 i.e. a cpu
	 * on the FVP. Ignore any other affinity level.
	 */
	if (target_afflvl != MPIDR_AFFLVL0)
		return PSCI_E_INVALID_PARAMS;

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * FVP handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int fvp_affinst_on(unsigned long mpidr,
		   unsigned long sec_entrypoint,
		   unsigned long ns_entrypoint,
		   unsigned int afflvl,
		   unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned long linear_id;
	mailbox_t *fvp_mboxes;
	unsigned int psysr;

	/*
	 * It's possible to turn on only affinity level 0 i.e. a cpu
	 * on the FVP. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		goto exit;

	/*
	 * Ensure that we do not cancel an inflight power off request
	 * for the target cpu. That would leave it in a zombie wfi.
	 * Wait for it to power off, program the jump address for the
	 * target cpu and then program the power controller to turn
	 * that cpu on
	 */
	do {
		psysr = fvp_pwrc_read_psysr(mpidr);
	} while (psysr & PSYSR_AFF_L0);

	linear_id = platform_get_core_pos(mpidr);
	fvp_mboxes = (mailbox_t *) (TZDRAM_BASE + MBOX_OFF);
	fvp_mboxes[linear_id].value = sec_entrypoint;
	flush_dcache_range((unsigned long) &fvp_mboxes[linear_id],
			   sizeof(unsigned long));

	fvp_pwrc_write_pponr(mpidr);

exit:
	return rc;
}

/*******************************************************************************
 * FVP handler called when an affinity instance is about to be turned off. The
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
int fvp_affinst_off(unsigned long mpidr,
		    unsigned int afflvl,
		    unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int gicc_base, ectlr;
	unsigned long cpu_setup, cci_setup;

	switch (afflvl) {
	case MPIDR_AFFLVL1:
		if (state == PSCI_STATE_OFF) {
			/*
			 * Disable coherency if this cluster is to be
			 * turned off
			 */
			cci_setup = fvp_get_cfgvar(CONFIG_HAS_CCI);
			if (cci_setup) {
				cci_disable_coherency(mpidr);
			}

			/*
			 * Program the power controller to turn the
			 * cluster off
			 */
			fvp_pwrc_write_pcoffr(mpidr);

		}
		break;

	case MPIDR_AFFLVL0:
		if (state == PSCI_STATE_OFF) {

			/*
			 * Take this cpu out of intra-cluster coherency if
			 * the FVP flavour supports the SMP bit.
			 */
			cpu_setup = fvp_get_cfgvar(CONFIG_CPU_SETUP);
			if (cpu_setup) {
				ectlr = read_cpuectlr();
				ectlr &= ~CPUECTLR_SMP_BIT;
				write_cpuectlr(ectlr);
			}

			/*
			 * Prevent interrupts from spuriously waking up
			 * this cpu
			 */
			gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);
			gic_cpuif_deactivate(gicc_base);

			/*
			 * Program the power controller to power this
			 * cpu off
			 */
			fvp_pwrc_write_ppoffr(mpidr);
		}
		break;

	default:
		assert(0);
	}

	return rc;
}

/*******************************************************************************
 * FVP handler called when an affinity instance is about to be suspended. The
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
int fvp_affinst_suspend(unsigned long mpidr,
			unsigned long sec_entrypoint,
			unsigned long ns_entrypoint,
			unsigned int afflvl,
			unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int gicc_base, ectlr;
	unsigned long cpu_setup, cci_setup, linear_id;
	mailbox_t *fvp_mboxes;

	switch (afflvl) {
	case MPIDR_AFFLVL1:
		if (state == PSCI_STATE_OFF) {
			/*
			 * Disable coherency if this cluster is to be
			 * turned off
			 */
			cci_setup = fvp_get_cfgvar(CONFIG_HAS_CCI);
			if (cci_setup) {
				cci_disable_coherency(mpidr);
			}

			/*
			 * Program the power controller to turn the
			 * cluster off
			 */
			fvp_pwrc_write_pcoffr(mpidr);

		}
		break;

	case MPIDR_AFFLVL0:
		if (state == PSCI_STATE_OFF) {
			/*
			 * Take this cpu out of intra-cluster coherency if
			 * the FVP flavour supports the SMP bit.
			 */
			cpu_setup = fvp_get_cfgvar(CONFIG_CPU_SETUP);
			if (cpu_setup) {
				ectlr = read_cpuectlr();
				ectlr &= ~CPUECTLR_SMP_BIT;
				write_cpuectlr(ectlr);
			}

			/* Program the jump address for the target cpu */
			linear_id = platform_get_core_pos(mpidr);
			fvp_mboxes = (mailbox_t *) (TZDRAM_BASE + MBOX_OFF);
			fvp_mboxes[linear_id].value = sec_entrypoint;
			flush_dcache_range((unsigned long) &fvp_mboxes[linear_id],
					   sizeof(unsigned long));

			/*
			 * Prevent interrupts from spuriously waking up
			 * this cpu
			 */
			gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);
			gic_cpuif_deactivate(gicc_base);

			/*
			 * Program the power controller to power this
			 * cpu off and enable wakeup interrupts.
			 */
			fvp_pwrc_set_wen(mpidr);
			fvp_pwrc_write_ppoffr(mpidr);
		}
		break;

	default:
		assert(0);
	}

	return rc;
}

/*******************************************************************************
 * FVP handler called when an affinity instance has just been powered on after
 * being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
int fvp_affinst_on_finish(unsigned long mpidr,
			  unsigned int afflvl,
			  unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned long linear_id, cpu_setup;
	mailbox_t *fvp_mboxes;
	unsigned int gicd_base, gicc_base, reg_val, ectlr;

	switch (afflvl) {

	case MPIDR_AFFLVL1:
		/* Enable coherency if this cluster was off */
		if (state == PSCI_STATE_OFF) {

			/*
			 * This CPU might have woken up whilst the
			 * cluster was attempting to power down. In
			 * this case the FVP power controller will
			 * have a pending cluster power off request
			 * which needs to be cleared by writing to the
			 * PPONR register. This prevents the power
			 * controller from interpreting a subsequent
			 * entry of this cpu into a simple wfi as a
			 * power down request.
			 */
			fvp_pwrc_write_pponr(mpidr);

			fvp_cci_setup();
		}
		break;

	case MPIDR_AFFLVL0:
		/*
		 * Ignore the state passed for a cpu. It could only have
		 * been off if we are here.
		 */

		/*
		 * Turn on intra-cluster coherency if the FVP flavour supports
		 * it.
		 */
		cpu_setup = fvp_get_cfgvar(CONFIG_CPU_SETUP);
		if (cpu_setup) {
			ectlr = read_cpuectlr();
			ectlr |= CPUECTLR_SMP_BIT;
			write_cpuectlr(ectlr);
		}

		/*
		 * Clear PWKUPR.WEN bit to ensure interrupts do not interfere
		 * with a cpu power down unless the bit is set again
		 */
		fvp_pwrc_clr_wen(mpidr);

		/* Zero the jump address in the mailbox for this cpu */
		fvp_mboxes = (mailbox_t *) (TZDRAM_BASE + MBOX_OFF);
		linear_id = platform_get_core_pos(mpidr);
		fvp_mboxes[linear_id].value = 0;
		flush_dcache_range((unsigned long) &fvp_mboxes[linear_id],
				   sizeof(unsigned long));

		gicd_base = fvp_get_cfgvar(CONFIG_GICD_ADDR);
		gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);

		/* Enable the gic cpu interface */
		gic_cpuif_setup(gicc_base);

		/* TODO: This setup is needed only after a cold boot */
		gic_pcpu_distif_setup(gicd_base);

		/* Allow access to the System counter timer module */
		reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
		reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
		reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
		mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(0), reg_val);
		mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(1), reg_val);

		reg_val = (1 << CNTNSAR_NS_SHIFT(0)) |
			(1 << CNTNSAR_NS_SHIFT(1));
		mmio_write_32(SYS_TIMCTL_BASE + CNTNSAR, reg_val);

		break;

	default:
		assert(0);
	}

	return rc;
}

/*******************************************************************************
 * FVP handler called when an affinity instance has just been powered on after
 * having been suspended earlier. The level and mpidr determine the affinity
 * instance.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
int fvp_affinst_suspend_finish(unsigned long mpidr,
			       unsigned int afflvl,
			       unsigned int state)
{
	return fvp_affinst_on_finish(mpidr, afflvl, state);
}


/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t fvp_plat_pm_ops = {
	fvp_affinst_standby,
	fvp_affinst_on,
	fvp_affinst_off,
	fvp_affinst_suspend,
	fvp_affinst_on_finish,
	fvp_affinst_suspend_finish,
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the fvp power controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &fvp_plat_pm_ops;
	return 0;
}

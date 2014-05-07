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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <platform.h>

/*******************************************************************************
 * This duplicates what the primary cpu did after a cold boot in BL1. The same
 * needs to be done when a cpu is hotplugged in. This function could also over-
 * ride any EL3 setup done by BL1 as this code resides in rw memory.
 ******************************************************************************/
void bl31_arch_setup(void)
{
	unsigned long tmp_reg = 0;
	uint64_t counter_freq;

	/* Enable alignment checks */
	tmp_reg = read_sctlr_el3();
	tmp_reg |= (SCTLR_A_BIT | SCTLR_SA_BIT);
	write_sctlr_el3(tmp_reg);

	/*
	 * Enable HVCs, route FIQs to EL3 and set the next EL to be AArch64
	 */
	tmp_reg = SCR_RES1_BITS | SCR_RW_BIT | SCR_HCE_BIT | SCR_FIQ_BIT;
	write_scr(tmp_reg);

	/*
	 * Enable SError and Debug exceptions
	 */
	enable_serror();
	enable_debug_exceptions();

	/* Program the counter frequency */
	counter_freq = plat_get_syscnt_freq();
	write_cntfrq_el0(counter_freq);
}

/*******************************************************************************
 * Detect what the security state of the next EL is and setup the minimum
 * required architectural state: program SCTRL to reflect the RES1 bits, and to
 * have MMU and caches disabled
 ******************************************************************************/
void bl31_next_el_arch_setup(uint32_t security_state)
{
	unsigned long id_aa64pfr0 = read_id_aa64pfr0_el1();
	unsigned long next_sctlr;
	unsigned long el_status;
	unsigned long scr = read_scr();

	/* Use the same endianness than the current BL */
	next_sctlr = (read_sctlr_el3() & SCTLR_EE_BIT);

	/* Find out which EL we are going to */
	el_status = (id_aa64pfr0 >> ID_AA64PFR0_EL2_SHIFT) & ID_AA64PFR0_ELX_MASK;

	if (security_state == NON_SECURE) {
		/* Check if EL2 is supported */
		if (el_status && (scr & SCR_HCE_BIT)) {
			/* Set SCTLR EL2 */
			next_sctlr |= SCTLR_EL2_RES1;
			write_sctlr_el2(next_sctlr);
			return;
		}
	}

	/*
	 * SCTLR_EL1 needs the same programming irrespective of the
	 * security state of EL1.
	 */
	next_sctlr |= SCTLR_EL1_RES1;
	write_sctlr_el1(next_sctlr);
}

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

/*******************************************************************************
 * Function that does the first bit of architectural setup that affects
 * execution in the non-secure address space.
 ******************************************************************************/
void bl1_arch_setup(void)
{
	unsigned long tmp_reg = 0;

	/* Enable alignment checks */
	tmp_reg = read_sctlr_el3();
	tmp_reg |= (SCTLR_A_BIT | SCTLR_SA_BIT);
	write_sctlr_el3(tmp_reg);
	isb();

	/*
	 * Enable HVCs, route FIQs to EL3, set the next EL to be AArch64, route
	 * external abort and SError interrupts to EL3
	 */
	tmp_reg = SCR_RES1_BITS | SCR_RW_BIT | SCR_HCE_BIT | SCR_EA_BIT |
		  SCR_FIQ_BIT;
	write_scr(tmp_reg);

	/*
	 * Enable SError and Debug exceptions
	 */
	enable_serror();
	enable_debug_exceptions();
}

/*******************************************************************************
 * Set the Secure EL1 required architectural state
 ******************************************************************************/
void bl1_arch_next_el_setup(void) {
	unsigned long next_sctlr;

	/* Use the same endianness than the current BL */
	next_sctlr = (read_sctlr_el3() & SCTLR_EE_BIT);

	/* Set SCTLR Secure EL1 */
	next_sctlr |= SCTLR_EL1_RES1;

	write_sctlr_el1(next_sctlr);
}

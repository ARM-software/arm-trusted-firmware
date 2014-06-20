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
	 * Route external abort and SError interrupts to EL3
	 * other SCR bits will be configured before exiting to a lower exception
	 * level
	 */
	tmp_reg = SCR_RES1_BITS | SCR_EA_BIT;
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

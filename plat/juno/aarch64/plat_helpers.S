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
#include <platform.h>

	.globl	plat_report_exception

	.section	.text, "ax"

	/* ---------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * On FVP platform, it updates the LEDs
	 * to indicate where we are
	 * ---------------------------------------------
	 */
plat_report_exception:
	/* Juno todo: Come up with a way of reporting errors */
	ret

	/*
	 * Return 0 to 3 for the A53s and 4 or 5 for the A57s
	 */
	.globl	platform_get_core_pos
platform_get_core_pos:; .type platform_get_core_pos, %function
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	eor	x0, x0, #(1 << MPIDR_AFFINITY_BITS)  // swap A53/A57 order
	add	x0, x1, x0, LSR #6
	ret


	/* -----------------------------------------------------
	 * void platform_is_primary_cpu (unsigned int mpid);
	 *
	 * Given the mpidr say whether this cpu is the primary
	 * cpu (applicable ony after a cold boot)
	 * -----------------------------------------------------
	 */
	.globl	platform_is_primary_cpu
platform_is_primary_cpu:; .type platform_is_primary_cpu, %function
	/* Warning: this function is called without a valid stack */
	/* Juno todo: allow configuration of primary CPU using SCC */
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PRIMARY_CPU
	cset	x0, eq
	ret

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
#include <asm_macros.S>
#include <psci.h>

	.globl	psci_aff_on_finish_entry
	.globl	psci_aff_suspend_finish_entry
	.globl	__psci_cpu_off
	.globl	__psci_cpu_suspend
	.globl	psci_power_down_wfi

	/* -----------------------------------------------------
	 * This cpu has been physically powered up. Depending
	 * upon whether it was resumed from suspend or simply
	 * turned on, call the common power on finisher with
	 * the handlers (chosen depending upon original state).
	 * For ease, the finisher is called with coherent
	 * stacks. This allows the cluster/cpu finishers to
	 * enter coherency and enable the mmu without running
	 * into issues. We switch back to normal stacks once
	 * all this is done.
	 * -----------------------------------------------------
	 */
func psci_aff_on_finish_entry
	adr	x23, psci_afflvl_on_finishers
	b	psci_aff_common_finish_entry

psci_aff_suspend_finish_entry:
	adr	x23, psci_afflvl_suspend_finishers

psci_aff_common_finish_entry:
	adr	x22, psci_afflvl_power_on_finish

	/* ---------------------------------------------
	 * Exceptions should not occur at this point.
	 * Set VBAR in order to handle and report any
	 * that do occur
	 * ---------------------------------------------
	 */
	adr	x0, early_exceptions
	msr	vbar_el3, x0
	isb

	/* ---------------------------------------------
	 * Use SP_EL0 for the C runtime stack.
	 * ---------------------------------------------
	 */
	msr	spsel, #0

	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Call the finishers starting from affinity
	 * level 0.
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	get_power_on_target_afflvl
	cmp	x0, xzr
	b.lt	_panic
	mov	x3, x23
	mov	x2, x0
	mov	x1, #MPIDR_AFFLVL0
	mrs	x0, mpidr_el1
	blr	x22

	/* --------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_stack

	b	el3_exit
_panic:
	b	_panic

	/* -----------------------------------------------------
	 * The following two stubs give the calling cpu a
	 * coherent stack to allow flushing of caches without
	 * suffering from stack coherency issues
	 * -----------------------------------------------------
	 */
func __psci_cpu_off
	func_prologue
	sub	sp, sp, #0x10
	stp	x19, x20, [sp, #0]
	mov	x19, sp
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack
	bl	psci_cpu_off
	mov	sp, x19
	ldp	x19, x20, [sp,#0]
	add	sp, sp, #0x10
	func_epilogue
	ret

func __psci_cpu_suspend
	func_prologue
	sub	sp, sp, #0x20
	stp	x19, x20, [sp, #0]
	stp	x21, x22, [sp, #0x10]
	mov	x19, sp
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	bl	psci_cpu_suspend
	mov	sp, x19
	ldp	x21, x22, [sp,#0x10]
	ldp	x19, x20, [sp,#0]
	add	sp, sp, #0x20
	func_epilogue
	ret

	/* --------------------------------------------
	 * This function is called to indicate to the
	 * power controller that it is safe to power
	 * down this cpu. It should not exit the wfi
	 * and will be released from reset upon power
	 * up. 'wfi_spill' is used to catch erroneous
	 * exits from wfi.
	 * --------------------------------------------
	 */
func psci_power_down_wfi
	dsb	sy		// ensure write buffer empty
	wfi
wfi_spill:
	b	wfi_spill


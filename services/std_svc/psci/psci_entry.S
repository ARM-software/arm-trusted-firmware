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
#include <xlat_tables.h>

	.globl	psci_aff_on_finish_entry
	.globl	psci_aff_suspend_finish_entry
	.globl	psci_power_down_wfi

	/* -----------------------------------------------------
	 * This cpu has been physically powered up. Depending
	 * upon whether it was resumed from suspend or simply
	 * turned on, call the common power on finisher with
	 * the handlers (chosen depending upon original state).
	 * -----------------------------------------------------
	 */
func psci_aff_on_finish_entry
	adr	x23, psci_afflvl_on_finishers
	b	psci_aff_common_finish_entry

psci_aff_suspend_finish_entry:
	adr	x23, psci_afflvl_suspend_finishers

psci_aff_common_finish_entry:
#if !RESET_TO_BL31
	/* ---------------------------------------------
	 * Perform any processor specific actions which
	 * undo or are in addition to the actions
	 * performed by the reset handler in the BootROM
	 * (BL1) e.g. cache, tlb invalidations, errata
	 * workarounds etc.
	 * ---------------------------------------------
	 */
	bl      reset_handler

	/* ---------------------------------------------
	 * Enable the instruction cache, stack pointer
	 * and data access alignment checks.
	 * It can be assumed that BL3-1 entrypoint code
	 * will do this when RESET_TO_BL31 is set. The
	 * same  assumption cannot be made when another
	 * boot loader executes before BL3-1 in the warm
	 * boot path e.g. BL1.
	 * ---------------------------------------------
	 */
	mov	x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
	mrs	x0, sctlr_el3
	orr	x0, x0, x1
	msr	sctlr_el3, x0
	isb
#endif

	/* ---------------------------------------------
	 * Initialise the pcpu cache pointer for the CPU
	 * ---------------------------------------------
	 */
	bl	init_cpu_data_ptr

	/* ---------------------------------------------
	 * Initialize the cpu_ops pointer.
	 * ---------------------------------------------
	 */
	bl	init_cpu_ops

	/* ---------------------------------------------
	 * Set the exception vectors
	 * ---------------------------------------------
	 */
	adr	x0, runtime_exceptions
	msr	vbar_el3, x0
	isb

	/* ---------------------------------------------
	 * Enable the SError interrupt now that the
	 * exception vectors have been setup.
	 * ---------------------------------------------
	 */
	msr	daifclr, #DAIF_ABT_BIT

	/* ---------------------------------------------
	 * Use SP_EL0 for the C runtime stack.
	 * ---------------------------------------------
	 */
	msr	spsel, #0

	/* --------------------------------------------
	 * Give ourselves a stack whose memory will be
	 * marked as Normal-IS-WBWA when the MMU is
	 * enabled.
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_stack

	/* --------------------------------------------
	 * Enable the MMU with the DCache disabled. It
	 * is safe to use stacks allocated in normal
	 * memory as a result. All memory accesses are
	 * marked nGnRnE when the MMU is disabled. So
	 * all the stack writes will make it to memory.
	 * All memory accesses are marked Non-cacheable
	 * when the MMU is enabled but D$ is disabled.
	 * So used stack memory is guaranteed to be
	 * visible immediately after the MMU is enabled
	 * Enabling the DCache at the same time as the
	 * MMU can lead to speculatively fetched and
	 * possibly stale stack memory being read from
	 * other caches. This can lead to coherency
	 * issues.
	 * --------------------------------------------
	 */
	mov	x0, #DISABLE_DCACHE
	bl	bl31_plat_enable_mmu

	/* ---------------------------------------------
	 * Call the finishers starting from affinity
	 * level 0.
	 * ---------------------------------------------
	 */
	bl	get_power_on_target_afflvl
	mov	x2, x23
	mov	x1, x0
	mov	x0, #MPIDR_AFFLVL0
	bl	psci_afflvl_power_on_finish

	b	el3_exit

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


/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __EL3_COMMON_MACROS_S__
#define __EL3_COMMON_MACROS_S__

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	/*
	 * Helper macro to initialise EL3 registers we care about.
	 */
	.macro el3_arch_init_common _exception_vectors
	/* ---------------------------------------------------------------------
	 * Enable the instruction cache and alignment checks
	 * ---------------------------------------------------------------------
	 */
	ldr	r1, =(SCTLR_RES1 | SCTLR_I_BIT | SCTLR_A_BIT)
	ldcopr	r0, SCTLR
	orr	r0, r0, r1
	stcopr	r0, SCTLR
	isb

	/* ---------------------------------------------------------------------
	 * Set the exception vectors (VBAR/MVBAR).
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =\_exception_vectors
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	isb

	/* -----------------------------------------------------
	 * Enable the SIF bit to disable instruction fetches
	 * from Non-secure memory.
	 * -----------------------------------------------------
	 */
	ldcopr	r0, SCR
	orr	r0, r0, #SCR_SIF_BIT
	stcopr	r0, SCR

	/* -----------------------------------------------------
	 * Enable the Asynchronous data abort now that the
	 * exception vectors have been setup.
	 * -----------------------------------------------------
	 */
	cpsie   a
	isb

	/* Enable access to Advanced SIMD registers */
	ldcopr	r0, NSACR
	bic	r0, r0, #NSASEDIS_BIT
	bic	r0, r0, #NSTRCDIS_BIT
	orr	r0, r0, #(NASCR_CP10_BIT | NASCR_CP11_BIT)
	stcopr	r0, NSACR
	isb

	/*
	 * Enable access to Advanced SIMD, Floating point and to the Trace
	 * functionality as well.
	 */
	ldcopr	r0, CPACR
	bic	r0, r0, #ASEDIS_BIT
	bic	r0, r0, #TRCDIS_BIT
	orr	r0, r0, #CPACR_ENABLE_FP_ACCESS
	stcopr	r0, CPACR
	isb

	vmrs	r0, FPEXC
	orr	r0, r0, #FPEXC_EN_BIT
	vmsr	FPEXC, r0
	isb
	.endm

/* -----------------------------------------------------------------------------
 * This is the super set of actions that need to be performed during a cold boot
 * or a warm boot in EL3. This code is shared by BL1 and BL32 (SP_MIN).
 *
 * This macro will always perform reset handling, architectural initialisations
 * and stack setup. The rest of the actions are optional because they might not
 * be needed, depending on the context in which this macro is called. This is
 * why this macro is parameterised ; each parameter allows to enable/disable
 * some actions.
 *
 *  _set_endian:
 *	Whether the macro needs to configure the endianness of data accesses.
 *
 *  _warm_boot_mailbox:
 *	Whether the macro needs to detect the type of boot (cold/warm). The
 *	detection is based on the platform entrypoint address : if it is zero
 *	then it is a cold boot, otherwise it is a warm boot. In the latter case,
 *	this macro jumps on the platform entrypoint address.
 *
 *  _secondary_cold_boot:
 *	Whether the macro needs to identify the CPU that is calling it: primary
 *	CPU or secondary CPU. The primary CPU will be allowed to carry on with
 *	the platform initialisations, while the secondaries will be put in a
 *	platform-specific state in the meantime.
 *
 *	If the caller knows this macro will only be called by the primary CPU
 *	then this parameter can be defined to 0 to skip this step.
 *
 * _init_memory:
 *	Whether the macro needs to initialise the memory.
 *
 * _init_c_runtime:
 *	Whether the macro needs to initialise the C runtime environment.
 *
 * _exception_vectors:
 *	Address of the exception vectors to program in the VBAR_EL3 register.
 * -----------------------------------------------------------------------------
 */
	.macro el3_entrypoint_common					\
		_set_endian, _warm_boot_mailbox, _secondary_cold_boot,	\
		_init_memory, _init_c_runtime, _exception_vectors

	/* Make sure we are in Secure Mode */
#if ASM_ASSERTION
	ldcopr	r0, SCR
	tst	r0, #SCR_NS_BIT
	ASM_ASSERT(eq)
#endif

	.if \_set_endian
		/* -------------------------------------------------------------
		 * Set the CPU endianness before doing anything that might
		 * involve memory reads or writes.
		 * -------------------------------------------------------------
		 */
		ldcopr	r0, SCTLR
		bic	r0, r0, #SCTLR_EE_BIT
		stcopr	r0, SCTLR
		isb
	.endif /* _set_endian */

	/* Switch to monitor mode */
	cps	#MODE32_mon
	isb

	.if \_warm_boot_mailbox
		/* -------------------------------------------------------------
		 * This code will be executed for both warm and cold resets.
		 * Now is the time to distinguish between the two.
		 * Query the platform entrypoint address and if it is not zero
		 * then it means it is a warm boot so jump to this address.
		 * -------------------------------------------------------------
		 */
		bl	plat_get_my_entrypoint
		cmp	r0, #0
		bxne	r0
	.endif /* _warm_boot_mailbox */

	/* ---------------------------------------------------------------------
	 * It is a cold boot.
	 * Perform any processor specific actions upon reset e.g. cache, TLB
	 * invalidations etc.
	 * ---------------------------------------------------------------------
	 */
	bl	reset_handler

	el3_arch_init_common \_exception_vectors

	.if \_secondary_cold_boot
		/* -------------------------------------------------------------
		 * Check if this is a primary or secondary CPU cold boot.
		 * The primary CPU will set up the platform while the
		 * secondaries are placed in a platform-specific state until the
		 * primary CPU performs the necessary actions to bring them out
		 * of that state and allows entry into the OS.
		 * -------------------------------------------------------------
		 */
		bl	plat_is_my_cpu_primary
		cmp	r0, #0
		bne	do_primary_cold_boot

		/* This is a cold boot on a secondary CPU */
		bl	plat_secondary_cold_boot_setup
		/* plat_secondary_cold_boot_setup() is not supposed to return */
		bl	plat_panic_handler

	do_primary_cold_boot:
	.endif /* _secondary_cold_boot */

	/* ---------------------------------------------------------------------
	 * Initialize memory now. Secondary CPU initialization won't get to this
	 * point.
	 * ---------------------------------------------------------------------
	 */

	.if \_init_memory
		bl	platform_mem_init
	.endif /* _init_memory */

	/* ---------------------------------------------------------------------
	 * Init C runtime environment:
	 *   - Zero-initialise the NOBITS sections. There are 2 of them:
	 *       - the .bss section;
	 *       - the coherent memory section (if any).
	 *   - Relocate the data section from ROM to RAM, if required.
	 * ---------------------------------------------------------------------
	 */
	.if \_init_c_runtime
#if IMAGE_BL32
		/* -----------------------------------------------------------------
		 * Invalidate the RW memory used by the BL32 (SP_MIN) image. This
		 * includes the data and NOBITS sections. This is done to
		 * safeguard against possible corruption of this memory by
		 * dirty cache lines in a system cache as a result of use by
		 * an earlier boot loader stage.
		 * -----------------------------------------------------------------
		 */
		ldr	r0, =__RW_START__
		ldr	r1, =__RW_END__
		sub	r1, r1, r0
		bl	inv_dcache_range
#endif /* IMAGE_BL32 */

		ldr	r0, =__BSS_START__
		ldr	r1, =__BSS_SIZE__
		bl	zeromem

#if USE_COHERENT_MEM
		ldr	r0, =__COHERENT_RAM_START__
		ldr	r1, =__COHERENT_RAM_UNALIGNED_SIZE__
		bl	zeromem
#endif

#if IMAGE_BL1
		/* -----------------------------------------------------
		 * Copy data from ROM to RAM.
		 * -----------------------------------------------------
		 */
		ldr	r0, =__DATA_RAM_START__
		ldr	r1, =__DATA_ROM_START__
		ldr	r2, =__DATA_SIZE__
		bl	memcpy4
#endif
	.endif /* _init_c_runtime */

	/* ---------------------------------------------------------------------
	 * Allocate a stack whose memory will be marked as Normal-IS-WBWA when
	 * the MMU is enabled. There is no risk of reading stale stack memory
	 * after enabling the MMU as only the primary CPU is running at the
	 * moment.
	 * ---------------------------------------------------------------------
	 */
	bl	plat_set_my_stack
	.endm

#endif /* __EL3_COMMON_MACROS_S__ */

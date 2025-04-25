/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_COMMON_MACROS_S
#define EL3_COMMON_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <context.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

	/*
	 * Helper macro to initialise EL3 registers we care about.
	 */
	.macro el3_arch_init_common
	/* ---------------------------------------------------------------------
	 * SCTLR_EL3 has already been initialised - read current value before
	 * modifying.
	 *
	 * SCTLR_EL3.I: Enable the instruction cache.
	 *
	 * SCTLR_EL3.SA: Enable Stack Alignment check. A SP alignment fault
	 *  exception is generated if a load or store instruction executed at
	 *  EL3 uses the SP as the base address and the SP is not aligned to a
	 *  16-byte boundary.
	 *
	 * SCTLR_EL3.A: Enable Alignment fault checking. All instructions that
	 *  load or store one or more registers have an alignment check that the
	 *  address being accessed is aligned to the size of the data element(s)
	 *  being accessed.
	 * ---------------------------------------------------------------------
	 */
	mov	x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
	mrs	x0, sctlr_el3
	orr	x0, x0, x1
	msr	sctlr_el3, x0
	isb

#ifdef IMAGE_BL31
	/* ---------------------------------------------------------------------
	 * Initialise the per-cpu cache pointer to the CPU.
	 * This is done early to enable crash reporting to have access to crash
	 * stack. Since crash reporting depends on cpu_data to report the
	 * unhandled exception, not doing so can lead to recursive exceptions
	 * due to a NULL TPIDR_EL3.
	 * ---------------------------------------------------------------------
	 */
	bl	plat_my_core_pos
	bl	_cpu_data_by_index
	msr	tpidr_el3, x0
#endif /* IMAGE_BL31 */

	/* ---------------------------------------------------------------------
	 * Initialise SCR_EL3, setting all fields rather than relying on hw.
	 * All fields are architecturally UNKNOWN on reset. The following fields
	 * do not change during the TF lifetime. The remaining fields are set to
	 * zero here but are updated ahead of transitioning to a lower EL in the
	 * function cm_init_context_common().
	 *
	 * SCR_EL3.EEL2: Set to one if S-EL2 is present and enabled.
	 *
	 * NOTE: Modifying EEL2 bit along with EA bit ensures that we mitigate
	 * against ERRATA_V2_3099206.
	 * ---------------------------------------------------------------------
	 */
	mov_imm	x0, SCR_RESET_VAL
#if IMAGE_BL31 && defined(SPD_spmd) && SPMD_SPM_AT_SEL2
	mrs	x1, id_aa64pfr0_el1
	and	x1, x1, #(ID_AA64PFR0_SEL2_MASK << ID_AA64PFR0_SEL2_SHIFT)
	cbz	x1, 1f
	orr	x0, x0, #SCR_EEL2_BIT
#endif
1:
	msr	scr_el3, x0

	/* ---------------------------------------------------------------------
	 * Initialise MDCR_EL3, setting all fields rather than relying on hw.
	 * Some fields are architecturally UNKNOWN on reset.
	 */
	mov_imm	x0, MDCR_EL3_RESET_VAL
	msr	mdcr_el3, x0

	/* ---------------------------------------------------------------------
	 * Initialise CPTR_EL3, setting all fields rather than relying on hw.
	 * All fields are architecturally UNKNOWN on reset.
	 * ---------------------------------------------------------------------
	 */
	mov_imm x0, CPTR_EL3_RESET_VAL
	msr	cptr_el3, x0

	.endm

/* -----------------------------------------------------------------------------
 * This is the super set of actions that need to be performed during a cold boot
 * or a warm boot in EL3. This code is shared by BL1 and BL31.
 *
 * This macro will always perform reset handling, architectural initialisations
 * and stack setup. The rest of the actions are optional because they might not
 * be needed, depending on the context in which this macro is called. This is
 * why this macro is parameterised ; each parameter allows to enable/disable
 * some actions.
 *
 *  _init_sctlr:
 *	Whether the macro needs to initialise SCTLR_EL3, including configuring
 *      the endianness of data accesses.
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
 *
 * _pie_fixup_size:
 *	Size of memory region to fixup Global Descriptor Table (GDT).
 *
 *	A non-zero value is expected when firmware needs GDT to be fixed-up.
 *
 * -----------------------------------------------------------------------------
 */
	.macro el3_entrypoint_common					\
		_init_sctlr, _warm_boot_mailbox, _secondary_cold_boot,	\
		_init_memory, _init_c_runtime, _exception_vectors,	\
		_pie_fixup_size

	.if \_init_sctlr
		/* -------------------------------------------------------------
		 * This is the initialisation of SCTLR_EL3 and so must ensure
		 * that all fields are explicitly set rather than relying on hw.
		 * Some fields reset to an IMPLEMENTATION DEFINED value and
		 * others are architecturally UNKNOWN on reset.
		 *
		 * SCTLR.EE: Set the CPU endianness before doing anything that
		 *  might involve memory reads or writes. Set to zero to select
		 *  Little Endian.
		 *
		 * SCTLR_EL3.WXN: For the EL3 translation regime, this field can
		 *  force all memory regions that are writeable to be treated as
		 *  XN (Execute-never). Set to zero so that this control has no
		 *  effect on memory access permissions.
		 *
		 * SCTLR_EL3.SA: Set to zero to disable Stack Alignment check.
		 *
		 * SCTLR_EL3.A: Set to zero to disable Alignment fault checking.
		 *
		 * SCTLR.DSSBS: Set to zero to disable speculation store bypass
		 *  safe behaviour upon exception entry to EL3.
		 * -------------------------------------------------------------
		 */
		mov_imm	x0, (SCTLR_RESET_VAL & ~(SCTLR_EE_BIT | SCTLR_WXN_BIT \
				| SCTLR_SA_BIT | SCTLR_A_BIT | SCTLR_DSSBS_BIT))
#if ENABLE_FEAT_RAS
		/* If FEAT_RAS is present assume FEAT_IESB is also present */
		orr	x0, x0, #SCTLR_IESB_BIT
#endif
		msr	sctlr_el3, x0
		isb
	.endif /* _init_sctlr */

	.if \_warm_boot_mailbox
		/* -------------------------------------------------------------
		 * This code will be executed for both warm and cold resets.
		 * Now is the time to distinguish between the two.
		 * Query the platform entrypoint address and if it is not zero
		 * then it means it is a warm boot so jump to this address.
		 * -------------------------------------------------------------
		 */
		bl	plat_get_my_entrypoint
		cbz	x0, do_cold_boot
		br	x0

	do_cold_boot:
	.endif /* _warm_boot_mailbox */

	.if \_pie_fixup_size
#if ENABLE_PIE
		/*
		 * ------------------------------------------------------------
		 * If PIE is enabled fixup the Global descriptor Table only
		 * once during primary core cold boot path.
		 *
		 * Compile time base address, required for fixup, is calculated
		 * using "pie_fixup" label present within first page.
		 * ------------------------------------------------------------
		 */
	pie_fixup:
		ldr	x0, =pie_fixup
		and	x0, x0, #~(PAGE_SIZE_MASK)
		mov_imm	x1, \_pie_fixup_size
		add	x1, x1, x0
		bl	fixup_gdt_reloc
#endif /* ENABLE_PIE */
	.endif /* _pie_fixup_size */

	/* ---------------------------------------------------------------------
	 * Set the exception vectors.
	 * ---------------------------------------------------------------------
	 */
	adr	x0, \_exception_vectors
	msr	vbar_el3, x0
	isb

	call_reset_handler

	el3_arch_init_common

	/* ---------------------------------------------------------------------
	 * Set the el3 execution context(i.e. root_context).
	 * ---------------------------------------------------------------------
	 */
	setup_el3_execution_context

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
		cbnz	w0, do_primary_cold_boot

		/* This is a cold boot on a secondary CPU */
		bl	plat_secondary_cold_boot_setup
		/* plat_secondary_cold_boot_setup() is not supposed to return */
		bl	el3_panic

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
#if defined(IMAGE_BL31) || (defined(IMAGE_BL2) && \
	((RESET_TO_BL2 && BL2_INV_DCACHE) || ENABLE_RME))
		/* -------------------------------------------------------------
		 * Invalidate the RW memory used by the BL31 image. This
		 * includes the data and NOBITS sections. This is done to
		 * safeguard against possible corruption of this memory by
		 * dirty cache lines in a system cache as a result of use by
		 * an earlier boot loader stage. If PIE is enabled however,
		 * RO sections including the GOT may be modified during
                 * pie fixup. Therefore, to be on the safe side, invalidate
		 * the entire image region if PIE is enabled.
		 * -------------------------------------------------------------
		 */
#if ENABLE_PIE
#if SEPARATE_CODE_AND_RODATA
		adrp	x0, __TEXT_START__
		add	x0, x0, :lo12:__TEXT_START__
#else
		adrp	x0, __RO_START__
		add	x0, x0, :lo12:__RO_START__
#endif /* SEPARATE_CODE_AND_RODATA */
#else
		adrp	x0, __RW_START__
		add	x0, x0, :lo12:__RW_START__
#endif /* ENABLE_PIE */
		adrp	x1, __RW_END__
		add	x1, x1, :lo12:__RW_END__
		sub	x1, x1, x0
		bl	inv_dcache_range
#if defined(IMAGE_BL31) && SEPARATE_NOBITS_REGION
		adrp	x0, __NOBITS_START__
		add	x0, x0, :lo12:__NOBITS_START__
		adrp	x1, __NOBITS_END__
		add	x1, x1, :lo12:__NOBITS_END__
		sub	x1, x1, x0
		bl	inv_dcache_range
#endif
#if defined(IMAGE_BL2) && SEPARATE_BL2_NOLOAD_REGION
		adrp	x0, __BL2_NOLOAD_START__
		add	x0, x0, :lo12:__BL2_NOLOAD_START__
		adrp	x1, __BL2_NOLOAD_END__
		add	x1, x1, :lo12:__BL2_NOLOAD_END__
		sub	x1, x1, x0
		bl	inv_dcache_range
#endif
#endif
		adrp	x0, __BSS_START__
		add	x0, x0, :lo12:__BSS_START__

		adrp	x1, __BSS_END__
		add	x1, x1, :lo12:__BSS_END__
		sub	x1, x1, x0
		bl	zeromem

#if USE_COHERENT_MEM
		adrp	x0, __COHERENT_RAM_START__
		add	x0, x0, :lo12:__COHERENT_RAM_START__
		adrp	x1, __COHERENT_RAM_END_UNALIGNED__
		add	x1, x1, :lo12: __COHERENT_RAM_END_UNALIGNED__
		sub	x1, x1, x0
		bl	zeromem
#endif

#if defined(IMAGE_BL1) ||	\
	(defined(IMAGE_BL2) && RESET_TO_BL2 && BL2_IN_XIP_MEM) || \
	(defined(IMAGE_BL31) && SEPARATE_RWDATA_REGION)

		adrp	x0, __DATA_RAM_START__
		add	x0, x0, :lo12:__DATA_RAM_START__
		adrp	x1, __DATA_ROM_START__
		add	x1, x1, :lo12:__DATA_ROM_START__
		adrp	x2, __DATA_RAM_END__
		add	x2, x2, :lo12:__DATA_RAM_END__
		sub	x2, x2, x0
		bl	memcpy16
#endif
	.endif /* _init_c_runtime */

	/* ---------------------------------------------------------------------
	 * Use SP_EL0 for the C runtime stack.
	 * ---------------------------------------------------------------------
	 */
	msr	spsel, #0

	/* ---------------------------------------------------------------------
	 * Allocate a stack whose memory will be marked as Normal-IS-WBWA when
	 * the MMU is enabled. There is no risk of reading stale stack memory
	 * after enabling the MMU as only the primary CPU is running at the
	 * moment.
	 * ---------------------------------------------------------------------
	 */
	bl	plat_set_my_stack

#if STACK_PROTECTOR_ENABLED
	.if \_init_c_runtime
	bl	update_stack_protector_canary
	.endif /* _init_c_runtime */
#endif
	.endm

	.macro	apply_at_speculative_wa
#if ERRATA_SPECULATIVE_AT
	/*
	 * This function expects x30 has been saved.
	 * Also, save x29 which will be used in the called function.
	 */
	str	x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	bl	save_and_update_ptw_el1_sys_regs
	ldr	x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
#endif
	.endm

	.macro	restore_ptw_el1_sys_regs
#if ERRATA_SPECULATIVE_AT
	/* -----------------------------------------------------------
	 * In case of ERRATA_SPECULATIVE_AT, must follow below order
	 * to ensure that page table walk is not enabled until
	 * restoration of all EL1 system registers. TCR_EL1 register
	 * should be updated at the end which restores previous page
	 * table walk setting of stage1 i.e.(TCR_EL1.EPDx) bits. ISB
	 * ensures that CPU does below steps in order.
	 *
	 * 1. Ensure all other system registers are written before
	 *    updating SCTLR_EL1 using ISB.
	 * 2. Restore SCTLR_EL1 register.
	 * 3. Ensure SCTLR_EL1 written successfully using ISB.
	 * 4. Restore TCR_EL1 register.
	 * -----------------------------------------------------------
	 */
	isb
	ldp	x28, x29, [sp, #CTX_ERRATA_SPEC_AT_OFFSET + CTX_ERRATA_SPEC_AT_SCTLR_EL1]
	msr	sctlr_el1, x28
	isb
	msr	tcr_el1, x29
#endif
	.endm

/* -----------------------------------------------------------------
 * The below macro reads SCR_EL3 from the context structure to
 * determine the security state of the context upon ERET.
 * ------------------------------------------------------------------
 */
	.macro get_security_state _ret:req, _scr_reg:req
		ubfx 	\_ret, \_scr_reg, #SCR_NSE_SHIFT, #1
		cmp 	\_ret, #1
		beq 	realm_state
		bfi	\_ret, \_scr_reg, #0, #1
		b 	end
	realm_state:
		mov 	\_ret, #2
	end:
	.endm

/*-----------------------------------------------------------------------------
 * Helper macro to configure EL3 registers we care about, while executing
 * at EL3/Root world. Root world has its own execution environment and
 * needs to have its settings configured to be independent of other worlds.
 * -----------------------------------------------------------------------------
 */
	.macro setup_el3_execution_context

	/* ---------------------------------------------------------------------
	 * The following registers need to be part of separate root context
	 * as their values are of importance during EL3 execution.
	 * Hence these registers are overwritten to their intital values,
	 * irrespective of whichever world they return from to ensure EL3 has a
	 * consistent execution context throughout the lifetime of TF-A.
	 *
	 * DAIF.A: Enable External Aborts and SError Interrupts at EL3.
	 *
	 * MDCR_EL3.SDD: Set to one to disable AArch64 Secure self-hosted debug.
	 *  Debug exceptions, other than Breakpoint Instruction exceptions, are
	 *  disabled from all ELs in Secure state.
	 *
	 * SCR_EL3.EA: Set to one to enable SError interrupts at EL3.
	 *
	 * SCR_EL3.SIF: Set to one to disable instruction fetches from
	 *  Non-secure memory.
	 *
	 * PMCR_EL0.DP: Set to one so that the cycle counter,
	 *  PMCCNTR_EL0 does not count when event counting is prohibited.
	 *  Necessary on PMUv3 <= p7 where MDCR_EL3.{SCCD,MCCD} are not
	 *  available.
	 *
	 * PSTATE.DIT: Set to one to enable the Data Independent Timing (DIT)
	 *  functionality, if implemented in EL3.
	 * ---------------------------------------------------------------------
	 */
		msr	daifclr, #DAIF_ABT_BIT

		mrs 	x15, mdcr_el3
		orr	x15, x15, #MDCR_SDD_BIT
		msr	mdcr_el3, x15

		mrs	x15, scr_el3
		orr	x15, x15, #SCR_EA_BIT
		orr	x15, x15, #SCR_SIF_BIT
		msr	scr_el3, x15

		mrs 	x15, pmcr_el0
		orr	x15, x15, #PMCR_EL0_DP_BIT
		msr	pmcr_el0, x15

#if ENABLE_FEAT_DIT
#if ENABLE_FEAT_DIT > 1
		mrs	x15, id_aa64pfr0_el1
		ubfx	x15, x15, #ID_AA64PFR0_DIT_SHIFT, #ID_AA64PFR0_DIT_LENGTH
		cbz	x15, 1f
#endif
		mov	x15, #DIT_BIT
		msr	DIT, x15
	1:
#endif

		isb
	.endm

#endif /* EL3_COMMON_MACROS_S */

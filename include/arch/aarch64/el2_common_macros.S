/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL2_COMMON_MACROS_S
#define EL2_COMMON_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#include <platform_def.h>

	/*
	 * Helper macro to initialise system registers at EL2.
	 */
	.macro el2_arch_init_common

	/* ---------------------------------------------------------------------
	 * SCTLR_EL2 has already been initialised - read current value before
	 * modifying.
	 *
	 * SCTLR_EL2.I: Enable the instruction cache.
	 *
	 * SCTLR_EL2.SA: Enable Stack Alignment check. A SP alignment fault
	 *  exception is generated if a load or store instruction executed at
	 *  EL2 uses the SP as the base address and the SP is not aligned to a
	 *  16-byte boundary.
	 *
	 * SCTLR_EL2.A: Enable Alignment fault checking. All instructions that
	 *  load or store one or more registers have an alignment check that the
	 *  address being accessed is aligned to the size of the data element(s)
	 *  being accessed.
	 * ---------------------------------------------------------------------
	 */
	mov	x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
	mrs	x0, sctlr_el2
	orr	x0, x0, x1
	msr	sctlr_el2, x0
	isb

	/* ---------------------------------------------------------------------
	 * Initialise HCR_EL2, setting all fields rather than relying on HW.
	 * All fields are architecturally UNKNOWN on reset. The following fields
	 * do not change during the TF lifetime. The remaining fields are set to
	 * zero here but are updated ahead of transitioning to a lower EL in the
	 * function cm_init_context_common().
	 *
	 * HCR_EL2.TWE: Set to zero so that execution of WFE instructions at
	 *  EL2, EL1 and EL0 are not trapped to EL2.
	 *
	 * HCR_EL2.TWI: Set to zero so that execution of WFI instructions at
	 *  EL2, EL1 and EL0 are not trapped to EL2.
	 *
	 * HCR_EL2.HCD: Set to zero to enable HVC calls at EL1 and above,
	 *  from both Security states and both Execution states.
	 *
	 * HCR_EL2.TEA: Set to one to route External Aborts and SError
	 * Interrupts to EL2 when executing at any EL.
	 *
	 * HCR_EL2.{API,APK}: For Armv8.3 pointer authentication feature,
	 * disable traps to EL2 when accessing key registers or using
	 * pointer authentication instructions from lower ELs.
	 * ---------------------------------------------------------------------
	 */
	mov_imm	x0, ((HCR_RESET_VAL | HCR_TEA_BIT) \
			& ~(HCR_TWE_BIT | HCR_TWI_BIT | HCR_HCD_BIT))
#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * If the pointer authentication registers are saved during world
	 * switches, enable pointer authentication everywhere, as it is safe to
	 * do so.
	 */
	orr	x0, x0, #(HCR_API_BIT | HCR_APK_BIT)
#endif  /* CTX_INCLUDE_PAUTH_REGS */
	msr	hcr_el2, x0

	/* ---------------------------------------------------------------------
	 * Initialise MDCR_EL2, setting all fields rather than relying on
	 * hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * MDCR_EL2.TDOSA: Set to zero so that EL2 and EL2 System register
	 *  access to the powerdown debug registers do not trap to EL2.
	 *
	 * MDCR_EL2.TDA: Set to zero to allow EL0, EL1 and EL2 access to the
	 *  debug registers, other than those registers that are controlled by
	 *  MDCR_EL2.TDOSA.
	 *
	 * MDCR_EL2.TPM: Set to zero so that EL0, EL1, and EL2 System
	 *  register accesses to all Performance Monitors registers do not trap
	 *  to EL2.
	 *
	 * MDCR_EL2.HPMD: Set to zero so that event counting by the program-
	 *  mable counters PMEVCNTR<n>_EL0 is prohibited in Secure state. If
	 *  ARMv8.2 Debug is not implemented this bit does not have any effect
	 *  on the counters unless there is support for the implementation
	 *  defined authentication interface
	 *  ExternalSecureNoninvasiveDebugEnabled().
	 * ---------------------------------------------------------------------
	 */
	mov_imm	x0, ((MDCR_EL2_RESET_VAL | \
		      MDCR_SPD32(MDCR_SPD32_DISABLE)) \
		      & ~(MDCR_EL2_HPMD | MDCR_TDOSA_BIT | \
		      MDCR_TDA_BIT | MDCR_TPM_BIT))

	msr	mdcr_el2, x0

	/* ---------------------------------------------------------------------
	 * Initialise PMCR_EL0 setting all fields rather than relying
	 * on hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * PMCR_EL0.DP: Set to one so that the cycle counter,
	 *  PMCCNTR_EL0 does not count when event counting is prohibited.
	 *
	 * PMCR_EL0.X: Set to zero to disable export of events.
	 *
	 * PMCR_EL0.D: Set to zero so that, when enabled, PMCCNTR_EL0
	 *  counts on every clock cycle.
	 * ---------------------------------------------------------------------
	 */
	mov_imm	x0, ((PMCR_EL0_RESET_VAL | PMCR_EL0_DP_BIT) & \
		    ~(PMCR_EL0_X_BIT | PMCR_EL0_D_BIT))

	msr	pmcr_el0, x0

	/* ---------------------------------------------------------------------
	 * Enable External Aborts and SError Interrupts now that the exception
	 * vectors have been setup.
	 * ---------------------------------------------------------------------
	 */
	msr	daifclr, #DAIF_ABT_BIT

	/* ---------------------------------------------------------------------
	 * Initialise CPTR_EL2, setting all fields rather than relying on hw.
	 * All fields are architecturally UNKNOWN on reset.
	 *
	 * CPTR_EL2.TCPAC: Set to zero so that any accesses to CPACR_EL1 do
	 * not trap to EL2.
	 *
	 * CPTR_EL2.TTA: Set to zero so that System register accesses to the
	 *  trace registers do not trap to EL2.
	 *
	 * CPTR_EL2.TFP: Set to zero so that accesses to the V- or Z- registers
	 *  by Advanced SIMD, floating-point or SVE instructions (if implemented)
	 *  do not trap to EL2.
	 */

	mov_imm x0, (CPTR_EL2_RESET_VAL & ~(TCPAC_BIT | TTA_BIT | TFP_BIT))
	msr	cptr_el2, x0

	/*
	 * If Data Independent Timing (DIT) functionality is implemented,
	 * always enable DIT in EL2
	 */
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #ID_AA64PFR0_DIT_SHIFT, #ID_AA64PFR0_DIT_LENGTH
	cmp	x0, #ID_AA64PFR0_DIT_SUPPORTED
	bne	1f
	mov	x0, #DIT_BIT
	msr	DIT, x0
1:
	.endm

/* -----------------------------------------------------------------------------
 * This is the super set of actions that need to be performed during a cold boot
 * or a warm boot in EL2. This code is shared by BL1 and BL31.
 *
 * This macro will always perform reset handling, architectural initialisations
 * and stack setup. The rest of the actions are optional because they might not
 * be needed, depending on the context in which this macro is called. This is
 * why this macro is parameterised ; each parameter allows to enable/disable
 * some actions.
 *
 *  _init_sctlr:
 *	Whether the macro needs to initialise SCTLR_EL2, including configuring
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
 *	Address of the exception vectors to program in the VBAR_EL2 register.
 *
 * _pie_fixup_size:
 *	Size of memory region to fixup Global Descriptor Table (GDT).
 *
 *	A non-zero value is expected when firmware needs GDT to be fixed-up.
 *
 * -----------------------------------------------------------------------------
 */
	.macro el2_entrypoint_common					\
		_init_sctlr, _warm_boot_mailbox, _secondary_cold_boot,	\
		_init_memory, _init_c_runtime, _exception_vectors,	\
		_pie_fixup_size

	.if \_init_sctlr
		/* -------------------------------------------------------------
		 * This is the initialisation of SCTLR_EL2 and so must ensure
		 * that all fields are explicitly set rather than relying on hw.
		 * Some fields reset to an IMPLEMENTATION DEFINED value and
		 * others are architecturally UNKNOWN on reset.
		 *
		 * SCTLR.EE: Set the CPU endianness before doing anything that
		 *  might involve memory reads or writes. Set to zero to select
		 *  Little Endian.
		 *
		 * SCTLR_EL2.WXN: For the EL2 translation regime, this field can
		 *  force all memory regions that are writeable to be treated as
		 *  XN (Execute-never). Set to zero so that this control has no
		 *  effect on memory access permissions.
		 *
		 * SCTLR_EL2.SA: Set to zero to disable Stack Alignment check.
		 *
		 * SCTLR_EL2.A: Set to zero to disable Alignment fault checking.
		 *
		 * SCTLR.DSSBS: Set to zero to disable speculation store bypass
		 *  safe behaviour upon exception entry to EL2.
		 * -------------------------------------------------------------
		 */
		mov_imm	x0, (SCTLR_RESET_VAL & ~(SCTLR_EE_BIT | SCTLR_WXN_BIT \
				| SCTLR_SA_BIT | SCTLR_A_BIT | SCTLR_DSSBS_BIT))
		msr	sctlr_el2, x0
		isb
	.endif /* _init_sctlr */

#if DISABLE_MTPMU
		bl	mtpmu_disable
#endif

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
	msr	vbar_el2, x0
	isb

	/* ---------------------------------------------------------------------
	 * It is a cold boot.
	 * Perform any processor specific actions upon reset e.g. cache, TLB
	 * invalidations etc.
	 * ---------------------------------------------------------------------
	 */
	bl	reset_handler

	el2_arch_init_common

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
		bl	el2_panic
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
		adrp	x0, __BSS_START__
		add	x0, x0, :lo12:__BSS_START__

		adrp	x1, __BSS_END__
		add	x1, x1, :lo12:__BSS_END__
		sub	x1, x1, x0
		bl	zeromem

#if defined(IMAGE_BL1) || (defined(IMAGE_BL2) && BL2_AT_EL3 && BL2_IN_XIP_MEM)
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
	 * Explicitly save x30 so as to free up a register and to enable
	 * branching and also, save x29 which will be used in the called
	 * function
	 */
	stp	x29, x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	bl	save_and_update_ptw_el1_sys_regs
	ldp	x29, x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
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
	ldp	x28, x29, [sp, #CTX_EL1_SYSREGS_OFFSET + CTX_SCTLR_EL1]
	msr	sctlr_el1, x28
	isb
	msr	tcr_el1, x29
#endif
	.endm

#endif /* EL2_COMMON_MACROS_S */

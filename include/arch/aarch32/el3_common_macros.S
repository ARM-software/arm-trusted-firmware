/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_COMMON_MACROS_S
#define EL3_COMMON_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <lib/xlat_tables/xlat_tables_defs.h>

#define PAGE_START_MASK		~(PAGE_SIZE_MASK)

	/*
	 * Helper macro to initialise EL3 registers we care about.
	 */
	.macro el3_arch_init_common
	/* ---------------------------------------------------------------------
	 * SCTLR has already been initialised - read current value before
	 * modifying.
	 *
	 * SCTLR.I: Enable the instruction cache.
	 *
	 * SCTLR.A: Enable Alignment fault checking. All instructions that load
	 *  or store one or more registers have an alignment check that the
	 *  address being accessed is aligned to the size of the data element(s)
	 *  being accessed.
	 * ---------------------------------------------------------------------
	 */
	ldr	r1, =(SCTLR_I_BIT | SCTLR_A_BIT)
	ldcopr	r0, SCTLR
	orr	r0, r0, r1
	stcopr	r0, SCTLR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise SCR, setting all fields rather than relying on the hw.
	 *
	 * SCR.SIF: Enabled so that Secure state instruction fetches from
	 *  Non-secure memory are not permitted.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =(SCR_RESET_VAL | SCR_SIF_BIT)
	stcopr	r0, SCR

	/* -----------------------------------------------------
	 * Enable the Asynchronous data abort now that the
	 * exception vectors have been setup.
	 * -----------------------------------------------------
	 */
	cpsie   a
	isb

	/* ---------------------------------------------------------------------
	 * Initialise NSACR, setting all the fields, except for the
	 * IMPLEMENTATION DEFINED field, rather than relying on the hw. Some
	 * fields are architecturally UNKNOWN on reset.
	 *
	 * NSACR_ENABLE_FP_ACCESS: Represents NSACR.cp11 and NSACR.cp10. The
	 *  cp11 field is ignored, but is set to same value as cp10. The cp10
	 *  field is set to allow access to Advanced SIMD and floating point
	 *  features from both Security states.
	 *
	 * NSACR.NSTRCDIS: When system register trace implemented, Set to one
	 *  so that NS System register accesses to all implemented trace
	 *  registers are disabled.
	 *  When system register trace is not implemented, this bit is RES0 and
	 *  hence set to zero.
	 * ---------------------------------------------------------------------
	 */
	ldcopr	r0, NSACR
	and	r0, r0, #NSACR_IMP_DEF_MASK
	orr	r0, r0, #(NSACR_RESET_VAL | NSACR_ENABLE_FP_ACCESS)
	ldcopr	r1, ID_DFR0
	ubfx	r1, r1, #ID_DFR0_COPTRC_SHIFT, #ID_DFR0_COPTRC_LENGTH
	cmp	r1, #ID_DFR0_COPTRC_SUPPORTED
	bne	1f
	orr	r0, r0, #NSTRCDIS_BIT
1:
	stcopr	r0, NSACR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise CPACR, setting all fields rather than relying on hw. Some
	 * fields are architecturally UNKNOWN on reset.
	 *
	 * CPACR.TRCDIS: Trap control for PL0 and PL1 System register accesses
	 *  to trace registers. Set to zero to allow access.
	 *
	 * CPACR_ENABLE_FP_ACCESS: Represents CPACR.cp11 and CPACR.cp10. The
	 *  cp11 field is ignored, but is set to same value as cp10. The cp10
	 *  field is set to allow full access from PL0 and PL1 to floating-point
	 *  and Advanced SIMD features.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =((CPACR_RESET_VAL | CPACR_ENABLE_FP_ACCESS) & ~(TRCDIS_BIT))
	stcopr	r0, CPACR
	isb

	/* ---------------------------------------------------------------------
	 * Initialise FPEXC, setting all fields rather than relying on hw. Some
	 * fields are architecturally UNKNOWN on reset and are set to zero
	 * except for field(s) listed below.
	 *
	 * FPEXC.EN: Enable access to Advanced SIMD and floating point features
	 *  from all exception levels.
         *
         * __SOFTFP__: Predefined macro exposed by soft-float toolchain.
         *  ARMv7 and Cortex-A32(ARMv8/aarch32) has both soft-float and
         *  hard-float variants of toolchain, avoid compiling below code with
         *  soft-float toolchain as "vmsr" instruction will not be recognized.
	 * ---------------------------------------------------------------------
	 */
#if ((ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_VFP)) && !(__SOFTFP__)
	ldr	r0, =(FPEXC_RESET_VAL | FPEXC_EN_BIT)
	vmsr	FPEXC, r0
	isb
#endif

#if (ARM_ARCH_MAJOR > 7)
	/* ---------------------------------------------------------------------
	 * Initialise SDCR, setting all the fields rather than relying on hw.
	 *
	 * SDCR.SPD: Disable AArch32 privileged debug. Debug exceptions from
	 *  Secure EL1 are disabled.
	 *
	 * SDCR.SCCD: Set to one so that cycle counting by PMCCNTR is prohibited
	 *  in Secure state. This bit is RES0 in versions of the architecture
	 *  earlier than ARMv8.5, setting it to 1 doesn't have any effect on
	 *  them.
	 *
	 * SDCR.TTRF: Set to one so that access to trace filter control
	 *  registers in non-monitor mode generate Monitor trap exception,
	 *  unless the access generates a higher priority exception when
	 *  trace filter control(FEAT_TRF) is implemented.
	 *  When FEAT_TRF is not implemented, this bit is RES0.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =((SDCR_RESET_VAL | SDCR_SPD(SDCR_SPD_DISABLE) | \
		      SDCR_SCCD_BIT) & ~SDCR_TTRF_BIT)
	ldcopr	r1, ID_DFR0
	ubfx	r1, r1, #ID_DFR0_TRACEFILT_SHIFT, #ID_DFR0_TRACEFILT_LENGTH
	cmp	r1, #ID_DFR0_TRACEFILT_SUPPORTED
	bne	1f
	orr	r0, r0, #SDCR_TTRF_BIT
1:
	stcopr	r0, SDCR

	/* ---------------------------------------------------------------------
	 * Initialise PMCR, setting all fields rather than relying
	 * on hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * PMCR.LP: Set to one so that event counter overflow, that
	 *  is recorded in PMOVSCLR[0-30], occurs on the increment
	 *  that changes PMEVCNTR<n>[63] from 1 to 0, when ARMv8.5-PMU
	 *  is implemented. This bit is RES0 in versions of the architecture
	 *  earlier than ARMv8.5, setting it to 1 doesn't have any effect
	 *  on them.
	 *  This bit is Reserved, UNK/SBZP in ARMv7.
	 *
	 * PMCR.LC: Set to one so that cycle counter overflow, that
	 *  is recorded in PMOVSCLR[31], occurs on the increment
	 *  that changes PMCCNTR[63] from 1 to 0.
	 *  This bit is Reserved, UNK/SBZP in ARMv7.
	 *
	 * PMCR.DP: Set to one to prohibit cycle counting whilst in Secure mode.
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =(PMCR_RESET_VAL | PMCR_DP_BIT | PMCR_LC_BIT | \
		      PMCR_LP_BIT)
#else
	ldr	r0, =(PMCR_RESET_VAL | PMCR_DP_BIT)
#endif
	stcopr	r0, PMCR

	/*
	 * If Data Independent Timing (DIT) functionality is implemented,
	 * always enable DIT in EL3
	 */
	ldcopr	r0, ID_PFR0
	and	r0, r0, #(ID_PFR0_DIT_MASK << ID_PFR0_DIT_SHIFT)
	cmp	r0, #ID_PFR0_DIT_SUPPORTED
	bne	1f
	mrs	r0, cpsr
	orr	r0, r0, #CPSR_DIT_BIT
	msr	cpsr_cxsf, r0
1:
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
 *  _init_sctlr:
 *	Whether the macro needs to initialise the SCTLR register including
 *	configuring the endianness of data accesses.
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

	/* Make sure we are in Secure Mode */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCR
	tst	r0, #SCR_NS_BIT
	ASM_ASSERT(eq)
#endif

	.if \_init_sctlr
		/* -------------------------------------------------------------
		 * This is the initialisation of SCTLR and so must ensure that
		 * all fields are explicitly set rather than relying on hw. Some
		 * fields reset to an IMPLEMENTATION DEFINED value.
		 *
		 * SCTLR.TE: Set to zero so that exceptions to an Exception
		 *  Level executing at PL1 are taken to A32 state.
		 *
		 * SCTLR.EE: Set the CPU endianness before doing anything that
		 *  might involve memory reads or writes. Set to zero to select
		 *  Little Endian.
		 *
		 * SCTLR.V: Set to zero to select the normal exception vectors
		 *  with base address held in VBAR.
		 *
		 * SCTLR.DSSBS: Set to zero to disable speculation store bypass
		 *  safe behaviour upon exception entry to EL3.
		 * -------------------------------------------------------------
		 */
		ldr     r0, =(SCTLR_RESET_VAL & ~(SCTLR_TE_BIT | SCTLR_EE_BIT | \
				SCTLR_V_BIT | SCTLR_DSSBS_BIT))
		stcopr	r0, SCTLR
		isb
	.endif /* _init_sctlr */

	/* Switch to monitor mode */
	cps	#MODE32_mon
	isb

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
		cmp	r0, #0
		bxne	r0
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
		ldr	r0, =pie_fixup
		ldr	r1, =PAGE_START_MASK
		and	r0, r0, r1
		mov_imm	r1, \_pie_fixup_size
		add	r1, r1, r0
		bl	fixup_gdt_reloc
#endif /* ENABLE_PIE */
	.endif /* _pie_fixup_size */

	/* ---------------------------------------------------------------------
	 * Set the exception vectors (VBAR/MVBAR).
	 * ---------------------------------------------------------------------
	 */
	ldr	r0, =\_exception_vectors
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	isb

	/* ---------------------------------------------------------------------
	 * It is a cold boot.
	 * Perform any processor specific actions upon reset e.g. cache, TLB
	 * invalidations etc.
	 * ---------------------------------------------------------------------
	 */
	bl	reset_handler

	el3_arch_init_common

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
		no_ret	plat_panic_handler

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
#if defined(IMAGE_BL32) || (defined(IMAGE_BL2) && BL2_AT_EL3)
		/* -----------------------------------------------------------------
		 * Invalidate the RW memory used by the image. This
		 * includes the data and NOBITS sections. This is done to
		 * safeguard against possible corruption of this memory by
		 * dirty cache lines in a system cache as a result of use by
		 * an earlier boot loader stage. If PIE is enabled however,
		 * RO sections including the GOT may be modified during
		 * pie fixup. Therefore, to be on the safe side, invalidate
		 * the entire image region if PIE is enabled.
		 * -----------------------------------------------------------------
		 */
#if ENABLE_PIE
#if SEPARATE_CODE_AND_RODATA
		ldr	r0, =__TEXT_START__
#else
		ldr	r0, =__RO_START__
#endif /* SEPARATE_CODE_AND_RODATA */
#else
		ldr	r0, =__RW_START__
#endif /* ENABLE_PIE */
		ldr	r1, =__RW_END__
		sub	r1, r1, r0
		bl	inv_dcache_range
#endif

		/*
		 * zeromem uses r12 whereas it is used to save previous BL arg3,
		 * save it in r7
		 */
		mov	r7, r12
		ldr	r0, =__BSS_START__
		ldr	r1, =__BSS_END__
		sub 	r1, r1, r0
		bl	zeromem

#if USE_COHERENT_MEM
		ldr	r0, =__COHERENT_RAM_START__
		ldr	r1, =__COHERENT_RAM_END_UNALIGNED__
		sub 	r1, r1, r0
		bl	zeromem
#endif

		/* Restore r12 */
		mov	r12, r7

#if defined(IMAGE_BL1) || (defined(IMAGE_BL2) && BL2_AT_EL3 && BL2_IN_XIP_MEM)
		/* -----------------------------------------------------
		 * Copy data from ROM to RAM.
		 * -----------------------------------------------------
		 */
		ldr	r0, =__DATA_RAM_START__
		ldr	r1, =__DATA_ROM_START__
		ldr	r2, =__DATA_RAM_END__
		sub 	r2, r2, r0
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

#if STACK_PROTECTOR_ENABLED
	.if \_init_c_runtime
	bl	update_stack_protector_canary
	.endif /* _init_c_runtime */
#endif
	.endm

#endif /* EL3_COMMON_MACROS_S */

/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CPU_MACROS_S
#define CPU_MACROS_S

#include <assert_macros.S>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>

	/*
	 * Write given expressions as quad words
	 *
	 * _count:
	 *	Write at least _count quad words. If the given number of
	 *	expressions is less than _count, repeat the last expression to
	 *	fill _count quad words in total
	 * _rest:
	 *	Optional list of expressions. _this is for parameter extraction
	 *	only, and has no significance to the caller
	 *
	 * Invoked as:
	 *	fill_constants 2, foo, bar, blah, ...
	 */
	.macro fill_constants _count:req, _this, _rest:vararg
	  .ifgt \_count
	    /* Write the current expression */
	    .ifb \_this
	      .error "Nothing to fill"
	    .endif
	    .quad \_this

	    /* Invoke recursively for remaining expressions */
	    .ifnb \_rest
	      fill_constants \_count-1, \_rest
	    .else
	      fill_constants \_count-1, \_this
	    .endif
	  .endif
	.endm

	/*
	 * Declare CPU operations
	 *
	 * _name:
	 *	Name of the CPU for which operations are being specified
	 * _midr:
	 *	Numeric value expected to read from CPU's MIDR
	 * _resetfunc:
	 *	Reset function for the CPU.
	 * _e_handler:
	 *	This is a placeholder for future per CPU exception handlers.
	 * _power_down_ops:
	 *	Comma-separated list of functions to perform power-down
	 *	operatios on the CPU. At least one, and up to
	 *	CPU_MAX_PWR_DWN_OPS number of functions may be specified.
	 *	Starting at power level 0, these functions shall handle power
	 *	down at subsequent power levels. If there aren't exactly
	 *	CPU_MAX_PWR_DWN_OPS functions, the last specified one will be
	 *	used to handle power down at subsequent levels
	 */
	.macro declare_cpu_ops_base _name:req, _midr:req, _resetfunc:req, \
		_e_handler:req, _power_down_ops:vararg
	.section .cpu_ops, "a"
	.align 3
	.type cpu_ops_\_name, %object
	.quad \_midr
#if defined(IMAGE_AT_EL3)
	.quad \_resetfunc
#endif
	.quad \_e_handler
#ifdef IMAGE_BL31
	/* Insert list of functions */
	fill_constants CPU_MAX_PWR_DWN_OPS, \_power_down_ops
#endif
	/*
	 * It is possible (although unlikely) that a cpu may have no errata in
	 * code. In that case the start label will not be defined. The list is
	 * intended to be used in a loop, so define it as zero-length for
	 * predictable behaviour. Since this macro is always called at the end
	 * of the cpu file (after all errata have been parsed) we can be sure
	 * that we are at the end of the list. Some cpus call declare_cpu_ops
	 * twice, so only do this once.
	 */
	.pushsection .rodata.errata_entries
	.ifndef \_name\()_errata_list_start
		\_name\()_errata_list_start:
	.endif
	.ifndef \_name\()_errata_list_end
		\_name\()_errata_list_end:
	.endif
	.popsection

	/* and now put them in cpu_ops */
	.quad \_name\()_errata_list_start
	.quad \_name\()_errata_list_end

#if REPORT_ERRATA
	.ifndef \_name\()_cpu_str
	  /*
	   * Place errata reported flag, and the spinlock to arbitrate access to
	   * it in the data section.
	   */
	  .pushsection .data
	  define_asm_spinlock \_name\()_errata_lock
	  \_name\()_errata_reported:
	  .word	0
	  .popsection

	  /* Place CPU string in rodata */
	  .pushsection .rodata
	  \_name\()_cpu_str:
	  .asciz "\_name"
	  .popsection
	.endif

	.quad \_name\()_cpu_str

#ifdef IMAGE_BL31
	/* Pointers to errata lock and reported flag */
	.quad \_name\()_errata_lock
	.quad \_name\()_errata_reported
#endif /* IMAGE_BL31 */
#endif /* REPORT_ERRATA */

#if defined(IMAGE_BL31) && CRASH_REPORTING
	.quad \_name\()_cpu_reg_dump
#endif
	.endm

	.macro declare_cpu_ops _name:req, _midr:req, _resetfunc:req, \
		_power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, 0, \_power_down_ops
	.endm

	.macro declare_cpu_ops_eh _name:req, _midr:req, _resetfunc:req, \
		_e_handler:req, _power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			\_e_handler, \_power_down_ops
	.endm

	/*
	 * This macro is used on some CPUs to detect if they are vulnerable
	 * to CVE-2017-5715.
	 */
	.macro	cpu_check_csv2 _reg _label
	mrs	\_reg, id_aa64pfr0_el1
	ubfx	\_reg, \_reg, #ID_AA64PFR0_CSV2_SHIFT, #ID_AA64PFR0_CSV2_LENGTH
	/*
	 * If the field equals 1, branch targets trained in one context cannot
	 * affect speculative execution in a different context.
	 *
	 * If the field equals 2 or 3, it means that the system is also aware of
	 * SCXTNUM_ELx register contexts. We aren't using them in the TF, so we
	 * expect users of the registers to do the right thing.
	 *
	 * Only apply mitigations if the value of this field is 0.
	 */
#if ENABLE_ASSERTIONS
	cmp	\_reg, #4 /* Only values 0 to 3 are expected */
	ASM_ASSERT(lo)
#endif

	cmp	\_reg, #0
	bne	\_label
	.endm

	/*
	 * Helper macro that reads the part number of the current
	 * CPU and jumps to the given label if it matches the CPU
	 * MIDR provided.
	 *
	 * Clobbers x0.
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp	w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label
	.endm


/*
 * Workaround wrappers for errata that apply at reset or runtime. Reset errata
 * will be applied automatically
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * _cve:
 *	Whether erratum is a CVE. CVE year if yes, 0 otherwise
 *
 * _id:
 *	Erratum or CVE number. Please combine with previous field with ERRATUM
 *	or CVE macros
 *
 * _chosen:
 *	Compile time flag on whether the erratum is included
 *
 * _split_wa:
 *	Flag that indicates whether an erratum has split workaround or not.
 *	Default value is 0.
 */
.macro add_erratum_entry _cpu:req, _cve:req, _id:req, _chosen:req, _split_wa=0
#if INCLUDE_ERRATA_LIST
	.pushsection .rodata.errata_entries
		.align	3
		.ifndef \_cpu\()_errata_list_start
		\_cpu\()_errata_list_start:
		.endif

		.quad	check_erratum_\_cpu\()_\_id
		/* Will fit CVEs with up to 10 character in the ID field */
		.word	\_id
		.hword	\_cve
		/* bit magic that appends chosen field based on _split_wa */
		.byte	((\_chosen * 0b11) & ((\_split_wa << 1) | \_chosen))
		.byte	0x0 /* alignment */
	.popsection
#endif
.endm

/*******************************************************************************
 * Errata workaround wrappers
 ******************************************************************************/
/*
 * Workaround wrappers for errata that apply at reset or runtime. Reset errata
 * will be applied automatically
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * _cve:
 *	Whether erratum is a CVE. CVE year if yes, 0 otherwise
 *
 * _id:
 *	Erratum or CVE number. Please combine with previous field with ERRATUM
 *	or CVE macros
 *
 * _chosen:
 *	Compile time flag on whether the erratum is included
 *
 * _split_wa:
 *	Flag that indicates whether an erratum has split workaround or not.
 *	Default value is 0.
 *
 * in body:
 *	clobber x0 to x7 (please only use those)
 *	argument x7 - cpu_rev_var
 *
 * _wa clobbers: x0-x8 (PCS compliant)
 */
.macro workaround_reset_start _cpu:req, _cve:req, _id:req, \
	_chosen:req, _split_wa=0

	add_erratum_entry \_cpu, \_cve, \_id, \_chosen, \_split_wa

	.if \_chosen
		/* put errata directly into the reset function */
		.pushsection .text.asm.\_cpu\()_reset_func, "ax"
	.else
		/* or something else that will get garbage collected by the
		 * linker */
		.pushsection .text.asm.erratum_\_cpu\()_\_id\()_wa, "ax"
	.endif
		/* revision is stored in x14, get it */
		mov	x0, x14
		bl	check_erratum_\_cpu\()_\_id
		/* save rev_var for workarounds that might need it */
		mov	x7, x14
		cbz	x0, erratum_\_cpu\()_\_id\()_skip_reset
.endm

/*
 * See `workaround_reset_start` for usage info. Additional arguments:
 *
 * _midr:
 *	Check if CPU's MIDR matches the CPU it's meant for. Must be specified
 *	for errata applied in generic code
 */
.macro workaround_runtime_start _cpu:req, _cve:req, _id:req, _chosen:req, _midr
	add_erratum_entry \_cpu, \_cve, \_id, \_chosen

	func erratum_\_cpu\()_\_id\()_wa
		mov	x8, x30
	/*
	 * Let errata specify if they need MIDR checking. Sadly, storing the
	 * MIDR in an .equ to retrieve automatically blows up as it stores some
	 * brackets in the symbol
	 */
	.ifnb \_midr
		jump_if_cpu_midr \_midr, 1f
		b	erratum_\_cpu\()_\_id\()_skip_runtime

		1:
	.endif
		/* save rev_var for workarounds that might need it but don't
		 * restore to x0 because few will care */
		mov	x7, x0
		bl	check_erratum_\_cpu\()_\_id
		cbz	x0, erratum_\_cpu\()_\_id\()_skip_runtime
.endm

/*
 * Usage and arguments identical to `workaround_reset_start`. The _cve argument
 * is kept here so the same #define can be used as that macro
 */
.macro workaround_reset_end _cpu:req, _cve:req, _id:req
	erratum_\_cpu\()_\_id\()_skip_reset:
	.popsection
.endm

/*
 * See `workaround_reset_start` for usage info. The _cve argument is kept here
 * so the same #define can be used as that macro. Additional arguments:
 *
 * _no_isb:
 *	Optionally do not include the trailing isb. Please disable with the
 *	NO_ISB macro
 */
.macro workaround_runtime_end _cpu:req, _cve:req, _id:req, _no_isb
	/*
	 * Runtime errata do not have a reset function to call the isb for them
	 * and missing the isb could be very problematic. It is also likely as
	 * they tend to be scattered in generic code.
	 */
	.ifb \_no_isb
		isb
	.endif
	erratum_\_cpu\()_\_id\()_skip_runtime:
		ret	x8
	endfunc erratum_\_cpu\()_\_id\()_wa
.endm

/*******************************************************************************
 * Errata workaround helpers
 ******************************************************************************/
/*
 * Set a bit in a system register. Can set multiple bits but is limited by the
 *  way the ORR instruction encodes them.
 *
 * _reg:
 *	Register to write to
 *
 * _bit:
 *	Bit to set. Please use a descriptive #define
 *
 * _assert:
 *	Optionally whether to read back and assert that the bit has been
 *	written. Please disable with NO_ASSERT macro
 *
 * clobbers: x1
 */
.macro sysreg_bit_set _reg:req, _bit:req, _assert=1
	mrs	x1, \_reg
	orr	x1, x1, #\_bit
	msr	\_reg, x1
.endm

/*
 * Clear a bit in a system register. Can clear multiple bits but is limited by
 *  the way the BIC instrucion encodes them.
 *
 * see sysreg_bit_set for usage
 */
.macro sysreg_bit_clear _reg:req, _bit:req
	mrs	x1, \_reg
	bic	x1, x1, #\_bit
	msr	\_reg, x1
.endm

/*
 * Toggle a bit in a system register. Can toggle multiple bits but is limited by
 *  the way the EOR instrucion encodes them.
 *
 * see sysreg_bit_set for usage
 */
.macro sysreg_bit_toggle _reg:req, _bit:req, _assert=1
	mrs	x1, \_reg
	eor	x1, x1, #\_bit
	msr	\_reg, x1
.endm

.macro override_vector_table _table:req
	adr	x1, \_table
	msr	vbar_el3, x1
.endm

/*
 * BFI : Inserts bitfield into a system register.
 *
 * BFI{cond} Rd, Rn, #lsb, #width
 */
.macro sysreg_bitfield_insert _reg:req, _src:req, _lsb:req, _width:req
	/* Source value for BFI */
	mov	x1, #\_src
	mrs	x0, \_reg
	bfi	x0, x1, #\_lsb, #\_width
	msr	\_reg, x0
.endm

.macro sysreg_bitfield_insert_from_gpr _reg:req, _gpr:req, _lsb:req, _width:req
	/* Source value in register for BFI */
	mov	x1, \_gpr
	mrs	x0, \_reg
	bfi	x0, x1, #\_lsb, #\_width
	msr	\_reg, x0
.endm

/*
 * Extract CPU revision and variant, and combine them into a single numeric for
 * easier comparison.
 *
 * _res:
 *	register where the result will be placed
 * _tmp:
 *	register to clobber for temporaries
 */
.macro get_rev_var _res:req, _tmp:req
	mrs	\_tmp, midr_el1

	/*
	 * Extract the variant[23:20] and revision[3:0] from MIDR, and pack them
	 * as variant[7:4] and revision[3:0] of x0.
	 *
	 * First extract x1[23:16] to x0[7:0] and zero fill the rest. Then
	 * extract x1[3:0] into x0[3:0] retaining other bits.
	 */
	ubfx	\_res, \_tmp, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), #(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfxil	\_res, \_tmp, #MIDR_REV_SHIFT, #MIDR_REV_BITS
.endm

/*
 * Apply erratum
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * _cve:
 *	Whether erratum is a CVE. CVE year if yes, 0 otherwise
 *
 * _id:
 *	Erratum or CVE number. Please combine with previous field with ERRATUM
 *	or CVE macros
 *
 * _chosen:
 *	Compile time flag on whether the erratum is included
 *
 * _get_rev:
 *	Optional parameter that determines whether to insert a call to the CPU revision fetching
 *	procedure. Stores the result of this in the temporary register x10 to allow for chaining
 *
 * clobbers: x0-x10 (PCS compliant)
 */
.macro apply_erratum _cpu:req, _cve:req, _id:req, _chosen:req, _get_rev=GET_CPU_REV
	.if (\_chosen && \_get_rev)
		mov	x9, x30
		bl	cpu_get_rev_var
		mov	x10, x0
	.elseif (\_chosen)
		mov	x9, x30
		mov	x0, x10
	.endif

	.if \_chosen
		bl	erratum_\_cpu\()_\_id\()_wa
		mov	x30, x9
	.endif
.endm

/*
 * Helpers to report if an erratum applies. Compares the given revision variant
 * to the given value. Return ERRATA_APPLIES or ERRATA_NOT_APPLIES accordingly.
 *
 * _rev_num: the given revision variant. Or
 * _rev_num_lo,_rev_num_hi: the lower and upper bounds of the revision variant
 *
 * in body:
 *	clobber: x0
 *	argument: x0 - cpu_rev_var
 */
.macro cpu_rev_var_ls _rev_num:req
	cmp	x0, #\_rev_num
	cset	x0, ls
.endm

.macro cpu_rev_var_hs _rev_num:req
	cmp	x0, #\_rev_num
	cset	x0, hs
.endm

.macro cpu_rev_var_range _rev_num_lo:req, _rev_num_hi:req
	cmp	x0, #\_rev_num_lo
	mov	x1, #\_rev_num_hi
	ccmp	x0, x1, #2, hs
	cset	x0, ls
.endm


#if __clang_major__ < 17
/*
 * A problem with clang version < 17 can cause resolving nested
 * 'cfi_startproc' to fail compilation.
 * So add a compatibility variant for start and endfunc expansions
 * to ignore `cfi_startproc` and `cfi_endproc`, this to be used only with
 * check_errata/reset macros if we build TF-A with clang version < 17
 */

.macro func_compat _name, _align=2
	.section .text.asm.\_name, "ax"
	.type \_name, %function
	.align \_align
	\_name:
#if ENABLE_BTI
	bti	jc
#endif
.endm

/*
 * This macro is used to mark the end of a function.
 */
.macro endfunc_compat _name
	.size \_name, . - \_name
.endm

#else

#define func_compat func
#define endfunc_compat endfunc

#endif /* __clang_version__ < 17 */

/*
 * Helpers to select which revisions errata apply to.
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * _cve:
 *	Whether erratum is a CVE. CVE year if yes, 0 otherwise
 *
 * _id:
 *	Erratum or CVE number. Please combine with previous field with ERRATUM
 *	or CVE macros
 *
 * _rev_num:
 *	Revision to apply to
 *
 * in body:
 *	clobber: x0 to x1
 *	argument: x0 - cpu_rev_var
 */
.macro check_erratum_ls _cpu:req, _cve:req, _id:req, _rev_num:req
	func_compat check_erratum_\_cpu\()_\_id
		cpu_rev_var_ls \_rev_num
		ret
	endfunc_compat check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_hs _cpu:req, _cve:req, _id:req, _rev_num:req
	func_compat check_erratum_\_cpu\()_\_id
		cpu_rev_var_hs \_rev_num
		ret
	endfunc_compat check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_range _cpu:req, _cve:req, _id:req, _rev_num_lo:req, _rev_num_hi:req
	func_compat check_erratum_\_cpu\()_\_id
		cpu_rev_var_range \_rev_num_lo, \_rev_num_hi
		ret
	endfunc_compat check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_chosen _cpu:req, _cve:req, _id:req, _chosen:req
	func_compat check_erratum_\_cpu\()_\_id
		.if \_chosen
			mov	x0, #ERRATA_APPLIES
		.else
			mov	x0, #ERRATA_MISSING
		.endif
		ret
	endfunc_compat check_erratum_\_cpu\()_\_id
.endm

/*
 * provide a shorthand for the name format for annoying errata
 * body: clobber x0 to x4
 */
.macro check_erratum_custom_start _cpu:req, _cve:req, _id:req
	func_compat check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_custom_end _cpu:req, _cve:req, _id:req
	endfunc_compat check_erratum_\_cpu\()_\_id
.endm

/*******************************************************************************
 * CPU reset function wrapper
 ******************************************************************************/

/*
 * Helper to register a cpu with the errata framework. Begins the definition of
 * the reset function.
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 */
.macro cpu_reset_prologue _cpu:req
	func_compat \_cpu\()_reset_func
		mov	x15, x30
		get_rev_var x14, x0
.endm

/*
 * Wrapper of the reset function to automatically apply all reset-time errata.
 * Will end with an isb.
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * in body:
 *	clobber x8 to x14
 *	argument x14 - cpu_rev_var
 */
.macro cpu_reset_func_start _cpu:req
	/* the func/endfunc macros will change sections. So change the section
	 * back to the reset function's */
	.section .text.asm.\_cpu\()_reset_func, "ax"
.endm

.macro cpu_reset_func_end _cpu:req
		isb
		ret	x15
	endfunc_compat \_cpu\()_reset_func
.endm

#endif /* CPU_MACROS_S */

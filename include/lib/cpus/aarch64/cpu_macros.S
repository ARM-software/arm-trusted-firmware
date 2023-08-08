/*
 * Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.
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
	 *	Reset function for the CPU. If there's no CPU reset function,
	 *	specify CPU_NO_RESET_FUNC
	 * _extra1:
	 *	This is a placeholder for future per CPU operations.  Currently,
	 *	some CPUs use this entry to set a test function to determine if
	 *	the workaround for CVE-2017-5715 needs to be applied or not.
	 * _extra2:
	 *	This is a placeholder for future per CPU operations. Currently
	 *	some CPUs use this entry to set a function to disable the
	 *	workaround for CVE-2018-3639.
	 * _extra3:
	 *	This is a placeholder for future per CPU operations. Currently,
	 *	some CPUs use this entry to set a test function to determine if
	 *	the workaround for CVE-2022-23960 needs to be applied or not.
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
		_extra1:req, _extra2:req, _extra3:req, _e_handler:req, _power_down_ops:vararg
	.section .cpu_ops, "a"
	.align 3
	.type cpu_ops_\_name, %object
	.quad \_midr
#if defined(IMAGE_AT_EL3)
	.quad \_resetfunc
#endif
	.quad \_extra1
	.quad \_extra2
	.quad \_extra3
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


	/*
	 * Mandatory errata status printing function for CPUs of
	 * this class.
	 */
	.quad \_name\()_errata_report
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
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, 0, 0, 0, 0, \
			\_power_down_ops
	.endm

	.macro declare_cpu_ops_eh _name:req, _midr:req, _resetfunc:req, \
		_e_handler:req, _power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			0, 0, 0, \_e_handler, \_power_down_ops
	.endm

	.macro declare_cpu_ops_wa _name:req, _midr:req, \
		_resetfunc:req, _extra1:req, _extra2:req, \
		_extra3:req, _power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			\_extra1, \_extra2, \_extra3, 0, \_power_down_ops
	.endm

/* TODO can be deleted once all CPUs have been converted */
#if REPORT_ERRATA
	/*
	 * Print status of a CPU errata
	 *
	 * _chosen:
	 *	Identifier indicating whether or not a CPU errata has been
	 *	compiled in.
	 * _cpu:
	 *	Name of the CPU
	 * _id:
	 *	Errata identifier
	 * _rev_var:
	 *	Register containing the combined value CPU revision and variant
	 *	- typically the return value of cpu_get_rev_var
	 */
	.macro report_errata _chosen, _cpu, _id, _rev_var=x8
	/* Stash a string with errata ID */
	.pushsection .rodata
	\_cpu\()_errata_\_id\()_str:
	.asciz	"\_id"
	.popsection

	/* Check whether errata applies */
	mov	x0, \_rev_var
	/* Shall clobber: x0-x7 */
	bl	check_errata_\_id

	.ifeq \_chosen
	/*
	 * Errata workaround has not been compiled in. If the errata would have
	 * applied had it been compiled in, print its status as missing.
	 */
	cbz	x0, 900f
	mov	x0, #ERRATA_MISSING
	.endif
900:
	adr	x1, \_cpu\()_cpu_str
	adr	x2, \_cpu\()_errata_\_id\()_str
	bl	errata_print_msg
	.endm
#endif

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
	 * If the field equals 2, it means that the system is also aware of
	 * SCXTNUM_ELx register contexts. We aren't using them in the TF, so we
	 * expect users of the registers to do the right thing.
	 *
	 * Only apply mitigations if the value of this field is 0.
	 */
#if ENABLE_ASSERTIONS
	cmp	\_reg, #3 /* Only values 0 to 2 are expected */
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
 * _apply_at_reset:
 *	Whether the erratum should be automatically applied at reset
 */
.macro add_erratum_entry _cpu:req, _cve:req, _id:req, _chosen:req, _apply_at_reset:req
	.pushsection .rodata.errata_entries
		.align	3
		.ifndef \_cpu\()_errata_list_start
		\_cpu\()_errata_list_start:
		.endif

		/* check if unused and compile out if no references */
		.if \_apply_at_reset && \_chosen
			.quad	erratum_\_cpu\()_\_id\()_wa
		.else
			.quad	0
		.endif
		/* TODO(errata ABI): this prevents all checker functions from
		 * being optimised away. Can be done away with unless the ABI
		 * needs them */
		.quad	check_erratum_\_cpu\()_\_id
		/* Will fit CVEs with up to 10 character in the ID field */
		.word	\_id
		.hword	\_cve
		.byte	\_chosen
		/* TODO(errata ABI): mitigated field for known but unmitigated
		 * errata */
		.byte	0x1
	.popsection
.endm

.macro _workaround_start _cpu:req, _cve:req, _id:req, _chosen:req, _apply_at_reset:req
	add_erratum_entry \_cpu, \_cve, \_id, \_chosen, \_apply_at_reset

	func erratum_\_cpu\()_\_id\()_wa
		mov	x8, x30

		/* save rev_var for workarounds that might need it but don't
		 * restore to x0 because few will care */
		mov	x7, x0
		bl	check_erratum_\_cpu\()_\_id
		cbz	x0, erratum_\_cpu\()_\_id\()_skip
.endm

.macro _workaround_end _cpu:req, _id:req
	erratum_\_cpu\()_\_id\()_skip:
		ret	x8
	endfunc erratum_\_cpu\()_\_id\()_wa
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
 * in body:
 *	clobber x0 to x7 (please only use those)
 *	argument x7 - cpu_rev_var
 *
 * _wa clobbers: x0-x8 (PCS compliant)
 */
.macro workaround_reset_start _cpu:req, _cve:req, _id:req, _chosen:req
	_workaround_start \_cpu, \_cve, \_id, \_chosen, 1
.endm

/*
 * See `workaround_reset_start` for usage info. Additional arguments:
 *
 * _midr:
 *	Check if CPU's MIDR matches the CPU it's meant for. Must be specified
 *	for errata applied in generic code
 */
.macro workaround_runtime_start _cpu:req, _cve:req, _id:req, _chosen:req, _midr
	/*
	 * Let errata specify if they need MIDR checking. Sadly, storing the
	 * MIDR in an .equ to retrieve automatically blows up as it stores some
	 * brackets in the symbol
	 */
	.ifnb \_midr
		jump_if_cpu_midr \_midr, 1f
		b	erratum_\_cpu\()_\_id\()_skip

		1:
	.endif
	_workaround_start \_cpu, \_cve, \_id, \_chosen, 0
.endm

/*
 * Usage and arguments identical to `workaround_reset_start`. The _cve argument
 * is kept here so the same #define can be used as that macro
 */
.macro workaround_reset_end _cpu:req, _cve:req, _id:req
	_workaround_end \_cpu, \_id
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
	_workaround_end \_cpu, \_id
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
 *	procedure. Stores the result of this in the temporary register x10.
 *
 * clobbers: x0-x10 (PCS compliant)
 */
.macro apply_erratum _cpu:req, _cve:req, _id:req, _chosen:req, _get_rev=GET_CPU_REV
	.if (\_chosen & \_get_rev)
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
 * Helpers to select which revisions errata apply to. Don't leave a link
 * register as the cpu_rev_var_*** will call the ret and we can save on one.
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
 *	clobber: x0 to x4
 *	argument: x0 - cpu_rev_var
 */
.macro check_erratum_ls _cpu:req, _cve:req, _id:req, _rev_num:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num
		b	cpu_rev_var_ls
	endfunc check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_hs _cpu:req, _cve:req, _id:req, _rev_num:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num
		b	cpu_rev_var_hs
	endfunc check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_range _cpu:req, _cve:req, _id:req, _rev_num_lo:req, _rev_num_hi:req
	func check_erratum_\_cpu\()_\_id
		mov	x1, #\_rev_num_lo
		mov	x2, #\_rev_num_hi
		b	cpu_rev_var_range
	endfunc check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_chosen _cpu:req, _cve:req, _id:req, _chosen:req
	func check_erratum_\_cpu\()_\_id
		.if \_chosen
			mov	x0, #ERRATA_APPLIES
		.else
			mov	x0, #ERRATA_MISSING
		.endif
		ret
	endfunc check_erratum_\_cpu\()_\_id
.endm

/* provide a shorthand for the name format for annoying errata */
.macro check_erratum_custom_start _cpu:req, _cve:req, _id:req
	func check_erratum_\_cpu\()_\_id
.endm

.macro check_erratum_custom_end _cpu:req, _cve:req, _id:req
	endfunc check_erratum_\_cpu\()_\_id
.endm


/*******************************************************************************
 * CPU reset function wrapper
 ******************************************************************************/

/*
 * Wrapper to automatically apply all reset-time errata. Will end with an isb.
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * in body:
 *	clobber x8 to x14
 *	argument x14 - cpu_rev_var
 */
.macro cpu_reset_func_start _cpu:req
	func \_cpu\()_reset_func
		mov	x15, x30
		bl	cpu_get_rev_var
		mov	x14, x0

		/* short circuit the location to avoid searching the list */
		adrp	x12, \_cpu\()_errata_list_start
		add	x12, x12, :lo12:\_cpu\()_errata_list_start
		adrp	x13, \_cpu\()_errata_list_end
		add	x13, x13, :lo12:\_cpu\()_errata_list_end

	errata_begin:
		/* if head catches up with end of list, exit */
		cmp	x12, x13
		b.eq	errata_end

		ldr	x10, [x12, #ERRATUM_WA_FUNC]
		/* TODO(errata ABI): check mitigated and checker function fields
		 * for 0 */
		ldrb	w11, [x12, #ERRATUM_CHOSEN]

		/* skip if not chosen */
		cbz	x11, 1f
		/* skip if runtime erratum */
		cbz	x10, 1f

		/* put cpu revision in x0 and call workaround */
		mov	x0, x14
		blr	x10
	1:
		add	x12, x12, #ERRATUM_ENTRY_SIZE
		b	errata_begin
	errata_end:
.endm

.macro cpu_reset_func_end _cpu:req
		isb
		ret	x15
	endfunc \_cpu\()_reset_func
.endm

/*
 * Maintain compatibility with the old scheme of each cpu has its own reporting.
 * TODO remove entirely once all cpus have been converted. This includes the
 * cpu_ops entry, as print_errata_status can call this directly for all cpus
 */
.macro errata_report_shim _cpu:req
	#if REPORT_ERRATA
	func \_cpu\()_errata_report
		/* normal stack frame for pretty debugging */
		stp	x29, x30, [sp, #-16]!
		mov	x29, sp

		bl	generic_errata_report

		ldp	x29, x30, [sp], #16
		ret
	endfunc \_cpu\()_errata_report
	#endif
.endm
#endif /* CPU_MACROS_S */

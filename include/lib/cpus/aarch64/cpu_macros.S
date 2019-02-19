/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CPU_MACROS_S
#define CPU_MACROS_S

#include <arch.h>
#include <assert_macros.S>
#include <lib/cpus/errata_report.h>

#define CPU_IMPL_PN_MASK	(MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) | \
				(MIDR_PN_MASK << MIDR_PN_SHIFT)

/* The number of CPU operations allowed */
#define CPU_MAX_PWR_DWN_OPS		2

/* Special constant to specify that CPU has no reset function */
#define CPU_NO_RESET_FUNC		0

#define CPU_NO_EXTRA1_FUNC		0
#define CPU_NO_EXTRA2_FUNC		0

/* Word size for 64-bit CPUs */
#define CPU_WORD_SIZE			8

#if defined(IMAGE_BL1) || defined(IMAGE_BL31) ||(defined(IMAGE_BL2) && BL2_AT_EL3)
#define IMAGE_AT_EL3
#endif

/*
 * Whether errata status needs reporting. Errata status is printed in debug
 * builds for both BL1 and BL31 images.
 */
#if (defined(IMAGE_BL1) || defined(IMAGE_BL31)) && DEBUG
# define REPORT_ERRATA	1
#else
# define REPORT_ERRATA	0
#endif


	.equ	CPU_MIDR_SIZE, CPU_WORD_SIZE
	.equ	CPU_EXTRA1_FUNC_SIZE, CPU_WORD_SIZE
	.equ	CPU_EXTRA2_FUNC_SIZE, CPU_WORD_SIZE
	.equ	CPU_RESET_FUNC_SIZE, CPU_WORD_SIZE
	.equ	CPU_PWR_DWN_OPS_SIZE, CPU_WORD_SIZE * CPU_MAX_PWR_DWN_OPS
	.equ	CPU_ERRATA_FUNC_SIZE, CPU_WORD_SIZE
	.equ	CPU_ERRATA_LOCK_SIZE, CPU_WORD_SIZE
	.equ	CPU_ERRATA_PRINTED_SIZE, CPU_WORD_SIZE
	.equ	CPU_REG_DUMP_SIZE, CPU_WORD_SIZE

#ifndef IMAGE_AT_EL3
	.equ	CPU_RESET_FUNC_SIZE, 0
#endif

/* The power down core and cluster is needed only in BL31 */
#ifndef IMAGE_BL31
	.equ	CPU_PWR_DWN_OPS_SIZE, 0
#endif

/* Fields required to print errata status. */
#if !REPORT_ERRATA
	.equ	CPU_ERRATA_FUNC_SIZE, 0
#endif

/* Only BL31 requieres mutual exclusion and printed flag.  */
#if !(REPORT_ERRATA && defined(IMAGE_BL31))
	.equ	CPU_ERRATA_LOCK_SIZE, 0
	.equ	CPU_ERRATA_PRINTED_SIZE, 0
#endif

#if !defined(IMAGE_BL31) || !CRASH_REPORTING
	.equ	CPU_REG_DUMP_SIZE, 0
#endif

/*
 * Define the offsets to the fields in cpu_ops structure.
 * Every offset is defined based in the offset and size of the previous
 * field.
 */
	.equ	CPU_MIDR, 0
	.equ	CPU_RESET_FUNC, CPU_MIDR + CPU_MIDR_SIZE
	.equ	CPU_EXTRA1_FUNC, CPU_RESET_FUNC + CPU_RESET_FUNC_SIZE
	.equ	CPU_EXTRA2_FUNC, CPU_EXTRA1_FUNC + CPU_EXTRA1_FUNC_SIZE
	.equ	CPU_PWR_DWN_OPS, CPU_EXTRA2_FUNC + CPU_EXTRA2_FUNC_SIZE
	.equ	CPU_ERRATA_FUNC, CPU_PWR_DWN_OPS + CPU_PWR_DWN_OPS_SIZE
	.equ	CPU_ERRATA_LOCK, CPU_ERRATA_FUNC + CPU_ERRATA_FUNC_SIZE
	.equ	CPU_ERRATA_PRINTED, CPU_ERRATA_LOCK + CPU_ERRATA_LOCK_SIZE
	.equ	CPU_REG_DUMP, CPU_ERRATA_PRINTED + CPU_ERRATA_PRINTED_SIZE
	.equ	CPU_OPS_SIZE, CPU_REG_DUMP + CPU_REG_DUMP_SIZE

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
	 *	This is a placeholder for future per CPU operations.  Currently
	 *	some CPUs use this entry to set a function to disable the
	 *	workaround for CVE-2018-3639.
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
		_extra1:req, _extra2:req, _power_down_ops:vararg
	.section cpu_ops, "a"
	.align 3
	.type cpu_ops_\_name, %object
	.quad \_midr
#if defined(IMAGE_AT_EL3)
	.quad \_resetfunc
#endif
	.quad \_extra1
	.quad \_extra2
#ifdef IMAGE_BL31
	/* Insert list of functions */
	fill_constants CPU_MAX_PWR_DWN_OPS, \_power_down_ops
#endif

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

#ifdef IMAGE_BL31
	/* Pointers to errata lock and reported flag */
	.quad \_name\()_errata_lock
	.quad \_name\()_errata_reported
#endif
#endif

#if defined(IMAGE_BL31) && CRASH_REPORTING
	.quad \_name\()_cpu_reg_dump
#endif
	.endm

	.macro declare_cpu_ops _name:req, _midr:req, _resetfunc:req, \
		_power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, 0, 0, \
			\_power_down_ops
	.endm

	.macro declare_cpu_ops_wa _name:req, _midr:req, \
		_resetfunc:req, _extra1:req, _extra2:req, \
		_power_down_ops:vararg
		declare_cpu_ops_base \_name, \_midr, \_resetfunc, \
			\_extra1, \_extra2, \_power_down_ops
	.endm

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

#endif /* CPU_MACROS_S */

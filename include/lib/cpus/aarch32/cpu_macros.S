/*
 * Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CPU_MACROS_S
#define CPU_MACROS_S

#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>

	/*
	 * Write given expressions as words
	 *
	 * _count:
	 *	Write at least _count words. If the given number of expressions
	 *	is less than _count, repeat the last expression to fill _count
	 *	words in total
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
	    .word \_this

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
	 *	Reset function for the CPU
	 * _power_down_ops:
	 *	Comma-separated list of functions to perform power-down
	 *	operatios on the CPU. At least one, and up to
	 *	CPU_MAX_PWR_DWN_OPS number of functions may be specified.
	 *	Starting at power level 0, these functions shall handle power
	 *	down at subsequent power levels. If there aren't exactly
	 *	CPU_MAX_PWR_DWN_OPS functions, the last specified one will be
	 *	used to handle power down at subsequent levels
	 */
	.macro declare_cpu_ops _name:req, _midr:req, _resetfunc:req, \
		_power_down_ops:vararg
	.section .cpu_ops, "a"
	.align 2
	.type cpu_ops_\_name, %object
	.word \_midr
#if defined(IMAGE_AT_EL3)
	.word \_resetfunc
#endif
#ifdef IMAGE_BL32
	/* Insert list of functions */
	fill_constants CPU_MAX_PWR_DWN_OPS, \_power_down_ops
#endif

	/*
	 * It is possible (although unlikely) that a cpu may have no errata in
	 * code. In that case the start label will not be defined. The list is
	 * inteded to be used in a loop, so define it as zero-length for
	 * predictable behaviour. Since this macro is always called at the end
	 * of the cpu file (after all errata have been parsed) we can be sure
	 * that we are at the end of the list. Some cpus call the macro twice,
	 * so only do this once.
	 */
	.pushsection .rodata.errata_entries
	.ifndef \_name\()_errata_list_start
		\_name\()_errata_list_start:
	.endif
	/* some call this multiple times, so only do this once */
	.ifndef \_name\()_errata_list_end
		\_name\()_errata_list_end:
	.endif
	.popsection

	/* and now put them in cpu_ops */
	.word \_name\()_errata_list_start
	.word \_name\()_errata_list_end

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

	.word \_name\()_cpu_str

#ifdef IMAGE_BL32
	/* Pointers to errata lock and reported flag */
	.word \_name\()_errata_lock
	.word \_name\()_errata_reported
#endif
#endif
	.endm

	/*
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 *
	 * Clobbers: r0-r1
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	ldcopr	r0, MIDR
	ubfx	r0, r0, #MIDR_PN_SHIFT, #12
	ldr	r1, =((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	cmp	r0, r1
	beq	\_label
	.endm

/*
 * NOTE an erratum and CVE id could clash. However, both numbers are very large
 * and the probablity is minuscule. Working around this makes code very
 * complicated and extremely difficult to read so it is not considered. In the
 * unlikely event that this does happen, prepending the CVE id with a 0 should
 * resolve the conflict
 */

/*
 * Add an entry for this erratum to the errata framework
 *
 * _cpu:
 *	Name of cpu as given to declare_cpu_ops
 *
 * _cve:
 *	Whether erratum is a CVE. CVE year if yes, 0 otherwise
 *
 * _id:
 *	Erratum or CVE number. Please combine with the previous field with the
 *	ERRATUM or CVE macros
 *
 * _chosen:
 *	Compile time flag on whether the erratum is included
 *
 * _special:
 *	The special non-standard name of an erratum
 */
.macro add_erratum_entry _cpu:req, _cve:req, _id:req, _chosen:req, _special
	.pushsection .rodata.errata_entries
		.align	2
		.ifndef \_cpu\()_errata_list_start
		\_cpu\()_errata_list_start:
		.endif

		.ifnb \_special
			.word	check_errata_\_special
		.elseif \_cve
			.word	check_errata_cve_\_cve\()_\_id
		.else
			.word	check_errata_\_id
		.endif
		/* Will fit CVEs with up to 10 character in the ID field */
		.word	\_id
		.hword	\_cve
		.byte	\_chosen
		.byte	0x0 /* alignment */
	.popsection
.endm

#endif /* CPU_MACROS_S */

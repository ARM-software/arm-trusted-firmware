/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL_COMMON_LD_H
#define BL_COMMON_LD_H

#include <platform_def.h>

#ifdef __aarch64__
#define STRUCT_ALIGN	8
#define BSS_ALIGN	16
#else
#define STRUCT_ALIGN	4
#define BSS_ALIGN	8
#endif

#ifndef DATA_ALIGN
#define DATA_ALIGN	1
#endif

#define CPU_OPS						\
	. = ALIGN(STRUCT_ALIGN);			\
	__CPU_OPS_START__ = .;				\
	KEEP(*(cpu_ops))				\
	__CPU_OPS_END__ = .;

#define PARSER_LIB_DESCS				\
	. = ALIGN(STRUCT_ALIGN);			\
	__PARSER_LIB_DESCS_START__ = .;			\
	KEEP(*(.img_parser_lib_descs))			\
	__PARSER_LIB_DESCS_END__ = .;

#define RT_SVC_DESCS					\
	. = ALIGN(STRUCT_ALIGN);			\
	__RT_SVC_DESCS_START__ = .;			\
	KEEP(*(rt_svc_descs))				\
	__RT_SVC_DESCS_END__ = .;

#define PMF_SVC_DESCS					\
	. = ALIGN(STRUCT_ALIGN);			\
	__PMF_SVC_DESCS_START__ = .;			\
	KEEP(*(pmf_svc_descs))				\
	__PMF_SVC_DESCS_END__ = .;

#define FCONF_POPULATOR					\
	. = ALIGN(STRUCT_ALIGN);			\
	__FCONF_POPULATOR_START__ = .;			\
	KEEP(*(.fconf_populator))			\
	__FCONF_POPULATOR_END__ = .;

/*
 * Keep the .got section in the RO section as it is patched prior to enabling
 * the MMU and having the .got in RO is better for security. GOT is a table of
 * addresses so ensure pointer size alignment.
 */
#define GOT						\
	. = ALIGN(STRUCT_ALIGN);			\
	__GOT_START__ = .;				\
	*(.got)						\
	__GOT_END__ = .;

/*
 * The base xlat table
 *
 * It is put into the rodata section if PLAT_RO_XLAT_TABLES=1,
 * or into the bss section otherwise.
 */
#define BASE_XLAT_TABLE					\
	. = ALIGN(16);					\
	*(base_xlat_table)

#if PLAT_RO_XLAT_TABLES
#define BASE_XLAT_TABLE_RO		BASE_XLAT_TABLE
#define BASE_XLAT_TABLE_BSS
#else
#define BASE_XLAT_TABLE_RO
#define BASE_XLAT_TABLE_BSS		BASE_XLAT_TABLE
#endif

#define RODATA_COMMON					\
	RT_SVC_DESCS					\
	FCONF_POPULATOR					\
	PMF_SVC_DESCS					\
	PARSER_LIB_DESCS				\
	CPU_OPS						\
	GOT						\
	BASE_XLAT_TABLE_RO

/*
 * .data must be placed at a lower address than the stacks if the stack
 * protector is enabled. Alternatively, the .data.stack_protector_canary
 * section can be placed independently of the main .data section.
 */
#define DATA_SECTION					\
	.data . : ALIGN(DATA_ALIGN) {			\
		__DATA_START__ = .;			\
		*(SORT_BY_ALIGNMENT(.data*))		\
		__DATA_END__ = .;			\
	}

/*
 * .rela.dyn needs to come after .data for the read-elf utility to parse
 * this section correctly.
 */
#define RELA_SECTION					\
	.rela.dyn : ALIGN(STRUCT_ALIGN) {		\
		__RELA_START__ = .;			\
		*(.rela*)				\
		__RELA_END__ = .;			\
	}

#if !(defined(IMAGE_BL31) && RECLAIM_INIT_CODE)
#define STACK_SECTION					\
	stacks (NOLOAD) : {				\
		__STACKS_START__ = .;			\
		*(tzfw_normal_stacks)			\
		__STACKS_END__ = .;			\
	}
#endif

/*
 * If BL doesn't use any bakery lock then __PERCPU_BAKERY_LOCK_SIZE__
 * will be zero. For this reason, the only two valid values for
 * __PERCPU_BAKERY_LOCK_SIZE__ are 0 or the platform defined value
 * PLAT_PERCPU_BAKERY_LOCK_SIZE.
 */
#ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
#define BAKERY_LOCK_SIZE_CHECK				\
	ASSERT((__PERCPU_BAKERY_LOCK_SIZE__ == 0) ||	\
	       (__PERCPU_BAKERY_LOCK_SIZE__ == PLAT_PERCPU_BAKERY_LOCK_SIZE), \
	       "PLAT_PERCPU_BAKERY_LOCK_SIZE does not match bakery lock requirements");
#else
#define BAKERY_LOCK_SIZE_CHECK
#endif

/*
 * Bakery locks are stored in normal .bss memory
 *
 * Each lock's data is spread across multiple cache lines, one per CPU,
 * but multiple locks can share the same cache line.
 * The compiler will allocate enough memory for one CPU's bakery locks,
 * the remaining cache lines are allocated by the linker script
 */
#if !USE_COHERENT_MEM
#define BAKERY_LOCK_NORMAL				\
	. = ALIGN(CACHE_WRITEBACK_GRANULE);		\
	__BAKERY_LOCK_START__ = .;			\
	__PERCPU_BAKERY_LOCK_START__ = .;		\
	*(bakery_lock)					\
	. = ALIGN(CACHE_WRITEBACK_GRANULE);		\
	__PERCPU_BAKERY_LOCK_END__ = .;			\
	__PERCPU_BAKERY_LOCK_SIZE__ = ABSOLUTE(__PERCPU_BAKERY_LOCK_END__ - __PERCPU_BAKERY_LOCK_START__); \
	. = . + (__PERCPU_BAKERY_LOCK_SIZE__ * (PLATFORM_CORE_COUNT - 1)); \
	__BAKERY_LOCK_END__ = .;			\
	BAKERY_LOCK_SIZE_CHECK
#else
#define BAKERY_LOCK_NORMAL
#endif

/*
 * Time-stamps are stored in normal .bss memory
 *
 * The compiler will allocate enough memory for one CPU's time-stamps,
 * the remaining memory for other CPUs is allocated by the
 * linker script
 */
#define PMF_TIMESTAMP					\
	. = ALIGN(CACHE_WRITEBACK_GRANULE);		\
	__PMF_TIMESTAMP_START__ = .;			\
	KEEP(*(pmf_timestamp_array))			\
	. = ALIGN(CACHE_WRITEBACK_GRANULE);		\
	__PMF_PERCPU_TIMESTAMP_END__ = .;		\
	__PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__); \
	. = . + (__PERCPU_TIMESTAMP_SIZE__ * (PLATFORM_CORE_COUNT - 1)); \
	__PMF_TIMESTAMP_END__ = .;


/*
 * The .bss section gets initialised to 0 at runtime.
 * Its base address has bigger alignment for better performance of the
 * zero-initialization code.
 */
#define BSS_SECTION					\
	.bss (NOLOAD) : ALIGN(BSS_ALIGN) {		\
		__BSS_START__ = .;			\
		*(SORT_BY_ALIGNMENT(.bss*))		\
		*(COMMON)				\
		BAKERY_LOCK_NORMAL			\
		PMF_TIMESTAMP				\
		BASE_XLAT_TABLE_BSS			\
		__BSS_END__ = .;			\
	}

/*
 * The xlat_table section is for full, aligned page tables (4K).
 * Removing them from .bss avoids forcing 4K alignment on
 * the .bss section. The tables are initialized to zero by the translation
 * tables library.
 */
#define XLAT_TABLE_SECTION				\
	xlat_table (NOLOAD) : {				\
		*(xlat_table)				\
	}

#endif /* BL_COMMON_LD_H */

/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_H
#define ERRATA_H

#include <lib/cpus/cpu_ops.h>

#define ERRATUM_CHECK_FUNC_SIZE	CPU_WORD_SIZE
#define ERRATUM_ID_SIZE		4
#define ERRATUM_CVE_SIZE	2
#define ERRATUM_CHOSEN_SIZE	1
#define ERRATUM_ALIGNMENT_SIZE	1

#define ERRATUM_CHECK_FUNC	0
#define ERRATUM_ID		ERRATUM_CHECK_FUNC + ERRATUM_CHECK_FUNC_SIZE
#define ERRATUM_CVE		ERRATUM_ID + ERRATUM_ID_SIZE
#define ERRATUM_CHOSEN		ERRATUM_CVE + ERRATUM_CVE_SIZE
#define ERRATUM_ALIGNMENT	ERRATUM_CHOSEN + ERRATUM_CHOSEN_SIZE
#define ERRATUM_ENTRY_SIZE	ERRATUM_ALIGNMENT + ERRATUM_ALIGNMENT_SIZE

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

/* Errata ID for smc workarounds */
#define ARCH_WORKAROUND_2		2
#define ARCH_WORKAROUND_3		3

#define INCLUDE_ERRATA_LIST	(		\
	REPORT_ERRATA			|	\
	ERRATA_ABI_SUPPORT		|	\
	WORKAROUND_CVE_2017_5715	|	\
	WORKAROUND_CVE_2018_3639	|	\
	WORKAROUND_CVE_2022_23960	|	\
	WORKAROUND_CVE_2024_7881)

#ifndef __ASSEMBLER__
#include <lib/cassert.h>

void print_errata_status(void);

/*
 * NOTE that this structure will be different on AArch32 and AArch64. The
 * uintptr_t will reflect the change and the alignment will be correct in both.
 */
struct erratum_entry {
	uintptr_t (*check_func)(uint64_t cpu_rev);
	/* Will fit CVEs with up to 10 character in the ID field */
	uint32_t id;
	/* Denote CVEs with their year or errata with 0 */
	uint16_t cve;
	/*
	 * a bitfield:
	 * bit 0 - denotes if the erratum is enabled in build.
	 * bit 1 - denotes if the erratum workaround is split and
	 * 	   also needs to be implemented at a lower EL.
	 */
	uint8_t chosen;
	uint8_t _alignment;
} __packed;

CASSERT(sizeof(struct erratum_entry) == ERRATUM_ENTRY_SIZE,
	assert_erratum_entry_asm_c_different_sizes);

/*
 * Runtime errata helpers.
 */
#if ERRATA_A75_764081
bool errata_a75_764081_applies(void);
#else
static inline bool errata_a75_764081_applies(void)
{
       return false;
}
#endif


bool check_if_trbe_disable_affected_core(void);
bool errata_ich_vmcr_el2_applies(void);
struct erratum_entry *find_erratum_entry(uint32_t errata_id);
int check_erratum_applies(uint32_t cve, int errata_id);

#else

/*
 * errata framework macro helpers
 *
 * NOTE an erratum and CVE id could clash. However, both numbers are very large
 * and the probablity is minuscule. Working around this makes code very
 * complicated and extremely difficult to read so it is not considered. In the
 * unlikely event that this does happen, prepending the CVE id with a 0 should
 * resolve the conflict
 */
#define NO_ISB			1
#define NO_ASSERT		0
#define NO_APPLY_AT_RESET	0
#define APPLY_AT_RESET		1
#define GET_CPU_REV		1
#define NO_GET_CPU_REV		0

/* useful for errata that end up always being worked around */
#define ERRATUM_ALWAYS_CHOSEN	1

#endif /* __ASSEMBLER__ */

#define ERRATUM(id)		0, id
#define CVE(year, id)		year, id

/* Macro to get CPU revision code for checking errata version compatibility. */
#define CPU_REV(r, p)		((r << 4) | p)

/* Used for errata that have split workaround */
#define SPLIT_WA			1

/* chosen bitfield entries */
#define WA_ENABLED_MASK			BIT(0)
#define SPLIT_WA_MASK			BIT(1)

#endif /* ERRATA_H */

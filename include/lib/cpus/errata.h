/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_REPORT_H
#define ERRATA_REPORT_H

#include <lib/cpus/cpu_ops.h>


#define ERRATUM_WA_FUNC_SIZE	CPU_WORD_SIZE
#define ERRATUM_CHECK_FUNC_SIZE	CPU_WORD_SIZE
#define ERRATUM_ID_SIZE		4
#define ERRATUM_CVE_SIZE	2
#define ERRATUM_CHOSEN_SIZE	1
#define ERRATUM_MITIGATED_SIZE	1

#define ERRATUM_WA_FUNC		0
#define ERRATUM_CHECK_FUNC	ERRATUM_WA_FUNC + ERRATUM_WA_FUNC_SIZE
#define ERRATUM_ID		ERRATUM_CHECK_FUNC + ERRATUM_CHECK_FUNC_SIZE
#define ERRATUM_CVE		ERRATUM_ID + ERRATUM_ID_SIZE
#define ERRATUM_CHOSEN		ERRATUM_CVE + ERRATUM_CVE_SIZE
#define ERRATUM_MITIGATED	ERRATUM_CHOSEN + ERRATUM_CHOSEN_SIZE
#define ERRATUM_ENTRY_SIZE	ERRATUM_MITIGATED + ERRATUM_MITIGATED_SIZE

#ifndef __ASSEMBLER__
#include <lib/cassert.h>

void print_errata_status(void);
void errata_print_msg(unsigned int status, const char *cpu, const char *id);

/*
 * NOTE that this structure will be different on AArch32 and AArch64. The
 * uintptr_t will reflect the change and the alignment will be correct in both.
 */
struct erratum_entry {
	uintptr_t (*wa_func)(uint64_t cpu_rev);
	uintptr_t (*check_func)(uint64_t cpu_rev);
	/* Will fit CVEs with up to 10 character in the ID field */
	uint32_t id;
	/* Denote CVEs with their year or errata with 0 */
	uint16_t cve;
	uint8_t chosen;
	/* TODO(errata ABI): placeholder for the mitigated field */
	uint8_t _mitigated;
} __packed;

CASSERT(sizeof(struct erratum_entry) == ERRATUM_ENTRY_SIZE,
	assert_erratum_entry_asm_c_different_sizes);
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
#define ERRATUM(id)		0, id
#define CVE(year, id)		year, id
#define NO_ISB			1
#define NO_ASSERT		0
#define NO_APPLY_AT_RESET	0
#define APPLY_AT_RESET		1
#define GET_CPU_REV		1
#define NO_GET_CPU_REV		0

/* useful for errata that end up always being worked around */
#define ERRATUM_ALWAYS_CHOSEN	1

#endif /* __ASSEMBLER__ */

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

/* Macro to get CPU revision code for checking errata version compatibility. */
#define CPU_REV(r, p)		((r << 4) | p)

#endif /* ERRATA_REPORT_H */

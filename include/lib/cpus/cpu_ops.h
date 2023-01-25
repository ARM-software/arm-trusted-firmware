/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_OPS_H
#define CPU_OPS_H

#include <arch.h>

#define CPU_IMPL_PN_MASK	(MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) | \
				(MIDR_PN_MASK << MIDR_PN_SHIFT)

/* Hardcode to keep compatible with assembly. sizeof(uintptr_t) */
#if __aarch64__
#define CPU_WORD_SIZE			8
#else
#define CPU_WORD_SIZE			4
#endif /* __aarch64__ */

/* The number of CPU operations allowed */
#define CPU_MAX_PWR_DWN_OPS		2
/* Special constant to specify that CPU has no reset function */
#define CPU_NO_RESET_FUNC		0

#if __aarch64__
#define CPU_NO_EXTRA1_FUNC		0
#define CPU_NO_EXTRA2_FUNC		0
#define CPU_NO_EXTRA3_FUNC		0
#endif /* __aarch64__ */


/*
 * Define the sizes of the fields in the cpu_ops structure. Word size is set per
 * Aarch so keep these definitions the same and each can include whatever it
 * needs.
 */
#define CPU_MIDR_SIZE		CPU_WORD_SIZE
#ifdef IMAGE_AT_EL3
#define CPU_RESET_FUNC_SIZE	CPU_WORD_SIZE
#else
#define CPU_RESET_FUNC_SIZE	0
#endif /* IMAGE_AT_EL3 */
#define CPU_EXTRA1_FUNC_SIZE	CPU_WORD_SIZE
#define CPU_EXTRA2_FUNC_SIZE	CPU_WORD_SIZE
#define CPU_EXTRA3_FUNC_SIZE	CPU_WORD_SIZE
#define CPU_E_HANDLER_FUNC_SIZE CPU_WORD_SIZE
/* The power down core and cluster is needed only in BL31 and BL32 */
#if defined(IMAGE_BL31) || defined(IMAGE_BL32)
#define CPU_PWR_DWN_OPS_SIZE	CPU_WORD_SIZE * CPU_MAX_PWR_DWN_OPS
#else
#define CPU_PWR_DWN_OPS_SIZE	0
#endif /* defined(IMAGE_BL31) || defined(IMAGE_BL32) */

/* Fields required to print errata status  */
#if REPORT_ERRATA
#define CPU_ERRATA_FUNC_SIZE	CPU_WORD_SIZE
/* BL1 doesn't require mutual exclusion and printed flag. */
#if defined(IMAGE_BL31) || defined(IMAGE_BL32)
#define CPU_ERRATA_LOCK_SIZE	CPU_WORD_SIZE
#define CPU_ERRATA_PRINTED_SIZE	CPU_WORD_SIZE
#else
#define CPU_ERRATA_LOCK_SIZE	0
#define CPU_ERRATA_PRINTED_SIZE	0
#endif /* defined(IMAGE_BL31) || defined(IMAGE_BL32) */
#else
#define CPU_ERRATA_FUNC_SIZE	0
#define CPU_ERRATA_LOCK_SIZE	0
#define CPU_ERRATA_PRINTED_SIZE	0
#endif /* REPORT_ERRATA */

#if defined(IMAGE_BL31) && CRASH_REPORTING
#define CPU_REG_DUMP_SIZE	CPU_WORD_SIZE
#else
#define CPU_REG_DUMP_SIZE	0
#endif /* defined(IMAGE_BL31) && CRASH_REPORTING */


/*
 * Define the offsets to the fields in cpu_ops structure. Every offset is
 * defined based on the offset and size of the previous field.
 */
#define CPU_MIDR		0
#define CPU_RESET_FUNC		CPU_MIDR + CPU_MIDR_SIZE
#if __aarch64__
#define CPU_EXTRA1_FUNC		CPU_RESET_FUNC + CPU_RESET_FUNC_SIZE
#define CPU_EXTRA2_FUNC		CPU_EXTRA1_FUNC + CPU_EXTRA1_FUNC_SIZE
#define CPU_EXTRA3_FUNC		CPU_EXTRA2_FUNC + CPU_EXTRA2_FUNC_SIZE
#define CPU_E_HANDLER_FUNC	CPU_EXTRA3_FUNC + CPU_EXTRA3_FUNC_SIZE
#define CPU_PWR_DWN_OPS		CPU_E_HANDLER_FUNC + CPU_E_HANDLER_FUNC_SIZE
#else
#define CPU_PWR_DWN_OPS		CPU_RESET_FUNC + CPU_RESET_FUNC_SIZE
#endif /* __aarch64__ */
#define CPU_ERRATA_FUNC		CPU_PWR_DWN_OPS + CPU_PWR_DWN_OPS_SIZE
#define CPU_ERRATA_LOCK		CPU_ERRATA_FUNC + CPU_ERRATA_FUNC_SIZE
#define CPU_ERRATA_PRINTED	CPU_ERRATA_LOCK + CPU_ERRATA_LOCK_SIZE
#if __aarch64__
#define CPU_REG_DUMP		CPU_ERRATA_PRINTED + CPU_ERRATA_PRINTED_SIZE
#define CPU_OPS_SIZE		CPU_REG_DUMP + CPU_REG_DUMP_SIZE
#else
#define CPU_OPS_SIZE		CPU_ERRATA_PRINTED + CPU_ERRATA_PRINTED_SIZE
#endif /* __aarch64__ */

#endif /* CPU_OPS_H */

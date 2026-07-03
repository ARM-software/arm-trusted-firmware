/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WA_CVE_2025_0647_CPPRCTX_H_
#define _WA_CVE_2025_0647_CPPRCTX_H_

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * API to perform CPP RCTX instruction functionality in EL3
 *
 * Rather than trapping EL3 to EL3 if CPP RCTX is needed, it is simpler to just
 * have an API that performs the workaround steps. TF-A does not support nested
 * exceptions outside of specific circumstances, and enabling that generically
 * is not trivial, so this is a simpler and faster solution.
 *
 * The workaround is not reliant on the config register passed to the CPP RCTX
 * instruction, but the argument is included for compatibility in systems that
 * might have some cores that need the workaround and some that do not. If the
 * workaround is not needed, the argument will be used in a normal CPP RCTX call
 * rather than the workaround procedure.
 */
void wa_cve_2025_0647_execute_cpp_el3(uint64_t arg);

#endif

/*
 * System register definitions used in this workaround
 *
 * Some of these definitions exist in CPU-specific header files but for the
 * purposes of this workaround it is convenient to have them all here. Currently
 * all CPUs affected by this issue share these definitions, but if future CPUs
 * have different register mappings this will need to be updated to pull from
 * the CPU-specific headers instead.
 */

#define	WA_CPUACTLR_EL1		S3_0_C15_C1_0
#define	WA_CPUACTLR2_EL1	S3_0_C15_C1_1
#define	WA_CPUECTLR_EL1		S3_0_C15_C1_4
#define	WA_CPUPSELR_EL3		S3_6_C15_C8_0
#define	WA_CPUPCR_EL3		S3_6_C15_C8_1
#define	WA_CPUPOR_EL3		S3_6_C15_C8_2
#define	WA_CPUPMR_EL3		S3_6_C15_C8_3

/* Flags passed to workaround function in X0. */
#define WA_LS_RCG_EN_BIT	(1)
#define	WA_LS_RCG_EN		BIT(WA_LS_RCG_EN_BIT)

/* Fields passed to init function in X0. */
#define	WA_USE_T32_OPCODE_SHIFT	(3)
#define	WA_USE_T32_OPCODE	BIT(WA_USE_T32_OPCODE_SHIFT)
#define	WA_PATCH_SLOT_MASK	(0x7)
#define	WA_PATCH_SLOT(x)	(x & WA_PATCH_SLOT_MASK)

#endif /* _WA_CVE_2025_0647_CPPRCTX_H_ */

/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A76AE_H
#define CORTEX_A76AE_H

#include <lib/utils_def.h>

/* Cortex-A76AE MIDR for revision 0 */
#define CORTEX_A76AE_MIDR		U(0x410FD0E0)

/* Cortex-A76 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A76AE_BHB_LOOP_COUNT	U(24)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A76AE_CPUPWRCTLR_EL1	S3_0_C15_C2_7

/* Definitions of register field mask in CORTEX_A76AE_CPUPWRCTLR_EL1 */
#define CORTEX_A76AE_CORE_PWRDN_EN_MASK	U(0x1)

#define CORTEX_A76AE_CPUECTLR_EL1	S3_0_C15_C1_4

#endif /* CORTEX_A76AE_H */

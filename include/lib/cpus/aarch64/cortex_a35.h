/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A35_H
#define CORTEX_A35_H

#include <lib/utils_def.h>

/* Cortex-A35 Main ID register for revision 0 */
#define CORTEX_A35_MIDR				U(0x410FD040)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 * CPUECTLR_EL1 is an implementation-specific register.
 ******************************************************************************/
#define CORTEX_A35_CPUECTLR_EL1			S3_1_C15_C2_1
#define CORTEX_A35_CPUECTLR_SMPEN_BIT		(ULL(1) << 6)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A35_CPUACTLR_EL1			S3_1_C15_C2_0

#define CORTEX_A35_CPUACTLR_EL1_ENDCCASCI	(ULL(1) << 44)

#endif /* CORTEX_A35_H */

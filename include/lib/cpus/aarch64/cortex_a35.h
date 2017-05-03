/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A35_H__
#define __CORTEX_A35_H__

/* Cortex-A35 Main ID register for revision 0 */
#define CORTEX_A35_MIDR				0x410FD040

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 * CPUECTLR_EL1 is an implementation-specific register.
 ******************************************************************************/
#define CORTEX_A35_CPUECTLR_EL1			S3_1_C15_C2_1
#define CORTEX_A35_CPUECTLR_SMPEN_BIT		(1 << 6)

#endif /* __CORTEX_A35_H__ */

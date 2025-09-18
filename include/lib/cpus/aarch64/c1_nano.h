/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C1_NANO_H
#define C1_NANO_H

#define C1_NANO_MIDR					U(0x410FD8A0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define C1_NANO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* C1_NANO_H */

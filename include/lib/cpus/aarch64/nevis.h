/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEVIS_H
#define NEVIS_H

#define NEVIS_MIDR					U(0x410FD8A0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEVIS_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEVIS_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEVIS_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* NEVIS_H */

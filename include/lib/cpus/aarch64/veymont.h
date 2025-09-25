/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VEYMONT_H
#define VEYMONT_H

#define VEYMONT_MIDR		                        U(0x410FD9A0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define VEYMONT_IMP_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define VEYMONT_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define VEYMONT_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* VEYMONT_H */


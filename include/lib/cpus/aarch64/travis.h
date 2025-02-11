/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRAVIS_H
#define TRAVIS_H

#define TRAVIS_MIDR					U(0x410FD8C0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define TRAVIS_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define TRAVIS_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define TRAVIS_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* TRAVIS_H */

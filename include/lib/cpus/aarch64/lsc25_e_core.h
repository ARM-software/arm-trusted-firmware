/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LSC25_E_CORE_H
#define LSC25_E_CORE_H

#include <lib/utils_def.h>

#define LSC25_E_CORE_MIDR					U(0x410FD930)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define LSC25_E_CORE_IMP_CPUECTLR_EL1				S3_0_C15_C1_5

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define LSC25_E_CORE_IMP_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define LSC25_E_CORE_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

/*******************************************************************************
 * SME Control registers
 ******************************************************************************/
#define LSC25_E_CORE_SVCRSM					S0_3_C4_C2_3
#define LSC25_E_CORE_SVCRZA					S0_3_C4_C4_3

#endif /* LSC25_E_CORE_H */

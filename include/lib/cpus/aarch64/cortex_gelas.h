/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_GELAS_H
#define CORTEX_GELAS_H

#include <lib/utils_def.h>

#define CORTEX_GELAS_MIDR				U(0x410FD8B0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_GELAS_IMP_CPUECTLR_EL1			S3_0_C15_C1_5

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_GELAS_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_GELAS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

/*******************************************************************************
 * SME Control registers
 ******************************************************************************/
#define CORTEX_GELAS_SVCRSM				S0_3_C4_C2_3
#define CORTEX_GELAS_SVCRZA				S0_3_C4_C4_3

#endif /* CORTEX_GELAS_H */

/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C1_PRO_H
#define C1_PRO_H

#include <lib/utils_def.h>

#define C1_PRO_MIDR				        U(0x410FD8B0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_PRO_IMP_CPUECTLR_EL1				S3_0_C15_C1_5
#define C1_PRO_CPUECTLR2_EL1_EXTLLC_BIT			U(10)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_PRO_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define C1_PRO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#endif /* C1_PRO_H */

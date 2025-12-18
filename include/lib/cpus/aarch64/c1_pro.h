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
 * CPU Generic System Control register specific definitions
 ******************************************************************************/
#define C1_PRO_IMP_CPUACTLR2_EL1			S3_0_C15_C1_1
#define C1_PRO_IMP_CPUPSELR_EL3				S3_6_C15_C8_0
#define C1_PRO_IMP_CPUPCR_EL3				S3_6_C15_C8_1
#define C1_PRO_IMP_CPUPOR_EL3				S3_6_C15_C8_2
#define C1_PRO_IMP_CPUPMR_EL3				S3_6_C15_C8_3

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_PRO_IMP_CPUECTLR_EL1				S3_0_C15_C1_4
#define C1_PRO_CPUECTLR2_EL1_EXTLLC_BIT			U(10)
#define C1_PRO_IMP_CPUECTLR2_EL1			S3_0_C15_C1_5

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_PRO_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define C1_PRO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#ifndef __ASSEMBLER__
long check_erratum_c1_pro_3300099(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* C1_PRO_H */

/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_N3_H
#define NEOVERSE_N3_H

#define NEOVERSE_N3_MIDR				U(0x410FD8E0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_N3_CPUECTLR_EL1			S3_0_C15_C1_4
#define NEOVERSE_N3_CPUECTLR_EL1_EXTLLC_BIT		(ULL(1) << 0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_N3_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_N3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#ifndef __ASSEMBLER__
long check_erratum_neoverse_n3_3699563(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* NEOVERSE_N3_H */

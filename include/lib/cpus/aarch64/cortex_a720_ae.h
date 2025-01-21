/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A720_AE_H
#define CORTEX_A720_AE_H

#define CORTEX_A720_AE_MIDR				U(0x410FD890)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A720_AE_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A720_AE_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_A720_AE_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#ifndef __ASSEMBLER__
long check_erratum_cortex_a720_ae_3699562(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A720_AE_H */

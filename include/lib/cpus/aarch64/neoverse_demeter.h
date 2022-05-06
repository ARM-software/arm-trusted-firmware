/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_DEMETER_H
#define NEOVERSE_DEMETER_H

#define NEOVERSE_DEMETER_MIDR				U(0x410FD4F0)

/* Neoverse Demeter loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_DEMETER_BHB_LOOP_COUNT			U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_DEMETER_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_DEMETER_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_DEMETER_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#endif /* NEOVERSE_DEMETER_H */

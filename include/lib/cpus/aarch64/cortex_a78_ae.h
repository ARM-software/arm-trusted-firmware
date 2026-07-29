/*
 * Copyright (c) 2019-2026, Arm Limited. All rights reserved.
 * Copyright (c) 2021-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A78_AE_H
#define CORTEX_A78_AE_H

#include <cortex_a78.h>

#define CORTEX_A78_AE_MIDR 				U(0x410FD420)

/* Cortex-A78AE loop count for CVE-2022-23960 mitigation */
#define CORTEX_A78_AE_BHB_LOOP_COUNT			U(32)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A78_AE_CPUECTLR_EL1			CORTEX_A78_CPUECTLR_EL1
#define CORTEX_A78_AE_CPUECTLR_EL1_BIT_8		CORTEX_A78_CPUECTLR_EL1_BIT_8

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_A78_AE_ACTLR2_EL1			CORTEX_A78_ACTLR2_EL1
#define CORTEX_A78_AE_ACTLR2_EL1_BIT_0			CORTEX_A78_ACTLR2_EL1_BIT_0
#define CORTEX_A78_AE_ACTLR2_EL1_BIT_40			CORTEX_A78_ACTLR2_EL1_BIT_40
#define CORTEX_A78_AE_CPUACTLR3_EL1			S3_0_C15_C1_2
#define CORTEX_A78_AE_CPUACTLR5_EL1			S3_0_C15_C9_0

#endif /* CORTEX_A78_AE_H */

/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CADDO_H
#define CADDO_H

#define CADDO_MIDR		                        U(0x410FDA00)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CADDO_IMP_CPUECTLR_EL1		        	S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CADDO_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CADDO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* CADDO_H */


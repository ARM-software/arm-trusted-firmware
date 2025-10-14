/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VENOM_H
#define VENOM_H

#define VENOM_MIDR		                        U(0x410FD980)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define VENOM_IMP_CPUECTLR_EL1		        	S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define VENOM_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define VENOM_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* VENOM_H */


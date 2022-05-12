/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_POSEIDON_H
#define NEOVERSE_POSEIDON_H


#define NEOVERSE_POSEIDON_MIDR                      		U(0x410FD830)

/* Neoverse Poseidon loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_POSEIDON_BHB_LOOP_COUNT			U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_POSEIDON_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_POSEIDON_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_POSEIDON_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* NEOVERSE_POSEIDON_H */

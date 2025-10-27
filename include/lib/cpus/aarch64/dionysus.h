/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DIONYSUS_H
#define DIONYSUS_H

#include <lib/utils_def.h>

#define DIONYSUS_MIDR						U(0x410FD940)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define DIONYSUS_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define DIONYSUS_CPUPWRCTLR_EL1					S3_0_C15_C2_7
#define DIONYSUS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT			U(1)

#endif /* DIONYSUS_H */

/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A32_H
#define CORTEX_A32_H

#include <lib/utils_def.h>

/* Cortex-A32 Main ID register for revision 0 */
#define CORTEX_A32_MIDR				U(0x410FD010)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 * CPUECTLR_EL1 is an implementation-specific register.
 ******************************************************************************/
#define CORTEX_A32_CPUECTLR_EL1			p15, 1, c15
#define CORTEX_A32_CPUECTLR_SMPEN_BIT		(ULL(1) << 6)

#endif /* CORTEX_A32_H */

/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A32_H__
#define __CORTEX_A32_H__

/* Cortex-A32 Main ID register for revision 0 */
#define CORTEX_A32_MIDR				0x410FD010

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 * CPUECTLR_EL1 is an implementation-specific register.
 ******************************************************************************/
#define CORTEX_A32_CPUECTLR_EL1			p15, 1, c15
#define CORTEX_A32_CPUECTLR_SMPEN_BIT		(1 << 6)

#endif /* __CORTEX_A32_H__ */

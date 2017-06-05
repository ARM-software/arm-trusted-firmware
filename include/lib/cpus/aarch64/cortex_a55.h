/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A55_H__
#define __CORTEX_A55_H__

/* Cortex-A55 MIDR for revision 0 */
#define CORTEX_A55_MIDR		0x410fd050

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A55_CPUPWRCTLR_EL1		S3_0_C15_C2_7
#define CORTEX_A55_CPUECTLR_EL1		S3_0_C15_C1_4

/* Definitions of register field mask in CORTEX_A55_CPUPWRCTLR_EL1 */
#define CORTEX_A55_CORE_PWRDN_EN_MASK	0x1

#endif /* __CORTEX_A55_H__ */

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A12_H
#define CORTEX_A12_H

/*******************************************************************************
 * Cortex-A12 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A12_MIDR			0x410FC0C0

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A12_ACTLR_SMP_BIT	(1 << 6)

#endif /* CORTEX_A12_H */

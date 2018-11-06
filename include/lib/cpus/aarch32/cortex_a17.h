/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A17_H
#define CORTEX_A17_H

/*******************************************************************************
 * Cortex-A17 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A17_MIDR			0x410FC0E0

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A17_ACTLR_SMP_BIT	(1 << 6)

#endif /* CORTEX_A17_H */

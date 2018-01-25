/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A15_H__
#define __CORTEX_A15_H__

/*******************************************************************************
 * Cortex-A15 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A15_MIDR			0x410FC0F0

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A15_ACTLR_INV_BTB_BIT	(1 << 0)
#define CORTEX_A15_ACTLR_SMP_BIT	(1 << 6)

#endif /* __CORTEX_A15_H__ */

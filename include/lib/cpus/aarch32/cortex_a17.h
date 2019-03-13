/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A17_H
#define CORTEX_A17_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Cortex-A17 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A17_MIDR			U(0x410FC0E0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A17_ACTLR_SMP_BIT	(U(1) << 6)

/*******************************************************************************
 * Implementation defined register specific definitions.
 ******************************************************************************/
#define CORTEX_A17_IMP_DEF_REG1		p15, 0, c15, c0, 1

#endif /* CORTEX_A17_H */

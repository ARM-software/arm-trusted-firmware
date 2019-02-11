/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A5_H
#define CORTEX_A5_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Cortex-A8 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A5_MIDR			U(0x410FC050)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A5_ACTLR_SMP_BIT		(U(1) << 6)

#endif /* CORTEX_A5_H */

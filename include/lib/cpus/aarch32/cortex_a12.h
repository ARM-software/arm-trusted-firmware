/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A12_H
#define CORTEX_A12_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Cortex-A12 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A12_MIDR			U(0x410FC0D0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A12_ACTLR_SMP_BIT	(U(1) << 6)

#endif /* CORTEX_A12_H */

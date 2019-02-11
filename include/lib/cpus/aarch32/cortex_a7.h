/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A7_H
#define CORTEX_A7_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Cortex-A7 midr with version/revision set to 0
 ******************************************************************************/
#define CORTEX_A7_MIDR			U(0x410FC070)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A7_ACTLR_SMP_BIT		(U(1) << 6)

#endif /* CORTEX_A7_H */

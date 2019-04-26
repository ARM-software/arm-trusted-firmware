/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL32_PARAM_H
#define BL32_PARAM_H

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* TF txet, ro, rw, Size: 2MB */
#define TZRAM_BASE		(0x0)
#define TZRAM_SIZE		(0x200000)

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
/*
 * Put BL32 at the top of the Trusted RAM
 */
#define BL32_BASE			(TZRAM_BASE + 0x100000)
#define BL32_LIMIT			(TZRAM_BASE + TZRAM_SIZE)

#endif /* BL32_PARAM_H */

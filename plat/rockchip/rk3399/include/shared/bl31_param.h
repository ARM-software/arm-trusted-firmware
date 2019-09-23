/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31_PARAM_H
#define BL31_PARAM_H

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* TF text, ro, rw, Size: 1MB */
#define TZRAM_BASE		(0x0)
#define TZRAM_SIZE		(0x100000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at the top of the Trusted RAM
 */
#define BL31_BASE		(TZRAM_BASE + 0x40000)
#define BL31_LIMIT		(TZRAM_BASE + TZRAM_SIZE)

#endif /* BL31_PARAM_H */

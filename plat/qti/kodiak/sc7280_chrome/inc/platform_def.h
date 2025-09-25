/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <kodiak_def.h>

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_BASE				0xC0000000
#define BL31_SIZE				0x00100000
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

#endif /* PLATFORM_DEF_H */

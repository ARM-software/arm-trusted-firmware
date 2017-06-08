/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HIKEY960_DEF_H__
#define __HIKEY960_DEF_H__

#include <common_def.h>
#include <tbbr_img_def.h>

#define DDR_BASE			0x0
#define DDR_SIZE			0xC0000000

#define DEVICE_BASE			0xE0000000
#define DEVICE_SIZE			0x20000000

/*
 * PL011 related constants
 */
#define PL011_UART5_BASE		0xFDF05000
#define PL011_UART6_BASE		0xFFF32000
#define PL011_BAUDRATE			115200
#define PL011_UART_CLK_IN_HZ		19200000

#define UFS_BASE			0
/* FIP partition */
#define HIKEY960_FIP_BASE		(UFS_BASE + 0x1400000)
#define HIKEY960_FIP_MAX_SIZE		(12 << 20)

#define HIKEY960_UFS_DESC_BASE		0x20000000
#define HIKEY960_UFS_DESC_SIZE		0x00200000	/* 2MB */
#define HIKEY960_UFS_DATA_BASE		0x10000000
#define HIKEY960_UFS_DATA_SIZE		0x0A000000	/* 160MB */

#endif /* __HIKEY960_DEF_H__ */

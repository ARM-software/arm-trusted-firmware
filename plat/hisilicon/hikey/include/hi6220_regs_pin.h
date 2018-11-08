/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HI6220_REGS_PIN_H
#define HI6220_REGS_PIN_H

#define IOMG_BASE				0xF7010000

#define IOMG_SD_CLK				(IOMG_BASE + 0x0C)
#define IOMG_SD_CMD				(IOMG_BASE + 0x10)
#define IOMG_SD_DATA0				(IOMG_BASE + 0x14)
#define IOMG_SD_DATA1				(IOMG_BASE + 0x18)
#define IOMG_SD_DATA2				(IOMG_BASE + 0x1C)
#define IOMG_SD_DATA3				(IOMG_BASE + 0x20)
#define IOMG_GPIO24				(IOMG_BASE + 0x140)

#define IOMG_MUX_FUNC0				0
#define IOMG_MUX_FUNC1				1
#define IOMG_MUX_FUNC2				2

#define IOCG1_BASE				0xF7010800
#define IOCG2_BASE				0xF8001800

#define IOCG_SD_CLK				(IOCG1_BASE + 0x0C)
#define IOCG_SD_CMD				(IOCG1_BASE + 0x10)
#define IOCG_SD_DATA0				(IOCG1_BASE + 0x14)
#define IOCG_SD_DATA1				(IOCG1_BASE + 0x18)
#define IOCG_SD_DATA2				(IOCG1_BASE + 0x1C)
#define IOCG_SD_DATA3				(IOCG1_BASE + 0x20)
#define IOCG_GPIO24				(IOCG1_BASE + 0x150)
#define IOCG_GPIO8				(IOCG2_BASE + 0x30)

#define IOCG_DRIVE_8MA				(2 << 4)
#define IOCG_DRIVE_10MA				(3 << 4)
#define IOCG_INPUT_16MA				0x64
#define IOCG_INPUT_12MA				0x54
#define IOCG_PULLDOWN				(1 << 1)
#define IOCG_PULLUP				(1 << 0)

#endif /* HI6220_REGS_PIN_H */

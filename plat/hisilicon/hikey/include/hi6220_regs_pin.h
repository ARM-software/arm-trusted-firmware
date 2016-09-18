/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __HI6220_PIN_H__
#define __HI6220_PIN_H__

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

#endif	/* __HI6220_PIN_H__ */

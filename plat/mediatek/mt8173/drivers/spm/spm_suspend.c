/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <bakery_lock.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <spm.h>
#include <spm_suspend.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware.
 * This driver controls the system power in system suspend flow.
 */

#define WAIT_UART_ACK_TIMES     80	/* 80 * 10us */

#define WAKE_SRC_FOR_SUSPEND					\
	(WAKE_SRC_KP | WAKE_SRC_EINT | WAKE_SRC_MD32 |		\
	WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_THERM |	\
	WAKE_SRC_SYSPWREQ | WAKE_SRC_ALL_MD32)

#define WAKE_SRC_FOR_MD32  0

#define spm_is_wakesrc_invalid(wakesrc)	\
	(!!((unsigned int)(wakesrc) & 0xc0003803))

#define I2C1_BASE		(I2C_BASE + 0x1000)
#define I2C1_DATA_PORT		(I2C1_BASE + 0x00)
#define I2C1_SLAVE_ADDR		(I2C1_BASE + 0x04)
#define I2C1_INTR_MASK		(I2C1_BASE + 0x08)
#define I2C1_INTR_STAT		(I2C1_BASE + 0x0C)
#define I2C1_CONTROL		(I2C1_BASE + 0x10)
#define I2C1_TRANSFER_LEN	(I2C1_BASE + 0x14)
#define I2C1_TIMING		(I2C1_BASE + 0x20)
#define I2C1_START		(I2C1_BASE + 0x24)
#define I2C_MASTER_WRITE	(0U)
#define I2C_MASK_TRANSAC_COMP	(1U)
#define I2C_START		(1U)
#define I2C_ALLOW_ALL_INTR	(0xFF)

#define ARMCA15PLL_CON0		(APMIXED_BASE + 0x200)
#define ARMCA15PLL_CON1		(APMIXED_BASE + 0x204)
#define ARMCA15PLL_PWR_CON0	(APMIXED_BASE + 0x20c)
#define ARMCA15PLL_PWR_ON	(1U << 0)
#define ARMCA15PLL_ISO_EN	(1U << 1)
#define ARMCA15PLL_EN		(1U << 0)

#define PERI_GLOBALCON_PDN0_SET	(0x10003008)
#define PERI_GLOBALCON_PDN0_CLR	(0x10003010)
#define	I2C1_PDN		(1U << 24)

const unsigned int spm_flags =
	SPM_DUALVCORE_PDN_DIS | SPM_PASR_DIS | SPM_DPD_DIS |
	SPM_CPU_DVS_DIS | SPM_OPT | SPM_INFRA_PDN_DIS;

enum wake_reason_t spm_wake_reason = WR_NONE;

/**********************************************************
 * PCM sequence for cpu suspend
 **********************************************************/
static const unsigned int suspend_binary_ca7[] = {
	0x81f58407, 0x81f68407, 0x803a0400, 0x803a8400, 0x1b80001f, 0x20000000,
	0x80300400, 0x80318400, 0x80328400, 0xa1d28407, 0x81f20407, 0x81409801,
	0xd8000245, 0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c031a0, 0x1200041f,
	0x80310400, 0x1b80001f, 0x2000000a, 0xa0110400, 0x18c0001f, 0x100062c8,
	0xe0e00010, 0xe0e00030, 0xe0e00070, 0xe0e000f0, 0x1b80001f, 0x2000001a,
	0xe0e00ff0, 0xe8208000, 0x10006354, 0xfffe7fff, 0xe8208000, 0x10006834,
	0x00000010, 0x81f00407, 0xa1dd0407, 0x81fd0407, 0xc2803780, 0x1290041f,
	0x8880000c, 0x2f7be75f, 0xd8200642, 0x17c07c1f, 0x1b00001f, 0x7fffe7ff,
	0xd0000680, 0x17c07c1f, 0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f,
	0x80880001, 0xd8000762, 0x17c07c1f, 0xd00027a0, 0x1200041f, 0xe8208000,
	0x10006834, 0x00000000, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0x20000004,
	0xd820092c, 0x17c07c1f, 0xe8208000, 0x10006834, 0x00000010, 0xd00011a0,
	0x17c07c1f, 0x18c0001f, 0x10006608, 0x1910001f, 0x10006608, 0x813b0404,
	0xe0c00004, 0x1880001f, 0x10006320, 0xc0c03680, 0xe080000f, 0xd8200b23,
	0x17c07c1f, 0x1b00001f, 0x7ffff7ff, 0xd00011a0, 0x17c07c1f, 0xe080001f,
	0xe8208000, 0x10006354, 0xffffffff, 0x18c0001f, 0x100062c8, 0xe0e000f0,
	0xe0e00030, 0xe0e00000, 0x81409801, 0xd8000fe5, 0x17c07c1f, 0x18c0001f,
	0x10004094, 0x1910001f, 0x1020e374, 0xe0c00004, 0x18c0001f, 0x10004098,
	0x1910001f, 0x1020e378, 0xe0c00004, 0x18c0001f, 0x10011094, 0x1910001f,
	0x10213374, 0xe0c00004, 0x18c0001f, 0x10011098, 0x1910001f, 0x10213378,
	0xe0c00004, 0x1910001f, 0x10213378, 0x18c0001f, 0x10006234, 0xc0c03360,
	0x17c07c1f, 0xc2803780, 0x1290841f, 0xa1d20407, 0x81f28407, 0xa1d68407,
	0xa0128400, 0xa0118400, 0xa0100400, 0xa01a8400, 0xa01a0400, 0x19c0001f,
	0x001c239f, 0x1b00001f, 0x3fffefff, 0xf0000000, 0x17c07c1f, 0x808d8001,
	0xd8201422, 0x17c07c1f, 0x803d8400, 0x1b80001f, 0x2000001a, 0x80340400,
	0x17c07c1f, 0x17c07c1f, 0x80310400, 0x81fa0407, 0x81f18407, 0x81f08407,
	0xa1dc0407, 0x1b80001f, 0x200000b6, 0xd00020e0, 0x17c07c1f, 0x1880001f,
	0x20000208, 0x81411801, 0xd8001605, 0x17c07c1f, 0xe8208000, 0x1000f600,
	0xd2000000, 0x1380081f, 0x18c0001f, 0x10006240, 0xe0e00016, 0xe0e0001e,
	0xe0e0000e, 0xe0e0000f, 0x80368400, 0x1380081f, 0x80370400, 0x1380081f,
	0x80360400, 0x803e0400, 0x1380081f, 0x80380400, 0x803b0400, 0xa01d8400,
	0x1b80001f, 0x20000034, 0x803d8400, 0x1b80001f, 0x20000152, 0x803d0400,
	0x1380081f, 0x18c0001f, 0x1000f5c8, 0x1910001f, 0x1000f5c8, 0xa1000404,
	0xe0c00004, 0x18c0001f, 0x100125c8, 0x1910001f, 0x100125c8, 0xa1000404,
	0xe0c00004, 0x1910001f, 0x100125c8, 0x80340400, 0x17c07c1f, 0x17c07c1f,
	0x80310400, 0xe8208000, 0x10000044, 0x00000100, 0x1b80001f, 0x20000068,
	0x1b80001f, 0x2000000a, 0x18c0001f, 0x10006240, 0xe0e0000d, 0xd8001e65,
	0x17c07c1f, 0x18c0001f, 0x100040f4, 0x1910001f, 0x100040f4, 0xa11c8404,
	0xe0c00004, 0x1b80001f, 0x2000000a, 0x813c8404, 0xe0c00004, 0x18c0001f,
	0x100110f4, 0x1910001f, 0x100110f4, 0xa11c8404, 0xe0c00004, 0x1b80001f,
	0x2000000a, 0x813c8404, 0xe0c00004, 0x1b80001f, 0x20000100, 0x81fa0407,
	0x81f18407, 0x81f08407, 0xe8208000, 0x10006354, 0xfffe7b47, 0x18c0001f,
	0x65930003, 0xc0c03080, 0x17c07c1f, 0xa1d80407, 0xa1dc0407, 0x18c0001f,
	0x10006608, 0x1910001f, 0x10006608, 0xa11b0404, 0xe0c00004, 0xc2803780,
	0x1291041f, 0x8880000c, 0x2f7be75f, 0xd8202222, 0x17c07c1f, 0x1b00001f,
	0x3fffe7ff, 0xd0002260, 0x17c07c1f, 0x1b00001f, 0xbfffe7ff, 0xf0000000,
	0x17c07c1f, 0x1890001f, 0x10006608, 0x808b0801, 0xd8202502, 0x17c07c1f,
	0x1880001f, 0x10006320, 0xc0c03400, 0xe080000f, 0xd8002663, 0x17c07c1f,
	0xe080001f, 0xa1da0407, 0x81fc0407, 0xa0110400, 0xa0140400, 0xa01d8400,
	0xd0002fc0, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x1890001f, 0x10006608,
	0x80c98801, 0x810a8801, 0x10918c1f, 0xa0939002, 0x8080080d, 0xd82027a2,
	0x12007c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0x20000004, 0xd800304c,
	0x17c07c1f, 0x1b00001f, 0xbfffe7ff, 0xd0003040, 0x17c07c1f, 0x81f80407,
	0x81fc0407, 0x18c0001f, 0x65930006, 0xc0c03080, 0x17c07c1f, 0x18c0001f,
	0x65930007, 0xc0c03080, 0x17c07c1f, 0x1880001f, 0x10006320, 0xc0c03400,
	0xe080000f, 0xd8002663, 0x17c07c1f, 0xe080001f, 0x18c0001f, 0x65930005,
	0xc0c03080, 0x17c07c1f, 0xa1da0407, 0xe8208000, 0x10000048, 0x00000100,
	0x1b80001f, 0x20000068, 0xa0110400, 0xa0140400, 0x18c0001f, 0x1000f5c8,
	0x1910001f, 0x1000f5c8, 0x81200404, 0xe0c00004, 0x18c0001f, 0x100125c8,
	0x1910001f, 0x100125c8, 0x81200404, 0xe0c00004, 0x1910001f, 0x100125c8,
	0xa01d0400, 0xa01b0400, 0xa0180400, 0x803d8400, 0xa01e0400, 0xa0160400,
	0xa0170400, 0xa0168400, 0x1b80001f, 0x20000104, 0x81411801, 0xd8002f85,
	0x17c07c1f, 0x18c0001f, 0x10006240, 0xc0c03360, 0x17c07c1f, 0xe8208000,
	0x1000f600, 0xd2000001, 0xd8000768, 0x17c07c1f, 0xc2803780, 0x1291841f,
	0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0x1900001f, 0x10006830,
	0xe1000003, 0x18c0001f, 0x10006834, 0xe0e00000, 0xe0e00001, 0xf0000000,
	0x17c07c1f, 0xe0f07f16, 0x1380201f, 0xe0f07f1e, 0x1380201f, 0xe0f07f0e,
	0x1b80001f, 0x20000104, 0xe0f07f0c, 0xe0f07f0d, 0xe0f07e0d, 0xe0f07c0d,
	0xe0f0780d, 0xf0000000, 0xe0f0700d, 0xe0f07f0d, 0xe0f07f0f, 0xe0f07f1e,
	0xf0000000, 0xe0f07f12, 0x11407c1f, 0x81f08407, 0x81f18407, 0x1b80001f,
	0x20000001, 0xa1d08407, 0xa1d18407, 0x1392841f, 0x812ab401, 0x80ebb401,
	0xa0c00c04, 0xd8203603, 0x17c07c1f, 0x80c01403, 0xd8203423, 0x01400405,
	0x1900001f, 0x10006814, 0xf0000000, 0xe1000003, 0xa1d00407, 0x1b80001f,
	0x20000208, 0x80ea3401, 0x1a00001f, 0x10006814, 0xf0000000, 0xe2000003,
	0x18c0001f, 0x10006b6c, 0x1910001f, 0x10006b6c, 0xa1002804, 0xf0000000,
	0xe0c00004, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001, 0xa1d48407, 0x1990001f,
	0x10006b08, 0x1a50001f, 0x10006610, 0x8246a401, 0xe8208000, 0x10006b6c,
	0x00000000, 0x1b00001f, 0x2f7be75f, 0x81469801, 0xd8004305, 0x17c07c1f,
	0x1b80001f, 0xd00f0000, 0x8880000c, 0x2f7be75f, 0xd8005da2, 0x17c07c1f,
	0xd0004340, 0x17c07c1f, 0x1b80001f, 0x500f0000, 0xe8208000, 0x10006354,
	0xfffe7b47, 0xc0c06940, 0x81401801, 0xd80048e5, 0x17c07c1f, 0x81f60407,
	0x18c0001f, 0x10006200, 0xc0c05e60, 0x12807c1f, 0xe8208000, 0x1000625c,
	0x00000001, 0x1b80001f, 0x20000080, 0xc0c05e60, 0x1280041f, 0x18c0001f,
	0x10006204, 0xc0c06200, 0x1280041f, 0x18c0001f, 0x10006208, 0xc0c05e60,
	0x12807c1f, 0xe8208000, 0x10006244, 0x00000001, 0x1b80001f, 0x20000080,
	0xc0c05e60, 0x1280041f, 0x18d0001f, 0x10200200, 0x18c0001f, 0x10006290,
	0xc0c05e60, 0x1280041f, 0xe8208000, 0x10006404, 0x00003101, 0xc2803780,
	0x1292041f, 0x81469801, 0xd8204a45, 0x17c07c1f, 0x1b00001f, 0x2f7be75f,
	0x1b80001f, 0x30000004, 0x8880000c, 0x2f7be75f, 0xd8005802, 0x17c07c1f,
	0xc0c064c0, 0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0f07fff, 0xe0e00fff,
	0xe0e000ff, 0x81449801, 0xd8004c85, 0x17c07c1f, 0x1a00001f, 0x10006604,
	0xe2200003, 0xc0c06580, 0x17c07c1f, 0xe2200005, 0xc0c06580, 0x17c07c1f,
	0xa1d38407, 0xa1d98407, 0x1800001f, 0x00000012, 0x1800001f, 0x00000e12,
	0x1800001f, 0x03800e12, 0x1800001f, 0x038e0e12, 0xe8208000, 0x10006310,
	0x0b1600f8, 0x1b00001f, 0xbfffe7ff, 0x1b80001f, 0x90100000, 0x80c00400,
	0xd8204fa3, 0xa1d58407, 0xa1dd8407, 0x1b00001f, 0x3fffefff, 0xd0004e60,
	0x17c07c1f, 0x1890001f, 0x100063e8, 0x88c0000c, 0x2f7be75f, 0xd80051c3,
	0x17c07c1f, 0x80c40001, 0xd8005143, 0x17c07c1f, 0x1b00001f, 0xbfffe7ff,
	0xd0005180, 0x17c07c1f, 0x1b00001f, 0x7ffff7ff, 0xd0004e60, 0x17c07c1f,
	0x80c40001, 0xd82052c3, 0x17c07c1f, 0xa1de0407, 0x1b00001f, 0x7fffe7ff,
	0xd0004e60, 0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0e001fe, 0xe0e003fc,
	0xe0e007f8, 0xe0e00ff0, 0x1b80001f, 0x20000020, 0xe0f07ff0, 0xe0f07f00,
	0x81449801, 0xd80055a5, 0x17c07c1f, 0x1a00001f, 0x10006604, 0xe2200002,
	0xc0c06580, 0x17c07c1f, 0xe2200004, 0xc0c06580, 0x17c07c1f, 0x1b80001f,
	0x200016a8, 0x1800001f, 0x03800e12, 0x1b80001f, 0x20000300, 0x1800001f,
	0x00000e12, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000012, 0x1b80001f,
	0x20000104, 0x10007c1f, 0x81f38407, 0x81f98407, 0x81f90407, 0x81f40407,
	0x1b80001f, 0x200016a8, 0x81401801, 0xd8005da5, 0x17c07c1f, 0xe8208000,
	0x10006404, 0x00002101, 0x18c0001f, 0x10006290, 0x1212841f, 0xc0c05fe0,
	0x12807c1f, 0xc0c05fe0, 0x1280041f, 0x18c0001f, 0x10006208, 0x1212841f,
	0xc0c05fe0, 0x12807c1f, 0xe8208000, 0x10006244, 0x00000000, 0x1b80001f,
	0x20000080, 0xc0c05fe0, 0x1280041f, 0xe8208000, 0x10200268, 0x000ffffe,
	0x18c0001f, 0x10006204, 0x1212841f, 0xc0c06340, 0x1280041f, 0x18c0001f,
	0x10006200, 0x1212841f, 0xc0c05fe0, 0x12807c1f, 0xe8208000, 0x1000625c,
	0x00000000, 0x1b80001f, 0x20000080, 0xc0c05fe0, 0x1280041f, 0x19c0001f,
	0x01411820, 0x1ac0001f, 0x55aa55aa, 0x10007c1f, 0xf0000000, 0xd8005f0a,
	0x17c07c1f, 0xe2e0004f, 0xe2e0006f, 0xe2e0002f, 0xd8205faa, 0x17c07c1f,
	0xe2e0002e, 0xe2e0003e, 0xe2e00032, 0xf0000000, 0x17c07c1f, 0xd80060aa,
	0x17c07c1f, 0xe2e00036, 0xe2e0003e, 0x1380201f, 0xe2e0003c, 0xd82061ca,
	0x17c07c1f, 0x1380201f, 0xe2e0007c, 0x1b80001f, 0x20000003, 0xe2e0005c,
	0xe2e0004c, 0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xd8206309, 0x17c07c1f,
	0xe2e0000d, 0xe2e0000c, 0xe2e0001c, 0xe2e0001e, 0xe2e00016, 0xe2e00012,
	0xf0000000, 0x17c07c1f, 0xd8206489, 0x17c07c1f, 0xe2e00016, 0x1380201f,
	0xe2e0001e, 0x1380201f, 0xe2e0001c, 0x1380201f, 0xe2e0000c, 0xe2e0000d,
	0xf0000000, 0x17c07c1f, 0xa1d40407, 0x1391841f, 0xa1d90407, 0x1393041f,
	0xf0000000, 0x17c07c1f, 0x18d0001f, 0x10006604, 0x10cf8c1f, 0xd8206583,
	0x17c07c1f, 0xf0000000, 0x17c07c1f, 0xe8208000, 0x11008014, 0x00000002,
	0xe8208000, 0x11008020, 0x00000101, 0xe8208000, 0x11008004, 0x000000d0,
	0x1a00001f, 0x11008000, 0xd800684a, 0xe220005d, 0xd820686a, 0xe2200000,
	0xe2200001, 0xe8208000, 0x11008024, 0x00000001, 0x1b80001f, 0x20000424,
	0xf0000000, 0x17c07c1f, 0xa1d10407, 0x1b80001f, 0x20000020, 0xf0000000,
	0x17c07c1f
};

/*
 * PCM binary for suspend scenario
 */
static const struct pcm_desc suspend_pcm_ca7 = {
	.version = "pcm_suspend_20150805_V1",
	.base = suspend_binary_ca7,
	.size = 847,
	.sess = 2,
	.replace = 0,
	.vec0 = EVENT_VEC(11, 1, 0, 0),
	.vec1 = EVENT_VEC(12, 1, 0, 54),
	.vec2 = EVENT_VEC(30, 1, 0, 143),
	.vec3 = EVENT_VEC(31, 1, 0, 277),
};

/*
 * SPM settings for suspend scenario
 */
static struct pwr_ctrl spm_ctrl = {
	.wake_src = WAKE_SRC_FOR_SUSPEND,
	.wake_src_md32 = WAKE_SRC_FOR_MD32,
	.r0_ctrl_en = 1,
	.r7_ctrl_en = 1,
	.infra_dcm_lock = 1,
	.wfi_op = WFI_OP_AND,
	.pcm_apsrc_req = 0,
	.ca7top_idle_mask = 0,
	.ca15top_idle_mask = 0,
	.mcusys_idle_mask = 0,
	.disp_req_mask = 0,
	.mfg_req_mask = 0,
	.md32_req_mask = 1,
	.srclkenai_mask = 1,
	.ca7_wfi0_en = 1,
	.ca7_wfi1_en = 1,
	.ca7_wfi2_en = 1,
	.ca7_wfi3_en = 1,
	.ca15_wfi0_en = 1,
	.ca15_wfi1_en = 1,
	.ca15_wfi2_en = 1,
	.ca15_wfi3_en = 1,
};

/*
 * go_to_sleep_before_wfi() - trigger SPM to enter suspend scenario
 */
static void go_to_sleep_before_wfi(const unsigned int spm_flags)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = &spm_ctrl;

	set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

	spm_set_sysclk_settle();

	INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
	     pwrctrl->timer_val, pwrctrl->wake_src,
	     is_cpu_pdn(pwrctrl->pcm_flags),
	     is_infra_pdn(pwrctrl->pcm_flags));

	spm_reset_and_init_pcm();
	spm_init_pcm_register();
	spm_set_power_control(pwrctrl);
	spm_set_wakeup_event(pwrctrl);
	spm_kick_pcm_to_run(pwrctrl);
	spm_init_event_vector(&suspend_pcm_ca7);
	spm_kick_im_to_fetch(&suspend_pcm_ca7);
}

/*
 * go_to_sleep_after_wfi() - get wakeup reason after
 * leaving suspend scenario and clean up SPM settings
 */
static enum wake_reason_t go_to_sleep_after_wfi(void)
{
	struct wake_status wakesta;
	static enum wake_reason_t last_wr = WR_NONE;

	spm_get_wakeup_status(&wakesta);
	spm_clean_after_wakeup();
	last_wr = spm_output_wake_reason(&wakesta);

	return last_wr;
}

static void mt6311_control(unsigned int enable)
{
	unsigned int val;
	unsigned int slave_addr = 0x6b;
	unsigned int data_port = 0x8a;

	/* i2c1 clock on */
	mmio_write_32(PERI_GLOBALCON_PDN0_CLR, I2C1_PDN);
	mmio_write_32(I2C1_TRANSFER_LEN, 2);
	mmio_write_32(I2C1_TIMING, 1);
	mmio_write_32(I2C1_SLAVE_ADDR, (slave_addr << 1) | I2C_MASTER_WRITE);
	mmio_write_32(I2C1_INTR_MASK, I2C_ALLOW_ALL_INTR);
	mmio_write_32(I2C1_DATA_PORT, data_port);
	mmio_write_32(I2C1_DATA_PORT, enable);
	mmio_write_32(I2C1_START, I2C_START);

	while((val = mmio_read_32(I2C1_INTR_STAT) & I2C_MASK_TRANSAC_COMP) == 0)
	        ;
	mmio_write_32(I2C1_INTR_STAT, val);

	/* i2c1 clock off */
	mmio_setbits_32(PERI_GLOBALCON_PDN0_SET, I2C1_PDN);
}

void bigcore_pll_on(void)
{
	mmio_setbits_32(ARMCA15PLL_PWR_CON0, ARMCA15PLL_PWR_ON);
	mmio_clrbits_32(ARMCA15PLL_PWR_CON0, ARMCA15PLL_ISO_EN);
	mmio_setbits_32(ARMCA15PLL_CON0, ARMCA15PLL_EN);
}

void bigcore_pll_off(void)
{
	mmio_clrbits_32(ARMCA15PLL_CON0, ARMCA15PLL_EN);
	mmio_setbits_32(ARMCA15PLL_PWR_CON0, ARMCA15PLL_ISO_EN);
	mmio_clrbits_32(ARMCA15PLL_PWR_CON0, ARMCA15PLL_PWR_ON);
}

void spm_system_suspend(void)
{
	bigcore_pll_off();
	mt6311_control(0);
	spm_lock_get();
	go_to_sleep_before_wfi(spm_flags);
	set_suspend_ready();
	spm_lock_release();
}

void spm_system_suspend_finish(void)
{
	spm_lock_get();
	spm_wake_reason = go_to_sleep_after_wfi();
	INFO("spm_wake_reason=%d\n", spm_wake_reason);
	clear_all_ready();
	spm_lock_release();
	mt6311_control(1);
	/* Add 1ms delay for powering on mt6311 */
	mdelay(1);

	bigcore_pll_on();
	/* Add 20us delay for turning on PLL*/
	udelay(20);
}

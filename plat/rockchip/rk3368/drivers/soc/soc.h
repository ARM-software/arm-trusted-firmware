/*
 * Copyright (C) 2016, Fuzhou Rockchip Electronics Co., Ltd.
 * All rights reserved.
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

#ifndef __SOC_H__
#define __SOC_H__

#ifndef BIT
#define BIT(nr)			(1 << (nr))
#endif

/*****************************************************************************
 * secure timer
 *****************************************************************************/
#define TIMER_LOADE_COUNT0	0x00
#define TIMER_LOADE_COUNT1	0x04
#define TIMER_CURRENT_VALUE0	0x08
#define TIMER_CURRENT_VALUE1	0x0C
#define TIMER_CONTROL_REG	0x10
#define TIMER_INTSTATUS		0x18

#define TIMER_EN			0x1

#define STIMER1_BASE		(STIME_BASE + 0x20)

/*****************************************************************************
 * sgrf reg, offset
 *****************************************************************************/
#define SGRF_SOC_CON(n)		(0x0 + (n) * 4)
#define SGRF_BUSDMAC_CON(n)	(0x100 + (n) * 4)

#define SGRF_SOC_CON_NS		0xffff0000

/*
 * con6[2]pmusram is security.
 * con6[6]stimer is security.
 */
#define PMUSRAM_S_SHIFT		2
#define PMUSRAM_S		1
#define STIMER_S_SHIFT		6
#define STIMER_S			1
#define SGRF_SOC_CON7_BITS	((0xffff << 16) | \
				 (PMUSRAM_S << PMUSRAM_S_SHIFT) | \
				 (STIMER_S << STIMER_S_SHIFT))

#define SGRF_BUSDMAC_CON0_NS	0xfffcfff8
#define SGRF_BUSDMAC_CON1_NS	0xffff0fff

/*****************************************************************************
 * cru reg, offset
 *****************************************************************************/
#define CRU_SOFTRST_CON		0x300
#define CRU_SOFTRSTS_CON(n)	(CRU_SOFTRST_CON + ((n) * 4))

#define SOFTRST_DMA1	0x40004
#define SOFTRST_DMA2	0x10001

#define RST_DMA1_MSK	0x2
#define RST_DMA2_MSK	0x0

void platform_soc_init(void);
extern const unsigned char rockchip_power_domain_tree_desc[];

#endif /* __SOC_H__ */

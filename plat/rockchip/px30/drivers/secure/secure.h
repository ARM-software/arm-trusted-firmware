/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_H
#define SECURE_H

/***************************************************************************
 * SGRF
 ***************************************************************************/
#define SGRF_SOC_CON(i)		((i) * 0x4)
#define SGRF_DMAC_CON(i)	(0x30 + (i) * 0x4)

#define SGRF_MST_S_ALL_NS	0xffffffff
#define SGRF_SLV_S_ALL_NS	0xffff0000
#define DMA_IRQ_BOOT_NS		0xffffffff
#define DMA_PERI_CH_NS_15_0	0xffffffff
#define DMA_PERI_CH_NS_19_16	0x000f000f
#define DMA_MANAGER_BOOT_NS	0x00010001
#define DMA_SOFTRST_REQ		BITS_WITH_WMASK(1, 0x1, 12)
#define DMA_SOFTRST_RLS		BITS_WITH_WMASK(0, 0x1, 12)

/***************************************************************************
 * DDR FIREWALL
 ***************************************************************************/
#define FIREWALL_DDR_FW_DDR_RGN(i)	((i) * 0x4)
#define FIREWALL_DDR_FW_DDR_MST(i)	(0x20 + (i) * 0x4)
#define FIREWALL_DDR_FW_DDR_CON_REG	0x40
#define FIREWALL_DDR_FW_DDR_RGN_NUM	8
#define FIREWALL_DDR_FW_DDR_MST_NUM	6

#define PLAT_MAX_DDR_CAPACITY_MB	4096
#define RG_MAP_SECURE(top, base)	((((top) - 1) << 16) | (base))

/**************************************************
 * secure timer
 **************************************************/

/* chanal0~5 */
#define STIMER_CHN_BASE(n)	(STIME_BASE + 0x20 * (n))

#define TIMER_LOAD_COUNT0	0x0
#define TIMER_LOAD_COUNT1	0x4

#define TIMER_CUR_VALUE0	0x8
#define TIMER_CUR_VALUE1	0xc

#define TIMER_CONTROL_REG	0x10
#define TIMER_INTSTATUS		0x18

#define TIMER_DIS		0x0
#define TIMER_EN		0x1

#define TIMER_FMODE		(0x0 << 1)
#define TIMER_RMODE		(0x1 << 1)

#define TIMER_LOAD_COUNT0_MSK	(0xffffffff)
#define TIMER_LOAD_COUNT1_MSK	(0xffffffff00000000)

void secure_timer_init(void);
void sgrf_init(void);

#endif /* SECURE_H */

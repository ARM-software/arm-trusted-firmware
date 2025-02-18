/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtc.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <rtc.h>

#define GTC_CFG0_PUSHEVT       (0x08U)
#define GTC_CFG1_CNTCR         (0x00U)
#define GTC_CFG1_CNT_LO        (0x08U)
#define GTC_CFG1_CNT_HI        (0x0cU)
#define GTC_CFG1_CNTFID0       (0x20U)

#define GTC_CFG1_CNTCR_HDBG    BIT(1)
#define GTC_CFG1_CNTCR_EN      BIT(0)

static struct gtc_cfg_data {
	struct rtc_time suspend;
	struct rtc_time resume;
	uint32_t		pushevt;
	uint32_t		hdbg;
	uint32_t		cnt_lo;
	uint32_t		cnt_hi;
	uint32_t		fid0;
} gtc;

int32_t k3_lpm_sleep_suspend_gtc(void)
{
	uint32_t reg;

	/* disable GTC */
	reg = mmio_read_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNTCR);
	reg &= ~GTC_CFG1_CNTCR_EN;
	mmio_write_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNTCR, reg);

	lpm_rtc_read_time(&gtc.suspend);

	gtc.hdbg    = reg & GTC_CFG1_CNTCR_HDBG;
	gtc.pushevt = mmio_read_32(K3_GTC_CFG0_BASE + GTC_CFG0_PUSHEVT);
	gtc.cnt_lo  = mmio_read_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNT_LO);
	gtc.cnt_hi  = mmio_read_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNT_HI);
	gtc.fid0    = mmio_read_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNTFID0);

	return 0;
}

int32_t k3_lpm_resume_gtc(void)
{
	uint64_t rtc1, rtc2, delta;

	mmio_write_32(K3_GTC_CFG0_BASE + GTC_CFG0_PUSHEVT, gtc.pushevt);
	mmio_write_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNTFID0, gtc.fid0);

	lpm_rtc_read_time(&gtc.resume);

	rtc1 = (uint64_t)(((uint64_t)gtc.suspend.sec_hi << 32) + gtc.suspend.sec_lo);
	rtc2 = (uint64_t)(((uint64_t)gtc.resume.sec_hi << 32) + gtc.resume.sec_lo);
	delta = (rtc2 - rtc1) * gtc.fid0;
	delta += (uint64_t)(((uint64_t)gtc.cnt_hi << 32) + gtc.cnt_lo);
	gtc.cnt_hi = (uint32_t)(delta >> 32);
	gtc.cnt_lo = (uint32_t)delta;

	mmio_write_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNT_LO, gtc.cnt_lo);
	mmio_write_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNT_HI, gtc.cnt_hi);

	/* enable GTC */
	mmio_write_32(K3_GTC_CFG1_BASE + GTC_CFG1_CNTCR, gtc.hdbg | GTC_CFG1_CNTCR_EN);

	return 0;
}

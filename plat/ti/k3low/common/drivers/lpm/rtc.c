/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <rtc.h>

/* RTC register offsets */
#define RTC_SUB_S_CNT                                            (0x04)
#define RTC_S_CNT_LSW                                            (0x08)
#define RTC_S_CNT_MSW                                            (0x0C)

void k3low_lpm_rtc_read_time(struct rtc_time *rtc)
{
	if (rtc != NULL) {
		rtc->sub_sec = mmio_read_32(K3_RTC_BASE + RTC_SUB_S_CNT);
		rtc->sec_lo = mmio_read_32(K3_RTC_BASE + RTC_S_CNT_LSW);
		rtc->sec_hi = mmio_read_32(K3_RTC_BASE + RTC_S_CNT_MSW);
	}
}

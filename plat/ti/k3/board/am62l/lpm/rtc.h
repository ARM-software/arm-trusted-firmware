/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_RTC_H__
#define __LPM_RTC_H__

#include <plat/common/platform.h>

struct rtc_time {
	uint32_t	sub_sec;
	uint32_t	sec_lo;
	uint32_t	sec_hi;
};

/**
 *  \brief  Read RTC counter
 */
void lpm_rtc_read_time(struct rtc_time *rtc);

/**
 *  \brief  Initialize RTC
 */
void rtc_init(void);

/**
 *  \brief  Resume RTC after lower power mode exit
 */
void rtc_resume(void);

/**
 *  \brief  SUspend RTC for lower power mode entry
 */
void rtc_suspend(void);

#endif /* __LPM_RTC_H__ */

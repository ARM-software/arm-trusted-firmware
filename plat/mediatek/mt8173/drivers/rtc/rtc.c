/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>

#include <mt8173_def.h>
#include <rtc.h>

void rtc_bbpu_power_down(void)
{
	uint16_t bbpu;

	/* pull PWRBB low */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
	if (Writeif_unlock()) {
		RTC_Write(RTC_BBPU, bbpu);
		if (!RTC_Write_Trigger())
			assert(0);
	} else {
		assert(0);
	}
}

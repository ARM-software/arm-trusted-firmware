/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <mt8173_def.h>
#include <pmic_wrap_init.h>
#include <rtc.h>

/* RTC busy status polling interval and retry count */
enum {
	RTC_WRTGR_POLLING_DELAY_MS	= 10,
	RTC_WRTGR_POLLING_CNT		= 100
};

static uint16_t RTC_Read(uint32_t addr)
{
	uint32_t rdata = 0;

	pwrap_read((uint32_t)addr, &rdata);
	return (uint16_t)rdata;
}

static void RTC_Write(uint32_t addr, uint16_t data)
{
	pwrap_write((uint32_t)addr, (uint32_t)data);
}

static inline int32_t rtc_busy_wait(void)
{
	uint64_t retry = RTC_WRTGR_POLLING_CNT;

	do {
		mdelay(RTC_WRTGR_POLLING_DELAY_MS);
		if (!(RTC_Read(RTC_BBPU) & RTC_BBPU_CBUSY))
			return 1;
		retry--;
	} while (retry);

	ERROR("[RTC] rtc cbusy time out!\n");
	return 0;
}

static int32_t Write_trigger(void)
{
	RTC_Write(RTC_WRTGR, 1);
	return rtc_busy_wait();
}

static int32_t Writeif_unlock(void)
{
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK1);
	if (!Write_trigger())
		return 0;
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK2);
	if (!Write_trigger())
		return 0;

	return 1;
}

void rtc_bbpu_power_down(void)
{
	uint16_t bbpu;

	/* pull PWRBB low */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
	if (Writeif_unlock()) {
		RTC_Write(RTC_BBPU, bbpu);
		if (!Write_trigger())
			assert(0);
	} else {
		assert(0);
	}
}

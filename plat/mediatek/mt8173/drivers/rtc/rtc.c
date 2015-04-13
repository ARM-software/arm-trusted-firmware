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
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
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
			assert(1);
	} else {
		assert(1);
	}
}

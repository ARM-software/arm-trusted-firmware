/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
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

static void rtc_disable_2sec_reboot(void)
{
	uint16_t reboot;

	reboot = (RTC_Read(RTC_AL_SEC) & ~RTC_BBPU_2SEC_EN) &
		 ~RTC_BBPU_AUTO_PDN_SEL;
	RTC_Write(RTC_AL_SEC, reboot);
	Write_trigger();
}

void rtc_power_off_sequence(void)
{
	uint16_t bbpu;

	rtc_disable_2sec_reboot();

	/* clear alarm */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_CLR | RTC_BBPU_PWREN;
	if (Writeif_unlock()) {
		RTC_Write(RTC_BBPU, bbpu);

		RTC_Write(RTC_AL_MASK, RTC_AL_MASK_DOW);
		Write_trigger();
		mdelay(1);

		bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		RTC_Write(RTC_BBPU, bbpu);
		Write_trigger();
		INFO("[RTC] BBPU=0x%x, IRQ_EN=0x%x, AL_MSK=0x%x, AL_SEC=0x%x\n",
		     RTC_Read(RTC_BBPU), RTC_Read(RTC_IRQ_EN),
		     RTC_Read(RTC_AL_MASK), RTC_Read(RTC_AL_SEC));
	}
}

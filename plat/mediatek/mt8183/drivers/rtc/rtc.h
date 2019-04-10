/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RTC_H__
#define __RTC_H__

/* RTC registers */
enum {
	RTC_BBPU = 0x0588,
	RTC_IRQ_STA = 0x058A,
	RTC_IRQ_EN = 0x058C,
	RTC_CII_EN = 0x058E
};

enum {
	RTC_AL_SEC = 0x05A0,
	RTC_AL_MIN = 0x05A2,
	RTC_AL_HOU = 0x05A4,
	RTC_AL_DOM = 0x05A6,
	RTC_AL_DOW = 0x05A8,
	RTC_AL_MTH = 0x05AA,
	RTC_AL_YEA = 0x05AC,
	RTC_AL_MASK = 0x0590
};

enum {
	RTC_OSC32CON = 0x05AE,
	RTC_CON = 0x05C4,
	RTC_WRTGR = 0x05C2
};

enum {
	RTC_PDN1 = 0x05B4,
	RTC_PDN2 = 0x05B6,
	RTC_SPAR0 = 0x05B8,
	RTC_SPAR1 = 0x05BA,
	RTC_PROT = 0x05BC,
	RTC_DIFF = 0x05BE,
	RTC_CALI = 0x05C0
};

enum {
	RTC_PROT_UNLOCK1 = 0x586A,
	RTC_PROT_UNLOCK2 = 0x9136
};

enum {
	RTC_BBPU_PWREN	= 1U << 0,
	RTC_BBPU_CLR	= 1U << 1,
	RTC_BBPU_INIT	= 1U << 2,
	RTC_BBPU_AUTO	= 1U << 3,
	RTC_BBPU_CLRPKY	= 1U << 4,
	RTC_BBPU_RELOAD	= 1U << 5,
	RTC_BBPU_CBUSY	= 1U << 6
};

enum {
	RTC_AL_MASK_SEC = 1U << 0,
	RTC_AL_MASK_MIN = 1U << 1,
	RTC_AL_MASK_HOU = 1U << 2,
	RTC_AL_MASK_DOM = 1U << 3,
	RTC_AL_MASK_DOW = 1U << 4,
	RTC_AL_MASK_MTH = 1U << 5,
	RTC_AL_MASK_YEA = 1U << 6
};

enum {
	RTC_BBPU_AUTO_PDN_SEL = 1U << 6,
	RTC_BBPU_2SEC_CK_SEL = 1U << 7,
	RTC_BBPU_2SEC_EN = 1U << 8,
	RTC_BBPU_2SEC_MODE = 0x3 << 9,
	RTC_BBPU_2SEC_STAT_CLEAR = 1U << 11,
	RTC_BBPU_2SEC_STAT_STA = 1U << 12
};

enum {
	RTC_BBPU_KEY	= 0x43 << 8
};

/* external API */
void rtc_power_off_sequence(void);

#endif /* __RTC_H__ */

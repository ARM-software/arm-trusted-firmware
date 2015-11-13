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

#ifndef __PLAT_DRIVER_RTC_H__
#define __PLAT_DRIVER_RTC_H__

/* RTC registers */
enum {
	RTC_BBPU = 0xE000,
	RTC_IRQ_STA = 0xE002,
	RTC_IRQ_EN = 0xE004,
	RTC_CII_EN = 0xE006
};

enum {
	RTC_OSC32CON = 0xE026,
	RTC_CON = 0xE03E,
	RTC_WRTGR = 0xE03C
};

enum {
	RTC_PDN1 = 0xE02C,
	RTC_PDN2 = 0xE02E,
	RTC_SPAR0 = 0xE030,
	RTC_SPAR1 = 0xE032,
	RTC_PROT = 0xE036,
	RTC_DIFF = 0xE038,
	RTC_CALI = 0xE03A
};

enum {
	RTC_PROT_UNLOCK1 = 0x586A,
	RTC_PROT_UNLOCK2 = 0x9136
};

enum {
	RTC_BBPU_PWREN	= 1U << 0,
	RTC_BBPU_BBPU	= 1U << 2,
	RTC_BBPU_AUTO	= 1U << 3,
	RTC_BBPU_CLRPKY	= 1U << 4,
	RTC_BBPU_RELOAD	= 1U << 5,
	RTC_BBPU_CBUSY	= 1U << 6
};

enum {
	RTC_BBPU_KEY	= 0x43 << 8
};

void rtc_bbpu_power_down(void);

#endif /* __PLAT_DRIVER_RTC_H__ */

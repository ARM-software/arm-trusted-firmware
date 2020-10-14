/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <rtc.h>


static void RTC_Config_Interface(uint32_t addr, uint16_t data,
			    uint16_t mask, uint16_t shift)
{
	uint16_t pmic_reg;

	pmic_reg = RTC_Read(addr);

	pmic_reg &= ~(mask << shift);
	pmic_reg |= (data << shift);

	RTC_Write(addr, pmic_reg);
}

static int32_t rtc_disable_2sec_reboot(void)
{
	uint16_t reboot;

	reboot = (RTC_Read(RTC_AL_SEC) & ~RTC_BBPU_2SEC_EN) &
		 ~RTC_BBPU_AUTO_PDN_SEL;
	RTC_Write(RTC_AL_SEC, reboot);

	return RTC_Write_Trigger();
}

static int32_t rtc_enable_k_eosc(void)
{
	uint16_t alm_dow, alm_sec;
	int16_t ret;

	/* Turning on eosc cali mode clock */
	RTC_Config_Interface(PMIC_RG_SCK_TOP_CKPDN_CON0_CLR, 1,
			PMIC_RG_RTC_EOSC32_CK_PDN_MASK,
			PMIC_RG_RTC_EOSC32_CK_PDN_SHIFT);

	alm_sec = RTC_Read(RTC_AL_SEC) & (~RTC_LPD_OPT_MASK);
	RTC_Write(RTC_AL_SEC, alm_sec);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	RTC_Write(RTC_CON, RTC_LPD_EN);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	RTC_Write(RTC_CON, RTC_LPD_RST);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	RTC_Write(RTC_CON, RTC_LPD_EN);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	RTC_Write(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
	RTC_Write(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	/* set RTC EOSC calibration period = 8sec */
	alm_dow = (RTC_Read(RTC_AL_DOW) & (~RTC_RG_EOSC_CALI_TD_MASK)) |
		  RTC_RG_EOSC_CALI_TD_8SEC;
	RTC_Write(RTC_AL_DOW, alm_dow);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	RTC_Write(RTC_BBPU,
		  RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD);
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	/* Enable K EOSC mode :use solution1 of eosc cali to fix mt6359p 32K*/
	RTC_Write(RTC_AL_YEA, (((RTC_Read(RTC_AL_YEA) | RTC_K_EOSC_RSV_0)
				& (~RTC_K_EOSC_RSV_1)) | (RTC_K_EOSC_RSV_2)));
	ret = RTC_Write_Trigger();
	if (ret == 0) {
		return 0;
	}

	INFO("[RTC] RTC_enable_k_eosc\n");

	return 1;
}

void rtc_power_off_sequence(void)
{
	uint16_t bbpu;
	int16_t ret;

	ret = rtc_disable_2sec_reboot();
	if (ret == 0) {
		return;
	}

	ret = rtc_enable_k_eosc();
	if (ret == 0) {
		return;
	}

	bbpu = RTC_BBPU_KEY | RTC_BBPU_PWREN;

	if (Writeif_unlock() != 0) {
		RTC_Write(RTC_BBPU,
			  bbpu | RTC_BBPU_RESET_ALARM | RTC_BBPU_RESET_SPAR);
		RTC_Write(RTC_AL_MASK, RTC_AL_MASK_DOW);
		ret = RTC_Write_Trigger();
		if (ret == 0) {
			return;
		}
		mdelay(1);

		bbpu = RTC_Read(RTC_BBPU);

		if (((bbpu & RTC_BBPU_RESET_ALARM) > 0) ||
		    ((bbpu & RTC_BBPU_RESET_SPAR) > 0)) {
			INFO("[RTC] timeout\n");
		}

		bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		RTC_Write(RTC_BBPU, bbpu);
		ret = RTC_Write_Trigger();
		if (ret == 0) {
			return;
		}
	}
}

/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <mt8173_def.h>
#include <plat_sip_calls.h>
#include <lib/psci/psci.h>
#include <smccc_helpers.h>
#include <wdt.h>

#define WDT_BASE		(RGU_BASE + 0)
#define WDT_MODE		(WDT_BASE + 0x00)
#define WDT_LENGTH		(WDT_BASE + 0x04)
#define WDT_RESTART		(WDT_BASE + 0x08)
#define WDT_SWRST		(WDT_BASE + 0x14)

#define WDT_MODE_DUAL_MODE	0x40
#define WDT_MODE_IRQ		0x8
#define WDT_MODE_KEY		0x22000000
#define WDT_MODE_EXTEN		0x4
#define WDT_MODE_EN		0x1
#define WDT_LENGTH_KEY		0x8
#define WDT_RESTART_KEY		0x1971
#define WDT_SWRST_KEY		0x1209


#define WDT_MIN_TIMEOUT 1
#define WDT_MAX_TIMEOUT 31

enum smcwd_call {
	SMCWD_INFO		= 0,
	SMCWD_SET_TIMEOUT	= 1,
	SMCWD_ENABLE		= 2,
	SMCWD_PET		= 3,
};

static int wdt_enabled_before_suspend;

/*
 * We expect the WDT registers to be correctly initialized by BL2 firmware
 * (which may be board specific), so we do not reinitialize them here.
 */

void wdt_trigger_reset(void)
{
	mmio_write_32(WDT_SWRST, WDT_SWRST_KEY);
}

void wdt_pet(void)
{
	mmio_write_32(WDT_RESTART, WDT_RESTART_KEY);
}

int wdt_set_timeout(uint32_t timeout)
{
	/* One tick here equals 512 32KHz ticks. 512 / 32000 * 125 / 2 = 1 */
	uint32_t ticks = timeout * 125 / 2;

	if (timeout < WDT_MIN_TIMEOUT || timeout > WDT_MAX_TIMEOUT)
		return PSCI_E_INVALID_PARAMS;

	mmio_write_32(WDT_LENGTH, ticks << 5 | WDT_LENGTH_KEY);

	return PSCI_E_SUCCESS;
}

void wdt_set_enable(int enable)
{
	if (enable)
		wdt_pet();
	mmio_clrsetbits_32(WDT_MODE, WDT_MODE_EN,
			   WDT_MODE_KEY | (enable ? WDT_MODE_EN : 0));
}

void wdt_suspend(void)
{
	wdt_enabled_before_suspend = mmio_read_32(WDT_MODE) & WDT_MODE_EN;
	if (wdt_enabled_before_suspend)
		wdt_set_enable(0);
}

void wdt_resume(void)
{
	if (wdt_enabled_before_suspend)
		wdt_set_enable(1);
}

uint64_t wdt_smc_handler(uint32_t x1,
			uint32_t x2,
			void *handle)
{
	int ret;

	switch (x1) {
	case SMCWD_INFO:
		SMC_RET3(handle, PSCI_E_SUCCESS,
			 WDT_MIN_TIMEOUT, WDT_MAX_TIMEOUT);
	case SMCWD_SET_TIMEOUT:
		ret = wdt_set_timeout(x2);
		SMC_RET1(handle, ret);
	case SMCWD_ENABLE:
		wdt_set_enable(x2 > 0);
		SMC_RET1(handle, PSCI_E_SUCCESS);
	case SMCWD_PET:
		wdt_pet();
		SMC_RET1(handle, PSCI_E_SUCCESS);
	default:
		ERROR("Unimplemented SMCWD call (%d)\n", x1);
		SMC_RET1(handle, PSCI_E_NOT_SUPPORTED);
	}
}

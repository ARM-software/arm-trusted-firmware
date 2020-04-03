/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <cmn_sec.h>

#pragma weak plat_tz_master_default_cfg
#pragma weak plat_tz_sdio_ns_master_set
#pragma weak plat_tz_usb_ns_master_set

void plat_tz_master_default_cfg(void)
{
	/* This function should be implemented in the platform side. */
	ERROR("%s: TZ CONFIGURATION NOT SET!!!\n", __func__);
}

void plat_tz_sdio_ns_master_set(uint32_t ns)
{
	/* This function should be implemented in the platform side. */
	ERROR("%s: TZ CONFIGURATION NOT SET!!!\n", __func__);
}

void plat_tz_usb_ns_master_set(uint32_t ns)
{
	/* This function should be implemented in the platform side. */
	ERROR("%s: TZ CONFIGURATION NOT SET!!!\n", __func__);
}

void tz_master_default_cfg(void)
{
	plat_tz_master_default_cfg();
}

void tz_sdio_ns_master_set(uint32_t ns)
{
	plat_tz_sdio_ns_master_set(ns);
}

void tz_usb_ns_master_set(uint32_t ns)
{
	plat_tz_usb_ns_master_set(ns);
}

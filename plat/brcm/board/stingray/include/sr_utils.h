/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SR_UTILS_H
#define SR_UTILS_H

#include <lib/mmio.h>

#include <chip_id.h>
#include <cmn_plat_util.h>
#include <sr_def.h>

static inline void brcm_stingray_set_qspi_mux(int enable_ap)
{
	mmio_write_32(QSPI_HOLD_N_MODE_SEL_CONTROL, enable_ap);
	mmio_write_32(QSPI_WP_N_MODE_SEL_CONTROL, enable_ap);
	mmio_write_32(QSPI_SCK_MODE_SEL_CONTROL, enable_ap);
	mmio_write_32(QSPI_CS_N_MODE_SEL_CONTROL, enable_ap);
	mmio_write_32(QSPI_MOSI_MODE_SEL_CONTROL, enable_ap);
	mmio_write_32(QSPI_MISO_MODE_SEL_CONTROL, enable_ap);
}

static inline void brcm_stingray_set_straps(uint32_t boot_source)
{
	/* Enable software strap override */
	mmio_setbits_32(CDRU_CHIP_STRAP_CTRL,
			BIT(CDRU_CHIP_STRAP_CTRL__SOFTWARE_OVERRIDE));

	/* set straps to the next boot source */
	mmio_clrsetbits_32(CDRU_CHIP_STRAP_DATA,
			   BOOT_SOURCE_MASK,
			   boot_source);

	/* Disable software strap override */
	mmio_clrbits_32(CDRU_CHIP_STRAP_CTRL,
			BIT(CDRU_CHIP_STRAP_CTRL__SOFTWARE_OVERRIDE));
}

#endif

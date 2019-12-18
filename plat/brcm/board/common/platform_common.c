/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/brcm/sotp.h>

#include <cmn_plat_util.h>
#include <platform_def.h>

uint32_t boot_source_get(void)
{
	uint32_t data;

#ifdef FORCE_BOOTSOURCE
	data = FORCE_BOOTSOURCE;
#else
	/* Read primary boot strap from CRMU persistent registers */
	data = mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG1);
	if (data & BOOT_SOURCE_SOFT_ENABLE_MASK) {
		data >>= BOOT_SOURCE_SOFT_DATA_OFFSET;
	} else {
		uint64_t sotp_atf_row;

		sotp_atf_row =
			sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC);

		if (sotp_atf_row & SOTP_BOOT_SOURCE_ENABLE_MASK) {
			/* Construct the boot source based on SOTP bits */
			data = 0;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS0)
				data |= 0x1;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS1)
				data |= 0x2;
			if (sotp_atf_row & SOTP_BOOT_SOURCE_BITS2)
				data |= 0x4;
		} else {

			/*
			 * This path is for L0 reset with
			 * Primary Boot source disabled in SOTP.
			 * BOOT_SOURCE_FROM_PR_ON_L1 compile flag will allow
			 * to never come back here so that the
			 * external straps will not be read on L1 reset.
			 */

			/* Use the external straps */
			data = mmio_read_32(ROM_S0_IDM_IO_STATUS);

#ifdef BOOT_SOURCE_FROM_PR_ON_L1
			/* Enable boot source read from PR#1 */
			mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
				BOOT_SOURCE_SOFT_ENABLE_MASK);

			/* set boot source */
			data &= BOOT_SOURCE_MASK;
			mmio_clrsetbits_32(CRMU_IHOST_SW_PERSISTENT_REG1,
			BOOT_SOURCE_MASK << BOOT_SOURCE_SOFT_DATA_OFFSET,
			data << BOOT_SOURCE_SOFT_DATA_OFFSET);
#endif
		}
	}
#endif
	return (data & BOOT_SOURCE_MASK);
}

void __dead2 plat_soft_reset(uint32_t reset)
{
	if (reset == SOFT_RESET_L3) {
		mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1, reset);
		mmio_write_32(CRMU_MAIL_BOX0, 0x0);
		mmio_write_32(CRMU_MAIL_BOX1, 0xFFFFFFFF);
	}

	if (reset != SOFT_SYS_RESET_L1)
		reset = SOFT_PWR_UP_RESET_L0;

	if (reset == SOFT_PWR_UP_RESET_L0)
		INFO("L0 RESET...\n");

	if (reset == SOFT_SYS_RESET_L1)
		INFO("L1 RESET...\n");

	console_flush();

	mmio_clrbits_32(CRMU_SOFT_RESET_CTRL, 1 << reset);

	while (1) {
		;
	}
}

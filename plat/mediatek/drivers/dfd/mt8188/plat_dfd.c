/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <dfd.h>
#include <plat_dfd.h>

static uint64_t dfd_cache_dump;
static bool dfd_enabled;
static uint64_t dfd_base_addr;
static uint64_t dfd_chain_length;

void dfd_setup(uint64_t base_addr, uint64_t chain_length, uint64_t cache_dump)
{
	mmio_write_32(MTK_DRM_LATCH_CTL1, MTK_DRM_LATCH_CTL1_VAL);
	mmio_write_32(MTK_DRM_LATCH_CTL2, MTK_DRM_LATCH_CTL2_VAL);
	mmio_write_32(MTK_WDT_LATCH_CTL2, MTK_WDT_LATCH_CTL2_VAL);

	mmio_clrbits_32(DFD_O_INTRF_MCU_PWR_CTL_MASK, BIT(2));
	mmio_setbits_32(DFD_V50_GROUP_0_63_DIFF, 0x1);
	sync_writel(DFD_INTERNAL_CTL, 0x5);
	mmio_setbits_32(DFD_INTERNAL_CTL, BIT(13));
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x1F << 3);
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x3 << 9);
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x3 << 19);

	mmio_write_32(DFD_INTERNAL_PWR_ON, 0xB);
	mmio_write_32(DFD_CHAIN_LENGTH0, chain_length);
	mmio_write_32(DFD_INTERNAL_SHIFT_CLK_RATIO, 0x0);
	mmio_write_32(DFD_INTERNAL_TEST_SO_OVER_64, 0x1);

	mmio_write_32(DFD_TEST_SI_0, 0x0);
	mmio_write_32(DFD_TEST_SI_1, 0x0);
	mmio_write_32(DFD_TEST_SI_2, 0x0);
	mmio_write_32(DFD_TEST_SI_3, 0x0);

	sync_writel(DFD_POWER_CTL, 0xF9);
	sync_writel(DFD_READ_ADDR, DFD_READ_ADDR_VAL);
	sync_writel(DFD_V30_CTL, 0xD);

	mmio_write_32(DFD_O_SET_BASEADDR_REG, base_addr >> 24);
	mmio_write_32(DFD_O_REG_0, 0);

	/* setup global variables for suspend and resume */
	dfd_enabled = true;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;
	dfd_cache_dump = cache_dump;

	if ((cache_dump & DFD_CACHE_DUMP_ENABLE) != 0UL) {
		mmio_write_32(MTK_DRM_LATCH_CTL2, MTK_DRM_LATCH_CTL2_CACHE_VAL);
		sync_writel(DFD_V35_ENABLE, 0x1);
		sync_writel(DFD_V35_TAP_NUMBER, 0xB);
		sync_writel(DFD_V35_TAP_EN, DFD_V35_TAP_EN_VAL);
		sync_writel(DFD_V35_SEQ0_0, DFD_V35_SEQ0_0_VAL);

		/* Cache dump only mode */
		sync_writel(DFD_V35_CTL, 0x1);
		mmio_write_32(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 0xF);
		mmio_write_32(DFD_CHAIN_LENGTH0, DFD_CHAIN_LENGTH_VAL);
		mmio_write_32(DFD_CHAIN_LENGTH1, DFD_CHAIN_LENGTH_VAL);
		mmio_write_32(DFD_CHAIN_LENGTH2, DFD_CHAIN_LENGTH_VAL);
		mmio_write_32(DFD_CHAIN_LENGTH3, DFD_CHAIN_LENGTH_VAL);

		if ((cache_dump & DFD_PARITY_ERR_TRIGGER) != 0UL) {
			sync_writel(DFD_HW_TRIGGER_MASK, 0xC);
			mmio_setbits_32(DFD_INTERNAL_CTL, 0x1 << 4);
		}
	}
	dsbsy();
}

void dfd_resume(void)
{
	if (dfd_enabled == true) {
		dfd_setup(dfd_base_addr, dfd_chain_length, dfd_cache_dump);
	}
}

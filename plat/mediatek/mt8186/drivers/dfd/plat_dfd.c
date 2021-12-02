/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mtk_sip_svc.h>
#include <plat_dfd.h>

static bool dfd_enabled;
static uint64_t dfd_base_addr;
static uint64_t dfd_chain_length;
static uint64_t dfd_cache_dump;

static void dfd_setup(uint64_t base_addr, uint64_t chain_length,
		      uint64_t cache_dump)
{
	mmio_write_32(MCUSYS_DFD_MAP, base_addr >> 24);
	mmio_write_32(WDT_DEBUG_CTL, WDT_DEBUG_CTL_VAL_0);

	sync_writel(DFD_INTERNAL_CTL, (BIT(0) | BIT(2)));

	mmio_setbits_32(DFD_INTERNAL_CTL, BIT(13));
	mmio_setbits_32(DFD_INTERNAL_CTL, BIT(3));
	mmio_setbits_32(DFD_INTERNAL_CTL, (BIT(19) | BIT(20)));
	mmio_write_32(DFD_INTERNAL_PWR_ON, (BIT(0) | BIT(1) | BIT(3)));
	mmio_write_32(DFD_CHAIN_LENGTH0, chain_length);
	mmio_write_32(DFD_INTERNAL_SHIFT_CLK_RATIO, 0);
	mmio_write_32(DFD_INTERNAL_TEST_SO_0, DFD_INTERNAL_TEST_SO_0_VAL);
	mmio_write_32(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 1);

	mmio_write_32(DFD_TEST_SI_0, DFD_TEST_SI_0_VAL);
	mmio_write_32(DFD_TEST_SI_1, DFD_TEST_SI_1_VAL);

	sync_writel(DFD_V30_CTL, 1);

	mmio_write_32(DFD_V30_BASE_ADDR, (base_addr & 0xFFF00000));

	/* setup global variables for suspend and resume */
	dfd_enabled = true;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;
	dfd_cache_dump = cache_dump;

	if ((cache_dump & DFD_CACHE_DUMP_ENABLE) != 0UL) {
		mmio_write_32(WDT_DEBUG_CTL, WDT_DEBUG_CTL_VAL_1);
		sync_writel(DFD_V35_ENALBE, 1);
		sync_writel(DFD_V35_TAP_NUMBER, DFD_V35_TAP_NUMBER_VAL);
		sync_writel(DFD_V35_TAP_EN, DFD_V35_TAP_EN_VAL);
		sync_writel(DFD_V35_SEQ0_0, DFD_V35_SEQ0_0_VAL);

		if (cache_dump & DFD_PARITY_ERR_TRIGGER) {
			sync_writel(DFD_HW_TRIGGER_MASK, DFD_HW_TRIGGER_MASK_VAL);
			mmio_setbits_32(DFD_INTERNAL_CTL, BIT(4));
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

uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			    uint64_t arg2, uint64_t arg3)
{
	uint64_t ret = 0L;

	switch (arg0) {
	case PLAT_MTK_DFD_SETUP_MAGIC:
		INFO("[%s] DFD setup call from kernel\n", __func__);
		dfd_setup(arg1, arg2, arg3);
		break;
	case PLAT_MTK_DFD_READ_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			ret = mmio_read_32(MISC1_CFG_BASE + arg1);
		}
		break;
	case PLAT_MTK_DFD_WRITE_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			sync_writel(MISC1_CFG_BASE + arg1, arg2);
		}
		break;
	default:
		ret = MTK_SIP_E_INVALID_PARAM;
		break;
	}

	return ret;
}

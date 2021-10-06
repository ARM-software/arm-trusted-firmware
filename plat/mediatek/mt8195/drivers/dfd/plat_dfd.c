/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
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
	mmio_write_32(MTK_WDT_LATCH_CTL2, MTK_WDT_LATCH_CTL2_VAL);
	mmio_write_32(MTK_WDT_INTERVAL, MTK_WDT_INTERVAL_VAL);
	mmio_write_32(MTK_DRM_LATCH_CTL2, MTK_DRM_LATCH_CTL2_VAL);
	mmio_write_32(MTK_DRM_LATCH_CTL1, MTK_DRM_LATCH_CTL1_VAL);

	/* Bit[2] = 0 (default=1), disable dfd apb bus protect_en */
	mmio_clrbits_32(DFD_O_INTRF_MCU_PWR_CTL_MASK, 0x1 << 2);

	/* Bit[0] : enable?mcusys_vproc?external_off?dfd?trigger -> 1 */
	mmio_setbits_32(DFD_V50_GROUP_0_63_DIFF, 0x1);

	/* bit[0]: rg_rw_dfd_internal_dump_en -> 1 */
	/* bit[2]: rg_rw_dfd_clock_stop_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x5);

	/* bit[13]: xreset_b_update_disable */
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x1 << 13);

	/*
	 * bit[10:3]: DFD trigger selection mask
	 * bit[3]: rg_rw_dfd_trigger_sel[0] = 1(enable wdt trigger)
	 * bit[4]: rg_rw_dfd_trigger_sel[1] = 1(enable HW trigger)
	 * bit[5]: rg_rw_dfd_trigger_sel[2] = 1(enable SW trigger)
	 * bit[6]: rg_rw_dfd_trigger_sel[3] = 1(enable SW non-security trigger)
	 * bit[7]: rg_rw_dfd_trigger_sel[4] = 1(enable timer trigger)
	 */
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x1F << 3);

	/*
	 * bit[9]  : rg_rw_dfd_trigger_sel[6] = 1(cpu_eb_sw_dfd_trigger)
	 * bit[10] : rg_rw_dfd_trigger_sel[7] = 1(cpu_eb_wdt_dfd_trigger)
	 */
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x3 << 9);

	/* bit[20:19]: rg_dfd_armpll_div_mux_sel switch to PLL2 for DFD */
	mmio_setbits_32(DFD_INTERNAL_CTL, 0x3 << 19);

	/*
	 * bit[0]: rg_rw_dfd_auto_power_on = 1
	 * bit[2:1]: rg_rw_dfd_auto_power_on_dely = 1(10us)
	 * bit[4:2]: rg_rw_dfd_power_on_wait_time = 1(20us)
	 */
	mmio_write_32(DFD_INTERNAL_PWR_ON, 0xB);

	/* longest scan chain length */
	mmio_write_32(DFD_CHAIN_LENGTH0, chain_length);

	/* bit[1:0]: rg_rw_dfd_shift_clock_ratio */
	mmio_write_32(DFD_INTERNAL_SHIFT_CLK_RATIO, 0x0);

	/* rg_dfd_test_so_over_64 */
	mmio_write_32(DFD_INTERNAL_TEST_SO_OVER_64, 0x1);

	/* DFD3.0 */
	mmio_write_32(DFD_TEST_SI_0, 0x0);
	mmio_write_32(DFD_TEST_SI_1, 0x0);
	mmio_write_32(DFD_TEST_SI_2, 0x0);
	mmio_write_32(DFD_TEST_SI_3, 0x0);

	/* for iLDO feature */
	sync_writel(DFD_POWER_CTL, 0xF9);

	/* read offset */
	sync_writel(DFD_READ_ADDR, DFD_READ_ADDR_VAL);

	/* for DFD-3.0 setup */
	sync_writel(DFD_V30_CTL, 0xD);

	/* set base address */
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

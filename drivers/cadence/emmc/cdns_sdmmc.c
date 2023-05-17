/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/delay_timer.h>
#include <drivers/mmc.h>
#include <lib/mmio.h>
#include <lib/utils.h>

/* Card busy and present */
#define CARD_BUSY					1
#define CARD_NOT_BUSY					0

/* 500 ms delay to read the RINST register */
#define DELAY_MS_SRS_READ				500
#define DELAY_RES					10

/* SRS12 error mask */
#define SRS12_ERR_MASK					0xFFFF8000

/* Check DV dfi_init val=0 */
#define IO_MASK_END_DATA				0x0

/* Check DV dfi_init val=2; DDR Mode */
#define IO_MASK_END_DATA_DDR				0x2
#define IO_MASK_START_DATA				0x0
#define DATA_SELECT_OE_END_DATA				0x1

#define TIMEOUT						100000

/* General define */
#define SDHC_REG_MASK					UINT_MAX
#define SD_HOST_BLOCK_SIZE				0x200
#define DTCVVAL_DEFAULT_VAL				0xE
#define CDMMC_DMA_MAX_BUFFER_SIZE			64*1024
#define CDNSMMC_ADDRESS_MASK				U(0x0f)
#define CONFIG_CDNS_DESC_COUNT				8

void cdns_init(void);
int cdns_send_cmd(struct mmc_cmd *cmd);
int cdns_set_ios(unsigned int clk, unsigned int width);
int cdns_prepare(int lba, uintptr_t buf, size_t size);
int cdns_read(int lba, uintptr_t buf, size_t size);
int cdns_write(int lba, uintptr_t buf, size_t size);

const struct mmc_ops cdns_sdmmc_ops = {
	.init			= cdns_init,
	.send_cmd		= cdns_send_cmd,
	.set_ios		= cdns_set_ios,
	.prepare		= cdns_prepare,
	.read			= cdns_read,
	.write			= cdns_write,
};

struct cdns_sdmmc_params cdns_params;
struct cdns_sdmmc_combo_phy sdmmc_combo_phy_reg;
struct cdns_sdmmc_sdhc sdmmc_sdhc_reg;
#ifdef CONFIG_DMA_ADDR_T_64BIT
struct cdns_idmac_desc cdns_desc[CONFIG_CDNS_DESC_COUNT];
#else
struct cdns_idmac_desc cdns_desc[CONFIG_CDNS_DESC_COUNT] __aligned(32);
#endif

bool data_cmd;

int cdns_wait_ics(uint16_t timeout, uint32_t cdn_srs_res)
{
	/* Clock for sdmclk and sdclk */
	uint32_t count = 0;
	uint32_t data = 0;

	/* Wait status command response ready */
	do {
		data = mmio_read_32(cdn_srs_res);
		count++;
		if (count >= timeout) {
			return -ETIMEDOUT;
		}
	} while ((data & (1 << SDMMC_CDN_ICS)) == 0);

	return 0;
}

int cdns_busy(void)
{
	unsigned int data;

	data = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS09);
	return (data & STATUS_DATA_BUSY) ? CARD_BUSY : CARD_NOT_BUSY;
}

int cdns_vol_reset(void)
{
	/* Reset embedded card */
	mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), (7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP));
	udelay(250);
	mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), (7 << SDMMC_CDN_BVS) | (0 << SDMMC_CDN_BP));
	udelay(500);

	/* Turn on supply voltage */
	/* BVS = 7, BP = 1, BP2 only in UHS2 mode */
	mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), (7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP));
	udelay(250);
	return 0;
}

void cdns_set_sdmmc_var(struct cdns_sdmmc_combo_phy *combo_phy_reg,
	struct cdns_sdmmc_sdhc *sdhc_reg)
{
	/* Values are taken by the reference of cadence IP documents */
	combo_phy_reg->cp_clk_wr_delay = 0;
	combo_phy_reg->cp_clk_wrdqs_delay = 0;
	combo_phy_reg->cp_data_select_oe_end = 0;
	combo_phy_reg->cp_dll_bypass_mode = 1;
	combo_phy_reg->cp_dll_locked_mode = 0;
	combo_phy_reg->cp_dll_start_point = 0;
	combo_phy_reg->cp_gate_cfg_always_on = 1;
	combo_phy_reg->cp_io_mask_always_on = 0;
	combo_phy_reg->cp_io_mask_end = 0;
	combo_phy_reg->cp_io_mask_start = 0;
	combo_phy_reg->cp_rd_del_sel = 52;
	combo_phy_reg->cp_read_dqs_cmd_delay = 0;
	combo_phy_reg->cp_read_dqs_delay = 0;
	combo_phy_reg->cp_sw_half_cycle_shift = 0;
	combo_phy_reg->cp_sync_method = 1;
	combo_phy_reg->cp_underrun_suppress = 1;
	combo_phy_reg->cp_use_ext_lpbk_dqs = 1;
	combo_phy_reg->cp_use_lpbk_dqs = 1;
	combo_phy_reg->cp_use_phony_dqs = 1;
	combo_phy_reg->cp_use_phony_dqs_cmd = 1;

	sdhc_reg->sdhc_extended_rd_mode = 1;
	sdhc_reg->sdhc_extended_wr_mode = 1;
	sdhc_reg->sdhc_hcsdclkadj = 0;
	sdhc_reg->sdhc_idelay_val = 0;
	sdhc_reg->sdhc_rdcmd_en = 1;
	sdhc_reg->sdhc_rddata_en = 1;
	sdhc_reg->sdhc_rw_compensate = 9;
	sdhc_reg->sdhc_sdcfsh = 0;
	sdhc_reg->sdhc_sdcfsl = 1;
	sdhc_reg->sdhc_wrcmd0_dly = 1;
	sdhc_reg->sdhc_wrcmd0_sdclk_dly = 0;
	sdhc_reg->sdhc_wrcmd1_dly = 0;
	sdhc_reg->sdhc_wrcmd1_sdclk_dly = 0;
	sdhc_reg->sdhc_wrdata0_dly = 1;
	sdhc_reg->sdhc_wrdata0_sdclk_dly = 0;
	sdhc_reg->sdhc_wrdata1_dly = 0;
	sdhc_reg->sdhc_wrdata1_sdclk_dly = 0;
}

static int cdns_program_phy_reg(struct cdns_sdmmc_combo_phy *combo_phy_reg,
	struct cdns_sdmmc_sdhc *sdhc_reg)
{
	uint32_t value = 0;
	int ret = 0;

	/* program PHY_DQS_TIMING_REG */
	value = (CP_USE_EXT_LPBK_DQS(combo_phy_reg->cp_use_ext_lpbk_dqs)) |
		(CP_USE_LPBK_DQS(combo_phy_reg->cp_use_lpbk_dqs)) |
		(CP_USE_PHONY_DQS(combo_phy_reg->cp_use_phony_dqs)) |
		(CP_USE_PHONY_DQS_CMD(combo_phy_reg->cp_use_phony_dqs_cmd));
	ret = cdns_sdmmc_write_phy_reg(MMC_REG_BASE + SDHC_CDNS_HRS04,
			COMBO_PHY_REG + PHY_DQS_TIMING_REG, MMC_REG_BASE +
			SDHC_CDNS_HRS05, value);
	if (ret != 0) {
		return ret;
	}

	/* program PHY_GATE_LPBK_CTRL_REG */
	value = (CP_SYNC_METHOD(combo_phy_reg->cp_sync_method)) |
		(CP_SW_HALF_CYCLE_SHIFT(combo_phy_reg->cp_sw_half_cycle_shift)) |
		(CP_RD_DEL_SEL(combo_phy_reg->cp_rd_del_sel)) |
		(CP_UNDERRUN_SUPPRESS(combo_phy_reg->cp_underrun_suppress)) |
		(CP_GATE_CFG_ALWAYS_ON(combo_phy_reg->cp_gate_cfg_always_on));
	ret = cdns_sdmmc_write_phy_reg(MMC_REG_BASE + SDHC_CDNS_HRS04,
			COMBO_PHY_REG + PHY_GATE_LPBK_CTRL_REG, MMC_REG_BASE +
			SDHC_CDNS_HRS05, value);
	if (ret != 0) {
		return ret;
	}

	/* program PHY_DLL_MASTER_CTRL_REG */
	value = (CP_DLL_BYPASS_MODE(combo_phy_reg->cp_dll_bypass_mode))
			| (CP_DLL_START_POINT(combo_phy_reg->cp_dll_start_point));
	ret = cdns_sdmmc_write_phy_reg(MMC_REG_BASE + SDHC_CDNS_HRS04,
			COMBO_PHY_REG + PHY_DLL_MASTER_CTRL_REG, MMC_REG_BASE
			+ SDHC_CDNS_HRS05, value);
	if (ret != 0) {
		return ret;
	}

	/* program PHY_DLL_SLAVE_CTRL_REG */
	value = (CP_READ_DQS_CMD_DELAY(combo_phy_reg->cp_read_dqs_cmd_delay))
		| (CP_CLK_WRDQS_DELAY(combo_phy_reg->cp_clk_wrdqs_delay))
		| (CP_CLK_WR_DELAY(combo_phy_reg->cp_clk_wr_delay))
		| (CP_READ_DQS_DELAY(combo_phy_reg->cp_read_dqs_delay));
	ret = cdns_sdmmc_write_phy_reg(MMC_REG_BASE + SDHC_CDNS_HRS04,
			COMBO_PHY_REG + PHY_DLL_SLAVE_CTRL_REG, MMC_REG_BASE
			+ SDHC_CDNS_HRS05, value);
	if (ret != 0) {
		return ret;
	}

	/* program PHY_CTRL_REG */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS04, COMBO_PHY_REG
			+ PHY_CTRL_REG);
	value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS05);

	/* phony_dqs_timing=0 */
	value &= ~(CP_PHONY_DQS_TIMING_MASK << CP_PHONY_DQS_TIMING_SHIFT);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS05, value);

	/* switch off DLL_RESET */
	do {
		value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09);
		value |= SDHC_PHY_SW_RESET;
		mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, value);
		value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09);
	/* polling PHY_INIT_COMPLETE */
	} while ((value & SDHC_PHY_INIT_COMPLETE) != SDHC_PHY_INIT_COMPLETE);

	/* program PHY_DQ_TIMING_REG */
	combo_phy_reg->cp_io_mask_end = 0U;
	value = (CP_IO_MASK_ALWAYS_ON(combo_phy_reg->cp_io_mask_always_on))
		| (CP_IO_MASK_END(combo_phy_reg->cp_io_mask_end))
		| (CP_IO_MASK_START(combo_phy_reg->cp_io_mask_start))
		| (CP_DATA_SELECT_OE_END(combo_phy_reg->cp_data_select_oe_end));

	ret = cdns_sdmmc_write_phy_reg(MMC_REG_BASE + SDHC_CDNS_HRS04,
			COMBO_PHY_REG + PHY_DQ_TIMING_REG, MMC_REG_BASE
			+ SDHC_CDNS_HRS05, value);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

int cdns_read(int lba, uintptr_t buf, size_t size)
{
	inv_dcache_range(buf, size);

	return 0;
}

void cdns_init(void)
{
	/* Dummy function pointer for cdns_init. */
}

int cdns_prepare(int dma_start_addr, uintptr_t dma_buff, size_t size)
{
	data_cmd = true;
	struct cdns_idmac_desc *desc;
	uint32_t desc_cnt, i;
	uint64_t desc_base;

	assert(((dma_buff & CDNSMMC_ADDRESS_MASK) == 0) &&
			(cdns_params.desc_size > 0) &&
			((MMC_REG_BASE & MMC_BLOCK_MASK) == 0) &&
			((cdns_params.desc_base & MMC_BLOCK_MASK) == 0) &&
			((cdns_params.desc_size & MMC_BLOCK_MASK) == 0));

	flush_dcache_range(dma_buff, size);

	desc_cnt = (size + (CDMMC_DMA_MAX_BUFFER_SIZE) - 1) / (CDMMC_DMA_MAX_BUFFER_SIZE);
	assert(desc_cnt * sizeof(struct cdns_idmac_desc) < cdns_params.desc_size);

	if (desc_cnt > CONFIG_CDNS_DESC_COUNT) {
		ERROR("Requested data transfer length %ld is greater than configured length %d",
				size, (CONFIG_CDNS_DESC_COUNT * CDMMC_DMA_MAX_BUFFER_SIZE));
		return -EINVAL;
	}

	desc = (struct cdns_idmac_desc *)cdns_params.desc_base;
	desc_base = (uint64_t)desc;
	i = 0;

	while ((i + 1) < desc_cnt) {
		desc->attr = ADMA_DESC_ATTR_VALID | ADMA_DESC_TRANSFER_DATA;
		desc->reserved = 0;
		desc->len = MAX_64KB_PAGE;
		desc->addr_lo = (dma_buff & UINT_MAX) + (CDMMC_DMA_MAX_BUFFER_SIZE * i);
#if CONFIG_DMA_ADDR_T_64BIT == 1
		desc->addr_hi = (dma_buff >> 32) & 0xffffffff;
#endif
		size -= CDMMC_DMA_MAX_BUFFER_SIZE;
		desc++;
		i++;
	}

	desc->attr = ADMA_DESC_ATTR_VALID | ADMA_DESC_TRANSFER_DATA |
			ADMA_DESC_ATTR_END;
	desc->reserved = 0;
	desc->len = size;
#if CONFIG_DMA_ADDR_T_64BIT == 1
	desc->addr_lo = (dma_buff & UINT_MAX) + (CDMMC_DMA_MAX_BUFFER_SIZE * i);
	desc->addr_hi = (dma_buff >> 32) & UINT_MAX;
#else
	desc->addr_lo = (dma_buff & UINT_MAX);
#endif

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS22, (uint32_t)desc_base);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS23, (uint32_t)(desc_base >> 32));
	flush_dcache_range(cdns_params.desc_base,
				desc_cnt * CDMMC_DMA_MAX_BUFFER_SIZE);

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS01,
			((512 << BLOCK_SIZE) | ((size/512) << BLK_COUNT_CT) | SDMA_BUF));
	return 0;
}

static void cdns_host_set_clk(int clk)
{
	uint32_t ret = 0;
	uint32_t sdclkfsval = 0;
	uint32_t dtcvval = DTCVVAL_DEFAULT_VAL;

	sdclkfsval = (cdns_params.clk_rate / 2000) / clk;
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, 0);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, (dtcvval << SDMMC_CDN_DTCV) |
			(sdclkfsval << SDMMC_CDN_SDCLKFS) | (1 << SDMMC_CDN_ICE));

	ret = cdns_wait_ics(5000, MMC_REG_BASE + SDHC_CDNS_SRS11);
	if (ret != 0U) {
		ERROR("Waiting SDMMC_CDN_ICS timeout");
	}

	/* Enable DLL reset */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09) &
			~SDHC_DLL_RESET_MASK);
	/* Set extended_wr_mode */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, (mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09)
			& SDHC_EXTENDED_WR_MODE_MASK) | (1 << EXTENDED_WR_MODE));
	/* Release DLL reset */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, mmio_read_32(MMC_REG_BASE
			+ SDHC_CDNS_HRS09) | 1);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, mmio_read_32(MMC_REG_BASE
			+ SDHC_CDNS_HRS09) | (3 << RDCMD_EN));

	do {
		mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09);
	} while (~mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09) & (1 << 1));

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, (dtcvval << SDMMC_CDN_DTCV) |
	(sdclkfsval << SDMMC_CDN_SDCLKFS) | (1 << SDMMC_CDN_ICE) | (1 << SDMMC_CDN_SDCE));
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS13, UINT_MAX);
}

int cdns_set_ios(unsigned int clk, unsigned int width)
{

	switch (width) {
	case MMC_BUS_WIDTH_1:
		mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), LEDC_OFF);
		break;
	case MMC_BUS_WIDTH_4:
		mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), DTW_4BIT);
		break;
	case MMC_BUS_WIDTH_8:
		mmio_write_32((MMC_REG_BASE + SDHC_CDNS_SRS10), EDTW_8BIT);
		break;
	default:
		assert(0);
		break;
	}
	cdns_host_set_clk(clk);

	return 0;
}

int cdns_sdmmc_write_sd_host_reg(uint32_t addr, uint32_t data)
{
	uint32_t value = 0;

	value = mmio_read_32(addr);
	value &= ~SDHC_REG_MASK;
	value |= data;
	mmio_write_32(addr, value);
	value = mmio_read_32(addr);
	if (value != data) {
		ERROR("SD host address is not set properly\n");
		return -ENXIO;
	}

	return 0;
}

int cdns_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int cdns_init_hrs_io(struct cdns_sdmmc_combo_phy *combo_phy_reg,
	struct cdns_sdmmc_sdhc *sdhc_reg)
{
	uint32_t value = 0;
	int ret = 0;

	/* program HRS09, register 42 */
	value = (SDHC_RDDATA_EN(sdhc_reg->sdhc_rddata_en))
		| (SDHC_RDCMD_EN(sdhc_reg->sdhc_rdcmd_en))
		| (SDHC_EXTENDED_WR_MODE(sdhc_reg->sdhc_extended_wr_mode))
		| (SDHC_EXTENDED_RD_MODE(sdhc_reg->sdhc_extended_rd_mode));
	ret = cdns_sdmmc_write_sd_host_reg(MMC_REG_BASE + SDHC_CDNS_HRS09, value);
	if (ret != 0) {
		ERROR("Program HRS09 failed");
		return ret;
	}

	/* program HRS10, register 43 */
	value = (SDHC_HCSDCLKADJ(sdhc_reg->sdhc_hcsdclkadj));
	ret = cdns_sdmmc_write_sd_host_reg(MMC_REG_BASE + SDHC_CDNS_HRS10, value);
	if (ret != 0) {
		ERROR("Program HRS10 failed");
		return ret;
	}

	/* program HRS16, register 48 */
	value = (SDHC_WRDATA1_SDCLK_DLY(sdhc_reg->sdhc_wrdata1_sdclk_dly))
		| (SDHC_WRDATA0_SDCLK_DLY(sdhc_reg->sdhc_wrdata0_sdclk_dly))
		| (SDHC_WRCMD1_SDCLK_DLY(sdhc_reg->sdhc_wrcmd1_sdclk_dly))
		| (SDHC_WRCMD0_SDCLK_DLY(sdhc_reg->sdhc_wrcmd0_sdclk_dly))
		| (SDHC_WRDATA1_DLY(sdhc_reg->sdhc_wrdata1_dly))
		| (SDHC_WRDATA0_DLY(sdhc_reg->sdhc_wrdata0_dly))
		| (SDHC_WRCMD1_DLY(sdhc_reg->sdhc_wrcmd1_dly))
		| (SDHC_WRCMD0_DLY(sdhc_reg->sdhc_wrcmd0_dly));
	ret = cdns_sdmmc_write_sd_host_reg(MMC_REG_BASE + SDHC_CDNS_HRS16, value);
	if (ret != 0) {
		ERROR("Program HRS16 failed");
		return ret;
	}

	/* program HRS07, register 40 */
	value = (SDHC_RW_COMPENSATE(sdhc_reg->sdhc_rw_compensate))
		| (SDHC_IDELAY_VAL(sdhc_reg->sdhc_idelay_val));
	ret = cdns_sdmmc_write_sd_host_reg(MMC_REG_BASE + SDHC_CDNS_HRS07, value);
	if (ret != 0) {
		ERROR("Program HRS07 failed");
		return ret;
	}

	return ret;
}

static int cdns_hc_set_clk(struct cdns_sdmmc_params *cdn_sdmmc_dev_mode_params)
{
	uint32_t ret = 0;
	uint32_t dtcvval, sdclkfsval;

	dtcvval = DTC_VAL;
	sdclkfsval = 0;

	if ((cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_DS) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_UHS_SDR12) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_SDR_BC)) {
		sdclkfsval = 4;
	} else if ((cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_HS) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_UHS_SDR25) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_UHS_DDR50) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_SDR)) {
		sdclkfsval = 2;
	} else if ((cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_UHS_SDR50) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_DDR) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_HS400) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_HS400es)) {
		sdclkfsval = 1;
	} else if ((cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == SD_UHS_SDR104) ||
		(cdn_sdmmc_dev_mode_params->cdn_sdmmc_dev_mode == EMMC_HS200)) {
		sdclkfsval = 0;
	}

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, 0);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, (dtcvval << SDMMC_CDN_DTCV) |
		(sdclkfsval << SDMMC_CDN_SDCLKFS) | (1 << SDMMC_CDN_ICE));
	ret = cdns_wait_ics(5000, MMC_REG_BASE + SDHC_CDNS_SRS11);
	if (ret != 0U) {
		ERROR("Waiting SDMMC_CDN_ICS timeout");
		return ret;
	}

	/* Enable DLL reset */
	mmio_write_32((MMC_REG_BASE + SDHC_CDNS_HRS09), mmio_read_32(MMC_REG_BASE
			+ SDHC_CDNS_HRS09) & ~SDHC_DLL_RESET_MASK);
	/* Set extended_wr_mode */
	mmio_write_32((MMC_REG_BASE + SDHC_CDNS_HRS09),
	(mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09) &	SDHC_EXTENDED_WR_MODE_MASK) |
			(1 << EXTENDED_WR_MODE));
	/* Release DLL reset */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, mmio_read_32(MMC_REG_BASE
			+ SDHC_CDNS_HRS09) | 1);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, mmio_read_32(MMC_REG_BASE
			+ SDHC_CDNS_HRS09) | (3 << RDCMD_EN));
	do {
		mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09);
	} while (~mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09) & (1 << 1));

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, (dtcvval << SDMMC_CDN_DTCV) |
		(sdclkfsval << SDMMC_CDN_SDCLKFS) | (1 << SDMMC_CDN_ICE) | (1 << SDMMC_CDN_SDCE));

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS13, UINT_MAX);
	return 0;
}

int cdns_reset(void)
{
	uint32_t data = 0;
	uint32_t count = 0;
	uint32_t value = 0;

	value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS11);
	value &= ~(0xFFFF);
	value |= 0x0;
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, value);
	udelay(500);

	/* Software reset */
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS00, 1);
	/* Wait status command response ready */
	do {
		data = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS00);
		count++;
		if (count >= 5000) {
			return -ETIMEDOUT;
		}
	/* Wait for HRS00.SWR */
	} while ((data & 1) == 1);

	/* Step 1, switch on DLL_RESET */
	value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_HRS09);
	value &= ~SDHC_PHY_SW_RESET;
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_HRS09, value);

	return 0;
}

int cdns_sd_host_init(struct cdns_sdmmc_combo_phy *mmc_combo_phy_reg,
struct cdns_sdmmc_sdhc *mmc_sdhc_reg)
{
	int ret = 0;

	ret = cdns_reset();
	if (ret != 0) {
		ERROR("Program phy reg init failed");
		return ret;
	}

	ret = cdns_program_phy_reg(&sdmmc_combo_phy_reg, &sdmmc_sdhc_reg);
	if (ret != 0) {
		ERROR("Program phy reg init failed");
		return ret;
	}

	ret = cdns_init_hrs_io(&sdmmc_combo_phy_reg, &sdmmc_sdhc_reg);
	if (ret != 0) {
		ERROR("Program init for HRS reg is failed");
		return ret;
	}

	ret = cdns_sd_card_detect();
	if (ret != 0) {
		ERROR("SD card does not detect");
		return ret;
	}

	ret = cdns_vol_reset();
	if (ret != 0) {
		ERROR("eMMC card reset failed");
		return ret;
	}

	ret = cdns_hc_set_clk(&cdns_params);
	if (ret != 0) {
		ERROR("hc set clk failed");
		return ret;
	}

	return 0;
}

void cdns_srs10_value_toggle(uint8_t write_val, uint8_t prev_val)
{
	uint32_t data_op = 0U;

	data_op = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS10);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS10, (data_op & (prev_val << 0)));
	mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS10);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS10, data_op | (write_val << 0));
}

void cdns_srs11_srs15_config(uint32_t srs11_val, uint32_t srs15_val)
{
	uint32_t data = 0U;

	data = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS11);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS11, (data | srs11_val));
	data = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS15);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS15, (data | srs15_val));
}

int cdns_send_cmd(struct mmc_cmd *cmd)
{
	uint32_t op = 0, ret = 0;
	uint8_t write_value = 0, prev_val = 0;
	uint32_t value;
	int32_t timeout;
	uint32_t cmd_indx;
	uint32_t status = 0, srs15_val = 0, srs11_val = 0;
	uint32_t status_check = 0;

	assert(cmd);
	cmd_indx = (cmd->cmd_idx) << COM_IDX;

	if (data_cmd) {
		switch (cmd->cmd_idx) {
		case SD_SWITCH:
			op = DATA_PRESENT;
			write_value = ADMA2_32 | DT_WIDTH;
			prev_val = ADMA2_32 | DT_WIDTH;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			srs15_val = BIT_AD_64 | HV4E | V18SE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		case SD_WRITE_SINGLE_BLOCK:
		case SD_READ_SINGLE_BLOCK:
			op = DATA_PRESENT;
			write_value = ADMA2_32 | HS_EN | DT_WIDTH | LEDC;
			prev_val = ADMA2_32 | HS_EN | DT_WIDTH;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = PVE | BIT_AD_64 | HV4E | SDR104_MODE | V18SE;
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS00, SAAR);
			break;

		case SD_WRITE_MULTIPLE_BLOCK:
		case SD_READ_MULTIPLE_BLOCK:
			op = DATA_PRESENT | AUTO_CMD_EN | MULTI_BLK_READ;
			write_value = ADMA2_32 | HS_EN | DT_WIDTH | LEDC;
			prev_val = ADMA2_32 | HS_EN | DT_WIDTH;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = PVE | BIT_AD_64 | HV4E | SDR104_MODE | V18SE;
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS00, SAAR);
			break;

		case SD_APP_SEND_SCR:
			op = DATA_PRESENT;
			write_value = ADMA2_32 | LEDC;
			prev_val = LEDC;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = BIT_AD_64 | HV4E | V18SE;
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		case SD_SEND_IF_COND:
			op = DATA_PRESENT | CMD_IDX_CHK_ENABLE;
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = HV4E;
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		default:
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			op = 0;
			break;
		}
	} else {
		switch (cmd->cmd_idx) {
		case SD_GO_IDLE_STATE:
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = HV4E;
			srs11_val = SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		case SD_ALL_SEND_CID:
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = HV4E | V18SE;
			srs11_val = SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		case SD_SEND_IF_COND:
			op = CMD_IDX_CHK_ENABLE;
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			srs15_val = HV4E;
			srs11_val = READ_CLK | SDMMC_CDN_ICE | SDMMC_CDN_ICS | SDMMC_CDN_SDCE;
			cdns_srs11_srs15_config(srs11_val, srs15_val);
			break;

		case SD_STOP_TRANSMISSION:
			op = CMD_STOP_ABORT_CMD;
			break;

		case SD_SEND_STATUS:
			break;

		case 1:
			cmd->cmd_arg = 0;
			break;

		case SD_SELECT_CARD:
			op = MULTI_BLK_READ;
			break;

		case SD_APP_CMD:
		default:
			write_value = LEDC;
			prev_val = 0x0;
			cdns_srs10_value_toggle(write_value, prev_val);
			op = 0;
			break;
		}
	}

	switch (cmd->resp_type) {
	case MMC_RESPONSE_NONE:
		op |= CMD_READ | MULTI_BLK_READ | DMA_ENABLED | BLK_CNT_EN;
		break;

	case MMC_RESPONSE_R2:
		op |= CMD_READ | MULTI_BLK_READ | DMA_ENABLED | BLK_CNT_EN |
			RES_TYPE_SEL_136 | CMD_CHECK_RESP_CRC;
		break;

	case MMC_RESPONSE_R3:
		op |= CMD_READ | MULTI_BLK_READ | DMA_ENABLED | BLK_CNT_EN |
			RES_TYPE_SEL_48;
		break;

	case MMC_RESPONSE_R1:
		if ((cmd->cmd_idx == SD_WRITE_SINGLE_BLOCK) || (cmd->cmd_idx
			== SD_WRITE_MULTIPLE_BLOCK)) {
			op |= DMA_ENABLED | BLK_CNT_EN | RES_TYPE_SEL_48
			| CMD_CHECK_RESP_CRC | CMD_IDX_CHK_ENABLE;
		} else {
			op |= DMA_ENABLED | BLK_CNT_EN | CMD_READ | RES_TYPE_SEL_48
			| CMD_CHECK_RESP_CRC | CMD_IDX_CHK_ENABLE;
		}
		break;

	default:
		op |= DMA_ENABLED | BLK_CNT_EN | CMD_READ | MULTI_BLK_READ |
			RES_TYPE_SEL_48 | CMD_CHECK_RESP_CRC | CMD_IDX_CHK_ENABLE;
		break;
	}

	timeout = TIMEOUT;
	do {
		udelay(100);
		ret = cdns_busy();
		if (--timeout <= 0) {
			udelay(50);
			panic();
		}
	} while (ret);

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS12, UINT_MAX);

	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS02, cmd->cmd_arg);
	mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS14, 0x00000000);
	if (cmd_indx == 1)
		mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS03, SDHC_CDNS_SRS03_VALUE);
	else
		mmio_write_32(MMC_REG_BASE + SDHC_CDNS_SRS03, op | cmd_indx);

	timeout = TIMEOUT;
	do {
		udelay(500);
		value = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS12);
	} while (((value & (INT_CMD_DONE | ERROR_INT)) == 0) && (timeout-- > 0));

	timeout = TIMEOUT;

	if (data_cmd) {
		data_cmd = false;
		do {
			udelay(250);
		} while (((value & TRAN_COMP) == 0) && (timeout-- > 0));
	}

	status_check = value & SRS12_ERR_MASK;
	if (status_check != 0U) {
		ERROR("SD host controller send command failed, SRS12 = %x", status);
		return -1;
	}

	if ((op & RES_TYPE_SEL_48) || (op & RES_TYPE_SEL_136)) {
		cmd->resp_data[0] = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS04);
		if (op & RES_TYPE_SEL_136) {
			cmd->resp_data[1] = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS05);
			cmd->resp_data[2] = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS06);
			cmd->resp_data[3] = mmio_read_32(MMC_REG_BASE + SDHC_CDNS_SRS07);
		}
	}

	return 0;
}

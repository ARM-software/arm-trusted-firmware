/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
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
void sd_host_adma_prepare(struct cdns_idmac_desc *desc_ptr, uintptr_t buf,
			  size_t size);
struct cdns_sdmmc_params cdns_params;
struct cdns_sdmmc_combo_phy sdmmc_combo_phy_reg;
struct cdns_sdmmc_sdhc sdmmc_sdhc_reg;
struct cdns_idmac_desc cdns_desc[CONFIG_CDNS_DESC_COUNT] __aligned(8);

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

		/* delay 300us to prevent CPU polling too frequently */
		udelay(300);
		if (count >= timeout) {
			return -ETIMEDOUT;
		}
	} while ((data & (1 << SDMMC_CDN_ICS)) == 0);

	return 0;
}

void cdns_set_sdmmc_var(struct cdns_sdmmc_combo_phy *combo_phy_reg,
			struct cdns_sdmmc_sdhc *sdhc_reg)
{
	/* Values are taken by the reference of cadence IP documents */
	combo_phy_reg->cp_clk_wr_delay = 0;
	combo_phy_reg->cp_clk_wrdqs_delay = 0;
	combo_phy_reg->cp_data_select_oe_end = 1;
	combo_phy_reg->cp_dll_bypass_mode = 1;
	combo_phy_reg->cp_dll_locked_mode = 0;
	combo_phy_reg->cp_dll_start_point = 254;
	combo_phy_reg->cp_gate_cfg_always_on = 1;
	combo_phy_reg->cp_io_mask_always_on = 0;
	combo_phy_reg->cp_io_mask_end = 5;
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
	sdhc_reg->sdhc_hcsdclkadj = 3;
	sdhc_reg->sdhc_idelay_val = 0;
	sdhc_reg->sdhc_rdcmd_en = 1;
	sdhc_reg->sdhc_rddata_en = 1;
	sdhc_reg->sdhc_rw_compensate = 10;
	sdhc_reg->sdhc_sdcfsh = 0;
	sdhc_reg->sdhc_sdcfsl = 0;
	sdhc_reg->sdhc_wrcmd0_dly = 1;
	sdhc_reg->sdhc_wrcmd0_sdclk_dly = 0;
	sdhc_reg->sdhc_wrcmd1_dly = 0;
	sdhc_reg->sdhc_wrcmd1_sdclk_dly = 0;
	sdhc_reg->sdhc_wrdata0_dly = 0;
	sdhc_reg->sdhc_wrdata0_sdclk_dly = 0;
	sdhc_reg->sdhc_wrdata1_dly = 0;
	sdhc_reg->sdhc_wrdata1_sdclk_dly = 0;
}

int cdns_program_phy_reg(struct cdns_sdmmc_combo_phy *combo_phy_reg,
				struct cdns_sdmmc_sdhc *sdhc_reg)
{
	uint32_t value = 0;
	int ret = 0;
	uint32_t timeout = 0;

	/* HRS00 - Software Reset */
	mmio_write_32((cdns_params.reg_base + SDHC_CDNS_HRS00), SDHC_CDNS_HRS00_SWR);

	/* Waiting for SDHC_CDNS_HRS00_SWR reset */
	timeout = TIMEOUT;
	do {
		udelay(250);
		if (--timeout <= 0) {
			NOTICE(" SDHC Software Reset failed!!!\n");
			panic();
		}
	} while (((mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS00) &
		SDHC_CDNS_HRS00_SWR) == 1));

	/* Step 1, switch on DLL_RESET */
	value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09);
	value &= ~SDHC_PHY_SW_RESET;
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09, value);

	/* program PHY_DQS_TIMING_REG */
	value = (CP_USE_EXT_LPBK_DQS(combo_phy_reg->cp_use_ext_lpbk_dqs)) |
		(CP_USE_LPBK_DQS(combo_phy_reg->cp_use_lpbk_dqs)) |
		(CP_USE_PHONY_DQS(combo_phy_reg->cp_use_phony_dqs)) |
		(CP_USE_PHONY_DQS_CMD(combo_phy_reg->cp_use_phony_dqs_cmd));
	ret = cdns_sdmmc_write_phy_reg(cdns_params.reg_base + SDHC_CDNS_HRS04,
					COMBO_PHY_REG + PHY_DQS_TIMING_REG,
					cdns_params.reg_base + SDHC_CDNS_HRS05, value);
	if (ret != 0U) {
		return ret;
	}

	/* program PHY_GATE_LPBK_CTRL_REG */
	value = (CP_SYNC_METHOD(combo_phy_reg->cp_sync_method)) |
		(CP_SW_HALF_CYCLE_SHIFT(combo_phy_reg->cp_sw_half_cycle_shift)) |
		(CP_RD_DEL_SEL(combo_phy_reg->cp_rd_del_sel)) |
		(CP_UNDERRUN_SUPPRESS(combo_phy_reg->cp_underrun_suppress)) |
		(CP_GATE_CFG_ALWAYS_ON(combo_phy_reg->cp_gate_cfg_always_on));
	ret = cdns_sdmmc_write_phy_reg(cdns_params.reg_base + SDHC_CDNS_HRS04,
				 COMBO_PHY_REG + PHY_GATE_LPBK_CTRL_REG,
				 cdns_params.reg_base + SDHC_CDNS_HRS05, value);
	if (ret != 0U) {
		return -ret;
	}

	/* program PHY_DLL_MASTER_CTRL_REG */
	value = (CP_DLL_BYPASS_MODE(combo_phy_reg->cp_dll_bypass_mode)) | (2 << 20) |
		(CP_DLL_START_POINT(combo_phy_reg->cp_dll_start_point));
	ret = cdns_sdmmc_write_phy_reg(cdns_params.reg_base + SDHC_CDNS_HRS04,
					COMBO_PHY_REG + PHY_DLL_MASTER_CTRL_REG,
					cdns_params.reg_base + SDHC_CDNS_HRS05, value);
	if (ret != 0U) {
		return ret;
	}

	/* program PHY_DLL_SLAVE_CTRL_REG */
	value = (CP_READ_DQS_CMD_DELAY(combo_phy_reg->cp_read_dqs_cmd_delay)) |
		(CP_CLK_WRDQS_DELAY(combo_phy_reg->cp_clk_wrdqs_delay)) |
		(CP_CLK_WR_DELAY(combo_phy_reg->cp_clk_wr_delay)) |
		(CP_READ_DQS_DELAY(combo_phy_reg->cp_read_dqs_delay));
	ret = cdns_sdmmc_write_phy_reg(cdns_params.reg_base + SDHC_CDNS_HRS04,
					COMBO_PHY_REG + PHY_DLL_SLAVE_CTRL_REG,
					cdns_params.reg_base + SDHC_CDNS_HRS05, value);
	if (ret != 0U) {
		return ret;
	}

	/* program PHY_CTRL_REG */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS04, COMBO_PHY_REG + PHY_CTRL_REG);
	value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS05);

	/* phony_dqs_timing=0 */
	value &= ~(CP_PHONY_DQS_TIMING_MASK << CP_PHONY_DQS_TIMING_SHIFT);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS05, value);

	/* switch off DLL_RESET */
	do {
		value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09);
		value |= SDHC_PHY_SW_RESET;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09, value);
		value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09);
	/* polling PHY_INIT_COMPLETE */
	} while ((value & SDHC_PHY_INIT_COMPLETE) != SDHC_PHY_INIT_COMPLETE);

	/* program PHY_DQ_TIMING_REG */
	value = (CP_IO_MASK_ALWAYS_ON(combo_phy_reg->cp_io_mask_always_on)) |
		(CP_IO_MASK_END(combo_phy_reg->cp_io_mask_end)) |
		(CP_IO_MASK_START(combo_phy_reg->cp_io_mask_start)) |
		(CP_DATA_SELECT_OE_END(combo_phy_reg->cp_data_select_oe_end));

	ret = cdns_sdmmc_write_phy_reg(cdns_params.reg_base + SDHC_CDNS_HRS04,
				 COMBO_PHY_REG + PHY_DQ_TIMING_REG,
				 cdns_params.reg_base + SDHC_CDNS_HRS05, value);
	if (ret != 0U) {
		return ret;
	}

	value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09);
	value |= (HRS_09_EXTENDED_RD_MODE | HRS_09_EXTENDED_WR_MODE |
		HRS_09_RDCMD_EN | HRS_09_RDDATA_EN);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09, value);

	value = 0;
	value = SDHC_HCSDCLKADJ(HRS_10_HCSDCLKADJ_VAL);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS10, value);

	value = 0;
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS16, value);

	value = (10 << 16);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS07, value);

	return 0;
}

int cdns_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

int cdns_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

void cdns_init(void)
{
	/* Dummy function pointer for cdns_init. */
}

int cdns_prepare(int dma_start_addr, uintptr_t dma_buff, size_t size)
{
	struct cdns_idmac_desc *cdns_desc_data;
	assert(((dma_buff & CDNSMMC_ADDRESS_MASK) == 0) &&
	 (cdns_params.desc_size > 0));

	cdns_desc_data = (struct cdns_idmac_desc *)cdns_params.desc_base;
	sd_host_adma_prepare(cdns_desc_data, dma_buff, size);

	return 0;
}

void cdns_host_set_clk(uint32_t clk)
{
	uint32_t ret = 0;
	uint32_t sdclkfsval = 0;
	uint32_t dtcvval = 0xE;

	sdclkfsval = (cdns_params.sdmclk / 2) / clk;
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS11, 0);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS11,
			(dtcvval << SDMMC_CDN_DTCV) | (sdclkfsval << SDMMC_CDN_SDCLKFS) |
			(1 << SDMMC_CDN_ICE));

	ret = cdns_wait_ics(5000, cdns_params.reg_base + SDHC_CDNS_SRS11);
	if (ret != 0) {
		ERROR("Waiting ICS timeout");
	}
	/* Enable DLL reset */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09,
		mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09) & ~0x00000001);
	/* Set extended_wr_mode */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09,
		(mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09) & 0xFFFFFFF7) |
			(1 << EXTENDED_WR_MODE));
	/* Release DLL reset */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09,
		mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09) | PHY_SW_RESET_EN);
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_HRS09,
		mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09) | RDCMD_EN);

	do {
		mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09);
	} while (~mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS09) &
		(PHY_INIT_COMPLETE_BIT));

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS11, (dtcvval << SDMMC_CDN_DTCV) |
			(sdclkfsval << SDMMC_CDN_SDCLKFS) | (1 << SDMMC_CDN_ICE) |
			(1 << SDMMC_CDN_SDCE));

	ret = cdns_wait_ics(5000, cdns_params.reg_base + SDHC_CDNS_SRS11);
	if (ret != 0)
		ERROR("Waiting ICS timeout");

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS13, 0xFFFFFFFF);
}

int cdns_set_ios(unsigned int clk, unsigned int width)
{
	uint32_t _status = 0;

	_status = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
	switch (width) {
	case MMC_BUS_WIDTH_1:
		_status &= ~(BIT4);
		break;

	case MMC_BUS_WIDTH_4:
		_status |= BIT4;
		break;

	case MMC_BUS_WIDTH_8:
		_status |= BIT8;
		break;

	default:
		assert(0);
		break;
	}
	mmio_write_32((cdns_params.reg_base + SDHC_CDNS_SRS10), _status);
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



void sd_host_oper_mode(enum sd_opr_modes opr_mode)
{

	uint32_t reg = 0;

	switch (opr_mode) {
	case SD_HOST_OPR_MODE_HV4E_0_SDMA_32:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg &= ~(HV4E | BIT_AD_64);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_1_SDMA_32:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg &= ~(HV4E | BIT_AD_64);
		reg |= (HV4E);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_1_SDMA_64:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg |= (HV4E | BIT_AD_64);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_0_ADMA_32:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		reg |= DMA_SEL_BIT_2;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg &= ~(HV4E | BIT_AD_64);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_0_ADMA_64:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		reg |= DMA_SEL_BIT_3;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg &= ~(HV4E | BIT_AD_64);
		reg |= BIT_AD_64;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_1_ADMA_32:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		reg |= DMA_SEL_BIT_2;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg &= ~(HV4E | BIT_AD_64);
		reg |= HV4E;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;

	case SD_HOST_OPR_MODE_HV4E_1_ADMA_64:
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);
		reg &= ~(DMA_SEL_BIT);
		reg |= DMA_SEL_BIT_2;
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg);
		reg = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS15);
		reg |= (HV4E | BIT_AD_64);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS15, reg);
		break;
	}
}

void card_reset(bool power_enable)
{
	uint32_t reg_value = 0;

	/* Reading SRS10 value before writing */
	reg_value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);

	if (power_enable == true) {
		reg_value &= ~((7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP));
		reg_value = ((1 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP));
	} else {
		reg_value &= ~((7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP));
	}
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg_value);
}

void high_speed_enable(bool mode)
{

	uint32_t reg_value = 0;
	/* Reading SRS10 value before writing */
	reg_value = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS10);

	if (mode == true) {
		reg_value |= HS_EN;
	} else {
		reg_value &= ~HS_EN;
	}

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS10, reg_value);
}

int cdns_reset(void)
{
	volatile uint32_t data = 0;
	uint32_t count = 0;

	/* Software reset */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS11, SRS11_SRFA);
	/* Wait status command response ready */
	do {
		data = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_HRS00);
		count++;
		if (count >= CDNS_TIMEOUT) {
			return -ETIMEDOUT;
		}
	/* Wait for SRS11 */
	} while (((SRS11_SRFA_CHK(data)) & 1) == 1);

	return 0;
}

void sdmmc_host_init(bool uhs2_enable)
{
	uint32_t timeout;

	/* SRS11 - Host Control  default value set */
	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS11, 0x0);

	/* Waiting for detect card */
	timeout = TIMEOUT;
	do {
		udelay(250);
		if (--timeout <= 0) {
			NOTICE(" SDHC Card Detecion failed!!!\n");
			panic();
		}
	} while (((mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS09) & CHECK_CARD) == 0));

	/* UHS2 Host setting */
	if (uhs2_enable == true) {
	/** need to implement*/
	}

	/* Card reset */

	card_reset(1);
	udelay(2500);
	card_reset(0);
	udelay(2500);
	card_reset(1);
	udelay(2500);

	/* Enable Interrupt Flags*/
	mmio_write_32((cdns_params.reg_base + SDHC_CDNS_SRS13), ~0);
	high_speed_enable(true);
}

int cdns_sd_host_init(struct cdns_sdmmc_combo_phy *mmc_combo_phy_reg,
		      struct cdns_sdmmc_sdhc *mmc_sdhc_reg)
{
	int ret = 0;

	ret = cdns_reset();
	if (ret != 0U) {
		ERROR("Program phy reg init failed");
		return ret;
	}

	ret = cdns_program_phy_reg(&sdmmc_combo_phy_reg, &sdmmc_sdhc_reg);
	if (ret != 0U) {
		ERROR("Program phy reg init failed");
		return ret;
	}
	sdmmc_host_init(0);
	cdns_host_set_clk(100000);

	sd_host_oper_mode(SD_HOST_OPR_MODE_HV4E_0_ADMA_64);

	return 0;
}

int cdns_send_cmd(struct mmc_cmd *cmd)
{
	uint32_t cmd_flags = 0;
	uint32_t timeout = 0;
	uint32_t status_check = 0;
	uint32_t mode = 0;
	uint32_t status;

	assert(cmd);

	cmd_flags = CDNS_HOST_CMD_INHIBIT | CDNS_HOST_DATA_INHIBIT;

	if ((cmd->cmd_idx == SD_STOP_TRANSMISSION) && (!data_cmd)) {
		cmd_flags &= ~CDNS_HOST_DATA_INHIBIT;
	}

	timeout = TIMEOUT;
	do {
		udelay(100);
		if (--timeout <= 0) {
			udelay(50);
			NOTICE("Timeout occur data and cmd line %x\n",
			 mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS09));
			panic();
		}
	} while ((mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS09) & (cmd_flags)));

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS12, 0xFFFFFFFF);
	cmd_flags = 0;
	cmd_flags = (cmd->cmd_idx) << COM_IDX;

	if ((cmd->resp_type & MMC_RSP_136) != 0) {
		cmd_flags |= RES_TYPE_SEL_136;
	} else if (((cmd->resp_type & MMC_RSP_48) != 0) &&
			((cmd->resp_type & MMC_RSP_BUSY) != 0)) {
		cmd_flags |= RES_TYPE_SEL_48_B;
	} else if ((cmd->resp_type & MMC_RSP_48) != 0) {
		cmd_flags |= RES_TYPE_SEL_48;
	} else {
		cmd_flags &= ~RES_TYPE_SEL_NO;
	}

	if ((cmd->resp_type & MMC_RSP_CRC) != 0) {
		cmd_flags |= CMD_CHECK_RESP_CRC;
	}

	if ((cmd->resp_type & MMC_RSP_CMD_IDX) != 0) {
		cmd_flags |= CMD_IDX_CHK_ENABLE;
	}

	if ((cmd->cmd_idx == MMC_ACMD(51)) || (cmd->cmd_idx == MMC_CMD(17)) ||
		(cmd->cmd_idx == MMC_CMD(18)) || (cmd->cmd_idx == MMC_CMD(24)) ||
		(cmd->cmd_idx == MMC_CMD(25)) || (cmd->cmd_idx == MMC_CMD(8) &&
		cdns_params.cdn_sdmmc_dev_type == MMC_IS_EMMC)) {
		mmio_write_8((cdns_params.reg_base + DTCV_OFFSET), DTCV_VAL);
		cmd_flags |= DATA_PRESENT;
		mode |= BLK_CNT_EN;

		mode |= (DMA_ENABLED);
		if ((cmd->cmd_idx == SD_WRITE_MULTIPLE_BLOCK) ||
		(cmd->cmd_idx == SD_READ_MULTIPLE_BLOCK)) {
			mode |= (MULTI_BLK_READ);
		} else {
			mode &= ~(MULTI_BLK_READ);
		}
		if ((cmd->cmd_idx == SD_WRITE_MULTIPLE_BLOCK) ||
		(cmd->cmd_idx == SD_WRITE_SINGLE_BLOCK)) {
			mode &= ~CMD_READ;
		} else {
			mode |= CMD_READ;
		}
		mmio_write_16(cdns_params.reg_base + SDHC_CDNS_SRS03, mode);

	} else {
		mmio_write_8((cdns_params.reg_base + DTCV_OFFSET), DTCV_VAL);
	}

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS02, cmd->cmd_arg);
	mmio_write_16((cdns_params.reg_base + CICE_OFFSET),
		SDHCI_MAKE_CMD(cmd->cmd_idx, cmd_flags));

	timeout = TIMEOUT;

	do {
		udelay(CDNS_TIMEOUT);
		status = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS12);
	} while (((status & (INT_CMD_DONE | ERROR_INT)) == 0) && (timeout-- > 0));

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS12, (SRS_12_CC_EN));
	status_check = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS12) & 0xffff8000;
	if (status_check != 0U) {
		timeout = TIMEOUT;
		ERROR("SD host controller send command failed, SRS12 = %x", status_check);
		return -1;
	}

	if (!((cmd_flags & RES_TYPE_SEL_NO) == 0)) {
		cmd->resp_data[0] = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS04);
		if ((cmd_flags & RES_TYPE_SEL_NO) == RES_TYPE_SEL_136) {
			cmd->resp_data[1] = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS05);
			cmd->resp_data[2] = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS06);
			cmd->resp_data[3] = mmio_read_32(cdns_params.reg_base + SDHC_CDNS_SRS07);
			/* 136-bit: RTS=01b, Response field R[127:8] - RESP3[23:0],
			 * RESP2[31:0], RESP1[31:0], RESP0[31:0]
			 * Subsystem expects 128 bits response but cadence SDHC sends
			 * 120 bits response from R[127:8]. Bits manupulation to address
			 * the correct responses for the 136 bit response type.
			 */
			cmd->resp_data[3] = ((cmd->resp_data[3] << 8) |
						((cmd->resp_data[2] >> 24) &
						CDNS_CSD_BYTE_MASK));
			cmd->resp_data[2] = ((cmd->resp_data[2] << 8) |
						((cmd->resp_data[1] >> 24) &
						CDNS_CSD_BYTE_MASK));
			cmd->resp_data[1] = ((cmd->resp_data[1] << 8) |
						((cmd->resp_data[0] >> 24) &
						CDNS_CSD_BYTE_MASK));
			cmd->resp_data[0] = (cmd->resp_data[0] << 8);
		}
	}

	mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS12, (SRS_12_CC_EN));

	return 0;
}

void sd_host_adma_prepare(struct cdns_idmac_desc *desc_ptr, uint64_t buf,
			  size_t size)
{
	uint32_t full_desc_cnt = 0;
	uint32_t non_full_desc_cnt = 0;
	uint64_t desc_address;
	uint32_t block_count;
	uint32_t transfer_block_size;

	full_desc_cnt = (size / PAGE_BUFFER_LEN);
	non_full_desc_cnt = (size % PAGE_BUFFER_LEN);
	for (int i = 0; i < full_desc_cnt; i++) {
		desc_ptr->attr = (ADMA_DESC_TRANSFER_DATA | ADMA_DESC_ATTR_VALID);
		desc_ptr->len = 0; // 0 means 64kb page size it will take
		desc_ptr->addr_lo = 0;
#if CONFIG_DMA_ADDR_T_64BIT == 1
		desc_ptr->addr_hi = (uint32_t)((buf >> 32) & 0xffffffff);
#endif
		if (non_full_desc_cnt == 0) {
			desc_ptr->attr |= (ADMA_DESC_ATTR_END);
		}
	buf += PAGE_BUFFER_LEN;
	}

	if (non_full_desc_cnt != 0) {
		desc_ptr->attr =
		(ADMA_DESC_TRANSFER_DATA | ADMA_DESC_ATTR_END | ADMA_DESC_ATTR_VALID);
		desc_ptr->addr_lo = buf & 0xffffffff;
		desc_ptr->len = size;
#if CONFIG_DMA_ADDR_T_64BIT == 1
		desc_ptr->addr_hi = (uint32_t)((buf >> 32) & 0xffffffff);
#endif
		desc_address = (uint64_t)desc_ptr;
		if (size > MMC_MAX_BLOCK_LEN) {
			transfer_block_size = MMC_MAX_BLOCK_LEN;
		} else {
			transfer_block_size = size;
		}

		block_count = (size / transfer_block_size);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS01,
				((transfer_block_size << BLOCK_SIZE) | SDMA_BUF |
				(block_count << BLK_COUNT_CT)));
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS22,
				(uint32_t)desc_address & 0xFFFFFFFF);
		mmio_write_32(cdns_params.reg_base + SDHC_CDNS_SRS23,
				(uint32_t)(desc_address >> 32 & 0xFFFFFFFF));
	}
}

int cdns_mmc_init(struct cdns_sdmmc_params *params,
		  struct mmc_device_info *info)
{

	int result = 0;

	assert((params != NULL) &&
		((params->reg_base & MMC_BLOCK_MASK) == 0) &&
		((params->desc_size & MMC_BLOCK_MASK) == 0) &&
		((params->reg_pinmux & MMC_BLOCK_MASK) == 0) &&
		((params->reg_phy & MMC_BLOCK_MASK) == 0) &&
		(params->desc_size > 0) &&
		(params->clk_rate > 0) &&
		(params->sdmclk > 0) &&
		((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

	memcpy(&cdns_params, params, sizeof(struct cdns_sdmmc_params));

	cdns_set_sdmmc_var(&sdmmc_combo_phy_reg, &sdmmc_sdhc_reg);
	result = cdns_sd_host_init(&sdmmc_combo_phy_reg, &sdmmc_sdhc_reg);
	if (result < 0) {
		return result;
	}

	cdns_params.cdn_sdmmc_dev_type = info->mmc_dev_type;
	cdns_params.cdn_sdmmc_dev_mode = SD_DS;

	result = mmc_init(&cdns_sdmmc_ops, params->clk_rate, params->bus_width,
			params->flags, info);

	return result;
}

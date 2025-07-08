/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/mmc.h>
#include <lib/mmio_poll.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <imx_usdhc.h>

/* These masks represent the commands which involve a data transfer. */
#define ADTC_MASK_SD			(BIT_32(6U) | BIT_32(17U) | BIT_32(18U) |\
					 BIT_32(24U) | BIT_32(25U))
#define ADTC_MASK_ACMD			(BIT_64(51U))

#define USDHC_TIMEOUT_US	(1U * 1000U) /* 1 msec */
#define USDHC_TRANSFER_TIMEOUT	(1U * 1000U * 1000U) /* 1 sec */

struct imx_usdhc_device_data {
	uint32_t addr;
	uint32_t blk_size;
	uint32_t blks;
	bool valid;
};

static void imx_usdhc_initialize(void);
static int imx_usdhc_send_cmd(struct mmc_cmd *cmd);
static int imx_usdhc_set_ios(unsigned int clk, unsigned int width);
static int imx_usdhc_prepare(int lba, uintptr_t buf, size_t size);
static int imx_usdhc_read(int lba, uintptr_t buf, size_t size);
static int imx_usdhc_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops imx_usdhc_ops = {
	.init		= imx_usdhc_initialize,
	.send_cmd	= imx_usdhc_send_cmd,
	.set_ios	= imx_usdhc_set_ios,
	.prepare	= imx_usdhc_prepare,
	.read		= imx_usdhc_read,
	.write		= imx_usdhc_write,
};

static imx_usdhc_params_t imx_usdhc_params;
static struct imx_usdhc_device_data imx_usdhc_data;

static bool imx_usdhc_is_buf_valid(void)
{
	return imx_usdhc_data.valid;
}

static bool imx_usdhc_is_buf_multiblk(void)
{
	return imx_usdhc_data.blks > 1U;
}

static void imx_usdhc_inval_buf_data(void)
{
	imx_usdhc_data.valid = false;
}

static int imx_usdhc_save_buf_data(uintptr_t buf, size_t size)
{
	uint32_t block_size;
	uint64_t blks;

	if (size <= MMC_BLOCK_SIZE) {
		block_size = (uint32_t)size;
	} else {
		block_size = MMC_BLOCK_SIZE;
	}

	if (buf > UINT32_MAX) {
		return -EOVERFLOW;
	}

	imx_usdhc_data.addr = (uint32_t)buf;
	imx_usdhc_data.blk_size = block_size;
	blks = size / block_size;
	imx_usdhc_data.blks = (uint32_t)blks;

	imx_usdhc_data.valid = true;

	return 0;
}

static void imx_usdhc_write_buf_data(void)
{
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	uint32_t addr, blks, blk_size;

	addr = imx_usdhc_data.addr;
	blks = imx_usdhc_data.blks;
	blk_size = imx_usdhc_data.blk_size;

	mmio_write_32(reg_base + DSADDR, addr);
	mmio_write_32(reg_base + BLKATT, BLKATT_BLKCNT(blks) |
		      BLKATT_BLKSIZE(blk_size));
}

#define IMX7_MMC_SRC_CLK_RATE (200 * 1000 * 1000)
static int imx_usdhc_set_clk(unsigned int clk)
{
	unsigned int sdhc_clk = IMX7_MMC_SRC_CLK_RATE;
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	unsigned int pre_div = 1U, div = 1U;
	uint32_t pstate;
	int ret;

	assert(clk > 0);

	while (sdhc_clk / (16 * pre_div) > clk && pre_div < 256)
		pre_div *= 2;

	while (((sdhc_clk / (div * pre_div)) > clk) && (div < 16U)) {
		div++;
	}

	pre_div >>= 1;
	div -= 1;
	clk = (pre_div << 8) | (div << 4);

	ret = mmio_read_32_poll_timeout(reg_base + PSTATE, pstate,
					(pstate & PSTATE_SDSTB) != 0U,
					USDHC_TIMEOUT_US);
	if (ret == -ETIMEDOUT) {
		ERROR("Unstable SD clock\n");
		return ret;
	}

	mmio_clrbits32(reg_base + VENDSPEC, VENDSPEC_CARD_CLKEN);
	mmio_clrsetbits32(reg_base + SYSCTRL, SYSCTRL_CLOCK_MASK, clk);
	udelay(10000);

	mmio_setbits32(reg_base + VENDSPEC, VENDSPEC_PER_CLKEN | VENDSPEC_CARD_CLKEN);

	return 0;
}

static void imx_usdhc_initialize(void)
{
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	uint32_t sysctrl;
	int ret;

	assert((imx_usdhc_params.reg_base & MMC_BLOCK_MASK) == 0);

	/* reset the controller */
	mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTA);

	/* wait for reset done */
	ret = mmio_read_32_poll_timeout(reg_base + SYSCTRL, sysctrl,
					(sysctrl & SYSCTRL_RSTA) == 0U,
					USDHC_TIMEOUT_US);
	if (ret == -ETIMEDOUT) {
		ERROR("Failed to reset the USDHC controller\n");
		panic();
	}

	mmio_write_32(reg_base + MMCBOOT, 0);
	mmio_write_32(reg_base + MIXCTRL, 0);
	mmio_write_32(reg_base + CLKTUNECTRLSTS, 0);

	mmio_write_32(reg_base + VENDSPEC, VENDSPEC_INIT);
	mmio_write_32(reg_base + DLLCTRL, 0);
	mmio_setbits32(reg_base + VENDSPEC, VENDSPEC_IPG_CLKEN | VENDSPEC_PER_CLKEN);

	/* Set the initial boot clock rate */
	ret = imx_usdhc_set_clk(MMC_BOOT_CLK_RATE);
	if (ret != 0) {
		panic();
	}

	udelay(100);

	/* Clear read/write ready status */
	mmio_clrbits32(reg_base + INTSTATEN, INTSTATEN_BRR | INTSTATEN_BWR);

	/* configure as little endian */
	mmio_write_32(reg_base + PROTCTRL, PROTCTRL_LE);

	/* Set timeout to the maximum value */
	mmio_clrsetbits32(reg_base + SYSCTRL, SYSCTRL_TIMEOUT_MASK,
			  SYSCTRL_TIMEOUT(15));

	/* set wartermark level as 16 for safe for MMC */
	mmio_clrsetbits32(reg_base + WATERMARKLEV, WMKLV_MASK, 16 | (16 << 16));
}

static bool is_data_transfer_to_card(const struct mmc_cmd *cmd)
{
	unsigned int cmd_idx = cmd->cmd_idx;

	return (cmd_idx == MMC_CMD(24)) || (cmd_idx == MMC_CMD(25));
}

static bool is_data_transfer_cmd(const struct mmc_cmd *cmd)
{
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	unsigned int cmd_idx = cmd->cmd_idx;
	uint32_t xfer_type;

	xfer_type = mmio_read_32(reg_base + XFERTYPE);

	if (XFERTYPE_GET_CMD(xfer_type) == MMC_CMD(55)) {
		return (ADTC_MASK_ACMD & BIT_64(cmd_idx)) != 0ULL;
	}

	if ((ADTC_MASK_SD & BIT_32(cmd->cmd_idx)) != 0U) {
		return true;
	}

	return false;
}

static int get_xfr_type(const struct mmc_cmd *cmd, bool data, uint32_t *xfertype)
{
	*xfertype = XFERTYPE_CMD(cmd->cmd_idx);

	switch (cmd->resp_type) {
	case MMC_RESPONSE_R2:
		*xfertype |= XFERTYPE_RSPTYP_136;
		*xfertype |= XFERTYPE_CCCEN;
		break;
	case MMC_RESPONSE_R4:
		*xfertype |= XFERTYPE_RSPTYP_48;
		break;
	case MMC_RESPONSE_R6:
		*xfertype |= XFERTYPE_RSPTYP_48;
		*xfertype |= XFERTYPE_CICEN;
		*xfertype |= XFERTYPE_CCCEN;
		break;
	case MMC_RESPONSE_R1B:
		*xfertype |= XFERTYPE_RSPTYP_48_BUSY;
		*xfertype |= XFERTYPE_CICEN;
		*xfertype |= XFERTYPE_CCCEN;
		break;
	case MMC_RESPONSE_NONE:
		break;
	default:
		ERROR("Invalid CMD response: %u\n", cmd->resp_type);
		return -EINVAL;
	}

	if (data) {
		*xfertype |= XFERTYPE_DPSEL;
	}

	return 0;
}

static int imx_usdhc_send_cmd(struct mmc_cmd *cmd)
{
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	unsigned int flags = INTSTATEN_CC | INTSTATEN_CTOE;
	uint32_t xfertype, pstate, intstat, sysctrl;
	unsigned int mixctl = 0;
	int err = 0, ret;
	bool data;

	assert(cmd);

	data = is_data_transfer_cmd(cmd);

	err = get_xfr_type(cmd, data, &xfertype);
	if (err != 0) {
		return err;
	}

	/* clear all irq status */
	mmio_write_32(reg_base + INTSTAT, 0xffffffff);

	/* Wait for the bus to be idle */
	err = mmio_read_32_poll_timeout(reg_base + PSTATE, pstate,
					(pstate & (PSTATE_CDIHB | PSTATE_CIHB)) == 0U,
					USDHC_TIMEOUT_US);
	if (err == -ETIMEDOUT) {
		ERROR("Failed to wait an idle bus\n");
		return err;
	}

	err = mmio_read_32_poll_timeout(reg_base + PSTATE, pstate,
					(pstate & PSTATE_DLA) == 0U,
					USDHC_TIMEOUT_US);
	if (err == -ETIMEDOUT) {
		ERROR("Active data line during the uSDHC init\n");
		return err;
	}

	mmio_write_32(reg_base + INTSIGEN, 0);

	if (data) {
		mixctl |= MIXCTRL_DMAEN;
	}

	if (!is_data_transfer_to_card(cmd)) {
		mixctl |= MIXCTRL_DTDSEL;
	}

	if ((cmd->cmd_idx != MMC_CMD(55)) && imx_usdhc_is_buf_valid()) {
		if (imx_usdhc_is_buf_multiblk()) {
			mixctl |= MIXCTRL_MSBSEL | MIXCTRL_BCEN;
		}

		imx_usdhc_write_buf_data();
		imx_usdhc_inval_buf_data();
	}

	/* Send the command */
	mmio_write_32(reg_base + CMDARG, cmd->cmd_arg);
	mmio_clrsetbits32(reg_base + MIXCTRL, MIXCTRL_DATMASK, mixctl);
	mmio_write_32(reg_base + XFERTYPE, xfertype);

	/* Wait for the command done */
	err = mmio_read_32_poll_timeout(reg_base + INTSTAT, intstat,
					(intstat & flags) != 0U,
					USDHC_TIMEOUT_US);
	if ((err == -ETIMEDOUT) || ((intstat & (INTSTATEN_CTOE | CMD_ERR)) != 0U)) {
		if ((intstat & (INTSTATEN_CTOE | CMD_ERR)) != 0U) {
			err = -EIO;
		}
		ERROR("imx_usdhc mmc cmd %d state 0x%x errno=%d\n",
		      cmd->cmd_idx, intstat, err);
		goto out;
	}

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		unsigned int cmdrsp3, cmdrsp2, cmdrsp1, cmdrsp0;

		cmdrsp3 = mmio_read_32(reg_base + CMDRSP3);
		cmdrsp2 = mmio_read_32(reg_base + CMDRSP2);
		cmdrsp1 = mmio_read_32(reg_base + CMDRSP1);
		cmdrsp0 = mmio_read_32(reg_base + CMDRSP0);
		cmd->resp_data[3] = (cmdrsp3 << 8) | (cmdrsp2 >> 24);
		cmd->resp_data[2] = (cmdrsp2 << 8) | (cmdrsp1 >> 24);
		cmd->resp_data[1] = (cmdrsp1 << 8) | (cmdrsp0 >> 24);
		cmd->resp_data[0] = (cmdrsp0 << 8);
	} else {
		cmd->resp_data[0] = mmio_read_32(reg_base + CMDRSP0);
	}

	/* Wait until all of the blocks are transferred */
	if (data) {
		flags = DATA_COMPLETE;
		err = mmio_read_32_poll_timeout(reg_base + INTSTAT, intstat,
						(((intstat & (INTSTATEN_DTOE | DATA_ERR)) != 0U) ||
						 ((intstat & flags) == flags)),
						USDHC_TRANSFER_TIMEOUT);
		if ((intstat & (INTSTATEN_DTOE | DATA_ERR)) != 0U) {
			err = -EIO;
			ERROR("imx_usdhc mmc data state 0x%x\n", intstat);
			goto out;
		}

		if (err == -ETIMEDOUT) {
			ERROR("Timeout in block transfer\n");
			goto out;
		}
	}

out:
	/* Reset CMD and DATA on error */
	if (err) {
		mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTC);
		ret = mmio_read_32_poll_timeout(reg_base + SYSCTRL, sysctrl,
						(sysctrl & SYSCTRL_RSTC) == 0U,
						USDHC_TIMEOUT_US);
		if (ret == -ETIMEDOUT) {
			ERROR("Failed to reset the CMD line\n");
		}

		if (data) {
			mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTD);
			ret = mmio_read_32_poll_timeout(reg_base + SYSCTRL, sysctrl,
							(sysctrl & SYSCTRL_RSTD) == 0U,
							USDHC_TIMEOUT_US);
			if (ret == -ETIMEDOUT) {
				ERROR("Failed to reset the data line\n");
			}
		}
	}

	/* clear all irq status */
	mmio_write_32(reg_base + INTSTAT, 0xffffffff);

	return err;
}

static int imx_usdhc_set_ios(unsigned int clk, unsigned int width)
{
	uintptr_t reg_base = imx_usdhc_params.reg_base;
	int ret;

	ret = imx_usdhc_set_clk(clk);
	if (ret != 0) {
		return ret;
	}

	if (width == MMC_BUS_WIDTH_4)
		mmio_clrsetbits32(reg_base + PROTCTRL, PROTCTRL_WIDTH_MASK,
				  PROTCTRL_WIDTH_4);
	else if (width == MMC_BUS_WIDTH_8)
		mmio_clrsetbits32(reg_base + PROTCTRL, PROTCTRL_WIDTH_MASK,
				  PROTCTRL_WIDTH_8);

	return 0;
}

static int imx_usdhc_prepare(int lba, uintptr_t buf, size_t size)
{
	flush_dcache_range(buf, size);
	return imx_usdhc_save_buf_data(buf, size);
}

static int imx_usdhc_read(int lba, uintptr_t buf, size_t size)
{
	inv_dcache_range(buf, size);
	return 0;
}

static int imx_usdhc_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

void imx_usdhc_init(imx_usdhc_params_t *params,
		    struct mmc_device_info *mmc_dev_info)
{
	int ret __maybe_unused;

	assert((params != 0) &&
	       ((params->reg_base & MMC_BLOCK_MASK) == 0) &&
	       ((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

#if PLAT_XLAT_TABLES_DYNAMIC
	ret = mmap_add_dynamic_region(params->reg_base, params->reg_base,
				      PAGE_SIZE,
				      MT_DEVICE | MT_RW | MT_SECURE);
	if (ret != 0) {
		ERROR("Failed to map the uSDHC registers\n");
		panic();
	}
#endif

	memcpy(&imx_usdhc_params, params, sizeof(imx_usdhc_params_t));
	mmc_init(&imx_usdhc_ops, params->clk_rate, params->bus_width,
		 params->flags, mmc_dev_info);
}

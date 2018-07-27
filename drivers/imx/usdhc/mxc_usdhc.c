/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mxc_usdhc.h>
#include <emmc.h>
#include <errno.h>
#include <mmio.h>
#include <string.h>

static void fsl_init(void);
static int fsl_send_cmd(emmc_cmd_t *cmd);
static int fsl_set_ios(int clk, int width);
static int fsl_prepare(int lba, uintptr_t buf, size_t size);
static int fsl_read(int lba, uintptr_t buf, size_t size);
static int fsl_write(int lba, uintptr_t buf, size_t size);

static const emmc_ops_t mxc_usdhc_ops = {
	.init		= fsl_init,
	.send_cmd	= fsl_send_cmd,
	.set_ios	= fsl_set_ios,
	.prepare	= fsl_prepare,
	.read		= fsl_read,
	.write		= fsl_write,
};

static mxc_usdhc_params_t fsl_params;

#define IMX7_MMC_SRC_CLK_RATE (200 * 1000 * 1000)
static void fsl_set_clk(int clk)
{
	int div = 1;
	int pre_div = 1;
	unsigned int sdhc_clk = IMX7_MMC_SRC_CLK_RATE;
	uintptr_t reg_base = fsl_params.reg_base;

	assert(clk > 0);

	while (sdhc_clk / (16 * pre_div) > clk && pre_div < 256)
		pre_div *= 2;

	while (sdhc_clk / div > clk && div < 16)
		div++;

	pre_div >>= 1;
	div -= 1;
	clk = (pre_div << 8) | (div << 4);

	mmio_clrbits32(reg_base + VENDSPEC, VENDSPEC_CARD_CLKEN);
	mmio_clrsetbits32(reg_base + SYSCTRL, SYSCTRL_CLOCK_MASK, clk);
	udelay(10000);

	mmio_setbits32(reg_base + VENDSPEC, VENDSPEC_PER_CLKEN | VENDSPEC_CARD_CLKEN);
}

static void fsl_init(void)
{
	unsigned int timeout = 10000;
	uintptr_t reg_base = fsl_params.reg_base;

	assert((fsl_params.reg_base & EMMC_BLOCK_MASK) == 0);

	/* reset the controller */
	mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTA);

	/* wait for reset done */
	while ((mmio_read_32(reg_base + SYSCTRL) & SYSCTRL_RSTA)) {
		if (!timeout) {
			printf("IMX MMC reset timeout.\n");
		}
		timeout--;
	}

	mmio_write_32(reg_base + MMCBOOT, 0);
	mmio_write_32(reg_base + MIXCTRL, 0);
	mmio_write_32(reg_base + CLKTUNECTRLSTS, 0);

	mmio_write_32(reg_base + VENDSPEC, VENDSPEC_INIT);
	mmio_write_32(reg_base + DLLCTRL, 0);
	mmio_setbits32(reg_base + VENDSPEC, VENDSPEC_IPG_CLKEN | VENDSPEC_PER_CLKEN);

	/* Set the initial boot clock rate */
	fsl_set_clk(EMMC_BOOT_CLK_RATE);
	udelay(100);

	/* Clear read/write ready status */
	mmio_clrbits32(reg_base + INTSTATEN, INTSTATEN_BRR | INTSTATEN_BWR);

	/* configure as little endian */
	mmio_write_32(reg_base + PROTCTRL, PROTCTRL_LE);

	/* Set timeout to the maximum value */
	mmio_clrsetbits32(reg_base + SYSCTRL, SYSCTRL_TIMEOUT_MASK,
			  SYSCTRL_TIMEOUT(15));

	/* set wartermark level as 16 for safe for EMMC */
	mmio_clrsetbits32(reg_base + WATERMARKLEV, WMKLV_MASK, 16 | (16 << 16));
}

#define FSL_CMD_RETRIES	1000

static int fsl_send_cmd(emmc_cmd_t *cmd)
{
	uintptr_t reg_base = fsl_params.reg_base;
	unsigned int xfertype = 0, mixctl = 0, multiple = 0, data = 0, err = 0;
	unsigned int state, flags = INTSTATEN_CC | INTSTATEN_CTOE;
	unsigned int cmd_retries = 0;

	assert(cmd);

	/* clear all irq status */
	mmio_write_32(reg_base + INTSTAT, 0xffffffff);

	/* Wait for the bus to be idle */
	do {
		state = mmio_read_32(reg_base + PSTATE);
	} while (state & (PSTATE_CDIHB | PSTATE_CIHB));

	while (mmio_read_32(reg_base + PSTATE) & PSTATE_DLA)
		;

	mmio_write_32(reg_base + INTSIGEN, 0);
	udelay(1000);

	switch (cmd->cmd_idx) {
	case EMMC_CMD12:
		xfertype |= XFERTYPE_CMDTYP_ABORT;
		break;
	case EMMC_CMD18:
		multiple = 1;
		/* fall thru for read op */
	case EMMC_CMD17:
	case EMMC_CMD8:
		mixctl |= MIXCTRL_DTDSEL;
		data = 1;
		break;
	case EMMC_CMD25:
		multiple = 1;
		/* fall thru for data op flag */
	case EMMC_CMD24:
		data = 1;
		break;
	default:
		break;
	}

	if (multiple) {
		mixctl |= MIXCTRL_MSBSEL;
		mixctl |= MIXCTRL_BCEN;
	}

	if (data) {
		xfertype |= XFERTYPE_DPSEL;
		mixctl |= MIXCTRL_DMAEN;
	}

	if (cmd->resp_type & MMC_RSP_48)
		xfertype |= XFERTYPE_RSPTYP_48;
	else if (cmd->resp_type & MMC_RSP_136)
		xfertype |= XFERTYPE_RSPTYP_136;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		xfertype |= XFERTYPE_RSPTYP_48_BUSY;

	if (cmd->resp_type & MMC_RSP_CMD_IDX)
		xfertype |= XFERTYPE_CICEN;

	if (cmd->resp_type & MMC_RSP_CRC)
		xfertype |= XFERTYPE_CCCEN;

	xfertype |= XFERTYPE_CMD(cmd->cmd_idx);

	/* Send the command */
	mmio_write_32(reg_base + CMDARG, cmd->cmd_arg);
	mmio_clrsetbits32(reg_base + MIXCTRL, MIXCTRL_DATMASK, mixctl);
	mmio_write_32(reg_base + XFERTYPE, xfertype);

	/* Wait for the command done */
	do {
		state = mmio_read_32(reg_base + INTSTAT);
		if (cmd_retries)
			udelay(1);
	} while ((!(state & flags)) && ++cmd_retries < FSL_CMD_RETRIES);

	if ((state & (INTSTATEN_CTOE | CMD_ERR)) || cmd_retries == FSL_CMD_RETRIES) {
		if (cmd_retries == FSL_CMD_RETRIES)
			err = -ETIMEDOUT;
		else
			err = -EIO;
		ERROR("fsl mmc cmd %d state 0x%x errno=%d\n",
		      cmd->cmd_idx, state, err);
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
		do {
			state = mmio_read_32(reg_base + INTSTAT);

			if (state & (INTSTATEN_DTOE | DATA_ERR)) {
				err = -EIO;
				ERROR("fsl mmc data state 0x%x\n", state);
				goto out;
			}
		} while ((state & flags) != flags);
	}

out:
	/* Reset CMD and DATA on error */
	if (err) {
		mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTC);
		while (mmio_read_32(reg_base + SYSCTRL) & SYSCTRL_RSTC)
			;

		if (data) {
			mmio_setbits32(reg_base + SYSCTRL, SYSCTRL_RSTD);
			while (mmio_read_32(reg_base + SYSCTRL) & SYSCTRL_RSTD)
				;
		}
	}

	/* clear all irq status */
	mmio_write_32(reg_base + INTSTAT, 0xffffffff);

	return err;
}

static int fsl_set_ios(int clk, int width)
{
	uintptr_t reg_base = fsl_params.reg_base;

	fsl_set_clk(clk);

	if (width == EMMC_BUS_WIDTH_4)
		mmio_clrsetbits32(reg_base + PROTCTRL, PROTCTRL_WIDTH_MASK,
				  PROTCTRL_WIDTH_4);
	else if (width == EMMC_BUS_WIDTH_8)
		mmio_clrsetbits32(reg_base + PROTCTRL, PROTCTRL_WIDTH_MASK,
				  PROTCTRL_WIDTH_8);

	return 0;
}

static int fsl_prepare(int lba, uintptr_t buf, size_t size)
{
	uintptr_t reg_base = fsl_params.reg_base;

	mmio_write_32(reg_base + DSADDR, buf);
	mmio_write_32(reg_base + BLKATT,
		      (size / EMMC_BLOCK_SIZE) << 16 | EMMC_BLOCK_SIZE);

	return 0;
}

static int fsl_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int fsl_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

void mxc_usdhc_init(mxc_usdhc_params_t *params)
{
	assert((params != 0) &&
	       ((params->reg_base & EMMC_BLOCK_MASK) == 0) &&
	       (params->clk_rate > 0) &&
	       ((params->bus_width == EMMC_BUS_WIDTH_1) ||
		(params->bus_width == EMMC_BUS_WIDTH_4) ||
		(params->bus_width == EMMC_BUS_WIDTH_8)));

	memcpy(&fsl_params, params, sizeof(mxc_usdhc_params_t));
	emmc_init(&mxc_usdhc_ops, params->clk_rate, params->bus_width,
		  params->flags);
}

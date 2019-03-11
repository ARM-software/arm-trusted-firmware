/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/mmc.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32_sdmmc2.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

/* Registers offsets */
#define SDMMC_POWER			0x00U
#define SDMMC_CLKCR			0x04U
#define SDMMC_ARGR			0x08U
#define SDMMC_CMDR			0x0CU
#define SDMMC_RESPCMDR			0x10U
#define SDMMC_RESP1R			0x14U
#define SDMMC_RESP2R			0x18U
#define SDMMC_RESP3R			0x1CU
#define SDMMC_RESP4R			0x20U
#define SDMMC_DTIMER			0x24U
#define SDMMC_DLENR			0x28U
#define SDMMC_DCTRLR			0x2CU
#define SDMMC_DCNTR			0x30U
#define SDMMC_STAR			0x34U
#define SDMMC_ICR			0x38U
#define SDMMC_MASKR			0x3CU
#define SDMMC_ACKTIMER			0x40U
#define SDMMC_IDMACTRLR			0x50U
#define SDMMC_IDMABSIZER		0x54U
#define SDMMC_IDMABASE0R		0x58U
#define SDMMC_IDMABASE1R		0x5CU
#define SDMMC_FIFOR			0x80U

/* SDMMC power control register */
#define SDMMC_POWER_PWRCTRL		GENMASK(1, 0)
#define SDMMC_POWER_DIRPOL		BIT(4)

/* SDMMC clock control register */
#define SDMMC_CLKCR_WIDBUS_4		BIT(14)
#define SDMMC_CLKCR_WIDBUS_8		BIT(15)
#define SDMMC_CLKCR_NEGEDGE		BIT(16)
#define SDMMC_CLKCR_HWFC_EN		BIT(17)
#define SDMMC_CLKCR_SELCLKRX_0		BIT(20)

/* SDMMC command register */
#define SDMMC_CMDR_CMDTRANS		BIT(6)
#define SDMMC_CMDR_CMDSTOP		BIT(7)
#define SDMMC_CMDR_WAITRESP		GENMASK(9, 8)
#define SDMMC_CMDR_WAITRESP_SHORT	BIT(8)
#define SDMMC_CMDR_WAITRESP_SHORT_NOCRC	BIT(9)
#define SDMMC_CMDR_CPSMEN		BIT(12)

/* SDMMC data control register */
#define SDMMC_DCTRLR_DTEN		BIT(0)
#define SDMMC_DCTRLR_DTDIR		BIT(1)
#define SDMMC_DCTRLR_DTMODE		GENMASK(3, 2)
#define SDMMC_DCTRLR_DBLOCKSIZE_0	BIT(4)
#define SDMMC_DCTRLR_DBLOCKSIZE_1	BIT(5)
#define SDMMC_DCTRLR_DBLOCKSIZE_3	BIT(7)
#define SDMMC_DCTRLR_DBLOCKSIZE		GENMASK(7, 4)
#define SDMMC_DCTRLR_FIFORST		BIT(13)

#define SDMMC_DCTRLR_CLEAR_MASK		(SDMMC_DCTRLR_DTEN | \
					 SDMMC_DCTRLR_DTDIR | \
					 SDMMC_DCTRLR_DTMODE | \
					 SDMMC_DCTRLR_DBLOCKSIZE)
#define SDMMC_DBLOCKSIZE_8		(SDMMC_DCTRLR_DBLOCKSIZE_0 | \
					 SDMMC_DCTRLR_DBLOCKSIZE_1)
#define SDMMC_DBLOCKSIZE_512		(SDMMC_DCTRLR_DBLOCKSIZE_0 | \
					 SDMMC_DCTRLR_DBLOCKSIZE_3)

/* SDMMC status register */
#define SDMMC_STAR_CCRCFAIL		BIT(0)
#define SDMMC_STAR_DCRCFAIL		BIT(1)
#define SDMMC_STAR_CTIMEOUT		BIT(2)
#define SDMMC_STAR_DTIMEOUT		BIT(3)
#define SDMMC_STAR_TXUNDERR		BIT(4)
#define SDMMC_STAR_RXOVERR		BIT(5)
#define SDMMC_STAR_CMDREND		BIT(6)
#define SDMMC_STAR_CMDSENT		BIT(7)
#define SDMMC_STAR_DATAEND		BIT(8)
#define SDMMC_STAR_DBCKEND		BIT(10)
#define SDMMC_STAR_DPSMACT		BIT(12)
#define SDMMC_STAR_RXFIFOHF		BIT(15)
#define SDMMC_STAR_RXFIFOE		BIT(19)
#define SDMMC_STAR_IDMATE		BIT(27)
#define SDMMC_STAR_IDMABTC		BIT(28)

/* SDMMC DMA control register */
#define SDMMC_IDMACTRLR_IDMAEN		BIT(0)

#define SDMMC_STATIC_FLAGS		(SDMMC_STAR_CCRCFAIL | \
					 SDMMC_STAR_DCRCFAIL | \
					 SDMMC_STAR_CTIMEOUT | \
					 SDMMC_STAR_DTIMEOUT | \
					 SDMMC_STAR_TXUNDERR | \
					 SDMMC_STAR_RXOVERR  | \
					 SDMMC_STAR_CMDREND  | \
					 SDMMC_STAR_CMDSENT  | \
					 SDMMC_STAR_DATAEND  | \
					 SDMMC_STAR_DBCKEND  | \
					 SDMMC_STAR_IDMATE   | \
					 SDMMC_STAR_IDMABTC)

#define TIMEOUT_US_10_MS		10000U
#define TIMEOUT_US_1_S			1000000U

#define DT_SDMMC2_COMPAT		"st,stm32-sdmmc2"

static void stm32_sdmmc2_init(void);
static int stm32_sdmmc2_send_cmd_req(struct mmc_cmd *cmd);
static int stm32_sdmmc2_send_cmd(struct mmc_cmd *cmd);
static int stm32_sdmmc2_set_ios(unsigned int clk, unsigned int width);
static int stm32_sdmmc2_prepare(int lba, uintptr_t buf, size_t size);
static int stm32_sdmmc2_read(int lba, uintptr_t buf, size_t size);
static int stm32_sdmmc2_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops stm32_sdmmc2_ops = {
	.init		= stm32_sdmmc2_init,
	.send_cmd	= stm32_sdmmc2_send_cmd,
	.set_ios	= stm32_sdmmc2_set_ios,
	.prepare	= stm32_sdmmc2_prepare,
	.read		= stm32_sdmmc2_read,
	.write		= stm32_sdmmc2_write,
};

static struct stm32_sdmmc2_params sdmmc2_params;

#pragma weak plat_sdmmc2_use_dma
bool plat_sdmmc2_use_dma(unsigned int instance, unsigned int memory)
{
	return false;
}

static void stm32_sdmmc2_init(void)
{
	uint32_t clock_div;
	uintptr_t base = sdmmc2_params.reg_base;

	clock_div = div_round_up(sdmmc2_params.clk_rate,
				 STM32MP_MMC_INIT_FREQ * 2);

	mmio_write_32(base + SDMMC_CLKCR, SDMMC_CLKCR_HWFC_EN | clock_div |
		      sdmmc2_params.negedge |
		      sdmmc2_params.pin_ckin);

	mmio_write_32(base + SDMMC_POWER,
		      SDMMC_POWER_PWRCTRL | sdmmc2_params.dirpol);

	mdelay(1);
}

static int stm32_sdmmc2_stop_transfer(void)
{
	struct mmc_cmd cmd_stop;

	zeromem(&cmd_stop, sizeof(struct mmc_cmd));

	cmd_stop.cmd_idx = MMC_CMD(12);
	cmd_stop.resp_type = MMC_RESPONSE_R1B;

	return stm32_sdmmc2_send_cmd(&cmd_stop);
}

static int stm32_sdmmc2_send_cmd_req(struct mmc_cmd *cmd)
{
	uint64_t timeout;
	uint32_t flags_cmd, status;
	uint32_t flags_data = 0;
	int err = 0;
	uintptr_t base = sdmmc2_params.reg_base;
	unsigned int cmd_reg, arg_reg;

	if (cmd == NULL) {
		return -EINVAL;
	}

	flags_cmd = SDMMC_STAR_CTIMEOUT;
	arg_reg = cmd->cmd_arg;

	if ((mmio_read_32(base + SDMMC_CMDR) & SDMMC_CMDR_CPSMEN) != 0U) {
		mmio_write_32(base + SDMMC_CMDR, 0);
	}

	cmd_reg = cmd->cmd_idx | SDMMC_CMDR_CPSMEN;

	if (cmd->resp_type == 0U) {
		flags_cmd |= SDMMC_STAR_CMDSENT;
	}

	if ((cmd->resp_type & MMC_RSP_48) != 0U) {
		if ((cmd->resp_type & MMC_RSP_136) != 0U) {
			flags_cmd |= SDMMC_STAR_CMDREND;
			cmd_reg |= SDMMC_CMDR_WAITRESP;
		} else if ((cmd->resp_type & MMC_RSP_CRC) != 0U) {
			flags_cmd |= SDMMC_STAR_CMDREND | SDMMC_STAR_CCRCFAIL;
			cmd_reg |= SDMMC_CMDR_WAITRESP_SHORT;
		} else {
			flags_cmd |= SDMMC_STAR_CMDREND;
			cmd_reg |= SDMMC_CMDR_WAITRESP_SHORT_NOCRC;
		}
	}

	switch (cmd->cmd_idx) {
	case MMC_CMD(1):
		arg_reg |= OCR_POWERUP;
		break;
	case MMC_CMD(8):
		if (sdmmc2_params.device_info->mmc_dev_type == MMC_IS_EMMC) {
			cmd_reg |= SDMMC_CMDR_CMDTRANS;
		}
		break;
	case MMC_CMD(12):
		cmd_reg |= SDMMC_CMDR_CMDSTOP;
		break;
	case MMC_CMD(17):
	case MMC_CMD(18):
		cmd_reg |= SDMMC_CMDR_CMDTRANS;
		if (sdmmc2_params.use_dma) {
			flags_data |= SDMMC_STAR_DCRCFAIL |
				      SDMMC_STAR_DTIMEOUT |
				      SDMMC_STAR_DATAEND |
				      SDMMC_STAR_RXOVERR |
				      SDMMC_STAR_IDMATE;
		}
		break;
	case MMC_ACMD(41):
		arg_reg |= OCR_3_2_3_3 | OCR_3_3_3_4;
		break;
	case MMC_ACMD(51):
		cmd_reg |= SDMMC_CMDR_CMDTRANS;
		if (sdmmc2_params.use_dma) {
			flags_data |= SDMMC_STAR_DCRCFAIL |
				      SDMMC_STAR_DTIMEOUT |
				      SDMMC_STAR_DATAEND |
				      SDMMC_STAR_RXOVERR |
				      SDMMC_STAR_IDMATE |
				      SDMMC_STAR_DBCKEND;
		}
		break;
	default:
		break;
	}

	if ((cmd->resp_type & MMC_RSP_BUSY) != 0U) {
		mmio_write_32(base + SDMMC_DTIMER, UINT32_MAX);
	}

	mmio_write_32(base + SDMMC_ARGR, arg_reg);

	mmio_write_32(base + SDMMC_CMDR, cmd_reg);

	status = mmio_read_32(base + SDMMC_STAR);

	timeout = timeout_init_us(TIMEOUT_US_10_MS);

	while ((status & flags_cmd) == 0U) {
		if (timeout_elapsed(timeout)) {
			err = -ETIMEDOUT;
			ERROR("%s: timeout 10ms (cmd = %d,status = %x)\n",
			      __func__, cmd->cmd_idx, status);
			goto err_exit;
		}

		status = mmio_read_32(base + SDMMC_STAR);
	}

	if ((status & (SDMMC_STAR_CTIMEOUT | SDMMC_STAR_CCRCFAIL)) != 0U) {
		if ((status & SDMMC_STAR_CTIMEOUT) != 0U) {
			err = -ETIMEDOUT;
			/*
			 * Those timeouts can occur, and framework will handle
			 * the retries. CMD8 is expected to return this timeout
			 * for eMMC
			 */
			if (!((cmd->cmd_idx == MMC_CMD(1)) ||
			      (cmd->cmd_idx == MMC_CMD(13)) ||
			      ((cmd->cmd_idx == MMC_CMD(8)) &&
			       (cmd->resp_type == MMC_RESPONSE_R7)))) {
				ERROR("%s: CTIMEOUT (cmd = %d,status = %x)\n",
				      __func__, cmd->cmd_idx, status);
			}
		} else {
			err = -EIO;
			ERROR("%s: CRCFAIL (cmd = %d,status = %x)\n",
			      __func__, cmd->cmd_idx, status);
		}

		goto err_exit;
	}

	if ((cmd_reg & SDMMC_CMDR_WAITRESP) != 0U) {
		if ((cmd->cmd_idx == MMC_CMD(9)) &&
		    ((cmd_reg & SDMMC_CMDR_WAITRESP) == SDMMC_CMDR_WAITRESP)) {
			/* Need to invert response to match CSD structure */
			cmd->resp_data[0] = mmio_read_32(base + SDMMC_RESP4R);
			cmd->resp_data[1] = mmio_read_32(base + SDMMC_RESP3R);
			cmd->resp_data[2] = mmio_read_32(base + SDMMC_RESP2R);
			cmd->resp_data[3] = mmio_read_32(base + SDMMC_RESP1R);
		} else {
			cmd->resp_data[0] = mmio_read_32(base + SDMMC_RESP1R);
			if ((cmd_reg & SDMMC_CMDR_WAITRESP) ==
			    SDMMC_CMDR_WAITRESP) {
				cmd->resp_data[1] = mmio_read_32(base +
								 SDMMC_RESP2R);
				cmd->resp_data[2] = mmio_read_32(base +
								 SDMMC_RESP3R);
				cmd->resp_data[3] = mmio_read_32(base +
								 SDMMC_RESP4R);
			}
		}
	}

	if (flags_data == 0U) {
		mmio_write_32(base + SDMMC_ICR, SDMMC_STATIC_FLAGS);

		return 0;
	}

	status = mmio_read_32(base + SDMMC_STAR);

	timeout = timeout_init_us(TIMEOUT_US_10_MS);

	while ((status & flags_data) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%s: timeout 10ms (cmd = %d,status = %x)\n",
			      __func__, cmd->cmd_idx, status);
			err = -ETIMEDOUT;
			goto err_exit;
		}

		status = mmio_read_32(base + SDMMC_STAR);
	};

	if ((status & (SDMMC_STAR_DTIMEOUT | SDMMC_STAR_DCRCFAIL |
		       SDMMC_STAR_TXUNDERR | SDMMC_STAR_RXOVERR |
		       SDMMC_STAR_IDMATE)) != 0U) {
		ERROR("%s: Error flag (cmd = %d,status = %x)\n", __func__,
		      cmd->cmd_idx, status);
		err = -EIO;
	}

err_exit:
	mmio_write_32(base + SDMMC_ICR, SDMMC_STATIC_FLAGS);
	mmio_clrbits_32(base + SDMMC_CMDR, SDMMC_CMDR_CMDTRANS);

	if ((err != 0) && ((status & SDMMC_STAR_DPSMACT) != 0U)) {
		int ret_stop = stm32_sdmmc2_stop_transfer();

		if (ret_stop != 0) {
			return ret_stop;
		}
	}

	return err;
}

static int stm32_sdmmc2_send_cmd(struct mmc_cmd *cmd)
{
	int8_t retry;
	int err = 0;

	assert(cmd != NULL);

	for (retry = 0; retry <= 3; retry++) {
		err = stm32_sdmmc2_send_cmd_req(cmd);
		if (err == 0) {
			return err;
		}

		if ((cmd->cmd_idx == MMC_CMD(1)) ||
		    (cmd->cmd_idx == MMC_CMD(13))) {
			return 0; /* Retry managed by framework */
		}

		/* Command 8 is expected to fail for eMMC */
		if (!(cmd->cmd_idx == MMC_CMD(8))) {
			WARN(" CMD%d, Retry: %d, Error: %d\n",
			     cmd->cmd_idx, retry, err);
		}

		udelay(10);
	}

	return err;
}

static int stm32_sdmmc2_set_ios(unsigned int clk, unsigned int width)
{
	uintptr_t base = sdmmc2_params.reg_base;
	uint32_t bus_cfg = 0;
	uint32_t clock_div, max_freq;
	uint32_t clk_rate = sdmmc2_params.clk_rate;
	uint32_t max_bus_freq = sdmmc2_params.device_info->max_bus_freq;

	switch (width) {
	case MMC_BUS_WIDTH_1:
		break;
	case MMC_BUS_WIDTH_4:
		bus_cfg |= SDMMC_CLKCR_WIDBUS_4;
		break;
	case MMC_BUS_WIDTH_8:
		bus_cfg |= SDMMC_CLKCR_WIDBUS_8;
		break;
	default:
		panic();
		break;
	}

	if (sdmmc2_params.device_info->mmc_dev_type == MMC_IS_EMMC) {
		if (max_bus_freq >= 52000000U) {
			max_freq = STM32MP_EMMC_HIGH_SPEED_MAX_FREQ;
		} else {
			max_freq = STM32MP_EMMC_NORMAL_SPEED_MAX_FREQ;
		}
	} else {
		if (max_bus_freq >= 50000000U) {
			max_freq = STM32MP_SD_HIGH_SPEED_MAX_FREQ;
		} else {
			max_freq = STM32MP_SD_NORMAL_SPEED_MAX_FREQ;
		}
	}

	clock_div = div_round_up(clk_rate, max_freq * 2);

	mmio_write_32(base + SDMMC_CLKCR,
		      SDMMC_CLKCR_HWFC_EN | clock_div | bus_cfg |
		      sdmmc2_params.negedge |
		      sdmmc2_params.pin_ckin);

	return 0;
}

static int stm32_sdmmc2_prepare(int lba, uintptr_t buf, size_t size)
{
	struct mmc_cmd cmd;
	int ret;
	uintptr_t base = sdmmc2_params.reg_base;
	uint32_t data_ctrl = SDMMC_DCTRLR_DTDIR;

	if (size == 8U) {
		data_ctrl |= SDMMC_DBLOCKSIZE_8;
	} else {
		data_ctrl |= SDMMC_DBLOCKSIZE_512;
	}

	sdmmc2_params.use_dma = plat_sdmmc2_use_dma(base, buf);

	if (sdmmc2_params.use_dma) {
		inv_dcache_range(buf, size);
	}

	/* Prepare CMD 16*/
	mmio_write_32(base + SDMMC_DTIMER, 0);

	mmio_write_32(base + SDMMC_DLENR, 0);

	mmio_write_32(base + SDMMC_DCTRLR, 0);

	zeromem(&cmd, sizeof(struct mmc_cmd));

	cmd.cmd_idx = MMC_CMD(16);
	if (size > MMC_BLOCK_SIZE) {
		cmd.cmd_arg = MMC_BLOCK_SIZE;
	} else {
		cmd.cmd_arg = size;
	}

	cmd.resp_type = MMC_RESPONSE_R1;

	ret = stm32_sdmmc2_send_cmd(&cmd);
	if (ret != 0) {
		ERROR("CMD16 failed\n");
		return ret;
	}

	/* Prepare data command */
	mmio_write_32(base + SDMMC_DTIMER, UINT32_MAX);

	mmio_write_32(base + SDMMC_DLENR, size);

	if (sdmmc2_params.use_dma) {
		mmio_write_32(base + SDMMC_IDMACTRLR,
			      SDMMC_IDMACTRLR_IDMAEN);
		mmio_write_32(base + SDMMC_IDMABASE0R, buf);

		flush_dcache_range(buf, size);
	}

	mmio_clrsetbits_32(base + SDMMC_DCTRLR,
			   SDMMC_DCTRLR_CLEAR_MASK,
			   data_ctrl);

	return 0;
}

static int stm32_sdmmc2_read(int lba, uintptr_t buf, size_t size)
{
	uint32_t error_flags = SDMMC_STAR_RXOVERR | SDMMC_STAR_DCRCFAIL |
			       SDMMC_STAR_DTIMEOUT;
	uint32_t flags = error_flags | SDMMC_STAR_DATAEND;
	uint32_t status;
	uint32_t *buffer;
	uintptr_t base = sdmmc2_params.reg_base;
	uintptr_t fifo_reg = base + SDMMC_FIFOR;
	uint64_t timeout;
	int ret;

	/* Assert buf is 4 bytes aligned */
	assert((buf & GENMASK(1, 0)) == 0U);

	buffer = (uint32_t *)buf;

	if (sdmmc2_params.use_dma) {
		inv_dcache_range(buf, size);

		return 0;
	}

	if (size <= MMC_BLOCK_SIZE) {
		flags |= SDMMC_STAR_DBCKEND;
	}

	timeout = timeout_init_us(TIMEOUT_US_1_S);

	do {
		status = mmio_read_32(base + SDMMC_STAR);

		if ((status & error_flags) != 0U) {
			ERROR("%s: Read error (status = %x)\n", __func__,
			      status);
			mmio_write_32(base + SDMMC_DCTRLR,
				      SDMMC_DCTRLR_FIFORST);

			mmio_write_32(base + SDMMC_ICR,
				      SDMMC_STATIC_FLAGS);

			ret = stm32_sdmmc2_stop_transfer();
			if (ret != 0) {
				return ret;
			}

			return -EIO;
		}

		if (timeout_elapsed(timeout)) {
			ERROR("%s: timeout 1s (status = %x)\n",
			      __func__, status);
			mmio_write_32(base + SDMMC_ICR,
				      SDMMC_STATIC_FLAGS);

			ret = stm32_sdmmc2_stop_transfer();
			if (ret != 0) {
				return ret;
			}

			return -ETIMEDOUT;
		}

		if (size < (8U * sizeof(uint32_t))) {
			if ((mmio_read_32(base + SDMMC_DCNTR) > 0U) &&
			    ((status & SDMMC_STAR_RXFIFOE) == 0U)) {
				*buffer = mmio_read_32(fifo_reg);
				buffer++;
			}
		} else if ((status & SDMMC_STAR_RXFIFOHF) != 0U) {
			uint32_t count;

			/* Read data from SDMMC Rx FIFO */
			for (count = 0; count < 8U; count++) {
				*buffer = mmio_read_32(fifo_reg);
				buffer++;
			}
		}
	} while ((status & flags) == 0U);

	mmio_write_32(base + SDMMC_ICR, SDMMC_STATIC_FLAGS);

	if ((status & SDMMC_STAR_DPSMACT) != 0U) {
		WARN("%s: DPSMACT=1, send stop\n", __func__);
		return stm32_sdmmc2_stop_transfer();
	}

	return 0;
}

static int stm32_sdmmc2_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int stm32_sdmmc2_dt_get_config(void)
{
	int sdmmc_node;
	void *fdt = NULL;
	const fdt32_t *cuint;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	if (fdt == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	sdmmc_node = fdt_node_offset_by_compatible(fdt, -1, DT_SDMMC2_COMPAT);

	while (sdmmc_node != -FDT_ERR_NOTFOUND) {
		cuint = fdt_getprop(fdt, sdmmc_node, "reg", NULL);
		if (cuint == NULL) {
			continue;
		}

		if (fdt32_to_cpu(*cuint) == sdmmc2_params.reg_base) {
			break;
		}

		sdmmc_node = fdt_node_offset_by_compatible(fdt, sdmmc_node,
							   DT_SDMMC2_COMPAT);
	}

	if (sdmmc_node == -FDT_ERR_NOTFOUND) {
		return -FDT_ERR_NOTFOUND;
	}

	if (fdt_get_status(sdmmc_node) == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	if (dt_set_pinctrl_config(sdmmc_node) != 0) {
		return -FDT_ERR_BADVALUE;
	}

	cuint = fdt_getprop(fdt, sdmmc_node, "clocks", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint++;
	sdmmc2_params.clock_id = fdt32_to_cpu(*cuint);

	cuint = fdt_getprop(fdt, sdmmc_node, "resets", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint++;
	sdmmc2_params.reset_id = fdt32_to_cpu(*cuint);

	if ((fdt_getprop(fdt, sdmmc_node, "st,use-ckin", NULL)) != NULL) {
		sdmmc2_params.pin_ckin = SDMMC_CLKCR_SELCLKRX_0;
	}

	if ((fdt_getprop(fdt, sdmmc_node, "st,sig-dir", NULL)) != NULL) {
		sdmmc2_params.dirpol = SDMMC_POWER_DIRPOL;
	}

	if ((fdt_getprop(fdt, sdmmc_node, "st,neg-edge", NULL)) != NULL) {
		sdmmc2_params.negedge = SDMMC_CLKCR_NEGEDGE;
	}

	cuint = fdt_getprop(fdt, sdmmc_node, "bus-width", NULL);
	if (cuint != NULL) {
		switch (fdt32_to_cpu(*cuint)) {
		case 4:
			sdmmc2_params.bus_width = MMC_BUS_WIDTH_4;
			break;

		case 8:
			sdmmc2_params.bus_width = MMC_BUS_WIDTH_8;
			break;

		default:
			break;
		}
	}

	return 0;
}

unsigned long long stm32_sdmmc2_mmc_get_device_size(void)
{
	return sdmmc2_params.device_info->device_size;
}

int stm32_sdmmc2_mmc_init(struct stm32_sdmmc2_params *params)
{
	assert((params != NULL) &&
	       ((params->reg_base & MMC_BLOCK_MASK) == 0U) &&
	       ((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

	memcpy(&sdmmc2_params, params, sizeof(struct stm32_sdmmc2_params));

	if (stm32_sdmmc2_dt_get_config() != 0) {
		ERROR("%s: DT error\n", __func__);
		return -ENOMEM;
	}

	stm32mp_clk_enable(sdmmc2_params.clock_id);

	stm32mp_reset_assert(sdmmc2_params.reset_id);
	udelay(2);
	stm32mp_reset_deassert(sdmmc2_params.reset_id);
	mdelay(1);

	sdmmc2_params.clk_rate = stm32mp_clk_get_rate(sdmmc2_params.clock_id);
	sdmmc2_params.device_info->ocr_voltage = OCR_3_2_3_3 | OCR_3_3_3_4;

	return mmc_init(&stm32_sdmmc2_ops, sdmmc2_params.clk_rate,
			sdmmc2_params.bus_width, sdmmc2_params.flags,
			sdmmc2_params.device_info);
}

/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <drivers/io/io_block.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "uniphier.h"

#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_READ_MULTIPLE_BLOCK	18

#define EXT_CSD_PART_CONF		179	/* R/W */

#define MMC_RSP_PRESENT BIT(0)
#define MMC_RSP_136	BIT(1)		/* 136 bit response */
#define MMC_RSP_CRC	BIT(2)		/* expect valid crc */
#define MMC_RSP_BUSY	BIT(3)		/* card may send busy */
#define MMC_RSP_OPCODE	BIT(4)		/* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)

#define SDHCI_DMA_ADDRESS	0x00
#define SDHCI_BLOCK_SIZE	0x04
#define  SDHCI_MAKE_BLKSZ(dma, blksz) ((((dma) & 0x7) << 12) | ((blksz) & 0xFFF))
#define SDHCI_BLOCK_COUNT	0x06
#define SDHCI_ARGUMENT		0x08
#define SDHCI_TRANSFER_MODE	0x0C
#define  SDHCI_TRNS_DMA		BIT(0)
#define  SDHCI_TRNS_BLK_CNT_EN	BIT(1)
#define  SDHCI_TRNS_ACMD12	BIT(2)
#define  SDHCI_TRNS_READ	BIT(4)
#define  SDHCI_TRNS_MULTI	BIT(5)
#define SDHCI_COMMAND		0x0E
#define  SDHCI_CMD_RESP_MASK	0x03
#define  SDHCI_CMD_CRC		0x08
#define  SDHCI_CMD_INDEX	0x10
#define  SDHCI_CMD_DATA		0x20
#define  SDHCI_CMD_ABORTCMD	0xC0
#define  SDHCI_CMD_RESP_NONE	0x00
#define  SDHCI_CMD_RESP_LONG	0x01
#define  SDHCI_CMD_RESP_SHORT	0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03
#define  SDHCI_MAKE_CMD(c, f) ((((c) & 0xff) << 8) | ((f) & 0xff))
#define SDHCI_RESPONSE		0x10
#define SDHCI_HOST_CONTROL	0x28
#define  SDHCI_CTRL_DMA_MASK	0x18
#define   SDHCI_CTRL_SDMA	0x00
#define SDHCI_BLOCK_GAP_CONTROL	0x2A
#define SDHCI_SOFTWARE_RESET	0x2F
#define  SDHCI_RESET_CMD	0x02
#define  SDHCI_RESET_DATA	0x04
#define SDHCI_INT_STATUS	0x30
#define  SDHCI_INT_RESPONSE	BIT(0)
#define  SDHCI_INT_DATA_END	BIT(1)
#define  SDHCI_INT_DMA_END	BIT(3)
#define  SDHCI_INT_ERROR	BIT(15)
#define SDHCI_SIGNAL_ENABLE	0x38

/* RCA assigned by Boot ROM */
#define UNIPHIER_EMMC_RCA	0x1000

struct uniphier_mmc_cmd {
	unsigned int cmdidx;
	unsigned int resp_type;
	unsigned int cmdarg;
	unsigned int is_data;
};

static int uniphier_emmc_block_addressing;

static int uniphier_emmc_send_cmd(uintptr_t host_base,
				  struct uniphier_mmc_cmd *cmd)
{
	uint32_t mode = 0;
	uint32_t end_bit;
	uint32_t stat, flags, dma_addr;

	mmio_write_32(host_base + SDHCI_INT_STATUS, -1);
	mmio_write_32(host_base + SDHCI_SIGNAL_ENABLE, 0);
	mmio_write_32(host_base + SDHCI_ARGUMENT, cmd->cmdarg);

	if (cmd->is_data)
		mode = SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN |
			SDHCI_TRNS_ACMD12 | SDHCI_TRNS_READ |
			SDHCI_TRNS_MULTI;

	mmio_write_16(host_base + SDHCI_TRANSFER_MODE, mode);

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
	else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (cmd->is_data)
		flags |= SDHCI_CMD_DATA;

	if (cmd->resp_type & MMC_RSP_BUSY || cmd->is_data)
		end_bit = SDHCI_INT_DATA_END;
	else
		end_bit = SDHCI_INT_RESPONSE;

	mmio_write_16(host_base + SDHCI_COMMAND,
		      SDHCI_MAKE_CMD(cmd->cmdidx, flags));

	do {
		stat = mmio_read_32(host_base + SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			return -EIO;

		if (stat & SDHCI_INT_DMA_END) {
			mmio_write_32(host_base + SDHCI_INT_STATUS, stat);
			dma_addr = mmio_read_32(host_base + SDHCI_DMA_ADDRESS);
			mmio_write_32(host_base + SDHCI_DMA_ADDRESS, dma_addr);
		}
	} while (!(stat & end_bit));

	return 0;
}

static int uniphier_emmc_switch_part(uintptr_t host_base, int part_num)
{
	struct uniphier_mmc_cmd cmd = {0};

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (EXT_CSD_PART_CONF << 16) | (part_num << 8) | (3 << 24);

	return uniphier_emmc_send_cmd(host_base, &cmd);
}

static int uniphier_emmc_is_over_2gb(uintptr_t host_base)
{
	struct uniphier_mmc_cmd cmd = {0};
	uint32_t csd40, csd72;	/* CSD[71:40], CSD[103:72] */
	int ret;

	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = UNIPHIER_EMMC_RCA << 16;

	ret = uniphier_emmc_send_cmd(host_base, &cmd);
	if (ret)
		return ret;

	csd40 = mmio_read_32(host_base + SDHCI_RESPONSE + 4);
	csd72 = mmio_read_32(host_base + SDHCI_RESPONSE + 8);

	return !(~csd40 & 0xffc00380) && !(~csd72 & 0x3);
}

static int uniphier_emmc_load_image(uintptr_t host_base,
				    uint32_t dev_addr,
				    unsigned long load_addr,
				    uint32_t block_cnt)
{
	struct uniphier_mmc_cmd cmd = {0};
	uint8_t tmp;

	assert((load_addr >> 32) == 0);

	mmio_write_32(host_base + SDHCI_DMA_ADDRESS, load_addr);
	mmio_write_16(host_base + SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, 512));
	mmio_write_16(host_base + SDHCI_BLOCK_COUNT, block_cnt);

	tmp = mmio_read_8(host_base + SDHCI_HOST_CONTROL);
	tmp &= ~SDHCI_CTRL_DMA_MASK;
	tmp |= SDHCI_CTRL_SDMA;
	mmio_write_8(host_base + SDHCI_HOST_CONTROL, tmp);

	tmp = mmio_read_8(host_base + SDHCI_BLOCK_GAP_CONTROL);
	tmp &= ~1;		/* clear Stop At Block Gap Request */
	mmio_write_8(host_base + SDHCI_BLOCK_GAP_CONTROL, tmp);

	cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = dev_addr;
	cmd.is_data = 1;

	return uniphier_emmc_send_cmd(host_base, &cmd);
}

static size_t uniphier_emmc_read(int lba, uintptr_t buf, size_t size)
{
	uintptr_t host_base = 0x5a000200;
	int ret;

	inv_dcache_range(buf, size);

	if (!uniphier_emmc_block_addressing)
		lba *= 512;

	ret = uniphier_emmc_load_image(host_base, lba, buf, size / 512);

	inv_dcache_range(buf, size);

	return ret ? 0 : size;
}

static const struct io_block_dev_spec uniphier_emmc_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_BLOCK_BUF_BASE,
		.length = UNIPHIER_BLOCK_BUF_SIZE,
	},
	.ops = {
		.read = uniphier_emmc_read,
	},
	.block_size = 512,
};

static int uniphier_emmc_hw_init(void)
{
	uintptr_t host_base = 0x5a000200;
	struct uniphier_mmc_cmd cmd = {0};
	int ret;

	/*
	 * deselect card before SEND_CSD command.
	 * Do not check the return code.  It fails, but it is OK.
	 */
	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1;

	uniphier_emmc_send_cmd(host_base, &cmd); /* CMD7 (arg=0) */

	/* reset CMD Line */
	mmio_write_8(host_base + SDHCI_SOFTWARE_RESET,
		     SDHCI_RESET_CMD | SDHCI_RESET_DATA);
	while (mmio_read_8(host_base + SDHCI_SOFTWARE_RESET))
		;

	ret = uniphier_emmc_is_over_2gb(host_base);
	if (ret < 0)
		return ret;

	uniphier_emmc_block_addressing = ret;

	cmd.cmdarg = UNIPHIER_EMMC_RCA << 16;

	/* select card again */
	ret = uniphier_emmc_send_cmd(host_base, &cmd);
	if (ret)
		return ret;

	/* switch to Boot Partition 1 */
	ret = uniphier_emmc_switch_part(host_base, 1);
	if (ret)
		return ret;

	return 0;
}

int uniphier_emmc_init(uintptr_t *block_dev_spec)
{
	int ret;

	ret = uniphier_emmc_hw_init();
	if (ret)
		return ret;

	*block_dev_spec = (uintptr_t)&uniphier_emmc_dev_spec;

	return 0;
}

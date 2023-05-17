/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_nand.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <platform_def.h>

/* NAND flash device information struct */
static cnf_dev_info_t dev_info;

/* Scratch buffers for read and write operations */
static uint8_t scratch_buff[PLATFORM_MTD_MAX_PAGE_SIZE];

/* Wait for controller to be in idle state */
static inline void cdns_nand_wait_idle(void)
{
	uint32_t reg = 0U;

	do {
		udelay(CNF_DEF_DELAY_US);
		reg = mmio_read_32(CNF_CMDREG(CTRL_STATUS));
	} while (CNF_GET_CTRL_BUSY(reg) != 0U);
}

/* Wait for given thread to be in ready state */
static inline void cdns_nand_wait_thread_ready(uint8_t thread_id)
{
	uint32_t reg = 0U;

	do {
		udelay(CNF_DEF_DELAY_US);
		reg = mmio_read_32(CNF_CMDREG(TRD_STATUS));
		reg &= (1U << (uint32_t)thread_id);
	} while (reg != 0U);
}

/* Check if the last operation/command in selected thread is completed */
static int cdns_nand_last_opr_status(uint8_t thread_id)
{
	uint8_t nthreads = 0U;
	uint32_t reg = 0U;

	/* Get number of threads */
	reg = mmio_read_32(CNF_CTRLPARAM(FEATURE));
	nthreads = CNF_GET_NTHREADS(reg);

	if (thread_id > nthreads) {
		ERROR("%s: Invalid thread ID\n", __func__);
		return -EINVAL;
	}

	/* Select thread */
	mmio_write_32(CNF_CMDREG(CMD_STAT_PTR), (uint32_t)thread_id);

	uint32_t err_mask = CNF_ECMD | CNF_EECC | CNF_EDEV | CNF_EDQS | CNF_EFAIL |
				CNF_EBUS | CNF_EDI | CNF_EPAR | CNF_ECTX | CNF_EPRO;

	do {
		udelay(CNF_DEF_DELAY_US * 2);
		reg = mmio_read_32(CNF_CMDREG(CMD_STAT));
	} while ((reg & CNF_CMPLT) == 0U);

	/* last operation is completed, make sure no other error bits are set */
	if ((reg & err_mask) == 1U) {
		ERROR("%s, CMD_STATUS:0x%x\n", __func__, reg);
		return -EIO;
	}

	return 0;
}

/* Set feature command */
int cdns_nand_set_feature(uint8_t feat_addr, uint8_t feat_val, uint8_t thread_id)
{
	/* Wait for thread to be ready */
	cdns_nand_wait_thread_ready(thread_id);

	/* Set feature address */
	mmio_write_32(CNF_CMDREG(CMD_REG1), (uint32_t)feat_addr);
	/* Set feature volume */
	mmio_write_32(CNF_CMDREG(CMD_REG2), (uint32_t)feat_val);

	/* Set feature command */
	uint32_t reg = (CNF_WORK_MODE_PIO << CNF_CMDREG0_CT);

	reg |= (thread_id << CNF_CMDREG0_TRD);
	reg |= (CNF_DEF_VOL_ID << CNF_CMDREG0_VOL);
	reg |= (CNF_INT_DIS << CNF_CMDREG0_INTR);
	reg |= (CNF_CT_SET_FEATURE << CNF_CMDREG0_CMD);
	mmio_write_32(CNF_CMDREG(CMD_REG0), reg);

	return cdns_nand_last_opr_status(thread_id);
}

/* Reset command to the selected device */
int cdns_nand_reset(uint8_t thread_id)
{
	/* Operation is executed in selected thread */
	cdns_nand_wait_thread_ready(thread_id);

	/* Select memory */
	mmio_write_32(CNF_CMDREG(CMD_REG4), (CNF_DEF_DEVICE << CNF_CMDREG4_MEM));

	/* Issue reset command */
	uint32_t reg = (CNF_WORK_MODE_PIO << CNF_CMDREG0_CT);

	reg |= (thread_id << CNF_CMDREG0_TRD);
	reg |= (CNF_DEF_VOL_ID << CNF_CMDREG0_VOL);
	reg |= (CNF_INT_DIS << CNF_CMDREG0_INTR);
	reg |= (CNF_CT_RESET_ASYNC << CNF_CMDREG0_CMD);
	mmio_write_32(CNF_CMDREG(CMD_REG0), reg);

	return cdns_nand_last_opr_status(thread_id);
}

/* Set operation work mode */
static void cdns_nand_set_opr_mode(uint8_t opr_mode)
{
	/* Wait for controller to be in idle state */
	cdns_nand_wait_idle();

	/* Reset DLL PHY */
	uint32_t reg = mmio_read_32(CNF_MINICTRL(DLL_PHY_CTRL));

	reg &= ~(1 << CNF_DLL_PHY_RST_N);
	mmio_write_32(CNF_MINICTRL(DLL_PHY_CTRL), reg);

	if (opr_mode == CNF_OPR_WORK_MODE_SDR) {
		/* Combo PHY Control Timing Block register settings */
		mmio_write_32(CP_CTB(CTRL_REG), CP_CTRL_REG_SDR);
		mmio_write_32(CP_CTB(TSEL_REG), CP_TSEL_REG_SDR);

		/* Combo PHY DLL register settings */
		mmio_write_32(CP_DLL(DQ_TIMING_REG), CP_DQ_TIMING_REG_SDR);
		mmio_write_32(CP_DLL(DQS_TIMING_REG), CP_DQS_TIMING_REG_SDR);
		mmio_write_32(CP_DLL(GATE_LPBK_CTRL_REG), CP_GATE_LPBK_CTRL_REG_SDR);
		mmio_write_32(CP_DLL(MASTER_CTRL_REG), CP_DLL_MASTER_CTRL_REG_SDR);

		/* Async mode timing settings */
		mmio_write_32(CNF_MINICTRL(ASYNC_TOGGLE_TIMINGS),
								(2 << CNF_ASYNC_TIMINGS_TRH) |
								(4 << CNF_ASYNC_TIMINGS_TRP) |
								(2 << CNF_ASYNC_TIMINGS_TWH) |
								(4 << CNF_ASYNC_TIMINGS_TWP));

		/* Set extended read and write mode */
		reg |= (1 << CNF_DLL_PHY_EXT_RD_MODE);
		reg |= (1 << CNF_DLL_PHY_EXT_WR_MODE);

		/* Set operation work mode in common settings */
		uint32_t data = mmio_read_32(CNF_MINICTRL(CMN_SETTINGS));

		data |= (CNF_OPR_WORK_MODE_SDR << CNF_CMN_SETTINGS_OPR);
		mmio_write_32(CNF_MINICTRL(CMN_SETTINGS), data);

	} else if (opr_mode == CNF_OPR_WORK_MODE_NVDDR) {
		; /* ToDo: add DDR mode settings also once available on SIMICS */
	} else {
		;
	}

	reg |= (1 << CNF_DLL_PHY_RST_N);
	mmio_write_32(CNF_MINICTRL(DLL_PHY_CTRL), reg);
}

/* Data transfer configuration */
static void cdns_nand_transfer_config(void)
{
	/* Wait for controller to be in idle state */
	cdns_nand_wait_idle();

	/* Configure data transfer parameters */
	mmio_write_32(CNF_CTRLCFG(TRANS_CFG0), 1);

	/* ECC is disabled */
	mmio_write_32(CNF_CTRLCFG(ECC_CFG0), 0);

	/* DMA burst select */
	mmio_write_32(CNF_CTRLCFG(DMA_SETTINGS),
					(CNF_DMA_BURST_SIZE_MAX << CNF_DMA_SETTINGS_BURST) |
					(1 << CNF_DMA_SETTINGS_OTE));

	/* Enable pre-fetching for 1K */
	mmio_write_32(CNF_CTRLCFG(FIFO_TLEVEL),
					(CNF_DMA_PREFETCH_SIZE << CNF_FIFO_TLEVEL_POS) |
					(CNF_DMA_PREFETCH_SIZE << CNF_FIFO_TLEVEL_DMA_SIZE));

	/* Select access type */
	mmio_write_32(CNF_CTRLCFG(MULTIPLANE_CFG), 0);
	mmio_write_32(CNF_CTRLCFG(CACHE_CFG), 0);
}

/* Update the nand flash device info */
static int cdns_nand_update_dev_info(void)
{
	uint32_t reg = 0U;

	/* Read the device type and number of LUNs */
	reg = mmio_read_32(CNF_CTRLPARAM(DEV_PARAMS0));
	dev_info.type = CNF_GET_DEV_TYPE(reg);
	if (dev_info.type == CNF_DT_UNKNOWN) {
		ERROR("%s: device type unknown\n", __func__);
		return -ENXIO;
	}
	dev_info.nluns = CNF_GET_NLUNS(reg);

	/* Pages per block */
	reg = mmio_read_32(CNF_CTRLCFG(DEV_LAYOUT));
	dev_info.npages_per_block = CNF_GET_NPAGES_PER_BLOCK(reg);

	/* Sector size and last sector size */
	reg = mmio_read_32(CNF_CTRLCFG(TRANS_CFG1));
	dev_info.sector_size = CNF_GET_SCTR_SIZE(reg);
	dev_info.last_sector_size = CNF_GET_LAST_SCTR_SIZE(reg);

	/* Page size and spare size */
	reg = mmio_read_32(CNF_CTRLPARAM(DEV_AREA));
	dev_info.page_size = CNF_GET_PAGE_SIZE(reg);
	dev_info.spare_size = CNF_GET_SPARE_SIZE(reg);

	/* Device blocks per LUN */
	dev_info.nblocks_per_lun = mmio_read_32(CNF_CTRLPARAM(DEV_BLOCKS_PLUN));

	/* Calculate block size and total device size */
	dev_info.block_size = (dev_info.npages_per_block * dev_info.page_size);
	dev_info.total_size = (dev_info.block_size * dev_info.nblocks_per_lun *
							dev_info.nluns);

	VERBOSE("CNF params: page %d, spare %d, block %d, total %lld\n",
				dev_info.page_size, dev_info.spare_size,
				dev_info.block_size, dev_info.total_size);

	return 0;
}

/* NAND Flash Controller/Host initialization */
int cdns_nand_host_init(void)
{
	uint32_t reg = 0U;
	int ret = 0;

	do {
		/* Read controller status register for init complete */
		reg = mmio_read_32(CNF_CMDREG(CTRL_STATUS));
	} while (CNF_GET_INIT_COMP(reg) == 0);

	ret = cdns_nand_update_dev_info();
	if (ret != 0) {
		return ret;
	}

	INFO("CNF: device discovery process completed and device type %d\n",
			dev_info.type);

	/* Enable data integrity, enable CRC and parity */
	reg = mmio_read_32(CNF_DI(CONTROL));
	reg |= (1 << CNF_DI_PAR_EN);
	reg |= (1 << CNF_DI_CRC_EN);
	mmio_write_32(CNF_DI(CONTROL), reg);

	/* Status polling mode, device control and status register */
	cdns_nand_wait_idle();
	reg = mmio_read_32(CNF_CTRLCFG(DEV_STAT));
	reg = reg & ~1;
	mmio_write_32(CNF_CTRLCFG(DEV_STAT), reg);

	/* Set operation work mode */
	cdns_nand_set_opr_mode(CNF_OPR_WORK_MODE_SDR);

	/* Set data transfer configuration parameters */
	cdns_nand_transfer_config();

	return 0;
}

/* erase: Block erase command */
int cdns_nand_erase(uint32_t offset, uint32_t size)
{
	/* Determine the starting block offset i.e row address */
	uint32_t row_address = dev_info.npages_per_block * offset;

	/* Wait for thread to be in ready state */
	cdns_nand_wait_thread_ready(CNF_DEF_TRD);

	/*Set row address */
	mmio_write_32(CNF_CMDREG(CMD_REG1), row_address);

	/* Operation bank number */
	mmio_write_32(CNF_CMDREG(CMD_REG4), (CNF_DEF_DEVICE << CNF_CMDREG4_MEM));

	/* Block erase command */
	uint32_t reg = (CNF_WORK_MODE_PIO << CNF_CMDREG0_CT);

	reg |= (CNF_DEF_TRD << CNF_CMDREG0_TRD);
	reg |= (CNF_DEF_VOL_ID << CNF_CMDREG0_VOL);
	reg |= (CNF_INT_DIS << CNF_CMDREG0_INTR);
	reg |= (CNF_CT_ERASE << CNF_CMDREG0_CMD);
	reg |= (((size-1) & 0xFF) << CNF_CMDREG0_CMD);
	mmio_write_32(CNF_CMDREG(CMD_REG0), reg);

	/* Wait for erase operation to complete */
	return cdns_nand_last_opr_status(CNF_DEF_TRD);
}

/* io mtd functions */
int cdns_nand_init_mtd(unsigned long long *size, unsigned int *erase_size)
{
	*size = dev_info.total_size;
	*erase_size = dev_info.block_size;

	return 0;
}

/* NAND Flash page read */
static int cdns_nand_read_page(uint32_t block, uint32_t page, uintptr_t buffer)
{
	/* Wait for thread to be ready */
	cdns_nand_wait_thread_ready(CNF_DEF_TRD);

	/* Select device */
	mmio_write_32(CNF_CMDREG(CMD_REG4),
					(CNF_DEF_DEVICE << CNF_CMDREG4_MEM));

	/* Set host memory address for DMA transfers */
	mmio_write_32(CNF_CMDREG(CMD_REG2), (buffer & 0xFFFF));
	mmio_write_32(CNF_CMDREG(CMD_REG3), ((buffer >> 32) & 0xFFFF));

	/* Set row address */
	uint32_t row_address = 0U;

	row_address |= ((page & 0x3F) | (block << 6));
	mmio_write_32(CNF_CMDREG(CMD_REG1), row_address);

	/* Page read command */
	uint32_t reg = (CNF_WORK_MODE_PIO << CNF_CMDREG0_CT);

	reg |= (CNF_DEF_TRD << CNF_CMDREG0_TRD);
	reg |= (CNF_DEF_VOL_ID << CNF_CMDREG0_VOL);
	reg |= (CNF_INT_DIS << CNF_CMDREG0_INTR);
	reg |= (CNF_DMA_MASTER_SEL << CNF_CMDREG0_DMA);
	reg |= (CNF_CT_PAGE_READ << CNF_CMDREG0_CMD);
	reg |= (((CNF_READ_SINGLE_PAGE-1) & 0xFF) << CNF_CMDREG0_CMD);
	mmio_write_32(CNF_CMDREG(CMD_REG0), reg);

	/* Wait for read operation to complete */
	if (cdns_nand_last_opr_status(CNF_DEF_TRD)) {
		ERROR("%s: Page read failed\n", __func__);
		return -EIO;
	}

	return 0;
}

int cdns_nand_read(unsigned int offset, uintptr_t buffer, size_t length,
					size_t *out_length)
{
	uint32_t block = offset / dev_info.block_size;
	uint32_t end_block = (offset + length - 1U) / dev_info.block_size;
	uint32_t page_start = (offset % dev_info.block_size) / dev_info.page_size;
	uint32_t start_offset = offset % dev_info.page_size;
	uint32_t nb_pages = dev_info.block_size / dev_info.page_size;
	uint32_t bytes_read = 0U;
	uint32_t page = 0U;
	int result = 0;

	VERBOSE("CNF: block %u-%u, page_start %u, len %zu, offset %u\n",
				block, end_block, page_start, length, offset);

	if ((offset >= dev_info.total_size) ||
		(offset + length-1 >= dev_info.total_size) ||
		(length == 0U)) {
		ERROR("CNF: Invalid read parameters\n");
		return -EINVAL;
	}

	*out_length = 0UL;

	while (block <= end_block) {
		for (page = page_start; page < nb_pages; page++) {
			if ((start_offset != 0U) || (length < dev_info.page_size)) {
				/* Partial page read */
				result = cdns_nand_read_page(block, page,
				(uintptr_t)scratch_buff);
				if (result != 0) {
					return result;
				}

				bytes_read = MIN((size_t)(dev_info.page_size - start_offset),
								length);

				memcpy((uint8_t *)buffer, scratch_buff + start_offset,
						bytes_read);
				start_offset = 0U;
			} else {
				/* Full page read */
				result = cdns_nand_read_page(block, page,
				(uintptr_t)scratch_buff);
				if (result != 0) {
					return result;
				}

				bytes_read = dev_info.page_size;
				memcpy((uint8_t *)buffer, scratch_buff, bytes_read);
			}

			length -= bytes_read;
			buffer += bytes_read;
			*out_length += bytes_read;

			/* All the bytes have read */
			if (length == 0U) {
				break;
			}

			udelay(CNF_READ_INT_DELAY_US);
		} /* for */

		page_start = 0U;
		block++;
	} /* while */

	return 0;
}

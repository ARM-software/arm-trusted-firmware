/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
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

/*
 * Scratch buffers for read and write operations
 * DMA transfer of Cadence NAND expects data 8 bytes aligned
 * to be written to register.
 */
static uint8_t *scratch_buff = (uint8_t *)PLAT_NAND_SCRATCH_BUFF;

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

static inline uint32_t cdns_nand_get_thread_status(uint8_t thread_id)
{
	uint32_t status = 0U;

	/* Select thread */
	mmio_write_32(CNF_CMDREG(CMD_STAT_PTR), (uint32_t)thread_id);

	/* Get last command status. */
	status = mmio_read_32(CNF_CMDREG(CMD_STAT));

	return status;
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
	mmio_write_32(CNF_CMDREG(CMD_REG4),
			(CNF_DEF_DEVICE << CNF_CMDREG4_MEM));

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
static void cdns_nand_set_opr_mode(void)
{
	/* Wait for controller to be in idle state */
	cdns_nand_wait_idle();
	/* NAND mini controller settings for SDR mode and Combo PHY settings. */
	mmio_write_32(CNF_MINICTRL(ASYNC_TOGGLE_TIMINGS), CNF_ASYNC_TOGGLE_TIMINGS_VAL);
	mmio_write_32(CNF_MINICTRL(TIMINGS0), CNF_MINICTRL_TIMINGS0_VAL);
	mmio_write_32(CNF_MINICTRL(TIMINGS1), CNF_MINICTRL_TIMINGS1_VAL);
	mmio_write_32(CNF_MINICTRL(TIMINGS2), CNF_MINICTRL_TIMINGS2_VAL);
	mmio_write_32(CNF_MINICTRL(DLL_PHY_CTRL), CNF_DLL_PHY_CTRL_VAL);
	mmio_write_32(CP_CTB(CTRL_REG), CP_CTRL_REG_SDR);
	mmio_write_32(CP_CTB(TSEL_REG), CP_TSEL_REG_SDR);
	mmio_write_32(CP_DLL(DQ_TIMING_REG), CP_DQ_TIMING_REG_SDR);
	mmio_write_32(CP_DLL(DQS_TIMING_REG), CP_DQS_TIMING_REG_SDR);
	mmio_write_32(CP_DLL(GATE_LPBK_CTRL_REG), CP_GATE_LPBK_CTRL_REG_SDR);
	mmio_write_32(CP_DLL(MASTER_CTRL_REG), CP_DLL_MASTER_CTRL_REG_SDR);
	mmio_write_32(CP_DLL(SLAVE_CTRL_REG), CP_DLL_SLAVE_CTRL_REG_SDR);

	/* Wait for controller to be in idle state */
	cdns_nand_wait_idle();
	/* Set operation work mode in common settings to SDR. */
	mmio_clrbits_32(CNF_MINICTRL(CMN_SETTINGS), (BIT(1) | BIT(0)));
}

/* Data transfer configuration */
static void cdns_nand_transfer_config(void)
{
	/* Wait for controller to be in idle state */
	cdns_nand_wait_idle();

	/* DMA burst select */
	mmio_write_32(CNF_CTRLCFG(DMA_SETTINGS),
			(CNF_DMA_BURST_SIZE_MAX << CNF_DMA_SETTINGS_BURST) |
			(1 << CNF_DMA_SETTINGS_OTE));

	/* Enable pre-fetching for 1K */
	mmio_write_32(CNF_CTRLCFG(FIFO_TLEVEL),
			(CNF_DMA_PREFETCH_SIZE << CNF_FIFO_TLEVEL_POS) |
			(CNF_DMA_PREFETCH_SIZE << CNF_FIFO_TLEVEL_DMA_SIZE));

	/* Disable cache and multi-plane operations. */
	mmio_write_32(CNF_CTRLCFG(MULTIPLANE_CFG), 0);
	mmio_write_32(CNF_CTRLCFG(CACHE_CFG), 0);

	/* ECC engine configuration. */
	mmio_write_32(CNF_CTRLCFG(ECC_CFG0), CNF_ECC_CFG0_VAL);

	/* Skip bytes details update - bytes, marker and offset. */
	mmio_write_32(CNF_MINICTRL(SKIP_BYTES_CFG), CNF_SKIP_BYTES_CFG_VAL);
	mmio_write_32(CNF_MINICTRL(SKIP_BYTES_OFFSET), CNF_SKIP_BYTES_OFFSET_VAL);

	/* Transfer config - sector count, sector size, last sector size. */
	mmio_write_32(CNF_CTRLCFG(TRANS_CFG0), CNF_TRANS_CFG0_VAL);
	mmio_write_32(CNF_CTRLCFG(TRANS_CFG1), CNF_TRANS_CFG1_VAL);

	/* Disable pre-fetching. */
	cdns_nand_wait_idle();
	mmio_write_32(CNF_CTRLCFG(FIFO_TLEVEL), 0);
}

/* Update the nand flash device info */
static int cdns_nand_update_dev_info(void)
{
	uint32_t reg = 0U;
	static const char *const device_type[] = {
		"Unknown",
		"ONFI",
		"JEDEC/Toggle",
		"Legacy"
	};

	NOTICE("CNF: NAND Flash Device details\n");

	/* Get Manufacturer ID and Device ID. */
	reg = mmio_read_32(CNF_CTRLPARAM(MFR_ID));
	dev_info.mfr_id = FIELD_GET(CNF_MFR_ID_MASK, reg);
	dev_info.dev_id = FIELD_GET(CNF_DEV_ID_MASK, reg);
	INFO(" -- Manufacturer ID: 0x%02x\n", dev_info.mfr_id);
	INFO(" -- Device ID: 0x%02x\n", dev_info.dev_id);

	/* Read the Device type and number of LUNs. */
	reg = mmio_read_32(CNF_CTRLPARAM(DEV_PARAMS0));
	dev_info.type = CNF_GET_DEV_TYPE(reg);
	NOTICE(" -- Device type '%s' detected\n", device_type[dev_info.type]);
	if (dev_info.type == CNF_DT_UNKNOWN) {
		ERROR("CNF: Device type is 'Unknown', exit\n");
		return -ENXIO;
	}
	dev_info.nluns = CNF_GET_NLUNS(reg);

	/* Pages per block - number of pages in a block. */
	reg = mmio_read_32(CNF_CTRLCFG(DEV_LAYOUT));
	dev_info.npages_per_block = CNF_GET_NPAGES_PER_BLOCK(reg);
	INFO(" -- Pages per block: %d\n", dev_info.npages_per_block);

	/* Sector size and last sector size */
	reg = mmio_read_32(CNF_CTRLCFG(TRANS_CFG1));
	dev_info.sector_size = CNF_GET_SCTR_SIZE(reg);
	dev_info.last_sector_size = CNF_GET_LAST_SCTR_SIZE(reg);

	/* Page size and spare size */
	reg = mmio_read_32(CNF_CTRLPARAM(DEV_AREA));
	dev_info.page_size = CNF_GET_PAGE_SIZE(reg);
	dev_info.spare_size = CNF_GET_SPARE_SIZE(reg);
	INFO(" -- Page main area size: %d bytes\n", dev_info.page_size);
	INFO(" -- Page spare area size: %d bytes\n", dev_info.spare_size);

	/* Device blocks per LUN */
	dev_info.nblocks_per_lun = mmio_read_32(CNF_CTRLPARAM(DEV_BLOCKS_PLUN));
	INFO(" -- Blocks per LUN: %d\n", dev_info.nblocks_per_lun);

	/* Calculate block size and total device size */
	dev_info.block_size = (dev_info.npages_per_block * dev_info.page_size);
	INFO(" -- Block size: %d bytes\n", dev_info.block_size);

	dev_info.total_size = ((unsigned long long)dev_info.block_size *
				(unsigned long long)dev_info.nblocks_per_lun *
				dev_info.nluns);
	NOTICE(" -- Total device size: %llu bytes\n", dev_info.total_size);

	return 0;
}

/* NAND Flash Controller/Host initialization */
int cdns_nand_host_init(void)
{
	uint32_t reg = 0U;
	int ret = 0;
	uint32_t timeout_count = (CNF_DD_INIT_COMP_US / CNF_DEF_DELAY_US);

	INFO("CNF: Starting Device Discovery Process\n");
	do {
		/* Read controller status register for init complete */
		reg = mmio_read_32(CNF_CMDREG(CTRL_STATUS));

		/* Verify the device INIT state, break if complete. */
		if (CNF_GET_INIT_COMP(reg))
			break;

		udelay(CNF_DEF_DELAY_US);
	} while (--timeout_count != 0);

	if (timeout_count == 0) {
		ERROR("CNF: Device Discovery Process timed out\n");
		return -ETIMEDOUT;
	}

	INFO("CNF: Device Discovery Process is completed\n");
	ret = cdns_nand_update_dev_info();
	if (ret != 0) {
		return ret;
	}

	/* Status polling mode, device control and status register. */
	cdns_nand_wait_idle();
	mmio_clrbits_32(CNF_CTRLCFG(RDST_CTRL_0), BIT(0));

	/* Write protect. */
	cdns_nand_wait_idle();
	mmio_setbits_32(CNF_MINICTRL(WP_SETTINGS), BIT(0));

	/* Set operation work mode */
	cdns_nand_set_opr_mode();

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

static uint32_t cdns_nand_get_row_address(uint32_t page, uint32_t block)
{
	uint32_t row_address = 0U;
	uint32_t req_bits = 0U;

	/* The device info is not populated yet. */
	if (dev_info.npages_per_block == 0U)
		return 0;

	for (uint32_t i = 0U; i < sizeof(uint32_t) * 8; i++) {
		if ((1U << i) & dev_info.npages_per_block)
			req_bits = i;
	}

	row_address = ((page & GENMASK_32((req_bits - 1), 0)) |
			(block << req_bits));

	return row_address;
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
	mmio_write_32(CNF_CMDREG(CMD_REG2), (buffer & UINT32_MAX));
	mmio_write_32(CNF_CMDREG(CMD_REG3), ((buffer >> 32) & UINT32_MAX));

	/* Set row address */
	mmio_write_32(CNF_CMDREG(CMD_REG1),
			cdns_nand_get_row_address(page, block));

	/* Page read command */
	uint32_t reg = (CNF_WORK_MODE_PIO << CNF_CMDREG0_CT);

	reg |= (CNF_DEF_TRD << CNF_CMDREG0_TRD);
	reg |= (CNF_DEF_VOL_ID << CNF_CMDREG0_VOL);
	reg |= (CNF_INT_DIS << CNF_CMDREG0_INTR);
	reg |= (CNF_DMA_MASTER_SEL << CNF_CMDREG0_DMA);
	reg |= (CNF_CT_PAGE_READ << CNF_CMDREG0_CMD);
	reg |= (((CNF_READ_SINGLE_PAGE - 1) & 0xFF) << CNF_CMDREG0_CMD);
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

	INFO("CNF: %s: block %u-%u, page_start %u, len %zu, offset %u\n",
		__func__, block, end_block, page_start, length, offset);

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
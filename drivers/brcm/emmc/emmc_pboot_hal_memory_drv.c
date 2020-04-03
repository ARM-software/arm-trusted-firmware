/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <emmc_api.h>
#include <cmn_plat_util.h>

#define MAX_CMD_RETRY      10

#if EMMC_USE_DMA
#define USE_DMA 1
#else
#define USE_DMA 0
#endif

struct emmc_global_buffer emmc_global_buf;
struct emmc_global_buffer *emmc_global_buf_ptr = &emmc_global_buf;

struct emmc_global_vars emmc_global_vars;
struct emmc_global_vars *emmc_global_vars_ptr = &emmc_global_vars;

static struct sd_handle *sdio_gethandle(void);
static uint32_t sdio_idle(struct sd_handle *p_sdhandle);

static uint32_t sdio_read(struct sd_handle *p_sdhandle,
			  uintptr_t mem_addr,
			  uintptr_t storage_addr,
			  size_t storage_size,
			  size_t bytes_to_read);

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
static uint32_t sdio_write(struct sd_handle *p_sdhandle,
			   uintptr_t mem_addr,
			   uintptr_t data_addr,
			   size_t bytes_to_write);
#endif

static struct sd_handle *sdio_init(void);
static int32_t bcm_emmc_card_ready_state(struct sd_handle *p_sdhandle);

static void init_globals(void)
{
	memset((void *)emmc_global_buf_ptr, 0, sizeof(*emmc_global_buf_ptr));
	memset((void *)emmc_global_vars_ptr, 0, sizeof(*emmc_global_vars_ptr));
}

/*
 * This function is used to change partition
 */
uint32_t emmc_partition_select(uint32_t partition)
{
	int rc;
	struct sd_handle *sd_handle = sdio_gethandle();

	if (sd_handle->device == 0) {
		EMMC_TRACE("eMMC init is not done");
		return 0;
	}

	switch (partition) {
	case EMMC_BOOT_PARTITION1:
		rc = set_boot_config(sd_handle,
				     SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_BOOT1);
		EMMC_TRACE(
		     "Change to Boot Partition 1 result:%d  (0 means SD_OK)\n",
		     rc);
		break;

	case EMMC_BOOT_PARTITION2:
		rc = set_boot_config(sd_handle,
				     SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_BOOT2);
		EMMC_TRACE(
		     "Change to Boot Partition 2 result:%d  (0 means SD_OK)\n",
		     rc);
		break;

	case EMMC_USE_CURRENT_PARTITION:
		rc = SD_OK;
		EMMC_TRACE("Stay on current partition");
		break;

	case EMMC_USER_AREA:
	default:
		rc = set_boot_config(sd_handle,
				     SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_USER);
		EMMC_TRACE("Change to User area result:%d  (0 means SD_OK)\n",
			   rc);
		break;

	}
	return (rc == SD_OK);
}

/*
 * Initialize emmc controller for eMMC
 * Returns 0 on fail condition
 */
uint32_t bcm_emmc_init(bool card_rdy_only)
{
	struct sd_handle *p_sdhandle;
	uint32_t result = 0;

	EMMC_TRACE("Enter emmc_controller_init()\n");

	/* If eMMC is already initialized, skip init */
	if (emmc_global_vars_ptr->init_done)
		return 1;

	init_globals();

	p_sdhandle = sdio_init();

	if (p_sdhandle == NULL) {
		ERROR("eMMC init failed");
		return result;
	}

	if (card_rdy_only) {
		/* Put the card in Ready state, Not complete init */
		result = bcm_emmc_card_ready_state(p_sdhandle);
		return !result;
	}

	if (sdio_idle(p_sdhandle) == EMMC_BOOT_OK) {
		set_config(p_sdhandle, SD_NORMAL_SPEED, MAX_CMD_RETRY, USE_DMA,
			   SD_DMA_BOUNDARY_256K, EMMC_BLOCK_SIZE,
			   EMMC_WFE_RETRY);

		if (!select_blk_sz(p_sdhandle,
				   p_sdhandle->device->cfg.blockSize)) {
			emmc_global_vars_ptr->init_done = 1;
			result = 1;
		} else {
			ERROR("Select Block Size failed\n");
		}
	} else {
		ERROR("eMMC init failed");
	}

	/* Initialization is failed, so deinit HW setting */
	if (result == 0)
		emmc_deinit();

	return result;
}

/*
 * Function to de-init SDIO controller for eMMC
 */
void emmc_deinit(void)
{
	emmc_global_vars_ptr->init_done = 0;
	emmc_global_vars_ptr->sdHandle.card = 0;
	emmc_global_vars_ptr->sdHandle.device = 0;
}

/*
 * Read eMMC memory
 * Returns read_size
 */
uint32_t emmc_read(uintptr_t mem_addr, uintptr_t storage_addr,
		   size_t storage_size, size_t bytes_to_read)
{
	struct sd_handle *sd_handle = sdio_gethandle();

	if (sd_handle->device == 0) {
		EMMC_TRACE("eMMC init is not done");
		return 0;
	}

	return sdio_read(sdio_gethandle(), mem_addr, storage_addr,
			 storage_size, bytes_to_read);
}

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
#define EXT_CSD_ERASE_GRP_SIZE 224

static int emmc_block_erase(uintptr_t mem_addr, size_t blocks)
{
	struct sd_handle *sd_handle = sdio_gethandle();

	if (sd_handle->device == 0) {
		ERROR("eMMC init is not done");
		return -1;
	}

	return erase_card(sdio_gethandle(), mem_addr, blocks);
}

int emmc_erase(uintptr_t mem_addr, size_t num_of_blocks, uint32_t partition)
{
	int err = 0;
	size_t block_count = 0, blocks = 0;
	size_t erase_group = 0;

	erase_group =
	emmc_global_buf_ptr->u.Ext_CSD_storage[EXT_CSD_ERASE_GRP_SIZE]*1024;

	INFO("eMMC Erase Group Size=0x%lx\n", erase_group);

	emmc_partition_select(partition);

	while (block_count < num_of_blocks) {
		blocks = ((num_of_blocks - block_count) > erase_group) ?
				  erase_group : (num_of_blocks - block_count);
			err = emmc_block_erase(mem_addr + block_count, blocks);
		if (err)
			break;

		block_count += blocks;
	}

	if (err == 0)
		INFO("eMMC Erase of partition %d successful\n", partition);
	else
		ERROR("eMMC Erase of partition %d Failed(%i)\n", partition, err);

	return err;
}
#endif

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
/*
 * Write to eMMC memory
 * Returns written_size
 */
uint32_t emmc_write(uintptr_t mem_addr, uintptr_t data_addr,
		    size_t bytes_to_write)
{
	struct sd_handle *sd_handle = sdio_gethandle();

	if (sd_handle->device == 0) {
		EMMC_TRACE("eMMC init is not done");
		return 0;
	}

	return sdio_write(sd_handle, mem_addr, data_addr, bytes_to_write);
}
#endif

/*
 * Send SDIO Cmd
 * Return 0 for pass condition
 */
uint32_t send_sdio_cmd(uint32_t cmdIndex, uint32_t argument,
		       uint32_t options, struct sd_resp *resp)
{
	struct sd_handle *sd_handle = sdio_gethandle();

	if (sd_handle->device == 0) {
		EMMC_TRACE("eMMC init is not done");
		return 1;
	}

	return send_cmd(sd_handle, cmdIndex, argument, options, resp);
}


/*
 * This function return SDIO handle
 */
struct sd_handle *sdio_gethandle(void)
{
	return &emmc_global_vars_ptr->sdHandle;
}

/*
 * Initialize SDIO controller
 */
struct sd_handle *sdio_init(void)
{
	uint32_t SDIO_base;
	struct sd_handle *p_sdhandle = &emmc_global_vars_ptr->sdHandle;

	SDIO_base = EMMC_CTRL_REGS_BASE_ADDR;

	if (SDIO_base == SDIO0_EMMCSDXC_SYSADDR)
		EMMC_TRACE(" ---> for SDIO 0 Controller\n\n");

	memset(p_sdhandle, 0, sizeof(struct sd_handle));

	p_sdhandle->device = &emmc_global_vars_ptr->sdDevice;
	p_sdhandle->card = &emmc_global_vars_ptr->sdCard;

	memset(p_sdhandle->device, 0, sizeof(struct sd_dev));
	memset(p_sdhandle->card, 0, sizeof(struct sd_card_info));

	if (chal_sd_start((CHAL_HANDLE *) p_sdhandle->device,
			  SD_PIO_MODE, SDIO_base, SDIO_base) != SD_OK)
		return NULL;

	set_config(p_sdhandle, SD_NORMAL_SPEED, MAX_CMD_RETRY, SD_DMA_OFF,
		   SD_DMA_BOUNDARY_4K, EMMC_BLOCK_SIZE, EMMC_WFE_RETRY);

	return &emmc_global_vars_ptr->sdHandle;
}

uint32_t sdio_idle(struct sd_handle *p_sdhandle)
{
	reset_card(p_sdhandle);

	SD_US_DELAY(1000);

	if (init_card(p_sdhandle, SD_CARD_DETECT_MMC) != SD_OK) {
		reset_card(p_sdhandle);
		reset_host_ctrl(p_sdhandle);
		return EMMC_BOOT_NO_CARD;
	}

	return EMMC_BOOT_OK;
}

/*
 * This function read eMMC
 */
uint32_t sdio_read(struct sd_handle *p_sdhandle,
		   uintptr_t mem_addr,
		   uintptr_t storage_addr,
		   size_t storage_size, size_t bytes_to_read)
{
	uint32_t offset = 0, blockAddr, readLen = 0, rdCount;
	uint32_t remSize, manual_copy_size;
	uint8_t *outputBuf = (uint8_t *) storage_addr;
	const size_t blockSize = p_sdhandle->device->cfg.blockSize;

	VERBOSE("EMMC READ: dst=0x%lx, src=0x%lx, size=0x%lx\n",
			storage_addr, mem_addr, bytes_to_read);

	if (storage_size < bytes_to_read)
		/* Don't have sufficient storage to complete the operation */
		return 0;

	/* Range check non high capacity memory */
	if ((p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY) == 0) {
		if (mem_addr > 0x80000000)
			return 0;
	}

	/* High capacity card use block address mode */
	if (p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY) {
		blockAddr = (uint32_t) (mem_addr / blockSize);
		offset = (uint32_t) (mem_addr - (blockAddr * blockSize));
	} else {
		blockAddr = (uint32_t) (mem_addr / blockSize) * blockSize;
		offset = (uint32_t) (mem_addr - blockAddr);
	}

	remSize = bytes_to_read;

	rdCount = 0;

	/* Process first unaligned block of MAX_READ_LENGTH */
	if (offset > 0) {
		if (!read_block(p_sdhandle, emmc_global_buf_ptr->u.tempbuf,
				blockAddr, SD_MAX_READ_LENGTH)) {

			if (remSize < (blockSize - offset)) {
				rdCount += remSize;
				manual_copy_size = remSize;
				remSize = 0;	/* read is done */
			} else {
				remSize -= (blockSize - offset);
				rdCount += (blockSize - offset);
				manual_copy_size = blockSize - offset;
			}

			/* Check for overflow */
			if (manual_copy_size > storage_size ||
			    (((uintptr_t)outputBuf + manual_copy_size) >
			     (storage_addr + storage_size))) {
				ERROR("EMMC READ: Overflow 1\n");
				return 0;
			}

			memcpy(outputBuf,
			       (void *)((uintptr_t)
				(emmc_global_buf_ptr->u.tempbuf + offset)),
			       manual_copy_size);

			/* Update Physical address */
			outputBuf += manual_copy_size;

			if (p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY)
				blockAddr++;
			else
				blockAddr += blockSize;
		} else {
			return 0;
		}
	}

	while (remSize >= blockSize) {

		if (remSize >= SD_MAX_BLK_TRANSFER_LENGTH)
			readLen = SD_MAX_BLK_TRANSFER_LENGTH;
		else
			readLen = (remSize / blockSize) * blockSize;

		/* Check for overflow */
		if ((rdCount + readLen) > storage_size ||
		    (((uintptr_t) outputBuf + readLen) >
		     (storage_addr + storage_size))) {
			ERROR("EMMC READ: Overflow\n");
			return 0;
		}

		if (!read_block(p_sdhandle, outputBuf, blockAddr, readLen)) {
			if (p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY)
				blockAddr += (readLen / blockSize);
			else
				blockAddr += readLen;

			remSize -= readLen;
			rdCount += readLen;

			/* Update Physical address */
			outputBuf += readLen;
		} else {
			return 0;
		}
	}

	/* process the last unaligned block reading */
	if (remSize > 0) {
		if (!read_block(p_sdhandle, emmc_global_buf_ptr->u.tempbuf,
				blockAddr, SD_MAX_READ_LENGTH)) {

			rdCount += remSize;
			/* Check for overflow */
			if (rdCount > storage_size ||
			    (((uintptr_t) outputBuf + remSize) >
			     (storage_addr + storage_size))) {
				ERROR("EMMC READ: Overflow\n");
				return 0;
			}

			memcpy(outputBuf,
				emmc_global_buf_ptr->u.tempbuf, remSize);

			/* Update Physical address */
			outputBuf += remSize;
		} else {
			rdCount = 0;
		}
	}

	return rdCount;
}

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
static uint32_t sdio_write(struct sd_handle *p_sdhandle, uintptr_t mem_addr,
			   uintptr_t data_addr, size_t bytes_to_write)
{

	uint32_t offset, blockAddr, writeLen, wtCount = 0;
	uint32_t remSize, manual_copy_size = 0;

	uint8_t *inputBuf = (uint8_t *)data_addr;

	/* range check non high capacity memory */
	if ((p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY) == 0) {
		if (mem_addr > 0x80000000)
			return 0;
	}

	/* the high capacity card use block address mode */
	if (p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY) {
		blockAddr =
		    (uint32_t)(mem_addr / p_sdhandle->device->cfg.blockSize);
		offset =
		    (uint32_t)(mem_addr -
			       blockAddr * p_sdhandle->device->cfg.blockSize);
	} else {
		blockAddr =
		    ((uint32_t)mem_addr / p_sdhandle->device->cfg.blockSize) *
		    p_sdhandle->device->cfg.blockSize;
		offset = (uint32_t) mem_addr - blockAddr;
	}

	remSize = bytes_to_write;

	wtCount = 0;

	/* process first unaligned block */
	if (offset > 0) {
		if (!read_block(p_sdhandle, emmc_global_buf_ptr->u.tempbuf,
				blockAddr, p_sdhandle->device->cfg.blockSize)) {

			if (remSize <
			    (p_sdhandle->device->cfg.blockSize - offset))
				manual_copy_size = remSize;
			else
				manual_copy_size =
				    p_sdhandle->device->cfg.blockSize - offset;

			memcpy((void *)((uintptr_t)
				(emmc_global_buf_ptr->u.tempbuf + offset)),
			       inputBuf,
			       manual_copy_size);

			/* Update Physical address */

			if (!write_block(p_sdhandle,
					 emmc_global_buf_ptr->u.tempbuf,
					 blockAddr,
					 p_sdhandle->device->cfg.blockSize)) {

				if (remSize <
				    (p_sdhandle->device->cfg.blockSize -
				     offset)) {
					wtCount += remSize;
					manual_copy_size = remSize;
					remSize = 0;	/* read is done */
				} else {
					remSize -=
					    (p_sdhandle->device->cfg.blockSize -
					     offset);
					wtCount +=
					    (p_sdhandle->device->cfg.blockSize -
					     offset);
					manual_copy_size =
					    p_sdhandle->device->cfg.blockSize -
					    offset;
				}

				inputBuf += manual_copy_size;

				if (p_sdhandle->device->ctrl.ocr &
				    SD_CARD_HIGH_CAPACITY)
					blockAddr++;
				else
					blockAddr +=
					    p_sdhandle->device->cfg.blockSize;
			} else
				return 0;
		} else {
			return 0;
		}
	}

	/* process block writing */
	while (remSize >= p_sdhandle->device->cfg.blockSize) {
		if (remSize >= SD_MAX_READ_LENGTH) {
			writeLen = SD_MAX_READ_LENGTH;
		} else {
			writeLen =
			    (remSize / p_sdhandle->device->cfg.blockSize) *
			     p_sdhandle->device->cfg.blockSize;
		}

		if (!write_block(p_sdhandle, inputBuf, blockAddr, writeLen)) {
			if (p_sdhandle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY)
				blockAddr +=
				    (writeLen /
				     p_sdhandle->device->cfg.blockSize);
			else
				blockAddr += writeLen;

			remSize -= writeLen;
			wtCount += writeLen;
			inputBuf += writeLen;
		} else {
			return 0;
		}
	}

	/* process the last unaligned block reading */
	if (remSize > 0) {
		if (!read_block(p_sdhandle,
				emmc_global_buf_ptr->u.tempbuf,
				blockAddr, p_sdhandle->device->cfg.blockSize)) {

			memcpy(emmc_global_buf_ptr->u.tempbuf,
			       inputBuf, remSize);

			/* Update Physical address */

			if (!write_block(p_sdhandle,
					 emmc_global_buf_ptr->u.tempbuf,
					 blockAddr,
					 p_sdhandle->device->cfg.blockSize)) {
				wtCount += remSize;
				inputBuf += remSize;
			} else {
				return 0;
			}
		} else {
			wtCount = 0;
		}
	}

	return wtCount;
}
#endif

/*
 * Function to put the card in Ready state by sending CMD0 and CMD1
 */
static int32_t bcm_emmc_card_ready_state(struct sd_handle *p_sdhandle)
{
	int32_t result = 0;
	uint32_t argument = MMC_CMD_IDLE_RESET_ARG; /* Exit from Boot mode */

	if (p_sdhandle) {
		send_sdio_cmd(SD_CMD_GO_IDLE_STATE, argument, 0, NULL);

		result = reset_card(p_sdhandle);
		if (result != SD_OK) {
			EMMC_TRACE("eMMC Reset error\n");
			return SD_RESET_ERROR;
		}
		SD_US_DELAY(2000);
		result = mmc_cmd1(p_sdhandle);
	}

	return result;
}

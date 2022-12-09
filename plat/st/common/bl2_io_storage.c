/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/fwu/fwu.h>
#include <drivers/fwu/fwu_metadata.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_encrypted.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_mtd.h>
#include <drivers/io/io_storage.h>
#include <drivers/mmc.h>
#include <drivers/partition/efi.h>
#include <drivers/partition/partition.h>
#include <drivers/raw_nand.h>
#include <drivers/spi_nand.h>
#include <drivers/spi_nor.h>
#include <drivers/st/io_mmc.h>
#include <drivers/st/stm32_fmc2_nand.h>
#include <drivers/st/stm32_qspi.h>
#include <drivers/st/stm32_sdmmc2.h>
#include <drivers/usb_device.h>
#include <lib/fconf/fconf.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <stm32cubeprogrammer.h>
#include <stm32mp_efi.h>
#include <stm32mp_fconf_getter.h>
#include <stm32mp_io_storage.h>
#include <usb_dfu.h>

/* IO devices */
uintptr_t fip_dev_handle;
uintptr_t storage_dev_handle;

static const io_dev_connector_t *fip_dev_con;

#ifndef DECRYPTION_SUPPORT_none
static const io_dev_connector_t *enc_dev_con;
uintptr_t enc_dev_handle;
#endif

#if STM32MP_SDMMC || STM32MP_EMMC
static struct mmc_device_info mmc_info;

static uint32_t block_buffer[MMC_BLOCK_SIZE] __aligned(MMC_BLOCK_SIZE);

static io_block_dev_spec_t mmc_block_dev_spec = {
	/* It's used as temp buffer in block driver */
	.buffer = {
		.offset = (size_t)&block_buffer,
		.length = MMC_BLOCK_SIZE,
	},
	.ops = {
		.read = mmc_read_blocks,
		.write = NULL,
	},
	.block_size = MMC_BLOCK_SIZE,
};

static const io_dev_connector_t *mmc_dev_con;
#endif /* STM32MP_SDMMC || STM32MP_EMMC */

#if STM32MP_SPI_NOR
static io_mtd_dev_spec_t spi_nor_dev_spec = {
	.ops = {
		.init = spi_nor_init,
		.read = spi_nor_read,
	},
};
#endif

#if STM32MP_RAW_NAND
static io_mtd_dev_spec_t nand_dev_spec = {
	.ops = {
		.init = nand_raw_init,
		.read = nand_read,
		.seek = nand_seek_bb
	},
};

static const io_dev_connector_t *nand_dev_con;
#endif

#if STM32MP_SPI_NAND
static io_mtd_dev_spec_t spi_nand_dev_spec = {
	.ops = {
		.init = spi_nand_init,
		.read = nand_read,
		.seek = nand_seek_bb
	},
};
#endif

#if STM32MP_SPI_NAND || STM32MP_SPI_NOR
static const io_dev_connector_t *spi_dev_con;
#endif

#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
static const io_dev_connector_t *memmap_dev_con;
#endif

io_block_spec_t image_block_spec = {
	.offset = 0U,
	.length = 0U,
};

int open_fip(const uintptr_t spec)
{
	return io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
}

#ifndef DECRYPTION_SUPPORT_none
int open_enc_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(enc_dev_handle, (uintptr_t)ENC_IMAGE_ID);
	if (result != 0) {
		return result;
	}

	result = io_open(enc_dev_handle, spec, &local_image_handle);
	if (result != 0) {
		return result;
	}

	VERBOSE("Using encrypted FIP\n");
	io_close(local_image_handle);

	return 0;
}
#endif

int open_storage(const uintptr_t spec)
{
	return io_dev_init(storage_dev_handle, 0);
}

#if STM32MP_EMMC_BOOT
static uint32_t get_boot_part_fip_header(void)
{
	io_block_spec_t emmc_boot_fip_block_spec = {
		.offset = STM32MP_EMMC_BOOT_FIP_OFFSET,
		.length = MMC_BLOCK_SIZE, /* We are interested only in first 4 bytes */
	};
	uint32_t magic = 0U;
	int io_result;
	size_t bytes_read;
	uintptr_t fip_hdr_handle;

	io_result = io_open(storage_dev_handle, (uintptr_t)&emmc_boot_fip_block_spec,
			    &fip_hdr_handle);
	assert(io_result == 0);

	io_result = io_read(fip_hdr_handle, (uintptr_t)&magic, sizeof(magic),
			    &bytes_read);
	if ((io_result != 0) || (bytes_read != sizeof(magic))) {
		panic();
	}

	io_close(fip_hdr_handle);

	VERBOSE("%s: eMMC boot magic at offset 256K: %08x\n",
		__func__, magic);

	return magic;
}
#endif

static void print_boot_device(boot_api_context_t *boot_context)
{
	switch (boot_context->boot_interface_selected) {
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD:
		INFO("Using SDMMC\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC:
		INFO("Using EMMC\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NOR_QSPI:
		INFO("Using QSPI NOR\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_FMC:
		INFO("Using FMC NAND\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_QSPI:
		INFO("Using SPI NAND\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_UART:
		INFO("Using UART\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB:
		INFO("Using USB\n");
		break;
	default:
		ERROR("Boot interface %u not found\n",
		      boot_context->boot_interface_selected);
		panic();
		break;
	}

	if (boot_context->boot_interface_instance != 0U) {
		INFO("  Instance %d\n", boot_context->boot_interface_instance);
	}
}

#if STM32MP_SDMMC || STM32MP_EMMC
static void boot_mmc(enum mmc_device_type mmc_dev_type,
		     uint16_t boot_interface_instance)
{
	int io_result __unused;
	struct stm32_sdmmc2_params params;

	zeromem(&params, sizeof(struct stm32_sdmmc2_params));

	mmc_info.mmc_dev_type = mmc_dev_type;

	switch (boot_interface_instance) {
	case 1:
		params.reg_base = STM32MP_SDMMC1_BASE;
		break;
	case 2:
		params.reg_base = STM32MP_SDMMC2_BASE;
		break;
	case 3:
		params.reg_base = STM32MP_SDMMC3_BASE;
		break;
	default:
		WARN("SDMMC instance not found, using default\n");
		if (mmc_dev_type == MMC_IS_SD) {
			params.reg_base = STM32MP_SDMMC1_BASE;
		} else {
			params.reg_base = STM32MP_SDMMC2_BASE;
		}
		break;
	}

	if (mmc_dev_type != MMC_IS_EMMC) {
		params.flags = MMC_FLAG_SD_CMD6;
	}

	params.device_info = &mmc_info;
	if (stm32_sdmmc2_mmc_init(&params) != 0) {
		ERROR("SDMMC%u init failed\n", boot_interface_instance);
		panic();
	}

	/* Open MMC as a block device to read FIP */
	io_result = register_io_dev_block(&mmc_dev_con);
	if (io_result != 0) {
		panic();
	}

	io_result = io_dev_open(mmc_dev_con, (uintptr_t)&mmc_block_dev_spec,
				&storage_dev_handle);
	assert(io_result == 0);

#if STM32MP_EMMC_BOOT
	if (mmc_dev_type == MMC_IS_EMMC) {
		io_result = mmc_part_switch_current_boot();
		assert(io_result == 0);

		if (get_boot_part_fip_header() != TOC_HEADER_NAME) {
			WARN("%s: Can't find FIP header on eMMC boot partition. Trying GPT\n",
			     __func__);
			io_result = mmc_part_switch_user();
			assert(io_result == 0);
			return;
		}

		VERBOSE("%s: FIP header found on eMMC boot partition\n",
			__func__);
		image_block_spec.offset = STM32MP_EMMC_BOOT_FIP_OFFSET;
		image_block_spec.length = mmc_boot_part_size() - STM32MP_EMMC_BOOT_FIP_OFFSET;
	}
#endif
}
#endif /* STM32MP_SDMMC || STM32MP_EMMC */

#if STM32MP_SPI_NOR
static void boot_spi_nor(boot_api_context_t *boot_context)
{
	int io_result __unused;

	io_result = stm32_qspi_init();
	assert(io_result == 0);

	io_result = register_io_dev_mtd(&spi_dev_con);
	assert(io_result == 0);

	/* Open connections to device */
	io_result = io_dev_open(spi_dev_con,
				(uintptr_t)&spi_nor_dev_spec,
				&storage_dev_handle);
	assert(io_result == 0);
}
#endif /* STM32MP_SPI_NOR */

#if STM32MP_RAW_NAND
static void boot_fmc2_nand(boot_api_context_t *boot_context)
{
	int io_result __unused;

	io_result = stm32_fmc2_init();
	assert(io_result == 0);

	/* Register the IO device on this platform */
	io_result = register_io_dev_mtd(&nand_dev_con);
	assert(io_result == 0);

	/* Open connections to device */
	io_result = io_dev_open(nand_dev_con, (uintptr_t)&nand_dev_spec,
				&storage_dev_handle);
	assert(io_result == 0);
}
#endif /* STM32MP_RAW_NAND */

#if STM32MP_SPI_NAND
static void boot_spi_nand(boot_api_context_t *boot_context)
{
	int io_result __unused;

	io_result = stm32_qspi_init();
	assert(io_result == 0);

	io_result = register_io_dev_mtd(&spi_dev_con);
	assert(io_result == 0);

	/* Open connections to device */
	io_result = io_dev_open(spi_dev_con,
				(uintptr_t)&spi_nand_dev_spec,
				&storage_dev_handle);
	assert(io_result == 0);
}
#endif /* STM32MP_SPI_NAND */

#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
static void mmap_io_setup(void)
{
	int io_result __unused;

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&storage_dev_handle);
	assert(io_result == 0);
}

#if STM32MP_UART_PROGRAMMER
static void stm32cubeprogrammer_uart(void)
{
	int ret __unused;
	boot_api_context_t *boot_context =
		(boot_api_context_t *)stm32mp_get_boot_ctx_address();
	uintptr_t uart_base;

	uart_base = get_uart_address(boot_context->boot_interface_instance);
	ret = stm32cubeprog_uart_load(uart_base, DWL_BUFFER_BASE, DWL_BUFFER_SIZE);
	assert(ret == 0);
}
#endif

#if STM32MP_USB_PROGRAMMER
static void stm32cubeprogrammer_usb(void)
{
	int ret __unused;
	struct usb_handle *pdev;

	/* Init USB on platform */
	pdev = usb_dfu_plat_init();

	ret = stm32cubeprog_usb_load(pdev, DWL_BUFFER_BASE, DWL_BUFFER_SIZE);
	assert(ret == 0);
}
#endif
#endif /* STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER */


void stm32mp_io_setup(void)
{
	int io_result __unused;
	boot_api_context_t *boot_context =
		(boot_api_context_t *)stm32mp_get_boot_ctx_address();

	print_boot_device(boot_context);

	if ((boot_context->boot_partition_used_toboot == 1U) ||
	    (boot_context->boot_partition_used_toboot == 2U)) {
		INFO("Boot used partition fsbl%u\n",
		     boot_context->boot_partition_used_toboot);
	}

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);

#ifndef DECRYPTION_SUPPORT_none
	io_result = register_io_dev_enc(&enc_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(enc_dev_con, (uintptr_t)NULL,
				&enc_dev_handle);
	assert(io_result == 0);
#endif

	switch (boot_context->boot_interface_selected) {
#if STM32MP_SDMMC
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD:
		dmbsy();
		boot_mmc(MMC_IS_SD, boot_context->boot_interface_instance);
		break;
#endif
#if STM32MP_EMMC
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC:
		dmbsy();
		boot_mmc(MMC_IS_EMMC, boot_context->boot_interface_instance);
		break;
#endif
#if STM32MP_SPI_NOR
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NOR_QSPI:
		dmbsy();
		boot_spi_nor(boot_context);
		break;
#endif
#if STM32MP_RAW_NAND
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_FMC:
		dmbsy();
		boot_fmc2_nand(boot_context);
		break;
#endif
#if STM32MP_SPI_NAND
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_QSPI:
		dmbsy();
		boot_spi_nand(boot_context);
		break;
#endif
#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
#if STM32MP_UART_PROGRAMMER
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_UART:
#endif
#if STM32MP_USB_PROGRAMMER
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB:
#endif
		dmbsy();
		mmap_io_setup();
		break;
#endif

	default:
		ERROR("Boot interface %d not supported\n",
		      boot_context->boot_interface_selected);
		panic();
		break;
	}
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	static bool gpt_init_done __unused;
	uint16_t boot_itf = stm32mp_get_boot_itf_selected();

	switch (boot_itf) {
#if STM32MP_SDMMC || STM32MP_EMMC
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC:
#if STM32MP_EMMC_BOOT
		if (image_block_spec.offset == STM32MP_EMMC_BOOT_FIP_OFFSET) {
			break;
		}
#endif
		/* fallthrough */
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD:
		if (!gpt_init_done) {
/*
 * With FWU Multi Bank feature enabled, the selection of
 * the image to boot will be done by fwu_init calling the
 * platform hook, plat_fwu_set_images_source.
 */
#if !PSA_FWU_SUPPORT
			const partition_entry_t *entry;
			const struct efi_guid img_type_guid = STM32MP_FIP_GUID;
			uuid_t img_type_uuid;

			guidcpy(&img_type_uuid, &img_type_guid);
			partition_init(GPT_IMAGE_ID);
			entry = get_partition_entry_by_type(&img_type_uuid);
			if (entry == NULL) {
				entry = get_partition_entry(FIP_IMAGE_NAME);
				if (entry == NULL) {
					ERROR("Could NOT find the %s partition!\n",
					      FIP_IMAGE_NAME);

					return -ENOENT;
				}
			}

			image_block_spec.offset = entry->start;
			image_block_spec.length = entry->length;
#endif
			gpt_init_done = true;
		} else {
			bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
			assert(bl_mem_params != NULL);

			mmc_block_dev_spec.buffer.offset = bl_mem_params->image_info.image_base;
			mmc_block_dev_spec.buffer.length = bl_mem_params->image_info.image_max_size;
		}

		break;
#endif

#if STM32MP_RAW_NAND || STM32MP_SPI_NAND
#if STM32MP_RAW_NAND
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_FMC:
#endif
#if STM32MP_SPI_NAND
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_QSPI:
#endif
		image_block_spec.offset = STM32MP_NAND_FIP_OFFSET;
		break;
#endif

#if STM32MP_SPI_NOR
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NOR_QSPI:
		image_block_spec.offset = STM32MP_NOR_FIP_OFFSET;
		break;
#endif

#if STM32MP_UART_PROGRAMMER
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_UART:
		if (image_id == FW_CONFIG_ID) {
			stm32cubeprogrammer_uart();
			/* FIP loaded at DWL address */
			image_block_spec.offset = DWL_BUFFER_BASE;
			image_block_spec.length = DWL_BUFFER_SIZE;
		}
		break;
#endif
#if STM32MP_USB_PROGRAMMER
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB:
		if (image_id == FW_CONFIG_ID) {
			stm32cubeprogrammer_usb();
			/* FIP loaded at DWL address */
			image_block_spec.offset = DWL_BUFFER_BASE;
			image_block_spec.length = DWL_BUFFER_SIZE;
		}
		break;
#endif

	default:
		ERROR("FIP Not found\n");
		panic();
	}

	return 0;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int rc;
	const struct plat_io_policy *policy;

	policy = FCONF_GET_PROPERTY(stm32mp, io_policies, image_id);
	rc = policy->check(policy->image_spec);
	if (rc == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return rc;
}

#if (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT
/*
 * In each boot in non-trial mode, we set the BKP register to
 * FWU_MAX_TRIAL_REBOOT, and return the active_index from metadata.
 *
 * As long as the update agent didn't update the "accepted" field in metadata
 * (i.e. we are in trial mode), we select the new active_index.
 * To avoid infinite boot loop at trial boot we decrement a BKP register.
 * If this counter is 0:
 *     - an unexpected TAMPER event raised (that resets the BKP registers to 0)
 *     - a power-off occurs before the update agent was able to update the
 *       "accepted' field
 *     - we already boot FWU_MAX_TRIAL_REBOOT times in trial mode.
 * we select the previous_active_index.
 */
#define INVALID_BOOT_IDX		0xFFFFFFFFU

uint32_t plat_fwu_get_boot_idx(void)
{
	/*
	 * Select boot index and update boot counter only once per boot
	 * even if this function is called several times.
	 */
	static uint32_t boot_idx = INVALID_BOOT_IDX;
	const struct fwu_metadata *data;

	data = fwu_get_metadata();

	if (boot_idx == INVALID_BOOT_IDX) {
		boot_idx = data->active_index;
		if (fwu_is_trial_run_state()) {
			if (stm32_get_and_dec_fwu_trial_boot_cnt() == 0U) {
				WARN("Trial FWU fails %u times\n",
				     FWU_MAX_TRIAL_REBOOT);
				boot_idx = data->previous_active_index;
			}
		} else {
			stm32_set_max_fwu_trial_boot_cnt();
		}
	}

	return boot_idx;
}

static void *stm32_get_image_spec(const uuid_t *img_type_uuid)
{
	unsigned int i;

	for (i = 0U; i < MAX_NUMBER_IDS; i++) {
		if ((guidcmp(&policies[i].img_type_guid, img_type_uuid)) == 0) {
			return (void *)policies[i].image_spec;
		}
	}

	return NULL;
}

void plat_fwu_set_images_source(const struct fwu_metadata *metadata)
{
	unsigned int i;
	uint32_t boot_idx;
	const partition_entry_t *entry;
	const uuid_t *img_type_uuid, *img_uuid;
	io_block_spec_t *image_spec;

	boot_idx = plat_fwu_get_boot_idx();
	assert(boot_idx < NR_OF_FW_BANKS);

	for (i = 0U; i < NR_OF_IMAGES_IN_FW_BANK; i++) {
		img_type_uuid = &metadata->img_entry[i].img_type_uuid;
		image_spec = stm32_get_image_spec(img_type_uuid);
		if (image_spec == NULL) {
			ERROR("Unable to get image spec for the image in the metadata\n");
			panic();
		}

		img_uuid =
			&metadata->img_entry[i].img_props[boot_idx].img_uuid;

		entry = get_partition_entry_by_uuid(img_uuid);
		if (entry == NULL) {
			ERROR("Unable to find the partition with the uuid mentioned in metadata\n");
			panic();
		}

		image_spec->offset = entry->start;
		image_spec->length = entry->length;
	}
}

static int plat_set_image_source(unsigned int image_id,
				 uintptr_t *handle,
				 uintptr_t *image_spec,
				 const char *part_name)
{
	struct plat_io_policy *policy;
	io_block_spec_t *spec;
	const partition_entry_t *entry = get_partition_entry(part_name);

	if (entry == NULL) {
		ERROR("Unable to find the %s partition\n", part_name);
		return -ENOENT;
	}

	policy = &policies[image_id];

	spec = (io_block_spec_t *)policy->image_spec;
	spec->offset = entry->start;
	spec->length = entry->length;

	*image_spec = policy->image_spec;
	*handle = *policy->dev_handle;

	return 0;
}

int plat_fwu_set_metadata_image_source(unsigned int image_id,
				       uintptr_t *handle,
				       uintptr_t *image_spec)
{
	char *part_name;

	assert((image_id == FWU_METADATA_IMAGE_ID) ||
	       (image_id == BKUP_FWU_METADATA_IMAGE_ID));

	partition_init(GPT_IMAGE_ID);

	if (image_id == FWU_METADATA_IMAGE_ID) {
		part_name = METADATA_PART_1;
	} else {
		part_name = METADATA_PART_2;
	}

	return plat_set_image_source(image_id, handle, image_spec,
				     part_name);
}
#endif /* (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT */

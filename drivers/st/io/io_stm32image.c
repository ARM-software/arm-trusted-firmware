/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <drivers/st/io_stm32image.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

static uintptr_t backend_dev_handle;
static uintptr_t backend_image_spec;
static uint32_t *stm32_img;
static uint8_t first_lba_buffer[MAX_LBA_SIZE] __aligned(4);
static struct stm32image_part_info *current_part;

/* STM32 Image driver functions */
static int stm32image_dev_open(const uintptr_t init_params,
			       io_dev_info_t **dev_info);
static int stm32image_partition_open(io_dev_info_t *dev_info,
				     const uintptr_t spec, io_entity_t *entity);
static int stm32image_partition_size(io_entity_t *entity, size_t *length);
static int stm32image_partition_read(io_entity_t *entity, uintptr_t buffer,
				     size_t length, size_t *length_read);
static int stm32image_partition_close(io_entity_t *entity);
static int stm32image_dev_init(io_dev_info_t *dev_info,
			       const uintptr_t init_params);
static int stm32image_dev_close(io_dev_info_t *dev_info);

/* Identify the device type as a virtual driver */
static io_type_t device_type_stm32image(void)
{
	return IO_TYPE_STM32IMAGE;
}

static const io_dev_connector_t stm32image_dev_connector = {
	.dev_open = stm32image_dev_open
};

static const io_dev_funcs_t stm32image_dev_funcs = {
	.type = device_type_stm32image,
	.open = stm32image_partition_open,
	.size = stm32image_partition_size,
	.read = stm32image_partition_read,
	.close = stm32image_partition_close,
	.dev_init = stm32image_dev_init,
	.dev_close = stm32image_dev_close,
};

static io_dev_info_t stm32image_dev_info = {
	.funcs = &stm32image_dev_funcs,
	.info = (uintptr_t)0,
};

static struct stm32image_device_info stm32image_dev;

static int get_part_idx_by_binary_type(uint32_t binary_type)
{
	int i;

	for (i = 0; i < STM32_PART_NUM; i++) {
		if (stm32image_dev.part_info[i].binary_type == binary_type) {
			return i;
		}
	}

	return -EINVAL;
}

/* Open a connection to the STM32IMAGE device */
static int stm32image_dev_open(const uintptr_t init_params,
			       io_dev_info_t **dev_info)
{
	int i;
	struct stm32image_device_info *device_info =
		(struct stm32image_device_info *)init_params;

	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&stm32image_dev_info;

	stm32image_dev.device_size = device_info->device_size;
	stm32image_dev.lba_size = device_info->lba_size;

	for (i = 0; i < STM32_PART_NUM; i++) {
		memcpy(stm32image_dev.part_info[i].name,
		       device_info->part_info[i].name, MAX_PART_NAME_SIZE);
		stm32image_dev.part_info[i].binary_type =
			device_info->part_info[i].binary_type;
		stm32image_dev.part_info[i].part_offset =
			device_info->part_info[i].part_offset;
		stm32image_dev.part_info[i].bkp_offset =
			device_info->part_info[i].bkp_offset;
	}

	return 0;
}

/* Do some basic package checks */
static int stm32image_dev_init(io_dev_info_t *dev_info,
			       const uintptr_t init_params)
{
	int result;

	if ((backend_dev_handle != 0U) || (backend_image_spec != 0U)) {
		ERROR("STM32 Image io supports only one session\n");
		return -ENOMEM;
	}

	/* Obtain a reference to the image by querying the platform layer */
	result = plat_get_image_source(STM32_IMAGE_ID, &backend_dev_handle,
				       &backend_image_spec);
	if (result != 0) {
		ERROR("STM32 image error (%i)\n", result);
		return -EINVAL;
	}

	return result;
}

/* Close a connection to the STM32 Image device */
static int stm32image_dev_close(io_dev_info_t *dev_info)
{
	backend_dev_handle = 0U;
	backend_image_spec = 0U;
	stm32_img = NULL;

	return 0;
}

/* Open a partition */
static int stm32image_partition_open(io_dev_info_t *dev_info,
				     const uintptr_t spec, io_entity_t *entity)
{
	const struct stm32image_part_info *partition_spec;
	int idx;

	assert(entity != NULL);

	partition_spec = (struct stm32image_part_info *)spec;
	assert(partition_spec != NULL);

	idx = get_part_idx_by_binary_type(partition_spec->binary_type);
	if ((idx < 0) || (idx > STM32_PART_NUM)) {
		ERROR("Wrong partition index (%d)\n", idx);
		return -EINVAL;
	}

	current_part = &stm32image_dev.part_info[idx];
	stm32_img = (uint32_t *)&current_part->part_offset;

	return 0;
}

/* Return the size of a partition */
static int stm32image_partition_size(io_entity_t *entity, size_t *length)
{
	int result;
	uintptr_t backend_handle;
	size_t bytes_read;
	boot_api_image_header_t *header =
		(boot_api_image_header_t *)first_lba_buffer;

	assert(entity != NULL);
	assert(length != NULL);

	/* Attempt to access the image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);

	if (result < 0) {
		ERROR("%s: io_open (%i)\n", __func__, result);
		return result;
	}

	/* Reset magic header value */
	header->magic = 0;

	while (header->magic == 0U) {
		result = io_seek(backend_handle, IO_SEEK_SET, *stm32_img);
		if (result != 0) {
			ERROR("%s: io_seek (%i)\n", __func__, result);
			break;
		}

		result = io_read(backend_handle, (uintptr_t)header,
				 MAX_LBA_SIZE, (size_t *)&bytes_read);
		if (result != 0) {
			if (current_part->bkp_offset == 0U) {
				ERROR("%s: io_read (%i)\n", __func__, result);
			}
			header->magic = 0;
		}

		if ((header->magic != BOOT_API_IMAGE_HEADER_MAGIC_NB) ||
		    (header->binary_type != current_part->binary_type) ||
		    (header->image_length >= stm32image_dev.device_size)) {
			VERBOSE("%s: partition %s not found at %x\n",
				__func__, current_part->name, *stm32_img);

			if (current_part->bkp_offset == 0U) {
				result = -ENOMEM;
				break;
			}

			/* Header not correct, check next offset for backup */
			*stm32_img += current_part->bkp_offset;
			if (*stm32_img > stm32image_dev.device_size) {
				/* No backup found, end of device reached */
				WARN("%s : partition %s not found\n",
				     __func__, current_part->name);
				result = -ENOMEM;
				break;
			}
			header->magic = 0;
		}
	}

	io_close(backend_handle);

	if (result != 0) {
		return result;
	}

	if (header->image_length < stm32image_dev.lba_size) {
		*length = stm32image_dev.lba_size;
	} else {
		*length = header->image_length;
	}

	INFO("STM32 Image size : %lu\n", (unsigned long)*length);

	return 0;
}

/* Read data from a partition */
static int stm32image_partition_read(io_entity_t *entity, uintptr_t buffer,
				     size_t length, size_t *length_read)
{
	int result = -EINVAL;
	uint8_t *local_buffer;
	boot_api_image_header_t *header =
		(boot_api_image_header_t *)first_lba_buffer;
	size_t hdr_sz = sizeof(boot_api_image_header_t);

	assert(entity != NULL);
	assert(buffer != 0U);
	assert(length_read != NULL);

	local_buffer = (uint8_t *)buffer;
	*length_read = 0U;

	while (*length_read == 0U) {
		int offset;
		int local_length;
		uintptr_t backend_handle;

		if (header->magic != BOOT_API_IMAGE_HEADER_MAGIC_NB) {
			/* Check for backup as image is corrupted */
			if (current_part->bkp_offset == 0U) {
				result = -ENOMEM;
				break;
			}

			*stm32_img += current_part->bkp_offset;
			if (*stm32_img >= stm32image_dev.device_size) {
				/* End of device reached */
				result = -ENOMEM;
				break;
			}

			local_buffer = (uint8_t *)buffer;

			result = stm32image_partition_size(entity, &length);
			if (result != 0) {
				break;
			}
		}

		/* Part of image already loaded with the header */
		memcpy(local_buffer, (uint8_t *)first_lba_buffer + hdr_sz,
		       MAX_LBA_SIZE - hdr_sz);
		local_buffer += MAX_LBA_SIZE - hdr_sz;
		offset = MAX_LBA_SIZE;

		/* New image length to be read */
		local_length = round_up(length - ((MAX_LBA_SIZE) - hdr_sz),
					stm32image_dev.lba_size);

		if ((header->load_address != 0U) &&
		    (header->load_address != buffer)) {
			ERROR("Wrong load address\n");
			panic();
		}

		result = io_open(backend_dev_handle, backend_image_spec,
				 &backend_handle);

		if (result != 0) {
			ERROR("%s: io_open (%i)\n", __func__, result);
			break;
		}

		result = io_seek(backend_handle, IO_SEEK_SET,
				 *stm32_img + offset);

		if (result != 0) {
			ERROR("%s: io_seek (%i)\n", __func__, result);
			*length_read = 0;
			io_close(backend_handle);
			break;
		}

		result = io_read(backend_handle, (uintptr_t)local_buffer,
				 local_length, length_read);

		/* Adding part of size already read from header */
		*length_read += MAX_LBA_SIZE - hdr_sz;

		if (result != 0) {
			ERROR("%s: io_read (%i)\n", __func__, result);
			*length_read = 0;
			header->magic = 0;
			continue;
		}

		result = stm32mp_check_header(header, buffer);
		if (result != 0) {
			ERROR("Header check failed\n");
			*length_read = 0;
			header->magic = 0;
		}

		result = stm32mp_auth_image(header, buffer);
		if (result != 0) {
			ERROR("Authentication Failed (%i)\n", result);
			return result;
		}

		inv_dcache_range(round_up((uintptr_t)(local_buffer + length - hdr_sz),
					  CACHE_WRITEBACK_GRANULE), *length_read - length + hdr_sz);

		io_close(backend_handle);
	}

	return result;
}

/* Close a partition */
static int stm32image_partition_close(io_entity_t *entity)
{
	current_part = NULL;

	return 0;
}

/* Register the stm32image driver with the IO abstraction */
int register_io_dev_stm32image(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&stm32image_dev_info);
	if (result == 0) {
		*dev_con = &stm32image_dev_connector;
	}

	return result;
}

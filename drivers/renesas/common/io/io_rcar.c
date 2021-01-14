/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>
#include <tools_share/uuid.h>

#include "io_rcar.h"
#include "io_common.h"
#include "io_private.h"
#include <platform_def.h>

extern int32_t plat_get_drv_source(uint32_t id, uintptr_t *dev,
				   uintptr_t *image_spec);

static int32_t rcar_dev_open(const uintptr_t dev_spec __attribute__ ((unused)),
			     io_dev_info_t **dev_info);
static int32_t rcar_dev_close(io_dev_info_t *dev_info);

typedef struct {
	const int32_t name;
	const uint32_t offset;
	const uint32_t attr;
} plat_rcar_name_offset_t;

typedef struct {
	/*
	 * Put position above the struct to allow {0} on static init.
	 * It is a workaround for a known bug in GCC
	 * http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
	 */
	uint32_t position;
	uint32_t no_load;
	uintptr_t offset;
	uint32_t size;
	uintptr_t dst;
	uintptr_t partition;	/* for eMMC */
	/* RCAR_EMMC_PARTITION_BOOT_0 */
	/* RCAR_EMMC_PARTITION_BOOT_1 */
	/* RCAR_EMMC_PARTITION_USER   */
} file_state_t;

#define RCAR_GET_FLASH_ADR(a, b)	((uint32_t)((0x40000U * (a)) + (b)))
#define RCAR_ATTR_SET_CALCADDR(a)	((a) & 0xF)
#define RCAR_ATTR_SET_ISNOLOAD(a)	(((a) & 0x1) << 16U)
#define RCAR_ATTR_SET_CERTOFF(a)	(((a) & 0xF) << 8U)
#define RCAR_ATTR_SET_ALL(a, b, c)	((uint32_t)(RCAR_ATTR_SET_CALCADDR(a) |\
					RCAR_ATTR_SET_ISNOLOAD(b) |\
					RCAR_ATTR_SET_CERTOFF(c)))

#define RCAR_ATTR_GET_CALCADDR(a)	((a) & 0xFU)
#define RCAR_ATTR_GET_ISNOLOAD(a)	(((a) >> 16) & 0x1U)
#define RCAR_ATTR_GET_CERTOFF(a)	((uint32_t)(((a) >> 8) & 0xFU))

#define RCAR_MAX_BL3X_IMAGE		(8U)
#define RCAR_SECTOR6_CERT_OFFSET	(0x400U)
#define RCAR_SDRAM_certESS		(0x43F00000U)
#define RCAR_CERT_SIZE			(0x800U)
#define RCAR_CERT_INFO_SIZE_OFFSET	(0x264U)
#define RCAR_CERT_INFO_DST_OFFSET	(0x154U)
#define RCAR_CERT_INFO_SIZE_OFFSET1	(0x364U)
#define RCAR_CERT_INFO_DST_OFFSET1	(0x1D4U)
#define RCAR_CERT_INFO_SIZE_OFFSET2	(0x464U)
#define RCAR_CERT_INFO_DST_OFFSET2	(0x254U)
#define RCAR_CERT_LOAD			(1U)

#define RCAR_FLASH_CERT_HEADER		RCAR_GET_FLASH_ADR(6U, 0U)
#define RCAR_EMMC_CERT_HEADER		(0x00030000U)

#define RCAR_COUNT_LOAD_BL33		(2U)
#define RCAR_COUNT_LOAD_BL33X		(3U)

static const plat_rcar_name_offset_t name_offset[] = {
	{BL31_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(0, 0, 0)},

	/* BL3-2 is optional in the platform */
	{BL32_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(1, 0, 1)},
	{BL33_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(2, 0, 2)},
	{BL332_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(3, 0, 3)},
	{BL333_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(4, 0, 4)},
	{BL334_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(5, 0, 5)},
	{BL335_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(6, 0, 6)},
	{BL336_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(7, 0, 7)},
	{BL337_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(8, 0, 8)},
	{BL338_IMAGE_ID, 0U, RCAR_ATTR_SET_ALL(9, 0, 9)},
};

#if TRUSTED_BOARD_BOOT
static const plat_rcar_name_offset_t cert_offset[] = {
	/* Certificates */
	{TRUSTED_KEY_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 0)},
	{SOC_FW_KEY_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 0)},
	{TRUSTED_OS_FW_KEY_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 0)},
	{NON_TRUSTED_FW_KEY_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 0)},
	{SOC_FW_CONTENT_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 0)},
	{TRUSTED_OS_FW_CONTENT_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 1)},
	{NON_TRUSTED_FW_CONTENT_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 2)},
	{BL332_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 3)},
	{BL333_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 4)},
	{BL334_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 5)},
	{BL335_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 6)},
	{BL336_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 7)},
	{BL337_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 8)},
	{BL338_CERT_ID, 0U, RCAR_ATTR_SET_ALL(0, 1, 9)},
};
#endif /* TRUSTED_BOARD_BOOT */

static file_state_t current_file = { 0 };

static uintptr_t rcar_handle, rcar_spec;
static uint64_t rcar_image_header[RCAR_MAX_BL3X_IMAGE + 2U] = { 0U };
static uint64_t rcar_image_header_prttn[RCAR_MAX_BL3X_IMAGE + 2U] = { 0U };
static uint64_t rcar_image_number = { 0U };
static uint32_t rcar_cert_load = { 0U };

static io_type_t device_type_rcar(void)
{
	return IO_TYPE_FIRMWARE_IMAGE_PACKAGE;
}

int32_t rcar_get_certificate(const int32_t name, uint32_t *cert)
{
#if TRUSTED_BOARD_BOOT
	int32_t i;

	for (i = 0; i < ARRAY_SIZE(cert_offset); i++) {
		if (name != cert_offset[i].name) {
			continue;
		}

		*cert = RCAR_CERT_SIZE;
		*cert *= RCAR_ATTR_GET_CERTOFF(cert_offset[i].attr);
		*cert += RCAR_SDRAM_certESS;
		return 0;
	}
#endif
	return -EINVAL;
}

static int32_t file_to_offset(const int32_t name, uintptr_t *offset,
			      uint32_t *cert, uint32_t *no_load,
			      uintptr_t *partition)
{
	uint32_t addr;
	int32_t i;

	for (i = 0; i < ARRAY_SIZE(name_offset); i++) {
		if (name != name_offset[i].name) {
			continue;
		}

		addr = RCAR_ATTR_GET_CALCADDR(name_offset[i].attr);
		if (rcar_image_number + 2U < addr) {
			continue;
		}

		*offset = rcar_image_header[addr];
		*cert = RCAR_CERT_SIZE;
		*cert *= RCAR_ATTR_GET_CERTOFF(name_offset[i].attr);
		*cert += RCAR_SDRAM_certESS;
		*no_load = RCAR_ATTR_GET_ISNOLOAD(name_offset[i].attr);
		*partition = rcar_image_header_prttn[addr];
		return IO_SUCCESS;
	}

#if TRUSTED_BOARD_BOOT
	for (i = 0; i < ARRAY_SIZE(cert_offset); i++) {
		if (name != cert_offset[i].name) {
			continue;
		}

		*no_load = RCAR_ATTR_GET_ISNOLOAD(cert_offset[i].attr);
		*partition = 0U;
		*offset = 0U;
		*cert = 0U;
		return IO_SUCCESS;
	}
#endif
	return -EINVAL;
}

#define RCAR_BOOT_KEY_CERT_NEW	(0xE6300F00U)
#define	RCAR_CERT_MAGIC_NUM	(0xE291F358U)

void rcar_read_certificate(uint64_t cert, uint32_t *len, uintptr_t *dst)
{
	uint32_t seed, val, info_1, info_2;
	uintptr_t size, dsth, dstl;

	cert &= 0xFFFFFFFFU;

	seed = mmio_read_32(RCAR_BOOT_KEY_CERT_NEW);
	val = mmio_read_32(RCAR_BOOT_KEY_CERT_NEW + 0xC);
	info_1 = (val >> 18) & 0x3U;
	val = mmio_read_32(cert + 0xC);
	info_2 = (val >> 21) & 0x3;

	if (seed == RCAR_CERT_MAGIC_NUM) {
		if (info_1 != 1) {
			ERROR("BL2: Cert is invalid.\n");
			*dst = 0;
			*len = 0;
			return;
		}

		if (info_2 > 2) {
			ERROR("BL2: Cert is invalid.\n");
			*dst = 0;
			*len = 0;
			return;
		}

		switch (info_2) {
		case 2:
			size = cert + RCAR_CERT_INFO_SIZE_OFFSET2;
			dstl = cert + RCAR_CERT_INFO_DST_OFFSET2;
			break;
		case 1:
			size = cert + RCAR_CERT_INFO_SIZE_OFFSET1;
			dstl = cert + RCAR_CERT_INFO_DST_OFFSET1;
			break;
		case 0:
			size = cert + RCAR_CERT_INFO_SIZE_OFFSET;
			dstl = cert + RCAR_CERT_INFO_DST_OFFSET;
			break;
		}

		*len = mmio_read_32(size) * 4U;
		dsth = dstl + 4U;
		*dst = ((uintptr_t) mmio_read_32(dsth) << 32) +
		    ((uintptr_t) mmio_read_32(dstl));
		return;
	}

	size = cert + RCAR_CERT_INFO_SIZE_OFFSET;
	*len = mmio_read_32(size) * 4U;
	dstl = cert + RCAR_CERT_INFO_DST_OFFSET;
	dsth = dstl + 4U;
	*dst = ((uintptr_t) mmio_read_32(dsth) << 32) +
	    ((uintptr_t) mmio_read_32(dstl));
}

static int32_t check_load_area(uintptr_t dst, uintptr_t len)
{
	uint32_t legacy = dst + len <= UINT32_MAX - 1 ? 1 : 0;
	uintptr_t dram_start, dram_end;
	uintptr_t prot_start, prot_end;
	int32_t result = IO_SUCCESS;

	dram_start = legacy ? DRAM1_BASE : DRAM_40BIT_BASE;

	dram_end = legacy ? DRAM1_BASE + DRAM1_SIZE :
	    DRAM_40BIT_BASE + DRAM_40BIT_SIZE;

	prot_start = legacy ? DRAM_PROTECTED_BASE : DRAM_40BIT_PROTECTED_BASE;

	prot_end = prot_start + DRAM_PROTECTED_SIZE;

	if (dst < dram_start || dst > dram_end - len) {
		ERROR("BL2: dst address is on the protected area.\n");
		result = IO_FAIL;
		goto done;
	}

	/* load image is within SDRAM protected area */
	if (dst >= prot_start && dst < prot_end) {
		ERROR("BL2: dst address is on the protected area.\n");
		result = IO_FAIL;
	}

	if (dst < prot_start && dst > prot_start - len) {
		ERROR("BL2: loaded data is on the protected area.\n");
		result = IO_FAIL;
	}
done:
	if (result == IO_FAIL) {
		ERROR("BL2: Out of range : dst=0x%lx len=0x%lx\n", dst, len);
	}

	return result;
}

static int32_t load_bl33x(void)
{
	static int32_t loaded = IO_NOT_SUPPORTED;
	uintptr_t dst, partition, handle;
	uint32_t noload, cert, len, i;
	uintptr_t offset;
	int32_t rc;
	size_t cnt;
	const int32_t img[] = {
		BL33_IMAGE_ID,
		BL332_IMAGE_ID,
		BL333_IMAGE_ID,
		BL334_IMAGE_ID,
		BL335_IMAGE_ID,
		BL336_IMAGE_ID,
		BL337_IMAGE_ID,
		BL338_IMAGE_ID
	};

	if (loaded != IO_NOT_SUPPORTED) {
		return loaded;
	}

	for (i = 1; i < rcar_image_number; i++) {
		rc = file_to_offset(img[i], &offset, &cert, &noload,
				    &partition);
		if (rc != IO_SUCCESS) {
			WARN("%s: failed to get offset\n", __func__);
			loaded = IO_FAIL;
			return loaded;
		}

		rcar_read_certificate((uint64_t) cert, &len, &dst);
		((io_drv_spec_t *) rcar_spec)->partition = partition;

		rc = io_open(rcar_handle, rcar_spec, &handle);
		if (rc != IO_SUCCESS) {
			WARN("%s: Failed to open FIP (%i)\n", __func__, rc);
			loaded = IO_FAIL;
			return loaded;
		}

		rc = io_seek(handle, IO_SEEK_SET, offset);
		if (rc != IO_SUCCESS) {
			WARN("%s: failed to seek\n", __func__);
			loaded = IO_FAIL;
			return loaded;
		}

		rc = check_load_area(dst, len);
		if (rc != IO_SUCCESS) {
			WARN("%s: check load area\n", __func__);
			loaded = IO_FAIL;
			return loaded;
		}

		rc = io_read(handle, dst, len, &cnt);
		if (rc != IO_SUCCESS) {
			WARN("%s: failed to read\n", __func__);
			loaded = IO_FAIL;
			return loaded;
		}
#if TRUSTED_BOARD_BOOT
		rc = auth_mod_verify_img(img[i], (void *)dst, len);
		if (rc != 0) {
			memset((void *)dst, 0x00, len);
			loaded = IO_FAIL;
			return loaded;
		}
#endif
		io_close(handle);
	}

	loaded = IO_SUCCESS;

	return loaded;
}

static int32_t rcar_dev_init(io_dev_info_t *dev_info, const uintptr_t name)
{
	uint64_t header[64] __aligned(FLASH_TRANS_SIZE_UNIT) = {0UL};
	uintptr_t handle;
	ssize_t offset;
	uint32_t i;
	int32_t rc;
	size_t cnt;

	/* Obtain a reference to the image by querying the platform layer */
	rc = plat_get_drv_source(name, &rcar_handle, &rcar_spec);
	if (rc != IO_SUCCESS) {
		WARN("Failed to obtain reference to img %ld (%i)\n", name, rc);
		return IO_FAIL;
	}

	if (rcar_cert_load == RCAR_CERT_LOAD) {
		return IO_SUCCESS;
	}

	rc = io_open(rcar_handle, rcar_spec, &handle);
	if (rc != IO_SUCCESS) {
		WARN("Failed to access img %ld (%i)\n", name, rc);
		return IO_FAIL;
	}

	/*
	 * get start address list
	 * [0] address num
	 * [1] BL33-1 image address
	 * [2] BL33-2 image address
	 * [3] BL33-3 image address
	 * [4] BL33-4 image address
	 * [5] BL33-5 image address
	 * [6] BL33-6 image address
	 * [7] BL33-7 image address
	 * [8] BL33-8 image address
	 */
	offset = name == EMMC_DEV_ID ? RCAR_EMMC_CERT_HEADER :
	    RCAR_FLASH_CERT_HEADER;
	rc = io_seek(handle, IO_SEEK_SET, offset);
	if (rc != IO_SUCCESS) {
		WARN("Firmware Image Package header failed to seek\n");
		goto error;
	}
#if RCAR_BL2_DCACHE == 1
	inv_dcache_range((uint64_t) header, sizeof(header));
#endif
	rc = io_read(handle, (uintptr_t) &header, sizeof(header), &cnt);
	if (rc != IO_SUCCESS) {
		WARN("Firmware Image Package header failed to read\n");
		goto error;
	}

	rcar_image_number = header[0];
	for (i = 0; i < rcar_image_number + 2; i++) {
		rcar_image_header[i] = header[i * 2 + 1];
		rcar_image_header_prttn[i] = header[i * 2 + 2];
	}

	if (rcar_image_number == 0 || rcar_image_number > RCAR_MAX_BL3X_IMAGE) {
		WARN("Firmware Image Package header check failed.\n");
		goto error;
	}

	rc = io_seek(handle, IO_SEEK_SET, offset + RCAR_SECTOR6_CERT_OFFSET);
	if (rc != IO_SUCCESS) {
		WARN("Firmware Image Package header failed to seek cert\n");
		goto error;
	}
#if RCAR_BL2_DCACHE == 1
	inv_dcache_range(RCAR_SDRAM_certESS,
			 RCAR_CERT_SIZE * (2 + rcar_image_number));
#endif
	rc = io_read(handle, RCAR_SDRAM_certESS,
		     RCAR_CERT_SIZE * (2 + rcar_image_number), &cnt);
	if (rc != IO_SUCCESS) {
		WARN("cert file read error.\n");
		goto error;
	}

	rcar_cert_load = RCAR_CERT_LOAD;
error:

	if (rc != IO_SUCCESS) {
		rc = IO_FAIL;
	}

	io_close(handle);

	return rc;

}

static int32_t rcar_file_open(io_dev_info_t *info, const uintptr_t file_spec,
			      io_entity_t *entity)
{
	const io_drv_spec_t *spec = (io_drv_spec_t *) file_spec;
	uintptr_t partition, offset, dst;
	uint32_t noload, cert, len;
	int32_t rc;

	/*
	 * Only one file open at a time. We need to  track state (ie, file
	 * cursor position). Since the header lives at offset zero, this entry
	 * should never be zero in an active file.
	 * Once the system supports dynamic memory allocation we will allow more
	 * than one open file at a time.
	 */
	if (current_file.offset != 0U) {
		WARN("%s: Only one open file at a time.\n", __func__);
		return IO_RESOURCES_EXHAUSTED;
	}

	rc = file_to_offset(spec->offset, &offset, &cert, &noload, &partition);
	if (rc != IO_SUCCESS) {
		WARN("Failed to open file name %ld (%i)\n", spec->offset, rc);
		return IO_FAIL;
	}

	if (noload != 0U) {
		current_file.offset = 1;
		current_file.dst = 0;
		current_file.size = 1;
		current_file.position = 0;
		current_file.no_load = noload;
		current_file.partition = 0;
		entity->info = (uintptr_t) &current_file;

		return IO_SUCCESS;
	}

	rcar_read_certificate((uint64_t) cert, &len, &dst);

	/* Baylibre: HACK */
	if (spec->offset == BL31_IMAGE_ID && len < RCAR_TRUSTED_SRAM_SIZE) {
		WARN("%s,%s\n", "r-car ignoring the BL31 size from certificate",
		     "using RCAR_TRUSTED_SRAM_SIZE instead");
		len = RCAR_TRUSTED_SRAM_SIZE;
	}

	current_file.partition = partition;
	current_file.no_load = noload;
	current_file.offset = offset;
	current_file.position = 0;
	current_file.size = len;
	current_file.dst = dst;
	entity->info = (uintptr_t) &current_file;

	return IO_SUCCESS;
}

static int32_t rcar_file_len(io_entity_t *entity, size_t *length)
{
	*length = ((file_state_t *) entity->info)->size;

	NOTICE("%s: len: 0x%08lx\n", __func__, *length);

	return IO_SUCCESS;
}

static int32_t rcar_file_read(io_entity_t *entity, uintptr_t buffer,
			      size_t length, size_t *cnt)
{
	file_state_t *fp = (file_state_t *) entity->info;
	ssize_t offset = fp->offset + fp->position;
	uintptr_t handle;
	int32_t rc;

#ifdef SPD_NONE
	static uint32_t load_bl33x_counter = 1;
#else
	static uint32_t load_bl33x_counter;
#endif
	if (current_file.no_load != 0U) {
		*cnt = length;
		return IO_SUCCESS;
	}

	((io_drv_spec_t *) rcar_spec)->partition = fp->partition;

	rc = io_open(rcar_handle, rcar_spec, &handle);
	if (rc != IO_SUCCESS) {
		WARN("Failed to open FIP (%i)\n", rc);
		return IO_FAIL;
	}

	rc = io_seek(handle, IO_SEEK_SET, offset);
	if (rc != IO_SUCCESS) {
		WARN("%s: failed to seek\n", __func__);
		goto error;
	}

	if (load_bl33x_counter == RCAR_COUNT_LOAD_BL33) {
		rc = check_load_area(buffer, length);
		if (rc != IO_SUCCESS) {
			WARN("%s: load area err\n", __func__);
			goto error;
		}
	}

	rc = io_read(handle, buffer, length, cnt);
	if (rc != IO_SUCCESS) {
		WARN("Failed to read payload (%i)\n", rc);
		goto error;
	}

	fp->position += *cnt;
	io_close(handle);

	load_bl33x_counter += 1;
	if (load_bl33x_counter == RCAR_COUNT_LOAD_BL33X) {
		return load_bl33x();
	}

	return IO_SUCCESS;
error:
	io_close(handle);
	return IO_FAIL;
}

static int32_t rcar_file_close(io_entity_t *entity)
{
	if (current_file.offset != 0U) {
		memset(&current_file, 0, sizeof(current_file));
	}

	entity->info = 0U;

	return IO_SUCCESS;
}

static const io_dev_funcs_t rcar_dev_funcs = {
	.type = &device_type_rcar,
	.open = &rcar_file_open,
	.seek = NULL,
	.size = &rcar_file_len,
	.read = &rcar_file_read,
	.write = NULL,
	.close = &rcar_file_close,
	.dev_init = &rcar_dev_init,
	.dev_close = &rcar_dev_close,
};

static const io_dev_info_t rcar_dev_info = {
	.funcs = &rcar_dev_funcs,
	.info = (uintptr_t) 0
};

static const io_dev_connector_t rcar_dev_connector = {
	.dev_open = &rcar_dev_open
};

static int32_t rcar_dev_open(const uintptr_t dev_spec __attribute__ ((unused)),
			     io_dev_info_t **dev_info)
{
	*dev_info = (io_dev_info_t *) &rcar_dev_info;

	return IO_SUCCESS;
}

static int32_t rcar_dev_close(io_dev_info_t *dev_info)
{
	rcar_handle = 0;
	rcar_spec = 0;

	return IO_SUCCESS;
}

int32_t rcar_register_io_dev(const io_dev_connector_t **dev_con)
{
	int32_t result;

	result = io_register_device(&rcar_dev_info);
	if (result == IO_SUCCESS) {
		*dev_con = &rcar_dev_connector;
	}

	return result;
}

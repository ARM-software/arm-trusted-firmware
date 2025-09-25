/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <qti_plat.h>

static const io_dev_connector_t *qti_fip_dev_con;
static uintptr_t qti_fip_dev_handle;

static const io_dev_connector_t *qti_backend_dev_con;
static uintptr_t qti_backend_dev_handle;

static io_block_spec_t qti_fip_spec = {
	.offset = PLAT_QTI_FIP_IOBASE,
	.length = PLAT_QTI_FIP_MAXSIZE,
};

static const io_uuid_spec_t qti_bl31_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t qti_bl32_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t qti_bl33_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

struct qti_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct qti_io_policy qti_io_policies[] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &qti_backend_dev_handle,
		.image_spec = (uintptr_t)&qti_fip_spec,
	},
	[BL31_IMAGE_ID] = {
		.dev_handle = &qti_fip_dev_handle,
		.image_spec = (uintptr_t)&qti_bl31_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &qti_fip_dev_handle,
		.image_spec = (uintptr_t)&qti_bl32_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &qti_fip_dev_handle,
		.image_spec = (uintptr_t)&qti_bl33_spec,
		.init_params = FIP_IMAGE_ID,
	},
};

static int qti_io_memmap_setup(void)
{
	int ret;

	ret = mmap_add_dynamic_region(qti_fip_spec.offset, qti_fip_spec.offset,
				      qti_fip_spec.length, MT_RO_DATA | MT_SECURE);
	if (ret) {
		return ret;
	}

	ret = register_io_dev_memmap(&qti_backend_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(qti_backend_dev_con, 0, &qti_backend_dev_handle);
}

static int qti_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&qti_fip_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(qti_fip_dev_con, 0, &qti_fip_dev_handle);
}

int qti_io_setup(void)
{
	int ret;

	ret = qti_io_memmap_setup();
	if (ret) {
		return ret;
	}

	ret = qti_io_fip_setup();
	if (ret) {
		return ret;
	}

	return 0;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(qti_io_policies));

	*dev_handle = *qti_io_policies[image_id].dev_handle;
	*image_spec = qti_io_policies[image_id].image_spec;
	init_params = qti_io_policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}

/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Preparation and Validation
 *
 * This module prepares and validates devices for operations by performing
 * initialization checks and handling exclusive access constraints before device operations.
 */

#include <errno.h>
#include <common/debug.h>

#include <ti_device.h>
#include <ti_device_prepare.h>
#include <ti_host_idx_mapping.h>

static int32_t device_prepare(uint8_t host_id, uint32_t id, uint8_t *host_idx,
			      struct ti_device **dev, bool exclusive)
{
	struct ti_device *local_device;
	uint8_t local_host_idx;

	local_device = device_api_lookup(id);
	if (!local_device) {
		VERBOSE("BAD_DEVICE: dev_id=%d\n", id);
		return -EINVAL;
	}

	if (local_device->initialized == 0U) {
		return -EINVAL;
	}

	local_host_idx = host_idx_lookup(host_id);
	if (local_host_idx == HOST_IDX_NONE) {
		return -EINVAL;
	}

	if (exclusive && (local_device->exclusive != 0U) &&
	    (local_device->exclusive != local_host_idx)) {
		VERBOSE("EXCLUSIVE_DEVICE: dev_id=%d holder_host=%d\n",
			id, host_id);
		return -EINVAL;
	}

	if (dev != NULL) {
		*dev = local_device;
	}
	if (host_idx != NULL) {
		*host_idx = local_host_idx;
	}

	return 0;
}

int32_t device_prepare_exclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				 struct ti_device **device_ptr)
{
	int32_t ret;

	/* Ensure devices are fully initialized to allow modification */
	ret = ti_devices_init_rw();
	if (ret != 0) {
		return ret;
	}

	return device_prepare(host_id, id, host_idx, device_ptr, true);
}

int32_t device_prepare_nonexclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				    struct ti_device **device_ptr)
{
	return device_prepare(host_id, id, host_idx, device_ptr, false);
}

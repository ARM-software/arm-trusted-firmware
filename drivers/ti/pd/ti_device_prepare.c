/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Preparation and Validation
 *
 * This module prepares and validates devices for operations by performing
 * initialization checks and handling exclusive access constraints before device operations.
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>

#include <ti_device.h>
#include <ti_device_prepare.h>
#include <ti_host_idx_mapping.h>

/**
 * device_prepare() - Look up a device and host, and validate access.
 * @host_id: Host ID making the request.
 * @id: Device ID to look up.
 * @host_idx: If non-NULL, filled with the resolved host index on success.
 * @dev: If non-NULL, filled with the device pointer on success.
 * @exclusive: If true, enforce exclusive ownership — fails if the device is
 *             marked exclusive and the caller is not the owner.
 *
 * Return: 0 on success, negative error code otherwise.
 */
static int32_t device_prepare(uint8_t host_id, uint32_t id, uint8_t *host_idx,
			      struct ti_device **dev, bool exclusive)
{
	struct ti_device *local_device;
	uint8_t local_host_idx;

	assert(dev != NULL || host_idx != NULL);

	local_device = ti_device_api_lookup(id);
	if (local_device == NULL) {
		VERBOSE("BAD_DEVICE: dev_id=%d\n", id);
		return -EINVAL;
	}

	if (local_device->initialized == 0U) {
		return -EINVAL;
	}

	local_host_idx = ti_host_idx_lookup(host_id);
	if (local_host_idx == TI_HOST_IDX_NONE) {
		return -EINVAL;
	}

	if (exclusive && (local_device->exclusive != 0U) &&
	    (local_device->exclusive != local_host_idx)) {
		VERBOSE("EXCLUSIVE_DEVICE: dev_id=%d holder=%d requester=%d\n",
			id, local_device->exclusive, host_id);
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

int32_t ti_device_prepare_exclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
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

int32_t ti_device_prepare_nonexclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				       struct ti_device **device_ptr)
{
	return device_prepare(host_id, id, host_idx, device_ptr, false);
}

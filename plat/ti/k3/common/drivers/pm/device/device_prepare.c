/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <device_prepare.h>
#include <device.h>
#include <lib/trace.h>
#include <types/errno.h>
#include <host_idx_mapping.h>

static int32_t device_prepare(uint8_t host_id, uint32_t id, uint8_t *host_idx, struct device **dev,
			      bool exclusive)
{
	int32_t ret = SUCCESS;
	struct device *local_device = NULL;
	uint8_t local_host_idx = HOST_IDX_NONE;

	local_device = device_api_lookup(id);
	if (!local_device) {
		pm_trace(TRACE_PM_ACTION_BAD_DEVICE, id);
		ret = -EINVAL;
	}

	if (ret == SUCCESS) {
		if (local_device->initialized == 0U) {
			ret = -EINVAL;
		}
	}

	if (ret == SUCCESS) {
		local_host_idx = host_idx_lookup(host_id);
		if (local_host_idx == HOST_IDX_NONE) {
			ret = -EINVAL;
		}
	}

	if ((ret == SUCCESS) && exclusive) {
		if ((local_device->exclusive != 0U) &&
		    (local_device->exclusive != local_host_idx)) {
			pm_trace(TRACE_PM_ACTION_EXCLUSIVE_DEVICE,
				 (((uint32_t) id << TRACE_PM_VAL_EXCLUSIVE_DEVICE_ID_SHIFT) &
				  TRACE_PM_VAL_EXCLUSIVE_DEVICE_ID_MASK) |
				 (((uint32_t) host_id << TRACE_PM_VAL_EXCLUSIVE_HOST_ID_SHIFT) &
				  TRACE_PM_VAL_EXCLUSIVE_HOST_ID_MASK));
			ret = -EINVAL;
		}
	}

	if (dev != NULL) {
		*dev = local_device;
	}
	if (host_idx != NULL) {
		*host_idx = local_host_idx;
	}

	return ret;
}

int32_t device_prepare_exclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				 struct device **device_ptr)
{
	int32_t ret;

	/* Ensure devices are fully initialized to allow modification */
	ret = devices_init_rw();

	if (ret == SUCCESS) {
		ret = device_prepare(host_id, id, host_idx, device_ptr, true);
	}
	return ret;
}

int32_t device_prepare_nonexclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				    struct device **device_ptr)
{
	return device_prepare(host_id, id, host_idx, device_ptr, false);
}

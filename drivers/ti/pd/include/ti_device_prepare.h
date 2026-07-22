/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Preparation API
 *
 * This header provides device preparation functions for power management
 * requests, including validation of device access permissions, exclusive
 * ownership checks, and host index/device lookup resolution.
 */

#ifndef TI_DEVICE_PREPARE_H
#define TI_DEVICE_PREPARE_H

#include <stdint.h>

struct ti_device;

/**
 * ti_device_prepare_exclusive() - Prepare to operate on device with exclusive check
 * @host_id: The host ID making the request.
 * @id: The device ID to operate on.
 * @host_idx: Optional output for host index. If non-NULL and lookup
 *            succeeds, filled with the host index value.
 * @device_ptr: Optional output for device pointer. If non-NULL and
 *              lookup succeeds, filled with the device pointer.
 *
 * This function takes the steps necessary for the PM subsystem to prepare
 * to operate on a device when receiving a power management request. It
 * ensures that if the device is marked exclusive, we are the owner. This
 * version should be used if device properties will be modified.
 *
 * Appropriate trace messages are produced if errors are encountered.
 *
 * Return: 0 if all checks succeeded, less than zero otherwise.
 */
int32_t ti_device_prepare_exclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				    struct ti_device **device_ptr);

/**
 * ti_device_prepare_nonexclusive() - Prepare to operate on device with non-exclusive check
 * @host_id: The host ID making the request.
 * @id: The device ID to operate on.
 * @host_idx: Optional output for host index. If non-NULL and lookup
 *              succeeds, filled with the host index value.
 * @device_ptr: Optional output for device pointer. If non-NULL and
 *              lookup succeeds, filled with the device pointer.
 *
 * This function takes the steps necessary for the PM subsystem to prepare
 * to operate on a device when receiving a power management request. No
 * check is made in regard to the exclusive state of the device. This
 * version should only be used if the device properties will not be
 * modified.
 *
 * Appropriate trace messages are produced if errors are encountered.
 *
 * Return: 0 if all checks succeeded, less than zero otherwise.
 */
int32_t ti_device_prepare_nonexclusive(uint8_t host_id, uint32_t id,
				       uint8_t *host_idx,
				       struct ti_device **device_ptr);

#endif /* TI_DEVICE_PREPARE_H */

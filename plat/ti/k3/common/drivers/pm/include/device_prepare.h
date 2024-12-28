/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEVICE_PREPARE_H
#define DEVICE_PREPARE_H

#include <types/short_types.h>

struct device;

/**
 * \brief Prepare to operate on device with exclusive check
 *
 * This function takes the steps necessary for the PM subsystem to prepare
 * to operate on a device when receiving a TISCI request. It ensures that
 * if the device is marked exclusive, we are the owner. This version should
 * be used if device properties will be modified.
 *
 * Appropriate trace messages are produced if errors are encountred.
 *
 * \param host_id
 * The host ID making the request.
 *
 * \param id
 * The device ID to operate on.
 *
 * \param host_idx
 * If this parameter is non-NULL and the host index lookup succeeds, the
 * pointer is filled with the host index value.
 *
 * \param device_ptr
 * If this parameter is non-NULL and the device ID lookup succeeds, the
 * pointer is filled with the device pointer.
 *
 * \return
 * SUCCESS if all checks succeeded, less than zero otherwise.
 */
int32_t device_prepare_exclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				 struct device **device_ptr);

/**
 * \brief Prepare to operate on device with exclusive check
 *
 * This function takes the steps necessary for the PM subsystem to prepare
 * to operate on a device when receiving a TISCI request. No check is made
 * in regard to the exclusive state of the device. This version should only
 * be used if the device properties will not be modified.
 *
 * Appropriate trace messages are produced if errors are encountred.
 *
 * \param host_id
 * The host ID making the request.
 *
 * \param id
 * The device ID to operate on.
 *
 * \param host_idx
 * If this parameter is non-NULL and the host index lookup succeeds, the
 * pointer is filled with the host index value.
 *
 * \param device_ptr
 * If this parameter is non-NULL and the device ID lookup succeeds, the
 * pointer is filled with the device pointer.
 *
 * \return
 * SUCCESS if all checks succeeded, less than zero otherwise.
 */
int32_t device_prepare_nonexclusive(uint8_t host_id, uint32_t id, uint8_t *host_idx,
				    struct device **device_ptr);

#endif

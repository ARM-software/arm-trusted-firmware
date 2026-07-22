/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Handler API
 *
 * This header provides device state handler functions and constants for
 * managing device states and retention modes as part of the power management
 * subsystem.
 */

#ifndef TI_DEVICE_HANDLER_H
#define TI_DEVICE_HANDLER_H

/*
 * Turn device off when possible. This must be used in conjunction with
 * SoC dependencies to identify the overall power domain state being
 * achieved.
 */
#define TI_DEVICE_SW_STATE_AUTO_OFF    0U

/* Disable device but keep in retention. */
#define TI_DEVICE_SW_STATE_RETENTION   1U

/* Turn device on for usage. */
#define TI_DEVICE_SW_STATE_ON	    2U

/*
 * Hardware state indicating device is on.
 */
#define TI_DEVICE_HW_STATE_ON	    1U

/**
 * ti_set_device_handler() - Set device power state
 * @dev_id: Device ID to set state for
 * @enable: true to enable device, false to disable
 *
 * Return: 0 on success, -EINVAL on invalid state, -EIO on hardware mismatch
 */
int32_t ti_set_device_handler(uint32_t dev_id, bool enable);

/*
 * ti_get_device_handler() - Get device power state
 * @dev_id: Device ID to get state for
 *
 * Return: true if enabled, false if disabled
 */
bool ti_get_device_handler(uint32_t dev_id);

#endif /* TI_DEVICE_HANDLER_H */

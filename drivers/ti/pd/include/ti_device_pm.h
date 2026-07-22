/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Power Management API
 *
 * This header provides device power management operations including device
 * enable/disable, reset isolation, retention control, context loss tracking,
 * and SoC-specific device initialization and state management functions.
 */

#ifndef TI_DEVICE_PM_H
#define TI_DEVICE_PM_H

#include <stdbool.h>

#include <ti_device.h>

/**
 * ti_soc_device_get_state() - Get the current device state (SoC specific impl.)
 * @dev: The device to query.
 *
 * Return: Non-zero if the device is enabled, zero if disabled.
 */
uint32_t ti_soc_device_get_state(struct ti_device *dev);

/**
 * ti_soc_device_set_reset_iso() - Set the reset isolation flag for a device (SoC specific impl.)
 * @dev: The device to modify.
 * @enable: True to enable reset isolation, false to disable.
 */
void ti_soc_device_set_reset_iso(struct ti_device *dev, bool enable);

/**
 * ti_soc_device_get_context_loss_count() - Get context loss count
 * @device: The device to query.
 *
 * Return: The current loss count of the device.
 */
uint32_t ti_soc_device_get_context_loss_count(struct ti_device *dev);

/**
 * ti_soc_device_enable() - Enable a device (SoC specific impl.)
 * @device: The device to modify.
 */
void ti_soc_device_enable(struct ti_device *dev);

/**
 * ti_soc_device_disable() - Disable a device (SoC specific impl.)
 * @device: The device to modify.
 * @domain_reset: True if the device is being disabled due to a domain reset.
 */
void ti_soc_device_disable(struct ti_device *dev, bool domain_reset);

/**
 * ti_soc_device_clear_flags() - Clear a device initialization flags
 * @dev: The device to modify.
 */
void ti_soc_device_clear_flags(struct ti_device *dev);

/**
 * ti_soc_device_ret_enable() - Enable retention on a device (SoC specific impl.)
 * @device: The device to modify.
 */
void ti_soc_device_ret_enable(struct ti_device *dev);

/**
 * ti_soc_device_ret_disable() - Disable retention on a device (SoC specific impl.)
 * @device: The device to modify.
 */
void ti_soc_device_ret_disable(struct ti_device *dev);

/**
 * ti_soc_device_init() - Perform SoC level initialization of a device.
 *
 * The device to init.
 *
 * Return: 0 on success, -EAGAIN if device is not yet ready to be initialized.
 */
int32_t ti_soc_device_init(struct ti_device *dev);

/**
 * ti_soc_device_init_complete() - Notify SoC device impl. that device init is complete.
 *
 * This allows the SoC implementation to drop any extra references it's been
 * holding during init.
 */
void ti_soc_device_init_complete(void);

/**
 * ti_device_disable() - Disables a device.
 * @device: The device to modify.
 * @domain_reset: True if the device is being disabled due to a domain reset.
 *
 * Performs the steps necessary to disable a device.
 */
void ti_device_disable(struct ti_device *dev, bool domain_reset);

/**
 * ti_device_set_state() - Enable/disable a device.
 * @device: The device to modify.
 * @host_idx: The index of the host making the request.
 * @enable: True to enable the device, false to allow the PMMC to power down the device.
 *
 * This indicates the desired state of a device by the HLOS. If a device is
 * enabled, the PMMC will make sure the device and its dependencies are
 * ready. If the device is not enabled, the PMMC will opportunistically
 * utilize power management modes of that device and its dependencies.
 *
 * Enabling a device (if disabled) increments the device's reference count.
 * Disabling a device (if enabled) decrements the devices' reference count.
 */
void ti_device_set_state(struct ti_device *dev, uint8_t host_idx, bool enable);

/**
 * ti_device_set_retention() - Enable/disable retention on a device.
 * @device: The device to modify.
 * @retention: True to enable retention, false to disable it.
 *
 * When a device is in retention, but disabled, the PMMC can still perform
 * power management tasks, but the device must keep its context. Enabling
 * retention is a way save power, but still be able to bring the device back
 * to full functionality quickly.
 */
void ti_device_set_retention(struct ti_device *dev, bool retention);

/**
 * ti_device_clear_flags() - Clear a device initialization flags
 * @dev: The device to modify.
 */
void ti_device_clear_flags(struct ti_device *dev);

/**
 * ti_device_set_reset_iso() - Set the reset isolation flag for a device.
 * @device: The device to modify.
 * @enable: True to enable reset isolation, false to disable.
 *
 * The effect of reset isolation is device and SoC specific, but it generally
 * prevents the device from undergoing reset with the rest of the SoC.
 */
static inline void ti_device_set_reset_iso(struct ti_device *dev, bool enable)
{
	ti_soc_device_set_reset_iso(dev, enable);
}

/**
 * ti_device_id_disable_clocks() - Disable all clocks for a device
 * @idx: The device index
 *
 * Disables all clocks associated with the specified device.
 *
 * Must be paired with ti_device_id_power_down() to complete the
 * device disable sequence.
 */
void ti_device_id_disable_clocks(ti_dev_idx_t idx);

/**
 * ti_device_id_power_down() - Power down a device without touching clocks
 * @idx: The device index
 *
 * Powers down the specified device without touching clocks. This function
 * is designed to be called when clock disable operations will be done by
 * ti_device_id_disable_clocks()
 */
void ti_device_id_power_down(ti_dev_idx_t idx);

/**
 * ti_device_id_enable_clocks() - Enable all clocks for a device
 * @idx: The device index
 *
 * Enables all clocks associated with the specified device.
 *
 * Must be paired with ti_device_id_power_up() to complete the
 * device enable sequence.
 */
void ti_device_id_enable_clocks(ti_dev_idx_t idx);

/**
 * ti_device_id_power_up() - Power up a device without touching clocks
 * @idx: The device index
 *
 * Powers up the specified device without touching clocks. Should be
 * paired with ti_device_id_enable_clocks() to complete the device
 * enable sequence.
 */
void ti_device_id_power_up(ti_dev_idx_t idx);

/* Return values for ti_device_get_state() and ti_soc_device_get_state() */
#define TI_DEVICE_STATE_DISABLED        0U /* Module is off (SwRstDisable) */
#define TI_DEVICE_STATE_ENABLED         1U /* Module is enabled or in retention */
#define TI_DEVICE_STATE_TRANSITIONING   2U /* Module is transitioning or domains mixed */

/**
 * ti_device_get_state() - Get the current device state.
 * @device: The device to query.
 *
 * Returns the current device state as configured by ti_device_set_state.
 *
 * Return: TI_DEVICE_STATE_DISABLED, TI_DEVICE_STATE_ENABLED, or
 *         TI_DEVICE_STATE_TRANSITIONING.
 */
static inline uint32_t ti_device_get_state(struct ti_device *dev)
{
	return ti_soc_device_get_state(dev);
}

#endif /* TI_DEVICE_PM_H */

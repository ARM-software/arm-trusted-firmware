/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEVICE_PM_H
#define DEVICE_PM_H

#include <stdbool.h>
#include <types/short_types.h>

struct notifier;
#define CONFIG_PM
#ifdef CONFIG_PM

/** Register a notifier for when the device enables/disables */
void register_device_notifier(struct notifier *n);

/**
 * \brief Get the current device state (SoC specific impl.)
 *
 * \param dev
 * The device to query.
 *
 * \return
 * True if the device is configured to be enabled, false if the device is
 * configured to be disabled.
 */
extern uint32_t soc_device_get_state(struct device *dev);

/**
 * \brief Set the reset isolation flag for a device (SoC specific impl.)
 *
 * \param dev
 * The device to modify.
 *
 * \param enable
 * True to enable reset isolation, false to disable.
 */
extern void soc_device_set_reset_iso(struct device *dev, bool enable);

/**
 * \brief Get the reset isolation flag for a device (SoC specific impl.)
 *
 * \param dev
 * The device to query.
 *
 * \return
 * True if reset isolation flag is set, false otherwise
 */
extern bool soc_device_get_reset_iso(struct device *dev);

/**
 * \brief Set the state of the device resets (SoC specific impl.)
 *
 * \param dev
 * The device to modify.
 *
 * \param resets
 * Bit-field representing desired reset state.
 */
extern void soc_device_set_resets(struct device *dev, uint32_t resets);

/**
 * \brief Get the current state of the device resets (SoC specific impl.)
 *
 * \param device
 * The device to query.
 *
 * \return
 * The current bitfield of device resets, as set by device_set_resets.
 */
extern uint32_t soc_device_get_resets(struct device *dev);

/**
 * \brief Get the current device context loss count (SoC specific impl.)
 *
 * \param device
 * The device to query.
 *
 * \return
 * The current loss count of the device.
 */
extern uint32_t soc_device_get_context_loss_count(struct device *dev);

/**
 * \brief Enable a device (SoC specific impl.)
 *
 * \param device
 * The device to modify.
 */
extern void soc_device_enable(struct device *dev);

/**
 * \brief Disable a device (SoC specific impl.)
 *
 * \param device
 * The device to modify.
 * \param domain_reset
 * True if the device is being disabled due to a domain reset.
 */
extern void soc_device_disable(struct device *dev, bool domain_reset);

/**
 * \brief Clear a device initialization flags
 *
 * \param dev The device to modify.
 *
 */
extern void soc_device_clear_flags(struct device *dev);

/**
 * \brief Enable retention on a device (SoC specific impl.)
 *
 * \param device
 * The device to modify.
 */
extern void soc_device_ret_enable(struct device *dev);

/**
 * \brief Disable retention on a device (SoC specific impl.)
 *
 * \param device
 * The device to modify.
 */
extern void soc_device_ret_disable(struct device *dev);

/**
 * \brief Perform SoC level initialization of a device.
 *
 * \param
 * The device to init.
 *
 * \return
 * 0 on success, -EDEFER if device is not yet ready to be initialized.
 */
extern int32_t soc_device_init(struct device *dev);

/**
 * \brief Notify SoC device impl. that device init is complete.
 *
 * This allows the SoC implementation to drop any extra references it's been
 * holding during init.
 */
extern void soc_device_init_complete(void);

/**
 * \brief Disables a device.
 *
 * Performs the steps necessary to disable a device.
 *
 * \param device
 * The device to modify.
 * \param domain_reset
 * True if the device is being disabled due to a domain reset.
 */
extern void device_disable(struct device *dev, bool domain_reset);

/**
 * \brief Set the reset isolation flag for a device.
 *
 * The effect of reset isolation is device and SoC specific, but it generally
 * prevents the device from undergoing reset with the rest of the SoC.
 *
 * \param device
 * The device to modify.
 *
 * \param enable
 * True to enable reset isolation, false to disable.
 */
static inline void device_set_reset_iso(struct device *dev, bool enable)
{
	soc_device_set_reset_iso(dev, enable);
}

/**
 * \brief Get the reset isolation flag for a device.
 *
 * \param device
 * The device to query.
 *
 * \return
 * True if the reset isolation flag is set, false otherwise.
 */
static inline bool device_get_reset_iso(struct device *dev)
{
	return soc_device_get_reset_iso(dev);
}

/**
 * \brief Set the state of the device resets.
 *
 * This sets the state of the device resets via bitfield. The meaning of each
 * bit is device and SoC specific.
 *
 * \param device
 * The device to modify.
 *
 * \param resets
 * Bit-field representing desired reset state.
 */
static inline void device_set_resets(struct device *dev, uint32_t resets)
{
	soc_device_set_resets(dev, resets);
}

/**
 * \brief Get the current state of the device resets.
 *
 * \param device
 * The device to query.
 *
 * \return
 * The current bitfield of device resets, as set by device_set_resets.
 */
static inline uint32_t device_get_resets(struct device *dev)
{
	return soc_device_get_resets(dev);
}

/**
 * \brief Get the current device context loss count.
 *
 * The context loss count of the device is tracked by the PMMC. Whenever the
 * device loses context, the context loss counter is incremented. The device
 * can only lose context when disabled. When enabling a device, the HLOS can
 * query the context loss count and compare it with the context loss count
 * read the last time the device was enabled. If the count differs, context
 * has been lost and must be restored.
 *
 * \param device
 * The device to query.
 *
 * \return
 * The current loss count of the device.
 */
static inline uint32_t device_get_context_loss_count(struct device *dev)
{
	return soc_device_get_context_loss_count(dev);
}

/**
 * \brief Call the suspend callback for a device.
 *
 * This calls the suspend callback for a device if it has one. It properly
 * checks for the DEVD_FLAG_DRV_DATA to ensure the device has extra driver
 * specific data (including a drv pointer) before attempting to access the
 * drv pointer. It also ensures the drv pointer and the suspend callback
 * are non-NULL
 *
 * \param device
 * Which device to call the suspend callback on. This argument is also passed
 * to the callback.
 */
void device_suspend(struct device *dev);

/**
 * \brief Enable/disable a device.
 *
 * This indicates the desired state of a device by the HLOS. If a device is
 * enabled, the PMMC will make sure the device and it's dependencies are
 * ready. If the device is not enabled, the PMMC will opportunistically
 * utilize power management modes of that device and it's dependencies.
 *
 * Enabling a device (if disabled) increments the device's reference count.
 * Disabling a device (if enabled) decrements the devices' reference count.
 *
 * \param device
 * The device to modify.
 *
 * \param host_idx
 * The index of the host making the requset.
 *
 * \param enable
 * True to enable the device, false to allow the PMMC to power down the device.
 */
void device_set_state(struct device *dev, uint8_t host_idx, bool enable);

/**
 * \brief Get the current device state.
 *
 * Returns the current device state as configured by device_set_state.
 *
 * \param device
 * The device to query.
 *
 * \return
 * True if the device is configured to be enabled, false if the device is
 * configured to be disabled.
 */
static inline uint32_t device_get_state(struct device *dev)
{
	return soc_device_get_state(dev);
}

/**
 * \brief Enable/disable retention on a device.
 *
 * When a device is in retention, but disabled, the PMMC can still perform
 * power management tasks, but the device must keep it's context. Enabling
 * retention is a way save power, but still be able to bring the device back
 * to full functionality quickly.
 *
 * \param device
 * The device to modify.
 *
 * \param retention
 * True to enable retention, false to disable it.
 */
void device_set_retention(struct device *dev, bool retention);

/**
 * \brief Clear a device initialization flags
 *
 * \param dev The device to modify.
 *
 */
void device_clear_flags(struct device *dev);

#else

static inline uint32_t device_get_state(struct device *dev)
{
	return 0;
}

static inline void device_set_state(struct device *dev, uint8_t host_idx, bool enable)
{
}
static inline void device_set_retention(struct device *dev, bool ret)
{
}

static int32_t soc_device_init(struct device *dev)
{
	return 0;
}

static inline void soc_device_init_complete(void)
{
}

static inline void device_clear_flags(struct device *dev)
{
}
#endif

#endif

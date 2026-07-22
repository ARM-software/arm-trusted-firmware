/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Core API
 *
 * This header defines the core device management API including device data
 * structures, device lookup functions, driver callbacks, device group
 * management, and device initialization/deinitialization operations.
 */

#ifndef TI_DEVICE_H
#define TI_DEVICE_H

#include <cdefs.h>
#include <stddef.h>

#include <lib/utils_def.h>
#include <ti_container_of.h>
#include <ti_devgrps.h>
#include <ti_hosts.h>
#include <ti_pm_types.h>
#include <ti_psc_soc_device.h>

/* Invalid device ID marker */
#define TI_DEV_ID_NONE			((ti_dev_idx_t) (-1L))

/*
 * Clock flags, shared with device flags field for devices with only one
 * clock.
 */
/* Disable device clock */
#define TI_DEV_CLK_FLAG_DISABLE		BIT(0)

/* Host index for power-on enabled device state */
#define TI_DEV_POWER_ON_ENABLED_HOST_IDX	TI_HOST_ID_CNT

/* Device flags */
/* Device retention mode flag */
#define TI_DEV_FLAG_RETENTION		BIT(4)

/* Starting bit position for per-host enabled flags */
#define TI_DEV_FLAG_ENABLED_BIT		5UL

/* Generate per-host enabled flag for a given host index */
#define TI_DEV_FLAG_ENABLED(host_idx)	(1ULL << ((TI_DEV_FLAG_ENABLED_BIT) + (host_idx)))

/* Device was enabled at power-on */
#define TI_DEV_FLAG_POWER_ON_ENABLED	TI_DEV_FLAG_ENABLED(TI_DEV_POWER_ON_ENABLED_HOST_IDX)

/* Note, can support TI_HOST_ID_CNT up to 26 */
/* Mask covering all per-host enabled flags */
#define TI_DEV_FLAG_ENABLED_MASK		(((1ULL << (TI_HOST_ID_CNT + 1ULL)) - 1ULL) << 5ULL)

/* Const flags for dev_data */

/* Set if struct ti_dev_data is contained within a drv_data */
#define TI_DEVD_FLAG_DRV_DATA		BIT(0)

/*
 * Set if the device is only for internal access and should not be accessible
 * via the msgmgr API from the HLOS.
 */
#define TI_DEVD_FLAG_INTERNAL		BIT(1)

/* Set if the devices init callback should be called at boot */
#define TI_DEVD_FLAG_DO_INIT		BIT(2)

/* Internal mapping from devices to device groups */
#define TI_PM_DEVGRP_00			1U

struct ti_device;
struct ti_dev_clk;
struct ti_dev_clk_data;

/*
 * Device driver callback interface
 *
 * Provides lifecycle callbacks for device-specific initialization
 * and deinitialization.
 */
struct ti_drv {
	/* Pre-initialization callback, returns error code on failure */
	int32_t (*pre_init)(struct ti_device *dev);

	/* Post-initialization callback, returns error code on failure */
	int32_t (*post_init)(struct ti_device *dev);

	/* Deinitialization callback */
	void	(*uninit)(struct ti_device *dev);
};

/*
 * Constant device configuration data
 *
 * Contains immutable device properties including clock associations,
 * SoC-specific data, and device group membership.
 */
struct ti_dev_data {
	/* Index into soc_dev_clk_data/soc_dev_clk array */
	uint16_t dev_clk_idx;

	/* SoC specific data */
	struct ti_soc_device_data soc;

	/* Number of device attached clocks */
	uint16_t n_clocks;

	/* Const flags for this device, DEVD_FLAG_... */
	uint8_t flags;

	/* Device group membership for power management */
	uint8_t pm_devgrp;
};

/*
 * Driver specific data for device
 *
 * This expands on the const device data to include data that is useful to
 * a driver for the device. If a device is described by a drv_data struct,
 * then the TI_DEVD_FLAG_DRV_DATA should be set.
 */
struct ti_drv_data {
	/* Driver for device if preset */
	const struct ti_drv *drv;

	/* Encapsulated dev_data */
	struct ti_dev_data dev_data;
};

/*
 * Dynamic device runtime data
 *
 * Tracks device state including ownership, initialization status, and
 * runtime flags such as per-host enabled state and retention mode.
 */
struct ti_device {
	/* Runtime flags (DEV_FLAG_*) */
	uint32_t flags;

	/* Host index of exclusive owner+1, 0 for none */
	uint8_t exclusive;

	/* Initialization state flag */
	uint8_t initialized;
};

/*
 * Devgroup information
 *
 * Information to locate and describe devgroups. A device's dev_data will
 * be located within it's devgroup elf section. The devgroup information
 * describes the start and end of that section.
 *
 * The devgroup information also provides the dev_clk_data and dev_clk
 * arrays for the devices within the devgroup. This allows these arrays
 * to be stored within their respective devgroup section.
 */
struct ti_devgroup {
	/* dev_clk_data array for this devgroup */
	const struct ti_dev_clk_data *dev_clk_data;

	/* dev_clk array for this devgroup */
	struct ti_dev_clk *dev_clk;

	/* Starting clock index for this devgroup */
	ti_clk_idx_t clk_idx;
};

/* Array of dynamic device runtime data for all SoC devices */
extern struct ti_device soc_devices[];

/* Total number of devices in the SoC */
extern const size_t soc_device_count;

/* Array of pointers to constant device configuration data */
extern const struct ti_dev_data *const soc_device_data_arr[];

/* Array of SoC device groups for power management */
extern const struct ti_devgroup soc_devgroups[];

/* Total number of device groups in the SoC */
extern const size_t soc_devgroup_count;

/**
 * ti_pm_devgroup_is_enabled() - Test a set of devgroups for PM
 * @groups: A bitfield of devgroups to test.
 *
 * Return: true if any of the devgroups in groups are enabled, false otherwise.
 */
bool ti_pm_devgroup_is_enabled(devgrp_t groups);

int32_t ti_devices_init(void);

/**
 * ti_devices_init_rw() - Complete deferred init.
 *
 * Ensure that deferred device init has completed. Note that if deferred init
 * has already completed, this function takes no action and returns 0.
 *
 * Return: 0 on success, <0 otherwise.
 */
int32_t ti_devices_init_rw(void);

/**
 * ti_device_id_valid() - Determine if a device ID from the API is valid.
 * @id: A 32 bit device ID from the msgmgr API.
 *
 * Return: True if the device ID refers to a valid SoC device, false otherwise
 */
static inline bool ti_device_id_valid(uint32_t id)
{
	return (id < (uint32_t) soc_device_count) && (soc_device_data_arr[id] != NULL);
}

/*
 * Internal lookup a device based on a device ID
 *
 * This does a lookup of a device based on the device ID and is meant only
 * for internal use.
 */
static inline struct ti_device *ti_device_lookup(ti_dev_idx_t id)
{
	return ti_device_id_valid(id) ? (soc_devices + id) : NULL;
}

/*
 * API lookup a device based on a device ID
 *
 * This does a lookup of a device based on a API id from the msgmgr API.
 * This function hides any devices with the TI_DEVD_FLAG_INTERNAL set.
 */
static inline struct ti_device *ti_device_api_lookup(uint32_t id)
{
	return (!ti_device_id_valid(id) ||
		((soc_device_data_arr[id]->flags & TI_DEVD_FLAG_INTERNAL) != 0U)) ?
		NULL : (soc_devices + id);
}

/**
 * ti_device_id() - Return the device ID based on a device pointer.
 * @device: The device pointer.
 *
 * The device pointer is just an index into the array of devices. This is
 * used to return a device ID. This function has no error checking for NULL
 * pointers.
 *
 * Return: The device ID.
 */
static inline uint32_t ti_device_id(struct ti_device *dev)
{
	return (uint32_t) (dev - soc_devices);
}

/*
 * Return the dev_data struct associated with a device.
 *
 * The device pointer is translated to a device index based on it's position
 * in the soc_devices array. That index is then used to find the correct
 * element within the soc_device_data array.
 */
static inline const struct ti_dev_data *ti_get_dev_data(struct ti_device *dev)
{
	return soc_device_data_arr[ti_device_id(dev)];
}

/*
 * Return a drv_data struct based on a passed dev_data struct
 *
 * The returned pointer is only valid if the TI_DEVD_FLAG_DRV_DATA flag is set.
 */
static inline const struct ti_drv_data *ti_to_drv_data(const struct ti_dev_data *data)
{
	return ti_container_of(data, const struct ti_drv_data, dev_data);
}

/*
 * Return the drv_data struct associated with a device.
 *
 * The returned pointer is only valid if the TI_DEVD_FLAG_DRV_DATA flag is set.
 */
static inline const struct ti_drv_data *ti_get_drv_data(struct ti_device *dev)
{
	const struct ti_dev_data *dev_datap = ti_get_dev_data(dev);

	return dev_datap ? ti_to_drv_data(dev_datap) : NULL;
}

/*
 * Find the devgroup associated with a given dev data entry.
 *
 * Each device is assigned to a devgroup. This function determines which
 * devgroup a device is assigned to by checking where it's dev_data is
 * stored.
 */
static inline const struct ti_devgroup *ti_dev_data_lookup_devgroup(const struct ti_dev_data *data)
{
	if (data == NULL) {
		return NULL;
	}

	return &soc_devgroups[data->pm_devgrp];
}

#endif /* TI_DEVICE_H */

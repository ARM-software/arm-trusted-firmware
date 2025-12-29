/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
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

#ifndef DEVICE_H
#define DEVICE_H

#include <cdefs.h>
#include <stddef.h>

#include <lib/utils_def.h>

#include <ti_hosts.h>
#include <ti_container_of.h>
#include <ti_pm_types.h>
#include <ti_soc_device.h>
#include <ti_devgrps.h>

/** \brief Invalid device ID marker */
#define DEV_ID_NONE			((dev_idx_t) (-1L))

/*
 * Clock flags, shared with device flags field for devices with only one
 * clock.
 */
/** \brief Disable device clock */
#define DEV_CLK_FLAG_DISABLE		BIT(0)

/** \brief Allow device clock frequency changes */
#define DEV_CLK_FLAG_ALLOW_FREQ_CHANGE	BIT(2)

/** \brief Host index for power-on enabled device state */
#define DEV_POWER_ON_ENABLED_HOST_IDX	HOST_ID_CNT

/* Device flags */
/** \brief Device retention mode flag */
#define DEV_FLAG_RETENTION		BIT(4)

/** \brief Starting bit position for per-host enabled flags */
#define DEV_FLAG_ENABLED_BIT		5UL

/** \brief Generate per-host enabled flag for a given host index */
#define DEV_FLAG_ENABLED(host_idx)	(1ULL << ((DEV_FLAG_ENABLED_BIT) + (host_idx)))

/** \brief Device was enabled at power-on */
#define DEV_FLAG_POWER_ON_ENABLED	DEV_FLAG_ENABLED(DEV_POWER_ON_ENABLED_HOST_IDX)

/* Note, can support HOST_ID_CNT up to 26 */
/** \brief Mask covering all per-host enabled flags */
#define DEV_FLAG_ENABLED_MASK		(((1ULL << (HOST_ID_CNT + 1ULL)) - 1ULL) << 5ULL)

/* Const flags for dev_data */

/** Set if struct ti_dev_data is contained within a drv_data */
#define DEVD_FLAG_DRV_DATA		BIT(0)

/**
 * Set if the device is only for internal access and should not be accessible
 * via the msgmgr API from the HLOS.
 */
#define DEVD_FLAG_INTERNAL		BIT(1)

/** Set if the devices init callback should be called at boot */
#define DEVD_FLAG_DO_INIT		BIT(2)

/** \brief Internal mapping from devices to device groups */
#define PM_DEVGRP_00			1U

struct ti_device;
struct ti_dev_clk;
struct ti_dev_clk_data;
struct resource;

/**
 * \brief Device driver callback interface
 *
 * Provides lifecycle callbacks for device-specific initialization,
 * deinitialization, and suspend operations.
 */
struct ti_drv {
	/** Pre-initialization callback, returns error code on failure */
	int32_t (*pre_init)(struct ti_device *dev);

	/** Post-initialization callback, returns error code on failure */
	int32_t (*post_init)(struct ti_device *dev);

	/** Deinitialization callback */
	void	(*uninit)(struct ti_device *dev);

	/** Suspend callback for power management */
	void	(*suspend)(struct ti_device *dev);
};

/**
 * \brief Constant device configuration data
 *
 * Contains immutable device properties including clock associations,
 * SoC-specific data, and device group membership.
 */
struct ti_dev_data {
	/** Index into soc_dev_clk_data/soc_dev_clk array */
	uint16_t dev_clk_idx;

	/** SoC specific data */
	struct ti_soc_device_data soc;

	/** Number of device attached clocks */
	uint16_t n_clocks;

	/** Const flags for this device, DEVD_FLAG_... */
	uint8_t flags;

	/** Device group membership for power management */
	uint8_t pm_devgrp;
};

/**
 * \brief Driver specific data for device
 *
 * This expands on the const device data to include data that is useful to
 * a driver for the device. If a device is described by a drv_data struct,
 * then the DEVD_FLAG_DRV_DATA should be set.
 */
struct ti_drv_data {
	/** Driver for device if preset */
	const struct ti_drv *drv;

	/** Resources for device */
	const uint8_t *r;

	/** Encapsulated dev_data */
	struct ti_dev_data dev_data;
};

/**
 * \brief Dynamic device runtime data
 *
 * Tracks device state including ownership, initialization status, and
 * runtime flags such as per-host enabled state and retention mode.
 */
struct ti_device {
	/** Host *index* of exclusive owner+1, 0 for none */
	uint8_t exclusive;

	/** Initialization state flag */
	uint8_t initialized;

	/** Runtime flags (DEV_FLAG_*) */
	uint32_t flags;
} __packed;

/**
 * \brief Devgroup information
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
	/** dev_clk_data array for this devgroup */
	const struct ti_dev_clk_data *dev_clk_data;

	/** dev_clk array for this devgroup */
	struct ti_dev_clk *dev_clk;

	/** Starting clock index for this devgroup */
	clk_idx_t clk_idx;
};

/** \brief Array of dynamic device runtime data for all SoC devices */
extern struct ti_device soc_devices[];

/** \brief Total number of devices in the SoC */
extern const size_t soc_device_count;

/** \brief Array of pointers to constant device configuration data */
extern const struct ti_dev_data *const soc_device_data_arr[];

/** \brief Array of SoC device groups for power management */
extern const struct ti_devgroup soc_devgroups[];

/** \brief Total number of device groups in the SoC */
extern const size_t soc_devgroup_count;

#define CONFIG_PM
#ifdef CONFIG_PM
/**
 * \brief Test a set of devgroups for PM
 *
 * \param groups A bitfield of devgroups to test.
 * \return true if any of the devgroups in groups are enabled, false
 * otherwise.
 */
bool ti_pm_devgroup_is_enabled(devgrp_t groups);

int32_t ti_devices_init(void);

/**
 * \brief Complete deferred init.
 *
 * Ensure that deferred device init has completed. Note that if deferred init
 * has already completed, this function takes no action and returns 0.
 *
 * \return 0 on success, <0 otherwise.
 */
int32_t ti_devices_init_rw(void);

/**
 * \brief Determine if a device ID from the API is valid.
 *
 * \param id
 * A 32 bit device ID from the msgmgr API.
 *
 * \return
 * True if the device ID refers to a valid SoC device, false otherwise
 */
static inline bool device_id_valid(uint32_t id)
{
	return (id < (uint32_t) soc_device_count) && soc_device_data_arr[id];
}

/**
 * \brief Internal lookup a device based on a device ID
 *
 * This does a lookup of a device based on the device ID and is meant only
 * for internal use.
 *
 * \param id
 * A 8 bit device ID.
 *
 * \return
 * The device pointer, or NULL if the device ID was not valid.
 */
static inline struct ti_device *device_lookup(dev_idx_t id)
{
	return device_id_valid(id) ? (soc_devices + id) : NULL;
}

/**
 * \brief API lookup a device based on a device ID
 *
 * This does a lookup of a device based on a API id from the msgmgr API.
 * This function hides any devices with the DEVD_FLAG_INTERNAL set.
 *
 * \param id
 * A 32 bit device ID from the msgmgr API.
 *
 * \return
 * The device pointer, or NULL if the device ID was not valid.
 */
static inline struct ti_device *device_api_lookup(uint32_t id)
{
	return (!device_id_valid(id) ||
		((soc_device_data_arr[id]->flags & DEVD_FLAG_INTERNAL) != 0U)) ?
		NULL : (soc_devices + id);
}

/**
 * \brief Return the device ID based on a device pointer.
 *
 * The device pointer is just an index into the array of devices. This is
 * used to return a device ID. This function has no error checking for NULL
 * pointers.
 *
 * \param device
 * The device pointer.
 *
 * \return
 * The device ID.
 */
static inline uint32_t device_id(struct ti_device *dev)
{
	return (uint32_t) (dev - soc_devices);
}

/**
 * \brief Return the dev_data struct associated with a device.
 *
 * The device pointer is translated to a device index based on it's position
 * in the soc_devices array. That index is then used to find the correct
 * element within the soc_device_data array.
 *
 * \param dev
 * The device for which a dev_data struct should be returned.
 *
 * \return
 * The pointer to the dev_data struct. No error checking is performed.
 */
static inline const struct ti_dev_data *get_dev_data(struct ti_device *dev)
{
	return soc_device_data_arr[device_id(dev)];
}

/**
 * \brief Return a drv_data struct based on a passed dev_data struct
 *
 * The returned pointer is only valid if the DEVD_FLAG_DRV_DATA flag is set.
 *
 * \param data
 * Pointer to the dev_data struct that is a member of a drv_data struct.
 *
 * \return
 * The pointer to the drv_data struct. No error checking is performed.
 */
static inline const struct ti_drv_data *to_drv_data(const struct ti_dev_data *data)
{
	return container_of(data, const struct ti_drv_data, dev_data);
}

/**
 * \brief Return the drv_data struct associated with a device.
 *
 * The returned pointer is only valid if the DEVD_FLAG_DRV_DATA flag is set.
 *
 * \param dev
 * The device for which a drv_data struct should be returned.
 *
 * \return
 * The pointer to the drv_data struct. No error checking is performed.
 */
static inline const struct ti_drv_data *get_drv_data(struct ti_device *dev)
{
	const struct ti_dev_data *dev_datap = get_dev_data(dev);

	return dev_datap ? to_drv_data(dev_datap) : NULL;
}

/**
 * \brief Find the devgroup associated with a given dev data entry.
 *
 * Each device is assigned to a devgroup. This function determines which
 * devgroup a device is assigned to by checking where it's dev_data is
 * stored.
 *
 * \param data
 * The dev_data to use for the lookup. Note if pointer is NULL, the function
 * will always return NULL.
 *
 * \return
 * The associated devgroup, or NULL if the lookup fails.
 */
static inline const struct ti_devgroup *dev_data_lookup_devgroup(const struct ti_dev_data *data)
{
	const struct ti_devgroup *ret;

	if (data != NULL) {
		ret = &soc_devgroups[data->pm_devgrp];
	} else {
		ret = NULL;
	}
	return ret;
}

#else
/**
 * \brief Test a set of devgroups for PM (stub)
 *
 * This is an empty stub for use when PM is disabled.
 *
 * \param groups A bitfield of devgroups to test (unused).
 * \return Always true
 */
static inline bool ti_pm_devgroup_is_enabled(devgrp_t groups __unused)
{
	return true;
}
#endif

#endif

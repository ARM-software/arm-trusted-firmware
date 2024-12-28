/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <compiler.h>
#include <device_limits.h>
#include <hosts.h>
#include <lib/bitops.h>
#include <lib/container_of.h>
#include <lib/utils_def.h>
#include <pm_types.h>
#include <soc_device.h>
#include <stddef.h>
#include <types.h>
#include <types/devgrps.h>
#include <types/short_types.h>

#define DEV_ID_NONE			((dev_idx_t) (-1L))

/*
 * Clock flags, shared with device flags field for devices with only one
 * clock.
 */
#define DEV_CLK_FLAG_DISABLE		BIT(0)
#define DEV_CLK_FLAG_ALLOW_SSC		BIT(1)
#define DEV_CLK_FLAG_ALLOW_FREQ_CHANGE	BIT(2)
#define DEV_CLK_FLAG_INPUT_TERM		BIT(3)

#define DEV_POWER_ON_ENABLED_HOST_IDX	HOST_ID_CNT

/* Device flags */
#define DEV_FLAG_RETENTION		BIT(4)
#define DEV_FLAG_ENABLED_BIT		5UL
#define DEV_FLAG_ENABLED(host_idx)	(1ULL << ((DEV_FLAG_ENABLED_BIT) + (host_idx)))
#define DEV_FLAG_POWER_ON_ENABLED	DEV_FLAG_ENABLED(DEV_POWER_ON_ENABLED_HOST_IDX)

/* Note, can support HOST_ID_CNT up to 26 */
#define DEV_FLAG_ENABLED_MASK		(((1ULL << (HOST_ID_CNT + 1ULL)) - 1ULL) << 5ULL)

/* Const flags for dev_data */

/** Set if struct dev_data is contained within a drv_data */
#define DEVD_FLAG_DRV_DATA		BIT(0)

/**
 * Set if the device is only for internal access and should not be accessible
 * via the msgmgr API from the HLOS.
 */
#define DEVD_FLAG_INTERNAL		BIT(1)

/** Set if the devices init callback should be called at boot */
#define DEVD_FLAG_DO_INIT		BIT(2)

/** Internal mapping from devices to device groups */
#define PM_DEVGRP_DMSC			0U
#define PM_DEVGRP_00			1U
#define PM_DEVGRP_01			2U
#define PM_DEVGRP_02			3U
#define PM_DEVGRP_03			4U
#define PM_DEVGRP_HSM			5U

/** Max devgrp mapping for each SoC*/
#define J7200_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_01 + 1U)
#define J721E_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_01 + 1U)
#define J721S2_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)
#define J784S4_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)

#define AM64X_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_01 + 1U)
#define AM6_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_01 + 1U)
#define AM62X_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)
#define AM62LX_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)
#define AM62AX_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)
#define AM62PX_PM_DEVGRP_RANGE_ID_MAX (PM_DEVGRP_HSM + 1U)

struct device;
struct dev_clk;
struct dev_clk_data;
struct resource;

struct drv {
	int32_t	(*pre_init)(struct device *dev);
	int32_t	(*post_init)(struct device *dev);
	void	(*uninit)(struct device *dev);
	void	(*suspend)(struct device *dev);
};

struct dev_data {
	/** Index into soc_dev_clk_data/soc_dev_clk array */
	uint16_t			dev_clk_idx;

	/** SoC specific data */
	struct soc_device_data	soc;

	/** Number of device attached clocks */
	uint16_t			n_clocks : 10;

	/** Const flags for this device, DEVD_FLAG_... */
	uint16_t			flags : 3;

	uint16_t			pm_devgrp : 3;
};

/**
 * \brief Driver specific data for device
 *
 * This expands on the const device data to include data that is useful to
 * a driver for the device. If a device is described by a drv_data struct,
 * then the DEVD_FLAG_DRV_DATA should be set.
 */
struct drv_data {
	/** Driver for device if preset */
	const struct drv	*drv;

	/** Resources for device */
	const uint8_t		*r;

	/** Encapsulated dev_data */
	struct dev_data		dev_data;
};

/*
 * Select allocation strategy for devices based on the number of flags we
 * need which in turn is driven by the number of hosts we need to keep
 * track of.
 */
#if HOST_ID_CNT <= 20
struct device {
	/** Host *index* of exclusive owner+1, 0 for none */
	uint32_t	exclusive : 5;	/* Up to 31 (only need up to 22) */
	uint32_t	initialized : 1;
	uint32_t	flags : 26;	/* 6 flags plus 20 host id flags */
} __packed;
#elif HOST_ID_CNT <= 26
struct device {
	uint32_t	flags;		/* 6 flags plus 26 host id flags */
	/** Host *index* of exclusive owner+1, 0 for none */
	uint8_t	exclusive : 7;	/* Up to 127 (only need up to 27) */
	uint8_t	initialized : 1;
} __packed;
#elif HOST_ID_CNT <= 58
struct device {
	uint64_t	flags;		/* 6 flags plus 58 host id flags */
	/** Host *index* of exclusive owner+1, 0 for none */
	uint8_t	exclusive : 7;	/* Up to 127 (only need up to 27) */
	uint8_t	initialized : 1;
} __packed;
#else
#error "No allocation strategy for struct device"
#endif

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
struct devgroup {
	/** dev_clk_data array for this devgroup */
	const struct dev_clk_data	*dev_clk_data;

	/** dev_clk array for this devgroup */
	struct dev_clk			*dev_clk;

	/** Starting clock index for this devgroup */
	clk_idx_t			clk_idx;
};

/** The table of dynamic device data */
extern struct device soc_devices[SOC_DEVICES_RANGE_ID_MAX];

/** The table of const device data */
extern const struct dev_data *const soc_device_data_arr[SOC_DEVICES_DATA_ARR_RANGE_ID_MAX];

/** The total number of SoC devices */
extern const size_t soc_device_count;

/** The array of SoC device groups. */
extern const struct devgroup soc_devgroups[PM_DEVGRP_RANGE_ID_MAX];

/** The number of SoC device groups. */
extern const size_t soc_devgroup_count;

/**
 * The device ID of the running CPU. This is just the PMMC dev, except for
 * host builds where it is the host CPU the build is running from.
 */
extern struct device *const this_dev;

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
static inline struct device *device_lookup(dev_idx_t id)
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
static inline struct device *device_api_lookup(uint32_t id)
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
static inline uint32_t device_id(struct device *dev)
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
static inline const struct dev_data *get_dev_data(struct device *dev)
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
static inline const struct drv_data *to_drv_data(const struct dev_data *data)
{
	return container_of(data, const struct drv_data, dev_data);
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
static inline const struct drv_data *get_drv_data(struct device *dev)
{
	const struct dev_data *dev_datap = get_dev_data(dev);

	return dev_datap ? to_drv_data(dev_datap) : NULL;
}

/**
 * \brief Return the drv associated with a device.
 *
 * The returned pointer is only valid if the DEVD_FLAG_DRV_DATA flag is set.
 *
 * \param dev
 * The device for which a drv should be returned.
 *
 * \return
 * The drv pointer. No error checking is performed.
 */
static inline const struct drv *get_dev_drv(struct device *dev)
{
	const struct dev_data *dev_datap = get_dev_data(dev);

	return dev_datap ? to_drv_data(dev_datap)->drv : NULL;
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
static inline const struct devgroup *dev_data_lookup_devgroup(const struct dev_data *data)
{
	const struct devgroup *ret;

	if (data != NULL) {
		ret = &soc_devgroups[data->pm_devgrp];
	} else {
		ret = NULL;
	}
	return ret;
}

/**
 * \brief Find the devgroup associated with a given device
 *
 * Each device is assigned to a devgroup. This function determines which
 * devgroup a device is assigned to by checking where it's dev_data is
 * stored.
 *
 * \param dev
 * The device to lookup.
 *
 * \return
 * The associated devgroup, or NULL if the lookup fails.
 */
static inline const struct devgroup *dev_lookup_devgroup(struct device *dev)
{
	return dev_data_lookup_devgroup(get_dev_data(dev));
}
#define CONFIG_PM
#ifdef CONFIG_PM
/**
 * \brief Enable a set of devgroups for PM
 *
 * \param groups A bitfield of devgroups to enable.
 */
void pm_devgroup_set_enabled(devgrp_t groups);

/**
 * \brief Test a set of devgroups for PM
 *
 * \param groups A bitfield of devgroups to test.
 * \return true if any of the devgroups in groups are enabled, false
 * otherwise.
 */
bool pm_devgroup_is_enabled(devgrp_t groups);
#else
/**
 * \brief Enable a set of devgroups for PM (stub)
 *
 * This is an empty stub for use when PM is disabled.
 *
 * \param groups A bitfield of devgroups to enable (unused).
 */
static inline void pm_devgroup_set_enabled(devgrp_t groups UNUSED)
{
}
/**
 * \brief Test a set of devgroups for PM (stub)
 *
 * This is an empty stub for use when PM is disabled.
 *
 * \param groups A bitfield of devgroups to test (unused).
 * \return Always true
 */
static inline bool pm_devgroup_is_enabled(devgrp_t groups UNUSED)
{
	return true;
}
#endif

int32_t devices_init(void);

/**
 * \brief Complete deferred init.
 *
 * Ensure that deferred device init has completed. Note that if deferred init
 * has already completed, this function takes no action and returns SUCCESS.
 *
 * \return SUCCESS on success, <0 otherwise.
 */
int32_t devices_init_rw(void);

/**
 * \brief De-initialize devices in a PM device group.
 *
 * During reset of a domain group, some devices need to be de-initialized to
 * properly reflect device's state. It prepares the device for re-initialiation
 * after the domain reset.
 *
 * \return SUCCESS on success, <0 otherwise.
 */
int32_t devices_deinit(uint8_t pm_devgrp);

/**
 * \brief Clear all initialization flags
 *
 * During low power mode suspend all initialization flags of devices are
 * cleared and during resume the devices are reinitilized.
 *
 * \return SUCCESS on success, <0 otherwise.
 */
int32_t devices_deinit_flags(void);

/**
 * \brief Drop device power up references.
 *
 * Upon powerup, devices that are enabled at boot are marked as enabled by
 * powerup. The powerup ref is dropped when a host requests the device enabled
 * or disabled. This function call can drop all remaining powerup refs.
 */
void devices_drop_power_up_ref(void);

#endif

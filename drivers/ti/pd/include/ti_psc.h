/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * PSC Driver API
 *
 * This header provides the Power and Sleep Controller (PSC) driver interface
 * including data structures for power domains and LPSC modules, state
 * management functions, reset control, and module dependency handling.
 */

#ifndef PSC_H
#define PSC_H

#include <ti_device.h>
#include <ti_pm_types.h>

/**
 * \def PSC_DEV_NONE
 * \brief Indicates that no device is mapped to this PSC module
 *
 * Used in device-to-PSC mapping tables to indicate that a PSC module
 * has no associated device.
 */
#define PSC_DEV_NONE		7U

/**
 * \def PSC_DEV_MULTIPLE
 * \brief Indicates that multiple devices are mapped to this PSC module
 *
 * Used in device-to-PSC mapping tables to indicate that a PSC module
 * controls multiple devices. The actual device list is stored elsewhere.
 */
#define PSC_DEV_MULTIPLE	6U

/**
 * \def PSC_PD_EXISTS
 * \brief Flag indicating that the power domain exists and is valid
 *
 * Set in ti_psc_pd_data.flags to indicate that this power domain is
 * present and the information in the record is valid.
 */
#define PSC_PD_EXISTS		BIT(0)

/**
 * \def PSC_PD_ALWAYSON
 * \brief Flag indicating that the power domain is always on
 *
 * Set in ti_psc_pd_data.flags to indicate that this power domain cannot
 * be powered down and is always active.
 */
#define PSC_PD_ALWAYSON		BIT(1)

/**
 * \def PSC_PD_DEPENDS
 * \brief Flag indicating that the power domain has dependencies
 *
 * Set in ti_psc_pd_data.flags to indicate that this power domain depends
 * on another power domain being active. The dependency is specified in
 * the depends field.
 */
#define PSC_PD_DEPENDS		BIT(2)

/**
 * \struct ti_psc_pd_data
 * \brief PSC power domain constant data
 *
 * This structure contains the constant configuration data for a PSC power
 * domain, including its characteristics, dependencies, and clock requirements.
 * This data is typically stored in ROM or const data sections.
 */
struct ti_psc_pd_data {
	/**
	 * \brief Power domain configuration flags
	 *
	 * Combination of PSC_PD_EXISTS, PSC_PD_ALWAYSON, and PSC_PD_DEPENDS
	 * flags indicating the power domain's characteristics.
	 */
	uint8_t flags;

	/**
	 * \brief Index of the power domain this one depends on
	 *
	 * If PSC_PD_DEPENDS is set in flags, this field contains the pd_idx_t
	 * of the power domain that must be active before this one can be enabled.
	 */
	uint8_t depends;	/* pd_idx_t */

	/**
	 * \brief Clock dependencies for power domain transitions
	 *
	 * Some domains need a clock running in order to transition. This
	 * is the id of that clock. If no such clock is needed, set to
	 * an invalid clock id (0). We currently support only 3 clocks.
	 */
	clk_idx_t clock_dep[3];
};

/**
 * \struct ti_psc_pd
 * \brief PSC power domain dynamic data
 *
 * This structure contains the runtime state information for a PSC power
 * domain, tracking its usage and power-up enablement status. This data
 * changes during system operation.
 */
struct ti_psc_pd {
	/**
	 * \brief Reference count tracking power domain usage
	 *
	 * Number of modules or users currently requiring this power domain
	 * to be active. The domain can only be powered down when this
	 * count reaches zero.
	 */
	uint8_t use_count;

	/**
	 * \brief Power-up enable status
	 *
	 * True if this power domain is enabled for power-up operations,
	 * false otherwise. This is used to track the requested state of
	 * the power domain.
	 */
	bool pwr_up_enabled;
};

/** Module is present, information in record is valid */
#define LPSC_MODULE_EXISTS	BIT(0)

/** Module is not capable of clock gating */
#define LPSC_NO_CLOCK_GATING	BIT(1)

/** Module depends on another module listed in depends field */
#define LPSC_DEPENDS		BIT(2)

/** Module implements configurable reset isolation */
#define LPSC_HAS_RESET_ISO	BIT(3)

/** Module implements a local reset */
#define LPSC_HAS_LOCAL_RESET	BIT(4)

/** States that enable module reset are disallowed */
#define LPSC_NO_MODULE_RESET	BIT(5)

/**
 * Set if the module data points to a dev_list, false it a dev_array is
 * embedded.
 */
#define LPSC_DEVICES_LIST	BIT(6)

/**
 * \struct ti_lpsc_module_data
 * \brief LPSC module constant configuration data
 *
 * This structure contains the constant configuration data for a Local Power
 * Sleep Controller (LPSC) module, including device associations, dependencies,
 * clock requirements, and module characteristics. This data is typically stored
 * in ROM or const data sections and defines the static properties of each
 * LPSC module in the system.
 */
struct ti_lpsc_module_data {
	/**
	 * \brief Device-to-LPSC module association
	 *
	 * Which devices are members of which PSC module is duplicated in the
	 * list of devices. However, it is duplicated here so that the entire
	 * list of devices does not need to be looped through to find that
	 * information. It could be generated dynamically but is stored in this
	 * way to save memory.
	 */
	union {
		/**
		 * \brief Embedded device array for small device lists
		 *
		 * If there are 4 or less devices, they can be stored here,
		 * terminated by the 4th device or DEV_ID_NONE. This avoids
		 * the need for a separate pointer and saves memory.
		 */
		dev_idx_t dev_array[sizeof(void *) / sizeof(dev_idx_t)];

		/**
		 * \brief Pointer to external device list for large device lists
		 *
		 * More than 4 devices must be stored in a separate list,
		 * terminated by DEV_ID_NONE. The LPSC_DEVICES_LIST flag must be
		 * set when this field is used instead of dev_array.
		 */
		const dev_idx_t *dev_list;
	} lpsc_dev;

	/**
	 * \brief Clock dependencies for module transitions
	 *
	 * Some modules need a clock running in order to transition. This
	 * is the id of that clock. If no such clock is needed, set to
	 * an invalid clock id (0). We currently support only one clock,
	 * this causes the structure size to be 8 bytes in the case of an
	 * 8 bit clk_idx_t and 12 bytes for a 16 bit clk_idx_t.
	 */
	clk_idx_t clock_dep[1];

	/**
	 * \brief Module configuration flags
	 *
	 * Combination of LPSC_MODULE_EXISTS, LPSC_NO_CLOCK_GATING, LPSC_DEPENDS,
	 * LPSC_HAS_RESET_ISO, LPSC_HAS_LOCAL_RESET, LPSC_NO_MODULE_RESET, and
	 * LPSC_DEVICES_LIST flags indicating the module's characteristics and
	 * capabilities.
	 */
	uint8_t flags;

	/**
	 * \brief Index of the LPSC module this one depends on
	 *
	 * If LPSC_DEPENDS is set in flags, this field contains the lpsc_idx_t
	 * of the LPSC module that must be active before this one can be enabled.
	 */
	lpsc_idx_t depends;

	/**
	 * \brief PSC index of the dependency module
	 *
	 * If LPSC_DEPENDS is set in flags, this field contains the psc_idx_t
	 * of the PSC that controls the dependency module. This allows for
	 * cross-PSC dependencies.
	 */
	uint8_t depends_psc_idx;	/* psc_idx_t */

	/**
	 * \brief Power domain index this module belongs to
	 *
	 * The pd_idx_t of the power domain that controls this LPSC module.
	 * The module can only be active when its power domain is powered up.
	 */
	uint8_t powerdomain;		/* pd_idx_t */
};

/**
 * \struct ti_lpsc_module
 * \brief LPSC module dynamic runtime data
 *
 * This structure contains the runtime state information for an LPSC module,
 * tracking its usage, power state, reset status, and loss count. This data
 * changes during system operation as modules are enabled, disabled, and reset.
 */
struct ti_lpsc_module {
	/**
	 * \brief Module reset loss counter
	 *
	 * Incremented after module reset. This value is used to detect when
	 * a module has undergone a reset and lost its state, allowing drivers
	 * to know when reinitialization is necessary.
	 */
	uint32_t loss_count;

	/**
	 * \brief Active usage reference count
	 *
	 * Non-zero if module should be active (clocks running). Incremented
	 * by ti_lpsc_module_get() and decremented by ti_lpsc_module_put().
	 * The module transitions to enabled state when this count is non-zero.
	 */
	uint8_t use_count;

	/**
	 * \brief Retention reference count
	 *
	 * Non-zero if module should be powered-up but may be clock-gated.
	 * Incremented by ti_lpsc_module_ret_get() and decremented by
	 * ti_lpsc_module_ret_put(). This allows the module to retain state
	 * while saving power by stopping clocks.
	 */
	uint8_t ret_count;

	/**
	 * \brief Current programmed software state
	 *
	 * Current programmed state of the module (MDSTAT_STATE_[...]).
	 * Reflects the state written to hardware and may differ from the
	 * actual hardware state during transitions.
	 */
	uint8_t sw_state;

	/**
	 * \brief Module reset retention flag
	 *
	 * True if the module is forced on due to a module reset. In this
	 * case sw_state indicates SWRSTDISABLE but this module holds
	 * a reference count to its powerdomain. This prevents the power
	 * domain from being disabled while a reset is active.
	 */
	bool sw_mrst_ret;

	/**
	 * \brief Power-up enable status
	 *
	 * Non-zero if the module power-up has been enabled. This tracks
	 * whether the module is currently in a power-up enabled state.
	 */
	uint8_t pwr_up_enabled;

	/**
	 * \brief Power-up retention status
	 *
	 * Non-zero if the module is in power-up retention mode. This allows
	 * the module to maintain state while in a low-power configuration.
	 */
	uint8_t pwr_up_ret;

	/**
	 * \brief Module reset active flag
	 *
	 * True if host has requested a module reset. This indicates that
	 * a reset operation is currently active or pending for this module.
	 */
	uint8_t mrst_active;
};

/** Declare a default PSC powerdomain entry */
#define PSC_PD(id)[(id)] = { .flags = PSC_PD_EXISTS }

/** Declare an always on PSC powerdomain entry */
#define PSC_PD_AON(id)[(id)] = { .flags = PSC_PD_ALWAYSON | PSC_PD_EXISTS }

/** Declare a PSC powerdomain with a dependency */
#define PSC_PD_DEP(id, dep)[(id)] = { .depends = (dep),		\
				      .flags = PSC_PD_EXISTS |	\
				      PSC_PD_DEPENDS }

/**
 * \var psc_drv
 * \brief PSC driver operations structure
 *
 * Global constant structure containing the PSC driver operations and
 * callbacks. This structure implements the ti_drv interface for PSC
 * devices and is used by the device management framework to interact
 * with PSC hardware.
 */
extern const struct ti_drv psc_drv;

struct ti_soc_device_data;

/**
 * \var soc_psc_multiple_domains
 * \brief Array of devices controlled by multiple PSC modules
 *
 * Global constant array containing pointers to device data structures for
 * devices that are controlled by multiple PSC modules. This allows the
 * system to handle complex device-to-PSC mappings where a single device
 * may require coordination across multiple PSC modules. The array is
 * terminated by a NULL pointer.
 */
extern const struct ti_soc_device_data *const soc_psc_multiple_domains[];

/**
 * \brief Get the index of a power domain within its PSC
 *
 * Calculates the power domain index by determining the offset of the
 * power domain structure within the PSC's power domain array.
 *
 * \param psc_dev
 * The PSC device that controls this power domain.
 *
 * \param pd
 * Pointer to the power domain structure.
 *
 * \return
 * The pd_idx_t index of the power domain within the PSC.
 */
pd_idx_t ti_psc_pd_idx(struct ti_device *psc_dev, const struct ti_psc_pd *pd);

/**
 * \brief Wait for a power domain transition to complete
 *
 * Polls the hardware status registers until the power domain transition
 * completes. This function blocks until the power domain reaches its
 * target state.
 *
 * \param psc_dev
 * The PSC device that controls this power domain.
 *
 * \param pd
 * Pointer to the power domain to wait for.
 */
void ti_psc_pd_wait(struct ti_device *psc_dev, struct ti_psc_pd *pd);

/**
 * \brief Increment power domain reference count and enable if needed
 *
 * Increments the use_count for the power domain and powers it up if this
 * is the first reference. Also handles dependency power domains recursively.
 *
 * \param psc_dev
 * The PSC device that controls this power domain.
 *
 * \param pd
 * Pointer to the power domain to enable.
 */
void ti_psc_pd_get(struct ti_device *psc_dev, struct ti_psc_pd *pd);

/**
 * \brief Decrement power domain reference count and disable if unused
 *
 * Decrements the use_count for the power domain and powers it down if
 * the count reaches zero. Also handles dependency power domains recursively.
 *
 * \param psc_dev
 * The PSC device that controls this power domain.
 *
 * \param pd
 * Pointer to the power domain to potentially disable.
 */
void ti_psc_pd_put(struct ti_device *psc_dev, struct ti_psc_pd *pd);

/**
 * \brief Get the index of an LPSC module within its PSC
 *
 * Calculates the LPSC module index by determining the offset of the
 * module structure within the PSC's module array.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module structure.
 *
 * \return
 * The lpsc_idx_t index of the module within the PSC.
 */
lpsc_idx_t ti_lpsc_module_idx(struct ti_device *psc_dev, const struct ti_lpsc_module *module);

/**
 * \brief Increment LPSC module active reference count and enable
 *
 * Increments the use_count for the module and enables it (clocks running)
 * if this is the first reference. Also handles power domain and dependency
 * modules recursively. The module will be fully powered and clocked.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module to enable.
 */
void ti_lpsc_module_get(struct ti_device *psc_dev, struct ti_lpsc_module *module);

/**
 * \brief Decrement LPSC module active reference count and disable if unused
 *
 * Decrements the use_count for the module. If the count reaches zero and
 * ret_count is also zero, the module is disabled. Also handles power domain
 * and dependency modules recursively.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module to potentially disable.
 */
void ti_lpsc_module_put(struct ti_device *psc_dev, struct ti_lpsc_module *module);

/**
 * \brief Increment LPSC module retention reference count
 *
 * Increments the ret_count for the module, ensuring it remains powered
 * but allowing clock gating. The module retains its state while saving
 * power. Also handles power domain and dependency modules recursively.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module to put in retention mode.
 */
void ti_lpsc_module_ret_get(struct ti_device *psc_dev, struct ti_lpsc_module *module);

/**
 * \brief Decrement LPSC module retention reference count
 *
 * Decrements the ret_count for the module. If both ret_count and use_count
 * reach zero, the module can be fully disabled. Also handles power domain
 * and dependency modules recursively.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module to release from retention mode.
 */
void ti_lpsc_module_ret_put(struct ti_device *psc_dev, struct ti_lpsc_module *module);

/**
 * \brief Wait for an LPSC module transition to complete
 *
 * Polls the hardware status registers until the module transition completes.
 * This function blocks until the module reaches its target state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * Pointer to the LPSC module to wait for.
 */
void lpsc_module_wait(struct ti_device *psc_dev, struct ti_lpsc_module *module);

/**
 * \brief Set the reset isolation flag for a PSC module
 *
 * This directly modifies the hardware state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to modify.
 *
 * \param enable
 * True to enable reset isolation, false to disable.
 */
void lpsc_module_set_reset_iso(struct ti_device *psc_dev,
			       struct ti_lpsc_module *module, bool enable);

/**
 * \brief Get the reset isolation setting from a PSC module.
 *
 * This queries the true hardware state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to query.
 *
 * \return
 * True if reset isolation is enabled for this module, false if otherwise.
 */
bool lpsc_module_get_reset_iso(struct ti_device *psc_dev,
			       struct ti_lpsc_module *module);

/**
 * \brief Set/clear the local reset state of a PSC module
 *
 * The function of the local reset is module specific and only available on
 * certain modules. The most common use is to hold processors (such as the ICSS
 * or DSP) in reset.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to modify.
 *
 * \param enable
 * True to enable local reset, false to release local reset.
 */
void lpsc_module_set_local_reset(struct ti_device *psc_dev,
				 struct ti_lpsc_module *module, bool enable);

/**
 * \brief Set/clear the module reset state of a PSC module
 *
 * The function of the module reset is module specific and only available on
 * certain modules.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to modify.
 *
 * \param enable
 * True to enable module reset, false to release module reset.
 */
void lpsc_module_set_module_reset(struct ti_device *psc_dev,
				  struct ti_lpsc_module *module, bool enable);

/**
 * \brief Get the local reset state from a PSC module.
 *
 * This queries the true hardware state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to query.
 *
 * \return
 * True if local reset is asserted for this module, false if otherwise.
 */
bool lpsc_module_get_local_reset(struct ti_device *psc_dev,
				 struct ti_lpsc_module *module);

/**
 * \brief Get the module reset state from a PSC module.
 *
 * This queries the true hardware state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to query.
 *
 * \return
 * True if module reset is asserted for this module, false if otherwise.
 */
bool lpsc_module_get_module_reset(struct ti_device *psc_dev,
				  const struct ti_lpsc_module *module);

/**
 * \brief Get the module state from a PSC module.
 *
 * This queries the true hardware state.
 *
 * \param psc_dev
 * The PSC device that controls this module.
 *
 * \param module
 * The PSC module to query.
 *
 * \return
 * 0 for a disabled module, 1 for an enabled module, 2 for a module in transition.
 */
uint32_t lpsc_module_get_state(struct ti_device *psc_dev,
			       struct ti_lpsc_module *module);

/**
 * \brief Look up a PSC device by its index
 *
 * Searches the system for the PSC device with the specified index.
 * This function is used to obtain a reference to a PSC device for
 * subsequent operations.
 *
 * \param id
 * The psc_idx_t index of the PSC device to find.
 *
 * \return
 * Pointer to the ti_device structure for the PSC, or NULL if not found.
 */
struct ti_device *ti_psc_lookup(psc_idx_t id);

/**
 * \brief Look up a power domain within a PSC device
 *
 * Returns a pointer to the power domain structure for the specified
 * power domain index within the given PSC device.
 *
 * \param psc_dev
 * The PSC device containing the power domain.
 *
 * \param id
 * The pd_idx_t index of the power domain within the PSC.
 *
 * \return
 * Pointer to the ti_psc_pd structure for the power domain, or NULL if invalid.
 */
struct ti_psc_pd *psc_lookup_pd(struct ti_device *psc_dev, pd_idx_t id);

/**
 * \brief Look up an LPSC module within a PSC device
 *
 * Returns a pointer to the LPSC module structure for the specified
 * module index within the given PSC device.
 *
 * \param psc_dev
 * The PSC device containing the module.
 *
 * \param id
 * The lpsc_idx_t index of the module within the PSC.
 *
 * \return
 * Pointer to the ti_lpsc_module structure for the module, or NULL if invalid.
 */
struct ti_lpsc_module *psc_lookup_lpsc(struct ti_device *psc_dev, lpsc_idx_t id);

/**
 * \brief Drop all power-up references across all PSC modules and domains
 *
 * Clears the pwr_up_enabled flags for all power domains and modules across
 * all PSC devices in the system. This is typically called during system
 * shutdown or when transitioning to a low-power state to release all
 * power-up references and allow the system to power down completely.
 */
void psc_drop_pwr_up_ref(void);

/**
 * \struct ti_psc_data
 * \brief PSC device dynamic runtime data
 *
 * This structure contains the runtime state information for a PSC device,
 * including tracking of enabled power domains and modules. This data
 * changes during system operation as power domains and modules are
 * enabled and disabled.
 */
struct ti_psc_data {
	/**
	 * \brief Linked list pointer to next PSC device
	 *
	 * Used to maintain a linked list of all PSC devices in the system.
	 * NULL indicates the end of the list.
	 */
	struct ti_device *next;

	/**
	 * \brief Bit field of currently enabled power domains
	 *
	 * Each bit represents a power domain within this PSC. A set bit
	 * indicates the power domain is currently enabled. Bit position
	 * corresponds to the power domain index.
	 */
	uint32_t pds_enabled;

	/**
	 * \brief Bit field array of currently enabled modules
	 *
	 * Array of bit fields tracking which LPSC modules are currently enabled.
	 * Each bit represents an LPSC module, with bit position corresponding
	 * to the module index. The array supports up to 128 modules (4 * 32 bits).
	 */
	uint32_t mods_enabled[4];
};

/**
 * \struct ti_psc_drv_data
 * \brief PSC driver data structure
 *
 * This structure contains all the data needed for a PSC driver instance,
 * including both constant configuration data and dynamic runtime data for
 * power domains and LPSC modules. Each PSC device in the system has an
 * associated ti_psc_drv_data structure that defines its complete state
 * and configuration.
 */
struct ti_psc_drv_data {
	/**
	 * \brief Base driver data structure
	 *
	 * Contains common driver data fields required by the device management
	 * framework. This must be the first member to allow casting between
	 * ti_drv_data and ti_psc_drv_data pointers.
	 */
	struct ti_drv_data drv_data;

	/**
	 * \brief Pointer to dynamic runtime data
	 *
	 * Points to the ti_psc_data structure containing runtime state
	 * information for this PSC device, including enabled power domains
	 * and modules.
	 */
	struct ti_psc_data *data;

	/**
	 * \brief Pointer to constant power domain data table
	 *
	 * Array of ti_psc_pd_data structures containing the constant
	 * configuration for each power domain in this PSC. The array
	 * has pd_count elements indexed by pd_idx_t.
	 */
	const struct ti_psc_pd_data *pd_data;

	/**
	 * \brief Pointer to dynamic power domain data table
	 *
	 * Array of ti_psc_pd structures containing the runtime state
	 * for each power domain in this PSC. The array has pd_count
	 * elements indexed by pd_idx_t.
	 */
	struct ti_psc_pd *powerdomains;

	/**
	 * \brief Pointer to constant LPSC module data table
	 *
	 * Array of ti_lpsc_module_data structures containing the constant
	 * configuration for each LPSC module in this PSC. The array has
	 * module_count elements indexed by lpsc_idx_t.
	 */
	const struct ti_lpsc_module_data *mod_data;

	/**
	 * \brief Pointer to dynamic LPSC module data table
	 *
	 * Array of ti_lpsc_module structures containing the runtime state
	 * for each LPSC module in this PSC. The array has module_count
	 * elements indexed by lpsc_idx_t.
	 */
	struct ti_lpsc_module *modules;

	/**
	 * \brief Total number of power domains in this PSC
	 *
	 * Defines the size of the pd_data and powerdomains arrays. Valid
	 * power domain indices range from 0 to (pd_count - 1).
	 */
	pd_idx_t pd_count;

	/**
	 * \brief Total number of LPSC modules in this PSC
	 *
	 * Defines the size of the mod_data and modules arrays. Valid
	 * LPSC module indices range from 0 to (module_count - 1).
	 */
	lpsc_idx_t module_count;

	/**
	 * \brief Index of this PSC in the system
	 *
	 * Unique identifier for this PSC device within the system. Used
	 * for PSC lookup operations and cross-PSC dependencies.
	 */
	psc_idx_t psc_idx;

	/**
	 * \brief Number of supported sleep/power states
	 *
	 * The total number of sleep modes or low-power states that this
	 * PSC supports. Used for power management and system suspend/resume
	 * operations.
	 */
	uint8_t sleep_mode_count;
};

static inline const struct ti_psc_drv_data *to_psc_drv_data(const struct ti_drv_data *data)
{
	return container_of(data, const struct ti_psc_drv_data, drv_data);
}

#endif

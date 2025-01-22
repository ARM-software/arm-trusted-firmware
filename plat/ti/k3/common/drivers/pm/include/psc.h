/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * FIXME: Perhaps remove off state, on state bumps use count,
 * this would maintain dependencies as off could not be forced
 */

#ifndef PSC_H
#define PSC_H

#include <device.h>
#include <lib/container_of.h>
#include <pm_types.h>
#include <device_limits.h>

#define PSC_DEV_NONE		7U
#define PSC_DEV_MULTIPLE	6U
#if defined(CONFIG_PSC_PD_MAX_COUNT_64)
#define PSC_PD_NONE		63U
#else
#define PSC_PD_NONE		31U
#endif
#define PSC_LPSC_NONE		55U

#define PSC_PD_EXISTS		BIT(0)
#define PSC_PD_ALWAYSON		BIT(1)
#define PSC_PD_DEPENDS		BIT(2)

struct psc_pd_data {
#if defined(CONFIG_PSC_PD_MAX_COUNT_64)
	uint8_t	flags;
	uint8_t	depends;	/* pd_idx_t */
#else
	uint8_t	flags : 3;
	uint8_t	depends : 5;	/* pd_idx_t */
#endif
	/**
	 * Some domains need a clock running in order to transition. This
	 * is the id of that clock. If no such clock is needed, set to
	 * an invalid clock id (0). We currently support only 3 clocks,
	 * this causes the structure size to be 4 bytes for an 8 bit
	 * clk_idx_t and 8 bytes for a 16 bit clk_idx_t.
	 */
	clk_idx_t clock_dep[3];
};

struct psc_pd {
	uint8_t	use_count : 7;
	bool	pwr_up_enabled;
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

struct lpsc_module_data {
	/*
	 * Which devices are members of which PSC module is duplicated in the
	 * list of devices. However, it is duplicated here so that the entire
	 * list of devices does not need to be looped through to find that
	 * information. It could be generated dynamically but is stored in this
	 * way to save memory.
	 */
	union {
		/*
		 * If there are 4 or less devices, they can be stored here,
		 * terminated by the 4th device or DEV_ID_NONE
		 */
		dev_idx_t	dev_array[sizeof(void *) / sizeof(dev_idx_t)];
		/*
		 * More than 4 devices must be stored in a separate list,
		 * terminated by DEV_ID_NONE. The LPSC_DEVICE_LIST flag must be
		 * set.
		 */
		const dev_idx_t *dev_list;
	} lpsc_dev;
	/**
	 * Some modules need a clock running in order to transition. This
	 * is the id of that clock. If no such clock is needed, set to
	 * an invalid clock id (0). We currently support only one clock,
	 * this causes the structure size to be 8 bytes in the case of an
	 * 8 bit clk_idx_t and 12 bytes for a 16 bit clk_idx_t.
	 */
	clk_idx_t	clock_dep[1];
	uint8_t		flags;
	lpsc_idx_t	depends;
#if defined(CONFIG_PSC_PD_MAX_COUNT_64)
	uint8_t		depends_psc_idx;	/* psc_idx_t */
	uint8_t		powerdomain;		/* pd_idx_t */
#else
	uint8_t		depends_psc_idx : 3;	/* psc_idx_t */
	uint8_t		powerdomain : 5;	/* pd_idx_t */
#endif
};

struct lpsc_module {
	/** Incremented after module reset */
	uint32_t	loss_count;
	/** Non-zero if module should be active (clocks running) */
	uint8_t	use_count;
	/** Non-zero if module should be powered-up */
	uint8_t	ret_count;
	/** Current programmed state (MDSTAT_STATE_[...]) */
	uint8_t	sw_state : 2;
	/**
	 * True if the module is forced on due to a module reset. In this
	 * case sw_state indicates SWRSTDISABLE but this module holds
	 * a reference count to it's powerdomain.
	 */
	bool	sw_mrst_ret;
	uint8_t	pwr_up_enabled : 1;
	uint8_t	pwr_up_ret : 1;
	/** True if host has requested a module reset */
	uint8_t	mrst_active : 1;
};

/** Declare a default PSC powerdomain entry */
#define PSC_PD(id)[(id)] = { .flags = PSC_PD_EXISTS }

/** Declare an always on PSC powerdomain entry */
#define PSC_PD_AON(id)[(id)] = { .flags = PSC_PD_ALWAYSON | PSC_PD_EXISTS }

/** Declare a PSC powerdomain with a dependency */
#define PSC_PD_DEP(id, dep)[(id)] = { .depends = (dep),	\
		.flags	= PSC_PD_EXISTS |			\
		PSC_PD_DEPENDS }

pd_idx_t psc_pd_idx(struct device *psc_dev, struct psc_pd *pd);
void psc_pd_wait(struct device *psc_dev, struct psc_pd *pd);
uint32_t psc_pd_get_state(struct device *psc_dev, struct psc_pd *pd);
void psc_pd_get(struct device *psc_dev, struct psc_pd *pd);
void psc_pd_put(struct device *psc_dev, struct psc_pd *pd);

lpsc_idx_t lpsc_module_idx(struct device *psc_dev, struct lpsc_module *module);
void lpsc_module_get(struct device *psc_dev, struct lpsc_module *module);
void lpsc_module_put(struct device *psc_dev, struct lpsc_module *module);
void lpsc_module_ret_get(struct device *psc_dev, struct lpsc_module *module);
void lpsc_module_ret_put(struct device *psc_dev, struct lpsc_module *module);
void lpsc_module_wait(struct device *psc_dev, struct lpsc_module *module);

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
void lpsc_module_set_reset_iso(struct device *psc_dev, struct lpsc_module *module, bool enable);

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
bool lpsc_module_get_reset_iso(struct device *psc_dev, struct lpsc_module *module);

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
void lpsc_module_set_local_reset(struct device *psc_dev, struct lpsc_module *module, bool enable);

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
void lpsc_module_set_module_reset(struct device *psc_dev, struct lpsc_module *module, bool enable);

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
bool lpsc_module_get_local_reset(struct device *psc_dev, struct lpsc_module *module);

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
bool lpsc_module_get_module_reset(struct device *psc_dev, struct lpsc_module *module);

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
uint32_t lpsc_module_get_state(struct device *psc_dev, struct lpsc_module *module);

struct device *psc_lookup(psc_idx_t id);
struct psc_pd *psc_lookup_pd(struct device *psc_dev, pd_idx_t id);
struct lpsc_module *psc_lookup_lpsc(struct device *psc_dev, lpsc_idx_t id);

void psc_drop_pwr_up_ref(void);

extern const struct drv psc_drv;

struct soc_device_data;
extern const struct soc_device_data *const
soc_psc_multiple_domains[SOC_MULTIPLE_DOMAINS_RANGE_ID_MAX];

struct psc_data {
	/** Linked list of PSC devices */
	struct device	*next;

	/** Bit field of currently enabled powerdomains */
	uint32_t		pds_enabled;

	/** Bit field of currently enabled modules */
	uint32_t		mods_enabled[4];
};

struct psc_sleep_mode {
	/**
	 * Bit field of powerdomains that must be disabled to reach this
	 * sleep mode.
	 */
	uint32_t	pds_disabled;

	/**
	 * Bit field of modules that must be disabled to reach this sleep
	 * mode.
	 */
	uint32_t	modules_disabled[4];
};

struct psc_drv_data {
	struct drv_data			drv_data;

	/** Dynamic data */
	struct psc_data			*data;

	/** The table of const PSC powerdomain data */
	const struct psc_pd_data	*pd_data;

	/** The table of dynamic PSC powerdomain data */
	struct psc_pd			*powerdomains;

	/** The table of const PSC module data */
	const struct lpsc_module_data	*mod_data;

	/** The table of dynamic PSC module data */
	struct lpsc_module		*modules;

	/** Table of possible sleep modes */
	const struct psc_sleep_mode	*sleep_modes;

	/** The total number of PSC powerdomains */
	pd_idx_t			pd_count;

	/** The total number of PSC modules */
	lpsc_idx_t			module_count;

	/** The index of this PSC */
	psc_idx_t			psc_idx;

	/** The total number of sleep states */
	uint8_t				sleep_mode_count;
};

static inline const struct psc_drv_data *to_psc_drv_data(
	const struct drv_data *data)
{
	return container_of(data, const struct psc_drv_data, drv_data);
}

#endif

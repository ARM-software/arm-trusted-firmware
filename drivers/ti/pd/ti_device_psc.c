/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device to PSC Module Mapping
 *
 * This module bridges devices and PSC LPSC modules, providing state queries,
 * reset control, and context loss tracking for device-to-PSC module associations.
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <ti_device.h>
#include <ti_device_pm.h>
#include <ti_psc.h>

/* Sentinel value indicating no LPSC module association */
#define PSC_LPSC_NONE		55U

/**
 * ti_soc_device_get_state_internal() - Get the PSC state for a single SoC device domain.
 * @dev: SoC device data identifying the PSC index and LPSC module.
 *
 * Return: The LPSC module state, or 0 if PSC or module is not found.
 */
static uint32_t ti_soc_device_get_state_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return 0U;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return 0U;
	}

	return ti_lpsc_module_get_state(psc_dev, module);
}

uint32_t ti_soc_device_get_state(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t ret = 2U;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			uint32_t this_ret;

			this_ret = ti_soc_device_get_state_internal(&domains[i]);
			if (i == 0U) {
				ret = this_ret;
			} else if (ret != this_ret) {
				/* Mixed state of our domains, label as transition */
				ret = 2U;
			} else {
				/* Do Nothing */
			}
		}
	} else {
		ret = ti_soc_device_get_state_internal(&data->soc);
	}

	return ret;
}

/**
 * ti_soc_device_set_reset_iso_internal() - Set reset isolation for a single SoC device domain.
 * @dev: SoC device data identifying the PSC index and LPSC module.
 * @enable: True to enable reset isolation, false to disable.
 */
static void ti_soc_device_set_reset_iso_internal(const struct ti_soc_device_data *dev, bool enable)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return;
	}

	ti_lpsc_module_set_reset_iso(psc_dev, module, enable);
}

void ti_soc_device_set_reset_iso(struct ti_device *dev, bool enable)
{
	const struct ti_dev_data *data;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		/* Reset ISO not supported for devices with multiple domains */
	} else {
		ti_soc_device_set_reset_iso_internal(&data->soc, enable);
	}
}

/**
 * ti_soc_device_get_context_loss_count_internal() - Get context loss count for a device domain.
 * @dev: SoC device data identifying the PSC index and LPSC module.
 *
 * Return: The context loss count for the LPSC module, or 0 if not found.
 */
static uint32_t ti_soc_device_get_context_loss_count_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return 0U;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return 0U;
	}

	return module->loss_count;
}

uint32_t ti_soc_device_get_context_loss_count(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t ret = 0U;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			ret += ti_soc_device_get_context_loss_count_internal(&domains[i]);
		}
	} else {
		ret = ti_soc_device_get_context_loss_count_internal(&data->soc);
	}

	return ret;
}

/**
 * ti_soc_device_enable_internal() - Enable the PSC LPSC module for a single SoC device domain.
 * @dev: SoC device data identifying the PSC index and LPSC module.
 */
static void ti_soc_device_enable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return;
	}

	ti_lpsc_module_get(psc_dev, module);
}

void ti_soc_device_enable(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	VERBOSE("DEVICE_ON: dev_id=%u\n", ti_device_id(dev));

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			ti_soc_device_enable_internal(&domains[i]);
		}
	} else {
		ti_soc_device_enable_internal(&data->soc);
	}
}

/**
 * ti_soc_device_disable_internal() - Disable the PSC LPSC module for a single SoC device domain.
 * @dev: SoC device data identifying the PSC index and LPSC module.
 */
static void ti_soc_device_disable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return;
	}

	ti_lpsc_module_put(psc_dev, module);
}

/**
 * ti_soc_device_disable_internal_flags_iterate() - Clear all PSC initialization flags.
 * @psc_dev: The PSC device whose flags are to be cleared.
 * @module: Entry-point LPSC module (flags cleared here first, then entire PSC reset).
 *
 * Clears initialization flags for every power domain and LPSC module in the
 * PSC referenced by psc_dev. Despite accepting a specific module as the entry
 * point, the implementation resets the entire PSC state intentionally: all
 * LPSCs must be disabled before a power domain can power off. Recurses into
 * any dependency PSC.
 */
static void ti_soc_device_disable_internal_flags_iterate(struct ti_device *psc_dev,
							 struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(psc_dev));
	const struct ti_lpsc_module_data *data;
	const struct ti_psc_drv_data *depends_psc;
	struct ti_device *depends_dev;
	struct ti_psc_pd *pd;
	struct ti_lpsc_module *temp;
	uint32_t idx;
	struct ti_lpsc_module *module_p = module;

	if (module_p != NULL) {
		module_p->use_count = 0U;
		module_p->ret_count = 0U;
		module_p->pwr_up_enabled = 0U;
		module_p->pwr_up_ret = 0U;
		module_p->sw_state = 0U;
		module_p->loss_count = 0U;
		module_p->mrst_active = 0U;
		module_p->sw_mrst_ret = false;
	}

	if (psc == NULL) {
		return;
	}

	for (idx = 0U; idx < psc->pd_count; idx++) {
		pd = &psc->powerdomains[idx];
		pd->use_count = 0U;
		pd->pwr_up_enabled = false;
	}
	for (idx = 0U; idx < psc->module_count; idx++) {
		temp = &psc->modules[idx];
		temp->use_count = 0U;
		temp->ret_count = 0U;
		temp->pwr_up_enabled = 0U;
		temp->pwr_up_ret = 0U;
		temp->sw_state = 0U;
		temp->sw_mrst_ret = false;
		temp->loss_count = 0U;
		temp->mrst_active = 0U;
	}

	psc->data->pds_enabled = 0U;
	idx = ti_lpsc_module_idx(psc_dev, module_p);
	data = &psc->mod_data[idx];

	if ((data->flags & TI_LPSC_DEPENDS) != 0U) {
		depends_dev = ti_psc_lookup((ti_psc_idx_t) data->depends_psc_idx);
		if (depends_dev != NULL) {
			depends_psc = ti_to_psc_drv_data(ti_get_drv_data(depends_dev));
			if ((depends_psc != NULL) && (module_p != NULL)) {
				module_p = &depends_psc->modules[(ti_lpsc_idx_t) data->depends];
				ti_soc_device_disable_internal_flags_iterate(depends_dev, module_p);
			}
		}
	}
}

/**
 * ti_soc_device_disable_internal_flags() - Clear PSC initialization flags for a device.
 * @dev: SoC device data identifying the PSC and LPSC module.
 *
 * Clears initialization flags for the PSC module associated with the given device.
 */
static void ti_soc_device_disable_internal_flags(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	ti_soc_device_disable_internal_flags_iterate(psc_dev, module);
}

void ti_soc_device_disable(struct ti_device *dev, bool domain_reset)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	VERBOSE("DEVICE_OFF: dev_id=%u\n", ti_device_id(dev));

	if (domain_reset == false) {
		if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
			domains = soc_psc_multiple_domains[data->soc.mod];
			for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
				ti_soc_device_disable_internal(&domains[i]);
			}
		} else {
			ti_soc_device_disable_internal(&data->soc);
		}
	}
}

void ti_soc_device_clear_flags(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	VERBOSE("CLEAR_FLAGS: dev_id=%u\n", ti_device_id(dev));

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			ti_soc_device_disable_internal_flags(&domains[i]);
		}
	} else {
		ti_soc_device_disable_internal_flags(&data->soc);
	}
}

static void ti_soc_device_ret_enable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_psc_pd *pd;
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return;
	}

	pd = ti_psc_lookup_pd(psc_dev, (ti_pd_idx_t) dev->pd);
	if (pd != NULL) {
		ti_psc_pd_get(psc_dev, pd);
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module != NULL) {
		ti_lpsc_module_ret_get(psc_dev, module);
	}
}

void ti_soc_device_ret_enable(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			ti_soc_device_ret_enable_internal(&domains[i]);
		}
	} else {
		ti_soc_device_ret_enable_internal(&data->soc);
	}
}

static void ti_soc_device_ret_disable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((ti_psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;
	struct ti_psc_pd *pd;

	if (psc_dev == NULL) {
		return;
	}

	module = ti_psc_lookup_lpsc(psc_dev, dev->mod);
	if (module != NULL) {
		ti_lpsc_module_ret_put(psc_dev, module);
	}

	pd = ti_psc_lookup_pd(psc_dev, (ti_pd_idx_t) dev->pd);
	if (pd != NULL) {
		ti_psc_pd_put(psc_dev, pd);
	}
}

void ti_soc_device_ret_disable(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *data;
	uint32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	if (data->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			ti_soc_device_ret_disable_internal(&domains[i]);
		}
	} else {
		ti_soc_device_ret_disable_internal(&data->soc);
	}
}

void ti_soc_device_init_complete(void)
{
	ti_psc_drop_pwr_up_ref();
}

static int32_t ti_soc_device_verify_mapping(const struct ti_psc_drv_data *psc,
					    uint32_t dev_id, const struct ti_soc_device_data *dev)
{
	const struct ti_lpsc_module_data *mdata;
	uint32_t i;

	/*
	 * Make sure the redundant const data stored in the PSC data is
	 * correct. This redundant data would ideally be checked or even
	 * generated at compile time.
	 */
	if (dev->mod != PSC_LPSC_NONE) {
		mdata = &psc->mod_data[dev->mod];

		if ((mdata->flags & TI_LPSC_DEVICES_LIST) != 0U) {
			for (i = 0U; mdata->lpsc_dev.dev_list[i] != dev_id; i++) {
				if (mdata->lpsc_dev.dev_list[i] == TI_DEV_ID_NONE) {
					VERBOSE("ACTION FAIL: INVALID_PSC_DATA\n");
					return -EINVAL;
				}
			}
		} else {
			for (i = 0U; mdata->lpsc_dev.dev_array[i] != dev_id; i++) {
				if ((i >= (ARRAY_SIZE(mdata->lpsc_dev.dev_array) - 1UL)) ||
				    (mdata->lpsc_dev.dev_array[i] == TI_DEV_ID_NONE)) {
					VERBOSE("ACTION FAIL: INVALID_PSC_DATA\n");
					return -EINVAL;
				}
			}
		}
	}

	return 0;
}

static int32_t ti_soc_device_init_internal(struct ti_device *dev)
{
	const struct ti_soc_device_data *domains;
	const struct ti_dev_data *devdata;
	const struct ti_drv_data *drvdata;
	struct ti_device *psc_dev = NULL;
	const struct ti_psc_drv_data *psc = NULL;
	uint32_t dev_id;
	int32_t ret;
	uint32_t i;

	dev_id = ti_device_id(dev);
	devdata = ti_get_dev_data(dev);

	/* Check if this PSC manages its own power domain */
	if ((devdata->flags & TI_DEVD_FLAG_DRV_DATA) != 0U) {
		drvdata = ti_to_drv_data(devdata);
		if (drvdata->drv == &psc_drv) {
			psc = ti_to_psc_drv_data(drvdata);
			if (psc->psc_idx == devdata->soc.psc_idx) {
				psc_dev = dev;
			} else {
				psc = NULL;
			}
		}
	}

	if (psc_dev != NULL) {
		/* We are our own PSC */
		return ti_soc_device_verify_mapping(psc, dev_id, &devdata->soc);
	}

	if (devdata->soc.psc_idx == TI_PSC_DEV_MULTIPLE) {
		/* Find all the PSCs this device needs */
		domains = soc_psc_multiple_domains[devdata->soc.mod];
		for (i = 0U; domains[i].psc_idx != TI_PSC_DEV_NONE; i++) {
			psc_dev = ti_psc_lookup((ti_psc_idx_t) domains[i].psc_idx);
			if (psc_dev == NULL) {
				return -EAGAIN;
			}

			psc = ti_to_psc_drv_data(ti_get_drv_data(psc_dev));
			ret = ti_soc_device_verify_mapping(psc, dev_id, &domains[i]);
			if (ret != 0) {
				return ret;
			}
		}

		return 0;
	}

	/* We just need the one PSC */
	psc_dev = ti_psc_lookup((ti_psc_idx_t) devdata->soc.psc_idx);
	if (psc_dev == NULL) {
		/*
		 * Don't try to bring this dev up before calling init
		 * on the device's PSC.
		 */
		return -EAGAIN;
	}

	psc = ti_to_psc_drv_data(ti_get_drv_data(psc_dev));
	return ti_soc_device_verify_mapping(psc, dev_id, &devdata->soc);
}

/* Defer all other device initialization until PSC initializes */
int32_t ti_soc_device_init(struct ti_device *dev)
{
	const struct ti_dev_data *devdata;

	assert(dev != NULL);

	devdata = ti_get_dev_data(dev);

	if (devdata->soc.psc_idx != TI_PSC_DEV_NONE) {
		return ti_soc_device_init_internal(dev);
	}

	return 0;
}

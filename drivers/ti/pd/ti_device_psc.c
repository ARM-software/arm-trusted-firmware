/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device to PSC Module Mapping
 *
 * This module bridges devices and PSC LPSC modules, providing state queries,
 * reset control, and context loss tracking for device-to-PSC module associations.
 */

#include <common/debug.h>
#include <errno.h>

#include <ti_device.h>
#include <ti_device_pm.h>
#include <ti_psc.h>

#define PSC_LPSC_NONE		55U

static uint32_t soc_device_get_state_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);
	uint32_t ret = 1;

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ret = lpsc_module_get_state(psc_dev, module);
		}
	}

	return ret;
}

uint32_t ti_soc_device_get_state(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);
	uint32_t ret = 2;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			uint32_t this_ret;

			this_ret = soc_device_get_state_internal(&domains[i]);
			if (i == 0U) {
				ret = this_ret;
			} else if (ret != this_ret) {
				/* Mixed state of our domains, label as transition */
				ret = 2;
			} else {
				/* Do Nothing */
			}
		}
	} else {
		ret = soc_device_get_state_internal(&data->soc);
	}

	return ret;
}

static void soc_device_set_reset_iso_internal(const struct ti_soc_device_data *dev, bool enable)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_set_reset_iso(psc_dev, module, enable);
		}
	}
}

void ti_soc_device_set_reset_iso(struct ti_device *dev, bool enable)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Reset ISO not supported for devices with multiple domains */
	} else {
		soc_device_set_reset_iso_internal(&data->soc, enable);
	}
}

static uint32_t soc_device_get_context_loss_count_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);
	struct ti_lpsc_module *module;

	if (psc_dev == NULL) {
		return 0U;
	}

	module = psc_lookup_lpsc(psc_dev, dev->mod);
	if (module == NULL) {
		return 0U;
	}

	return module->loss_count;
}

uint32_t ti_soc_device_get_context_loss_count(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);
	uint32_t ret = 0;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			ret += soc_device_get_context_loss_count_internal(&domains[i]);
		}
	} else {
		ret = soc_device_get_context_loss_count_internal(&data->soc);
	}

	return ret;
}

static void soc_device_enable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ti_lpsc_module_get(psc_dev, module);
		}
	}
}

void ti_soc_device_enable(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	VERBOSE("DEVICE_ON: dev_id=%d\n", device_id(dev));

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_enable_internal(&domains[i]);
		}
	} else {
		soc_device_enable_internal(&data->soc);
	}
}

static void soc_device_disable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ti_lpsc_module_put(psc_dev, module);
		}
	}
}

/**
 * @brief This function clear all flags associated with the module and its dependent modules
 *
 * @param psc_dev PSC associated with module
 * @param module which the flag needs to be cleared
 */
static void soc_device_disable_internal_flags_iterate(struct ti_device *psc_dev,
						      struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = to_psc_drv_data(get_drv_data(psc_dev));
	const struct ti_lpsc_module_data *data;
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
		for (idx = 0U; idx < ARRAY_SIZE(psc->data->mods_enabled); idx++) {
			psc->data->mods_enabled[idx] = 0U;
		}
	}

	if (psc == NULL) {
		return;
	}

	for (idx = 0U; idx < psc->pd_count; idx++) {
		struct ti_psc_pd *pd = &psc->powerdomains[idx];

		pd->use_count = 0U;
		pd->pwr_up_enabled = false;
	}
	for (idx = 0; idx < psc->module_count; idx++) {
		struct ti_lpsc_module *temp = &psc->modules[idx];

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

	if ((data->flags & LPSC_DEPENDS) != 0UL) {
		const struct ti_psc_drv_data *depends_psc;
		struct ti_device *depends_dev;

		depends_dev = ti_psc_lookup((psc_idx_t) data->depends_psc_idx);
		if (depends_dev != NULL) {
			depends_psc = to_psc_drv_data(get_drv_data(depends_dev));
			if (depends_psc != NULL && module_p != NULL) {
				module_p = &depends_psc->modules[(lpsc_idx_t) data->depends];
				soc_device_disable_internal_flags_iterate(depends_dev, module_p);
			}
		}
	}
}

/**
 * @brief Clear all initialization flags associated with a device
 *
 * @param dev The device that flags needs to be cleared
 */
static void soc_device_disable_internal_flags(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);

		soc_device_disable_internal_flags_iterate(psc_dev, module);
	}
}

void ti_soc_device_disable(struct ti_device *dev, bool domain_reset)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	VERBOSE("DEVICE_OFF: dev_id=%d\n", device_id(dev));

	if (domain_reset) {
		/* No action */
	} else if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_disable_internal(&domains[i]);
		}
	} else {
		soc_device_disable_internal(&data->soc);
	}
}

void ti_soc_device_clear_flags(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	VERBOSE("DEVICE_OFF: dev_id=%d\n", device_id(dev));

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_disable_internal_flags(&domains[i]);
		}
	} else {
		soc_device_disable_internal_flags(&data->soc);
	}
}

static void soc_device_ret_enable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_psc_pd *pd;
		struct ti_lpsc_module *module;

		pd = psc_lookup_pd(psc_dev, (pd_idx_t) dev->pd);
		if (pd != NULL) {
			ti_psc_pd_get(psc_dev, pd);
		}

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ti_lpsc_module_ret_get(psc_dev, module);
		}
	}
}

void ti_soc_device_ret_enable(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_ret_enable_internal(&domains[i]);
		}
	} else {
		soc_device_ret_enable_internal(&data->soc);
	}
}

static void soc_device_ret_disable_internal(const struct ti_soc_device_data *dev)
{
	struct ti_device *psc_dev = ti_psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct ti_lpsc_module *module;
		struct ti_psc_pd *pd;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ti_lpsc_module_ret_put(psc_dev, module);
		}

		pd = psc_lookup_pd(psc_dev, (pd_idx_t) dev->pd);
		if (pd != NULL) {
			ti_psc_pd_put(psc_dev, pd);
		}
	}
}

void ti_soc_device_ret_disable(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_ret_disable_internal(&domains[i]);
		}
	} else {
		soc_device_ret_disable_internal(&data->soc);
	}
}

void ti_soc_device_init_complete(void)
{
	psc_drop_pwr_up_ref();
}

static int32_t soc_device_verify_mapping(const struct ti_psc_drv_data *psc,
					 uint32_t dev_id, const struct ti_soc_device_data *dev)
{
	/*
	 * Make sure the redundant const data stored in the PSC data is
	 * correct. This redundant data would idealy be checked or even
	 * generated at compile time.
	 */
	if (dev->mod != PSC_LPSC_NONE) {
		const struct ti_lpsc_module_data *mdata = &psc->mod_data[dev->mod];
		uint32_t i;

		if ((mdata->flags & LPSC_DEVICES_LIST) != 0UL) {
			for (i = 0U; mdata->lpsc_dev.dev_list[i] != dev_id; i++) {
				if (mdata->lpsc_dev.dev_list[i] == DEV_ID_NONE) {
					VERBOSE("ACTION FAIL: INVALID_PSC_DATA\n");
					return -EINVAL;
				}
			}
		} else {
			for (i = 0U; mdata->lpsc_dev.dev_array[i] != dev_id; i++) {
				if ((i >= (ARRAY_SIZE(mdata->lpsc_dev.dev_array) - 1UL)) ||
				    (mdata->lpsc_dev.dev_array[i] == DEV_ID_NONE)) {
					VERBOSE("ACTION FAIL: INVALID_PSC_DATA\n");
					return -EINVAL;
				}
			}
		}
	}

	return 0;
}

static int32_t soc_device_init_internal(struct ti_device *dev)
{
	const struct ti_dev_data *devdata;
	struct ti_device *psc_dev = NULL;
	const struct ti_psc_drv_data *psc = NULL;
	uint32_t dev_id;
	int32_t ret;

	dev_id = device_id(dev);
	devdata = get_dev_data(dev);

	/* Check if this PSC manages it's own power domain */
	if ((devdata->flags & DEVD_FLAG_DRV_DATA) != 0U) {
		const struct ti_drv_data *drvdata;

		drvdata = to_drv_data(devdata);
		if (drvdata->drv == &psc_drv) {
			psc = to_psc_drv_data(drvdata);
			if (psc->psc_idx == devdata->soc.psc_idx) {
				psc_dev = dev;
			} else {
				psc = NULL;
			}
		}
	}

	if (psc_dev != NULL) {
		/* We are our own PSC */
		return soc_device_verify_mapping(psc, dev_id, &devdata->soc);
	}

	if (devdata->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Find all the PSCs this device needs */
		uint32_t i;
		const struct ti_soc_device_data *domains;

		domains = soc_psc_multiple_domains[devdata->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			psc_dev = ti_psc_lookup((psc_idx_t) domains[i].psc_idx);
			if (!psc_dev) {
				return -EAGAIN;
			}

			psc = to_psc_drv_data(get_drv_data(psc_dev));
			ret = soc_device_verify_mapping(psc, dev_id, &domains[i]);
			if (ret != 0) {
				return ret;
			}
		}

		return 0;
	}

	/* We just need the one PSC */
	psc_dev = ti_psc_lookup((psc_idx_t) devdata->soc.psc_idx);
	if (!psc_dev) {
		/*
		 * Don't try to bring this dev up before calling init
		 * on the device's PSC.
		 */
		return -EAGAIN;
	}

	psc = to_psc_drv_data(get_drv_data(psc_dev));
	return soc_device_verify_mapping(psc, dev_id, &devdata->soc);
}

/* Defer all other device initialization until PSC initializes */
int32_t ti_soc_device_init(struct ti_device *dev)
{
	const struct ti_dev_data *devdata;

	devdata = get_dev_data(dev);

	if (devdata->soc.psc_idx != PSC_DEV_NONE) {
		return soc_device_init_internal(dev);
	}

	return 0;
}

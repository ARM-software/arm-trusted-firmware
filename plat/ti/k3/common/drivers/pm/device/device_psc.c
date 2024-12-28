/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psc.h>
#include <lib/container_of.h>
#include <device.h>
#include <types/errno.h>
#include <types/array_size.h>
#include <lib/trace.h>
#include <device_pm.h>
#include <common/debug.h>

static uint32_t soc_device_get_state_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);
	uint32_t ret = 1;

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ret = lpsc_module_get_state(psc_dev, module);
		}
	}

	return ret;
}

uint32_t soc_device_get_state(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);
	uint32_t ret = 2;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

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

static void soc_device_set_reset_iso_internal(const struct soc_device_data *dev, bool enable)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_set_reset_iso(psc_dev, module, enable);
		}
	}
}

void soc_device_set_reset_iso(struct device *dev, bool enable)
{
	const struct dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Reset ISO not supported for devices with multiple domains */
	} else {
		soc_device_set_reset_iso_internal(&data->soc, enable);
	}
}

static bool soc_device_get_reset_iso_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);
	bool ret = false;

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ret = lpsc_module_get_reset_iso(psc_dev, module);
		}
	}

	return ret;
}

bool soc_device_get_reset_iso(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);
	bool ret = false;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Reset ISO not supported for devices with multiple domains */
	} else {
		ret = soc_device_get_reset_iso_internal(&data->soc);
	}

	return ret;
}

static void soc_device_set_resets_internal(const struct soc_device_data *dev, uint32_t resets)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			bool local_reset = false;
			bool module_reset = false;

			if ((resets & BIT(0)) != 0U) {
				local_reset = true;
			}
			if ((resets & BIT(1)) != 0U) {
				module_reset = true;
			}
			lpsc_module_set_local_reset(psc_dev, module, local_reset);
			lpsc_module_set_module_reset(psc_dev, module, module_reset);
		}
	}
}

void soc_device_set_resets(struct device *dev, uint32_t resets)
{
	const struct dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Resets not supported for devices with multiple domains */
	} else {
		soc_device_set_resets_internal(&data->soc, resets);
	}
}

static uint32_t soc_device_get_resets_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);
	uint32_t resets = 0U;

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			bool ret;

			ret = lpsc_module_get_local_reset(psc_dev, module);
			if (ret) {
				resets |= BIT(0);
			}
			ret = lpsc_module_get_module_reset(psc_dev, module);
			if (ret) {
				resets |= BIT(1);
			}
		}
	}

	return resets;
}

uint32_t soc_device_get_resets(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);
	uint32_t resets = 0;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Resets not supported for devices with multiple domains */
	} else {
		resets = soc_device_get_resets_internal(&data->soc);
	}

	return resets;
}

static uint32_t soc_device_get_context_loss_count_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);
	uint32_t ret = 0;

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			ret = module->loss_count;
		}
	}

	return ret;
}

uint32_t soc_device_get_context_loss_count(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);
	uint32_t ret = 0;

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			ret += soc_device_get_context_loss_count_internal(&domains[i]);
		}
	} else {
		ret = soc_device_get_context_loss_count_internal(&data->soc);
	}

	return ret;
}

static void soc_device_enable_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_get(psc_dev, module);
		}
	}
}

void soc_device_enable(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);

	pm_trace(TRACE_PM_ACTION_DEVICE_ON, device_id(dev));

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_enable_internal(&domains[i]);
		}
	} else {
		soc_device_enable_internal(&data->soc);
	}
}

static void soc_device_disable_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_put(psc_dev, module);
		}
	}
}

/**
 * @brief This function clear all flags associated with the module and its dependent modules
 *
 * @param psc_dev PSC associated with module
 * @param module which the flag needs to be cleared
 */
static void soc_device_disable_internal_flags_iterate(struct device *psc_dev,
						      struct lpsc_module *module)
{
	const struct psc_drv_data *psc = to_psc_drv_data(get_drv_data(psc_dev));
	const struct lpsc_module_data *data;
	uint32_t idx;
	struct lpsc_module *module_p = module;

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

	if (psc != NULL) {
		for (idx = 0U; idx < psc->pd_count; idx++) {
			struct psc_pd *pd = psc->powerdomains + idx;

			pd->use_count = 0U;
			pd->pwr_up_enabled = false;
		}
		for (idx = 0; idx < psc->module_count; idx++) {
			struct lpsc_module *temp = psc->modules + idx;

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
		idx = lpsc_module_idx(psc_dev, module_p);
		data = psc->mod_data + idx;

		if ((data->flags & LPSC_DEPENDS) != 0UL) {
			const struct psc_drv_data *depends_psc = psc;
			struct device *depends_dev = psc_dev;

			if ((depends_dev != NULL) || (depends_psc != NULL)) {
				depends_dev = psc_lookup((psc_idx_t) data->depends_psc_idx);
				depends_psc = to_psc_drv_data(get_drv_data(depends_dev));
			}

			if (depends_dev && module_p) {
				module_p = depends_psc->modules + (lpsc_idx_t) data->depends;
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
static void soc_device_disable_internal_flags(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;

		module = psc_lookup_lpsc(psc_dev, dev->mod);

		soc_device_disable_internal_flags_iterate(psc_dev, module);
	}
}

void soc_device_disable(struct device *dev, bool domain_reset)
{
	const struct dev_data *data = get_dev_data(dev);

	pm_trace(TRACE_PM_ACTION_DEVICE_OFF, device_id(dev));

	if (domain_reset) {
		/* No action */
	} else if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_disable_internal(&domains[i]);
		}
	} else {
		soc_device_disable_internal(&data->soc);
	}
}

void soc_device_clear_flags(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);

	pm_trace(TRACE_PM_ACTION_DEVICE_OFF, device_id(dev));

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_disable_internal_flags(&domains[i]);
		}
	} else {
		soc_device_disable_internal_flags(&data->soc);
	}
}

static void soc_device_ret_enable_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct psc_pd *pd;
		struct lpsc_module *module;

		pd = psc_lookup_pd(psc_dev, (pd_idx_t) dev->pd);
		if (pd != NULL) {
			psc_pd_get(psc_dev, pd);
		}

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_ret_get(psc_dev, module);
		}
	}
}

void soc_device_ret_enable(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_ret_enable_internal(&domains[i]);
		}
	} else {
		soc_device_ret_enable_internal(&data->soc);
	}
}

static void soc_device_ret_disable_internal(const struct soc_device_data *dev)
{
	struct device *psc_dev = psc_lookup((psc_idx_t) dev->psc_idx);

	if (psc_dev != NULL) {
		struct lpsc_module *module;
		struct psc_pd *pd;

		module = psc_lookup_lpsc(psc_dev, dev->mod);
		if (module != NULL) {
			lpsc_module_ret_put(psc_dev, module);
		}

		pd = psc_lookup_pd(psc_dev, (pd_idx_t) dev->pd);
		if (pd != NULL) {
			psc_pd_put(psc_dev, pd);
		}
	}
}

void soc_device_ret_disable(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);

	if (data->soc.psc_idx == PSC_DEV_MULTIPLE) {
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[data->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			soc_device_ret_disable_internal(&domains[i]);
		}
	} else {
		soc_device_ret_disable_internal(&data->soc);
	}
}

void soc_device_init_complete(void)
{
	psc_drop_pwr_up_ref();
}

static int32_t soc_device_verify_mapping(const struct psc_drv_data *psc,
					 uint32_t dev_id, const struct soc_device_data *dev)
{
	int32_t ret = 0;

	/*
	 * Make sure the redundant const data stored in the PSC data is
	 * correct. This redundant data would idealy be checked or even
	 * generated at compile time.
	 */
	if (dev->mod != PSC_LPSC_NONE) {
		const struct lpsc_module_data *mdata = psc->mod_data + dev->mod;
		uint32_t i;

		if ((mdata->flags & LPSC_DEVICES_LIST) != 0UL) {
			for (i = 0U; mdata->lpsc_dev.dev_list[i] != dev_id; i++) {
				if (mdata->lpsc_dev.dev_list[i] == DEV_ID_NONE) {
					ret = -EINVAL;
					break;
				}
			}
		} else {
			for (i = 0U; mdata->lpsc_dev.dev_array[i] != dev_id; i++) {
				if ((i >= (ARRAY_SIZE(mdata->lpsc_dev.dev_array) - 1UL)) ||
				    (mdata->lpsc_dev.dev_array[i] == DEV_ID_NONE)) {
					ret = -EINVAL;
					break;
				}
			}
		}
	}

	if (ret != 0L) {
		pm_trace(TRACE_PM_ACTION_INVALID_PSC_DATA | TRACE_PM_ACTION_FAIL, 0U);
	}

	return ret;
}

static int32_t soc_device_init_internal(struct device *dev)
{
	const struct dev_data *devdata;
	struct device *psc_dev = NULL;
	const struct psc_drv_data *psc = NULL;
	uint32_t dev_id;
	int32_t ret = 0;

	dev_id = device_id(dev);
	devdata = get_dev_data(dev);

	/* Check if this PSC manages it's own power domain */
	if ((devdata->flags & DEVD_FLAG_DRV_DATA) != 0U) {
		const struct drv_data *drvdata;

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

	if (psc != NULL) {
		/* We are our own PSC */
		ret = soc_device_verify_mapping(psc, dev_id, &devdata->soc);
	} else if (devdata->soc.psc_idx == PSC_DEV_MULTIPLE) {
		/* Find all the PSCs this device needs */
		uint32_t i;
		const struct soc_device_data *domains;

		domains = soc_psc_multiple_domains[devdata->soc.mod];
		for (i = 0; domains[i].psc_idx != PSC_DEV_NONE; i++) {
			psc_dev = psc_lookup((psc_idx_t) domains[i].psc_idx);
			if (ret != 0) {
				/* Do nothing */
			} else if (!psc_dev) {
				ret = -EDEFER;
			} else {
				psc = to_psc_drv_data(get_drv_data(psc_dev));
				ret = soc_device_verify_mapping(psc, dev_id, &domains[i]);
			}
		}
	} else {
		/* We just need the one PSC */
		psc_dev = psc_lookup((psc_idx_t) devdata->soc.psc_idx);
		if (!psc_dev) {
			/*
			 * Don't try to bring this dev up before calling init
			 * on the device's PSC.
			 */
			ret = -EDEFER;
		} else {
			psc = to_psc_drv_data(get_drv_data(psc_dev));
			ret = soc_device_verify_mapping(psc, dev_id, &devdata->soc);
		}
	}

	return ret;
}

/* Defer all other device initialization until PSC initializes */
int32_t soc_device_init(struct device *dev)
{
	const struct dev_data *devdata;
	int32_t ret = 0;

	devdata = get_dev_data(dev);

	if (devdata->soc.psc_idx != PSC_DEV_NONE) {
		ret = soc_device_init_internal(dev);
	}

	return ret;
}

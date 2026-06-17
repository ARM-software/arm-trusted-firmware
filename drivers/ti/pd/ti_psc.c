/*
 * Copyright (c) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Power/Sleep Controller (PSC) Driver
 *
 * This driver manages the PSC hardware including power domain state transitions,
 * LPSC module clock gating, and dependency management between modules and power domains.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <ti_clk.h>
#include <ti_device.h>
#include <ti_device_pm.h>
#include <ti_psc.h>

#define PSC_PTCMD		0x120U
#define PSC_PTCMD_H		0x124U
#define PSC_PTSTAT		0x128U
#define PSC_PTSTAT_H		0x12CU
#define PSC_PDSTAT_BASE		0x200U
#define PSC_PDCTL_BASE		0x300U
#define PSC_MDSTAT_BASE		0x800U
#define PSC_MDCTL_BASE		0xa00U

static inline uint32_t psc_reg(uint32_t base, uint32_t id)
{
	return base + (4U * id);
}

#define MDSTAT_STATE_MASK		0x3fU
#define MDSTAT_BUSY_MASK		0x30U
#define MDSTAT_STATE_SWRSTDISABLE	0x00U
#define MDSTAT_STATE_SYNCRST		0x01U
#define MDSTAT_STATE_DISABLE		0x02U
#define MDSTAT_STATE_ENABLE		0x03U

#define MDSTAT_MRSTDONE		BIT(11) /* Module reset done */
#define MDSTAT_LRSTDONE		BIT(9)	/* Local reset done */

#define MDCTL_RESET_ISO		BIT(12)		/* Enable reset isolation */
#define MDCTL_LRST		BIT(8)		/* Assert local reset when 0 */

#define PDSTAT_STATE_MASK		0x1fU

#define PDCTL_STATE_MASK	BIT(0)
#define PDCTL_STATE_OFF		0U
#define PDCTL_STATE_ON		1U

/* Timeout values for PSC state transitions */
#define PSC_TRANSITION_TIMEOUT_US	10000U	/* 10ms timeout */
#define PSC_TRANSITION_DELAY_US		100U	/* 100us delay between checks */
#define PSC_TRANSITION_RETRY_COUNT	(PSC_TRANSITION_TIMEOUT_US / PSC_TRANSITION_DELAY_US)

/* Module state return values */
#define TI_LPSC_STATE_DISABLED		0U	/* Module is disabled */
#define TI_LPSC_STATE_ENABLED		1U	/* Module is enabled or in retention */
#define TI_LPSC_STATE_TRANSITIONING	2U	/* Module is transitioning */

static struct ti_device *psc_devs;

static void lpsc_module_get_internal(struct ti_device *dev,
				     struct ti_lpsc_module *module,
				     bool use, bool ret);
static void lpsc_module_put_internal(struct ti_device *dev,
				     struct ti_lpsc_module *module,
				     bool use, bool ret);

static uint32_t psc_read(struct ti_device *dev, uint32_t reg)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

	if (psc->base == 0U)
		return 0U;

	return mmio_read_32(psc->base + reg);
}

static void psc_write(struct ti_device *dev, uint32_t val, uint32_t reg)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

	if (psc->base == 0U)
		return;

	mmio_write_32(psc->base + reg, val);
}

ti_pd_idx_t ti_psc_pd_idx(struct ti_device *dev, const struct ti_psc_pd *pd)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ptrdiff_t diff = pd - psc->powerdomains;

	return (ti_pd_idx_t) diff;
}

static inline struct ti_psc_pd *psc_idx2pd(const struct ti_psc_drv_data *psc,
					   ti_pd_idx_t id)
{
	return &psc->powerdomains[id];
}

static const struct ti_psc_pd_data *get_psc_pd_data(struct ti_device *dev,
						    struct ti_psc_pd *pd)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

	return &psc->pd_data[ti_psc_pd_idx(dev, pd)];
}

ti_lpsc_idx_t ti_lpsc_module_idx(struct ti_device *dev, const struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ptrdiff_t diff = module - psc->modules;

	return (ti_lpsc_idx_t) diff;
}

static inline struct ti_lpsc_module *psc_idx2mod(const struct ti_psc_drv_data *psc,
						 ti_lpsc_idx_t id)
{
	return &psc->modules[id];
}

void ti_psc_pd_wait(struct ti_device *dev, struct ti_psc_pd *pd)
{
	uint32_t psc_ptstat = PSC_PTSTAT;
	ti_pd_idx_t psc_idx    = ti_psc_pd_idx(dev, pd);

	/*
	 * Skip waiting for power domains with TI_PSC_PD_SKIP_WAIT flag.
	 * Used for domains where transitions complete based on hardware
	 * conditions rather than PSC status register updates.
	 */
	if ((get_psc_pd_data(dev, pd)->flags & TI_PSC_PD_SKIP_WAIT) != 0U) {
		return;
	}

	if (0U == (get_psc_pd_data(dev, pd)->flags & TI_PSC_PD_ALWAYSON)) {
		uint32_t retry_count = PSC_TRANSITION_RETRY_COUNT;
		/* power domain >= 32 uses PSC_PTSTAT_H register */
		if (psc_idx >= 32U) {
			psc_ptstat = PSC_PTSTAT_H;
		}
		while ((((psc_read(dev, psc_ptstat)) &
			 BIT(psc_idx % 32U)) != 0U) && (retry_count > 0U)) {
			udelay(PSC_TRANSITION_DELAY_US);
			retry_count--;
		}
		if (retry_count == 0U) {
			/* Directly convert to psc to get psc_idx */
			VERBOSE("PSC transition timeout: psc_id=%d pd_id=%d\n",
				(ti_to_psc_drv_data(ti_get_drv_data(dev)))->psc_idx,
				ti_psc_pd_idx(dev, pd));
		}
	}
}

static void pd_initiate(struct ti_device *dev, struct ti_psc_pd *pd)
{
	uint32_t psc_ptcmd = PSC_PTCMD;
	ti_pd_idx_t psc_idx   = ti_psc_pd_idx(dev, pd);

	/* power domain >= 32 uses PSC_PTCMD_H register */
	if (psc_idx >= 32U) {
		psc_ptcmd = PSC_PTCMD_H;
	}

	/* Note: This is a state machine reg */
	psc_write(dev, (uint32_t) BIT(psc_idx % 32U), psc_ptcmd);
}

/**
 * psc_pd_clk_get() - Enable clocks necessary for power domain
 * @data: The const data for the power domain
 *
 * This function calls ti_clk_get on each of the clocks listed
 * under clock_dep for a given power domain. Some power domains
 * require certain clocks to be running while the power domain
 * is in transition or on.
 */
static void psc_pd_clk_get(const struct ti_psc_pd_data *data)
{
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(data->clock_dep); i++) {
		if (data->clock_dep[i] != 0U) {
			struct ti_clk *clkp = ti_clk_lookup(data->clock_dep[i]);

			if (clkp != NULL) {
				(void) ti_clk_get(clkp);
			}
		}
	}
}

void ti_psc_pd_get(struct ti_device *dev, struct ti_psc_pd *pd)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_psc_pd_idx(dev, pd);
	uint32_t pdctl;

	VERBOSE("PD_GET: psc_id=%d pd_id=%d use_count=%d\n",
		psc->psc_idx, idx, pd->use_count);

	if ((pd->use_count++) != 0U) {
		return;
	}

	if ((psc->pd_data[idx].flags & TI_PSC_PD_ALWAYSON) != 0U) {
		return;
	}

	/* Verify any previous transitions have completed */
	ti_psc_pd_wait(dev, pd);

	if ((psc->pd_data[idx].flags & TI_PSC_PD_DEPENDS) != 0U) {
		ti_psc_pd_get(dev, psc_idx2pd(psc,
					      (ti_pd_idx_t) psc->pd_data[idx].depends));
	}

	psc_pd_clk_get(&psc->pd_data[idx]);

	pdctl = psc_read(dev, psc_reg(PSC_PDCTL_BASE, idx));

	if ((pdctl & PDCTL_STATE_MASK) != PDCTL_STATE_ON) {
		/* Avoid redundant power-up transitions */
		pdctl &= ~PDCTL_STATE_MASK;
		pdctl |= PDCTL_STATE_ON;

		/* Note: This is a state machine reg */
		psc_write(dev, pdctl, psc_reg(PSC_PDCTL_BASE, idx));

		pd_initiate(dev, pd);
		ti_psc_pd_wait(dev, pd);
	}

	psc->data->pds_enabled |= (uint32_t) BIT(idx);
}

/**
 * psc_pd_clk_put() - Disable clocks necessary for power domain
 * @data: The const data for the power domain
 *
 * This function calls ti_clk_put on each of the clocks listed
 * under clock_dep for a given power domain. Some power domains
 * require certain clocks to be running while the power domain
 * is in transition or on.
 */
static void psc_pd_clk_put(const struct ti_psc_pd_data *data)
{
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(data->clock_dep); i++) {
		if (data->clock_dep[i] != 0U) {
			struct ti_clk *clkp = ti_clk_lookup(data->clock_dep[i]);

			if (clkp != NULL) {
				ti_clk_put(clkp);
			}
		}
	}
}

void ti_psc_pd_put(struct ti_device *dev, struct ti_psc_pd *pd)
{
	uint32_t pdctl;
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_psc_pd_idx(dev, pd);

	VERBOSE("PD_PUT: psc_id=%d pd_id=%d use_count=%d\n",
		psc->psc_idx, idx, pd->use_count);

	if ((--pd->use_count) != 0U) {
		return;
	}

	if ((psc->pd_data[idx].flags & TI_PSC_PD_ALWAYSON) != 0U) {
		return;
	}

	/* Verify any previous transitions have completed */
	ti_psc_pd_wait(dev, pd);

	pdctl = psc_read(dev, psc_reg(PSC_PDCTL_BASE, idx));
	if ((pdctl & PDCTL_STATE_MASK) != PDCTL_STATE_OFF) {
		/* Avoid redundant power-up transitions */
		pdctl &= ~PDCTL_STATE_MASK;
		pdctl |= PDCTL_STATE_OFF;
		/* Note: This is a state machine reg */
		psc_write(dev, pdctl, psc_reg(PSC_PDCTL_BASE, idx));

		pd_initiate(dev, pd);
		ti_psc_pd_wait(dev, pd);
	}

	psc_pd_clk_put(&psc->pd_data[idx]);

	if ((psc->pd_data[idx].flags & TI_PSC_PD_DEPENDS) != 0U) {
		ti_psc_pd_put(dev, psc_idx2pd(psc,
					      (ti_pd_idx_t) psc->pd_data[idx].depends));
	}

	psc->data->pds_enabled &= ~((uint32_t) BIT(idx));
}

/**
 * lpsc_module_sync_state() - Sync the hardware state of a module with it's software state.
 * @dev: The device struct for this PSC.
 * @module: The module data for this module.
 * @domain_reset: True if syncing just prior to a domain reset,
 *                in which case actual transitions are avoided.
 *
 * This function examines the current state of a given LPSC module and
 * compares it with the last programmed state. If there have been changes it
 * programs the hardware appropriately. It also enables/disables any
 * dependencies as necessary.
 */
static void lpsc_module_sync_state(struct ti_device *dev, struct ti_lpsc_module *module,
				   bool domain_reset)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t mdctl;
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	struct ti_psc_pd *pd = psc_idx2pd(psc, (ti_pd_idx_t) data->powerdomain);
	uint8_t state;		     /* Target module state */
	uint8_t old_state;	     /* Original module state */
	bool new_mrst_ret;     /* MRST induced retention */
	bool old_msrt_ret;
	bool old_pwr;	       /* Power domain should be enabled */
	bool old_ret;	       /* Retention of any kind */
	bool old_en;	       /* Enabled (clocks running) */
	bool old_rst;
	bool new_pwr;
	bool new_ret;
	bool new_en;
	bool new_rst;
	bool transition;       /* A state transition is necessary */
	bool get_en;	       /* Moving from disabled to enabled */
	bool get_ret;	       /* Moving from off to retention */
	bool get_pwr;	       /* Moving from power domain off to on */
	bool put_en;	       /* Moving from enabled to disabled */
	bool put_ret;	       /* Moving from retention to off */
	bool put_pwr;	       /* Moving from power domain on to off */

	/*
	 * Determine target state based on usage counts and module reset:
	 *
	 * +---------+-----------+-------+-------------------------------+
	 * + Enabled | Retention | Reset | State			 |
	 * +=========+===========+=======+===============================+
	 * | No	     | No	 | NA	 | SwRstDisabled (may power off) |
	 * +---------+-----------+-------+-------------------------------+
	 * | No	     | Yes	 | No	 | Disabled			 |
	 * +---------+-----------+-------+-------------------------------+
	 * | No	     | Yes	 | Yes	 | SwRstDisabled (powered-on)	 |
	 * +---------+-----------+-------+-------------------------------+
	 * | Yes     | NA	 | No	 | Enabled			 |
	 * +---------+-----------+-------+-------------------------------+
	 * | Yes     | NA	 | Yes	 | SyncReset			 |
	 * +---------+-----------+-------+-------------------------------+
	 */
	new_mrst_ret = false;
	if ((module->use_count == 0U) && (module->ret_count == 0U)) {
		state = MDSTAT_STATE_SWRSTDISABLE;
	} else if (module->use_count == 0U) {
		/* Retention enabled, but module disabled */
		if (module->mrst_active == 0U) {
			state = MDSTAT_STATE_DISABLE;
		} else {
			new_mrst_ret = true;
			state = MDSTAT_STATE_SWRSTDISABLE;
		}
	} else {
		/* Module enabled (retention setting is don't care) */
		if (module->mrst_active == 0U) {
			state = MDSTAT_STATE_ENABLE;
		} else {
			state = MDSTAT_STATE_SYNCRST;
		}
	}

	/* Promote target state based on disallowed states */
	if ((state == MDSTAT_STATE_SWRSTDISABLE) &&
	    ((data->flags & TI_LPSC_NO_MODULE_RESET) != 0U)) {
		state = MDSTAT_STATE_DISABLE;
		new_mrst_ret = false;
	}
	if ((state == MDSTAT_STATE_DISABLE) && ((data->flags & TI_LPSC_NO_CLOCK_GATING) != 0U)) {
		state = MDSTAT_STATE_ENABLE;
	}
	if ((state == MDSTAT_STATE_SYNCRST) && ((data->flags & TI_LPSC_NO_MODULE_RESET) != 0U)) {
		state = MDSTAT_STATE_ENABLE;
	}

	/* Track transition of old state to new state */
	old_state = module->sw_state;
	module->sw_state = state;
	old_msrt_ret = module->sw_mrst_ret;
	module->sw_mrst_ret =  new_mrst_ret;

	/* Previous setting of retention, enable, and reset */
	old_ret = old_state != MDSTAT_STATE_SWRSTDISABLE;
	old_pwr = old_ret || old_msrt_ret;
	old_en = (old_state == MDSTAT_STATE_SYNCRST) || (old_state == MDSTAT_STATE_ENABLE);
	old_rst = (old_state != MDSTAT_STATE_ENABLE) && (old_state != MDSTAT_STATE_DISABLE);

	/* New setting of retention, enable, and reset */
	new_ret = state != MDSTAT_STATE_SWRSTDISABLE;
	new_pwr = new_ret || new_mrst_ret;
	new_en = (state == MDSTAT_STATE_SYNCRST) || (state == MDSTAT_STATE_ENABLE);
	new_rst = (state != MDSTAT_STATE_ENABLE) && (state != MDSTAT_STATE_DISABLE);

	/* Are we transitioning from no retention/enable to retention/enable? */
	get_ret = !old_ret && new_ret;
	get_pwr = !old_pwr && new_pwr;
	get_en = !old_en && new_en;

	/* Are we transitioning from retention/enable to no retention/enable? */
	put_ret = old_ret && !new_ret;
	put_pwr = old_pwr && !new_pwr;
	put_en = old_en && !new_en;

	/* Make sure our parent LPSC is enabled as necessary */
	if ((get_en || get_ret) && ((data->flags & TI_LPSC_DEPENDS) != 0UL)) {
		const struct ti_psc_drv_data *depends_psc = psc;
		struct ti_device *depends_dev = dev;

		if (data->depends_psc_idx != psc->psc_idx) {
			depends_dev = ti_psc_lookup((ti_psc_idx_t) data->depends_psc_idx);
			depends_psc = ti_to_psc_drv_data(ti_get_drv_data(depends_dev));
		}
		if (depends_dev == NULL) {
			VERBOSE("ACTION FAIL: PSC_INVALID_DEP_DATA dep_pd_id=%d pd_id=%d pos=1\n",
				psc->psc_idx, data->depends_psc_idx);
		} else {
			/*
			 * Moving from a clock stop state to a clock enabled
			 * state or from a retention disable to a retention
			 * enabled state, bump the reference count on our
			 * dependency.
			 */
			lpsc_module_get_internal(depends_dev,
						 psc_idx2mod(depends_psc, data->depends),
						 get_en, get_ret);
		}
	}

	if (old_rst && !new_rst) {
		/* Coming out of reset, bump the context loss count. */
		module->loss_count++;
	}

	if (get_pwr) {
		/* Make sure pd is on. */
		ti_psc_pd_get(dev, pd);
	}

	mdctl = psc_read(dev, psc_reg(PSC_MDCTL_BASE, idx));
	transition = (mdctl & MDSTAT_STATE_MASK) != state;
	if (transition && (domain_reset == false)) {
		/* Verify any previous transitions have completed */
		ti_psc_pd_wait(dev, pd);

		mdctl &= ~MDSTAT_STATE_MASK;
		mdctl |= (uint32_t) state;
		/* Note: This is a state machine reg */
		psc_write(dev, mdctl, psc_reg(PSC_MDCTL_BASE, idx));
	}

	if (domain_reset == false) {
		if (put_pwr) {
			/* Module is ready for power down, drop ref count on pd */
			ti_psc_pd_put(dev, pd);
			if ((pd->use_count != 0U) && transition) {
				/*
				 * If ti_psc_pd_put has a use count of zero, it already
				 * initaited the transition, otherwise we need to
				 * do the transition ourselves.
				 */
				pd_initiate(dev, pd);
			}
		} else if (transition) {
			/*
			 * Initiate transition
			 */
			pd_initiate(dev, pd);
		}
	}

	/* Allow our parent LPSC to be disabled as necessary */
	if ((put_en || put_ret) && ((data->flags & TI_LPSC_DEPENDS) != 0UL)) {
		const struct ti_psc_drv_data *depends_psc = psc;
		struct ti_device *depends_dev = dev;

		if (data->depends_psc_idx != psc->psc_idx) {
			depends_dev = ti_psc_lookup((ti_psc_idx_t) data->depends_psc_idx);
			depends_psc = ti_to_psc_drv_data(ti_get_drv_data(depends_dev));
		}
		if (depends_dev == NULL) {
			VERBOSE("ACTION FAIL: PSC_INVALID_DEP_DATA dep_pd_id=%d pd_id=%d pos=2\n",
				psc->psc_idx, data->depends_psc_idx);
		} else if ((domain_reset == true) && (depends_dev == dev)) {
			/* Ignore self dependencies during domain reset */
		} else {
			/*
			 * Moving from a clock enabled state to a clock stop
			 * state or from a retention enable to a retention
			 * disabled state, drop the reference count on our
			 * dependency.
			 */
			lpsc_module_put_internal(depends_dev,
						 psc_idx2mod(depends_psc, data->depends),
						 put_en, put_ret);
		}
	}
}

uint32_t ti_lpsc_module_get_state(struct ti_device *dev, struct ti_lpsc_module *module)
{
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	uint8_t state;
	uint32_t ret;

	state = (uint8_t)(psc_read(dev, psc_reg(PSC_MDSTAT_BASE, idx)) & MDSTAT_STATE_MASK);

	if (state == MDSTAT_STATE_SWRSTDISABLE) {
		ret = TI_LPSC_STATE_DISABLED;
	} else if ((state == MDSTAT_STATE_DISABLE) ||
		   (state == MDSTAT_STATE_ENABLE) ||
		   (state == MDSTAT_STATE_SYNCRST)) {
		ret = TI_LPSC_STATE_ENABLED;
	} else {
		ret = TI_LPSC_STATE_TRANSITIONING;
	}

	return ret;
}

void ti_lpsc_module_set_reset_iso(struct ti_device *dev, struct ti_lpsc_module *module,
				  bool enable)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	bool is_enabled;
	uint32_t mdctl;

	if (0U == (data->flags & TI_LPSC_HAS_RESET_ISO)) {
		return;
	}

	mdctl = psc_read(dev, psc_reg(PSC_MDCTL_BASE, idx));
	is_enabled = (mdctl & MDCTL_RESET_ISO) != 0U;

	if (enable != is_enabled) {
		if (enable) {
			mdctl |= MDCTL_RESET_ISO;
		} else {
			mdctl &= ~MDCTL_RESET_ISO;
		}
		/* Note: This is a state machine reg */
		psc_write(dev, mdctl, psc_reg(PSC_MDCTL_BASE, idx));
	}
}

bool ti_lpsc_module_get_reset_iso(struct ti_device *dev, struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	bool ret;

	if (0U == (data->flags & TI_LPSC_HAS_RESET_ISO)) {
		ret = false;
	} else {
		ret = (psc_read(dev, psc_reg(PSC_MDCTL_BASE, idx)) & MDCTL_RESET_ISO) != 0U;
	}
	return ret;
}

/* Does not bump context loss count */
void ti_lpsc_module_set_local_reset(struct ti_device *dev, struct ti_lpsc_module *module,
				    bool enable)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	bool is_enabled;
	uint32_t mdctl;

	if (0U == (data->flags & TI_LPSC_HAS_LOCAL_RESET)) {
		return;
	}

	mdctl = psc_read(dev, (uint32_t) psc_reg(PSC_MDCTL_BASE, idx));
	is_enabled = (mdctl & MDCTL_LRST) == 0U;

	if (enable != is_enabled) {
		VERBOSE("SET_LOCAL_RESET: psc_id=%d lpsc_id=%d enable=%d\n",
			psc->psc_idx, idx, ((enable == true) ? 1U : 0U));

		if (enable) {
			mdctl &= ~MDCTL_LRST;
		} else {
			mdctl |= MDCTL_LRST;
		}

		/* Note: This is a state machine reg */
		psc_write(dev, mdctl, psc_reg(PSC_MDCTL_BASE, idx));
	}
}

void ti_lpsc_module_set_module_reset(struct ti_device *dev, struct ti_lpsc_module *module,
				     bool enable)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	bool is_enabled;

	is_enabled = module->mrst_active != 0U;

	if ((enable != is_enabled) && ((data->flags & TI_LPSC_NO_MODULE_RESET) == 0U)) {
		VERBOSE("SET_MODULE_RESET/SET_LOCAL_RESET: psc_id=%d lpsc_id=%d enable=%d\n",
			psc->psc_idx, idx, (enable ? 1U : 0U));

		if (enable) {
			module->mrst_active = 1U;
		} else {
			module->mrst_active = 0U;
		}

		lpsc_module_sync_state(dev, module, false);
		ti_lpsc_module_wait(dev, module);
	}
}

bool ti_lpsc_module_get_local_reset(struct ti_device *dev, struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	bool ret;

	if (0U == (data->flags & TI_LPSC_HAS_LOCAL_RESET)) {
		ret = false;
	} else {
		ret = (psc_read(dev, (uint32_t) psc_reg(PSC_MDCTL_BASE, idx)) & MDCTL_LRST) == 0U;
	}
	return ret;
}

bool ti_lpsc_module_get_module_reset(struct ti_device *dev __unused,
				     const struct ti_lpsc_module *module)
{
	return module->mrst_active == 1U;
}

void ti_lpsc_module_wait(struct ti_device *dev, struct ti_lpsc_module *module)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, module);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];
	struct ti_psc_pd *pd = psc_idx2pd(psc, (ti_pd_idx_t) data->powerdomain);
	uint32_t retry_count = PSC_TRANSITION_RETRY_COUNT;

	ti_psc_pd_wait(dev, pd);

	while ((((psc_read(dev, psc_reg(PSC_MDSTAT_BASE, idx))) & MDSTAT_BUSY_MASK) != 0U) &&
	       (retry_count > 0U)) {
		udelay(PSC_TRANSITION_DELAY_US);
		retry_count--;
	}
	if (retry_count == 0U) {
		VERBOSE("PSC module transition timeout: psc_id=%d lpsc_id=%d\n",
			psc->psc_idx, idx);
	}

	/* Only wait for reset to complete if module is in use */
	if ((module->use_count) != 0U) {
		uint32_t mask = MDSTAT_MRSTDONE;

		retry_count = PSC_TRANSITION_RETRY_COUNT;

		if (!ti_lpsc_module_get_local_reset(dev, module)) {
			mask |= MDSTAT_LRSTDONE;
		}
		while ((0U == ((psc_read(dev, psc_reg(PSC_MDSTAT_BASE, idx))) & mask)) &&
		       (retry_count > 0U)) {
			udelay(PSC_TRANSITION_DELAY_US);
			retry_count--;
		}
		if (retry_count == 0U) {
			VERBOSE("PSC module reset done timeout: psc_id=%d lpsc_id=%d\n",
				psc->psc_idx, idx);
		}
	}
}

static void lpsc_module_clk_get(struct ti_device *dev, struct ti_lpsc_module *mod)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, mod);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];

	/* Get clock dependency - currently only one clock supported */
	if (data->clock_dep[0] != 0U) {
		struct ti_clk *clkp = ti_clk_lookup(data->clock_dep[0]);

		if (clkp != NULL) {
			(void) ti_clk_get(clkp);
		}
	}
}

/**
 * lpsc_module_clk_put() - Disable clocks necessary for LPSC module
 * @dev: The device for this PSC.
 * @mod: The const data for the LPSC module.
 * @wait: True to wait for the module to complete transitioning
 *        before disabling clocks. Set to false when called as
 *        part of a domain reset.
 *
 * This function calls ti_clk_put on each of the clocks listed
 * under clock_dep for a given LPSC module. Some modules
 * require certain clocks to be running while the module
 * is in transition or on.
 */
static void lpsc_module_clk_put(struct ti_device *dev, struct ti_lpsc_module *mod,
				bool wait)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx = ti_lpsc_module_idx(dev, mod);
	const struct ti_lpsc_module_data *data = &psc->mod_data[idx];

	/* Put clock dependency - currently only one clock supported */
	if (data->clock_dep[0] != 0U) {
		struct ti_clk *clkp = ti_clk_lookup(data->clock_dep[0]);

		if (clkp != NULL) {
			/*
			 * We have to wait for the transition to complete
			 * taking a clock away.
			 */
			if (wait) {
				ti_lpsc_module_wait(dev, mod);
			}
			ti_clk_put(clkp);
		}
	}
}

static void lpsc_module_get_internal(struct ti_device *dev,
				     struct ti_lpsc_module *module,
				     bool use, bool ret)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	bool modify = false;

	if (use) {
		VERBOSE("MODULE_GET: psc_id=%d lpsc_id=%d use_count=%d\n",
			psc->psc_idx, ti_lpsc_module_idx(dev, module),
			module->use_count);
		module->use_count++;
		if (module->use_count == 1U) {
			lpsc_module_clk_get(dev, module);
			modify = true;
		}
	}

	if (ret) {
		VERBOSE("RETENTION_GET: psc_id=%d lpsc_id=%d ret_count=%d\n",
			psc->psc_idx, ti_lpsc_module_idx(dev, module),
			module->ret_count);
		module->ret_count++;
		if (module->ret_count == 1U) {
			lpsc_module_clk_get(dev, module);
			modify = true;
		}
	}

	if (modify) {
		lpsc_module_sync_state(dev, module, false);
		ti_lpsc_module_wait(dev, module);
	}
}

static void lpsc_module_put_internal(struct ti_device *dev,
				     struct ti_lpsc_module *module,
				     bool use, bool ret)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	bool modify = false;

	if (use) {
		VERBOSE("MODULE_PUT: psc_id=%d lpsc_id=%d use_count=%d\n",
			psc->psc_idx, ti_lpsc_module_idx(dev, module),
			module->use_count);
		module->use_count--;
		if (module->use_count == 0U) {
			modify = true;
		}
	}

	if (ret) {
		VERBOSE("RETENTION_PUT: psc_id=%d lpsc_id=%d ret_count=%d\n",
			psc->psc_idx, ti_lpsc_module_idx(dev, module),
			module->ret_count);
		module->ret_count--;
		if (module->ret_count == 0U) {
			modify = true;
		}
	}

	if (modify) {
		lpsc_module_sync_state(dev, module, false);
		if ((module->use_count == 0U) && use) {
			lpsc_module_clk_put(dev, module, true);
		}
		if ((module->ret_count == 0U) && ret) {
			lpsc_module_clk_put(dev, module, true);
		}
	}
}

void ti_lpsc_module_get(struct ti_device *dev, struct ti_lpsc_module *module)
{
	lpsc_module_get_internal(dev, module, true, false);
}

void ti_lpsc_module_put(struct ti_device *dev, struct ti_lpsc_module *module)
{
	lpsc_module_put_internal(dev, module, true, false);
}

void ti_lpsc_module_ret_get(struct ti_device *dev, struct ti_lpsc_module *module)
{
	lpsc_module_get_internal(dev, module, false, true);
}

void ti_lpsc_module_ret_put(struct ti_device *dev, struct ti_lpsc_module *module)
{
	lpsc_module_put_internal(dev, module, false, true);
}

/* Drop power up ref counts */
static void psc_pd_drop_pwr_up_ref(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_pd_idx_t idx;

	for (idx = 0U; idx < psc->pd_count; idx++) {
		struct ti_psc_pd *pd = psc_idx2pd(psc, idx);

		if ((pd->pwr_up_enabled) != false) {
			pd->pwr_up_enabled = false;
			ti_psc_pd_put(dev, pd);
		}
	}
}

struct ti_device *ti_psc_lookup(ti_psc_idx_t id)
{
	struct ti_device *dev;

	dev = psc_devs;
	while (dev != NULL) {
		const struct ti_psc_drv_data *psc;

		psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

		if ((ti_psc_idx_t) psc->psc_idx == id) {
			break;
		}

		dev = psc->data->next;
	}

	return dev;
}

struct ti_psc_pd *ti_psc_lookup_pd(struct ti_device *dev, ti_pd_idx_t id)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	struct ti_psc_pd *pd = NULL;

	if ((id < psc->pd_count) &&
	    ((psc->pd_data[id].flags & TI_PSC_PD_EXISTS) != 0U)) {
		pd = psc_idx2pd(psc, id);
	}
	return pd;
}

struct ti_lpsc_module *ti_psc_lookup_lpsc(struct ti_device *dev, ti_pd_idx_t id)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	struct ti_lpsc_module *mod = NULL;

	if ((id < psc->module_count) &&
	    ((psc->mod_data[id].flags & TI_LPSC_MODULE_EXISTS) != 0U)) {
		mod = psc_idx2mod(psc, id);
	}
	return mod;
}

/**
 * psc_initialize_pds() - Initialize all the PSC powerdomains.
 *
 * This initializes the powerdomains by waiting for them to finish any
 * active transitions, reading their state, and synchronizing it with the
 * in memory state.
 */
static void psc_initialize_pds(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_pd_idx_t idx;
	struct ti_psc_pd *pd;
	uint8_t state;

	/* First pass, find out which domains are on */
	for (idx = 0U; idx < psc->pd_count; idx++) {
		pd = psc_idx2pd(psc, idx);

		if (0U == (psc->pd_data[idx].flags & TI_PSC_PD_EXISTS)) {
			continue;
		}

		VERBOSE("PD_INIT: psc_id=%d lpsc_id=%d\n", psc->psc_idx, idx);
		ti_psc_pd_wait(dev, pd);
		state = (uint8_t) psc_read(dev, psc_reg(PSC_PDSTAT_BASE, (uint32_t) idx));
		state &= (uint8_t) PDSTAT_STATE_MASK;

		/*
		 * Mark a PD as power up in use so we don't power everything
		 * off before PMMC startup is complete
		 */
		pd->pwr_up_enabled =  ((state == PDCTL_STATE_ON) ||
				       ((psc->pd_data[idx].flags & TI_PSC_PD_ALWAYSON) != 0U));
	}

	/* Second pass, sync use count and impossible hardware states */
	for (idx = 0U; idx < psc->pd_count; idx++) {
		pd = psc_idx2pd(psc, idx);

		if (pd->pwr_up_enabled != false) {
			ti_psc_pd_get(dev, pd);
		}
	}
}

/**
 * psc_uninitialize_pds() - Unititialize all the power domains of a PSC.
 * @dev: The device associated with this PSC.
 *
 * This sets all the power domains in a PSC to a pre-initialized state
 * in preparation for the reset of it's reset domain. Because power
 * domains can only have dependencies on other domains within the same
 * PSC, this just means ensuring that the clock references are dropped.
 */
static void psc_uninitialize_pds(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_pd_idx_t idx;

	for (idx = 0U; idx < psc->pd_count; idx++) {
		struct ti_psc_pd *pd = psc_idx2pd(psc, idx);

		if (((psc->pd_data[idx].flags & TI_PSC_PD_EXISTS) != 0U) &&
		    (pd->use_count != 0U) &&
		    ((psc->pd_data[idx].flags & TI_PSC_PD_ALWAYSON) == 0U)) {
			psc_pd_clk_put(&psc->pd_data[idx]);
		}

		pd->use_count = 0U;
		pd->pwr_up_enabled = false;
	}

	psc->data->pds_enabled = 0U;
}

/* Drop power up ref counts */
void ti_psc_drop_pwr_up_ref(void)
{
	struct ti_device *dev = psc_devs;
	const struct ti_psc_drv_data *psc;
	ti_lpsc_idx_t idx;

	while (dev != NULL) {
		psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

		for (idx = 0U; idx < psc->module_count; idx++) {
			struct ti_lpsc_module *mod = psc_idx2mod(psc, idx);

			if (mod->pwr_up_enabled != 0U) {
				mod->pwr_up_enabled = 0U;
				ti_lpsc_module_put(dev, mod);
			}
			if (mod->pwr_up_ret != 0U) {
				mod->pwr_up_ret = 0U;
				ti_lpsc_module_ret_put(dev, mod);
			}
		}
		dev = psc->data->next;
	}
}

/**
 * psc_initialize_modules() - Initialize all the PSC modules.
 *
 * This initializes the modules by waiting for them to finish any active
 * transitions and reading their state.
 */
static void psc_initialize_modules(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_lpsc_idx_t idx;

	/* First pass, find out which modules are enabled */
	for (idx = 0U; idx < psc->module_count; idx++) {
		struct ti_lpsc_module *mod = psc_idx2mod(psc, idx);
		uint32_t val;
		uint32_t retry_count;

		if (0U == (psc->mod_data[idx].flags & TI_LPSC_MODULE_EXISTS)) {
			continue;
		}

		retry_count = PSC_TRANSITION_RETRY_COUNT;
		while ((((psc_read(dev, psc_reg(PSC_MDSTAT_BASE,
						(uint32_t) idx))) &
						MDSTAT_BUSY_MASK) != 0U) &&
						(retry_count > 0U)) {
			udelay(PSC_TRANSITION_DELAY_US);
			retry_count--;
		}
		if (retry_count == 0U) {
			VERBOSE("PSC module busy timeout: psc_id=%d lpsc_id=%d\n",
				psc->psc_idx, idx);
		}

		val = psc_read(dev, psc_reg(PSC_MDSTAT_BASE, (uint32_t) idx));
		val &= MDSTAT_STATE_MASK;

		/* Ref count as if we are moving out of off state */
		mod->sw_state = MDSTAT_STATE_SWRSTDISABLE;
		mod->sw_mrst_ret = false;

		if ((val == MDSTAT_STATE_ENABLE) || (val == MDSTAT_STATE_SYNCRST)) {
			mod->pwr_up_enabled = 1U;
			mod->pwr_up_ret = 1U;
		} else if (val == MDSTAT_STATE_DISABLE) {
			mod->pwr_up_enabled = 0U;
			mod->pwr_up_ret = 1U;
		} else if (val == MDSTAT_STATE_SWRSTDISABLE) {
			mod->pwr_up_enabled = 0U;
			mod->pwr_up_ret = 0U;
		} else {
			/* Invalid initial state, try turning everything on */
			mod->pwr_up_ret = 1U;
			mod->pwr_up_enabled = 1U;
		}
	}
}

/**
 * psc_initialize_modules_finish() - Finish initializing all the PSC modules.
 *
 * This finishes the initialization of modules by synchronizing their
 * state with the in memory state.
 */
static void psc_initialize_modules_finish(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_lpsc_idx_t idx;

	/* Second pass, sync ref counts */
	for (idx = 0U; idx < psc->module_count; idx++) {
		struct ti_lpsc_module *mod = psc_idx2mod(psc, idx);

		lpsc_module_get_internal(dev, mod, (mod->pwr_up_enabled != 0U),
					 (mod->pwr_up_ret != 0U));
	}

	psc_pd_drop_pwr_up_ref(dev);
}

/**
 * psc_uninitialize_modules() - Unititialize all the LPSC modules of a PSC.
 * @dev: The device associated with this PSC.
 *
 * This sets all the LPSC modules in a PSC to a pre-initialized state
 * in preparation for the reset of it's reset domain. Because LPSC
 * modules can have dependencies on other domains, those dependencies
 * need to be droppe as appropriate. Any clock dependencies are also
 * dropped.
 */
static void psc_uninitialize_modules(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	ti_lpsc_idx_t idx;

	/* First pass, find out which modules are enabled */
	for (idx = 0U; idx < psc->module_count; idx++) {
		struct ti_lpsc_module *mod = psc_idx2mod(psc, idx);

		if ((psc->mod_data[idx].flags & TI_LPSC_MODULE_EXISTS) == 0U) {
			continue;
		}

		if (mod->use_count != 0U) {
			lpsc_module_clk_put(dev, mod, false);
		}
		if (mod->ret_count != 0U) {
			lpsc_module_clk_put(dev, mod, false);
		}

		mod->use_count = 0U;
		mod->ret_count = 0U;
		mod->pwr_up_enabled = 0U;
		mod->pwr_up_ret = 0U;

		lpsc_module_sync_state(dev, mod, true);
		mod->sw_state = MDSTAT_STATE_SWRSTDISABLE;
		mod->sw_mrst_ret = false;
	}
}

/**
 * psc_module_deps_ready() - Check if PSCs we depend on have completed their initial config
 *
 * PSCs can have domains that depend on domains in other PSCs. We break up
 * initialization into two stages because PSCs can have cross dependencies.
 * Once all PSCs in a cross dependency set have completed, each one can
 * move to the second stage of initialization. We use the -EAGAIN system
 * to manage this.
 *
 * Return: 0 on success, <0 on failure.
 */
static int32_t psc_module_deps_ready(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc = ti_to_psc_drv_data(ti_get_drv_data(dev));
	uint32_t idx;
	ti_psc_idx_t id;

	for (idx = 0; idx < psc->module_count; idx++) {
		if (((psc->mod_data[idx].flags & TI_LPSC_DEPENDS) != 0UL) &&
		    (psc->mod_data[idx].depends_psc_idx != psc->psc_idx)) {
			id = (ti_psc_idx_t) psc->mod_data[idx].depends_psc_idx;
			if (ti_psc_lookup(id) == NULL) {
				return -EAGAIN;
			}
		}
	}

	return 0;
}

static int32_t psc_pre_init(struct ti_device *dev)
{
	const struct ti_drv_data *data;
	const struct ti_psc_drv_data *psc;

	data = ti_get_drv_data(dev);
	psc = ti_to_psc_drv_data(data);

	/* Only perform initial configuration if it has not been completed */
	if (ti_psc_lookup((ti_psc_idx_t) psc->psc_idx) == NULL) {
		if (psc->base == 0U) {
			return -EINVAL;
		}

		psc_initialize_pds(dev);
		psc_initialize_modules(dev);

		/* This marks initial config as complete */
		psc->data->next = psc_devs;
		psc_devs = dev;
	}

	return 0;
}

static int32_t psc_post_init(struct ti_device *dev)
{
	int32_t ret;

	ret = psc_module_deps_ready(dev);

	if (ret == 0) {
		psc_initialize_modules_finish(dev);
	}

	return ret;
}

/**
 * psc_uninit() - Uninitialize this PSC.
 * @dev: The device associated with this PSC.
 *
 * Perform the steps necessary to bring this PSC back to a pre-init state.
 * This is performed before a reset domain reset so that the PSC can be
 * re-initialized after the reset is complete.
 */
static void psc_uninit(struct ti_device *dev)
{
	const struct ti_psc_drv_data *psc;
	struct ti_device *curr_dev;
	struct ti_device **last_dev;

	psc = ti_to_psc_drv_data(ti_get_drv_data(dev));

	psc_uninitialize_modules(dev);
	psc_uninitialize_pds(dev);

	/* Remove from list of uninitialized PSCs */
	curr_dev = psc_devs;
	last_dev = &psc_devs;
	while ((curr_dev != NULL) && (curr_dev != dev)) {
		const struct ti_psc_drv_data *curr_psc;

		curr_psc = ti_to_psc_drv_data(ti_get_drv_data(curr_dev));
		curr_dev = curr_psc->data->next;
		last_dev = &curr_psc->data->next;
	}
	if (curr_dev == dev) {
		*last_dev = psc->data->next;
		psc->data->next = NULL;
	}
}

const struct ti_drv psc_drv = {
	.pre_init	= psc_pre_init,
	.post_init	= psc_post_init,
	.uninit		= psc_uninit,
};

/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Management Framework Core
 *
 * This is the core clock framework that provides the central clock tree
 * management, including reference counting, parent-child relationships,
 * frequency propagation, and clock state management. It implements the
 * main clock API functions for get/put, set frequency, query frequency,
 * and state control that are used by device drivers and power management.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>

#include <common/debug.h>

#include <ti_clk.h>
#include <ti_clk_mux.h>
#include <ti_device.h>

uint32_t ti_clk_value_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			       uint32_t min_hz,
			       uint32_t max_hz,
			       bool *changed)
{
	assert(clkp != NULL);
	assert(changed != NULL);

	/* Validate that target frequency is within acceptable range */
	if (target_hz < min_hz || target_hz > max_hz) {
		*changed = false;
		return 0;
	}

	/*
	 * Store the frequency value. ti_clk_set_value() validates that
	 * freq_idx is within bounds of soc_clock_values[] array.
	 */
	if (!ti_clk_set_value(clkp->freq_idx, target_hz)) {
		*changed = false;
		return 0;
	}

	*changed = true;
	return target_hz;
}

uint32_t ti_clk_value_get_freq(struct ti_clk *clkp)
{
	assert(clkp != NULL);

	return ti_clk_get_value(clkp->freq_idx);
}

uint32_t ti_clk_get_parent_freq(struct ti_clk *clkp)
{
	const struct ti_clk_parent *p;
	struct ti_clk *parent_clk;

	assert(clkp != NULL);

	p = ti_clk_mux_get_parent(clkp);

	if ((p != NULL) && (p->div != 0U)) {
		parent_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
		if (parent_clk != NULL) {
			return ti_clk_get_freq(parent_clk) / p->div;
		}
	}

	return 0;
}

uint32_t ti_clk_generic_set_freq_parent(struct ti_clk *clkp, struct ti_clk *parent,
					uint32_t target_hz, uint32_t min_hz,
					uint32_t max_hz,
					bool *changed, uint32_t div)
{
	uint32_t parent_min_hz, parent_target_hz, parent_max_hz;
	uint32_t actual_parent_hz;
	uint32_t max_possible_child_hz;
	uint32_t max_possible_parent_hz;

	assert(parent != NULL);
	assert(changed != NULL);
	assert(div != 0U);

	*changed = false;

	(void)clkp;

	/*
	 * This clock's output frequency is derived from parent via divider:
	 *   child_freq = parent_freq / div
	 *
	 * To achieve desired child frequency, we must configure parent:
	 *   required_parent_freq = desired_child_freq * div
	 *
	 * Since multiplication can overflow uint32_t, we calculate the
	 * maximum representable child and parent frequencies.
	 */
	max_possible_child_hz = UINT32_MAX / div;
	max_possible_parent_hz = max_possible_child_hz * div;

	/* If child's minimum requirement is too high, cannot satisfy */
	if (min_hz > max_possible_child_hz) {
		return 0;
	}

	/* Convert child frequency range to parent frequency range */
	parent_min_hz = min_hz * div;

	if (target_hz > max_possible_child_hz) {
		parent_target_hz = max_possible_parent_hz;
	} else {
		parent_target_hz = target_hz * div;
	}

	if (max_hz > max_possible_child_hz) {
		parent_max_hz = max_possible_parent_hz;
	} else {
		parent_max_hz = max_hz * div;
	}

	/* Request parent clock to operate in the required range */
	actual_parent_hz = ti_clk_set_freq(parent, parent_target_hz,
					   parent_min_hz, parent_max_hz,
					   changed);

	/* Convert actual parent frequency back to child frequency */
	return actual_parent_hz / div;
}

/**
 * ti_clk_generic_set_freq() - Generic clock set-frequency handler.
 * @clkp: The clock to set the frequency on.
 * @target_hz: Target frequency in Hz.
 * @min_hz: Minimum acceptable frequency in Hz.
 * @max_hz: Maximum acceptable frequency in Hz.
 * @changed: Set to true if the frequency was changed.
 *
 * Return: The actual frequency set, or 0 on failure.
 */
static uint32_t ti_clk_generic_set_freq(struct ti_clk *clkp,
					uint32_t target_hz,
					uint32_t min_hz,
					uint32_t max_hz,
					bool *changed)
{
	const struct ti_clk_parent *p;
	struct ti_clk *parent;
	uint32_t freq;

	p = ti_clk_mux_get_parent(clkp);
	*changed = false;

	if ((p != NULL) && ((clkp->data_flags & TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ) != 0U)) {
		parent = ti_clk_lookup((ti_clk_idx_t) p->clk);

		if (parent != NULL) {
			return ti_clk_generic_set_freq_parent(clkp, parent,
							      target_hz,
							      min_hz, max_hz,
							      changed,
							      p->div);
		}
	} else {
		freq = ti_clk_get_freq(clkp);

		if ((freq >= min_hz) && (freq <= max_hz)) {
			return freq;
		}
	}

	return 0;
}

uint32_t ti_clk_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			 uint32_t min_hz, uint32_t max_hz,
			 bool *changed)
{
	uint32_t ret;

	assert(clkp != NULL);
	assert(changed != NULL);

	*changed = false;

	if ((__atomic_load_n(&clkp->flags, __ATOMIC_ACQUIRE) & TI_CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if (clkp->drv->set_freq != NULL) {
		ret = clkp->drv->set_freq(clkp, target_hz, min_hz,
					  max_hz, changed);
	} else {
		ret = ti_clk_generic_set_freq(clkp, target_hz, min_hz, max_hz,
					      changed);
	}

	if (ret != 0U) {
		VERBOSE("CLOCK_SET_RATE: clk_id=%u freq=%u\n", ti_clk_id(clkp), target_hz);
	} else {
		WARN("CLOCK_SET_RATE failed: clk_id=%u freq=%u\n", ti_clk_id(clkp), target_hz);
	}

	return ret;
}

uint32_t ti_clk_get_freq(struct ti_clk *clkp)
{
	uint32_t ret;

	assert(clkp != NULL);

	if ((__atomic_load_n(&clkp->flags, __ATOMIC_ACQUIRE) & TI_CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if (clkp->drv->get_freq != NULL) {
		ret = clkp->drv->get_freq(clkp);
	} else {
		ret = ti_clk_get_parent_freq(clkp);
	}

	return ret;
}

uint32_t ti_clk_get_state(struct ti_clk *clkp)
{
	uint32_t ret = TI_CLK_HW_STATE_DISABLED;
	const struct ti_clk_parent *p;
	struct ti_clk *clkp_parent;

	assert(clkp != NULL);

	if ((__atomic_load_n(&clkp->flags, __ATOMIC_ACQUIRE) & TI_CLK_FLAG_INITIALIZED) == 0U) {
		ret = TI_CLK_HW_STATE_DISABLED;
	} else if (clkp->drv->get_state != NULL) {
		ret = clkp->drv->get_state(clkp);
	} else {
		p = ti_clk_mux_get_parent(clkp);

		if (p != NULL) {
			clkp_parent = ti_clk_lookup((ti_clk_idx_t) p->clk);

			if (clkp_parent != NULL) {
				ret = ti_clk_get_state(clkp_parent);
			}
		}
	}

	return ret;
}

bool ti_clk_set_state(struct ti_clk *clkp, bool enable)
{
	bool ret = true;

	assert(clkp != NULL);

	if ((__atomic_load_n(&clkp->flags, __ATOMIC_ACQUIRE) & TI_CLK_FLAG_INITIALIZED) == 0U) {
		/* defer action */
		return ret;
	}

	if (clkp->drv->set_state != NULL) {
		ret = clkp->drv->set_state(clkp, enable);
	}

	return ret;
}

bool ti_clk_get(struct ti_clk *clkp)
{
	bool ret = true;
	const struct ti_clk_parent *p;
	struct ti_clk *clkp_parent = NULL;

	assert(clkp != NULL);

	if (__atomic_load_n(&clkp->ref_count, __ATOMIC_ACQUIRE) == 0U) {
		p = ti_clk_mux_get_parent(clkp);
		if (p != NULL) {
			clkp_parent = ti_clk_lookup((ti_clk_idx_t) p->clk);
		}

		if (clkp_parent != NULL) {
			ret = ti_clk_get(clkp_parent);
		}

		if (ret == true) {
			ret = ti_clk_set_state(clkp, true);
			if (ret == false) {
				WARN("CLOCK_GET failed: clk_id=%u\n", ti_clk_id(clkp));
				if (clkp_parent != NULL) {
					ti_clk_put(clkp_parent);
				}
			} else {
				VERBOSE("CLOCK_GET: clk_id=%u\n", ti_clk_id(clkp));
			}
		}
	}

	if (ret) {
		(void)__atomic_fetch_add(&clkp->ref_count, 1U, __ATOMIC_ACQ_REL);
	}

	return ret;
}

void ti_clk_put(struct ti_clk *clkp)
{
	uint8_t new_count;
	const struct ti_clk_parent *p;
	struct ti_clk *clkp_parent;

	assert(clkp != NULL);
	assert(__atomic_load_n(&clkp->ref_count, __ATOMIC_ACQUIRE) > 0U);

	new_count = __atomic_sub_fetch(&clkp->ref_count, 1U, __ATOMIC_ACQ_REL);
	if (new_count == 0U) {
		p = ti_clk_mux_get_parent(clkp);
		ti_clk_set_state(clkp, false);
		VERBOSE("CLOCK_PUT: clk_id=%u\n", ti_clk_id(clkp));
		if (p != NULL) {
			clkp_parent = ti_clk_lookup((ti_clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				ti_clk_put(clkp_parent);
			}
		}
	}
}

static int32_t ti_clk_register_clock(struct ti_clk *clkp)
{
	struct ti_clk *clkp_parent = NULL;
	const struct ti_clk_parent *p;
	uint8_t parent_flags;
	int32_t ret;

	p = ti_clk_mux_get_parent(clkp);
	if (p != NULL) {
		clkp_parent = ti_clk_lookup((ti_clk_idx_t) p->clk);
	}
	if (clkp_parent != NULL) {
		parent_flags = __atomic_load_n(&clkp_parent->flags, __ATOMIC_ACQUIRE);
		if ((parent_flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
			return -EAGAIN;
		}
	}

	if (clkp->drv->init != NULL) {
		ret = clkp->drv->init(clkp);
		if (ret != 0) {
			return ret;
		}
	}

	(void)__atomic_or_fetch(&clkp->flags, TI_CLK_FLAG_INITIALIZED, __ATOMIC_ACQ_REL);
	if (__atomic_load_n(&clkp->ref_count, __ATOMIC_ACQUIRE) != 0U) {
		if (!ti_clk_set_state(clkp, true)) {
			WARN("CLOCK_ENABLE failed during init: clk_id=%u\n", ti_clk_id(clkp));
		}
		VERBOSE("CLOCK_ENABLE: clk_id=%u\n", ti_clk_id(clkp));
	}
	if (clkp->drv->get_state != NULL) {
		if (clkp->drv->get_state(clkp) != TI_CLK_HW_STATE_DISABLED) {
			(void)__atomic_or_fetch(&clkp->flags,
						TI_CLK_FLAG_PWR_UP_EN,
						__ATOMIC_ACQ_REL);
		}
	}

	return 0;
}

void ti_clk_drop_pwr_up_en(void)
{
	ti_clk_idx_t i;
	uint8_t flags;

	for (i = 0U; i < soc_clock_count; i++) {
		flags = __atomic_load_n(&soc_clocks[i].flags, __ATOMIC_ACQUIRE);
		if ((flags & TI_CLK_FLAG_PWR_UP_EN) != 0U) {
			ti_clk_put(&soc_clocks[i]);
			(void)__atomic_and_fetch(&soc_clocks[i].flags,
						 (uint8_t)~TI_CLK_FLAG_PWR_UP_EN,
						 __ATOMIC_ACQ_REL);
		}
	}
}

int32_t ti_clk_init(void)
{
	bool progress;
	bool contents;
	int32_t last_error = 0;
	uint32_t i;
	uint32_t clock_count = soc_clock_count;
	struct ti_clk *clkp;
	uint8_t clk_flags;
	int32_t curr;
	devgrp_t devgrp;

	/* soc_devgroups[0] is unused; real entries start at index TI_PM_DEVGRP_00 (1U) */
	for (i = 1U; i < soc_devgroup_count; i++) {
		/* Translate compressed internal representation to bitfield */
		devgrp = (devgrp_t) BIT(i - 1U);

		/* First disabled devgroup, stop at this clock index */
		if (ti_pm_devgroup_is_enabled(devgrp) == false) {
			clock_count = soc_devgroups[i].clk_idx;
			break;
		}
	}

	contents = false;
	progress = false;

	/* Loop through all the clocks to initialize them */
	for (i = 0U; i < clock_count; i++) {
		clkp = &soc_clocks[i];

		clk_flags = __atomic_load_n(&clkp->flags, __ATOMIC_ACQUIRE);
		if (((clk_flags & TI_CLK_FLAG_INITIALIZED) == 0U) &&
		    (clkp->drv != NULL)) {
			contents = true;
			curr = ti_clk_register_clock(clkp);
			if (curr != -EAGAIN) {
				progress = true;
				if (curr != 0) {
					last_error = curr;
				}
			}
		}
	}

	if (progress) {
		for (i = 0U; i < clock_count; i++) {
			clk_flags = __atomic_load_n(&soc_clocks[i].flags, __ATOMIC_ACQUIRE);
			if ((clk_flags & TI_CLK_FLAG_PWR_UP_EN) != 0U) {
				if (!ti_clk_get(&soc_clocks[i])) {
					/* ti_clk_get failed for one of the clocks */
					return -ENODEV;
				}
			}
		}
		return last_error;
	}

	if (contents) {
		/* We processed at least one clock but didn't make progress */
		return -EAGAIN;
	}

	/* No clocks needed processing - this is success */
	return 0;
}

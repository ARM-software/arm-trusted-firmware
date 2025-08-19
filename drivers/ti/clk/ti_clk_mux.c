/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Multiplexer Driver
 *
 * This driver implements clock multiplexer (mux) functionality, allowing
 * selection between multiple parent clock sources. It handles parent
 * selection via register writes, validates parent choices, manages clock
 * dividers associated with each parent, and properly handles reference
 * counting when switching between parents.
 */

#include <common/debug.h>
#include <errno.h>
#include <ti_clk_mux.h>
#include <ti_container_of.h>
#include <ti_io.h>

static uint32_t ti_clk_mux_get_parent_value(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_data_mux *mux;
	const struct ti_clk_data_mux_reg *reg;
	uint32_t reg_val;
	uint32_t n_minus_one;

	mux = container_of(clk_datap->data, const struct ti_clk_data_mux, data);
	reg = container_of(mux, const struct ti_clk_data_mux_reg, data_mux);

	/*
	 * Hack, temporarily return parent 0 for muxes without register
	 * assignments.
	 */
	if (reg->reg == 0U) {
		reg_val = 0U;
	} else {
		reg_val = readl(reg->reg);
		reg_val >>= (uint32_t) reg->bit;

		n_minus_one = mux->num_parents - 1U;
		reg_val &= MASK_COVER_FOR_NUMBER(n_minus_one);
	}

	return reg_val;
}

static const struct ti_clk_parent *ti_clk_mux_get_parent_internal(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_data_mux *mux;
	uint32_t val;
	bool valid_parent;
	bool valid_div;

	mux = container_of(clk_datap->data, const struct ti_clk_data_mux, data);
	val = ti_clk_mux_get_parent_value(clkp);

	valid_parent = (val < mux->num_parents);
	valid_div = (mux->parents[val].div != 0U);

	return (valid_parent && valid_div) ? &mux->parents[val] : NULL;
}

static bool ti_clk_mux_set_parent_internal(struct ti_clk *clkp, uint8_t new_parent)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_data_mux *mux;
	const struct ti_clk_data_mux_reg *reg;
	uint32_t reg_val;
	uint32_t parent_val;
	uint32_t mask;
	uint32_t inverted_mask;
	uint32_t n_minus_one;
	uint32_t reg_bit;
	bool ret = true;

	mux = container_of(clk_datap->data, const struct ti_clk_data_mux, data);
	reg = container_of(mux, const struct ti_clk_data_mux_reg, data_mux);

	if (reg->reg == 0U) {
		/*
		 * Hack, temporarily ignore assignments for muxes without
		 * register assignments.
		 */
	} else {
		reg_val = readl(reg->reg);
		n_minus_one = mux->num_parents - 1U;
		reg_bit = (uint32_t)reg->bit;
		mask = (MASK_COVER_FOR_NUMBER(n_minus_one) << reg_bit);
		inverted_mask = ~mask;
		reg_val &= inverted_mask;
		parent_val = (uint32_t)new_parent << reg_bit;
		reg_val |= parent_val;
		writel(reg_val, reg->reg);

		VERBOSE("CLOCK_SET_PARENT: clk_id=%d new_parent=%d\n",
			clk_id(clkp), new_parent);
	}

	return ret;
}

/**
 * @brief Save mux clock parent selection during suspend
 *
 * Saves the current parent selection of a mux clock before entering
 * low power mode so it can be restored during resume.
 *
 * @param clkp Mux clock to save parent selection for
 *
 * @return 0 on success
 */
static int32_t ti_clk_mux_suspend_save(struct ti_clk *clkp)
{
	clkp->saved_val = ti_clk_mux_get_parent_value(clkp);

	return 0;
}

/**
 * @brief Restore mux clock parent selection during resume
 *
 * Restores the saved parent selection of a mux clock after exiting
 * low power mode, returning the clock to its pre-suspend state.
 *
 * @param clkp Mux clock to restore parent selection for
 *
 * @return 0 on success, -EINVAL on failure
 */
static int32_t ti_clk_mux_resume_restore(struct ti_clk *clkp)
{
	bool error;
	int32_t ret = 0;

	error = ti_clk_mux_set_parent_internal(clkp, (uint8_t)(clkp->saved_val));
	if (error == false) {
		ret = -EINVAL;
	}

	return ret;
}

const struct ti_clk_drv_mux ti_clk_drv_mux_reg_ro = {
	.get_parent = ti_clk_mux_get_parent_internal,
};

const struct ti_clk_drv_mux ti_clk_drv_mux_reg = {
	.set_parent = ti_clk_mux_set_parent_internal,
	.get_parent = ti_clk_mux_get_parent_internal,
	.drv			= {
		.suspend_save	= ti_clk_mux_suspend_save,
		.resume_restore = ti_clk_mux_resume_restore,
	},
};

const struct ti_clk_parent *ti_clk_mux_get_parent(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_parent *ret = NULL;
	bool is_mux_type;

	is_mux_type = (clk_datap->type == TI_CLK_TYPE_MUX);

	if (is_mux_type) {
		const struct ti_clk_drv_mux *mux;

		mux = container_of(clk_datap->drv, const struct ti_clk_drv_mux, drv);
		ret = mux->get_parent(clkp);
	} else {
		bool valid_div = (clk_datap->parent.div > 0U);

		ret = valid_div ? &clk_datap->parent : NULL;
	}

	return ret;
}

bool ti_clk_mux_set_parent(struct ti_clk *clkp, uint8_t new_parent)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_drv_mux *mux_drv = NULL;
	const struct ti_clk_data_mux *mux_data = NULL;
	const struct ti_clk_parent *op;
	struct ti_clk *parent;
	bool ret = true;
	bool done = false;

	if (clk_datap->type != TI_CLK_TYPE_MUX) {
		ret = false;
		done = true;
	}

	if (!done) {
		mux_data = container_of(clk_datap->data,
					const struct ti_clk_data_mux, data);
		if (new_parent >= mux_data->num_parents) {
			ret = false;
			done = true;
		} else if (mux_data->parents[new_parent].div == 0U) {
			ret = false;
			done = true;
		} else {
			/* Do Nothing */
		}
	}

	if (!done) {
		mux_drv = container_of(clk_datap->drv,
				       const struct ti_clk_drv_mux, drv);
		if (!mux_drv->set_parent) {
			ret = false;
			done = true;
		}
	}

	if (!done) {
		op = mux_drv->get_parent(clkp);
		if (op && (op->clk == mux_data->parents[new_parent].clk)
		    && (op->div == mux_data->parents[new_parent].div)) {
			ret = true;
			done = true;
		}
	}

	if (!done) {
		parent = clk_lookup((clk_idx_t) mux_data->parents[new_parent].clk);
		if (!parent) {
			ret = false;
			done = true;
		} else if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
			ret = false;
			done = true;
		} else if (clkp->ref_count == 0U) {
			/* No get necessary */
		} else if (!ti_clk_get(parent)) {
			ret = false;
			done = true;
		} else {
			/* Do Nothing */
		}
	}

	if (!done) {
		if (!mux_drv->set_parent(clkp, new_parent)) {
			if (clkp->ref_count != 0U) {
				ti_clk_put(parent);
			}
			ret = false;
			done = true;
		}
	}

	if (!done) {
		if (op && (clkp->ref_count != 0U)) {
			struct ti_clk *op_parent;

			op_parent = clk_lookup((clk_idx_t) op->clk);
			if (op_parent != NULL) {
				ti_clk_put(op_parent);
			}
		}
	}

	return ret;
}

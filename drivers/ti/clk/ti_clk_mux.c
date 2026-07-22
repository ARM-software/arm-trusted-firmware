/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
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

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <ti_clk_mux.h>
#include <ti_container_of.h>

static uint32_t ti_clk_mux_get_parent_value(struct ti_clk *clkp)
{
	const struct ti_clk_data_mux *mux;
	const struct ti_clk_data_mux_reg *reg;
	uint32_t reg_val;
	uint32_t n_minus_one;

	assert(clkp != NULL);

	mux = ti_container_of(clkp->data, const struct ti_clk_data_mux, data);
	reg = ti_container_of(mux, const struct ti_clk_data_mux_reg, data_mux);

	/*
	 * Muxes without register assignments (reg == 0) have a fixed
	 * parent at index 0.
	 */
	if (reg->reg == 0U) {
		reg_val = 0U;
	} else {
		reg_val = mmio_read_32(reg->reg);
		reg_val >>= (uint32_t) reg->bit;

		n_minus_one = mux->num_parents - 1U;
		reg_val &= TI_MASK_COVER_FOR_NUMBER(n_minus_one);
	}

	return reg_val;
}

static const struct ti_clk_parent *ti_clk_mux_get_parent_internal(struct ti_clk *clkp)
{
	const struct ti_clk_data_mux *mux;
	uint32_t val;
	bool valid_parent;
	bool valid_div;

	assert(clkp != NULL);

	mux = ti_container_of(clkp->data, const struct ti_clk_data_mux, data);
	val = ti_clk_mux_get_parent_value(clkp);

	valid_parent = (val < mux->num_parents);
	valid_div = (mux->parents[val].div != 0U);

	return (valid_parent && valid_div) ? &mux->parents[val] : NULL;
}

static bool ti_clk_mux_set_parent_internal(struct ti_clk *clkp, uint8_t new_parent)
{
	const struct ti_clk_data_mux *mux;
	const struct ti_clk_data_mux_reg *reg;
	uint32_t mask;
	uint32_t val;

	assert(clkp != NULL);

	mux = ti_container_of(clkp->data, const struct ti_clk_data_mux, data);
	reg = ti_container_of(mux, const struct ti_clk_data_mux_reg, data_mux);

	if (reg->reg == 0U) {
		/*
		 * Muxes without register assignments are fixed and cannot
		 * change parent selection.
		 */
	} else {
		mask = TI_MASK_COVER_FOR_NUMBER(mux->num_parents - 1U)
				<< (uint32_t)reg->bit;
		val = (uint32_t)new_parent << (uint32_t)reg->bit;

		mmio_clrsetbits_32((uintptr_t)reg->reg, mask, val);

		VERBOSE("CLOCK_SET_PARENT: clk_id=%d new_parent=%d\n",
			ti_clk_id(clkp), new_parent);
	}

	return true;
}

const struct ti_clk_drv_mux ti_clk_drv_mux_reg_ro = {
	.get_parent = ti_clk_mux_get_parent_internal,
};

const struct ti_clk_drv_mux ti_clk_drv_mux_reg = {
	.set_parent = ti_clk_mux_set_parent_internal,
	.get_parent = ti_clk_mux_get_parent_internal,
};

const struct ti_clk_parent *ti_clk_mux_get_parent(struct ti_clk *clkp)
{
	const struct ti_clk_parent *ret = NULL;
	const struct ti_clk_drv_mux *mux;
	bool is_mux_type;
	bool valid_div;

	assert(clkp != NULL);

	is_mux_type = (clkp->type == TI_CLK_TYPE_MUX);

	if (is_mux_type) {
		mux = ti_container_of(clkp->drv, const struct ti_clk_drv_mux, drv);
		ret = mux->get_parent(clkp);
	} else {
		valid_div = (clkp->parent.div > 0U);
		ret = valid_div ? &clkp->parent : NULL;
	}

	return ret;
}

bool ti_clk_mux_set_parent(struct ti_clk *clkp, uint8_t new_parent)
{
	const struct ti_clk_drv_mux *mux_drv = NULL;
	const struct ti_clk_data_mux *mux_data = NULL;
	const struct ti_clk_parent *op = NULL;
	struct ti_clk *parent = NULL;
	struct ti_clk *op_parent = NULL;

	assert(clkp != NULL);

	if (clkp->type != TI_CLK_TYPE_MUX) {
		return false;
	}

	mux_data = ti_container_of(clkp->data,
				   const struct ti_clk_data_mux, data);
	if (new_parent >= mux_data->num_parents) {
		return false;
	}

	if (mux_data->parents[new_parent].div == 0U) {
		return false;
	}

	mux_drv = ti_container_of(clkp->drv,
				  const struct ti_clk_drv_mux, drv);
	if (mux_drv->set_parent == NULL) {
		return false;
	}

	op = mux_drv->get_parent(clkp);
	if ((op != NULL) && (op->clk == mux_data->parents[new_parent].clk) &&
	    (op->div == mux_data->parents[new_parent].div)) {
		return true;
	}

	parent = ti_clk_lookup((ti_clk_idx_t) mux_data->parents[new_parent].clk);
	if (parent == NULL) {
		return false;
	}

	if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
		return false;
	}

	if (clkp->ref_count != 0U) {
		if (!ti_clk_get(parent)) {
			return false;
		}
	}

	if (!mux_drv->set_parent(clkp, new_parent)) {
		if (clkp->ref_count != 0U) {
			ti_clk_put(parent);
		}
		return false;
	}

	if ((op != NULL) && (clkp->ref_count != 0U)) {
		op_parent = ti_clk_lookup((ti_clk_idx_t) op->clk);
		if (op_parent != NULL) {
			ti_clk_put(op_parent);
		}
	}

	return true;
}

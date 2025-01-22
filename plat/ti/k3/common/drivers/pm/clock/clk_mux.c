/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_mux.h>
#include <lib/container_of.h>
#include <ilog.h>
#include <lib/io.h>
#include <lib/trace.h>

static uint32_t clk_mux_get_parent_value(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_mux *mux;
	const struct clk_data_mux_reg *reg;
	uint32_t v;

	mux = container_of(clk_datap->data, const struct clk_data_mux, data);
	reg = container_of(mux, const struct clk_data_mux_reg, data_mux);

	/*
	 * Hack, temporarily return parent 0 for muxes without register
	 * assignments.
	 */
	if (reg->reg == 0U) {
		v = 0;
	} else {
		v = readl(reg->reg);
		v >>= reg->bit;

		v &= (uint32_t) ((1U << ilog32(mux->n - 1U)) - 1U);
	}

	return v;
}

static const struct clk_parent *clk_mux_get_parent(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_mux *mux;
	uint32_t v;


	mux = container_of(clk_datap->data, const struct clk_data_mux, data);


	v = clk_mux_get_parent_value(clkp);


	return ((v < mux->n) && (mux->parents[v].div != 0U)) ?
		&mux->parents[v] : NULL;
}

static bool clk_mux_set_parent(struct clk *clkp, uint8_t new_parent)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_mux *mux;
	const struct clk_data_mux_reg *reg;
	uint32_t v;
	bool ret = true;

	mux = container_of(clk_datap->data, const struct clk_data_mux, data);
	reg = container_of(mux, const struct clk_data_mux_reg, data_mux);

	if (reg->reg == 0U) {
		/*
		 * Hack, temporarily ignore assignments for muxes without
		 * register assignments.
		 */
	} else {
		v = readl(reg->reg);
		v &= ~(((1U << ilog32(mux->n - 1U)) - 1U) << reg->bit);
		v |= (uint32_t) (new_parent << reg->bit);
		ti_clk_writel(v, reg->reg);
		pm_trace(TRACE_PM_ACTION_CLOCK_SET_PARENT,
			 (((uint16_t) new_parent << TRACE_PM_VAL_CLOCK_VAL_SHIFT) &
			  TRACE_PM_VAL_CLOCK_VAL_MASK) |
			 ((clk_id(clkp) << TRACE_PM_VAL_CLOCK_ID_SHIFT) &
			  TRACE_PM_VAL_CLOCK_ID_MASK));
	}

	return ret;
}

const struct clk_drv_mux clk_drv_mux_reg_ro = {
	.get_parent	= clk_mux_get_parent,
};

const struct clk_drv_mux clk_drv_mux_reg = {
	.set_parent		= clk_mux_set_parent,
	.get_parent		= clk_mux_get_parent,
};

const struct clk_parent *clk_get_parent(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_parent *ret = NULL;

	if (clk_datap->type == CLK_TYPE_MUX) {
		const struct clk_drv_mux *mux;

		mux = container_of(clk_datap->drv, const struct clk_drv_mux,
				   drv);
		ret = mux->get_parent(clkp);
	} else {
		ret = ((clk_datap->parent.div > 0U) ? &clk_datap->parent : NULL);
	}

	return ret;
}

/* FIXME: freq change ok/notify? new freq in range? */
bool clk_set_parent(struct clk *clkp, uint8_t new_parent)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_drv_mux *mux_drv = NULL;
	const struct clk_data_mux *mux_data = NULL;
	const struct clk_parent *op;
	struct clk *parent;
	bool ret = true;
	bool done = false;

	if (clk_datap->type != CLK_TYPE_MUX) {
		ret = false;
		done = true;
	}

	if (!done) {
		mux_data = container_of(clk_datap->data,
					const struct clk_data_mux, data);
		if (new_parent >= mux_data->n) {
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
				       const struct clk_drv_mux, drv);
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
		} else if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
			ret = false;
			done = true;
		} else if (clkp->ref_count == 0U) {
			/* No get necessary */
		} else if (!clk_get(parent)) {
			ret = false;
			done = true;
		} else {
			/* Do Nothing */
		}
	}

	if (!done) {
		if (!mux_drv->set_parent(clkp, new_parent)) {
			if (clkp->ref_count != 0U) {
				clk_put(parent);
			}
			ret = false;
			done = true;
		}
	}

	if (!done) {
		if (op && (clkp->ref_count != 0U)) {
			struct clk *op_parent;

			op_parent = clk_lookup((clk_idx_t) op->clk);
			if (op_parent != NULL) {
				clk_put(op_parent);
			}
		}
	}

	return ret;
}

/*
 * Copyright (C) 2022-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include "clk-stm32-core.h"
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#define TIMEOUT_US_200MS	U(200000)
#define TIMEOUT_US_2S		U(2000000)
#define CLKSRC_TIMEOUT		TIMEOUT_US_200MS

static struct spinlock reg_lock;
static struct spinlock refcount_lock;

static struct stm32_clk_priv *stm32_clock_data;

const struct stm32_clk_ops clk_mux_ops;

struct stm32_clk_priv *clk_stm32_get_priv(void)
{
	return stm32_clock_data;
}

static void _clk_lock(struct spinlock *lock)
{
	if (stm32mp_lock_available()) {
		/* Assume interrupts are masked */
		spin_lock(lock);
	}
}

static void _clk_unlock(struct spinlock *lock)
{
	if (stm32mp_lock_available()) {
		spin_unlock(lock);
	}
}

void clk_stm32_rcc_regs_lock(void)
{
	_clk_lock(&reg_lock);
}

void clk_stm32_rcc_regs_unlock(void)
{
	_clk_unlock(&reg_lock);
}

struct clk_oscillator_data *clk_oscillator_get_data(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_osc_cfg *osc_cfg = clk->clock_cfg;
	int osc_id = osc_cfg->osc_id;

	return &priv->osci_data[osc_id];
}

static int clk_gate_enable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_gate_cfg *cfg = clk->clock_cfg;

	mmio_setbits_32(priv->base + cfg->offset, BIT_32(cfg->bit_idx));

	/* Make sure the clock register has been written */
	(void)mmio_read_32(priv->base + cfg->offset);

	return 0;
}

static void clk_gate_disable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_gate_cfg *cfg = clk->clock_cfg;

	dmbsy(); /* Ensure previous transactions are performed. */

	mmio_clrbits_32(priv->base + cfg->offset, BIT_32(cfg->bit_idx));

	/* Make sure the clock register has been written */
	(void)mmio_read_32(priv->base + cfg->offset);
}

static bool clk_gate_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_gate_cfg *cfg = clk->clock_cfg;

	return ((mmio_read_32(priv->base + cfg->offset) & BIT_32(cfg->bit_idx)) != 0U);
}

const struct stm32_clk_ops clk_gate_ops = {
	.enable		= clk_gate_enable,
	.disable	= clk_gate_disable,
	.is_enabled	= clk_gate_is_enabled,
};

void _clk_stm32_gate_disable(struct stm32_clk_priv *priv, uint16_t gate_id)
{
	const struct gate_cfg *gate = &priv->gates[gate_id];
	uintptr_t addr = priv->base + gate->offset;

	if (gate->set_clr != 0U) {
		mmio_write_32(addr + RCC_MP_ENCLRR_OFFSET, BIT_32(gate->bit_idx));
	} else {
		mmio_clrbits_32(addr, BIT_32(gate->bit_idx));
	}
}

int _clk_stm32_gate_enable(struct stm32_clk_priv *priv, uint16_t gate_id)
{
	const struct gate_cfg *gate = &priv->gates[gate_id];
	uintptr_t addr = priv->base + gate->offset;

	if (gate->set_clr != 0U) {
		mmio_write_32(addr, BIT_32(gate->bit_idx));

	} else {
		mmio_setbits_32(addr, BIT_32(gate->bit_idx));
	}

	return 0;
}

const struct clk_stm32 *_clk_get(struct stm32_clk_priv *priv, int id)
{
	if ((unsigned int)id >= priv->num) {
		panic();
	}

	return &priv->clks[id];
}

static const struct stm32_clk_ops *_clk_get_ops(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);

	assert(clk->ops != (uint8_t)NO_OPS);

	return priv->ops_array[clk->ops];
}

#define clk_div_mask(_width) GENMASK_32(((_width) - 1U), 0U)

static unsigned int _get_table_div(const struct clk_div_table *table,
				   unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++) {
		if (clkt->val == val) {
			return clkt->div;
		}
	}

	return 0;
}

static unsigned int _get_div(const struct clk_div_table *table,
			     unsigned int val, unsigned long flags,
			     uint8_t width)
{
	if ((flags & CLK_DIVIDER_ONE_BASED) != 0UL) {
		return val;
	}

	if ((flags & CLK_DIVIDER_POWER_OF_TWO) != 0UL) {
		return BIT_32(val);
	}

	if ((flags & CLK_DIVIDER_MAX_AT_ZERO) != 0UL) {
		return (val != 0U) ? val : BIT_32(width);
	}

	if (table != NULL) {
		return _get_table_div(table, val);
	}

	return val + 1U;
}

int clk_mux_set_parent(struct stm32_clk_priv *priv, uint16_t pid, uint8_t sel)
{
	const struct parent_cfg *parents = &priv->parents[pid & MUX_PARENT_MASK];
	const struct mux_cfg *mux = parents->mux;
	uintptr_t address = priv->base + mux->offset;
	uint32_t mask;
	uint64_t timeout;

	mask = MASK_WIDTH_SHIFT(mux->width, mux->shift);

	mmio_clrsetbits_32(address, mask, (sel << mux->shift) & mask);

	if (mux->bitrdy == MUX_NO_BIT_RDY) {
		return 0;
	}

	timeout = timeout_init_us(CLKSRC_TIMEOUT);

	mask = BIT_32(mux->bitrdy);

	while ((mmio_read_32(address) & mask) == 0U) {
		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}
	}

	return 0;
}

int _clk_stm32_set_parent(struct stm32_clk_priv *priv, int clk, int clkp)
{
	const struct parent_cfg *parents;
	uint16_t pid;
	uint8_t sel;
	int old_parent;

	pid = priv->clks[clk].parent;

	if ((pid == CLK_IS_ROOT) || (pid < MUX_MAX_PARENTS)) {
		return -EINVAL;
	}

	old_parent = _clk_stm32_get_parent(priv, clk);
	if (old_parent < 0) {
		return old_parent;
	}
	if (old_parent == clkp) {
		return 0;
	}

	parents = &priv->parents[pid & MUX_PARENT_MASK];

	for (sel = 0; sel <  parents->num_parents; sel++) {
		if (parents->id_parents[sel] == (uint16_t)clkp) {
			bool clk_was_enabled = _clk_stm32_is_enabled(priv, clk);
			int err = 0;

			/* Enable the parents (for glitch free mux) */
			_clk_stm32_enable(priv, clkp);
			_clk_stm32_enable(priv, old_parent);

			err = clk_mux_set_parent(priv, pid, sel);

			_clk_stm32_disable(priv, old_parent);

			if (clk_was_enabled) {
				_clk_stm32_disable(priv, old_parent);
			} else {
				_clk_stm32_disable(priv, clkp);
			}

			return err;
		}
	}

	return -EINVAL;
}

int clk_mux_get_parent(struct stm32_clk_priv *priv, uint32_t mux_id)
{
	const struct parent_cfg *parent;
	const struct mux_cfg *mux;
	uint32_t mask;

	if (mux_id >= priv->nb_parents) {
		panic();
	}

	parent = &priv->parents[mux_id];
	mux = parent->mux;

	mask = MASK_WIDTH_SHIFT(mux->width, mux->shift);

	return (mmio_read_32(priv->base + mux->offset) & mask) >> mux->shift;
}

int _clk_stm32_set_parent_by_index(struct stm32_clk_priv *priv, int clk, int sel)
{
	uint16_t pid;

	pid = priv->clks[clk].parent;

	if ((pid == CLK_IS_ROOT) || (pid < MUX_MAX_PARENTS)) {
		return -EINVAL;
	}

	return clk_mux_set_parent(priv, pid, sel);
}

int _clk_stm32_get_parent(struct stm32_clk_priv *priv, int clk_id)
{
	const struct stm32_clk_ops *ops = _clk_get_ops(priv, clk_id);
	const struct parent_cfg *parent;
	uint16_t mux_id;
	int sel;

	mux_id = priv->clks[clk_id].parent;
	if (mux_id == CLK_IS_ROOT) {
		return CLK_IS_ROOT;
	}

	if (mux_id < MUX_MAX_PARENTS) {
		return mux_id & MUX_PARENT_MASK;
	}

	mux_id &= MUX_PARENT_MASK;
	parent = &priv->parents[mux_id];

	if (ops->get_parent != NULL) {
		sel = ops->get_parent(priv, clk_id);
	} else {
		sel = clk_mux_get_parent(priv, mux_id);
	}

	if ((sel >= 0) && (sel < parent->num_parents)) {
		return parent->id_parents[sel];
	}

	return -EINVAL;
}

int _clk_stm32_get_parent_index(struct stm32_clk_priv *priv, int clk_id)
{
	uint16_t mux_id;

	mux_id = priv->clks[clk_id].parent;
	if (mux_id == CLK_IS_ROOT) {
		return CLK_IS_ROOT;
	}

	if (mux_id < MUX_MAX_PARENTS) {
		return mux_id & MUX_PARENT_MASK;
	}

	mux_id &= MUX_PARENT_MASK;

	return clk_mux_get_parent(priv, mux_id);
}

int _clk_stm32_get_parent_by_index(struct stm32_clk_priv *priv, int clk_id, int idx)
{
	const struct parent_cfg *parent;
	uint16_t mux_id;

	mux_id = priv->clks[clk_id].parent;
	if (mux_id == CLK_IS_ROOT) {
		return CLK_IS_ROOT;
	}

	if (mux_id < MUX_MAX_PARENTS) {
		return mux_id & MUX_PARENT_MASK;
	}

	mux_id &= MUX_PARENT_MASK;
	parent = &priv->parents[mux_id];

	if (idx < parent->num_parents) {
		return parent->id_parents[idx];
	}

	return -EINVAL;
}

int clk_get_index(struct stm32_clk_priv *priv, unsigned long binding_id)
{
	unsigned int i;

	for (i = 0U; i < priv->num; i++) {
		if (binding_id == priv->clks[i].binding) {
			return (int)i;
		}
	}

	return -EINVAL;
}

unsigned long _clk_stm32_get_rate(struct stm32_clk_priv *priv, int id)
{
	const struct stm32_clk_ops *ops = _clk_get_ops(priv, id);
	int parent;

	if ((unsigned int)id >= priv->num) {
		return 0UL;
	}

	parent = _clk_stm32_get_parent(priv, id);
	if (parent < 0) {
		return 0UL;
	}

	if (ops->recalc_rate != NULL) {
		unsigned long prate = 0UL;

		if (parent != CLK_IS_ROOT) {
			prate = _clk_stm32_get_rate(priv, parent);
		}

		return ops->recalc_rate(priv, id, prate);
	}

	if (parent == CLK_IS_ROOT) {
		panic();
	}

	return _clk_stm32_get_rate(priv, parent);
}

unsigned long _clk_stm32_get_parent_rate(struct stm32_clk_priv *priv, int id)
{
	int parent_id = _clk_stm32_get_parent(priv, id);

	if (parent_id < 0) {
		return 0UL;
	}

	return _clk_stm32_get_rate(priv, parent_id);
}

static uint8_t _stm32_clk_get_flags(struct stm32_clk_priv *priv, int id)
{
	return priv->clks[id].flags;
}

bool _stm32_clk_is_flags(struct stm32_clk_priv *priv, int id, uint8_t flag)
{
	if ((_stm32_clk_get_flags(priv, id) & flag) != 0U) {
		return true;
	}

	return false;
}

int clk_stm32_enable_call_ops(struct stm32_clk_priv *priv, uint16_t id)
{
	const struct stm32_clk_ops *ops = _clk_get_ops(priv, id);

	if (ops->enable != NULL) {
		ops->enable(priv, id);
	}

	return 0;
}

static int _clk_stm32_enable_core(struct stm32_clk_priv *priv, int id)
{
	int parent;
	int ret = 0;

	if (priv->gate_refcounts[id] == 0U) {
		parent = _clk_stm32_get_parent(priv, id);
		if (parent < 0) {
			return parent;
		}
		if (parent != CLK_IS_ROOT) {
			ret = _clk_stm32_enable_core(priv, parent);
			if (ret != 0) {
				return ret;
			}
		}
		clk_stm32_enable_call_ops(priv, id);
	}

	priv->gate_refcounts[id]++;

	if (priv->gate_refcounts[id] == UINT8_MAX) {
		ERROR("%s: %d max enable count !", __func__, id);
		panic();
	}

	return 0;
}

int _clk_stm32_enable(struct stm32_clk_priv *priv, int id)
{
	int ret;

	_clk_lock(&refcount_lock);
	ret = _clk_stm32_enable_core(priv, id);
	_clk_unlock(&refcount_lock);

	return ret;
}

void clk_stm32_disable_call_ops(struct stm32_clk_priv *priv, uint16_t id)
{
	const struct stm32_clk_ops *ops = _clk_get_ops(priv, id);

	if (ops->disable != NULL) {
		ops->disable(priv, id);
	}
}

static void _clk_stm32_disable_core(struct stm32_clk_priv *priv, int id)
{
	int parent;

	if ((priv->gate_refcounts[id] == 1U) && _stm32_clk_is_flags(priv, id, CLK_IS_CRITICAL)) {
		return;
	}

	if (priv->gate_refcounts[id] == 0U) {
		/* case of clock ignore unused */
		if (_clk_stm32_is_enabled(priv, id)) {
			clk_stm32_disable_call_ops(priv, id);
			return;
		}
		VERBOSE("%s: %d already disabled !\n\n", __func__, id);
		return;
	}

	if (--priv->gate_refcounts[id] > 0U) {
		return;
	}

	clk_stm32_disable_call_ops(priv, id);

	parent = _clk_stm32_get_parent(priv, id);
	if ((parent >= 0) && (parent != CLK_IS_ROOT)) {
		_clk_stm32_disable_core(priv, parent);
	}
}

void _clk_stm32_disable(struct stm32_clk_priv *priv, int id)
{
	_clk_lock(&refcount_lock);

	_clk_stm32_disable_core(priv, id);

	_clk_unlock(&refcount_lock);
}

bool _clk_stm32_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct stm32_clk_ops *ops = _clk_get_ops(priv, id);

	if (ops->is_enabled != NULL) {
		return ops->is_enabled(priv, id);
	}

	return priv->gate_refcounts[id];
}

static int clk_stm32_enable(unsigned long binding_id)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int id;

	id = clk_get_index(priv, binding_id);
	if (id == -EINVAL) {
		ERROR("%s: unsupported clock id %lu\n", __func__, binding_id);
		return id;
	}

	return _clk_stm32_enable(priv, id);
}

static void clk_stm32_disable(unsigned long binding_id)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int id;

	id = clk_get_index(priv, binding_id);
	if (id != -EINVAL) {
		_clk_stm32_disable(priv, id);
	} else {
		ERROR("%s: unsupported clock id %lu\n", __func__, binding_id);
	}
}

static bool clk_stm32_is_enabled(unsigned long binding_id)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int id;

	id = clk_get_index(priv, binding_id);
	if (id == -EINVAL) {
		ERROR("%s: unsupported clock id %lu\n", __func__, binding_id);
		return false;
	}

	return _clk_stm32_is_enabled(priv, id);
}

static unsigned long clk_stm32_get_rate(unsigned long binding_id)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int id;

	id = clk_get_index(priv, binding_id);
	if (id == -EINVAL) {
		ERROR("%s: unsupported clock id %lu\n", __func__, binding_id);
		return 0UL;
	}

	return _clk_stm32_get_rate(priv, id);
}

static int clk_stm32_get_parent(unsigned long binding_id)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int id;

	id = clk_get_index(priv, binding_id);
	if (id == -EINVAL) {
		ERROR("%s: unsupported clock id %lu\n", __func__, binding_id);
		return id;
	}

	return _clk_stm32_get_parent(priv, id);
}

static const struct clk_ops stm32mp_clk_ops = {
	.enable		= clk_stm32_enable,
	.disable	= clk_stm32_disable,
	.is_enabled	= clk_stm32_is_enabled,
	.get_rate	= clk_stm32_get_rate,
	.get_parent	= clk_stm32_get_parent,
};

void clk_stm32_enable_critical_clocks(void)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	unsigned int i;

	for (i = 0U; i < priv->num; i++) {
		if (_stm32_clk_is_flags(priv, i, CLK_IS_CRITICAL)) {
			_clk_stm32_enable(priv, i);
		}
	}
}

static void stm32_clk_register(void)
{
	clk_register(&stm32mp_clk_ops);
}

uint32_t clk_stm32_div_get_value(struct stm32_clk_priv *priv, int div_id)
{
	const struct div_cfg *divider = &priv->div[div_id];
	uint32_t val = 0;

	val = mmio_read_32(priv->base + divider->offset) >> divider->shift;
	val &= clk_div_mask(divider->width);

	return val;
}

unsigned long _clk_stm32_divider_recalc(struct stm32_clk_priv *priv,
					int div_id,
					unsigned long prate)
{
	const struct div_cfg *divider = &priv->div[div_id];
	uint32_t val = clk_stm32_div_get_value(priv, div_id);
	unsigned int div = 0U;

	div = _get_div(divider->table, val, divider->flags, divider->width);
	if (div == 0U) {
		return prate;
	}

	return div_round_up((uint64_t)prate, div);
}

unsigned long clk_stm32_divider_recalc(struct stm32_clk_priv *priv, int id,
				       unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_stm32_div_cfg *div_cfg = clk->clock_cfg;

	return _clk_stm32_divider_recalc(priv, div_cfg->id, prate);
}

const struct stm32_clk_ops clk_stm32_divider_ops = {
	.recalc_rate	= clk_stm32_divider_recalc,
};

int clk_stm32_set_div(struct stm32_clk_priv *priv, uint32_t div_id, uint32_t value)
{
	const struct div_cfg *divider;
	uintptr_t address;
	uint64_t timeout;
	uint32_t mask;

	if (div_id >= priv->nb_div) {
		panic();
	}

	divider = &priv->div[div_id];
	address = priv->base + divider->offset;

	mask = MASK_WIDTH_SHIFT(divider->width, divider->shift);
	mmio_clrsetbits_32(address, mask, (value << divider->shift) & mask);

	if (divider->bitrdy == DIV_NO_BIT_RDY) {
		return 0;
	}

	timeout = timeout_init_us(CLKSRC_TIMEOUT);
	mask = BIT_32(divider->bitrdy);

	while ((mmio_read_32(address) & mask) == 0U) {
		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}
	}

	return 0;
}

int _clk_stm32_gate_wait_ready(struct stm32_clk_priv *priv, uint16_t gate_id,
			       bool ready_on)
{
	const struct gate_cfg *gate = &priv->gates[gate_id];
	uintptr_t address = priv->base + gate->offset;
	uint32_t mask_rdy = BIT_32(gate->bit_idx);
	uint64_t timeout;
	uint32_t mask_test;

	if (ready_on) {
		mask_test = BIT_32(gate->bit_idx);
	} else {
		mask_test = 0U;
	}

	timeout = timeout_init_us(TIMEOUT_US_2S);

	while ((mmio_read_32(address) & mask_rdy) != mask_test) {
		if (timeout_elapsed(timeout)) {
			break;
		}
	}

	if ((mmio_read_32(address) & mask_rdy) != mask_test) {
		return -ETIMEDOUT;
	}

	return 0;
}

int clk_stm32_gate_enable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_stm32_gate_cfg *cfg = clk->clock_cfg;
	const struct gate_cfg *gate = &priv->gates[cfg->id];
	uintptr_t addr = priv->base + gate->offset;

	if (gate->set_clr != 0U) {
		mmio_write_32(addr, BIT_32(gate->bit_idx));

	} else {
		mmio_setbits_32(addr, BIT_32(gate->bit_idx));
	}

	return 0;
}

void clk_stm32_gate_disable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_stm32_gate_cfg *cfg = clk->clock_cfg;
	const struct gate_cfg *gate = &priv->gates[cfg->id];
	uintptr_t addr = priv->base + gate->offset;

	if (gate->set_clr != 0U) {
		mmio_write_32(addr + RCC_MP_ENCLRR_OFFSET, BIT_32(gate->bit_idx));
	} else {
		mmio_clrbits_32(addr, BIT_32(gate->bit_idx));
	}
}

bool _clk_stm32_gate_is_enabled(struct stm32_clk_priv *priv, int gate_id)
{
	const struct gate_cfg *gate;
	uintptr_t addr;

	gate = &priv->gates[gate_id];
	addr = priv->base + gate->offset;

	return ((mmio_read_32(addr) & BIT_32(gate->bit_idx)) != 0U);
}

bool clk_stm32_gate_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_stm32_gate_cfg *cfg = clk->clock_cfg;

	return _clk_stm32_gate_is_enabled(priv, cfg->id);
}

const struct stm32_clk_ops clk_stm32_gate_ops = {
	.enable		= clk_stm32_gate_enable,
	.disable	= clk_stm32_gate_disable,
	.is_enabled	= clk_stm32_gate_is_enabled,
};

const struct stm32_clk_ops clk_fixed_factor_ops = {
	.recalc_rate	= fixed_factor_recalc_rate,
};

unsigned long fixed_factor_recalc_rate(struct stm32_clk_priv *priv,
				       int id, unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	const struct fixed_factor_cfg *cfg = clk->clock_cfg;
	unsigned long long rate;

	rate = (unsigned long long)prate * cfg->mult;

	if (cfg->div == 0U) {
		ERROR("division by zero\n");
		panic();
	}

	return (unsigned long)(rate / cfg->div);
};

#define APB_DIV_MASK	GENMASK_32(2, 0)
#define TIM_PRE_MASK	BIT_32(0)

static unsigned long timer_recalc_rate(struct stm32_clk_priv *priv,
				       int id, unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	const struct clk_timer_cfg *cfg = clk->clock_cfg;
	uint32_t prescaler, timpre;
	uintptr_t rcc_base = priv->base;

	prescaler = mmio_read_32(rcc_base + cfg->apbdiv) &
		APB_DIV_MASK;

	timpre = mmio_read_32(rcc_base + cfg->timpre) &
		TIM_PRE_MASK;

	if (prescaler == 0U) {
		return prate;
	}

	return prate * (timpre + 1U) * 2U;
};

const struct stm32_clk_ops clk_timer_ops = {
	.recalc_rate	= timer_recalc_rate,
};

static unsigned long clk_fixed_rate_recalc(struct stm32_clk_priv *priv, int id,
					   unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct clk_stm32_fixed_rate_cfg *cfg = clk->clock_cfg;

	return cfg->rate;
}

const struct stm32_clk_ops clk_stm32_fixed_rate_ops = {
	.recalc_rate	= clk_fixed_rate_recalc,
};

int stm32_clk_parse_fdt_by_name(void *fdt, int node, const char *name, uint32_t *tab, uint32_t *nb)
{
	const fdt32_t *cell;
	int len = 0;
	uint32_t i;

	cell = fdt_getprop(fdt, node, name, &len);
	if (cell == NULL) {
		*nb = 0U;
		return 0;
	}

	for (i = 0; i < ((uint32_t)len / sizeof(uint32_t)); i++) {
		uint32_t val = fdt32_to_cpu(cell[i]);

		tab[i] = val;
	}

	*nb = (uint32_t)len / sizeof(uint32_t);

	return 0;
}

int clk_stm32_init(struct stm32_clk_priv *priv, uintptr_t base)
{
	unsigned int i;

	stm32_clock_data = priv;

	priv->base = base;

	for (i = 0U; i < priv->num; i++) {
		const struct stm32_clk_ops *ops = _clk_get_ops(priv, i);

		if (ops->init != NULL) {
			ops->init(priv, i);
		}
	}

	stm32_clk_register();

	return 0;
}

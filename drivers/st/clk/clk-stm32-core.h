/*
 * Copyright (C) 2022-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef CLK_STM32_CORE_H
#define CLK_STM32_CORE_H

struct mux_cfg {
	uint16_t offset;
	uint8_t shift;
	uint8_t width;
	uint8_t bitrdy;
};

struct gate_cfg {
	uint16_t offset;
	uint8_t bit_idx;
	uint8_t set_clr;
};

struct clk_div_table {
	uint16_t val;
	uint16_t div;
};

struct div_cfg {
	const struct clk_div_table *table;
	uint16_t offset;
	uint8_t shift;
	uint8_t width;
	uint8_t flags;
	uint8_t bitrdy;
};

struct parent_cfg {
	const uint16_t *id_parents;
	struct mux_cfg *mux;
	uint8_t num_parents;
};

struct stm32_clk_priv;

struct stm32_clk_ops {
	unsigned long (*recalc_rate)(struct stm32_clk_priv *priv, int id, unsigned long rate);
	int (*get_parent)(struct stm32_clk_priv *priv, int id);
	int (*set_rate)(struct stm32_clk_priv *priv, int id, unsigned long rate,
			unsigned long prate);
	int (*enable)(struct stm32_clk_priv *priv, int id);
	void (*disable)(struct stm32_clk_priv *priv, int id);
	bool (*is_enabled)(struct stm32_clk_priv *priv, int id);
	void (*init)(struct stm32_clk_priv *priv, int id);
};

struct clk_stm32 {
	uint16_t binding;
	uint16_t parent;
	uint8_t ops;
	uint8_t flags;
	void *clock_cfg;
};

struct stm32_clk_priv {
	uintptr_t base;
	const uint32_t num;
	const struct clk_stm32 *clks;
	const struct parent_cfg *parents;
	const uint32_t nb_parents;
	const struct gate_cfg *gates;
	const uint32_t nb_gates;
	const struct div_cfg *div;
	const uint32_t nb_div;
	struct clk_oscillator_data *osci_data;
	const uint32_t nb_osci_data;
	uint8_t *gate_refcounts;
	void *pdata;
	const struct stm32_clk_ops **ops_array;
};

struct stm32_clk_bypass {
	uint16_t offset;
	uint8_t bit_byp;
	uint8_t bit_digbyp;
};

struct stm32_clk_css {
	uint16_t offset;
	uint8_t bit_css;
};

struct stm32_clk_drive {
	uint16_t offset;
	uint8_t drv_shift;
	uint8_t drv_width;
	uint8_t drv_default;
};

struct clk_oscillator_data {
	const char *name;
	struct stm32_clk_bypass *bypass;
	struct stm32_clk_css *css;
	struct stm32_clk_drive *drive;
	unsigned long frequency;
	uint16_t id_clk;
	uint16_t gate_id;
	uint16_t gate_rdy_id;

};

struct clk_gate_cfg {
	uint32_t offset;
	uint8_t bit_idx;
};

/* CLOCK FLAGS */
#define CLK_IS_CRITICAL			BIT(0)
#define CLK_IGNORE_UNUSED		BIT(1)
#define CLK_SET_RATE_PARENT		BIT(2)

#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)
#define CLK_DIVIDER_BIG_ENDIAN		BIT(7)

#define MUX_MAX_PARENTS			U(0x8000)
#define MUX_PARENT_MASK			GENMASK(14, 0)
#define MUX_FLAG			U(0x8000)
#define MUX(mux)			((mux) | MUX_FLAG)

#define NO_GATE				0
#define _NO_ID				UINT16_MAX
#define CLK_IS_ROOT			UINT16_MAX
#define MUX_NO_BIT_RDY			UINT8_MAX
#define DIV_NO_BIT_RDY			UINT8_MAX

#define MASK_WIDTH_SHIFT(_width, _shift) \
	GENMASK(((_width) + (_shift) - 1U), (_shift))

void clk_stm32_rcc_regs_lock(void);
void clk_stm32_rcc_regs_unlock(void);

int clk_stm32_init(struct stm32_clk_priv *priv, uintptr_t base);
void clk_stm32_enable_critical_clocks(void);

struct stm32_clk_priv *clk_stm32_get_priv(void);

int clk_get_index(struct stm32_clk_priv *priv, unsigned long binding_id);
const struct clk_stm32 *_clk_get(struct stm32_clk_priv *priv, int id);

void clk_oscillator_set_bypass(struct stm32_clk_priv *priv, int id, bool digbyp, bool bypass);
void clk_oscillator_set_drive(struct stm32_clk_priv *priv, int id, uint8_t lsedrv);
void clk_oscillator_set_css(struct stm32_clk_priv *priv, int id, bool css);

int _clk_stm32_gate_wait_ready(struct stm32_clk_priv *priv, uint16_t gate_id, bool ready_on);

int clk_oscillator_wait_ready(struct stm32_clk_priv *priv, int id, bool ready_on);
int clk_oscillator_wait_ready_on(struct stm32_clk_priv *priv, int id);
int clk_oscillator_wait_ready_off(struct stm32_clk_priv *priv, int id);

int clk_stm32_get_counter(unsigned long binding_id);

void _clk_stm32_gate_disable(struct stm32_clk_priv *priv, uint16_t gate_id);
int _clk_stm32_gate_enable(struct stm32_clk_priv *priv, uint16_t gate_id);

int _clk_stm32_set_parent(struct stm32_clk_priv *priv, int id, int src_id);
int _clk_stm32_set_parent_by_index(struct stm32_clk_priv *priv, int clk, int sel);

int _clk_stm32_get_parent(struct stm32_clk_priv *priv, int id);
int _clk_stm32_get_parent_by_index(struct stm32_clk_priv *priv, int clk_id, int idx);
int _clk_stm32_get_parent_index(struct stm32_clk_priv *priv, int clk_id);

unsigned long _clk_stm32_get_rate(struct stm32_clk_priv *priv, int id);
unsigned long _clk_stm32_get_parent_rate(struct stm32_clk_priv *priv, int id);

bool _stm32_clk_is_flags(struct stm32_clk_priv *priv, int id, uint8_t flag);

int _clk_stm32_enable(struct stm32_clk_priv *priv, int id);
void _clk_stm32_disable(struct stm32_clk_priv *priv, int id);

int clk_stm32_enable_call_ops(struct stm32_clk_priv *priv, uint16_t id);
void clk_stm32_disable_call_ops(struct stm32_clk_priv *priv, uint16_t id);

bool _clk_stm32_is_enabled(struct stm32_clk_priv *priv, int id);

int _clk_stm32_divider_set_rate(struct stm32_clk_priv *priv, int div_id,
				unsigned long rate, unsigned long parent_rate);

int clk_stm32_divider_set_rate(struct stm32_clk_priv *priv, int id, unsigned long rate,
			       unsigned long prate);

unsigned long _clk_stm32_divider_recalc(struct stm32_clk_priv *priv,
					int div_id,
					unsigned long prate);

unsigned long clk_stm32_divider_recalc(struct stm32_clk_priv *priv, int idx,
				       unsigned long prate);

int clk_stm32_gate_enable(struct stm32_clk_priv *priv, int idx);
void clk_stm32_gate_disable(struct stm32_clk_priv *priv, int idx);

bool _clk_stm32_gate_is_enabled(struct stm32_clk_priv *priv, int gate_id);
bool clk_stm32_gate_is_enabled(struct stm32_clk_priv *priv, int idx);

uint32_t clk_stm32_div_get_value(struct stm32_clk_priv *priv, int div_id);
int clk_stm32_set_div(struct stm32_clk_priv *priv, uint32_t div_id, uint32_t value);
int clk_mux_set_parent(struct stm32_clk_priv *priv, uint16_t pid, uint8_t sel);
int clk_mux_get_parent(struct stm32_clk_priv *priv, uint32_t mux_id);

int stm32_clk_parse_fdt_by_name(void *fdt, int node, const char *name, uint32_t *tab, uint32_t *nb);

#ifdef CFG_STM32_CLK_DEBUG
void clk_stm32_display_clock_info(void);
#endif

struct clk_stm32_div_cfg {
	uint8_t id;
};

#define STM32_DIV(idx, _binding, _parent, _flags, _div_id) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_binding),\
		.parent		=  (_parent),\
		.flags		= (_flags),\
		.clock_cfg	= &(struct clk_stm32_div_cfg){\
			.id	= (_div_id),\
		},\
		.ops		= STM32_DIVIDER_OPS,\
	}

struct clk_stm32_gate_cfg {
	uint8_t id;
};

#define STM32_GATE(idx, _binding, _parent, _flags, _gate_id) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_binding),\
		.parent		=  (_parent),\
		.flags		= (_flags),\
		.clock_cfg	= &(struct clk_stm32_gate_cfg){\
			.id	= (_gate_id),\
		},\
		.ops		= STM32_GATE_OPS,\
	}

struct fixed_factor_cfg {
	uint8_t mult;
	uint8_t div;
};

unsigned long fixed_factor_recalc_rate(struct stm32_clk_priv *priv,
				       int _idx, unsigned long prate);

#define FIXED_FACTOR(idx, _idx, _parent, _mult, _div) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_idx),\
		.parent		= (_parent),\
		.clock_cfg	= &(struct fixed_factor_cfg){\
			.mult	= (_mult),\
			.div	= (_div),\
		},\
		.ops		= FIXED_FACTOR_OPS,\
	}

#define GATE(idx, _binding, _parent, _flags, _offset, _bit_idx) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_binding),\
		.parent		=  (_parent),\
		.flags		= (_flags),\
		.clock_cfg	= &(struct clk_gate_cfg){\
			.offset		= (_offset),\
			.bit_idx	= (_bit_idx),\
		},\
		.ops		= GATE_OPS,\
	}

#define STM32_MUX(idx, _binding, _mux_id, _flags) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_binding),\
		.parent		= (MUX(_mux_id)),\
		.flags		= (_flags),\
		.clock_cfg	= NULL,\
		.ops		= STM32_MUX_OPS\
	}

struct clk_timer_cfg {
	uint32_t apbdiv;
	uint32_t timpre;
};

#define CK_TIMER(idx, _idx, _parent, _flags, _apbdiv, _timpre) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_idx),\
		.parent		= (_parent),\
		.flags		= (CLK_SET_RATE_PARENT | (_flags)),\
		.clock_cfg	= &(struct clk_timer_cfg){\
			.apbdiv = (_apbdiv),\
			.timpre = (_timpre),\
		},\
		.ops		= STM32_TIMER_OPS,\
	}

struct clk_stm32_fixed_rate_cfg {
	unsigned long rate;
};

#define CLK_FIXED_RATE(idx, _binding, _rate) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_binding),\
		.parent		= (CLK_IS_ROOT),\
		.clock_cfg	= &(struct clk_stm32_fixed_rate_cfg){\
			.rate	= (_rate),\
		},\
		.ops		= STM32_FIXED_RATE_OPS,\
	}

#define BYPASS(_offset, _bit_byp, _bit_digbyp) &(struct stm32_clk_bypass){\
	.offset		= (_offset),\
	.bit_byp	= (_bit_byp),\
	.bit_digbyp	= (_bit_digbyp),\
}

#define CSS(_offset, _bit_css)	&(struct stm32_clk_css){\
	.offset		= (_offset),\
	.bit_css	= (_bit_css),\
}

#define DRIVE(_offset, _shift, _width, _default) &(struct stm32_clk_drive){\
	.offset		= (_offset),\
	.drv_shift	= (_shift),\
	.drv_width	= (_width),\
	.drv_default	= (_default),\
}

#define OSCILLATOR(idx_osc, _id, _name, _gate_id, _gate_rdy_id, _bypass, _css, _drive) \
	[(idx_osc)] = (struct clk_oscillator_data){\
		.name		= (_name),\
		.id_clk		= (_id),\
		.gate_id	= (_gate_id),\
		.gate_rdy_id	= (_gate_rdy_id),\
		.bypass		= (_bypass),\
		.css		= (_css),\
		.drive		= (_drive),\
	}

struct clk_oscillator_data *clk_oscillator_get_data(struct stm32_clk_priv *priv, int id);

void clk_stm32_osc_init(struct stm32_clk_priv *priv, int id);
bool clk_stm32_osc_gate_is_enabled(struct stm32_clk_priv *priv, int id);
int clk_stm32_osc_gate_enable(struct stm32_clk_priv *priv, int id);
void clk_stm32_osc_gate_disable(struct stm32_clk_priv *priv, int id);

struct stm32_osc_cfg {
	uint8_t osc_id;
};

#define CLK_OSC(idx, _idx, _parent, _osc_id) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_idx),\
		.parent		= (_parent),\
		.flags		= CLK_IS_CRITICAL,\
		.clock_cfg	= &(struct stm32_osc_cfg){\
			.osc_id = (_osc_id),\
		},\
		.ops		= STM32_OSC_OPS,\
	}

#define CLK_OSC_FIXED(idx, _idx, _parent, _osc_id) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_idx),\
		.parent		= (_parent),\
		.flags		= CLK_IS_CRITICAL,\
		.clock_cfg	= &(struct stm32_osc_cfg){\
			.osc_id	= (_osc_id),\
		},\
		.ops		= STM32_OSC_NOGATE_OPS,\
	}

extern const struct stm32_clk_ops clk_mux_ops;
extern const struct stm32_clk_ops clk_stm32_divider_ops;
extern const struct stm32_clk_ops clk_stm32_gate_ops;
extern const struct stm32_clk_ops clk_fixed_factor_ops;
extern const struct stm32_clk_ops clk_gate_ops;
extern const struct stm32_clk_ops clk_timer_ops;
extern const struct stm32_clk_ops clk_stm32_fixed_rate_ops;
extern const struct stm32_clk_ops clk_stm32_osc_ops;
extern const struct stm32_clk_ops clk_stm32_osc_nogate_ops;

enum {
	NO_OPS,
	FIXED_FACTOR_OPS,
	GATE_OPS,
	STM32_MUX_OPS,
	STM32_DIVIDER_OPS,
	STM32_GATE_OPS,
	STM32_TIMER_OPS,
	STM32_FIXED_RATE_OPS,
	STM32_OSC_OPS,
	STM32_OSC_NOGATE_OPS,

	STM32_LAST_OPS
};

#endif /* CLK_STM32_CORE_H */

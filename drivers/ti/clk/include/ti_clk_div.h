/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Divider API Header
 *
 * This header defines the data structures and interfaces for programmable
 * clock dividers. It provides divider data structures, driver interfaces,
 * and APIs for divider value get/set operations supporting both power-of-2
 * and arbitrary integer division ratios.
 */

#ifndef TI_CLK_DIV_H
#define TI_CLK_DIV_H

#include <ti_clk.h>

/*
 * Base divider clock data structure
 *
 * This structure contains the fundamental divider configuration including
 * the maximum divider value and default divider ratio.
 */
struct ti_clk_data_div {
	/* Base driver data structure */
	struct ti_clk_drv_data data;
	/* Maximum divider value (divider range is 1 to max_div) */
	uint16_t max_div;
	/* Default divider value at initialization */
	uint16_t default_div;
};

/*
 * Register-based divider clock data structure
 *
 * This structure defines a divider implemented via a hardware register field.
 * The divider value is stored in a bit field within the specified register.
 */
struct ti_clk_data_div_reg {
	/* Register address containing divider field */
	uint32_t reg;
	/* Base divider data */
	struct ti_clk_data_div data_div;
	/* Starting bit position of divider field */
	uint8_t bit;
	/* If true, divider values start at 1 (not 0) */
	uint8_t start_at_1;
};

/*
 * Register-based divider with GO bit for change propagation
 *
 * This structure extends the basic register divider with a GO bit mechanism.
 * After modifying the divider value, the GO bit must be set to propagate
 * the change through the clock tree.
 */
struct ti_clk_data_div_reg_go {
	/* Register address containing divider field */
	uint32_t reg;
	/* Base divider data */
	struct ti_clk_data_div data_div;
	/* Starting bit position of divider field */
	uint8_t bit;
	/* If true, divider values start at 1 (not 0) */
	uint8_t start_at_1;
	/* GO bit number to trigger divider change */
	uint8_t go;
};

/*
 * Divider clock driver operations structure
 *
 * This structure extends the base clock driver with divider-specific operations
 * for getting, setting, and validating divider values.
 */
struct ti_clk_drv_div {
	/* Base clock driver operations */
	struct ti_clk_drv drv;

	/*
	 * Set the divider value for a clock
	 */
	bool (*set_div)(struct ti_clk *clkp, uint32_t div);

	/*
	 * Get the current divider value for a clock
	 */
	uint32_t (*get_div)(struct ti_clk *clkp);

	/*
	 * Validate if a divider value is supported
	 */
	bool (*valid_div)(struct ti_clk *clkp, uint32_t div);
};

/* Divider driver for read-write register-based dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg;

/* Divider driver for read-only register-based dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg_ro;

/* Divider driver for register-based dividers with GO bit propagation */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg_go;

/* Divider driver for fixed (non-programmable) dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_fixed;

/**
 * ti_clk_div_set_freq() - Sets the frequency of a divider clock
 * @clkp: The divider clock to configure
 * @target_hz: Target frequency in Hz
 * @min_hz: Minimum acceptable frequency in Hz
 * @max_hz: Maximum acceptable frequency in Hz
 * @changed: Output parameter set to true if frequency changed
 *
 * This function adjusts the divider value to achieve the target frequency.
 * It can also propagate frequency changes to the parent if allowed.
 *
 * Return: Best achievable frequency in Hz, 0 on failure
 */
uint32_t ti_clk_div_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			     uint32_t min_hz, uint32_t max_hz,
			     bool *changed);

/**
 * ti_clk_div_get_freq() - Gets the current frequency of a divider clock
 * @clkp: The divider clock to query
 *
 * Return: Current frequency in Hz
 */
uint32_t ti_clk_div_get_freq(struct ti_clk *clkp);

/**
 * ti_clk_div_set_freq_static_parent() - Sets frequency for dividers with fixed parent frequency
 * @clkp: The divider clock to configure
 * @target_hz: Target frequency in Hz
 * @min_hz: Minimum acceptable frequency in Hz
 * @max_hz: Maximum acceptable frequency in Hz
 * @changed: Output parameter set to true if frequency changed
 *
 * This variant is optimized for dividers whose parent frequency cannot
 * be changed. It only adjusts the divider value itself.
 *
 * Return: Best achievable frequency in Hz, 0 on failure
 */
uint32_t ti_clk_div_set_freq_static_parent(struct ti_clk *clkp, uint32_t target_hz,
					   uint32_t min_hz, uint32_t max_hz,
					   bool *changed);

/**
 * ti_clk_div_reg_go_get_div() - Gets divider value for GO-bit register divider
 * @clkp: The divider clock to query
 *
 * Return: Current divider value
 */
uint32_t ti_clk_div_reg_go_get_div(struct ti_clk *clkp);

/**
 * ti_clk_div_reg_go_set_div() - Sets divider value for GO-bit register divider
 * @clkp: The divider clock to modify
 * @div: The divider value to set
 *
 * Sets the divider value and triggers the GO bit to propagate the change.
 *
 * Return: True if successful
 */
bool ti_clk_div_reg_go_set_div(struct ti_clk *clkp, uint32_t div);

/**
 * ti_clk_div_reg_get_div() - Gets divider value from register-based divider
 * @clkp: The divider clock to query
 *
 * Return: Current divider value
 */
uint32_t ti_clk_div_reg_get_div(struct ti_clk *clkp);

/**
 * ti_clk_div_reg_set_div() - Sets divider value for register-based divider
 * @clkp: The divider clock to modify
 * @div: The divider value to set
 *
 * Return: True if successful
 */
bool ti_clk_div_reg_set_div(struct ti_clk *clkp, uint32_t div);

/**
 * ti_clk_get_div() - Generic divider value getter
 * @clkp: The divider clock to query
 *
 * Dispatches to the appropriate driver-specific get_div function.
 *
 * Return: Current divider value
 */
uint32_t ti_clk_get_div(struct ti_clk *clkp);

/**
 * ti_clk_div_init() - Initializes a divider clock
 * @clkp: The divider clock to initialize
 *
 * Performs initialization including setting default divider value and
 * configuring hardware registers.
 *
 * Return: 0 on success, error code otherwise
 */
int32_t ti_clk_div_init(struct ti_clk *clkp);

#endif /* TI_CLK_DIV_H */

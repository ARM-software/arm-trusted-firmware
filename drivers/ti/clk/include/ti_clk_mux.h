/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Multiplexer API Header
 *
 * This header defines the data structures and interfaces for clock
 * multiplexers. It provides mux data structures, parent selection
 * interfaces, and driver operations for selecting between multiple
 * parent clock sources with optional per-parent dividers.
 */

#ifndef TI_CLK_MUX_H
#define TI_CLK_MUX_H

#include <ti_clk.h>

/*
 * Base multiplexer clock data structure
 *
 * This structure contains the fundamental multiplexer configuration including
 * the number of parent options and the parent selection table.
 */
struct ti_clk_data_mux {
	/* Base driver data structure */
	struct ti_clk_drv_data data;
	/* Number of parent clock options */
	uint32_t num_parents;
	/* Array of parent clock descriptors */
	const struct ti_clk_parent *parents;
};

/*
 * Register-based multiplexer clock data structure
 *
 * This structure defines a multiplexer implemented via a hardware register field.
 * The parent selection index is stored in a bit field within the specified register.
 */
struct ti_clk_data_mux_reg {
	/* Base multiplexer data */
	struct ti_clk_data_mux data_mux;
	/* Register addr containing mux selection field */
	uint32_t reg;
	/* Starting bit position of mux selection field */
	uint8_t bit;
};

/*
 * Multiplexer clock driver operations structure
 *
 * This structure extends the base clock driver with multiplexer-specific
 * operations for getting and setting the parent clock selection.
 */
struct ti_clk_drv_mux {
	/* Base clock driver operations */
	struct ti_clk_drv drv;

	/*
	 * Set the parent clock for a multiplexer
	 */
	bool (*set_parent)(struct ti_clk *clkp, uint8_t parent_idx);

	/*
	 * Get the current parent clock for a multiplexer
	 */
	const struct ti_clk_parent *(*get_parent)(struct ti_clk *clkp);
};

/* Multiplexer driver for read-only register-based muxes */
extern const struct ti_clk_drv_mux ti_clk_drv_mux_reg_ro;

/* Multiplexer driver for read-write register-based muxes */
extern const struct ti_clk_drv_mux ti_clk_drv_mux_reg;

/*
 * Gets the current parent clock of a multiplexer
 *
 * Dispatches to the appropriate driver-specific get_parent function to
 * retrieve the currently selected parent clock descriptor.
 */
const struct ti_clk_parent *ti_clk_mux_get_parent(struct ti_clk *clkp);

/**
 * ti_clk_mux_set_parent() - Sets the parent clock for a multiplexer
 * @clkp: The multiplexer clock to modify
 * @new_parent: Index of the new parent to select
 *
 * Changes the multiplexer's parent selection to the specified parent index.
 * This may involve writing to hardware registers and updating clock frequencies.
 *
 * Return: True if the parent was successfully changed
 */
bool ti_clk_mux_set_parent(struct ti_clk *clkp, uint8_t new_parent);

#endif /* TI_CLK_MUX_H */

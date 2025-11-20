/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
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

#ifndef CLK_MUX_H
#define CLK_MUX_H

#include <ti_clk.h>

/**
 * \brief Base multiplexer clock data structure
 *
 * This structure contains the fundamental multiplexer configuration including
 * the number of parent options and the parent selection table.
 */
struct ti_clk_data_mux {
	struct ti_clk_drv_data data;		/** Base driver data structure */
	uint32_t num_parents;			/** Number of parent clock options */
	const struct ti_clk_parent *parents;	/** Array of parent clock descriptors */
};

/**
 * \brief Register-based multiplexer clock data structure
 *
 * This structure defines a multiplexer implemented via a hardware register field.
 * The parent selection index is stored in a bit field within the specified register.
 */
struct ti_clk_data_mux_reg {
	struct ti_clk_data_mux data_mux;	/** Base multiplexer data */
	uint32_t reg;				/** Register addr containing mux selection field */
	uint8_t bit;				/** Starting bit position of mux selection field */
};

/**
 * \brief Multiplexer clock driver operations structure
 *
 * This structure extends the base clock driver with multiplexer-specific
 * operations for getting and setting the parent clock selection.
 */
struct ti_clk_drv_mux {
	struct ti_clk_drv drv;		/** Base clock driver operations */

	/**
	 * \brief Set the parent clock for a multiplexer
	 *
	 * \param clkp The multiplexer clock to modify
	 * \param parent_idx The parent index to select
	 *
	 * \return True if the parent was successfully set
	 */
	bool (*set_parent)(struct ti_clk *clkp, uint8_t parent_idx);

	/**
	 * \brief Get the current parent clock for a multiplexer
	 *
	 * \param clkp The multiplexer clock to query
	 *
	 * \return Pointer to the current parent clock descriptor
	 */
	const struct ti_clk_parent * (*get_parent)(struct ti_clk *clkp);
};

/** Multiplexer driver for read-only register-based muxes */
extern const struct ti_clk_drv_mux ti_clk_drv_mux_reg_ro;

/** Multiplexer driver for read-write register-based muxes */
extern const struct ti_clk_drv_mux ti_clk_drv_mux_reg;

/**
 * \brief Gets the current parent clock of a multiplexer
 *
 * Dispatches to the appropriate driver-specific get_parent function to
 * retrieve the currently selected parent clock descriptor.
 *
 * \param clkp The multiplexer clock to query
 *
 * \return Pointer to the current parent clock descriptor
 */
const struct ti_clk_parent *ti_clk_mux_get_parent(struct ti_clk *clkp);

/**
 * \brief Sets the parent clock for a multiplexer
 *
 * Changes the multiplexer's parent selection to the specified parent index.
 * This may involve writing to hardware registers and updating clock frequencies.
 *
 * \param clkp The multiplexer clock to modify
 * \param new_parent Index of the new parent to select
 *
 * \return True if the parent was successfully changed
 */
bool ti_clk_mux_set_parent(struct ti_clk *clkp, uint8_t new_parent);

#endif

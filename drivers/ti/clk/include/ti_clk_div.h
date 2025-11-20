/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
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

#ifndef CLK_DIV_H
#define CLK_DIV_H

#include <ti_clk.h>

/**
 * \brief Base divider clock data structure
 *
 * This structure contains the fundamental divider configuration including
 * the maximum divider value and default divider ratio.
 */
struct ti_clk_data_div {
	struct ti_clk_drv_data data;	/** Base driver data structure */
	uint16_t max_div;		/** Maximum divider value (divider range is 1 to max_div) */
	uint16_t default_div;		/** Default divider value at initialization */
};

/**
 * \brief Register-based divider clock data structure
 *
 * This structure defines a divider implemented via a hardware register field.
 * The divider value is stored in a bit field within the specified register.
 */
struct ti_clk_data_div_reg {
	uint32_t reg;			/** Register address containing divider field */
	struct ti_clk_data_div data_div;	/** Base divider data */
	uint8_t bit;			/** Starting bit position of divider field */
	uint8_t start_at_1;		/** If true, divider values start at 1 (not 0) */
};

/**
 * \brief Register-based divider with GO bit for change propagation
 *
 * This structure extends the basic register divider with a GO bit mechanism.
 * After modifying the divider value, the GO bit must be set to propagate
 * the change through the clock tree.
 */
struct ti_clk_data_div_reg_go {
	uint32_t reg;			/** Register address containing divider field */
	struct ti_clk_data_div data_div;	/** Base divider data */
	uint8_t bit;			/** Starting bit position of divider field */
	uint8_t start_at_1;		/** If true, divider values start at 1 (not 0) */
	uint8_t go;			/** GO bit number to trigger divider change */
};

/**
 * \brief Divider clock driver operations structure
 *
 * This structure extends the base clock driver with divider-specific operations
 * for getting, setting, and validating divider values.
 */
struct ti_clk_drv_div {
	struct ti_clk_drv drv;		/** Base clock driver operations */

	/**
	 * \brief Set the divider value for a clock
	 *
	 * \param clkp The clock to modify
	 * \param div The divider value to set
	 *
	 * \return True if the divider was successfully set
	 */
	bool (*set_div)(struct ti_clk *clkp, uint32_t div);

	/**
	 * \brief Get the current divider value for a clock
	 *
	 * \param clkp The clock to query
	 *
	 * \return The current divider value
	 */
	uint32_t (*get_div)(struct ti_clk *clkp);

	/**
	 * \brief Validate if a divider value is supported
	 *
	 * \param clkp The clock to check
	 * \param div The divider value to validate
	 *
	 * \return True if the divider value is valid for this clock
	 */
	bool (*valid_div)(struct ti_clk *clkp, uint32_t div);
};

/** Divider driver for read-write register-based dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg;

/** Divider driver for read-only register-based dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg_ro;

/** Divider driver for register-based dividers with GO bit propagation */
extern const struct ti_clk_drv_div ti_clk_drv_div_reg_go;

/** Divider driver for fixed (non-programmable) dividers */
extern const struct ti_clk_drv_div ti_clk_drv_div_fixed;

/**
 * \brief Notifies a divider clock of parent frequency changes
 *
 * This function is called when a divider's parent clock frequency changes.
 * It validates or applies the frequency change.
 *
 * \param clkp The divider clock to notify
 * \param parent_freq_hz New parent frequency in Hz
 * \param query True to validate only, false to apply the change
 *
 * \return True if the change is acceptable or was successfully applied
 */
bool ti_clk_div_notify_freq(struct ti_clk *clkp, uint32_t parent_freq_hz, bool query);

/**
 * \brief Sets the frequency of a divider clock
 *
 * This function adjusts the divider value to achieve the target frequency.
 * It can also propagate frequency changes to the parent if allowed.
 *
 * \param clkp The divider clock to configure
 * \param target_hz Target frequency in Hz
 * \param min_hz Minimum acceptable frequency in Hz
 * \param max_hz Maximum acceptable frequency in Hz
 * \param query True to query only without applying changes
 * \param changed Output parameter set to true if frequency changed
 *
 * \return Best achievable frequency in Hz, 0 on failure
 */
uint32_t ti_clk_div_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			     uint32_t min_hz, uint32_t max_hz,
			     bool query, bool *changed);

/**
 * \brief Gets the current frequency of a divider clock
 *
 * \param clkp The divider clock to query
 *
 * \return Current frequency in Hz
 */
uint32_t ti_clk_div_get_freq(struct ti_clk *clkp);

/**
 * \brief Sets frequency for dividers with fixed parent frequency
 *
 * This variant is optimized for dividers whose parent frequency cannot
 * be changed. It only adjusts the divider value itself.
 *
 * \param clkp The divider clock to configure
 * \param target_hz Target frequency in Hz
 * \param min_hz Minimum acceptable frequency in Hz
 * \param max_hz Maximum acceptable frequency in Hz
 * \param query True to query only without applying changes
 * \param changed Output parameter set to true if frequency changed
 *
 * \return Best achievable frequency in Hz, 0 on failure
 */
uint32_t ti_clk_div_set_freq_static_parent(struct ti_clk *clkp, uint32_t target_hz,
					    uint32_t min_hz, uint32_t max_hz,
					    bool query, bool *changed);

/**
 * \brief Gets divider value for GO-bit register divider
 *
 * \param clkp The divider clock to query
 *
 * \return Current divider value
 */
uint32_t ti_clk_div_reg_go_get_div(struct ti_clk *clkp);

/**
 * \brief Sets divider value for GO-bit register divider
 *
 * Sets the divider value and triggers the GO bit to propagate the change.
 *
 * \param clkp The divider clock to modify
 * \param div The divider value to set
 *
 * \return True if successful
 */
bool ti_clk_div_reg_go_set_div(struct ti_clk *clkp, uint32_t div);

/**
 * \brief Gets divider value from register-based divider
 *
 * \param clkp The divider clock to query
 *
 * \return Current divider value
 */
uint32_t ti_clk_div_reg_get_div(struct ti_clk *clkp);

/**
 * \brief Sets divider value for register-based divider
 *
 * \param clkp The divider clock to modify
 * \param div The divider value to set
 *
 * \return True if successful
 */
bool ti_clk_div_reg_set_div(struct ti_clk *clkp, uint32_t div);

/**
 * \brief Generic divider value getter
 *
 * Dispatches to the appropriate driver-specific get_div function.
 *
 * \param clkp The divider clock to query
 *
 * \return Current divider value
 */
uint32_t ti_clk_get_div(struct ti_clk *clkp);

/**
 * \brief Initializes a divider clock
 *
 * Performs initialization including setting default divider value and
 * configuring hardware registers.
 *
 * \param clkp The divider clock to initialize
 *
 * \return 0 on success, error code otherwise
 */
int32_t ti_clk_div_init(struct ti_clk *clkp);

#endif

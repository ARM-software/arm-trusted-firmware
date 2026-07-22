/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Framework API Header
 *
 * This header defines the core clock framework data structures and API
 * functions. It provides clock data types, clock driver interfaces, parent-
 * child relationships, frequency ranges, and the main API for clock
 * operations including get/put, frequency control, and state management.
 */

#ifndef TI_CLK_H
#define TI_CLK_H

#include <assert.h>
#include <stddef.h>

#include <common/debug.h>
#include <lib/utils_def.h>

#include <ti_pm_types.h>

/*
 * Generates a 32-bit mask that covers all set bits in a number.
 * number must be non-zero; __builtin_clz(0) is undefined behavior.
 */
static inline uint32_t ti_mask_cover_for_number(uint32_t number)
{
	assert(number != 0U);
	return (uint32_t)((1UL << (32U - (uint32_t)__builtin_clz(number))) - 1UL);
}

#define TI_MASK_COVER_FOR_NUMBER(number) ti_mask_cover_for_number(number)

/*
 * Initializes a clock range entry with min and max frequencies
 */
#define TI_CLK_RANGE(id, min, max)[(id)] =            \
		{ .min_hz = (uint32_t) (min),         \
		  .max_hz = (uint32_t) (max) }

/*
 * Initializes a clock default frequency entry with min, target, and max
 */
#define TI_CLK_DEFAULT(id, min, target, max)[(id)] =		\
		{ .min_hz = (uint32_t) (min),			\
		  .target_hz = (uint32_t) (target),		\
		  .max_hz = (uint32_t) (max) }

/* Clock type identifier for divider clocks */
#define TI_CLK_TYPE_DIV	1U

/* Clock type identifier for multiplexer clocks */
#define TI_CLK_TYPE_MUX	2U

/* Allows clock to modify its parent's frequency */
#define TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ	BIT(0)

/* Skips hardware reinitialization during clock setup */
#define TI_CLK_DATA_FLAG_NO_HW_REINIT		BIT(2)

/* Clock should be enabled during power-up */
#define TI_CLK_FLAG_PWR_UP_EN			((uint8_t) BIT(0))

/* Clock has been initialized */
#define TI_CLK_FLAG_INITIALIZED			((uint8_t) BIT(2))

/*
 * Clock has a cached frequency value stored in soc_clock_values[] array.
 * Used for clocks like PLLs where the frequency is programmed and stored
 * rather than derived from parent clocks or hardware registers.
 */
#define TI_CLK_FLAG_CACHED				((uint8_t) BIT(3))

/* Clock hardware is disabled */
#define TI_CLK_HW_STATE_DISABLED	0U

/* Clock hardware is enabled */
#define TI_CLK_HW_STATE_ENABLED	1U

/* Clock hardware is in transition state */
#define TI_CLK_HW_STATE_TRANSITION	2U

/*
 * Defines a frequency range with minimum and maximum values
 */
struct ti_clk_range {
	/* Minimum frequency in Hz */
	uint32_t min_hz;
	/* Maximum frequency in Hz */
	uint32_t max_hz;
};

/*
 * Defines a default frequency configuration with min, target, and max
 */
struct ti_clk_default {
	/* Minimum acceptable frequency in Hz */
	uint32_t min_hz;
	/* Target frequency in Hz */
	uint32_t target_hz;
	/* Maximum acceptable frequency in Hz */
	uint32_t max_hz;
};

/*
 * Represents a clock's parent and associated divider
 */
struct ti_clk_parent {
	/* Parent clock ID */
	uint16_t clk;
	/* Divider value applied to parent frequency */
	uint8_t div;
};

/*
 * Clock driver-specific data (placeholder for extensibility)
 */
struct ti_clk_drv_data {
};

/*
 * Clock structure combining runtime state and configuration
 *
 * This structure merges what was previously ti_clk (runtime) and ti_clk_data
 * (configuration). Since everything runs from RAM on this platform, there's no
 * benefit to separating them into parallel arrays.
 */
struct ti_clk {
	/* Runtime state - modified during operation */
	/* Reference count for clock usage */
	uint8_t ref_count;
	/* Runtime flags (TI_CLK_FLAG_*) */
	uint8_t flags;

	/* Configuration - set at initialization, read-only afterwards */
	/* Clock driver operations */
	const struct ti_clk_drv *drv;
	/* Driver-specific data */
	const struct ti_clk_drv_data *data;
	/* Parent clock and divider */
	const struct ti_clk_parent parent;
	/* soc_clock_values[] index for cached clocks */
	const uint16_t freq_idx;
	/* Index into shared range table */
	const uint8_t range_idx;
	/* Clock type (TI_CLK_TYPE_*) */
	const uint8_t type;
	/* Configuration flags (TI_CLK_DATA_FLAG_*) */
	const uint8_t data_flags;
};

struct ti_clk_drv {
	/* Perform any necessary initialization */
	int32_t (*init)(struct ti_clk *clkp);

	/*
	 * Set the current state of a clock.
	 */
	bool (*set_state)(struct ti_clk *clkp, bool enabled);

	/*
	 * Get the current state of a clock.
	 */
	uint32_t (*get_state)(struct ti_clk *clkp);

	/*
	 * Program a clock to run at a given frequency. The minimum
	 * and maximum frequency parameters allow the best nearest
	 * match to be chosen. The clock tree is walked to accomplish
	 * this.
	 */
	uint32_t (*set_freq)(struct ti_clk *clkp, uint32_t target_hz,
			     uint32_t min_hz, uint32_t max_hz,
			     bool *changed);

	/*
	 * Return the frequency this clock runs at.
	 */
	uint32_t (*get_freq)(struct ti_clk *clkp);

};

/* The table of clock structures */
extern struct ti_clk soc_clocks[];
extern const size_t soc_clock_count;

/* The table of shared const clock ranges */
extern const struct ti_clk_range soc_clock_ranges[];
extern const size_t soc_clock_range_count;

/* The table of default frequencies */
extern const struct ti_clk_default soc_clock_freq_defaults[];
extern const size_t soc_clock_freq_defaults_count;

/*
 * The table of cached clock frequency values.
 * Some clocks (e.g., PLLs) store their programmed frequency in this array
 * indexed by clk->freq_idx, rather than deriving it from parent clocks or
 * hardware registers. Clocks using this mechanism have TI_CLK_FLAG_CACHED set.
 */
extern uint32_t soc_clock_values[];
extern const size_t soc_clock_value_count;

/**
 * ti_clk_value_set_freq() - Sets frequency for clocks with cached frequency values
 * @clkp: The clock to modify (must have valid freq_idx)
 * @target_hz: Target frequency in Hz
 * @min_hz: Minimum acceptable frequency
 * @max_hz: Maximum acceptable frequency
 * @changed: Output parameter indicating if frequency changed
 *
 * Used by clocks that store their frequency in soc_clock_values[] array
 * (e.g., PLLs). The frequency is validated against min/max range and stored
 * at the index specified by clk->freq_idx.
 *
 * Return: Target frequency in Hz on success, 0 on failure
 */
uint32_t ti_clk_value_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			       uint32_t min_hz, uint32_t max_hz,
			       bool *changed);

/**
 * ti_clk_value_get_freq() - Gets the cached frequency value for a clock
 * @clkp: The clock to query (must have valid freq_idx)
 *
 * Retrieves the frequency stored in soc_clock_values[] array for clocks
 * that cache their frequency (e.g., PLLs). The value is read from the index
 * specified by clk->freq_idx.
 *
 * Return: The cached frequency in Hz, or 0 if freq_idx is out of bounds
 */
uint32_t ti_clk_value_get_freq(struct ti_clk *clkp);

/**
 * ti_clk_get_parent_freq() - Gets the parent clock's current frequency
 * @clkp: The clock whose parent frequency to retrieve
 *
 * Return: Parent frequency in Hz
 */
uint32_t ti_clk_get_parent_freq(struct ti_clk *clkp);

/**
 * ti_clk_generic_set_freq_parent() - Generic frequency setter that adjusts parent frequency
 * @clkp: The clock to modify
 * @parent: The parent clock
 * @target_hz: Target frequency in Hz
 * @min_hz: Minimum acceptable frequency
 * @max_hz: Maximum acceptable frequency
 * @changed: Output parameter indicating if frequency changed
 * @div: Divider from parent to child (child = parent/div).
 *        Used to translate child frequency to parent frequency.
 *
 * Return: Best frequency found in Hz, 0 on failure
 */
uint32_t ti_clk_generic_set_freq_parent(struct ti_clk *clkp, struct ti_clk *parent,
					uint32_t target_hz, uint32_t min_hz,
					uint32_t max_hz,
					bool *changed, uint32_t div);

/**
 * ti_clk_set_freq() - Sets a clock's frequency
 * @clkp: The clock to modify
 * @target_hz: Target frequency in Hz
 * @min_hz: Minimum acceptable frequency
 * @max_hz: Maximum acceptable frequency
 * @changed: Output parameter indicating if frequency changed
 *
 * Return: Best frequency found in Hz, 0 on failure
 */
uint32_t ti_clk_set_freq(struct ti_clk *clkp, uint32_t target_hz, uint32_t min_hz,
			 uint32_t max_hz, bool *changed);

/**
 * ti_clk_get_freq() - Gets a clock's current frequency
 * @clkp: The clock to query
 *
 * Return: Current frequency in Hz
 */
uint32_t ti_clk_get_freq(struct ti_clk *clkp);

/**
 * ti_clk_get_state() - Gets a clock's hardware state
 * @clkp: The clock to query
 *
 * Return: Clock state (TI_CLK_HW_STATE_*)
 */
uint32_t ti_clk_get_state(struct ti_clk *clkp);

/**
 * ti_clk_set_state() - Sets a clock's hardware state (enable/disable)
 * @clkp: The clock to modify
 * @enable: True to enable, false to disable
 *
 * Return: True if successful
 */
bool ti_clk_set_state(struct ti_clk *clkp, bool enable);

/**
 * ti_clk_get() - Increments a clock's reference count and enables it
 * @clkp: The clock to get
 *
 * Return: True if successful
 */
bool ti_clk_get(struct ti_clk *clkp);

/**
 * ti_clk_put() - Decrements a clock's reference count and disables if zero
 * @clkp: The clock to put
 */
void ti_clk_put(struct ti_clk *clkp);

/**
 * ti_clk_init() - Initializes the TI clock framework
 *
 * Performs one pass of clock initialization. Clocks are processed in array
 * order. Each clock is skipped if its parent clock is not yet initialized,
 * allowing the function to handle parent-child dependencies.
 *
 * Design for arbitrary clock ordering:
 * - The function can be called multiple times to initialize clocks with
 *   multi-level dependencies
 * - Each call initializes clocks whose parent dependencies are now met
 * - Returns 0 if progress was made, -EAGAIN if no progress possible
 *
 * Current implementation:
 * - The clock table is topologically sorted (parents before children)
 * - A single call initializes all clocks
 * - Multiple calls are not necessary but are safe (subsequent calls
 *   detect all clocks initialized and return 0 quickly)
 *
 * Return: 0 on success (clocks initialized or no clocks need init),
 *         -EAGAIN if no progress made (circular dependency - fatal),
 *         -ENODEV if a power-up clock failed to enable,
 *         or other negative error code on failure
 */
int32_t ti_clk_init(void);

/**
 * ti_clk_drop_pwr_up_en() - Clears power-up enable flag on all clocks
 */
void ti_clk_drop_pwr_up_en(void);

/**
 * ti_clk_id_valid() - Determine if a clock ID is valid.
 * @id: The internal clock API ID.
 *
 * Return: True if the clock ID refers to a valid SoC clock, false otherwise
 */
static inline bool ti_clk_id_valid(ti_clk_idx_t id)
{
	return (id < soc_clock_count) && (soc_clocks[id].drv != NULL);
}

/*
 * Lookup an internal clock based on a clock ID
 *
 * This does a lookup of a clock based on the clock ID.
 */
static inline struct ti_clk *ti_clk_lookup(ti_clk_idx_t id)
{
	return ti_clk_id_valid(id) ? (soc_clocks + id) : NULL;
}

/*
 * Return the clock ID based on a clock pointer.
 *
 * The clock pointer is just an index into the array of clocks. This is
 * used to return a clock ID. This function has no error checking for NULL
 * pointers.
 */
static inline ti_clk_idx_t ti_clk_id(struct ti_clk *clkp)
{
	return (ti_clk_idx_t) (clkp - soc_clocks);
}

/*
 * Return a shared clock range with bounds checking.
 *
 * The clock infrastructure contains a set of shared clock ranges. Many
 * clocks and PLLs have min/max ranges. However, these min/max ranges can
 * take up a significant amount of storage. As many clocks share the same
 * ranges, we use a shared table of ranges. The clocks can then index that
 * table with a small uint8_t.
 */
static inline const struct ti_clk_range *ti_clk_get_range(ti_clk_idx_t idx)
{
	if (idx >= soc_clock_range_count) {
		return NULL;
	}
	return soc_clock_ranges + idx;
}

/**
 * ti_clk_set_value() - Set a cached clock frequency value with bounds checking.
 * @idx: The index into soc_clock_values[] array (from clk->freq_idx).
 * @value_hz: The frequency value in Hz to cache.
 *
 * Stores a frequency value in the soc_clock_values[] array for clocks that
 * cache their frequency (e.g., PLLs with TI_CLK_FLAG_CACHED set). Validates
 * the index is within array bounds before writing.
 *
 * Return: True if the value was cached successfully, false if index is out of bounds.
 */
static inline bool ti_clk_set_value(uint16_t idx, uint32_t value_hz)
{
	if (idx >= soc_clock_value_count) {
		return false;
	}
	soc_clock_values[idx] = value_hz;
	return true;
}

/**
 * ti_clk_get_value() - Get a cached clock frequency value with bounds checking.
 * @idx: The index into soc_clock_values[] array (from clk->freq_idx).
 *
 * Retrieves a frequency value from the soc_clock_values[] array for clocks
 * that cache their frequency (e.g., PLLs with TI_CLK_FLAG_CACHED set).
 * Validates the index is within array bounds before reading.
 *
 * Return: The cached frequency value in Hz, or 0 if index is out of bounds.
 */
static inline uint32_t ti_clk_get_value(uint16_t idx)
{
	if (idx >= soc_clock_value_count) {
		return 0;
	}
	return soc_clock_values[idx];
}

#endif /* TI_CLK_H */

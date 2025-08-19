/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
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

#include <stddef.h>

#include <common/debug.h>
#include <lib/utils_def.h>

#include <ti_pm_types.h>

/**
 * \brief Generates a 32-bit mask that covers all set bits in a number
 */
#define MASK_COVER_FOR_NUMBER(number) ((1UL << (32 - __builtin_clz(number))) - 1UL)

/** \brief Converts frequency from GHz to Hz */
#define FREQ_GHZ(freq)	((uint32_t) ((double) freq * 1000000000.0))

/** \brief Converts frequency from MHz to Hz */
#define FREQ_MHZ(freq)	((uint32_t) ((double) freq * 1000000.0))

/** \brief Converts frequency from KHz to Hz */
#define FREQ_KHZ(freq)	((uint32_t) ((double) freq * 1000.0))

/** \brief Converts frequency from Hz to Hz (no-op for consistency) */
#define FREQ_HZ(freq)	((uint32_t) (freq))

/**
 * \brief Initializes a clock range entry with min and max frequencies
 */
#define CLK_RANGE(id, min, max)[(id)] =				\
		{ .min_hz = (uint32_t) (min),			\
		  .max_hz = (uint32_t) (max) }

/**
 * \brief Initializes a clock default frequency entry with min, target, and max
 */
#define CLK_DEFAULT(id, min, target, max)[(id)] =		\
		{ .min_hz = (uint32_t) (min),			\
		  .target_hz = (uint32_t) (target),		\
		  .max_hz = (uint32_t) (max) }

/** \brief Clock type identifier for divider clocks */
#define TI_CLK_TYPE_DIV	1U

/** \brief Clock type identifier for multiplexer clocks */
#define TI_CLK_TYPE_MUX	2U

/** \brief Allows clock to modify its parent's frequency */
#define TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ	BIT(0)

/** \brief Skips hardware reinitialization during clock setup */
#define TI_CLK_DATA_FLAG_NO_HW_REINIT		BIT(2)

/** \brief Clock should be enabled during power-up */
#define TI_CLK_FLAG_PWR_UP_EN			((uint8_t) BIT(0))

/** \brief Clock has been initialized */
#define TI_CLK_FLAG_INITIALIZED			((uint8_t) BIT(2))

/** set if a cached frequency is stored in freq_idx (used for PLLs) */
#define TI_CLK_FLAG_CACHED				((uint8_t) BIT(3))

/** set if a clock has suspend handler called but not resume handler */
#define TI_CLK_FLAG_SUSPENDED			((uint8_t)BIT(4))

/** \brief Clock hardware is disabled */
#define TI_CLK_HW_STATE_DISABLED	0U

/** \brief Clock hardware is enabled */
#define TI_CLK_HW_STATE_ENABLED	1U

/** \brief Clock hardware is in transition state */
#define TI_CLK_HW_STATE_TRANSITION 2U

/**
 * \brief Defines a frequency range with minimum and maximum values
 */
struct ti_clk_range {
	uint32_t min_hz;	/** Minimum frequency in Hz */
	uint32_t max_hz;	/** Maximum frequency in Hz */
};

/**
 * \brief Defines a default frequency configuration with min, target, and max
 */
struct ti_clk_default {
	uint32_t min_hz;	/** Minimum acceptable frequency in Hz */
	uint32_t target_hz;	/** Target frequency in Hz */
	uint32_t max_hz;	/** Maximum acceptable frequency in Hz */
};

/**
 * \brief Dynamic clock runtime data structure
 */
struct ti_clk {
	uint8_t ref_count;		/** Reference count for clock usage */
	uint8_t freq_change_block_count;	/** Count of frequency change blocks */
	uint8_t flags;			/** Runtime flags (TI_CLK_FLAG_*) */
	uint32_t saved_val;		/** variable to save the clock value during lpm */
};

/**
 * \brief Represents a clock's parent and associated divider
 */
struct ti_clk_parent {
	uint16_t clk;	/** Parent clock ID */
	uint8_t div;	/** Divider value applied to parent frequency */
};

/**
 * \brief Clock driver-specific data (placeholder for extensibility)
 */
struct ti_clk_drv_data {
};

/**
 * \brief Constant clock configuration data
 */
struct ti_clk_data {
	const struct ti_clk_drv *drv;		/** Clock driver operations */
	const struct ti_clk_drv_data *data;	/** Driver-specific data */
	struct ti_clk_parent parent;		/** Parent clock and divider */
	uint16_t freq_idx;			/** Index into frequency value table */
	uint8_t range_idx;			/** Index into shared range table */
	uint8_t type;				/** Clock type (TI_CLK_TYPE_*) */
	uint8_t flags;				/** Configuration flags (TI_CLK_DATA_FLAG_*) */
};

struct ti_clk_drv {
	/** Perform any necessary intitialization */
	int32_t (*init)(struct ti_clk *clkp);

	/**
	 * \brief Set the current state of a clock.
	 *
	 * \param clk The clock to modify
	 * \param enabled True to enable, false to gate/power down
	 *
	 * \return true if the action succeeded
	 */
	bool (*set_state)(struct ti_clk *clkp, bool enabled);

	/**
	 * \brief Get the current state of a clock.
	 *
	 * \param clk The clock to query
	 *
	 * \return true if the clock is running
	 */
	uint32_t (*get_state)(struct ti_clk *clkp);

	/**
	 * \brief Program a clock to run at a given frequency. The minimum
	 * and maximum frequency parameters allow the best nearest match
	 * to be chosen. The clock tree is walked to accomplish this.
	 *
	 * \param clk The clock to modify/query
	 * \param target_hz The target frequency in Hz
	 * \param min_hz The minimum acceptable frequency
	 * \param max_hz The maximum acceptable frequency
	 * \param query true to only determine what the result of the function
	 * would be if query were set to false, but do nothing.
	 *
	 * \return Best frequency found in Hz, returns 0 for failure
	 */
	uint32_t (*set_freq)(struct ti_clk *clkp, uint32_t target_hz,
			     uint32_t min_hz, uint32_t max_hz,
			     bool query, bool *changed);

	/**
	 * \brief Return the frequency this clock runs at.
	 *
	 * \param clk The clock to query
	 *
	 * \return The frequency in Hz
	 */
	uint32_t (*get_freq)(struct ti_clk *clkp);

	/**
	 * \brief Notify a clock that its parent frequency has changed.
	 *
	 * \param clk The clock to query
	 * \param parent_freq_hz The new parent frequency in Hz.
	 * \param query true to only determine if the clock (and it's children)
	 * would accept this change, false to actually modify the clock tree.
	 *
	 * \return True if the change (would) succeed
	 */
	bool (*notify_freq)(struct ti_clk *clkp, uint32_t parent_freq_hz,
			    bool query);

	/**
	 * \brief Suspend and save clock context during suspend path.
	 *
	 * \param clkp The clock to suspend and save
	 *
	 * \return SUCCESS for success, error code otherwise
	 */
	int32_t (*suspend_save)(struct ti_clk *clkp);

	/**
	 * \brief Resume and restore clock context during resume path.
	 *
	 * \param clkp The clock to resume and restore
	 *
	 * \return SUCCESS for success, error code otherwise
	 */
	int32_t (*resume_restore)(struct ti_clk *clkp);
};

/** The table of dynamic clock data */
extern struct ti_clk soc_clocks[];
extern const size_t soc_clock_count;

/** The table of const clock data */
extern const struct ti_clk_data soc_clock_data[];

/** The table of shared const clock ranges */
extern const struct ti_clk_range soc_clock_ranges[];

/** The table of default frequencies */
extern const struct ti_clk_default soc_clock_freq_defaults[];

/** The table of dynamic stored clock frequencies (for freq_idx) */
extern uint32_t soc_clock_values[];

/**
 * \brief Sets frequency for value-based clocks (e.g., PLLs)
 *
 * \param clkp The clock to modify
 * \param target_hz Target frequency in Hz
 * \param min_hz Minimum acceptable frequency
 * \param max_hz Maximum acceptable frequency
 * \param query True to query only, false to apply changes
 * \param changed Output parameter indicating if frequency changed
 *
 * \return Best frequency found in Hz, 0 on failure
 */
uint32_t ti_clk_value_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			    uint32_t min_hz, uint32_t max_hz,
			    bool query, bool *changed);

/**
 * \brief Gets the stored frequency value for value-based clocks
 *
 * \param clkp The clock to query
 *
 * \return The stored frequency in Hz
 */
uint32_t ti_clk_value_get_freq(struct ti_clk *clkp);

/**
 * \brief Gets the parent clock's current frequency
 *
 * \param clkp The clock whose parent frequency to retrieve
 *
 * \return Parent frequency in Hz
 */
uint32_t ti_clk_get_parent_freq(struct ti_clk *clkp);

/**
 * \brief Notifies a clock that its parent frequency has changed
 *
 * \param clkp The clock to notify
 * \param parent_freq_hz New parent frequency in Hz
 * \param query True to check if change is acceptable, false to apply
 *
 * \return True if change is accepted
 */
bool ti_clk_notify_freq(struct ti_clk *clkp, uint32_t parent_freq_hz, bool query);

/**
 * \brief Notifies sibling clocks of a parent frequency change
 *
 * \param clkp The clock initiating the change
 * \param parent The parent clock
 * \param parent_freq New parent frequency in Hz
 * \param query True to check if change is acceptable, false to apply
 *
 * \return True if all siblings accept the change
 */
bool clk_notify_sibling_freq(struct ti_clk *clkp, struct ti_clk *parent,
			     uint32_t parent_freq, bool query);

/**
 * \brief Notifies all child clocks of a frequency change
 *
 * \param parent The parent clock
 * \param parent_freq New parent frequency in Hz
 * \param query True to check if change is acceptable, false to apply
 *
 * \return True if all children accept the change
 */
bool clk_notify_children_freq(struct ti_clk *parent, uint32_t parent_freq,
			      bool query);

/**
 * \brief Generic frequency setter that adjusts parent frequency
 *
 * \param clkp The clock to modify
 * \param parent The parent clock
 * \param target_hz Target frequency in Hz
 * \param min_hz Minimum acceptable frequency
 * \param max_hz Maximum acceptable frequency
 * \param query True to query only, false to apply
 * \param changed Output parameter indicating if frequency changed
 * \param div Divider value
 *
 * \return Best frequency found in Hz, 0 on failure
 */
uint32_t ti_clk_generic_set_freq_parent(struct ti_clk *clkp, struct ti_clk *parent,
				     uint32_t target_hz, uint32_t min_hz,
				     uint32_t max_hz, bool query,
				     bool *changed, uint32_t div);

/**
 * \brief Sets a clock's frequency
 *
 * \param clkp The clock to modify
 * \param target_hz Target frequency in Hz
 * \param min_hz Minimum acceptable frequency
 * \param max_hz Maximum acceptable frequency
 * \param query True to query only, false to apply
 * \param changed Output parameter indicating if frequency changed
 *
 * \return Best frequency found in Hz, 0 on failure
 */
uint32_t clk_set_freq(struct ti_clk *clkp, uint32_t target_hz, uint32_t min_hz,
		      uint32_t max_hz, bool query, bool *changed);

/**
 * \brief Gets a clock's current frequency
 *
 * \param clkp The clock to query
 *
 * \return Current frequency in Hz
 */
uint32_t ti_clk_get_freq(struct ti_clk *clkp);

/**
 * \brief Gets a clock's hardware state
 *
 * \param clkp The clock to query
 *
 * \return Clock state (TI_CLK_HW_STATE_*)
 */
uint32_t ti_clk_get_state(struct ti_clk *clkp);

/**
 * \brief Sets a clock's hardware state (enable/disable)
 *
 * \param clkp The clock to modify
 * \param enable True to enable, false to disable
 *
 * \return True if successful
 */
bool ti_clk_set_state(struct ti_clk *clkp, bool enable);

/**
 * \brief Increments a clock's reference count and enables it
 *
 * \param clkp The clock to get
 *
 * \return True if successful
 */
bool ti_clk_get(struct ti_clk *clkp);

/**
 * \brief Decrements a clock's reference count and disables if zero
 *
 * \param clkp The clock to put
 */
void ti_clk_put(struct ti_clk *clkp);

/**
 * \brief Allows frequency changes on a clock
 *
 * \param clkp The clock to unlock for frequency changes
 */
void ti_clk_freq_change_allow(struct ti_clk *clkp);

/**
 * \brief Blocks frequency changes on a clock
 *
 * \param clkp The clock to lock against frequency changes
 */
void ti_clk_freq_change_block(struct ti_clk *clkp);

/**
 * \brief Initializes the TI clock framework
 *
 * \return 0 on success, error code otherwise
 */
int32_t ti_clk_init(void);

/**
 * \brief Suspend all clocks on the device
 *
 * Iterates through all clocks and calls their suspend_save handlers to save
 * clock state before entering low power mode. Uses multiple passes to handle
 * clock tree dependencies, ensuring parent clocks are saved before children.
 * Bounded by LPM_CLK_MAX_TRIES to prevent infinite loops.
 *
 * \return 0 on success, -ETIMEDOUT if max tries exceeded, error code otherwise
 */
int32_t ti_clks_suspend(void);

/**
 * \brief Resume all clocks on the device
 *
 * Iterates through all clocks and calls their resume_restore handlers to
 * restore clock state after exiting low power mode. Uses multiple passes to
 * handle clock tree dependencies, ensuring parent clocks are restored before
 * children. Bounded by LPM_CLK_MAX_TRIES to prevent infinite loops.
 *
 * \return 0 on success, -ETIMEDOUT if max tries exceeded, error code otherwise
 */
int32_t ti_clks_resume(void);

/**
 * \brief Clears power-up enable flag on all clocks
 */
void ti_clk_drop_pwr_up_en(void);

/**
 * \brief Determine if a clock ID is valid.
 *
 * \param id
 * The internal clock API ID.
 *
 * \return
 * True if the clock ID refers to a valid SoC clock, false otherwise
 */
static inline bool clk_id_valid(clk_idx_t id)
{
	return (id < soc_clock_count) && (soc_clock_data[id].drv);
}

/**
 * \brief Lookup an internal clock based on a clock ID
 *
 * This does a lookup of a clock based on the clock ID.
 *
 * \param id
 * The internal clock API ID.
 *
 * \return
 * The clock pointer, or NULL if the clock ID was not valid.
 */
static inline struct ti_clk *clk_lookup(clk_idx_t id)
{
	return clk_id_valid(id) ? (soc_clocks + id) : NULL;
}

/**
 * \brief Return the clock ID based on a clock pointer.
 *
 * The clock pointer is just an index into the array of clocks. This is
 * used to return a clock ID. This function has no error checking for NULL
 * pointers.
 *
 * \param clk
 * The clock pointer.
 *
 * \return
 * The clock ID.
 */
static inline clk_idx_t clk_id(struct ti_clk *clkp)
{
	return (clk_idx_t) (clkp - soc_clocks);
}

/**
 * \brief Return a shared clock range.
 *
 * The clock infrastructure contains a set of shared clock ranges. Many
 * clocks and PLLs have min/max ranges. However, these min/max ranges can
 * take up a significant amount of storage. As many clocks share the same
 * ranges, we use a shared table of ranges. The clocks can then index that
 * table with a small uint8_t.
 *
 * \param idx
 * The index into the range table.
 *
 * \return
 * The range associated with the index, no error checking is performed.
 */
static inline const struct ti_clk_range *clk_get_range(clk_idx_t idx)
{
	return soc_clock_ranges + idx;
}

/**
 * \brief Return the clk_data struct associated with a clock.
 *
 * The clock pointer is translated to a clock index based on it's position
 * in the soc_clocks array. That index is then used to find the correct
 * element within the soc_clock_data array.
 *
 * \param clk
 * The clock for which a clk_data struct should be returned.
 *
 * \return
 * The pointer to the clk_data struct. No error checking is performed.
 */
static inline const struct ti_clk_data *clk_get_data(struct ti_clk *clkp)
{
	return soc_clock_data + clk_id(clkp);
}

#endif

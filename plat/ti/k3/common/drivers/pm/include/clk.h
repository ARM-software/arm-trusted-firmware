/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_H
#define CLK_H

#include <types/errno.h>
#include <stddef.h>
#include <pm_types.h>
#include <clock_limits.h>
#include <lib/utils_def.h>
#include <common/debug.h>

#define FREQ_GHZ(freq)	((uint32_t) ((double) freq * 1000000000.0))
#define FREQ_MHZ(freq)	((uint32_t) ((double) freq * 1000000.0))
#define FREQ_KHZ(freq)	((uint32_t) ((double) freq * 1000.0))
#define FREQ_HZ(freq)	((uint32_t) (freq))

#define CLK_RANGE(id, min, max)[(id)] =				\
	{ .min_hz = (uint32_t) (min), .max_hz = (uint32_t) (max) }

#define CLK_DEFAULT(id, min, target, max)[(id)] =			\
	{ .min_hz = (uint32_t) (min), .target_hz = (uint32_t) (target), .max_hz = (uint32_t) (max) }

#define CLK_RANGE_ANY 0

/* Type */
#define CLK_TYPE_CLK	0U
#define CLK_TYPE_DIV	1U
#define CLK_TYPE_MUX	2U

/* clk_data flags */
#define CLK_DATA_FLAG_MODIFY_PARENT_FREQ	BIT(0)
#define CLK_DATA_FLAG_ALLOW_FREQ_CHANGE		BIT(1)
#define CLK_DATA_FLAG_NO_HW_REINIT		BIT(2)
#define CLK_DATA_FLAG_BLOCK_FREQ_CHANGE		BIT(3)

#define CLK_FLAG_PWR_UP_EN			((uint8_t) BIT(0))
#define CLK_FLAG_PLL_BYPASS_FREQ		((uint8_t) BIT(1)) /* For PLLs */
#define CLK_FLAG_INITIALIZED			((uint8_t) BIT(2))

/** set if a cached frequency is stored in freq_idx (used for PLLs) */
#define CLK_FLAG_CACHED				((uint8_t) BIT(3))

/** set if a clock has suspend handler called but not resume handler */
#define CLK_FLAG_SUSPENDED			((uint8_t) BIT(4))

#define CLK_HW_STATE_DISABLED	0U
#define CLK_HW_STATE_ENABLED	1U
#define CLK_HW_STATE_TRANSITION 2U

#define CLK_ID_NONE ((clk_idx_t) (0xFFFFFFFFU))

struct clk_range {
	uint32_t	min_hz;
	uint32_t	max_hz;
};

struct clk_default {
	uint32_t	min_hz;
	uint32_t	target_hz;
	uint32_t	max_hz;
};

struct clk {
	uint8_t	ref_count;
	uint8_t	ssc_block_count;
	uint8_t	freq_change_block_count;
	uint8_t	flags;
};

/* Only allows div up to 63, and clk_ids up to 1023 */
struct clk_parent {
	uint16_t	clk : 10;
	uint16_t	div : 6;
};

struct clk_drv_data {
};

struct clk_data {
	const struct clk_drv		*drv;
	const struct clk_drv_data	*data;
	struct clk_parent		parent;
	uint16_t				freq_idx;
	uint8_t				range_idx;
	uint8_t				type : 2;
	uint8_t				flags : 6;
};

struct clk_drv {
	/** Perform any necessary intitialization */
	int32_t (*init)(struct clk *clkp);

	/**
	 * \brief Set the current state of a clock.
	 *
	 * \param clk The clock to modify
	 * \param enabled True to enable, false to gate/power down
	 *
	 * \return true if the action succeeded
	 */
	bool (*set_state)(struct clk *clkp, bool enabled);

	/**
	 * \brief Get the current state of a clock.
	 *
	 * \param clk The clock to query
	 *
	 * \return true if the clock is running
	 */
	uint32_t (*get_state)(struct clk *clkp);

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
	uint32_t (*set_freq)(struct clk *clkp, uint32_t target_hz, uint32_t min_hz, uint32_t max_hz,
			     bool query, bool *changed);

	/**
	 * \brief Return the frequency this clock runs at.
	 *
	 * \param clk The clock to query
	 *
	 * \return The frequency in Hz
	 */
	uint32_t (*get_freq)(struct clk *clkp);

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
	bool (*notify_freq)(struct clk *clkp, uint32_t parent_freq_hz, bool query);

};

struct clk_data_reg {
	struct clk_drv_data	data;
	uint32_t			reg;
	uint8_t			bit;
};

/** The table of dynamic clock data */
extern struct clk soc_clocks[SOC_CLOCKS_RANGE_ID_MAX];

/** The table of const clock data */
extern const struct clk_data soc_clock_data[SOC_CLOCK_DATA_RANGE_ID_MAX];

/** The table of shared const clock ranges */
extern const struct clk_range soc_clock_ranges[SOC_CLOCK_RANGES_ID_MAX];

/** The table of default frequencies */
extern const struct clk_default soc_clock_freq_defaults[SOC_CLOCK_FREQ_DEFAULTS_RANGE_ID_MAX];

/** The table of dynamic stored clock frequencies (for freq_idx) */
extern uint32_t soc_clock_values[SOC_CLOCK_VALUES_RANGE_ID_MAX];

/** The total number of SoC clocks */
extern const size_t soc_clock_count;

uint32_t clk_value_set_freq(struct clk *clkp, uint32_t target_hz, uint32_t min_hz, uint32_t max_hz,
			    bool query, bool *changed);
uint32_t clk_value_get_freq(struct clk *clkp);

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
static inline struct clk *clk_lookup(clk_idx_t id)
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
static inline clk_idx_t clk_id(struct clk *clkp)
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
static inline const struct clk_range *clk_get_range(clk_idx_t idx)
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
static inline const struct clk_data *clk_get_data(struct clk *clkp)
{
	return soc_clock_data + clk_id(clkp);
}

uint32_t clk_get_parent_freq(struct clk *clkp);
bool clk_notify_freq(struct clk *clkp, uint32_t parent_freq_hz, bool query);
bool clk_notify_sibling_freq(struct clk *clkp, struct clk *parent, uint32_t parent_freq,
			     bool query);
bool clk_notify_children_freq(struct clk *parent, uint32_t parent_freq, bool query);

uint32_t clk_generic_set_freq_parent(struct clk *clkp, struct clk *parent, uint32_t target_hz,
				     uint32_t min_hz, uint32_t max_hz, bool query,
				     bool *changed, uint32_t d);
uint32_t clk_set_freq(struct clk *clkp, uint32_t target_hz, uint32_t min_hz, uint32_t max_hz,
		      bool query, bool *changed);
uint32_t clk_get_freq(struct clk *clkp);
uint32_t clk_get_state(struct clk *clkp);
bool clk_set_state(struct clk *clkp, bool enable);
bool clk_get(struct clk *clkp);
void clk_put(struct clk *clkp);
void clk_ssc_allow(struct clk *clkp);
void clk_ssc_block(struct clk *clkp);
void clk_freq_change_allow(struct clk *clkp);
void clk_freq_change_block(struct clk *clkp);

extern const struct clk_drv clk_drv_input;

int32_t clk_init(void);

/**
 * \brief Deinitialize the clocks in a PM devgrp.
 *
 * The PM devgrp provides the first and last clock ID to deinitialize. Iterate
 * through the chosen clock IDs, and disable all clocks are that are turned on. Then,
 * clear the initialized and power up flag.
 *
 * \param pm_devgrp
 * The clocks in this PM devgrp will be deinitialized.
 *
 * \return
 * The status of deinitialization. SUCCESS if the action succeeded
 */
int32_t clk_deinit_pm_devgrp(uint8_t pm_devgrp);

static inline int32_t clks_suspend(void)
{
	return SUCCESS;
}
static inline int32_t clks_resume(void)
{
	return SUCCESS;
}

void clk_drop_pwr_up_en(void);


static inline struct clk *clk_p(clk_idx_t clk_id)
{
	return (struct clk *) (clk_id + soc_clocks);
}

#endif

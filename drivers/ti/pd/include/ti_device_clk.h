/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Clock Management API
 *
 * This header defines the device clock API for managing clocks attached to
 * devices. It provides clock type definitions, device clock data structures,
 * and functions for clock control including enable/disable, parent selection,
 * frequency configuration, and state management.
 */

#ifndef TI_DEVICE_CLK_H
#define TI_DEVICE_CLK_H

#include <ti_build_assert.h>
#include <ti_pm_types.h>

/*
 * Sentinel value for invalid or non-existent device clock ID
 *
 * Used to indicate that a device clock index is invalid, uninitialized, or
 * that no clock exists at the requested position. Functions returning
 * ti_dev_clk_idx_t use this value to signal errors or missing clocks.
 */
#define TI_DEV_CLK_ID_NONE ((ti_dev_clk_idx_t) (~0U))

/*
 * Input clock type.
 *
 * This is a normal clock, it's an input to the IP from elsewhere.
 */
#define TI_DEV_CLK_TABLE_TYPE_INPUT	0U

/*
 * Mux clock type.
 *
 * This is a clock mux that is meant to be controlled directly. The potential
 * parents of the mux are listed after the mux as TYPE_PARENT. Set frequency
 * calls to a mux are passed to the current parent.
 */
#define TI_DEV_CLK_TABLE_TYPE_MUX		1U

/*
 * Mux parent clock type.
 *
 * This is a potential parent for a mux. Normal clocks are set to follow the
 * device state (requested when device is enabled, unrequested otherwise) but
 * mux parents are set to always unrequested by default. If they are selected
 * they will get a ref count from the mux clock.
 *
 * Mux parents are present so they can be enabled and configured before being
 * selected via the mux.
 */
#define TI_DEV_CLK_TABLE_TYPE_PARENT	2U

/*
 * Output clock type.
 *
 * This is an output clock from the device. Setting the frequency informs
 * potential consumers of the frequency but otherwise has no effect.
 */
#define TI_DEV_CLK_TABLE_TYPE_OUTPUT	3U

struct ti_device;

/*
 * Const clock device data
 *
 * This is the fixed data for tracking clocks attached to devices.
 */
struct ti_dev_clk_data {
	/* Type of table entry */
	uint8_t type;

	/*
	 * Modifying the frequency of this clock will attempt to modify
	 * the parent freq.
	 */
	uint8_t modify_parent_freq;

	/*
	 * Clock divider, not valid for output, max 255 for parent otherwise
	 * up to USHORT_MAX/8.
	 */
	uint16_t div;

	/* For mux, the number of parents, for parents the index for the mux */
	uint8_t idx;

	/* For mux, the number of reserved parent slots */
	uint8_t n_reserved_parents;

	/* Which SoC clock this plugs into */
	uint16_t clk;
};

/*
 * Dynamic clock device data
 *
 * This tracks the current state of a device attached clock via flags.
 * This is just how the upstream clock is configured for the current
 * device. The upstream clock still has its own data and state.
 *
 * A pointer to a bss that has been reserved for this data is pointed
 * to by the device. If there is only one clock on the device, the field
 * is shared with the device flags and a forced cast is made to return
 * the field.
 */
struct ti_dev_clk {
	uint8_t flags;
};

/* Declare a device clock of type input */
#define TI_DEV_CLK(base, id, parent, _div)[(base) + (id)] =	\
		{ .clk = (parent), .div = (_div),		\
		  .type = TI_DEV_CLK_TABLE_TYPE_INPUT,		\
		  .modify_parent_freq	= 1U }

/* Declare a device clock of type mux */
#define TI_DEV_CLK_MUX(base, id, parent, _div, n_parents, n_reserved)[(base) + (id)] = \
		{ .clk = (parent), .div = (_div),			\
		  .type = TI_DEV_CLK_TABLE_TYPE_MUX,			\
		  .idx = (n_parents),					\
		  .n_reserved_parents = (n_reserved),			\
		  .modify_parent_freq = 1U }

/*
 * Declare a device clock of type mux parent. To match clk_mux_parent,
 * divider here can only be 8 bit
 */
#define TI_DEV_CLK_PARENT(base, id, parent, _div, _idx)[(base) + (id)] =	\
		{ .clk = (parent),					\
		  .type = TI_DEV_CLK_TABLE_TYPE_PARENT,			\
		  .div = (_div) + (int) TI_BUILD_ASSERT_OR_ZERO(_div < 256), \
		  .idx = (_idx),					\
		  .modify_parent_freq	= 1U }

/* Declare a device clock of type output. */
#define TI_DEV_CLK_OUTPUT(base, id, child)[(base) + (id)] =		\
		{ .clk = (child),					\
		  .type = TI_DEV_CLK_TABLE_TYPE_OUTPUT,			\
		  .modify_parent_freq = 1U }

/*
 * Get the dynamic data associated with a device clock.
 */
struct ti_dev_clk *ti_get_dev_clk(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/*
 * Get the const data associated with a device clock.
 */
const struct ti_dev_clk_data *ti_get_dev_clk_data(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/*
 * Lookup the clock API clock associated with a device clock index.
 */
struct ti_clk *ti_dev_get_clk(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_set_gated() - Set the gate/ungate property for a device clock
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 * @gated: True to gate the clock, false to ungate.
 *
 * This sets/clears the gate/ungate property for a device clock. If a clock
 * is gated, the internal clock API clock that it references is not ref
 * counted. If it is ungated, then the refcount is incremented when the device
 * is enabled and decremented when the device is disabled.
 *
 * Return: True if operation succeeded, false if it failed or clk_idx was invalid.
 */
bool ti_device_clk_set_gated(struct ti_device *dev, ti_dev_clk_idx_t clk_idx, bool gated);

/**
 * ti_device_clk_get_sw_gated() - Get the current state of the gated flag.
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 *
 * Return: The current state of the gated flag as set by ti_device_clk_set_gated.
 */
bool ti_device_clk_get_sw_gated(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_set_parent() - Change a device mux clocks's parent.
 * @dev: The device ID that the clock mux and new parent is connected to.
 * @clk_idx: The index of the clock mux on this device.
 * @parent_idx: The index of the new clock parent on this device.
 *
 * Return: True if parent change was successful, false if it
 *         failed or one of the clock indexes was invalid.
 */
bool ti_device_clk_set_parent(struct ti_device *dev, ti_dev_clk_idx_t clk_idx,
			      ti_dev_clk_idx_t parent_idx);

/*
 * Get the current parent of a device mux clock.
 */
ti_dev_clk_idx_t ti_device_clk_get_parent(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/*
 * Get the number of parents of a given device mux clock.
 */
ti_dev_clk_idx_t ti_device_clk_get_num_parents(struct ti_device *dev,
					       ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_set_freq() - Set the frequency of a device's clock
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 * @min_freq_hz: The minimum acceptable frequency (Hz).
 * @target_freq_hz: The target frequency in Hz. The clock API will
 *                  attempt to set a frequency as close as possible.
 * @max_freq_hz: The maximum acceptable frequency (Hz).
 *
 * This locates the correct clock and calls the internal clock API
 * set function. If the clock is a mux type, we instead send the request to
 * the parent. This is because calling set freq on the mux may switch the mux
 * which is not what we want on muxes that are exposed on devices.
 *
 * Return: True if the new frequency was accepted, false otherwise.
 */
bool ti_device_clk_set_freq(struct ti_device *dev, ti_dev_clk_idx_t clk_idx,
			    uint32_t min_freq_hz, uint32_t target_freq_hz,
			    uint32_t max_freq_hz);

/**
 * ti_device_clk_get_freq() - Get the current frequency of a device clock.
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 *
 * Return: Current clock frequency in Hz if enabled, or the
 *         frequency it would run at if enabled. Returns 0 if
 *         clk_idx is invalid or the clock cannot run.
 */
uint32_t ti_device_clk_get_freq(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_enable() - Enable a device clock
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 *
 * This is called by the device pm code for each clock when the device is
 * enabled. This allows the device clk code to take appropriate action.
 */
void ti_device_clk_enable(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_disable() - Disable a device clock
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 *
 * This is called by the device pm code for each clock when the device is
 * disabled. This allows the device clk code to take appropriate action.
 */
void ti_device_clk_disable(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

/**
 * ti_device_clk_init() - Initialize a device clock
 * @dev: The device ID that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 *
 * This is called by the device pm code for each clock when the device is
 * disabled. This allows the device clk code to take appropriate action such
 * setting default state.
 */
void ti_device_clk_init(struct ti_device *dev, ti_dev_clk_idx_t clk_idx);

#endif /* TI_DEVICE_CLK_H */

/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI LFOSC0 (Low Frequency Oscillator 0) Driver
 *
 * This driver provides software support for the low frequency oscillator,
 * which is typically a 32 KHz crystal used for RTC (Real-Time Clock) and
 * low-power timekeeping.
 */

#include <cdefs.h>

#include <lib/mmio.h>

#include <ti_clk.h>
#include <ti_clk_soc_lfosc0.h>
#include <ti_container_of.h>

/**
 * ti_clk_soc_lfosc0_get_freq() - Get the LFOSC0 oscillator frequency.
 * @clk_ptr: Clock structure containing board-specific data.
 *
 * Return: Oscillator frequency if enabled, 0 otherwise.
 */
static uint32_t ti_clk_soc_lfosc0_get_freq(struct ti_clk *clk_ptr)
{
	uint32_t reg_val;
	const struct ti_clk_data_lfosc0 *data;

	if (clk_ptr == NULL) {
		ERROR("LFOSC0 get_freq: NULL clk_ptr\n");
		return 0U;
	}

	if (clk_ptr->data == NULL) {
		ERROR("LFOSC0 get_freq: NULL data\n");
		return 0U;
	}

	data = ti_container_of(clk_ptr->data, const struct ti_clk_data_lfosc0, data);

	if (data->rtc_base == 0U) {
		ERROR("LFOSC0 get_freq: invalid rtc_base\n");
		return 0U;
	}

	reg_val = mmio_read_32(data->rtc_base + data->lfxosc_ctrl_offset);
	reg_val &= data->disable_mask;
	if (reg_val == 0U) {
		return data->freq_hz;
	}

	return 0U;
}

/**
 * ti_clk_soc_lfosc0_get_state() - Get the LFOSC0 oscillator state.
 * @clk_ptr: Clock structure containing board-specific data.
 *
 * Return: TI_CLK_HW_STATE_ENABLED if oscillator is enabled, DISABLED otherwise.
 */
static uint32_t ti_clk_soc_lfosc0_get_state(struct ti_clk *clk_ptr)
{
	uint32_t reg_val;
	const struct ti_clk_data_lfosc0 *data;

	if (clk_ptr == NULL) {
		ERROR("LFOSC0 get_state: NULL clk_ptr\n");
		return TI_CLK_HW_STATE_DISABLED;
	}

	if (clk_ptr->data == NULL) {
		ERROR("LFOSC0 get_state: NULL data\n");
		return TI_CLK_HW_STATE_DISABLED;
	}

	data = ti_container_of(clk_ptr->data, const struct ti_clk_data_lfosc0, data);

	if (data->rtc_base == 0U) {
		ERROR("LFOSC0 get_state: invalid rtc_base\n");
		return TI_CLK_HW_STATE_DISABLED;
	}

	reg_val = mmio_read_32(data->rtc_base + data->lfxosc_ctrl_offset);
	reg_val &= data->disable_mask;
	if (reg_val == 0U) {
		return TI_CLK_HW_STATE_ENABLED;
	}

	return TI_CLK_HW_STATE_DISABLED;
}

const struct ti_clk_drv ti_clk_drv_soc_lfosc0 = {
	.get_freq = ti_clk_soc_lfosc0_get_freq,
	.get_state = ti_clk_soc_lfosc0_get_state,
};

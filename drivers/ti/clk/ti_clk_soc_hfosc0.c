/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI HFOSC0 (High Frequency Oscillator 0) Driver
 *
 * This driver provides software support for the system clock crystal,
 * which is the primary external oscillator providing the reference clock
 * for the entire SoC.
 */

#include <cdefs.h>

#include <lib/mmio.h>

#include <ti_clk.h>
#include <ti_clk_soc_hfosc0.h>
#include <ti_container_of.h>

/**
 * ti_clk_soc_hfosc0_get_freq() - Get the HFOSC0 oscillator frequency.
 * @clk_ptr: Clock structure containing board-specific data.
 *
 * Reads the DEVSTAT register to determine the configured oscillator frequency
 * and validates it against the SoC-supported frequency list.
 *
 * Return: The oscillator frequency in Hz, or 0 if unsupported or out of range.
 */
static uint32_t ti_clk_soc_hfosc0_get_freq(struct ti_clk *clk_ptr)
{
	uint32_t reg_val;
	uint32_t freq_from_reg;
	uint32_t i;
	const struct ti_clk_data_hfosc0 *data;

	const uint32_t freq_table[] = {
		19200000U,
		20000000U,
		24000000U,
		25000000U,
		26000000U,
		27000000U,
	};

	if (clk_ptr == NULL) {
		ERROR("HFOSC0 get_freq: NULL clk_ptr\n");
		return 0U;
	}

	if (clk_ptr->data == NULL) {
		ERROR("HFOSC0 get_freq: NULL data\n");
		return 0U;
	}

	data = ti_container_of(clk_ptr->data, const struct ti_clk_data_hfosc0, data);

	if (data->wkup_ctrl_mmr_base == 0U) {
		ERROR("HFOSC0 get_freq: invalid wkup_ctrl_mmr_base\n");
		return 0U;
	}

	if (data->supported_freqs == NULL) {
		ERROR("HFOSC0 get_freq: NULL supported_freqs\n");
		return 0U;
	}

	reg_val = mmio_read_32(data->wkup_ctrl_mmr_base + data->devstat_offset);
	reg_val &= data->devstat_freq_mask;

	if (reg_val >= (uint32_t)ARRAY_SIZE(freq_table)) {
		return 0U;
	}

	freq_from_reg = freq_table[reg_val];

	for (i = 0U; data->supported_freqs[i] != 0U; i++) {
		if (freq_from_reg == data->supported_freqs[i]) {
			return freq_from_reg;
		}
	}

	return 0U;
}

/**
 * ti_clk_soc_hfosc0_get_state() - Get the HFOSC0 oscillator state.
 * @clk_ptr: Clock structure (unused, HFOSC0 is always enabled).
 *
 * Return: TI_CLK_HW_STATE_ENABLED always, as HFOSC0 is a fixed reference.
 */
static uint32_t ti_clk_soc_hfosc0_get_state(struct ti_clk *clk_ptr __unused)
{
	return TI_CLK_HW_STATE_ENABLED;
}

const struct ti_clk_drv ti_clk_drv_soc_hfosc0 = {
	.get_freq = ti_clk_soc_hfosc0_get_freq,
	.get_state = ti_clk_soc_hfosc0_get_state,
};

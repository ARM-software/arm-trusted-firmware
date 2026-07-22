/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Generic PLL API Header
 *
 * This header defines the common PLL data structures and interfaces shared
 * across different PLL variants. It provides PLL table entries, PLL data
 * structures for configuration parameters (VCO ranges, dividers, multipliers),
 * and the common PLL frequency calculation functions.
 */

#ifndef TI_CLK_PLL_H
#define TI_CLK_PLL_H

#include <ti_clk.h>
#include <ti_devgrps.h>

/*
 * PLL table entry
 *
 * This describes a precomputed PLL setting. This should be preferred
 * over brute force calculated settings. Additionally, PLL table entries
 * can be outside spec'd PLL parameters such as minimum VCO frequency.
 *
 * Note that the input frequency is not specified but the input frequency
 * range can be calculated from the given parameters.
 *
 * Current data type sizes are chosen based on currently supported PLLs.
 * If support for an additional PLL with a larger range is added, then
 * the data sizes will either need to increase or be selected at compile
 * time.
 */
struct ti_pll_table_entry {
	/* Minimum output frequency */
	uint32_t freq_min_hz;

	/* Maximum output frequency */
	uint32_t freq_max_hz;

	/* Multiplier setting */
	uint16_t pllm;

	/* Input divider setting */
	uint8_t plld;

	/* Output divider setting */
	uint8_t clkod;

	/* Fractional multiplier part */
	uint32_t pllfm;
};

/* The SoC specific table of precomputed PLL settings */
extern const struct ti_pll_table_entry ti_soc_pll_table[];

/* PLL specific const clock data. */
struct ti_clk_data_pll {
	/* Common const clock data */
	struct ti_clk_drv_data data;

	/*
	 * Precomputed PLL entries
	 *
	 * Points to an array of indexes into the PLL table. NULL if there are no
	 * entries.
	 */
	const uint8_t *pll_entries;

	/*
	 * Number of PLL entries
	 *
	 * Number of valid entries in pll_entries array.
	 */
	size_t pll_entries_count;

	/*
	 * VCO range.
	 *
	 * This points to the valid VCO frequency range in the range table.
	 */
	uint8_t vco_range_idx;

	/*
	 * VCO input range.
	 *
	 * This points to the valid VCO input frequency range in the range
	 * table.
	 */
	uint8_t vco_in_range_idx;

	/*
	 * Default frequency.
	 *
	 * This points to an index in the default frequency table. This
	 * allows the PLL to be programmed to a specific frequency at boot
	 * time. A table entry of 0 indicates that the current state of
	 * the hardware should be used.
	 */
	uint8_t default_freq_idx;

	/*
	 * Device Group.
	 *
	 * Device group(s) this PLL is contained in or DEVGRP_ALL to
	 * indicate unspecified.
	 */
	devgrp_t devgrp;

	/*
	 * Enable fractional support.
	 *
	 * True to enable fractional support if supported by this PLL. If
	 * this is false, fractional frequencies can still be read, but
	 * new fractional frequencies cannot be programmed.
	 */
	bool fractional_support;
};

/* Const PLL data for use by the pll_calc function. */
struct ti_pll_data {
	/* Maximum PLL divider. */
	uint32_t plld_max;

	/* Maximum PLL multiplier. */
	uint32_t pllm_max;

	/* Bits of fractional PLL multiplier. */
	uint32_t pllfm_bits;

	/* Maximum PLL post-divider. */
	uint32_t clkod_max;

	/*
	 * Indicate if a divider is valid.
	 *
	 * Some PLLs cannot use all values in their range, such as odd or
	 * even values. This callback allows the pll_calc function to skip
	 * over invalid values.
	 */
	bool (*plld_valid)(struct ti_clk *clkp, uint32_t plld);

	/*
	 * Indicate if a multiplier is valid.
	 *
	 * Some PLLs cannot use all values in their range, such as odd or
	 * even values. This callback allows the pll_calc function to skip
	 * over invalid values.
	 */
	bool (*pllm_valid)(struct ti_clk *clkp, uint32_t pllm, bool is_frac);

	/*
	 * Stride value for a given pllm.
	 *
	 * Some PLLs have additional multipliers enabled to reach certain
	 * pllm values. Normally, these are handled by treating the values
	 * in between possible values as invalid. However, when dealing with
	 * fractional multipliers the pll_calc function must understand that
	 * the fractional multiplier will also be multiplied by this value.
	 *
	 * Provide the extra multiplier value that gets applied at a given
	 * pllm value.
	 */
	uint32_t (*pllm_stride)(struct ti_clk *clkp, uint32_t pllm);

	/*
	 * Indicate if a post-divider is valid.
	 *
	 * Some PLLs cannot use all values in their range, such as odd or
	 * even values. This callback allows the pll_calc function to skip
	 * over invalid values.
	 */
	bool (*clkod_valid)(struct ti_clk *clkp, uint32_t clkod);

	/*
	 * Indicate a bin for this setting combination.
	 *
	 * Some PLLs prefer certain combinations or ranges of settings over
	 * others, such as a multiplier below 512. Binning allows pll_calc
	 * to act on that preference. This function returns a bin number for
	 * each pllm/plld/clkod combination passed. pll_calc will always
	 * prefer a setting with a higher bin value over a lower bin value.
	 */
	int32_t (*bin)(struct ti_clk *clkp, uint32_t plld, uint32_t pllm,
		       bool is_frac, uint32_t clkod);

	/*
	 * Try to find a larger pllm value that exists in a better bin
	 *
	 * The pll calculation code functions by testing every allowable plld
	 * and clkod combination. For each combination, it tests a pllm value
	 * that produces a frequency at or below the target, and a pllm value
	 * that produces a frequency above the target. However, this may skip
	 * certain plld/pllm/clkod combinations that are in a better bin.
	 * This function allows the pll driver code to return a larger pllm
	 * value that is in a better bin than the current pllm value. It
	 * will be called by the pll calculation code until it returns 0.
	 *
	 * Note that the pll calculation function will assume the pllm value
	 * returned is valid and will not call pllm_valid to test it.
	 */
	uint32_t (*bin_next_pllm)(struct ti_clk *clkp, uint32_t plld,
				  uint32_t pllm, uint32_t clkod);

	/*
	 * Try to find a smaller pllm value that exists in a better bin
	 *
	 * The pll calculation code functions by testing every allowable plld
	 * and clkod combination. For each combination, it tests a pllm value
	 * that produces a frequency at or below the target, and a pllm value
	 * that produces a frequency above the target. However, this may skip
	 * certain plld/pllm/clkod combinations that are in a better bin.
	 * This function allows the pll driver code to return a smaller pllm
	 * value that is in a better bin than the current pllm value. It
	 * will be called by the pll calculation code until it returns 0.
	 *
	 * Note that the pll calculation function will assume the pllm value
	 * returned is valid and will not call pllm_valid to test it.
	 */
	uint32_t (*bin_prev_pllm)(struct ti_clk *clkp, uint32_t plld,
				  uint32_t pllm, uint32_t clkod);

	/*
	 * Return fitness value based on VCO frequency.
	 *
	 * While PLLs already have a maximum and minimum allowable VCO
	 * frequency, many prefer certain VCO frequencies over others. If the
	 * bin and frequency delta of a given pllm/plld/clkod combination are
	 * identical, the vco_fitness function is used to pick a preferred
	 * combination.
	 */
	uint32_t (*vco_fitness)(struct ti_clk *clkp, uint32_t vco, bool is_frac);
};

/**
 * ti_pll_calc() - Calculate ideal PLL settings.
 * @clk: The PLL to calculate settings for.
 * @pll_data: The const parameters of the PLL that give allowable settings and preferences.
 * @input: The input frequency in Hz.
 * @output: The desired output frequency in Hz.
 * @min: The minimum acceptable output frequency in Hz.
 * @max: The maximum acceptable output frequency in Hz.
 * @plld: Storage for generated divider value.
 * @pllm: Storage for generated multiplier value.
 * @pllfm: Storage for generated fractional multiplier value.
 * @clkod: Storage for generated post-divider value.
 *
 * This calculates the ideal settings that can be used to generate an output
 * frequenccy given an input frequency and PLL properties. It iterates through
 * possible divider, multiplier, and post-divider values to find the best
 * combination. Preferences are sorted by:
 * - bin
 * - frequency delta
 * - VCO fitness
 * Where bin is the bin value returned by the bin callback, frequency delta
 * is the difference between the desired frequency and generated frequency,
 * and VCO fitness is the fitness value returned by the vco_fitness callback.
 *
 * Return: Frequency produced with the calculated plld/pllm/clkod
 *         values. 0 if no combination could produce an output
 *         between min and max.
 */
uint32_t ti_pll_calc(struct ti_clk *clkp, const struct ti_pll_data *pll_d,
		     uint32_t input, uint32_t output, uint32_t min, uint32_t max,
		     uint32_t *plld, uint32_t *pllm, uint32_t *pllfm,
		     uint32_t *clkod);

/**
 * ti_pll_init() - Base PLL initialization function
 * @clk: The PLL to calculate settings for.
 *
 * This contains the common PLL initialization code. This includes setting
 * the default frequency if applicable.
 *
 * Return: 0 on success, error code on error.
 */
int32_t ti_pll_init(struct ti_clk *clkp);

#endif /* TI_CLK_PLL_H */

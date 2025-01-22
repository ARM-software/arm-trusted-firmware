/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_pll.h>
#include <limits.h>
#include <lib/container_of.h>
#include <div64.h>
#include <mul.h>
#include <types/errno.h>

struct pll_consider_data {
	/* Original parameters */
	struct clk			*clk;
	const struct pll_data		*data;
	const struct clk_data_pll	*pll;
	const struct clk_range		*vco;
	const struct clk_range		*vco_in;
	uint32_t				input;
	uint32_t				output;
	uint32_t				min;
	uint32_t				max;

	/* Setting to consider */
	uint32_t				curr_plld;
	uint32_t				curr_clkod;
	/* clkod * plld */
	uint32_t				clkod_plld;
	/* max of vco min and output min * clkod */
	uint32_t				vco_min;
	/* min of vco max and output max * clkod */
	uint32_t				vco_max;
	/* Current best results */
	uint32_t				min_delta;
	uint32_t				min_delta_rem;
	uint32_t				min_delta_div;
	uint32_t				max_fitness;
	int32_t					max_bin;
	uint32_t				best_actual;
	uint32_t				*plld;
	uint32_t				*pllm;
	uint32_t				*pllfm;
	uint32_t				*clkod;
};

enum consider_result {
	PLLM_NO_VALUE,	/* No result stored yet */
	PLLM_UNTESTED,	/* We did not actually test the value */
	PLLM_HIGH,	/* pllm value was too high */
	PLLM_LOW,	/* pllm value was too low */
	PLLM_OK,	/* pllm value produced a frequency within limits */
};

/*
 * Consider the given pll table entry as a new possible best match. Make sure
 * our desired output frequeny falls within the table's given frequency range
 * and that the actual frequency produced falls within the desired output
 * frequency range. Note that no VCO testing is done. PLL table entries are
 * permitted to violate PLL specifications.
 */
static void pll_consider_entry(struct pll_consider_data		*data,
			       const struct pll_table_entry	*entry)
{
	uint64_t rem64;
	uint32_t rem;
	uint64_t actual64;
	uint32_t actual;
	uint32_t clkod_plld;
	bool pll_consider_done = false;

	/* Check if desired output is in given range for PLL table entry. */
	if ((data->output < entry->freq_min_hz) ||
	    (data->output > entry->freq_max_hz)) {
		pll_consider_done = true;
	}

	if (!pll_consider_done) {
		/*
		 * Determine actual frequency given table entry would produce:
		 *
		 * actual = input * pllm / (plld * clkod)
		 *
		 * Note: We break up the calculation in order to avoid a div64.
		 */
		clkod_plld = (uint32_t) (entry->plld * entry->clkod);

		actual64 = ((uint64_t) (data->input / clkod_plld)) * entry->pllm;
		rem64 = ((uint64_t) (data->input % clkod_plld)) * entry->pllm;

		if (rem64 > (uint64_t) ULONG_MAX) {
			/*
			 * Note: This is only true if clkod * plld * pllm >
			 * ULONG_MAX. This is not true for any currently
			 * supported PLLs, this 64 bit division is only
			 * included for completeness
			 */
			actual64 += pm_div64(&rem64, clkod_plld);
			rem = (uint32_t) rem64;
		} else {
			actual64 += (uint64_t) (((uint32_t) rem64) / clkod_plld);
			rem = ((uint32_t) rem64) % clkod_plld;
		}


		if (entry->pllfm != 0UL) {
			uint64_t fret;
			uint64_t frem;
			uint32_t stride = 1U;
			uint32_t pllfm_bits;
			uint32_t pllfm_range;
			uint32_t pllfm_mask;

			pllfm_bits = data->data->pllfm_bits;
			pllfm_range = (uint32_t) (1UL << pllfm_bits);
			pllfm_mask = pllfm_range - 1U;

			if (data->data->pllm_stride != NULL) {
				stride = data->data->pllm_stride(data->clk, entry->pllm);
			}

			/* Calculate fractional component of frequency */
			fret = ((uint64_t) (data->input / clkod_plld)) * entry->pllfm;
			frem = ((uint64_t) (data->input % clkod_plld)) * entry->pllfm;
			if (frem > (uint64_t) ULONG_MAX) {
				fret += pm_div64(&frem, clkod_plld);
			} else if (frem >= clkod_plld) {
				fret += (uint64_t) (((uint32_t) frem) / clkod_plld);
				frem = frem % (uint64_t) clkod_plld;
			} else {
				/* Do Nothing */
			}

			fret *= stride;
			frem *= stride;
			if (frem > (uint64_t) ULONG_MAX) {
				fret += pm_div64(&frem, clkod_plld);
			} else if (frem >= clkod_plld) {
				fret += (uint64_t) (((uint32_t) frem) / clkod_plld);
				frem = (uint64_t) (((uint32_t) frem) % clkod_plld);
			} else {
				/* Do Nothing */
			}

			frem += (uint64_t) (((uint32_t) (fret & pllfm_mask)) * clkod_plld);

			actual64 += fret >> pllfm_bits;
			rem += (uint32_t) (frem >> pllfm_bits);

			actual64 += (uint64_t) (((uint32_t) rem) / clkod_plld);
			rem += ((uint32_t) rem) % clkod_plld;
		}

		if (actual64 > (uint64_t) ULONG_MAX) {
			pll_consider_done = true;
		}
	}

	if (!pll_consider_done) {
		actual = (uint32_t) actual64;

		/*
		 * Check if output is within our allowable bounds. Don't round
		 * up, accept anything up to but not including max + 1.
		 */
		if ((actual < data->min) || (actual > data->max)) {
			pll_consider_done = true;
		}
	}

	if (!pll_consider_done) {
		uint32_t delta;
		uint32_t delta_rem;
		/* Check how close we got */
		if (data->output > actual) {
			delta = data->output - actual;
			if (rem != 0U) {
				delta_rem = clkod_plld - rem;
				delta--;
			} else {
				delta_rem = 0U;
			}
		} else {
			delta = actual - data->output;
			delta_rem = rem;
		}

		if ((delta < data->min_delta) ||
		    ((delta == data->min_delta) && (data->min_delta_rem != 0U) &&
		     ((delta_rem * data->min_delta_div) < (data->min_delta_rem * clkod_plld)))) {
			*data->pllm = entry->pllm;
			*data->pllfm = entry->pllfm;
			*data->plld = entry->plld;
			*data->clkod = entry->clkod;
			data->min_delta = delta;
			data->min_delta_rem = delta_rem;
			data->min_delta_div = clkod_plld;
			data->best_actual = actual;
		}
	}
}

/*
 * Consider the given plld/pllm/pllfm/clkod combination as a new best match.
 * Make sure it produces a valid VCO frequency and a valid output frequency. If
 * it's better than the current best match, store it as the new best match.
 */
static enum consider_result pll_consider(struct pll_consider_data *data,
					 uint32_t curr_pllm, uint32_t curr_pllfm, uint32_t stride)
{
	uint32_t rem;
	uint64_t rem64;
	uint64_t result64;
	uint32_t vco;
	int32_t bin;
	uint32_t rem_div;
	enum consider_result ret = PLLM_NO_VALUE;

	bin = data->data->bin(data->clk, data->curr_plld, curr_pllm,
			      curr_pllfm != 0UL, data->curr_clkod);

	/* Always ignore worse bins. */
	if (bin < data->max_bin) {
		ret = PLLM_UNTESTED;
	}


	rem_div = data->curr_plld;

	if (ret == PLLM_NO_VALUE) {
		/*
		 * Calculate the VCO frequency this combination will produce:
		 *
		 * vco = input * pllm / plld
		 *
		 * We break up the operation into (input / plld) * pllm to
		 * avoid a div64. We save the remainder so we can round the
		 * result down and up.
		 */
		result64 = ((uint64_t) (data->input / rem_div)) * curr_pllm;
		rem64 = ((uint64_t) (data->input % rem_div)) * curr_pllm;

		if (rem64 > (uint64_t) ULONG_MAX) {
			result64 += pm_div64(&rem64, rem_div);
			rem = (uint32_t) rem64;
		} else {
			result64 += (uint64_t) (((uint32_t) rem64) / rem_div);
			rem = ((uint32_t) rem64) % rem_div;
		}

		if (curr_pllfm != 0UL) {
			uint64_t fret;
			uint64_t frem;
			uint32_t mask;
			uint32_t bits;

			bits = data->data->pllfm_bits;
			mask = (uint32_t) (1UL << bits) - 1U;

			/* Calculate fractional part of frequency */
			fret = ((uint64_t) (data->input / rem_div)) * curr_pllfm;
			frem = ((uint64_t) (data->input % rem_div)) * curr_pllfm;
			if (frem > (uint64_t) ULONG_MAX) {
				fret += pm_div64(&frem, rem_div);
			} else {
				fret += ((uint32_t) frem) / rem_div;
				frem = (uint64_t) (((uint32_t) frem) % rem_div);
			}

			/* Fold in multiplier */
			fret *= stride;
			frem *= stride;
			if (frem > (uint64_t) ULONG_MAX) {
				fret += pm_div64(&frem, rem_div);
			} else {
				fret += (uint64_t) (((uint32_t) frem) / rem_div);
				frem = (uint64_t) (((uint32_t) frem) % rem_div);
			}

			/* Fold fractional part back into remainder */
			frem += (fret & (uint64_t) mask) * rem_div;

			/* Add integer part */
			result64 += fret >> bits;
			rem += (uint32_t) (frem >> bits);

			result64 += (uint64_t) rem / rem_div;
			rem = rem % rem_div;
		}

		/* Make sure our result at least fits within 32-bits */
		if (result64 > (uint64_t) ULONG_MAX) {
			ret = PLLM_HIGH;
		} else {
			vco = (uint32_t) result64;
		}
	}

	if (ret == PLLM_NO_VALUE) {
		/* Round down for min check. */
		if (vco < data->vco_min) {
			ret = PLLM_LOW;
		}
	}

	if (ret == PLLM_NO_VALUE) {
		if (vco > data->vco_max) {
			/*
			 * Normally round down for max check (ignore remainder).
			 * This allows all frequencies up to but not including
			 * +1.
			 */
			ret = PLLM_HIGH;
		} else if ((vco == data->vco->max_hz) && (rem != 0UL)) {
			/*
			 * If we are at the vco limit and not just the output
			 * limit, round up, don't allow any remainder.
			 */
			ret = PLLM_HIGH;
		} else {
			/* Do Nothing */
		}
	}

	if (ret == PLLM_NO_VALUE) {
		uint32_t actual;
		uint32_t delta;
		uint32_t delta_rem;
		bool better;
		bool same;

		/*
		 * Calculate the actual frequency produced:
		 *
		 * actual = input * pllm / (clkod * plld)
		 * actual = vco / clkod
		 *
		 * We already have the vco frequency from above, so we
		 * simply use that as a starting point.
		 */
		actual = vco / data->curr_clkod;
		rem += (vco % data->curr_clkod) * rem_div;
		rem_div *= data->curr_clkod;
		if (rem >= rem_div) {
			actual++;
			rem -= rem_div;
		}

		/* Check how close we got. */
		if (data->output > actual) {
			delta = data->output - actual;
			if (rem != 0U) {
				delta_rem = rem_div - rem;
				delta--;
			} else {
				delta_rem = 0U;
			}
		} else {
			delta = actual - data->output;
			delta_rem = rem;
		}

		better = false;
		same = false;
		if (delta < data->min_delta) {
			/* Easy, clear winner */
			better = true;
		} else if (delta == data->min_delta) {
			if ((delta_rem == 0U) && (data->min_delta_rem == 0U)) {
				/* Both are bang on at same delta */
				same = true;
			} else if (delta_rem == 0U) {
				/*
				 * Other delta has a remainder, but we don't,
				 * we are closer.
				 */
				better = true;
			} else if (data->min_delta_rem != 0U) {
				/*
				 * We both have remainders, cross multiply
				 * and compare result to detrmine same, better,
				 * or worse match.
				 *
				 * An exception to this is if both values are
				 * within 1 of the target. In this case we
				 * always prefer the one above the target.
				 * This ensures that if they program 50Hz and
				 * we can return 49.9Hz or 50.9Hz, we return
				 * 50Hz. Otherwise if they then decided to
				 * program 49Hz they would likely get 48.9Hz
				 * and so on which would return 48 on query,
				 * and so on.
				 */
				if ((delta == 0UL) && (data->best_actual != actual)) {
					/* One result at target, one at target - 1 */
					if (actual == data->output) {
						/* Our result is above, we are better */
						better = true;
					} else {
						/* Other result is above, they are better */
					}
				} else {
					uint32_t a = delta_rem * data->min_delta_div;
					uint32_t b = data->min_delta_rem * rem_div;

					if (a < b) {
						better = true;
					} else if (a == b) {
						same = true;
					} else {
						/* Do Nothing */
					}
				}
			} else {
				/* Do Nothing */
			}
		} else {
			/* Do Nothing */
		}

		/* Bin is the same, but delta is worse. */
		if ((bin == data->max_bin) && !same && !better) {
			/* No action */
		} else {
			uint32_t fitness;

			fitness = data->data->vco_fitness(data->clk, vco, curr_pllfm != 0UL);

			/* Bin and delta are the same, but fitness is same or worse. */
			if ((bin == data->max_bin) && same && (fitness <= data->max_fitness)) {
				/* No action */
			} else {
				/*
				 * New winner. bin is better, or bin is the
				 * same and delta is better, or bin and delta
				 * are the same and fitness is better.
				 */
				*data->plld = data->curr_plld;
				*data->pllm = curr_pllm;
				*data->pllfm = curr_pllfm;
				*data->clkod = data->curr_clkod;
				data->max_bin = bin;
				data->min_delta = delta;
				data->min_delta_rem = delta_rem;
				data->min_delta_div = rem_div;
				data->max_fitness = fitness;
				data->best_actual = actual;
			}
		}
		ret = PLLM_OK;
	}

	return ret;
}

static inline void pll_consider_fractional(struct pll_consider_data *data,
					   uint32_t curr_pllm, uint32_t pllm_rem,
					   uint32_t pllfm_estimate, uint32_t stride)
{
	uint32_t pllfm_range;
	uint32_t lowest_pllfm;
	uint32_t highest_pllfm;
	uint64_t pllfm_input;
	uint64_t best_delta;
	uint32_t best_pllfm;
	uint32_t curr_pllfm;
	bool found_best;
	uint64_t rem_target;

	pllfm_range = (uint32_t) (1UL << data->data->pllfm_bits);

	/* Because it's an estimate, we must check +/- 1 */
	lowest_pllfm = pllfm_estimate - 1U;
	highest_pllfm = pllfm_estimate + 1U;

	/*
	 * Clip at bonudaries. Note that we test the values
	 * associated with pll frac disabled and with the
	 * next pllm value. If either value is the closest
	 * value, we reject the configuration since the
	 * non-frac pll_consider call will handle it.
	 */
	if (pllfm_estimate == 0UL) {
		lowest_pllfm = 0U;
	}
	if (pllfm_estimate > pllfm_range) {
		highest_pllfm = pllfm_range;
	}

	/*
	 * Exception to the above. If curr_pllm+stride is not a valid
	 * multiplier, ignore the value 1 past the max register value
	 * above as it *cannot* be handled by the non-frac case.
	 */
	if (pllfm_estimate == pllfm_range) {
		if ((curr_pllm == data->data->pllm_max) ||
		    !((data->data->pllm_valid == NULL) ||
		      data->data->pllm_valid(data->clk, curr_pllm + stride, false))) {
			highest_pllfm--;
		}
	}

	/*
	 * Additional exception. If the curr_pllm+stride value produces an
	 * invalid frequency, it cannot be handled by the non-frac case. This
	 * should be an uncommon case.
	 */
	if (pllfm_estimate == pllfm_range) {
		uint32_t test_pllm;
		uint64_t test_vco;
		uint64_t test_vco_rem;

		test_pllm = curr_pllm + stride;

		test_vco = ((uint64_t) (data->input / data->curr_plld)) * test_pllm;
		test_vco_rem = ((uint64_t) (data->input % data->curr_plld)) * test_pllm;
		if (test_vco_rem > (uint64_t) ULONG_MAX) {
			test_vco += pm_div64(&test_vco_rem, test_pllm);
		} else {
			uint32_t extra = ((uint32_t) test_vco_rem) / test_pllm;

			test_vco += extra;
			test_vco_rem -= (uint64_t) (extra * test_pllm);
		}
		if (test_vco > data->vco_max) {
			highest_pllfm--;
		} else if ((test_vco == data->vco->max_hz) && (test_vco_rem != 0UL)) {
			highest_pllfm--;
		} else {
			/* Do Nothing */
		}
	}

	/*
	 * Find the value that gets closest to the actual remainder.
	 *
	 * remainder = pllfm * input / (pllfm range * plld)
	 * remainder * plld * pllfm range = pllfm * input
	 *
	 * Note that pllm_rem is already multiplied by plld, so:
	 *
	 * pllm_rem * pllfm range = pllfm * input
	 */
	rem_target = (uint64_t) (((uint64_t) pllm_rem) * (1UL << data->data->pllfm_bits));

	/* Start at the lowest and walk it up to the highest */
	pllfm_input = ((uint64_t) lowest_pllfm) * data->input;

	found_best = false;
	best_delta = 0ULL;
	best_pllfm = 0U;

	for (curr_pllfm = lowest_pllfm; curr_pllfm <= highest_pllfm; curr_pllfm++) {
		uint64_t delta;

		/* Find out how far we are off */
		if (rem_target >= pllfm_input) {
			delta = rem_target - pllfm_input;
		} else {
			delta = pllfm_input - rem_target;
		}

		/* See if it's better than our best */
		if (!found_best || (delta < best_delta)) {
			best_delta = delta;
			found_best = true;
			best_pllfm = curr_pllfm;
		}

		/* pllfm_input = pllfm * input */
		pllfm_input += data->input;
	}

	if (found_best) {
		if ((best_pllfm == 0UL) || (best_pllfm == pllfm_range)) {
			/*
			 * Ignore the result, we will be better served by a
			 * whole number pllm result.
			 */
			found_best = false;
		}
	}

	if (found_best) {
		pll_consider(data, curr_pllm, best_pllfm, stride);
	}
}

/*
 * Given an ideal pllm value, calculate a floor result (low) and a ceiling
 * result (high). The remainder passed is zero if the ideal pllm value is a
 * whole number, non-zero if there is a fractional component. Both the low
 * and high values are walked up/down until they find the first valid pllm
 * value. If no such value exists, they return 0.
 */
static inline void pll_find_pllm(struct clk *clkp, const struct pll_data *data,
				 uint32_t ideal, uint32_t remainder,
				 uint32_t *low, uint32_t *high)
{
	*low = 0U;
	*high = 0U;

	if (ideal >= data->pllm_max) {
		/*
		 * The ideal pllm value is above the range of valid pllm
		 * values. We can walk it lower for binning, but we can't
		 * walk it any higher.
		 */
		*low = data->pllm_max;
		*high = 0U;
	} else if (ideal == 0U) {
		/* pllm cannot be zero. Start at 1 and walk up if necessary */
		*low = 0U;
		*high = 1U;
	} else {
		/*
		 * The output frequency of a PLL is calculated by:
		 *
		 * output = input * pllm / (plld * clkod)
		 *
		 * We solve for pllm and look for the floor and
		 * ceiling result:
		 *
		 * low = floor(output * clkod * plld / input)
		 * high = ceil(output * clkod * plld / input)
		 *
		 * If there is low fractional component in the result,
		 * low == high.
		 */
		*low = ideal;
		*high = ideal + remainder;
	}

	/* Walk pllm down to find the first valid value */
	while (data->pllm_valid && !data->pllm_valid(clkp, *low, false)) {
		if (0U == (--(*low))) {
			/*
			 * Walked off the end of valid values, low = 0
			 * indicades no valid low value.
			 */
			break;
		}
	}

	/* Walk pllm up to find the second valid value */
	while (data->pllm_valid && !data->pllm_valid(clkp, *high, false)) {
		if ((*high)++ == data->pllm_max) {
			/*
			 * Walked off the end of valid values, set high = 0
			 * to indicate no valid high value.
			 */
			*high = 0U;
			break;
		}
	}
}

/*
 * Calculate ideal plld/pllm/clkod values for a given input frequency and
 * desired output frequency. The core component of a PLL is the Voltage
 * Controlled Oscillator (VCO). The PLL adjusts the frequency of the VCO
 * so that it counts out a specified number of clock cycles for each input
 * clock cycle. Most PLLs produce a cleaner output for higher VCO values.
 *
 * The number of VCO output cycles counter per input tick is referred to
 * as the PLL multiplier, or pllm. Typical PLLs permit a wide range of
 * values, from 1 to a value of hundreds or often thousands. Lower
 * multiplier values are typically preferred for better PLL performance.
 *
 * In order to produce a wider range of possible frequencies, PLLs also
 * provide an input divider, plld. The input clock is divided before
 * providing syncronization ticks for the VCO.
 *
 * VCOs typically must run at high frequencies (hundreds of MHz, with a max
 * generally in the GHz range), usually above the desired output frequency.
 * For this reason, PLLs contain clock output dividers that device the VCO
 * output frequency.
 *
 * Because the performance requirements of each PLL is different, we allow
 * the PLL driver to compute the fitness of a given VCO frequency and prefer
 * higher fitness values for two sets of plld/pllm/clkod values that produce
 * the same frequency.
 *
 * We also allow the PLL driver to bin results. Binning is currently used to
 * prefer all pllm values below 512 to higher values. This is because the
 * hardware team has indicated that for our current PLLs, values 512 and
 * above should only be used if absolutely necessary to reach a specific
 * frequency tolerance.
 *
 * The general algorithm of the function is for each plld/clkod combination
 * we find the best pllm value. For our current PLLs, plld can range from 1
 * to 64, and clkod from 1 to 16 excluding odd values except for 1. This
 * gives 576 possible combinations.
 *
 * The range of plld values is typically limited to a much smaller range due
 * to the input frequency / plld needing to remain above the minimum VCO
 * input frequency. For instance, with a PLL input frequency of 24MHz and a
 * minimum VCO input frequency of 3.2MHz, the maximum plld value is 7. This
 * reduces the number of plld/clkod combinations to 63 in common practice.
 *
 * The clkod can be limited by comparing the output frequency range to the
 * VCO frequency range. If there is no output frequency range (0 to
 * ULONG_MAX) then the full clkod range must be tested. However, if the
 * HLOS passes a frequency range of 95MHz to 100MHz, and the VCO range
 * is 700MHz to 4.2GHz, then the allowable clkod range becomes 8 to
 * 16, 5 values. This reduces the number of plld/clkod combinations down
 * to 35.
 *
 * For each clkod value, we produce an ideal VCO value. Then, for each
 * plld value, we find a pllm value that will produce that VCO value.
 * Because the given pllm/plld combination wil llikely produce a value
 * not exactly equal to the desired VCO value, we usually test two pllm
 * values. One that produces the frequency below the target VCO value, and
 * one that produces the frequency above the target VCO value.
 *
 * In order to find a value that is within the best bin, we ask the PLL
 * driver to provide us with additional pllm values. For each pllm value
 * we test, we ask the driver if another pllm value exists in another
 * bin either above or below the current pllm value. We repeat this
 * process until the driver indicates no more such values exist. Our
 * current PLL drivers only have two bins. One for values below 512, bin 1,
 * and one for higher values, bin 0. In practice this means if our low
 * pllm value is in bin 0, we will also test 511, which is in bin 1.
 *
 * The pll_calc function will also test if a PLL table entry exists for
 * the desired output frequency and produces a frequency within the desired
 * output range given the input frequency. If so, it will just return the
 * table entry rather than attempting to calculate plld/pllm/clkod values.
 *
 * Note on 32/64 bit math:
 *
 * div64 can be very expensive compared to a simple udiv instruction.
 * Avoid using it by splitting up the division into two parts:
 *
 * (A*B)/C = (A/C) * B
 *
 * The LHS division is likely to require 64 bit division, but the RHS
 * will not require 64 bit division (assuming A and C are 32 bits).
 * However, precision is lost if integer division is done. Therefore
 * the division needs to be split up further so the remainder can
 * be tracked.
 *
 * integer part = A//C
 * remainder = A%C
 *
 * Those values can be recombined to form the original result:
 *
 * A/C = A//C + (A%C)/C
 *
 * We can then mix the B term in:
 *
 * (A/C)*B = (A//C) * B + ((A%C) * B) / C
 *
 * The final division may be larger than 32 bits if the number if
 * (C-1)*Bmax > ULONG_MAX. This can easily be checked. In
 * our case, this is the combination of the plld, pllm, and clkod
 * values which fits within 32 bits.
 *
 * A check is added in case the final division does need 64
 * bits due to programming error or unexpected input.
 */
static inline void pll_internal_calc(struct pll_consider_data *consider_data)
{
	struct clk *clkp = consider_data->clk;
	const struct pll_data *data = consider_data->data;
	uint32_t lowest_clkod;
	uint32_t highest_clkod;
	uint32_t clkod;
	uint32_t lowest_plld;
	uint32_t highest_plld;
	uint32_t vco_target;
	uint64_t input_inverse64 = 0UL;
	bool do_frac;

	/*
	 * Find allowable plld range. plld is the PLL input divider. Valid
	 * plld values are constrained by the PLL limitations on plld and
	 * also by the input frequency limits of the VCO:
	 *
	 * VCO input frequecy = input frequency / plld
	 *
	 * To calculate our valid range, we use:
	 *
	 * lowest plld = ceil(input frequency / VCO input maximum)
	 * highest plld = floor(input frequency / VCO input minimum)
	 *
	 * We then clip lowest/highest to the PLL's plld range. Note that
	 * after clipping, lowest_plld may be larger than highest_plld, in
	 * which case there are no possible combinations and we will return
	 * 0.
	 */
	lowest_plld = 1U + ((consider_data->input - 1U) / consider_data->vco_in->max_hz);
	if (consider_data->vco_in->min_hz != 0U) {
		highest_plld = consider_data->input / consider_data->vco_in->min_hz;
	} else {
		highest_plld = data->plld_max;
	}

	if (highest_plld > data->plld_max) {
		highest_plld = data->plld_max;
	}

	/*
	 * Find allowable clkod range. clkod is the PLL output divider. Valid
	 * clkod values are constrained by the PLL limitations on clkod and
	 * also by the relationship between the maximum/minimum VCO
	 * frequencies and the maximum/minimum output frequency. The output
	 * frequency of a PLL is given by:
	 *
	 * output frequency = VCO frequency / clkod
	 *
	 * To calculate the range of possible clkod values, we use:
	 *
	 * lowest clkod = ceil(Minimum VCO freq / maximum output freq)
	 * highest clkod = floor(Maximum VCO freq / minimum output freq)
	 *
	 * Any lower dividers would not be able to divide the lowest possible
	 * VCO frequency enough to be at or below the maximum output
	 * frequency.
	 *
	 * Any higher dividers would divide the highest possible VCO frequency
	 * by too much and it be below the minimum output frequency.
	 *
	 * Note that we want to accept all frequency up to but not including
	 * max + 1. For example, if the user gives a max frequency of 687Hz,
	 * we will accept 687.8Hz, but not 688Hz. We add one to the max
	 * frequency here to calculate for all frequencies up to and including
	 * max + 1, and then later reject exact matches to max + 1.
	 *
	 * We then clip lowest/highest to the PLL's clkod range. As with plld,
	 * the resulting lowest value may be above the highest value.
	 */
	if (consider_data->max == (uint32_t) ULONG_MAX) {
		/* Avoid divide by zero, result will always be one anyway */
		lowest_clkod = 1U;
	} else if (consider_data->vco->min_hz == 0UL) {
		/* 0 is a legal value for vco min_hz */
		lowest_clkod = 0U;
	} else {
		lowest_clkod = 1U + ((consider_data->vco->min_hz - 1U) / (consider_data->max + 1U));
	}
	if (consider_data->min == 0UL) {
		highest_clkod = data->clkod_max;
	} else {
		highest_clkod = consider_data->vco->max_hz / consider_data->min;
	}

	/* Make sure clkod is in range */
	if (highest_clkod > data->clkod_max) {
		highest_clkod = data->clkod_max;
	}
	if (lowest_clkod == 0U) {
		lowest_clkod++;
	}

	/*
	 * The PLL fractional multiplier can provide a much closer output
	 * result over a wide range of values. However, calculating it is
	 * potentially difficult especially when considering a wide range of
	 * values.
	 *
	 * The basic equation for finding the register value considers only the
	 * frequency range between pllm values, the possible range is thus
	 * input_freq * (pllm + 1) - input_freq * pllm which simplifies to
	 * input_freq. The relation is thus:
	 *
	 *	 fraction freq	   register value
	 *     ---------------- = ----------------
	 *	vco input freq	   register range
	 *
	 * And solving for the register value:
	 *
	 * reg = fractional freq * register range / vco input freq
	 *
	 * Note that register range and fraction freq range are typically of
	 * the order 2^24. Multiplying them both together requires a 64 bit
	 * type. Performing this 64-bit division for every possible pllm would
	 * be prohibitive.
	 *
	 * We instead use an approximation method. First we note the relation:
	 *
	 * vco input freq = input freq / plld
	 *
	 * We pre-calculate an inverse value using a single div64. We use a
	 * large numerator to retain as much precision as possible. The value
	 * chosen for this extra precision is 32 bits:
	 *
	 * inverse = register range * 2^32 / input freq
	 * vco input freq = input freq / plld
	 * input freq = vco input freq * plld
	 * inverse = register range * 2^32 / (vco input freq * plld)
	 * inverse = (register range / vco input freq) * (2^32 / plld)
	 *
	 * Note that the larger the input frequency, the smaller the inverse
	 * value and the smaller the input frequency, the larger the inverse
	 * value. Input frequencies at or below the register range produce a
	 * 64-bit value, input frequencies above the register range produce a
	 * 32-bit value. Due to this relation, there is no risk of a 64-bit
	 * overflow in the multplication below.
	 *
	 * When we need to calculate the reg value, we use this inverse:
	 *
	 * reg = fractional freq * register range / vco input freq
	 * reg = fractional freq * inverse * plld / 2^32
	 *
	 * The upper 32 bits will contain the integer portion, and the lower
	 * 32 bits the fractional portion. Note that the result is not full
	 * precision because the remainder from the initial 64 bit division is
	 * not used in the calculation. The remainder can be as large as the
	 * input frequency using it would require another 64 bit division.
	 *
	 * The result is an estimate and may need to be adjusted to obtain the
	 * final value. The error can be show as:
	 *
	 * error = fraction freq * (register range %% (vco input freq * plld)) /
	 *			(vco input freq * plld) * plld / 2^32
	 *
	 * Or a maximum value of:
	 *
	 * error = fraction freq * (vco input freq*plld-1) /
	 *			(vco input freq * plld) * plld / 2^32
	 *
	 * Or approximately:
	 *
	 * error = fraction freq * plld / 2^32
	 *
	 * This value will be less than 1 if fraction freq * plld < 2^32. Note
	 * that the maximum value of the fractional freq in the vco input
	 * freq - 1. Or approximately input freq / plld. The expression can be
	 * more simplly represented by:
	 *
	 * input freq < 2^32
	 *
	 * Thus estimate is off by a fractional value and we only need to check
	 * the values directly above and below the estimate to check which value
	 * is closest.
	 */
	do_frac = (data->pllfm_bits != 0UL) && consider_data->pll->fractional_support;
	if (do_frac) {
		uint64_t input_inverse_rem64;

		input_inverse_rem64 = (uint64_t) 1UL << data->pllfm_bits;
		input_inverse_rem64 <<= 32U;
		input_inverse64 = pm_div64(&input_inverse_rem64, consider_data->input);
	}

	for (clkod = lowest_clkod; clkod <= highest_clkod; clkod++) {
		uint32_t extra;
		uint32_t plld;
		uint32_t ideal_pllm;
		uint64_t ideal_pllm_rem;
		uint32_t ideal_pllm_step;
		uint32_t ideal_pllm_step_rem;

		/*
		 * We can just compare the VCO value against the below
		 * generated limit to be sure it fits within the legal
		 * VCO limits and the allowable output frequency.
		 */
		if (mul32_check_overflow(clkod, consider_data->min, &consider_data->vco_min)) {
			consider_data->vco_min = (uint32_t) ULONG_MAX;
		}
		if (consider_data->vco_min < consider_data->vco->min_hz) {
			consider_data->vco_min = consider_data->vco->min_hz;
		}

		if (mul32_check_overflow(clkod, consider_data->max, &consider_data->vco_max)) {
			consider_data->vco_max = (uint32_t) ULONG_MAX;
		} else {
			/* vco_max = clkod * (max + 1U) - 1U */
			consider_data->vco_max += clkod - 1U;
			if (consider_data->vco_max < (clkod - 1UL)) {
				/* Overflow occurred */
				consider_data->vco_max = (uint32_t) ULONG_MAX;
			}
		}
		if (consider_data->vco_max > consider_data->vco->max_hz) {
			consider_data->vco_max = consider_data->vco->max_hz;
		}

		if (data->clkod_valid && !data->clkod_valid(clkp, clkod)) {
			continue;
		}

		consider_data->curr_clkod = clkod;

		/*
		 * vco_target is our ideal VCO frequency for a given clkod value:
		 *
		 * output = VCO frequency / clkod
		 * VCO frequency = output * clkod
		 *
		 * Since we are incrementing clkod by one each loop, we can just do
		 * the multiplication once here with the initial clkod value and
		 * then add output each time we increment clkod. We start with
		 * the lowest_clkod - 1 to make the continue logic a bit easier.
		 */
		if (mul32_check_overflow(consider_data->output, clkod, &vco_target)) {
			vco_target = (uint32_t) ULONG_MAX;
		}

		/*
		 * Clip our desired VCO frequency to the range of
		 * allowable VCO frequencies.
		 */
		if (vco_target < consider_data->vco->min_hz) {
			vco_target = consider_data->vco->min_hz;
		} else if (vco_target > consider_data->vco->max_hz) {
			vco_target = consider_data->vco->max_hz;
		} else {
			/* Do Nothing */
		}

		/*
		 * How much to add to out ideal_pllm value and remainder
		 * each time we add 1 to plld.
		 * Note: clipped to ULONG_MAX by comparison with vco->max_hz
		 */
		ideal_pllm_step = vco_target / consider_data->input;
		ideal_pllm_step_rem = vco_target % consider_data->input;

		/*
		 * Initial value is ideal_pllm_step * lowest_plld, we
		 * actually start at lowest_plld -1 to make the loop
		 * logic easier.
		 */
		ideal_pllm = ideal_pllm_step * (lowest_plld - 1U);
		ideal_pllm_rem = ideal_pllm_step_rem;
		ideal_pllm_rem *= (uint64_t) (lowest_plld - 1UL);

		/* Handle the carry */
		while (ideal_pllm_rem > (uint64_t) ULONG_MAX) {
			/*
			 * Note: lowest_plld will only be > 1 if:
			 *
			 * (input - 1) > vco_in->max_hz
			 *
			 * This cannot hold true for any currently
			 * supported PLLs. This loop to handle
			 * ideal_plld_rem values > ULONG_MAX is
			 * only here for completeness as it will
			 * execute at max lowest_plld - 1 times.
			 */
			ideal_pllm++;
			ideal_pllm_rem -= consider_data->input;
		}
		extra = ((uint32_t) ideal_pllm_rem) / consider_data->input;
		ideal_pllm += extra;
		ideal_pllm_rem -= ((uint64_t) extra) * consider_data->input;

		for (plld = lowest_plld; plld <= highest_plld; plld++) {
			uint32_t low_pllm = 0U;
			uint32_t high_pllm = 0U;
			enum consider_result ret;

			consider_data->curr_plld = plld;

			/* pllm = vco_target * plld / input */
			ideal_pllm += ideal_pllm_step;
			ideal_pllm_rem += ideal_pllm_step_rem;

			/* Handle the carry */
			if (ideal_pllm_rem >= consider_data->input) {
				ideal_pllm++;
				ideal_pllm_rem -= consider_data->input;
			}

			consider_data->clkod_plld += clkod * plld;

			if (data->plld_valid &&
			    !data->plld_valid(clkp, plld)) {
				continue;
			}

			/*
			 * Calculate the closest pllm value. We already have
			 * our VCO frequency we are targeting given the
			 * current output frequency and clkod value. We can
			 * then determine the best pllm value given the input
			 * frequency and plld value:
			 *
			 * VCO frequency = input frequency * pllm / plld
			 * ideal pllm = VCO frequency * plld / input frequency
			 *
			 * Along with the integer result, we get a remainder.
			 * We use this to take the find the ceiling and
			 * floor result.
			 */

			/*
			 * Closest pllm values that will produce a frequency
			 * just below and above our target.
			 */
			pll_find_pllm(clkp, data, ideal_pllm,
				      (ideal_pllm_rem != 0UL) ? 1U : 0U,
				      &low_pllm, &high_pllm);

			/*
			 * If we have a low_pllm value, it means the ideal pllm
			 * integer multiplier produces a value below the ideal
			 * frequency. A fractional addition can help.
			 * Note that if the low_pllm is equal to the ideal_pllm
			 * without a remainder, a fractional value won't get us
			 * any closer.
			 */
			if (do_frac && (low_pllm != 0U) &&
			    (low_pllm <= ideal_pllm) &&
			    ((ideal_pllm_rem != 0UL) || (low_pllm != ideal_pllm)) &&
			    ((data->pllm_valid == NULL) ||
			     data->pllm_valid(clkp, low_pllm, true))) {
				/*
				 * Some PLLs have an internal multiplier that
				 * combines with the programmable multiplier.
				 * Make sure to consider this when calculating
				 * the register value. The stride will typically
				 * by high_pllm - low_pllm.
				 */
				uint32_t estimate_pllfm;
				uint32_t stride = 1U;
				uint64_t frem = ideal_pllm_rem;

				if (data->pllm_stride != NULL) {
					stride = data->pllm_stride(clkp, low_pllm);
				}
				if (stride != 1UL) {
					/*
					 * Adjust the remainder according to our
					 * position within the stride.
					 */
					frem += (uint64_t) ((ideal_pllm - low_pllm) *
							    consider_data->input);
					frem /= stride;
				}
				estimate_pllfm = (uint32_t) ((frem * input_inverse64) >> 32U);
				if (stride != 1UL) {
					/* estimate_pllfm /= stride; */
				}
				/*
				 * Calculate out pllfm estimate as discussed
				 * above. Pass it off to our fractional
				 * consider function.
				 */
				pll_consider_fractional(consider_data, low_pllm,
							(uint32_t) frem, estimate_pllfm, stride);
			}

			if ((low_pllm != 0U) && (low_pllm == high_pllm)) {
				/*
				 * If we produce the exact frequency we want,
				 * and ideal_pllm is a valid pllm value, then
				 * low_pllm will be the same as high_pllm. In
				 * this case move high_pllm to the next bin
				 * if one exists to avoid duplicating
				 * calculations.
				 */
				if (data->bin_next_pllm != NULL) {
					high_pllm = data->bin_next_pllm(
						clkp,
						consider_data->curr_plld,
						high_pllm,
						consider_data->curr_clkod);
				} else {
					high_pllm = 0U;
				}
			}

			while (low_pllm != 0U) {
				/*
				 * This value will be at or below our target
				 * frequency.
				 */
				ret = pll_consider(consider_data, low_pllm, 0U, 0U);

				/*
				 * Keep walking down to the next bin until we
				 * run out of bins or get a pllm value that
				 * is too low.
				 */
				if ((ret != PLLM_LOW) && data->bin_prev_pllm) {
					low_pllm = data->bin_prev_pllm(
						clkp,
						consider_data->curr_plld,
						low_pllm,
						consider_data->curr_clkod);
				} else {
					low_pllm = 0U;
				}
			}

			while (high_pllm != 0U) {
				/*
				 * This value will be above our target
				 * frequency.
				 */
				ret = pll_consider(consider_data, high_pllm, 0U, 0U);

				/*
				 * Keep walking up to the next bin until we
				 * run out of bins or get a pllm value that
				 * is too high.
				 */
				if ((ret != PLLM_HIGH) && data->bin_next_pllm) {
					high_pllm = data->bin_next_pllm(
						clkp,
						consider_data->curr_plld,
						high_pllm,
						consider_data->curr_clkod);
				} else {
					high_pllm = 0U;
				}
			}
		}
	}
}

uint32_t pll_calc(struct clk *clkp, const struct pll_data *data,
		  uint32_t input, uint32_t output, uint32_t min, uint32_t max,
		  uint32_t *plld, uint32_t *pllm, uint32_t *pllfm, uint32_t *clkod)
{
	uint32_t ret = 0U;


	if ((input != 0U) && (output != 0U)) {
		const struct clk_data *clk_datap = clk_get_data(clkp);
		const struct clk_data_pll *pll = container_of(clk_datap->data,
							      struct clk_data_pll, data);
		struct pll_consider_data consider_data = {
			.clk		= clkp,
			.data		= data,
			.pll		= pll,
			.vco		= clk_get_range(pll->vco_range_idx),
			.vco_in		= clk_get_range(pll->vco_in_range_idx),
			.input		= input,
			.output		= output,
			.min		= min,
			.max		= max,

			.min_delta	= (uint32_t) ULONG_MAX,
			.max_fitness	= 0U,
			.max_bin	= 0,
			.best_actual	= 0U,
			.plld		= plld,
			.pllm		= pllm,
			.pllfm		= pllfm,
			.clkod		= clkod,
		};

		/* Check tables */
		if ((consider_data.pll->pll_entries) != NULL) {
			int32_t i;

			for (i = 0; (consider_data.pll->pll_entries[i] != PLL_TABLE_LAST); i++) {
				pll_consider_entry(&consider_data,
						   &soc_pll_table
						   [consider_data.pll->pll_entries[i]]);
				if (consider_data.min_delta == 0U) {
					/* Exact match found, done */
					break;
				}
			}
		}

		/* No table match found, perform calcualtion */
		if (consider_data.min_delta == (uint32_t) ULONG_MAX) {
			pll_internal_calc(&consider_data);
		}

		/*
		 * When returning frequencies, used the rounded down value. This
		 * makes sure that if freq=ret, min=ret, max=ret is passed as
		 * arguments, we always succeed and calculate the same parameters
		 * as before even though ret may contain a fractional component.
		 */
		ret = consider_data.best_actual;
	}

	return ret;
}

int32_t pll_init(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_drv *drv;
	const struct clk_data_pll *data_pll;
	int32_t ret = SUCCESS;
	uint32_t freq = 0U;

	drv = clk_datap->drv;
	data_pll = container_of(clk_datap->data,
				const struct clk_data_pll, data);

	if (data_pll->default_freq_idx != 0U) {
		const struct clk_default *dflt;
		bool changed;

		/* Lookup entry */
		dflt = &soc_clock_freq_defaults[data_pll->default_freq_idx];

		/* Attempt to set default frequency */
		freq = drv->set_freq(clkp, dflt->target_hz, dflt->min_hz, dflt->max_hz,
				     false, &changed);

		/* set_freq returns 0 if default frequency is not set */
		if (freq == 0U) {
			ret = -EFAIL;
		}
	}

	/*
	 * We must always assume we are enabled as we could be operating
	 * clocks in bypass.
	 */
	if (ret == SUCCESS) {
		clkp->flags |= CLK_FLAG_PWR_UP_EN;
	}

	return ret;
}

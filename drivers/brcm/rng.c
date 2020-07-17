/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define RNG_CTRL_REG		(RNG_BASE_ADDR + 0x00)
#define RNG_CTRL_MASK		0x00001FFF
#define RNG_CTRL_ENABLE		0x00000001
#define RNG_CTRL_DISABLE	0x00000000

#define RNG_SOFT_RESET_REG	(RNG_BASE_ADDR + 0x04)
#define RNG_SOFT_RESET_MASK	0x00000001

#define RNG_FIFO_DATA_REG	(RNG_BASE_ADDR + 0x20)

#define RNG_FIFO_COUNT_REG	(RNG_BASE_ADDR + 0x24)
#define RNG_FIFO_COUNT_MASK	0x000000FF

#define RNG_FIFO_WORDS_MAX	16
#define MAX_WAIT_COUNT_50US	20000


static void rng_reset(void)
{
	/* Disable RBG */
	mmio_clrbits_32(RNG_CTRL_REG, RNG_CTRL_MASK);

	/* Reset RNG and RBG */
	mmio_setbits_32(RNG_SOFT_RESET_REG, RNG_SOFT_RESET_MASK);

	/* Take all out of reset */
	mmio_clrbits_32(RNG_SOFT_RESET_REG, RNG_SOFT_RESET_MASK);
}

static void rng_enable(void)
{
	/* Setup RNG. */
	mmio_clrsetbits_32(RNG_CTRL_REG, RNG_CTRL_MASK, RNG_CTRL_ENABLE);
}

int rng_init(void)
{
	rng_reset();

	rng_enable();

	return 0;
}

int rng_read(uint32_t *p_out, uint32_t *words_read)
{
	uint32_t available_words;
	uint32_t i;
	uint32_t word_processed = 0;
	uint32_t wait_count = MAX_WAIT_COUNT_50US;

	if (*words_read == 0) {
		ERROR("RNG Parameter: No word requested\n");
		return -1;
	}

	do {
		available_words = mmio_read_32(RNG_FIFO_COUNT_REG);
		available_words &= RNG_FIFO_COUNT_MASK;

		if (available_words != 0) {
			available_words = MIN(available_words,
					*words_read - word_processed);

			for (i = 0; i < available_words; i++)
				p_out[word_processed + i] =
					mmio_read_32(RNG_FIFO_DATA_REG);
			word_processed += available_words;
		} else {
			udelay(50);
		}

		if (word_processed == *words_read)
			break;

	} while (--wait_count);

	if (word_processed != *words_read) {
		ERROR("RNG Timeout: requested %d word(s) got %d\n",
				*words_read, word_processed);
		*words_read = word_processed;
		return -1;
	}

	return 0;
}

/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <lib/spinlock.h>
#include <plat/common/plat_trng.h>

/*
 * # Entropy pool
 * Note that the TRNG Firmware interface can request up to 192 bits of entropy
 * in a single call or three 64bit words per call. We have 4 words in the pool
 * so that when we have 1-63 bits in the pool, and we have a request for
 * 192 bits of entropy, we don't have to throw out the leftover 1-63 bits of
 * entropy.
 */
#define WORDS_IN_POOL (4)
static uint64_t entropy[WORDS_IN_POOL];
/* index in bits of the first bit of usable entropy */
static uint32_t entropy_bit_index;
/* then number of valid bits in the entropy pool */
static uint32_t entropy_bit_size;

static spinlock_t trng_pool_lock;

#define BITS_PER_WORD (sizeof(entropy[0]) * 8)
#define BITS_IN_POOL (WORDS_IN_POOL * BITS_PER_WORD)
#define ENTROPY_MIN_WORD (entropy_bit_index / BITS_PER_WORD)
#define ENTROPY_FREE_BIT (entropy_bit_size + entropy_bit_index)
#define _ENTROPY_FREE_WORD (ENTROPY_FREE_BIT / BITS_PER_WORD)
#define ENTROPY_FREE_INDEX (_ENTROPY_FREE_WORD % WORDS_IN_POOL)
/* ENTROPY_WORD_INDEX(0) includes leftover bits in the lower bits */
#define ENTROPY_WORD_INDEX(i) ((ENTROPY_MIN_WORD + i) % WORDS_IN_POOL)

/*
 * Fill the entropy pool until we have at least as many bits as requested.
 * Returns true after filling the pool, and false if the entropy source is out
 * of entropy and the pool could not be filled.
 * Assumes locks are taken.
 */
static bool trng_fill_entropy(uint32_t nbits)
{
	while (nbits > entropy_bit_size) {
		bool valid = plat_get_entropy(&entropy[ENTROPY_FREE_INDEX]);

		if (valid) {
			entropy_bit_size += BITS_PER_WORD;
			assert(entropy_bit_size <= BITS_IN_POOL);
		} else {
			return false;
		}
	}
	return true;
}

/*
 * Pack entropy into the out buffer, filling and taking locks as needed.
 * Returns true on success, false on failure.
 *
 * Note: out must have enough space for nbits of entropy
 */
bool trng_pack_entropy(uint32_t nbits, uint64_t *out)
{
	bool success = true;

	spin_lock(&trng_pool_lock);

	if (!trng_fill_entropy(nbits)) {
		success = false;
		goto out;
	}

	const unsigned int rshift = entropy_bit_index % BITS_PER_WORD;
	const unsigned int lshift = BITS_PER_WORD - rshift;
	const int to_fill = ((nbits + BITS_PER_WORD - 1) / BITS_PER_WORD);
	int word_i;

	for (word_i = 0; word_i < to_fill; word_i++) {
		/*
		 * Repack the entropy from the pool into the passed in out
		 * buffer. This takes the lower bits from the valid upper bits
		 * of word_i and the upper bits from the lower bits of
		 * (word_i + 1).
		 *
		 * I found the following diagram useful. note: `e` represents
		 * valid entropy, ` ` represents invalid bits (not entropy) and
		 * `x` represents valid entropy that must not end up in the
		 * packed word.
		 *
		 *          |---------entropy pool----------|
		 * C var    |--(word_i + 1)-|----word_i-----|
		 * bit idx  |7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|
		 *          [x,x,e,e,e,e,e,e|e,e, , , , , , ]
		 *          |   [e,e,e,e,e,e,e,e]           |
		 *          |   |--out[word_i]--|           |
		 *    lshift|---|               |--rshift---|
		 *
		 *          ==== Which is implemented as ====
		 *
		 *          |---------entropy pool----------|
		 * C var    |--(word_i + 1)-|----word_i-----|
		 * bit idx  |7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|
		 *          [x,x,e,e,e,e,e,e|e,e, , , , , , ]
		 * C expr       << lshift       >> rshift
		 * bit idx   5 4 3 2 1 0                 7 6
		 *          [e,e,e,e,e,e,0,0|0,0,0,0,0,0,e,e]
		 *                ==== bit-wise or ====
		 *                   5 4 3 2 1 0 7 6
		 *                  [e,e,e,e,e,e,e,e]
		 */
		out[word_i] = 0;
		out[word_i] |= entropy[ENTROPY_WORD_INDEX(word_i)] >> rshift;

		/*
		 * Note that a shift of 64 bits is treated as a shift of 0 bits.
		 * When the shift amount is the same as the BITS_PER_WORD, we
		 * don't want to include the next word of entropy, so we skip
		 * the `|=` operation.
		 */
		if (lshift != BITS_PER_WORD) {
			out[word_i] |= entropy[ENTROPY_WORD_INDEX(word_i + 1)]
				<< lshift;
		}
	}
	const uint64_t mask = ~0ULL >> (BITS_PER_WORD - (nbits % BITS_PER_WORD));

	out[to_fill - 1] &= mask;

	entropy_bit_index = (entropy_bit_index + nbits) % BITS_IN_POOL;
	entropy_bit_size -= nbits;

out:
	spin_unlock(&trng_pool_lock);

	return success;
}

void trng_entropy_pool_setup(void)
{
	int i;

	for (i = 0; i < WORDS_IN_POOL; i++) {
		entropy[i] = 0;
	}
	entropy_bit_index = 0;
	entropy_bit_size = 0;
}

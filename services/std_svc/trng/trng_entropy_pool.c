/*
 * Copyright (c) 2021-2022, ARM Limited. All rights reserved.
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
#define WORDS_IN_POOL	(4)
static uint64_t entropy[WORDS_IN_POOL];
/* index in bits of the first bit of usable entropy */
static uint32_t entropy_bit_index;
/* then number of valid bits in the entropy pool */
static uint32_t entropy_bit_size;

static spinlock_t trng_pool_lock;

#define BITS_PER_WORD		(sizeof(entropy[0]) * 8)
#define BITS_IN_POOL		(WORDS_IN_POOL * BITS_PER_WORD)
#define ENTROPY_MIN_WORD	(entropy_bit_index / BITS_PER_WORD)
#define ENTROPY_FREE_BIT	(entropy_bit_size + entropy_bit_index)
#define _ENTROPY_FREE_WORD	(ENTROPY_FREE_BIT / BITS_PER_WORD)
#define ENTROPY_FREE_INDEX	(_ENTROPY_FREE_WORD % WORDS_IN_POOL)
/* ENTROPY_WORD_INDEX(0) includes leftover bits in the lower bits */
#define ENTROPY_WORD_INDEX(i)	((ENTROPY_MIN_WORD + i) % WORDS_IN_POOL)

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
	bool ret = true;
	uint32_t bits_to_discard = nbits;
	spin_lock(&trng_pool_lock);

	if (!trng_fill_entropy(nbits)) {
		ret = false;
		goto out;
	}

	const unsigned int rshift = entropy_bit_index % BITS_PER_WORD;
	const unsigned int lshift = BITS_PER_WORD - rshift;
	const int to_fill = ((nbits + BITS_PER_WORD - 1) / BITS_PER_WORD);
	int word_i;

	for (word_i = 0; word_i < to_fill; word_i++) {
		/*
		 * Repack the entropy from the pool into the passed in out
		 * buffer. This takes lesser bits from the valid upper bits
		 * of word_i and more bits from the lower bits of (word_i + 1).
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
		out[word_i] |= entropy[ENTROPY_WORD_INDEX(word_i)] >> rshift;

		/**
		 * Discarding the used/packed entropy bits from the respective
		 * words, (word_i) and (word_i+1) as applicable.
		 * In each iteration of the loop, we pack 64bits of entropy to
		 * the output buffer. The bits are picked linearly starting from
		 * 1st word (entropy[0]) till 4th word (entropy[3]) and then
		 * rolls back (entropy[0]). Discarding of bits is managed
		 * similarly.
		 *
		 * The following diagram illustrates the logic:
		 *
		 *          |---------entropy pool----------|
		 * C var    |--(word_i + 1)-|----word_i-----|
		 * bit idx  |7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|
		 *          [e,e,e,e,e,e,e,e|e,e,0,0,0,0,0,0]
		 *          |   [e,e,e,e,e,e,e,e]           |
		 *          |   |--out[word_i]--|           |
		 *    lshift|---|               |--rshift---|
		 *          |e,e|0,0,0,0,0,0,0,0|0,0,0,0,0,0|
		 *              |<==   ||    ==>|
		 *               bits_to_discard (from these bytes)
		 *
		 * variable(bits_to_discard): Tracks the amount of bits to be
		 * discarded and is updated accordingly in each iteration.
		 *
		 * It monitors these packed bits from respective word_i and
		 * word_i+1 and overwrites them with zeros accordingly.
		 * It discards linearly from the lowest index and moves upwards
		 * until bits_to_discard variable becomes zero.
		 *
		 * In the above diagram,for example, we pack 2bytes(7th and 6th
		 * from word_i) and 6bytes(0th till 5th from word_i+1), combine
		 * and pack them as 64bit to output buffer out[i].
		 * Depending on the number of bits requested, we discard the
		 * bits from these packed bytes by overwriting them with zeros.
		 */

		/*
		 * If the bits to be discarded is lesser than the amount of bits
		 * copied to the output buffer from word_i, we discard that much
		 * amount of bits only.
		 */
		if (bits_to_discard < (BITS_PER_WORD - rshift)) {
			entropy[ENTROPY_WORD_INDEX(word_i)] &=
			(~0ULL << ((bits_to_discard+rshift) % BITS_PER_WORD));
			bits_to_discard = 0;
		} else {
		/*
		 * If the bits to be discarded is more than the amount of valid
		 * upper bits from word_i, which has been copied to the output
		 * buffer, we just set the entire word_i to 0, as the lower bits
		 * will be already zeros from previous operations, and the
		 * bits_to_discard is updated precisely.
		 */
			entropy[ENTROPY_WORD_INDEX(word_i)] = 0;
			bits_to_discard -= (BITS_PER_WORD - rshift);
		}

		/*
		 * Note that a shift of 64 bits is treated as a shift of 0 bits.
		 * When the shift amount is the same as the BITS_PER_WORD, we
		 * don't want to include the next word of entropy, so we skip
		 * the `|=` operation.
		 */
		if (lshift != BITS_PER_WORD) {
			out[word_i] |= entropy[ENTROPY_WORD_INDEX(word_i + 1)]
				<< lshift;
			/**
			 * Discarding the remaining packed bits from upperword
			 * (word[i+1]) which was copied to output buffer by
			 * overwriting with zeros.
			 *
			 * If the remaining bits to be discarded is lesser than
			 * the amount of bits from [word_i+1], which has been
			 * copied to the output buffer, we overwrite that much
			 * amount of bits only.
			 */
			if (bits_to_discard < (BITS_PER_WORD - lshift)) {
				entropy[ENTROPY_WORD_INDEX(word_i+1)]  &=
				(~0ULL << ((bits_to_discard) % BITS_PER_WORD));
				bits_to_discard = 0;
			} else {
			/*
			 * If bits to discard is more than the bits from word_i+1
			 * which got packed into the output, then we discard all
			 * those copied bits.
			 *
			 * Note: we cannot set the entire word_i+1 to 0, as
			 * there are still some unused valid entropy bits at the
			 * upper end for future use.
			 */
				entropy[ENTROPY_WORD_INDEX(word_i+1)]  &=
				(~0ULL << ((BITS_PER_WORD - lshift) % BITS_PER_WORD));
				bits_to_discard -= (BITS_PER_WORD - lshift);
		}

		}
	}

	/* Mask off higher bits if only part of the last word was requested */
	if ((nbits % BITS_PER_WORD) != 0) {
		const uint64_t mask = UINT64_MAX >>
				      (BITS_PER_WORD - (nbits % BITS_PER_WORD));
		out[to_fill - 1] &= mask;
	}

	entropy_bit_index = (entropy_bit_index + nbits) % BITS_IN_POOL;
	entropy_bit_size -= nbits;

out:
	spin_unlock(&trng_pool_lock);

	return ret;
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

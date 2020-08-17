/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>

#include <lib/mmio.h>

#include <qti_rng.h>
#include <qti_rng_io.h>

int qti_rng_get_data(uint8_t *out, uint32_t out_len)
{
	uint32_t tmp_rndm = 0;
	uint32_t bytes_left = out_len;
	int i = 0;

	if (NULL == out || 0 == out_len) {
		return -1;
	}

	/*
	 * RNG HW initialized at previous boot image.
	 * RNG clocks are expected to be ON.
	 */

	do {
		/* There is no data to read */
		if ((mmio_read_32(SEC_PRNG_STATUS) &
		     SEC_PRNG_STATUS_DATA_AVAIL_BMSK) == 0) {
			continue;
		}

		while ((tmp_rndm = mmio_read_32(SEC_PRNG_DATA_OUT)) == 0) {
			;
		}

		for (i = 0; i < 4; i++) {
			*out = (uint8_t) (tmp_rndm >> (8 * i));

			out++;
			bytes_left--;

			if (bytes_left == 0) {
				break;
			}
		}

	} while (bytes_left != 0);

	return 0;
}

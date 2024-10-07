/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

#include "rng_plat.h"

static void trng_external_swrst(void)
{
	/* External swrst to reset whole rng module */
	mmio_setbits_32(TRNG_SWRST_SET_REG, RNG_SWRST_B);
	mmio_setbits_32(TRNG_SWRST_CLR_REG, RNG_SWRST_B);

	/* Disable irq */
	mmio_clrbits_32(RNG_IRQ_CFG, IRQ_EN);
	/* Set default cutoff value */
	mmio_write_32(RNG_HTEST, RNG_DEFAULT_CUTOFF);
	/* Enable rng */
	mmio_setbits_32(RNG_EN, DRBG_EN | NRBG_EN);
}

static bool get_entropy_32(uint32_t *out)
{
	uint64_t time = timeout_init_us(MTK_TIMEOUT_POLL);
	int retry_times = 0;

	while (!(mmio_read_32(RNG_STATUS) & DRBG_VALID)) {
		if (mmio_read_32(RNG_STATUS) & (RNG_ERROR | APB_ERROR)) {
			mmio_clrbits_32(RNG_EN, DRBG_EN | NRBG_EN);

			mmio_clrbits_32(RNG_SWRST, SWRST_B);
			mmio_setbits_32(RNG_SWRST, SWRST_B);

			mmio_setbits_32(RNG_EN, DRBG_EN | NRBG_EN);
		}

		if (timeout_elapsed(time)) {
			trng_external_swrst();
			time = timeout_init_us(MTK_TIMEOUT_POLL);
			retry_times++;
		}

		if (retry_times > MTK_RETRY_CNT) {
			ERROR("%s: trng NOT ready\n", __func__);
			return false;
		}
	}

	*out = mmio_read_32(RNG_OUT);

	return true;
}

/* Get random number from HWRNG and return 8 bytes of entropy.
 * Return 'true' when random value generated successfully, otherwise return
 * 'false'.
 */
bool plat_get_entropy(uint64_t *out)
{
	uint32_t seed[2] = { 0 };
	int i = 0;

	assert(out);
	assert(!check_uptr_overflow((uintptr_t)out, sizeof(*out)));

	/* Disable interrupt mode */
	mmio_clrbits_32(RNG_IRQ_CFG, IRQ_EN);
	/* Set rng health test cutoff value */
	mmio_write_32(RNG_HTEST, RNG_DEFAULT_CUTOFF);
	/* Enable rng module */
	mmio_setbits_32(RNG_EN, DRBG_EN | NRBG_EN);

	for (i = 0; i < ARRAY_SIZE(seed); i++) {
		if (!get_entropy_32(&seed[i]))
			return false;
	}

	/* Output 8 bytes entropy by combining 2 32-bit random numbers. */
	*out = ((uint64_t)seed[0] << 32) | seed[1];

	return true;
}

/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

#include <mtk_mmap_pool.h>
#include <mtk_sip_svc.h>
#include "rng_plat.h"

static spinlock_t rng_lock;

static int trng_wait(uint32_t reg, uint32_t expected_value)
{
	uint64_t timeout = timeout_init_us(TRNG_TIME_OUT);
	uint32_t value = 0;

	do {
		value = mmio_read_32(reg);
		if ((value & expected_value) == expected_value)
			return 0;

		udelay(10);
	} while (!timeout_elapsed(timeout));

	return -ETIMEDOUT;
}

static int trng_write(uint32_t reg, uint32_t value,
			   uint32_t read_reg, uint32_t expected_value)
{
	int retry = MTK_TRNG_MAX_ROUND;
	uint32_t read_value = 0;

	do {
		mmio_write_32(reg, value);

		read_value = mmio_read_32(read_reg);
		if ((read_value & value) == expected_value)
			return 0;

		udelay(10);
	} while (--retry > 0);

	return -ETIMEDOUT;
}

static uint32_t trng_prng(uint32_t *rand)
{
	int32_t ret = 0;
	uint32_t seed[4] = {0};

	if (rand == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	/* ungate */
	ret = trng_write(TRNG_PDN_CLR, TRNG_PDN_VALUE, TRNG_PDN_STATUS, 0);
	if (ret) {
		ERROR("%s: ungate fail\n", __func__);
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	/* read random data once and drop it */
	seed[0] = mmio_read_32(TRNG_DATA);

	/* enable von-neumann extractor */
	mmio_setbits_32(TRNG_CONF, TRNG_CONF_VON_EN);

	/* start */
	mmio_setbits_32(TRNG_CTRL, TRNG_CTRL_START);

	/* get seeds from trng */
	for (int i = 0; i < ARRAY_SIZE(seed); i++) {
		ret = trng_wait(TRNG_CTRL, TRNG_CTRL_RDY);
		if (ret) {
			ERROR("%s: trng NOT ready\n", __func__);
			return MTK_SIP_E_NOT_SUPPORTED;
		}

		seed[i] = mmio_read_32(TRNG_DATA);
	}

	/* stop */
	mmio_clrbits_32(TRNG_CTRL, TRNG_CTRL_START);

	/* gate */
	ret = trng_write(TRNG_PDN_SET, TRNG_PDN_VALUE, TRNG_PDN_STATUS, TRNG_PDN_VALUE);
	if (ret) {
		ERROR("%s: gate fail\n", __func__);
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	for (int i = 0; i < ARRAY_SIZE(seed); i++)
		rand[i] = seed[i];

	return 0;
}

static uint32_t get_true_rnd(uint32_t *val, uint32_t num)
{
	uint32_t rand[4] = {0};
	uint32_t ret;

	if (val == NULL || num > ARRAY_SIZE(rand))
		return MTK_SIP_E_INVALID_PARAM;

	spin_lock(&rng_lock);
	ret = trng_prng(rand);
	spin_unlock(&rng_lock);

	for (int i = 0; i < num; i++)
		val[i] = rand[i];

	return ret;
}

/*
 * plat_get_entropy - get 64-bit random number data which is used form
 * atf early stage
 * output - out: output 64-bit entropy combine with 2 32-bit random number
 */
bool plat_get_entropy(uint64_t *out)
{
	uint32_t entropy_pool[2] = {0};
	uint32_t ret;

	assert(out);
	assert(!check_uptr_overflow((uintptr_t)out, sizeof(*out)));

	/* Get 2 32-bits entropy */
	ret = get_true_rnd(entropy_pool, ARRAY_SIZE(entropy_pool));
	if (ret)
		return false;

	/* Output 8 bytes entropy combine with 2 32-bit random number. */
	*out = ((uint64_t)entropy_pool[0] << 32) | entropy_pool[1];

	return true;
}

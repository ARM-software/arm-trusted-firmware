/*
 * Copyright (c) 2019-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/sha_common_macros.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_hash.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

#if STM32_HASH_VER == 2
#define DT_HASH_COMPAT			"st,stm32f756-hash"
#endif
#if STM32_HASH_VER == 4
#define DT_HASH_COMPAT			"st,stm32mp13-hash"
#endif

#define HASH_CR				0x00U
#define HASH_DIN			0x04U
#define HASH_STR			0x08U
#define HASH_SR				0x24U
#define HASH_HREG(x)			(0x310U + ((x) * 0x04U))

/* Control Register */
#define HASH_CR_INIT			BIT(2)
#define HASH_CR_DATATYPE_SHIFT		U(4)
#if STM32_HASH_VER == 2
#define HASH_CR_ALGO_SHA1		0x0U
#define HASH_CR_ALGO_MD5		BIT(7)
#define HASH_CR_ALGO_SHA224		BIT(18)
#define HASH_CR_ALGO_SHA256		(BIT(18) | BIT(7))
#endif
#if STM32_HASH_VER == 4
#define HASH_CR_ALGO_SHIFT		U(17)
#define HASH_CR_ALGO_SHA1		(0x0U << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA224		(0x2U << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA256		(0x3U << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA384		(0xCU << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA512_224		(0xDU << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA512_256		(0xEU << HASH_CR_ALGO_SHIFT)
#define HASH_CR_ALGO_SHA512		(0xFU << HASH_CR_ALGO_SHIFT)
#endif

/* Status Flags */
#define HASH_SR_DCIS			BIT(1)
#define HASH_SR_BUSY			BIT(3)

/* STR Register */
#define HASH_STR_NBLW_MASK		GENMASK(4, 0)
#define HASH_STR_DCAL			BIT(8)

#define RESET_TIMEOUT_US_1MS		1000U
#define HASH_TIMEOUT_US			10000U

enum stm32_hash_data_format {
	HASH_DATA_32_BITS,
	HASH_DATA_16_BITS,
	HASH_DATA_8_BITS,
	HASH_DATA_1_BIT
};

struct stm32_hash_instance {
	uintptr_t base;
	unsigned int clock;
	size_t digest_size;
};

struct stm32_hash_remain {
	uint32_t buffer;
	size_t length;
};

/* Expect a single HASH peripheral */
static struct stm32_hash_instance stm32_hash;
static struct stm32_hash_remain stm32_remain;

static uintptr_t hash_base(void)
{
	return stm32_hash.base;
}

static int hash_wait_busy(void)
{
	uint64_t timeout = timeout_init_us(HASH_TIMEOUT_US);

	while ((mmio_read_32(hash_base() + HASH_SR) & HASH_SR_BUSY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%s: busy timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int hash_wait_computation(void)
{
	uint64_t timeout = timeout_init_us(HASH_TIMEOUT_US);

	while ((mmio_read_32(hash_base() + HASH_SR) & HASH_SR_DCIS) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%s: busy timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int hash_write_data(uint32_t data)
{
	int ret;

	ret = hash_wait_busy();
	if (ret != 0) {
		return ret;
	}

	mmio_write_32(hash_base() + HASH_DIN, data);

	return 0;
}

static void hash_hw_init(enum stm32_hash_algo_mode mode)
{
	uint32_t reg;

	reg = HASH_CR_INIT | (HASH_DATA_8_BITS << HASH_CR_DATATYPE_SHIFT);

	switch (mode) {
#if STM32_HASH_VER == 2
	case HASH_MD5SUM:
		reg |= HASH_CR_ALGO_MD5;
		stm32_hash.digest_size = MD5_DIGEST_SIZE;
		break;
#endif
	case HASH_SHA1:
		reg |= HASH_CR_ALGO_SHA1;
		stm32_hash.digest_size = SHA1_DIGEST_SIZE;
		break;
	case HASH_SHA224:
		reg |= HASH_CR_ALGO_SHA224;
		stm32_hash.digest_size = SHA224_DIGEST_SIZE;
		break;
#if STM32_HASH_VER == 4
	case HASH_SHA384:
		reg |= HASH_CR_ALGO_SHA384;
		stm32_hash.digest_size = SHA384_DIGEST_SIZE;
		break;
	case HASH_SHA512:
		reg |= HASH_CR_ALGO_SHA512;
		stm32_hash.digest_size = SHA512_DIGEST_SIZE;
		break;
#endif
	/* Default selected algo is SHA256 */
	case HASH_SHA256:
	default:
		reg |= HASH_CR_ALGO_SHA256;
		stm32_hash.digest_size = SHA256_DIGEST_SIZE;
		break;
	}

	mmio_write_32(hash_base() + HASH_CR, reg);
}

static int hash_get_digest(uint8_t *digest)
{
	int ret;
	uint32_t i;
	uint32_t dsg;

	ret = hash_wait_computation();
	if (ret != 0) {
		return ret;
	}

	for (i = 0U; i < (stm32_hash.digest_size / sizeof(uint32_t)); i++) {
		dsg = __builtin_bswap32(mmio_read_32(hash_base() +
						     HASH_HREG(i)));
		memcpy(digest + (i * sizeof(uint32_t)), &dsg, sizeof(uint32_t));
	}

	/*
	 * Clean hardware context as HASH could be used later
	 * by non-secure software
	 */
	hash_hw_init(HASH_SHA256);

	return 0;
}

int stm32_hash_update(const uint8_t *buffer, size_t length)
{
	size_t remain_length = length;
	int ret = 0;

	if ((length == 0U) || (buffer == NULL)) {
		return 0;
	}

	clk_enable(stm32_hash.clock);

	if (stm32_remain.length != 0U) {
		uint32_t copysize;

		copysize = MIN((sizeof(uint32_t) - stm32_remain.length),
			       length);
		memcpy(((uint8_t *)&stm32_remain.buffer) + stm32_remain.length,
		       buffer, copysize);
		remain_length -= copysize;
		buffer += copysize;
		if (stm32_remain.length == sizeof(uint32_t)) {
			ret = hash_write_data(stm32_remain.buffer);
			if (ret != 0) {
				goto exit;
			}

			zeromem(&stm32_remain, sizeof(stm32_remain));
		}
	}

	while (remain_length / sizeof(uint32_t) != 0U) {
		uint32_t tmp_buf;

		memcpy(&tmp_buf, buffer, sizeof(uint32_t));
		ret = hash_write_data(tmp_buf);
		if (ret != 0) {
			goto exit;
		}

		buffer += sizeof(uint32_t);
		remain_length -= sizeof(uint32_t);
	}

	if (remain_length != 0U) {
		assert(stm32_remain.length == 0U);

		memcpy((uint8_t *)&stm32_remain.buffer, buffer, remain_length);
		stm32_remain.length = remain_length;
	}

exit:
	clk_disable(stm32_hash.clock);

	return ret;
}

int stm32_hash_final(uint8_t *digest)
{
	int ret;

	clk_enable(stm32_hash.clock);

	if (stm32_remain.length != 0U) {
		ret = hash_write_data(stm32_remain.buffer);
		if (ret != 0) {
			clk_disable(stm32_hash.clock);
			return ret;
		}

		mmio_clrsetbits_32(hash_base() + HASH_STR, HASH_STR_NBLW_MASK,
				   8U * stm32_remain.length);
		zeromem(&stm32_remain, sizeof(stm32_remain));
	} else {
		mmio_clrbits_32(hash_base() + HASH_STR, HASH_STR_NBLW_MASK);
	}

	mmio_setbits_32(hash_base() + HASH_STR, HASH_STR_DCAL);

	ret = hash_get_digest(digest);

	clk_disable(stm32_hash.clock);

	return ret;
}

int stm32_hash_final_update(const uint8_t *buffer, uint32_t length,
			    uint8_t *digest)
{
	int ret;

	ret = stm32_hash_update(buffer, length);
	if (ret != 0) {
		return ret;
	}

	return stm32_hash_final(digest);
}

void stm32_hash_init(enum stm32_hash_algo_mode mode)
{
	clk_enable(stm32_hash.clock);

	hash_hw_init(mode);

	clk_disable(stm32_hash.clock);

	zeromem(&stm32_remain, sizeof(stm32_remain));
}

int stm32_hash_register(void)
{
	struct dt_node_info hash_info;
	int node;

	for (node = dt_get_node(&hash_info, -1, DT_HASH_COMPAT);
	     node != -FDT_ERR_NOTFOUND;
	     node = dt_get_node(&hash_info, node, DT_HASH_COMPAT)) {
		if (hash_info.status != DT_DISABLED) {
			break;
		}
	}

	if (node == -FDT_ERR_NOTFOUND) {
		return -ENODEV;
	}

	if (hash_info.clock < 0) {
		return -EINVAL;
	}

	stm32_hash.base = hash_info.base;
	stm32_hash.clock = hash_info.clock;

	clk_enable(stm32_hash.clock);

	if (hash_info.reset >= 0) {
		uint32_t id = (uint32_t)hash_info.reset;

		if (stm32mp_reset_assert(id, RESET_TIMEOUT_US_1MS) != 0) {
			panic();
		}
		udelay(20);
		if (stm32mp_reset_deassert(id, RESET_TIMEOUT_US_1MS) != 0) {
			panic();
		}
	}

	clk_disable(stm32_hash.clock);

	return 0;
}

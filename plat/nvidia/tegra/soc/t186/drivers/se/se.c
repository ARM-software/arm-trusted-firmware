/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <string.h>

#include <bpmp_ipc.h>
#include <pmc.h>
#include <security_engine.h>
#include <tegra_private.h>

#include "se_private.h"

/*******************************************************************************
 * Constants and Macros
 ******************************************************************************/
#define SE0_MAX_BUSY_TIMEOUT_MS		U(100)	/* 100ms */
#define BYTES_IN_WORD			U(4)
#define SHA256_MAX_HASH_RESULT		U(7)
#define SHA256_DST_SIZE			U(32)
#define SHA_FIRST_OP			U(1)
#define MAX_SHA_ENGINE_CHUNK_SIZE	U(0xFFFFFF)
#define SHA256_MSG_LENGTH_ONETIME	U(0xffff)

/*
 * Check that SE operation has completed after kickoff
 * This function is invoked after an SE operation has been started,
 * and it checks the following conditions:
 * 1. SE0_INT_STATUS = SE0_OP_DONE
 * 2. SE0_STATUS = IDLE
 * 3. SE0_ERR_STATUS is clean.
 */
static int32_t tegra_se_operation_complete(void)
{
	uint32_t val = 0U;

	/* Read SE0 interrupt register to ensure H/W operation complete */
	val = tegra_se_read_32(SE0_INT_STATUS_REG_OFFSET);
	if (SE0_INT_OP_DONE(val) == SE0_INT_OP_DONE_CLEAR) {
		ERROR("%s: Engine busy state too many times! val = 0x%x\n",
			__func__, val);
		return -ETIMEDOUT;
	}

	/* Read SE0 status idle to ensure H/W operation complete */
	val = tegra_se_read_32(SE0_SHA_STATUS_0);
	if (val != SE0_SHA_STATUS_IDLE) {
		ERROR("%s: Idle state timeout! val = 0x%x\n", __func__,
			val);
		return -ETIMEDOUT;
	}

	/* Ensure that no errors are thrown during operation */
	val = tegra_se_read_32(SE0_ERR_STATUS_REG_OFFSET);
	if (val != SE0_ERR_STATUS_CLEAR) {
		ERROR("%s: Error during SE operation! val = 0x%x",
			__func__, val);
		return -ENOTSUP;
	}

	return 0;
}

/*
 * Security engine primitive normal operations
 */
static int32_t tegra_se_start_normal_operation(uint64_t src_addr,
		uint32_t nbytes, uint32_t last_buf, uint32_t src_len_inbytes)
{
	int32_t ret = 0;
	uint32_t val = 0U;
	uint32_t src_in_lo;
	uint32_t src_in_msb;
	uint32_t src_in_hi;

	if ((src_addr == 0UL) || (nbytes == 0U))
		return -EINVAL;

	src_in_lo = (uint32_t)src_addr;
	src_in_msb = ((uint32_t)(src_addr >> 32U) & 0xffU);
	src_in_hi = ((src_in_msb << SE0_IN_HI_ADDR_HI_0_MSB_SHIFT) |
				(nbytes & 0xffffffU));

	/* set SRC_IN_ADDR_LO and SRC_IN_ADDR_HI*/
	tegra_se_write_32(SE0_IN_ADDR, src_in_lo);
	tegra_se_write_32(SE0_IN_HI_ADDR_HI, src_in_hi);

	val = tegra_se_read_32(SE0_INT_STATUS_REG_OFFSET);
	if (val > 0U) {
		tegra_se_write_32(SE0_INT_STATUS_REG_OFFSET, 0x00000U);
	}

	/* Enable SHA interrupt for SE0 Operation */
	tegra_se_write_32(SE0_SHA_INT_ENABLE, 0x1aU);

	/* flush to DRAM for SE to use the updated contents */
	flush_dcache_range(src_addr, src_len_inbytes);

	/* Start SHA256 operation */
	if (last_buf == 1U) {
		tegra_se_write_32(SE0_OPERATION_REG_OFFSET, SE0_OP_START |
				SE0_UNIT_OPERATION_PKT_LASTBUF_FIELD);
	} else {
		tegra_se_write_32(SE0_OPERATION_REG_OFFSET, SE0_OP_START);
	}

	/* Wait for SE-operation to finish */
	udelay(SE0_MAX_BUSY_TIMEOUT_MS * 100U);

	/* Check SE0 operation status */
	ret = tegra_se_operation_complete();
	if (ret != 0) {
		ERROR("SE operation complete Failed! 0x%x", ret);
		return ret;
	}

	return 0;
}

static int32_t tegra_se_calculate_sha256_hash(uint64_t src_addr,
						uint32_t src_len_inbyte)
{
	uint32_t val, last_buf, i;
	int32_t ret = 0;
	uint32_t operations;
	uint64_t src_len_inbits;
	uint32_t len_bits_msb;
	uint32_t len_bits_lsb;
	uint32_t number_of_operations, max_bytes, bytes_left, remaining_bytes;

	if (src_len_inbyte > MAX_SHA_ENGINE_CHUNK_SIZE) {
		ERROR("SHA input chunk size too big: 0x%x\n", src_len_inbyte);
		return -EINVAL;
	}

	if (src_addr == 0UL) {
		return -EINVAL;
	}

	/* number of bytes per operation */
	max_bytes = SHA256_HASH_SIZE_BYTES * SHA256_MSG_LENGTH_ONETIME;

	src_len_inbits = src_len_inbyte * 8U;
	len_bits_msb = (uint32_t)(src_len_inbits >> 32U);
	len_bits_lsb = (uint32_t)(src_len_inbits & 0xFFFFFFFF);

	/* program SE0_CONFIG for SHA256 operation */
	val = SE0_CONFIG_ENC_ALG_SHA | SE0_CONFIG_ENC_MODE_SHA256 |
		SE0_CONFIG_DEC_ALG_NOP | SE0_CONFIG_DST_HASHREG;
	tegra_se_write_32(SE0_SHA_CONFIG, val);

	/* set SE0_SHA_MSG_LENGTH registers */
	tegra_se_write_32(SE0_SHA_MSG_LENGTH_0, len_bits_lsb);
	tegra_se_write_32(SE0_SHA_MSG_LEFT_0, len_bits_lsb);
	tegra_se_write_32(SE0_SHA_MSG_LENGTH_1, len_bits_msb);

	/* zero out unused SE0_SHA_MSG_LENGTH and SE0_SHA_MSG_LEFT */
	tegra_se_write_32(SE0_SHA_MSG_LENGTH_2, 0U);
	tegra_se_write_32(SE0_SHA_MSG_LENGTH_3, 0U);
	tegra_se_write_32(SE0_SHA_MSG_LEFT_1, 0U);
	tegra_se_write_32(SE0_SHA_MSG_LEFT_2, 0U);
	tegra_se_write_32(SE0_SHA_MSG_LEFT_3, 0U);

	number_of_operations = src_len_inbyte / max_bytes;
	remaining_bytes = src_len_inbyte % max_bytes;
	if (remaining_bytes > 0U) {
		number_of_operations += 1U;
	}

	/*
	 * 1. Operations == 1:	program SE0_SHA_TASK register to initiate SHA256
	 *			hash generation by setting
	 *			1(SE0_SHA_CONFIG_HW_INIT_HASH) to SE0_SHA_TASK
	 *			and start SHA256-normal operation.
	 * 2. 1 < Operations < number_of_operations: program SE0_SHA_TASK to
	 *			0(SE0_SHA_CONFIG_HW_INIT_HASH_DISABLE) to load
	 *			intermediate SHA256 digest result from
	 *			HASH_RESULT register to continue SHA256
	 *			generation and start SHA256-normal operation.
	 * 3. Operations == number_of_operations: continue with step 2 and set
	 *			max_bytes to bytes_left to process final
	 *			hash-result generation and
	 *			start SHA256-normal operation.
	 */
	bytes_left = src_len_inbyte;
	for (operations = 1U; operations <= number_of_operations;
								operations++) {
		if (operations == SHA_FIRST_OP) {
			val = SE0_SHA_CONFIG_HW_INIT_HASH;
		} else {
			/* Load intermediate SHA digest result to
			 * SHA:HASH_RESULT(0..7) to continue the SHA
			 * calculation and tell the SHA engine to use it.
			 */
			for (i = 0U; (i / BYTES_IN_WORD) <=
				SHA256_MAX_HASH_RESULT; i += BYTES_IN_WORD) {
				val = tegra_se_read_32(SE0_SHA_HASH_RESULT_0 +
									i);
				tegra_se_write_32(SE0_SHA_HASH_RESULT_0 + i,
									val);
			}
			val = SE0_SHA_CONFIG_HW_INIT_HASH_DISABLE;
			if (len_bits_lsb <= (max_bytes * 8U)) {
				len_bits_lsb = (remaining_bytes * 8U);
			} else {
				len_bits_lsb -= (max_bytes * 8U);
			}
			tegra_se_write_32(SE0_SHA_MSG_LEFT_0, len_bits_lsb);
		}
		tegra_se_write_32(SE0_SHA_TASK_CONFIG, val);

		max_bytes = (SHA256_HASH_SIZE_BYTES *
						SHA256_MSG_LENGTH_ONETIME);
		if (bytes_left < max_bytes) {
			max_bytes = bytes_left;
			last_buf = 1U;
		} else {
			bytes_left = bytes_left - max_bytes;
			last_buf = 0U;
		}
		/* start operation */
		ret = tegra_se_start_normal_operation(src_addr, max_bytes,
					last_buf, src_len_inbyte);
		if (ret != 0) {
			ERROR("Error during SE operation! 0x%x", ret);
			return -EINVAL;
		}
	}

	return ret;
}

/*
 * Handler to generate SHA256 and save SHA256 hash to PMC-Scratch register.
 */
int32_t tegra_se_save_sha256_hash(uint64_t bl31_base, uint32_t src_len_inbyte)
{
	int32_t ret = 0;
	uint32_t val = 0U, hash_offset = 0U, scratch_offset = 0U, security;

	/*
	 * Set SE_SOFT_SETTINGS=SE_SECURE to prevent NS process to change SE
	 * registers.
	 */
	security = tegra_se_read_32(SE0_SECURITY);
	tegra_se_write_32(SE0_SECURITY, security | SE0_SECURITY_SE_SOFT_SETTING);

	ret = tegra_se_calculate_sha256_hash(bl31_base, src_len_inbyte);
	if (ret != 0L) {
		ERROR("%s: SHA256 generation failed\n", __func__);
		return ret;
	}

	/*
	 * Reset SE_SECURE to previous value.
	 */
	tegra_se_write_32(SE0_SECURITY, security);

	/* read SHA256_HASH_RESULT and save to PMC Scratch registers */
	scratch_offset = SECURE_SCRATCH_TZDRAM_SHA256_HASH_START;
	while (scratch_offset <= SECURE_SCRATCH_TZDRAM_SHA256_HASH_END) {

		val = tegra_se_read_32(SE0_SHA_HASH_RESULT_0 + hash_offset);
		mmio_write_32(TEGRA_SCRATCH_BASE + scratch_offset, val);

		hash_offset += BYTES_IN_WORD;
		scratch_offset += BYTES_IN_WORD;
	}

	return ret;
}


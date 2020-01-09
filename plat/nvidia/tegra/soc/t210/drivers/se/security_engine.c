/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <se_private.h>
#include <security_engine.h>
#include <tegra_platform.h>

/*******************************************************************************
 * Constants and Macros
 ******************************************************************************/

#define TIMEOUT_100MS	100U	// Timeout in 100ms
#define RNG_AES_KEY_INDEX   1

/*******************************************************************************
 * Data structure and global variables
 ******************************************************************************/

/* The security engine contexts are formatted as follows:
 *
 * SE1 CONTEXT:
 * #--------------------------------#
 * |        Random Data   1 Block   |
 * #--------------------------------#
 * |        Sticky Bits   2 Blocks  |
 * #--------------------------------#
 * | Key Table           64 Blocks  |
 * |     For each Key (x16):        |
 * |      Key:         2 Blocks     |
 * |      Original-IV: 1 Block      |
 * |      Updated-IV:  1 Block      |
 * #--------------------------------#
 * |        RSA Keys     64 Blocks  |
 * #--------------------------------#
 * |        Known Pattern 1 Block   |
 * #--------------------------------#
 *
 * SE2/PKA1 CONTEXT:
 * #--------------------------------#
 * |        Random Data   1 Block   |
 * #--------------------------------#
 * |        Sticky Bits   2 Blocks  |
 * #--------------------------------#
 * | Key Table           64 Blocks  |
 * |     For each Key (x16):        |
 * |      Key:         2 Blocks     |
 * |      Original-IV: 1 Block      |
 * |      Updated-IV:  1 Block      |
 * #--------------------------------#
 * |        RSA Keys     64 Blocks  |
 * #--------------------------------#
 * |        PKA sticky bits 1 Block |
 * #--------------------------------#
 * |        PKA keys    512 Blocks  |
 * #--------------------------------#
 * |        Known Pattern 1 Block   |
 * #--------------------------------#
 */

/* Known pattern data */
static const uint32_t se_ctx_known_pattern_data[SE_CTX_KNOWN_PATTERN_SIZE_WORDS] = {
	/* 128 bit AES block */
	0x0C0D0E0F,
	0x08090A0B,
	0x04050607,
	0x00010203,
};

/* SE input and output linked list buffers */
static tegra_se_io_lst_t se1_src_ll_buf;
static tegra_se_io_lst_t se1_dst_ll_buf;

/* SE2 input and output linked list buffers */
static tegra_se_io_lst_t se2_src_ll_buf;
static tegra_se_io_lst_t se2_dst_ll_buf;

/* SE1 security engine device handle */
static tegra_se_dev_t se_dev_1 = {
	.se_num = 1,
	/* Setup base address for se */
	.se_base = TEGRA_SE1_BASE,
	/* Setup context size in AES blocks */
	.ctx_size_blks = SE_CTX_SAVE_SIZE_BLOCKS_SE1,
	/* Setup SRC buffers for SE operations */
	.src_ll_buf = &se1_src_ll_buf,
	/* Setup DST buffers for SE operations */
	.dst_ll_buf = &se1_dst_ll_buf,
	/* Setup context save destination */
	.ctx_save_buf = (uint32_t *)(TEGRA_TZRAM_CARVEOUT_BASE),
};

/* SE2 security engine device handle */
static tegra_se_dev_t se_dev_2 = {
	.se_num = 2,
	/* Setup base address for se */
	.se_base = TEGRA_SE2_BASE,
	/* Setup context size in AES blocks */
	.ctx_size_blks = SE_CTX_SAVE_SIZE_BLOCKS_SE2,
	/* Setup SRC buffers for SE operations */
	.src_ll_buf = &se2_src_ll_buf,
	/* Setup DST buffers for SE operations */
	.dst_ll_buf = &se2_dst_ll_buf,
	/* Setup context save destination */
	.ctx_save_buf = (uint32_t *)(TEGRA_TZRAM_CARVEOUT_BASE + 0x1000),
};

static bool ecid_valid;

/*******************************************************************************
 * Functions Definition
 ******************************************************************************/

static void tegra_se_make_data_coherent(const tegra_se_dev_t *se_dev)
{
	flush_dcache_range(((uint64_t)(se_dev->src_ll_buf)),
			sizeof(tegra_se_io_lst_t));
	flush_dcache_range(((uint64_t)(se_dev->dst_ll_buf)),
			sizeof(tegra_se_io_lst_t));
}

/*
 * Check that SE operation has completed after kickoff
 * This function is invoked after an SE operation has been started,
 * and it checks the following conditions:
 * 1. SE_INT_STATUS = SE_OP_DONE
 * 2. SE_STATUS = IDLE
 * 3. AHB bus data transfer complete.
 * 4. SE_ERR_STATUS is clean.
 */
static int32_t tegra_se_operation_complete(const tegra_se_dev_t *se_dev)
{
	uint32_t val = 0;
	int32_t ret = 0;
	uint32_t timeout;

	/* Poll the SE interrupt register to ensure H/W operation complete */
	val = tegra_se_read_32(se_dev, SE_INT_STATUS_REG_OFFSET);
	for (timeout = 0; (SE_INT_OP_DONE(val) == SE_INT_OP_DONE_CLEAR) &&
			(timeout < TIMEOUT_100MS); timeout++) {
		mdelay(1);
		val = tegra_se_read_32(se_dev, SE_INT_STATUS_REG_OFFSET);
	}

	if (timeout == TIMEOUT_100MS) {
		ERROR("%s: ERR: Atomic context save operation timeout!\n",
				__func__);
		ret = -ETIMEDOUT;
	}

	/* Poll the SE status idle to ensure H/W operation complete */
	if (ret == 0) {
		val = tegra_se_read_32(se_dev, SE_STATUS_OFFSET);
		for (timeout = 0; (val != 0U) && (timeout < TIMEOUT_100MS);
				timeout++) {
			mdelay(1);
			val = tegra_se_read_32(se_dev, SE_STATUS_OFFSET);
		}

		if (timeout == TIMEOUT_100MS) {
			ERROR("%s: ERR: MEM_INTERFACE and SE state "
					"idle state timeout.\n", __func__);
			ret = -ETIMEDOUT;
		}
	}

	/* Check AHB bus transfer complete */
	if (ret == 0) {
		val = mmio_read_32(TEGRA_AHB_ARB_BASE + ARAHB_MEM_WRQUE_MST_ID_OFFSET);
		for (timeout = 0; ((val & (ARAHB_MST_ID_SE_MASK | ARAHB_MST_ID_SE2_MASK)) != 0U) &&
				(timeout < TIMEOUT_100MS); timeout++) {
			mdelay(1);
			val = mmio_read_32(TEGRA_AHB_ARB_BASE + ARAHB_MEM_WRQUE_MST_ID_OFFSET);
		}

		if (timeout == TIMEOUT_100MS) {
			ERROR("%s: SE write over AHB timeout.\n", __func__);
			ret = -ETIMEDOUT;
		}
	}

	/* Ensure that no errors are thrown during operation */
	if (ret == 0) {
		val = tegra_se_read_32(se_dev, SE_ERR_STATUS_REG_OFFSET);
		if (val != 0U) {
			ERROR("%s: error during SE operation! 0x%x", __func__, val);
			ret = -ENOTSUP;
		}
	}

	return ret;
}

/*
 * Returns true if the SE engine is configured to perform SE context save in
 * hardware.
 */
static inline bool tegra_se_atomic_save_enabled(const tegra_se_dev_t *se_dev)
{
	uint32_t val;

	val = tegra_se_read_32(se_dev, SE_CTX_SAVE_AUTO_REG_OFFSET);
	return (SE_CTX_SAVE_AUTO_ENABLE(val) == SE_CTX_SAVE_AUTO_EN);
}

/*
 * Wait for SE engine to be idle and clear pending interrupts before
 * starting the next SE operation.
 */
static int32_t tegra_se_operation_prepare(const tegra_se_dev_t *se_dev)
{
	int32_t ret = 0;
	uint32_t val = 0;
	uint32_t timeout;

	/* Wait for previous operation to finish */
	val = tegra_se_read_32(se_dev, SE_STATUS_OFFSET);
	for (timeout = 0; (val != 0U) && (timeout < TIMEOUT_100MS); timeout++) {
		mdelay(1);
		val = tegra_se_read_32(se_dev, SE_STATUS_OFFSET);
	}

	if (timeout == TIMEOUT_100MS) {
		ERROR("%s: ERR: SE status is not idle!\n", __func__);
		ret = -ETIMEDOUT;
	}

	/* Clear any pending interrupts from  previous operation */
	val = tegra_se_read_32(se_dev, SE_INT_STATUS_REG_OFFSET);
	tegra_se_write_32(se_dev, SE_INT_STATUS_REG_OFFSET, val);
	return ret;
}

/*
 * SE atomic context save. At SC7 entry, SE driver triggers the
 * hardware automatically performs the context save operation.
 */
static int32_t tegra_se_context_save_atomic(const tegra_se_dev_t *se_dev)
{
	int32_t ret = 0;
	uint32_t val = 0;
	uint32_t blk_count_limit = 0;
	uint32_t block_count;

	/* Check that previous operation is finalized */
	ret = tegra_se_operation_prepare(se_dev);

	/* Read the context save progress counter: block_count
	 * Ensure no previous context save has been triggered
	 * SE_CTX_SAVE_AUTO.CURR_CNT == 0
	 */
	if (ret == 0) {
		val = tegra_se_read_32(se_dev, SE_CTX_SAVE_AUTO_REG_OFFSET);
		block_count = SE_CTX_SAVE_GET_BLK_COUNT(val);
		if (block_count != 0U) {
			ERROR("%s: ctx_save triggered multiple times\n",
					__func__);
			ret = -EALREADY;
		}
	}

	/* Set the destination block count when the context save complete */
	if (ret == 0) {
		blk_count_limit = block_count + se_dev->ctx_size_blks;
	}

	/* Program SE_CONFIG register as for RNG operation
	 * SE_CONFIG.ENC_ALG = RNG
	 * SE_CONFIG.DEC_ALG = NOP
	 * SE_CONFIG.ENC_MODE is ignored
	 * SE_CONFIG.DEC_MODE is ignored
	 * SE_CONFIG.DST = MEMORY
	 */
	if (ret == 0) {
		val = (SE_CONFIG_ENC_ALG_RNG |
			SE_CONFIG_DEC_ALG_NOP |
			SE_CONFIG_DST_MEMORY);
		tegra_se_write_32(se_dev, SE_CONFIG_REG_OFFSET, val);

		tegra_se_make_data_coherent(se_dev);

		/* SE_CTX_SAVE operation */
		tegra_se_write_32(se_dev, SE_OPERATION_REG_OFFSET,
				SE_OP_CTX_SAVE);

		ret = tegra_se_operation_complete(se_dev);
	}

	/* Check that context has written the correct number of blocks */
	if (ret == 0) {
		val = tegra_se_read_32(se_dev, SE_CTX_SAVE_AUTO_REG_OFFSET);
		if (SE_CTX_SAVE_GET_BLK_COUNT(val) != blk_count_limit) {
			ERROR("%s: expected %d blocks but %d were written\n",
					__func__, blk_count_limit, val);
			ret = -ECANCELED;
		}
	}

	return ret;
}

/*
 * Security engine primitive operations, including normal operation
 * and the context save operation.
 */
static int tegra_se_perform_operation(const tegra_se_dev_t *se_dev, uint32_t nbytes,
					bool context_save)
{
	uint32_t nblocks = nbytes / TEGRA_SE_AES_BLOCK_SIZE;
	int ret = 0;

	assert(se_dev);

	/* Use device buffers for in and out */
	tegra_se_write_32(se_dev, SE_OUT_LL_ADDR_REG_OFFSET, ((uint64_t)(se_dev->dst_ll_buf)));
	tegra_se_write_32(se_dev, SE_IN_LL_ADDR_REG_OFFSET, ((uint64_t)(se_dev->src_ll_buf)));

	/* Check that previous operation is finalized */
	ret = tegra_se_operation_prepare(se_dev);
	if (ret != 0) {
		goto op_error;
	}

	/* Program SE operation size */
	if (nblocks) {
		tegra_se_write_32(se_dev, SE_BLOCK_COUNT_REG_OFFSET, nblocks - 1);
	}

	/* Make SE LL data coherent before the SE operation */
	tegra_se_make_data_coherent(se_dev);

	/* Start hardware operation */
	if (context_save)
		tegra_se_write_32(se_dev, SE_OPERATION_REG_OFFSET, SE_OP_CTX_SAVE);
	else
		tegra_se_write_32(se_dev, SE_OPERATION_REG_OFFSET, SE_OP_START);

	/* Wait for operation to finish */
	ret = tegra_se_operation_complete(se_dev);

op_error:
	return ret;
}

/*
 * Normal security engine operations other than the context save
 */
int tegra_se_start_normal_operation(const tegra_se_dev_t *se_dev, uint32_t nbytes)
{
	return tegra_se_perform_operation(se_dev, nbytes, false);
}

/*
 * Security engine context save operation
 */
int tegra_se_start_ctx_save_operation(const tegra_se_dev_t *se_dev, uint32_t nbytes)
{
	return tegra_se_perform_operation(se_dev, nbytes, true);
}

/*
 * Security Engine sequence to generat SRK
 * SE and SE2 will generate different SRK by different
 * entropy seeds.
 */
static int tegra_se_generate_srk(const tegra_se_dev_t *se_dev)
{
	int ret = PSCI_E_INTERN_FAIL;
	uint32_t val;

	/* Confgure the following hardware register settings:
	 * SE_CONFIG.DEC_ALG = NOP
	 * SE_CONFIG.ENC_ALG = RNG
	 * SE_CONFIG.DST = SRK
	 * SE_OPERATION.OP = START
	 * SE_CRYPTO_LAST_BLOCK = 0
	 */
	se_dev->src_ll_buf->last_buff_num = 0;
	se_dev->dst_ll_buf->last_buff_num = 0;

	/* Configure random number generator */
	if (ecid_valid)
		val = (DRBG_MODE_FORCE_INSTANTION | DRBG_SRC_ENTROPY);
	else
		val = (DRBG_MODE_FORCE_RESEED | DRBG_SRC_ENTROPY);
	tegra_se_write_32(se_dev, SE_RNG_CONFIG_REG_OFFSET, val);

	/* Configure output destination = SRK */
	val = (SE_CONFIG_ENC_ALG_RNG |
		SE_CONFIG_DEC_ALG_NOP |
		SE_CONFIG_DST_SRK);
	tegra_se_write_32(se_dev, SE_CONFIG_REG_OFFSET, val);

	/* Perform hardware operation */
	ret = tegra_se_start_normal_operation(se_dev, 0);

	return ret;
}

/*
 * Generate plain text random data to some memory location using
 * SE/SE2's SP800-90 random number generator. The random data size
 * must be some multiple of the AES block size (16 bytes).
 */
static int tegra_se_lp_generate_random_data(tegra_se_dev_t *se_dev)
{
	int ret = 0;
	uint32_t val;

	/* Set some arbitrary memory location to store the random data */
	se_dev->dst_ll_buf->last_buff_num = 0;
	if (!se_dev->ctx_save_buf) {
		ERROR("%s: ERR: context save buffer NULL pointer!\n", __func__);
		return PSCI_E_NOT_PRESENT;
	}
	se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(((tegra_se_context_t *)
					se_dev->ctx_save_buf)->rand_data)));
	se_dev->dst_ll_buf->buffer[0].data_len = SE_CTX_SAVE_RANDOM_DATA_SIZE;


	/* Confgure the following hardware register settings:
	 * SE_CONFIG.DEC_ALG = NOP
	 * SE_CONFIG.ENC_ALG = RNG
	 * SE_CONFIG.ENC_MODE = KEY192
	 * SE_CONFIG.DST = MEMORY
	 */
	val = (SE_CONFIG_ENC_ALG_RNG |
		SE_CONFIG_DEC_ALG_NOP |
		SE_CONFIG_ENC_MODE_KEY192 |
		SE_CONFIG_DST_MEMORY);
	tegra_se_write_32(se_dev, SE_CONFIG_REG_OFFSET, val);

	/* Program the RNG options in SE_CRYPTO_CONFIG as follows:
	 * XOR_POS = BYPASS
	 * INPUT_SEL = RANDOM (Entropy or LFSR)
	 * HASH_ENB = DISABLE
	 */
	val = (SE_CRYPTO_INPUT_RANDOM |
		SE_CRYPTO_XOR_BYPASS |
		SE_CRYPTO_CORE_ENCRYPT |
		SE_CRYPTO_HASH_DISABLE |
		SE_CRYPTO_KEY_INDEX(RNG_AES_KEY_INDEX) |
		SE_CRYPTO_IV_ORIGINAL);
	tegra_se_write_32(se_dev, SE_CRYPTO_REG_OFFSET, val);

	/* Configure RNG */
	if (ecid_valid)
		val = (DRBG_MODE_FORCE_INSTANTION | DRBG_SRC_LFSR);
	else
		val = (DRBG_MODE_FORCE_RESEED | DRBG_SRC_LFSR);
	tegra_se_write_32(se_dev, SE_RNG_CONFIG_REG_OFFSET, val);

	/* SE normal operation */
	ret = tegra_se_start_normal_operation(se_dev, SE_CTX_SAVE_RANDOM_DATA_SIZE);

	return ret;
}

/*
 * Encrypt memory blocks with SRK as part of the security engine context.
 * The data blocks include: random data and the known pattern data, where
 * the random data is the first block and known pattern is the last block.
 */
static int tegra_se_lp_data_context_save(tegra_se_dev_t *se_dev,
		uint64_t src_addr, uint64_t dst_addr, uint32_t data_size)
{
	int ret = 0;

	se_dev->src_ll_buf->last_buff_num = 0;
	se_dev->dst_ll_buf->last_buff_num = 0;
	se_dev->src_ll_buf->buffer[0].addr = src_addr;
	se_dev->src_ll_buf->buffer[0].data_len = data_size;
	se_dev->dst_ll_buf->buffer[0].addr = dst_addr;
	se_dev->dst_ll_buf->buffer[0].data_len = data_size;

	/* By setting the context source from memory and calling the context save
	 * operation, the SE encrypts the memory data with SRK.
	 */
	tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET, SE_CTX_SAVE_SRC_MEM);

	ret = tegra_se_start_ctx_save_operation(se_dev, data_size);

	return ret;
}

/*
 * Context save the key table access control sticky bits and
 * security status of each key-slot. The encrypted sticky-bits are
 * 32 bytes (2 AES blocks) and formatted as the following structure:
 * {	bit in registers			bit in context save
 *	SECURITY_0[4]				158
 *	SE_RSA_KEYTABLE_ACCE4SS_1[2:0]		157:155
 *	SE_RSA_KEYTABLE_ACCE4SS_0[2:0]		154:152
 *	SE_RSA_SECURITY_PERKEY_0[1:0]		151:150
 *	SE_CRYPTO_KEYTABLE_ACCESS_15[7:0]	149:142
 *	...,
 *	SE_CRYPTO_KEYTABLE_ACCESS_0[7:0]	29:22
 *	SE_CRYPTO_SECURITY_PERKEY_0[15:0]	21:6
 *	SE_TZRAM_SECURITY_0[1:0]		5:4
 *	SE_SECURITY_0[16]			3:3
 *	SE_SECURITY_0[2:0] }			2:0
 */
static int tegra_se_lp_sticky_bits_context_save(tegra_se_dev_t *se_dev)
{
	int ret = PSCI_E_INTERN_FAIL;
	uint32_t val = 0;

	se_dev->dst_ll_buf->last_buff_num = 0;
	if (!se_dev->ctx_save_buf) {
		ERROR("%s: ERR: context save buffer NULL pointer!\n", __func__);
		return PSCI_E_NOT_PRESENT;
	}
	se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(((tegra_se_context_t *)
						se_dev->ctx_save_buf)->sticky_bits)));
	se_dev->dst_ll_buf->buffer[0].data_len = SE_CTX_SAVE_STICKY_BITS_SIZE;

	/*
	 * The 1st AES block save the sticky-bits context 1 - 16 bytes (0 - 3 words).
	 * The 2nd AES block save the sticky-bits context 17 - 32 bytes (4 - 7 words).
	 */
	for (int i = 0; i < 2; i++) {
		val = SE_CTX_SAVE_SRC_STICKY_BITS |
			SE_CTX_SAVE_STICKY_WORD_QUAD(i);
		tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

		/* SE context save operation */
		ret = tegra_se_start_ctx_save_operation(se_dev,
				SE_CTX_SAVE_STICKY_BITS_SIZE);
		if (ret)
			break;
		se_dev->dst_ll_buf->buffer[0].addr += SE_CTX_SAVE_STICKY_BITS_SIZE;
	}

	return ret;
}

static int tegra_se_aeskeytable_context_save(tegra_se_dev_t *se_dev)
{
	uint32_t val = 0;
	int ret = 0;

	se_dev->dst_ll_buf->last_buff_num = 0;
	if (!se_dev->ctx_save_buf) {
		ERROR("%s: ERR: context save buffer NULL pointer!\n", __func__);
		ret = -EINVAL;
		goto aes_keytable_save_err;
	}

	/* AES key context save */
	for (int slot = 0; slot < TEGRA_SE_AES_KEYSLOT_COUNT; slot++) {
		se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
						((tegra_se_context_t *)se_dev->
						 ctx_save_buf)->key_slots[slot].key)));
		se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_KEY_128_SIZE;
		for (int i = 0; i < 2; i++) {
			val = SE_CTX_SAVE_SRC_AES_KEYTABLE |
				SE_CTX_SAVE_KEY_INDEX(slot) |
				SE_CTX_SAVE_WORD_QUAD(i);
			tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

			/* SE context save operation */
			ret = tegra_se_start_ctx_save_operation(se_dev,
					TEGRA_SE_KEY_128_SIZE);
			if (ret) {
				ERROR("%s: ERR: AES key CTX_SAVE OP failed, "
						"slot=%d, word_quad=%d.\n",
						__func__, slot, i);
				goto aes_keytable_save_err;
			}
			se_dev->dst_ll_buf->buffer[0].addr += TEGRA_SE_KEY_128_SIZE;
		}

		/* OIV context save */
		se_dev->dst_ll_buf->last_buff_num = 0;
		se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
						((tegra_se_context_t *)se_dev->
						 ctx_save_buf)->key_slots[slot].oiv)));
		se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_AES_IV_SIZE;

		val = SE_CTX_SAVE_SRC_AES_KEYTABLE |
			SE_CTX_SAVE_KEY_INDEX(slot) |
			SE_CTX_SAVE_WORD_QUAD_ORIG_IV;
		tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

		/* SE context save operation */
		ret = tegra_se_start_ctx_save_operation(se_dev, TEGRA_SE_AES_IV_SIZE);
		if (ret) {
			ERROR("%s: ERR: OIV CTX_SAVE OP failed, slot=%d.\n",
					__func__, slot);
			goto aes_keytable_save_err;
		}

		/* UIV context save */
		se_dev->dst_ll_buf->last_buff_num = 0;
		se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
						((tegra_se_context_t *)se_dev->
						 ctx_save_buf)->key_slots[slot].uiv)));
		se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_AES_IV_SIZE;

		val = SE_CTX_SAVE_SRC_AES_KEYTABLE |
			SE_CTX_SAVE_KEY_INDEX(slot) |
			SE_CTX_SAVE_WORD_QUAD_UPD_IV;
		tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

		/* SE context save operation */
		ret = tegra_se_start_ctx_save_operation(se_dev, TEGRA_SE_AES_IV_SIZE);
		if (ret) {
			ERROR("%s: ERR: UIV CTX_SAVE OP failed, slot=%d\n",
					__func__, slot);
			goto aes_keytable_save_err;
		}
	}

aes_keytable_save_err:
	return ret;
}

static int tegra_se_lp_rsakeytable_context_save(tegra_se_dev_t *se_dev)
{
	uint32_t val = 0;
	int ret = 0;
	/* First the modulus and then the exponent must be
	 * encrypted and saved. This is repeated for SLOT 0
	 * and SLOT 1. Hence the order:
	 * SLOT 0 exponent : RSA_KEY_INDEX : 0
	 * SLOT 0 modulus : RSA_KEY_INDEX : 1
	 * SLOT 1 exponent : RSA_KEY_INDEX : 2
	 * SLOT 1 modulus : RSA_KEY_INDEX : 3
	 */
	const unsigned int key_index_mod[TEGRA_SE_RSA_KEYSLOT_COUNT][2] = {
		/* RSA key slot 0 */
		{SE_RSA_KEY_INDEX_SLOT0_EXP, SE_RSA_KEY_INDEX_SLOT0_MOD},
		/* RSA key slot 1 */
		{SE_RSA_KEY_INDEX_SLOT1_EXP, SE_RSA_KEY_INDEX_SLOT1_MOD},
	};

	se_dev->dst_ll_buf->last_buff_num = 0;
	se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
					((tegra_se_context_t *)se_dev->
					 ctx_save_buf)->rsa_keys)));
	se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_KEY_128_SIZE;

	for (int slot = 0; slot < TEGRA_SE_RSA_KEYSLOT_COUNT; slot++) {
		/* loop for modulus and exponent */
		for (int index = 0; index < 2; index++) {
			for (int word_quad = 0; word_quad < 16; word_quad++) {
				val = SE_CTX_SAVE_SRC_RSA_KEYTABLE |
					SE_CTX_SAVE_RSA_KEY_INDEX(
						key_index_mod[slot][index]) |
					SE_CTX_RSA_WORD_QUAD(word_quad);
				tegra_se_write_32(se_dev,
					SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

				/* SE context save operation */
				ret = tegra_se_start_ctx_save_operation(se_dev,
						TEGRA_SE_KEY_128_SIZE);
				if (ret) {
					ERROR("%s: ERR: slot=%d.\n",
						__func__, slot);
					goto rsa_keytable_save_err;
				}

				/* Update the pointer to the next word quad */
				se_dev->dst_ll_buf->buffer[0].addr +=
					TEGRA_SE_KEY_128_SIZE;
			}
		}
	}

rsa_keytable_save_err:
	return ret;
}

static int tegra_se_pkakeytable_sticky_bits_save(tegra_se_dev_t *se_dev)
{
	int ret = 0;

	se_dev->dst_ll_buf->last_buff_num = 0;
	se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
					((tegra_se2_context_blob_t *)se_dev->
					 ctx_save_buf)->pka_ctx.sticky_bits)));
	se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_AES_BLOCK_SIZE;

	/* PKA1 sticky bits are 1 AES block (16 bytes) */
	tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET,
			SE_CTX_SAVE_SRC_PKA1_STICKY_BITS |
			SE_CTX_STICKY_WORD_QUAD_WORDS_0_3);

	/* SE context save operation */
	ret = tegra_se_start_ctx_save_operation(se_dev, 0);
	if (ret) {
		ERROR("%s: ERR: PKA1 sticky bits CTX_SAVE OP failed\n",
				__func__);
		goto pka_sticky_bits_save_err;
	}

pka_sticky_bits_save_err:
	return ret;
}

static int tegra_se_pkakeytable_context_save(tegra_se_dev_t *se_dev)
{
	uint32_t val = 0;
	int ret = 0;

	se_dev->dst_ll_buf->last_buff_num = 0;
	se_dev->dst_ll_buf->buffer[0].addr = ((uint64_t)(&(
					((tegra_se2_context_blob_t *)se_dev->
					 ctx_save_buf)->pka_ctx.pka_keys)));
	se_dev->dst_ll_buf->buffer[0].data_len = TEGRA_SE_KEY_128_SIZE;

	/* for each slot, save word quad 0-127 */
	for (int slot = 0; slot < TEGRA_SE_PKA1_KEYSLOT_COUNT; slot++) {
		for (int word_quad = 0; word_quad < 512/4; word_quad++) {
			val = SE_CTX_SAVE_SRC_PKA1_KEYTABLE |
				SE_CTX_PKA1_WORD_QUAD_L((slot * 128) +
						word_quad) |
				SE_CTX_PKA1_WORD_QUAD_H((slot * 128) +
						word_quad);
			tegra_se_write_32(se_dev,
					SE_CTX_SAVE_CONFIG_REG_OFFSET, val);

			/* SE context save operation */
			ret = tegra_se_start_ctx_save_operation(se_dev,
					TEGRA_SE_KEY_128_SIZE);
			if (ret) {
				ERROR("%s: ERR: pka1 keytable ctx save error\n",
						__func__);
				goto pka_keytable_save_err;
			}

			/* Update the pointer to the next word quad */
			se_dev->dst_ll_buf->buffer[0].addr +=
				TEGRA_SE_KEY_128_SIZE;
		}
	}

pka_keytable_save_err:
	return ret;
}

static int tegra_se_save_SRK(tegra_se_dev_t *se_dev)
{
	tegra_se_write_32(se_dev, SE_CTX_SAVE_CONFIG_REG_OFFSET,
			SE_CTX_SAVE_SRC_SRK);

	/* SE context save operation */
	return tegra_se_start_ctx_save_operation(se_dev, 0);
}

/*
 *  Lock both SE from non-TZ clients.
 */
static inline void tegra_se_lock(tegra_se_dev_t *se_dev)
{
	uint32_t val;

	assert(se_dev);
	val = tegra_se_read_32(se_dev, SE_SECURITY_REG_OFFSET);
	val |= SE_SECURITY_TZ_LOCK_SOFT(SE_SECURE);
	tegra_se_write_32(se_dev, SE_SECURITY_REG_OFFSET, val);
}

/*
 * Use SRK to encrypt SE state and save to TZRAM carveout
 */
static int tegra_se_context_save_sw(tegra_se_dev_t *se_dev)
{
	int err = 0;

	assert(se_dev);

	/* Lock entire SE/SE2 as TZ protected */
	tegra_se_lock(se_dev);

	INFO("%s: generate SRK\n", __func__);
	/* Generate SRK */
	err = tegra_se_generate_srk(se_dev);
	if (err) {
		ERROR("%s: ERR: SRK generation failed\n", __func__);
		return err;
	}

	INFO("%s: generate random data\n", __func__);
	/* Generate random data */
	err = tegra_se_lp_generate_random_data(se_dev);
	if (err) {
		ERROR("%s: ERR: LP random pattern generation failed\n", __func__);
		return err;
	}

	INFO("%s: encrypt random data\n", __func__);
	/* Encrypt the random data block */
	err = tegra_se_lp_data_context_save(se_dev,
		((uint64_t)(&(((tegra_se_context_t *)se_dev->
					ctx_save_buf)->rand_data))),
		((uint64_t)(&(((tegra_se_context_t *)se_dev->
					ctx_save_buf)->rand_data))),
		SE_CTX_SAVE_RANDOM_DATA_SIZE);
	if (err) {
		ERROR("%s: ERR: random pattern encryption failed\n", __func__);
		return err;
	}

	INFO("%s: save SE sticky bits\n", __func__);
	/* Save AES sticky bits context */
	err = tegra_se_lp_sticky_bits_context_save(se_dev);
	if (err) {
		ERROR("%s: ERR: sticky bits context save failed\n", __func__);
		return err;
	}

	INFO("%s: save AES keytables\n", __func__);
	/* Save AES key table context */
	err = tegra_se_aeskeytable_context_save(se_dev);
	if (err) {
		ERROR("%s: ERR: LP keytable save failed\n", __func__);
		return err;
	}

	/* RSA key slot table context save */
	INFO("%s: save RSA keytables\n", __func__);
	err = tegra_se_lp_rsakeytable_context_save(se_dev);
	if (err) {
		ERROR("%s: ERR: rsa key table context save failed\n", __func__);
		return err;
	}

	/* Only SE2 has an interface with PKA1; thus, PKA1's context is saved
	 * via SE2.
	 */
	if (se_dev->se_num == 2) {
		/* Encrypt PKA1 sticky bits on SE2 only */
		INFO("%s: save PKA sticky bits\n", __func__);
		err = tegra_se_pkakeytable_sticky_bits_save(se_dev);
		if (err) {
			ERROR("%s: ERR: PKA sticky bits context save failed\n", __func__);
			return err;
		}

		/* Encrypt PKA1 keyslots on SE2 only */
		INFO("%s: save PKA keytables\n", __func__);
		err = tegra_se_pkakeytable_context_save(se_dev);
		if (err) {
			ERROR("%s: ERR: PKA key table context save failed\n", __func__);
			return err;
		}
	}

	/* Encrypt known pattern */
	if (se_dev->se_num == 1) {
		err = tegra_se_lp_data_context_save(se_dev,
			((uint64_t)(&se_ctx_known_pattern_data)),
			((uint64_t)(&(((tegra_se_context_blob_t *)se_dev->ctx_save_buf)->known_pattern))),
			SE_CTX_KNOWN_PATTERN_SIZE);
	} else if (se_dev->se_num == 2) {
		err = tegra_se_lp_data_context_save(se_dev,
			((uint64_t)(&se_ctx_known_pattern_data)),
			((uint64_t)(&(((tegra_se2_context_blob_t *)se_dev->ctx_save_buf)->known_pattern))),
			SE_CTX_KNOWN_PATTERN_SIZE);
	}
	if (err) {
		ERROR("%s: ERR: save LP known pattern failure\n", __func__);
		return err;
	}

	/* Write lp context buffer address into PMC scratch register */
	if (se_dev->se_num == 1) {
		/* SE context address */
		mmio_write_32((uint64_t)TEGRA_PMC_BASE + PMC_SECURE_SCRATCH117_OFFSET,
				((uint64_t)(se_dev->ctx_save_buf)));
	} else if (se_dev->se_num == 2) {
		/* SE2 & PKA1 context address */
		mmio_write_32((uint64_t)TEGRA_PMC_BASE + PMC_SECURE_SCRATCH116_OFFSET,
				((uint64_t)(se_dev->ctx_save_buf)));
	}

	/* Saves SRK to PMC secure scratch registers for BootROM, which
	 * verifies and restores the security engine context on warm boot.
	 */
	err = tegra_se_save_SRK(se_dev);
	if (err < 0) {
		ERROR("%s: ERR: LP SRK save failure\n", __func__);
		return err;
	}

	INFO("%s: SE context save done \n", __func__);

	return err;
}

/*
 * Initialize the SE engine handle
 */
void tegra_se_init(void)
{
	uint32_t val = 0;
	INFO("%s: start SE init\n", __func__);

	/* Generate random SRK to initialize DRBG */
	tegra_se_generate_srk(&se_dev_1);
	tegra_se_generate_srk(&se_dev_2);

	/* determine if ECID is valid */
	val = mmio_read_32(TEGRA_FUSE_BASE + FUSE_JTAG_SECUREID_VALID);
	ecid_valid = (val == ECID_VALID);

	INFO("%s: SE init done\n", __func__);
}

static void tegra_se_enable_clocks(void)
{
	uint32_t val = 0;

	/* Enable entropy clock */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_W);
	val |= ENTROPY_CLK_ENB_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_W, val);

	/* De-Assert Entropy Reset */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEVICES_W);
	val &= ~ENTROPY_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEVICES_W, val);

	/* Enable SE clock */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_V);
	val |= SE_CLK_ENB_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_V, val);

	/* De-Assert SE Reset */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEVICES_V);
	val &= ~SE_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEVICES_V, val);
}

static void tegra_se_disable_clocks(void)
{
	uint32_t val = 0;

	/* Disable entropy clock */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_W);
	val &= ~ENTROPY_CLK_ENB_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_W, val);

	/* Disable SE clock */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_V);
	val &= ~SE_CLK_ENB_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_OUT_ENB_V, val);
}

/*
 * Security engine power suspend entry point.
 * This function is invoked from PSCI power domain suspend handler.
 */
int32_t tegra_se_suspend(void)
{
	int32_t ret = 0;
	uint32_t val = 0;

	/* SE does not use SMMU in EL3, disable SMMU.
	 * This will be re-enabled by kernel on resume */
	val = mmio_read_32(TEGRA_MC_BASE + MC_SMMU_PPCS_ASID_0);
	val &= ~PPCS_SMMU_ENABLE;
	mmio_write_32(TEGRA_MC_BASE + MC_SMMU_PPCS_ASID_0, val);

	tegra_se_enable_clocks();

	if (tegra_se_atomic_save_enabled(&se_dev_2) &&
			tegra_se_atomic_save_enabled(&se_dev_1)) {
		/* Atomic context save se2 and pka1 */
		INFO("%s: SE2/PKA1 atomic context save\n", __func__);
		if (ret == 0) {
			ret = tegra_se_context_save_atomic(&se_dev_2);
		}

		/* Atomic context save se */
		if (ret == 0) {
			INFO("%s: SE1 atomic context save\n", __func__);
			ret = tegra_se_context_save_atomic(&se_dev_1);
		}

		if (ret == 0) {
			INFO("%s: SE atomic context save done\n", __func__);
		}
	} else if (!tegra_se_atomic_save_enabled(&se_dev_2) &&
			!tegra_se_atomic_save_enabled(&se_dev_1)) {
		/* SW context save se2 and pka1 */
		INFO("%s: SE2/PKA1 legacy(SW) context save\n", __func__);
		if (ret == 0) {
			ret = tegra_se_context_save_sw(&se_dev_2);
		}

		/* SW context save se */
		if (ret == 0) {
			INFO("%s: SE1 legacy(SW) context save\n", __func__);
			ret = tegra_se_context_save_sw(&se_dev_1);
		}

		if (ret == 0) {
			INFO("%s: SE SW context save done\n", __func__);
		}
	} else {
		ERROR("%s: One SE set for atomic CTX save, the other is not\n",
			 __func__);
	}

	tegra_se_disable_clocks();

	return ret;
}

/*
 * Save TZRAM to shadow TZRAM in AON
 */
int32_t tegra_se_save_tzram(void)
{
	uint32_t val = 0;
	int32_t ret = 0;
	uint32_t timeout;

	INFO("%s: SE TZRAM save start\n", __func__);
	tegra_se_enable_clocks();

	val = (SE_TZRAM_OP_REQ_INIT | SE_TZRAM_OP_MODE_SAVE);
	tegra_se_write_32(&se_dev_1, SE_TZRAM_OPERATION, val);

	val = tegra_se_read_32(&se_dev_1, SE_TZRAM_OPERATION);
	for (timeout = 0; (SE_TZRAM_OP_BUSY(val) == SE_TZRAM_OP_BUSY_ON) &&
			(timeout < TIMEOUT_100MS); timeout++) {
		mdelay(1);
		val = tegra_se_read_32(&se_dev_1, SE_TZRAM_OPERATION);
	}

	if (timeout == TIMEOUT_100MS) {
		ERROR("%s: ERR: TZRAM save timeout!\n", __func__);
		ret = -ETIMEDOUT;
	}

	if (ret == 0) {
		INFO("%s: SE TZRAM save done!\n", __func__);
	}

	tegra_se_disable_clocks();

	return ret;
}

/*
 * The function is invoked by SE resume
 */
static void tegra_se_warm_boot_resume(const tegra_se_dev_t *se_dev)
{
	uint32_t val;

	assert(se_dev);

	/* Lock RNG source to ENTROPY on resume */
	val = DRBG_RO_ENT_IGNORE_MEM_ENABLE |
		DRBG_RO_ENT_SRC_LOCK_ENABLE |
		DRBG_RO_ENT_SRC_ENABLE;
	tegra_se_write_32(se_dev, SE_RNG_SRC_CONFIG_REG_OFFSET, val);

	/* Set a random value to SRK to initialize DRBG */
	tegra_se_generate_srk(se_dev);
}

/*
 * The function is invoked on SC7 resume
 */
void tegra_se_resume(void)
{
	tegra_se_warm_boot_resume(&se_dev_1);
	tegra_se_warm_boot_resume(&se_dev_2);
}

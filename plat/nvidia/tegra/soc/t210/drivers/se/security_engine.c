/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <psci.h>
#include <se_private.h>
#include <security_engine.h>
#include <tegra_platform.h>

/*******************************************************************************
 * Constants and Macros
 ******************************************************************************/

#define TIMEOUT_100MS	100UL	// Timeout in 100ms

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

/* SE input and output linked list buffers */
static tegra_se_io_lst_t se1_src_ll_buf;
static tegra_se_io_lst_t se1_dst_ll_buf;

/* SE2 input and output linked list buffers */
static tegra_se_io_lst_t se2_src_ll_buf;
static tegra_se_io_lst_t se2_dst_ll_buf;

/* SE1 security engine device handle */
static tegra_se_dev_t se_dev_1 = {
	.se_num = 1,
	/* setup base address for se */
	.se_base = TEGRA_SE1_BASE,
	/* Setup context size in AES blocks */
	.ctx_size_blks = SE_CTX_SAVE_SIZE_BLOCKS_SE1,
	/* Setup SRC buffers for SE operations */
	.src_ll_buf = &se1_src_ll_buf,
	/* Setup DST buffers for SE operations */
	.dst_ll_buf = &se1_dst_ll_buf,
};

/* SE2 security engine device handle */
static tegra_se_dev_t se_dev_2 = {
	.se_num = 2,
	/* setup base address for se */
	.se_base = TEGRA_SE2_BASE,
	/* Setup context size in AES blocks */
	.ctx_size_blks = SE_CTX_SAVE_SIZE_BLOCKS_SE2,
	/* Setup SRC buffers for SE operations */
	.src_ll_buf = &se2_src_ll_buf,
	/* Setup DST buffers for SE operations */
	.dst_ll_buf = &se2_dst_ll_buf,
};

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
 * Check for context save operation complete
 * This function is invoked after the context save operation,
 * and it checks the following conditions:
 * 1. SE_INT_STATUS = SE_OP_DONE
 * 2. SE_STATUS = IDLE
 * 3. AHB bus data transfer complete.
 * 4. SE_ERR_STATUS is clean.
 */
static int32_t tegra_se_context_save_complete(const tegra_se_dev_t *se_dev)
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
 * Verify the SE context save auto has been enabled.
 * SE_CTX_SAVE_AUTO.ENABLE == ENABLE
 * If the SE context save auto is not enabled, then set
 * the context save auto enable and lock the setting.
 * If the SE context save auto is not enabled and the
 * enable setting is locked, then return an error.
 */
static inline int32_t tegra_se_ctx_save_auto_enable(const tegra_se_dev_t *se_dev)
{
	uint32_t val;
	int32_t ret = 0;

	val = tegra_se_read_32(se_dev, SE_CTX_SAVE_AUTO_REG_OFFSET);
	if (SE_CTX_SAVE_AUTO_ENABLE(val) == SE_CTX_SAVE_AUTO_DIS) {
		if (SE_CTX_SAVE_AUTO_LOCK(val) == SE_CTX_SAVE_AUTO_LOCK_EN) {
			ERROR("%s: ERR: Cannot enable atomic. Write locked!\n",
					__func__);
			ret = -EACCES;
		}

		/* Program SE_CTX_SAVE_AUTO */
		if (ret == 0) {
			tegra_se_write_32(se_dev, SE_CTX_SAVE_AUTO_REG_OFFSET,
					SE_CTX_SAVE_AUTO_LOCK_EN |
					SE_CTX_SAVE_AUTO_EN);
		}
	}

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
	uint32_t block_count, timeout;

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

	/* Clear any pending interrupts */
	if (ret == 0) {
		val = tegra_se_read_32(se_dev, SE_INT_STATUS_REG_OFFSET);
		tegra_se_write_32(se_dev, SE_INT_STATUS_REG_OFFSET, val);

		/* Ensure HW atomic context save has been enabled
		 * This should have been done at boot time.
		 * SE_CTX_SAVE_AUTO.ENABLE == ENABLE
		 */
		ret = tegra_se_ctx_save_auto_enable(se_dev);
	}

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

		ret = tegra_se_context_save_complete(se_dev);
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
 * Initialize the SE engine handle
 */
void tegra_se_init(void)
{
	INFO("%s: start SE init\n", __func__);

	/* TODO: Bug 1854340. Generate random SRK */

	INFO("%s: SE init done\n", __func__);
}

/*
 * Security engine power suspend entry point.
 * This function is invoked from PSCI power domain suspend handler.
 */
int32_t tegra_se_suspend(void)
{
	int32_t ret = 0;

	/* Atomic context save se2 and pka1 */
	INFO("%s: SE2/PKA1 atomic context save\n", __func__);
	ret = tegra_se_context_save_atomic(&se_dev_2);

	/* Atomic context save se */
	if (ret == 0) {
		INFO("%s: SE1 atomic context save\n", __func__);
		ret = tegra_se_context_save_atomic(&se_dev_1);
	}

	if (ret == 0) {
		INFO("%s: SE atomic context save done\n", __func__);
	}

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

	/* Enable and lock the SE atomic context save setting */
	if (tegra_se_ctx_save_auto_enable(se_dev) != 0) {
		ERROR("%s: ERR: enable SE%d context save auto failed!\n",
			__func__, se_dev->se_num);
	}

	/* TODO: Bug 1854340. Set a random value to SRK */
}

/*
 * The function is invoked on SC7 resume
 */
void tegra_se_resume(void)
{
	tegra_se_warm_boot_resume(&se_dev_1);
	tegra_se_warm_boot_resume(&se_dev_2);
}

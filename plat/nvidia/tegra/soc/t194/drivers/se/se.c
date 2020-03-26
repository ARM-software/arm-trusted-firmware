/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <bpmp_ipc.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <se.h>
#include <tegra_platform.h>

#include "se_private.h"

/*******************************************************************************
 * Constants and Macros
 ******************************************************************************/
#define ERR_STATUS_SW_CLEAR		U(0xFFFFFFFF)
#define INT_STATUS_SW_CLEAR		U(0xFFFFFFFF)
#define MAX_TIMEOUT_MS			U(1000)	/* Max. timeout of 1s */
#define NUM_SE_REGS_TO_SAVE		U(4)

#define BYTES_IN_WORD			U(4)
#define SHA256_MAX_HASH_RESULT		U(7)
#define SHA256_DST_SIZE			U(32)
#define SHA_FIRST_OP			U(1)
#define MAX_SHA_ENGINE_CHUNK_SIZE	U(0xFFFFFF)
#define SHA256_MSG_LENGTH_ONETIME	U(0xFFFF)

/*******************************************************************************
 * Data structure and global variables
 ******************************************************************************/
static uint32_t se_regs[NUM_SE_REGS_TO_SAVE];

/*
 * Check that SE operation has completed after kickoff.
 *
 * This function is invoked after an SE operation has been started,
 * and it checks the following conditions:
 *
 * 1. SE_STATUS = IDLE
 * 2. AHB bus data transfer is complete.
 * 3. SE_ERR_STATUS is clean.
 */
static bool tegra_se_is_operation_complete(void)
{
	uint32_t val = 0, timeout = 0, sha_status, aes_status;
	int32_t ret = 0;
	bool se_is_busy, txn_has_errors, txn_successful;

	/*
	 * Poll the status register to check if the operation
	 * completed.
	 */
	do {
		val = tegra_se_read_32(CTX_SAVE_AUTO_STATUS);
		se_is_busy = ((val & CTX_SAVE_AUTO_SE_BUSY) != 0U);

		/* sleep until SE finishes */
		if (se_is_busy) {
			mdelay(1);
			timeout++;
		}

	} while (se_is_busy && (timeout < MAX_TIMEOUT_MS));

	/* any transaction errors? */
	txn_has_errors = (tegra_se_read_32(SHA_ERR_STATUS) != 0U) ||
			 (tegra_se_read_32(AES0_ERR_STATUS) != 0U);

	/* transaction successful? */
	sha_status = tegra_se_read_32(SHA_INT_STATUS) & SHA_SE_OP_DONE;
	aes_status = tegra_se_read_32(AES0_INT_STATUS) & AES0_SE_OP_DONE;
	txn_successful = (sha_status == SHA_SE_OP_DONE) &&
			 (aes_status == AES0_SE_OP_DONE);

	if ((timeout == MAX_TIMEOUT_MS) || txn_has_errors || !txn_successful) {
		ERROR("%s: Atomic context save operation failed!\n",
				__func__);
		ret = -ECANCELED;
	}

	return (ret == 0);
}

/*
 * Wait for SE engine to be idle and clear any pending interrupts, before
 * starting the next SE operation.
 */
static bool tegra_se_is_ready(void)
{
	int32_t ret = 0;
	uint32_t val = 0, timeout = 0;
	bool se_is_ready;

	/* Wait for previous operation to finish */
	do {
		val = tegra_se_read_32(CTX_SAVE_AUTO_STATUS);
		se_is_ready = (val == CTX_SAVE_AUTO_SE_READY);

		/* sleep until SE is ready */
		if (!se_is_ready) {
			mdelay(1);
			timeout++;
		}

	} while (!se_is_ready && (timeout < MAX_TIMEOUT_MS));

	if (timeout == MAX_TIMEOUT_MS) {
		ERROR("%s: SE is not ready!\n", __func__);
		ret = -ETIMEDOUT;
	}

	/* Clear any pending interrupts from previous operation */
	tegra_se_write_32(AES0_INT_STATUS, INT_STATUS_SW_CLEAR);
	tegra_se_write_32(AES1_INT_STATUS, INT_STATUS_SW_CLEAR);
	tegra_se_write_32(RSA_INT_STATUS, INT_STATUS_SW_CLEAR);
	tegra_se_write_32(SHA_INT_STATUS, INT_STATUS_SW_CLEAR);

	/* Clear error status for each engine seen from current port */
	tegra_se_write_32(AES0_ERR_STATUS, ERR_STATUS_SW_CLEAR);
	tegra_se_write_32(AES1_ERR_STATUS, ERR_STATUS_SW_CLEAR);
	tegra_se_write_32(RSA_ERR_STATUS, ERR_STATUS_SW_CLEAR);
	tegra_se_write_32(SHA_ERR_STATUS, ERR_STATUS_SW_CLEAR);

	return (ret == 0);
}

/*
 * During System Suspend, this handler triggers the hardware context
 * save operation.
 */
static int32_t tegra_se_save_context(void)
{
	int32_t ret = -ECANCELED;

	/*
	 * 1. Ensure all SE Driver including RNG1/PKA1 are shut down.
	 *    TSEC/R5s are powergated/idle. All tasks on SE1~SE4, RNG1,
	 *    PKA1 are wrapped up. SE0 is ready for use.
	 * 2. Clear interrupt/error in SE0 status register.
	 * 3. Scrub SE0 register to avoid false failure for illegal
	 *    configuration. Probably not needed, dependent on HW
	 *    implementation.
	 * 4. Check SE is ready for HW CTX_SAVE by polling
	 *    SE_CTX_SAVE_AUTO_STATUS.SE_READY.
	 *
	 *    Steps 1-4 are executed by tegra_se_is_ready().
	 *
	 * 5. Issue context save command.
	 * 6. Check SE is busy with CTX_SAVE, the command in step5 was not
	 *    dropped for ongoing traffic in any of SE port/engine.
	 * 7. Poll SE register or wait for SE APB interrupt for task completion
	 *    a. Polling: Read SE_CTX_SAVE_AUTO_STATUS.BUSY till it reports IDLE
	 *    b. Interrupt: After receiving interrupt from SE APB, read
	 *       SE_CTX_SAVE_AUTO_STATUS.BUSY till it reports IDLE.
	 * 8. Check AES0 and SHA ERR_STATUS to ensure no error case.
	 * 9. Check AES0 and SHA INT_STATUS to ensure operation has successfully
	 *    completed.
	 *
	 *    Steps 6-9 are executed by tegra_se_is_operation_complete().
	 */
	if (tegra_se_is_ready()) {

		/* Issue context save command */
		tegra_se_write_32(AES0_OPERATION, SE_OP_CTX_SAVE);

		/* Wait for operation to finish */
		if (tegra_se_is_operation_complete()) {
			ret = 0;
		}
	}

	return ret;
}

/*
 * Check that SE operation has completed after kickoff
 * This function is invoked after an SE operation has been started,
 * and it checks the following conditions:
 * 1. SE0_INT_STATUS = SE0_OP_DONE
 * 2. SE0_STATUS = IDLE
 * 3. SE0_ERR_STATUS is clean.
 */
static int32_t tegra_se_sha256_hash_operation_complete(void)
{
	uint32_t val = 0U;

	/* Poll the SE interrupt register to ensure H/W operation complete */
	val = tegra_se_read_32(SE0_INT_STATUS_REG_OFFSET);
	while (SE0_INT_OP_DONE(val) == SE0_INT_OP_DONE_CLEAR) {
		val = tegra_se_read_32(SE0_INT_STATUS_REG_OFFSET);
		if (SE0_INT_OP_DONE(val) != SE0_INT_OP_DONE_CLEAR) {
			break;
		}
	}

	/* Poll the SE status idle to ensure H/W operation complete */
	val = tegra_se_read_32(SE0_SHA_STATUS_0);
	while (val != SE0_SHA_STATUS_IDLE) {
		val = tegra_se_read_32(SE0_SHA_STATUS_0);
		if (val == SE0_SHA_STATUS_IDLE) {
			break;
		}
	}

	/* Ensure that no errors are thrown during operation */
	val = tegra_se_read_32(SE0_ERR_STATUS_REG_OFFSET);
	if (val != 0U) {
		ERROR("%s: error during SE operation! 0x%x", __func__,
				val);
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
	uint32_t val = 0U;
	uint32_t src_in_lo;
	uint32_t src_in_msb;
	uint32_t src_in_hi;
	int32_t ret = 0;

	if ((src_addr == 0ULL) || (nbytes == 0U))
		return -EINVAL;

	src_in_lo = (uint32_t)src_addr;
	src_in_msb = (uint32_t)((src_addr >> 32U) & 0xFFU);
	src_in_hi = ((src_in_msb << SE0_IN_HI_ADDR_HI_0_MSB_SHIFT) |
				(nbytes & MAX_SHA_ENGINE_CHUNK_SIZE));

	/* set SRC_IN_ADDR_LO and SRC_IN_ADDR_HI*/
	tegra_se_write_32(SE0_IN_ADDR, src_in_lo);
	tegra_se_write_32(SE0_IN_HI_ADDR_HI, src_in_hi);

	val = tegra_se_read_32(SE0_INT_STATUS_REG_OFFSET);
	if (val > 0U) {
		tegra_se_write_32(SE0_INT_STATUS_REG_OFFSET, 0x0U);
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

	return ret;
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

	if (src_addr == 0ULL) {
		return -EINVAL;
	}

	/* number of bytes per operation */
	max_bytes = (SHA256_HASH_SIZE_BYTES * SHA256_MSG_LENGTH_ONETIME);

	src_len_inbits = (uint32_t)(src_len_inbyte * 8U);
	len_bits_msb = (uint32_t)(src_len_inbits >> 32U);
	len_bits_lsb = (uint32_t)src_len_inbits;

	/* program SE0_CONFIG for SHA256 operation */
	val =  (uint32_t)(SE0_CONFIG_ENC_ALG_SHA | SE0_CONFIG_ENC_MODE_SHA256 |
		SE0_CONFIG_DEC_ALG_NOP | SE0_CONFIG_DST_HASHREG);
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

	number_of_operations = (src_len_inbyte / max_bytes);
	remaining_bytes = (src_len_inbyte % max_bytes);
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
	 *			hash-result generation and start SHA256-normal
	 *			operation.
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

static int32_t tegra_se_save_sha256_pmc_scratch(void)
{
	uint32_t val = 0U, hash_offset = 0U, scratch_offset = 0U;
	int32_t ret;

	/* Check SE0 operation status */
	ret = tegra_se_sha256_hash_operation_complete();
	if (ret != 0) {
		ERROR("SE operation complete Failed! 0x%x", ret);
		return ret;
	}

	for (scratch_offset = SECURE_SCRATCH_TZDRAM_SHA256_HASH_START;
			scratch_offset <= SECURE_SCRATCH_TZDRAM_SHA256_HASH_END;
					scratch_offset += BYTES_IN_WORD) {
		val = tegra_se_read_32(SE0_SHA_HASH_RESULT_0 + hash_offset);
		mmio_write_32((uint32_t)(TEGRA_SCRATCH_BASE + scratch_offset),
									val);
		hash_offset += BYTES_IN_WORD;
	}
	return 0;
}

/*
 * Handler to generate SHA256 and save HASH-result to pmc-scratch register
 */
int32_t tegra_se_calculate_save_sha256(uint64_t src_addr,
						uint32_t src_len_inbyte)
{
	uint32_t security;
	int32_t val = 0;

	/* Set SE_SOFT_SETTINGS=SE_SECURE to prevent NS process to change SE
	 * registers.
	 */
	security = tegra_se_read_32(SE0_SECURITY);
	tegra_se_write_32(SE0_SECURITY, security | SE0_SECURITY_SE_SOFT_SETTING);

	/* Bootrom enable IN_ID bit in SE0_SHA_GSCID_0 register during SC7-exit, causing
	 * SE0 ignores SE0 operation, and therefore failure of 2nd iteration of SC7 cycle.
	 */
	tegra_se_write_32(SE0_SHA_GSCID_0, 0x0U);

	/* Calculate SHA256 of BL31 */
	val = tegra_se_calculate_sha256_hash(src_addr, src_len_inbyte);
	if (val != 0) {
		ERROR("%s: SHA256 generation failed\n", __func__);
		return val;
	}

	/*
	 * Reset SE_SECURE to previous value.
	 */
	tegra_se_write_32(SE0_SECURITY, security);

	/* copy sha256_dst to PMC Scratch register */
	val = tegra_se_save_sha256_pmc_scratch();
	if (val != 0) {
		ERROR("%s: SE0 status Error.\n", __func__);
	}

	return val;
}

/*
 * Handler to power down the SE hardware blocks - SE, RNG1 and PKA1. This
 * needs to be called only during System Suspend.
 */
int32_t tegra_se_suspend(void)
{
	int32_t ret = 0;

	/* initialise communication channel with BPMP */
	assert(tegra_bpmp_ipc_init() == 0);

	/* Enable SE clock before SE context save */
	ret = tegra_bpmp_ipc_enable_clock(TEGRA194_CLK_SE);
	assert(ret == 0);

	/* save SE registers */
	se_regs[0] = mmio_read_32(TEGRA_SE0_BASE + SE0_MUTEX_WATCHDOG_NS_LIMIT);
	se_regs[1] = mmio_read_32(TEGRA_SE0_BASE + SE0_AES0_ENTROPY_SRC_AGE_CTRL);
	se_regs[2] = mmio_read_32(TEGRA_RNG1_BASE + RNG1_MUTEX_WATCHDOG_NS_LIMIT);
	se_regs[3] = mmio_read_32(TEGRA_PKA1_BASE + PKA1_MUTEX_WATCHDOG_NS_LIMIT);

	/* Save SE context. The BootROM restores it during System Resume */
	ret = tegra_se_save_context();
	if (ret != 0) {
		ERROR("%s: context save failed (%d)\n", __func__, ret);
	}

	/* Disable SE clock after SE context save */
	ret = tegra_bpmp_ipc_disable_clock(TEGRA194_CLK_SE);
	assert(ret == 0);

	return ret;
}

/*
 * Handler to power up the SE hardware block(s) during System Resume.
 */
void tegra_se_resume(void)
{
	int32_t ret = 0;

	/* initialise communication channel with BPMP */
	assert(tegra_bpmp_ipc_init() == 0);

	/* Enable SE clock before SE context restore */
	ret = tegra_bpmp_ipc_enable_clock(TEGRA194_CLK_SE);
	assert(ret == 0);

	/*
	 * When TZ takes over after System Resume, TZ should first reconfigure
	 * SE_MUTEX_WATCHDOG_NS_LIMIT, PKA1_MUTEX_WATCHDOG_NS_LIMIT,
	 * RNG1_MUTEX_WATCHDOG_NS_LIMIT and SE_ENTROPY_SRC_AGE_CTRL before
	 * other operations.
	 */
	mmio_write_32(TEGRA_SE0_BASE + SE0_MUTEX_WATCHDOG_NS_LIMIT, se_regs[0]);
	mmio_write_32(TEGRA_SE0_BASE + SE0_AES0_ENTROPY_SRC_AGE_CTRL, se_regs[1]);
	mmio_write_32(TEGRA_RNG1_BASE + RNG1_MUTEX_WATCHDOG_NS_LIMIT, se_regs[2]);
	mmio_write_32(TEGRA_PKA1_BASE + PKA1_MUTEX_WATCHDOG_NS_LIMIT, se_regs[3]);

	/* Disable SE clock after SE context restore */
	ret = tegra_bpmp_ipc_disable_clock(TEGRA194_CLK_SE);
	assert(ret == 0);
}

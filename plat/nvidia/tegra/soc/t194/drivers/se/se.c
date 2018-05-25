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
#define ERR_STATUS_SW_CLEAR	U(0xFFFFFFFF)
#define INT_STATUS_SW_CLEAR	U(0xFFFFFFFF)
#define MAX_TIMEOUT_MS		U(100)	/* Timeout in 100ms */
#define NUM_SE_REGS_TO_SAVE	U(4)

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
 * Handler to power down the SE hardware blocks - SE, RNG1 and PKA1. This
 * needs to be called only during System Suspend.
 */
int32_t tegra_se_suspend(void)
{
	int32_t ret = 0;

	/* initialise communication channel with BPMP */
	assert(tegra_bpmp_ipc_init() == 0);

	/* Enable SE clock before SE context save */
	ret = tegra_bpmp_ipc_enable_clock(TEGRA_CLK_SE);
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
	ret = tegra_bpmp_ipc_disable_clock(TEGRA_CLK_SE);
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
	ret = tegra_bpmp_ipc_enable_clock(TEGRA_CLK_SE);
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
	ret = tegra_bpmp_ipc_disable_clock(TEGRA_CLK_SE);
	assert(ret == 0);
}

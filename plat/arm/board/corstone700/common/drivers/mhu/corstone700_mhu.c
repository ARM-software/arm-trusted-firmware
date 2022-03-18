/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include "corstone700_mhu.h"
#include <plat_arm.h>
#include <platform_def.h>

ARM_INSTANTIATE_LOCK;

#pragma weak plat_arm_pwrc_setup

/*
 * Slot 31 is reserved because the MHU hardware uses this register bit to
 * indicate a non-secure access attempt. The total number of available slots is
 * therefore 31 [30:0].
 */
#define MHU_MAX_SLOT_ID		30

void mhu_secure_message_start(uintptr_t address, unsigned int slot_id)
{
	unsigned int intr_stat_check;
	uint64_t timeout_cnt;
	volatile uint8_t expiration;

	assert(slot_id <= MHU_MAX_SLOT_ID);
	arm_lock_get();

	/*
	 * Make sure any previous command has finished
	 * and polling timeout not expired
	 */

	timeout_cnt = timeout_init_us(MHU_POLL_INTR_STAT_TIMEOUT);

	do {
		intr_stat_check = (mmio_read_32(address + CPU_INTR_S_STAT) &
						(1 << slot_id));

		expiration = timeout_elapsed(timeout_cnt);

	} while ((intr_stat_check != 0U) && (expiration == 0U));

	/*
	 * Note: No risk of timer overflows while waiting
	 * for the timeout expiration.
	 * According to Armv8 TRM: System counter roll-over
	 * time of not less than 40 years
	 */
}

void mhu_secure_message_send(uintptr_t address,
				unsigned int slot_id,
				unsigned int message)
{
	unsigned char access_ready;
	uint64_t timeout_cnt;
	volatile uint8_t expiration;

	assert(slot_id <= MHU_MAX_SLOT_ID);
	assert((mmio_read_32(address + CPU_INTR_S_STAT) &
						(1 << slot_id)) == 0U);

	MHU_V2_ACCESS_REQUEST(address);

	timeout_cnt = timeout_init_us(MHU_POLL_INTR_STAT_TIMEOUT);

	do {
		access_ready = MHU_V2_IS_ACCESS_READY(address);
		expiration = timeout_elapsed(timeout_cnt);

	} while ((access_ready == 0U) && (expiration == 0U));

	/*
	 * Note: No risk of timer overflows while waiting
	 * for the timeout expiration.
	 * According to Armv8 TRM: System counter roll-over
	 * time of not less than 40 years
	 */

	mmio_write_32(address + CPU_INTR_S_SET, message);
}

void mhu_secure_message_end(uintptr_t address, unsigned int slot_id)
{
	assert(slot_id <= MHU_MAX_SLOT_ID);
	/*
	 * Clear any response we got by writing one in the relevant slot bit to
	 * the CLEAR register
	 */
	MHU_V2_CLEAR_REQUEST(address);

	arm_lock_release();
}

void __init mhu_secure_init(void)
{
	arm_lock_init();

	/*
	 * The STAT register resets to zero. Ensure it is in the expected state,
	 * as a stale or garbage value would make us think it's a message we've
	 * already sent.
	 */

	assert(mmio_read_32(PLAT_SDK700_MHU0_SEND + CPU_INTR_S_STAT) == 0);
}

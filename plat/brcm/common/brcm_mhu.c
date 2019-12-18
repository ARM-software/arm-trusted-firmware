/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>

#include <brcm_mhu.h>
#include <platform_def.h>

#include "m0_ipc.h"

#define PLAT_MHU_INTR_REG	AP_TO_SCP_MAILBOX1

/* SCP MHU secure channel registers */
#define SCP_INTR_S_STAT		CRMU_IHOST_SW_PERSISTENT_REG11
#define SCP_INTR_S_SET		CRMU_IHOST_SW_PERSISTENT_REG11
#define SCP_INTR_S_CLEAR	CRMU_IHOST_SW_PERSISTENT_REG11

/* CPU MHU secure channel registers */
#define CPU_INTR_S_STAT		CRMU_IHOST_SW_PERSISTENT_REG10
#define CPU_INTR_S_SET		CRMU_IHOST_SW_PERSISTENT_REG10
#define CPU_INTR_S_CLEAR	CRMU_IHOST_SW_PERSISTENT_REG10

static DEFINE_BAKERY_LOCK(bcm_lock);

/*
 * Slot 31 is reserved because the MHU hardware uses this register bit to
 * indicate a non-secure access attempt. The total number of available slots is
 * therefore 31 [30:0].
 */
#define MHU_MAX_SLOT_ID		30

void mhu_secure_message_start(unsigned int slot_id)
{
	int iter = 1000000;

	assert(slot_id <= MHU_MAX_SLOT_ID);

	bakery_lock_get(&bcm_lock);
	/* Make sure any previous command has finished */
	do {
		if (!(mmio_read_32(PLAT_BRCM_MHU_BASE + CPU_INTR_S_STAT) &
		   (1 << slot_id)))
			break;

		 udelay(1);

	} while (--iter);

	assert(iter != 0);
}

void mhu_secure_message_send(unsigned int slot_id)
{
	uint32_t response, iter = 1000000;

	assert(slot_id <= MHU_MAX_SLOT_ID);
	assert(!(mmio_read_32(PLAT_BRCM_MHU_BASE + CPU_INTR_S_STAT) &
							(1 << slot_id)));

	/* Send command to SCP */
	mmio_setbits_32(PLAT_BRCM_MHU_BASE + CPU_INTR_S_SET, 1 << slot_id);
	mmio_write_32(CRMU_MAIL_BOX0, MCU_IPC_MCU_CMD_SCPI);
	mmio_write_32(PLAT_BRCM_MHU_BASE + PLAT_MHU_INTR_REG, 0x1);

	/* Wait until IPC transport acknowledges reception of SCP command */
	do {
		response = mmio_read_32(CRMU_MAIL_BOX0);
		if ((response & ~MCU_IPC_CMD_REPLY_MASK) ==
		   (MCU_IPC_CMD_DONE_MASK | MCU_IPC_MCU_CMD_SCPI))
			break;

		udelay(1);

	} while (--iter);

	assert(iter != 0);
}

uint32_t mhu_secure_message_wait(void)
{
	/* Wait for response from SCP */
	uint32_t response, iter = 1000000;

	do {
		response = mmio_read_32(PLAT_BRCM_MHU_BASE + SCP_INTR_S_STAT);
		if (!response)
			break;

		udelay(1);
	} while (--iter);
	assert(iter != 0);

	return response;
}

void mhu_secure_message_end(unsigned int slot_id)
{
	assert(slot_id <= MHU_MAX_SLOT_ID);

	/*
	 * Clear any response we got by writing one in the relevant slot bit to
	 * the CLEAR register
	 */
	mmio_clrbits_32(PLAT_BRCM_MHU_BASE + SCP_INTR_S_CLEAR, 1 << slot_id);
	bakery_lock_release(&bcm_lock);
}

void mhu_secure_init(void)
{
	bakery_lock_init(&bcm_lock);

	/*
	 * The STAT register resets to zero. Ensure it is in the expected state,
	 * as a stale or garbage value would make us think it's a message we've
	 * already sent.
	 */
	mmio_write_32(PLAT_BRCM_MHU_BASE + CPU_INTR_S_STAT, 0);
	mmio_write_32(PLAT_BRCM_MHU_BASE + SCP_INTR_S_STAT, 0);
}

void plat_brcm_pwrc_setup(void)
{
	mhu_secure_init();
}

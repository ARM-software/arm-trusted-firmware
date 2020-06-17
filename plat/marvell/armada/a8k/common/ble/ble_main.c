/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>

#include <marvell_plat_priv.h>
#include <marvell_pm.h>
#include <plat_marvell.h>

#define BR_FLAG_SILENT		0x1
#define SKIP_IMAGE_CODE		0xDEADB002

void mailbox_clean(void)
{
	uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

	memset(mailbox, 0, PLAT_MARVELL_MAILBOX_SIZE);
}

int exec_ble_main(int bootrom_flags)
{
	int skip = 0;
	uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

	/*
	 * In some situations, like boot from UART, bootrom will
	 * request to avoid printing to console. in that case don't
	 * initialize the console and prints will be ignored
	 */
	if ((bootrom_flags & BR_FLAG_SILENT) == 0)
		marvell_console_boot_init();

	NOTICE("Starting binary extension\n");

	/* initialize time (for delay functionality) */
	plat_delay_timer_init();

	ble_plat_setup(&skip);

	/* if there's skip image request, bootrom will load from the image
	 * saved on the next address of the flash
	 */
	if (skip)
		return SKIP_IMAGE_CODE;

	/*
	 * Check if the mailbox magic number is stored at index MBOX_IDX_MAGIC
	 * and the suspend to RAM magic number at index MBOX_IDX_SUSPEND_MAGIC.
	 * If the above is true, this is the recovery from suspend to RAM state.
	 * In such case the mailbox should remain intact, since it stores the
	 * warm boot jump address to be used by the TF-A in BL31.
	 * Othervise the mailbox should be cleaned from a garbage data.
	 */
	if (mailbox[MBOX_IDX_MAGIC] != MVEBU_MAILBOX_MAGIC_NUM ||
	    mailbox[MBOX_IDX_SUSPEND_MAGIC] != MVEBU_MAILBOX_SUSPEND_STATE) {
		NOTICE("Cold boot\n");
		mailbox_clean();
	} else {
		void (*bootrom_exit)(void) =
			(void (*)(void))mailbox[MBOX_IDX_ROM_EXIT_ADDR];

		INFO("Recovery...\n");
		/*
		 * If this is recovery from suspend, two things has to be done:
		 * 1. Define the DRAM region as executable memory for preparing
		 *    jump to TF-A
		 * 2. Instead of returning control to the BootROM, invalidate
		 *    and flush caches, and continue execution at address stored
		 *    in the mailbox.
		 * This should be done until the BootROM have a native support
		 * for the system restore flow.
		 */
		marvell_ble_prepare_exit();
		bootrom_exit();
	}

	return 0;
}

/* NOTE: don't notify this function, all code must be added to exec_ble_main
 * in order to keep the end of ble_main as a fixed address.
 */
int  __attribute__ ((section(".entry"))) ble_main(int bootrom_flags)
{
	volatile int ret;

	ret = exec_ble_main(bootrom_flags);
	return ret;
}

/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include <plat_common.h>
#if TRUSTED_BOARD_BOOT
#include <snvs.h>
#endif

/*
 * Error handler
 */
void plat_error_handler(int err)
{
#if TRUSTED_BOARD_BOOT
	uint32_t mode;
	bool sb = check_boot_mode_secure(&mode);
#endif

	switch (err) {
	case -ENOENT:
	case -EAUTH:
		printf("Authentication failure\n");
#if TRUSTED_BOARD_BOOT
		/* For SB production mode i.e ITS = 1 */
		if (sb == true) {
			if (mode == 1)
				transition_snvs_soft_fail();
			else
				transition_snvs_non_secure();
		}
#endif
		break;
	default:
		/* Unexpected error */
		break;
	}

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}

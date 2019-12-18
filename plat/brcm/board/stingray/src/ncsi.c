/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <ncsi.h>
#include <sr_def.h>
#include <sr_utils.h>

static const char *const io_drives[] = {
	"2mA", "4mA", "6mA", "8mA",
	"10mA", "12mA", "14mA", "16mA"
};

void brcm_stingray_ncsi_init(void)
{
	unsigned int i = 0;
	unsigned int selx = 0;

#if NCSI_IO_DRIVE_STRENGTH_MA == 2
	selx = 0x0;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 4
	selx = 0x1;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 6
	selx = 0x2;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 8
	selx = 0x3;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 10
	selx = 0x4;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 12
	selx = 0x5;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 14
	selx = 0x6;
#elif NCSI_IO_DRIVE_STRENGTH_MA == 16
	selx = 0x7;
#else
	ERROR("Unsupported NCSI_IO_DRIVE_STRENGTH_MA. Please check it.\n");
	return;
#endif
	INFO("ncsi io drives: %s\n", io_drives[selx]);

	for (i = 0; i < NITRO_NCSI_IOPAD_CONTROL_NUM; i++) {
		mmio_clrsetbits_32((NITRO_NCSI_IOPAD_CONTROL_BASE + (i * 4)),
				   PAD_SELX_MASK, PAD_SELX_VALUE(selx));
	}

	INFO("ncsi init done\n");
}

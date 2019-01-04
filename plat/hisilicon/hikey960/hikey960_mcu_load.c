/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <hi3660.h>

#define ADDR_CONVERT(addr)		((addr) < 0x40000 ?	\
					 (addr) + 0xFFF30000 :	\
					 (addr) + 0x40000000)

static void fw_data_init(void)
{
	unsigned long data_head_addr;
	unsigned int *data_addr;

	data_head_addr = mmio_read_32((uintptr_t) HISI_DATA_HEAD_BASE) + 0x14;
	data_addr = (unsigned int *) ADDR_CONVERT(data_head_addr);

	memcpy((void *)HISI_DATA0_BASE,
	       (const void *)(unsigned long)ADDR_CONVERT(data_addr[0]),
	       HISI_DATA0_SIZE);
	memcpy((void *)HISI_DATA1_BASE,
	       (const void *)(unsigned long)ADDR_CONVERT(data_addr[1]),
	       HISI_DATA1_SIZE);
}

int load_lpm3(void)
{
	INFO("start fw loading\n");

	fw_data_init();

	flush_dcache_range((uintptr_t)HISI_RESERVED_MEM_BASE,
			   HISI_RESERVED_MEM_SIZE);

	sev();
	sev();

	INFO("fw load success\n");

	return 0;
}

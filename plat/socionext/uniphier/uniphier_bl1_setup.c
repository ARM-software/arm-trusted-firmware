/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_mmu_helpers.h>

#include "uniphier.h"

void bl1_early_platform_setup(void)
{
	uniphier_console_setup();
}

void bl1_plat_arch_setup(void)
{
	uniphier_mmap_setup(UNIPHIER_SEC_DRAM_BASE, UNIPHIER_SEC_DRAM_SIZE,
			    NULL);
	enable_mmu_el3(0);
}

void bl1_platform_setup(void)
{
	unsigned int soc;
	int ret;

	soc = uniphier_get_soc_id();
	if (soc == UNIPHIER_SOC_UNKNOWN) {
		ERROR("unsupported SoC\n");
		plat_error_handler(-ENOTSUP);
	}

	ret = uniphier_io_setup(soc);
	if (ret) {
		ERROR("failed to setup io devices\n");
		plat_error_handler(ret);
	}
}

static meminfo_t uniphier_tzram_layout = {
	.total_base = UNIPHIER_SEC_DRAM_BASE,
	.total_size = UNIPHIER_SEC_DRAM_SIZE,
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &uniphier_tzram_layout;
}

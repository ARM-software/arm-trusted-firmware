/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <desc_image_load.h>
#include <errno.h>
#include <io/io_storage.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_tables_v2.h>

#include "uniphier.h"

static meminfo_t uniphier_bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);
static int uniphier_bl2_kick_scp;

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	uniphier_bl2_tzram_layout = *mem_layout;

	uniphier_console_setup();
}

static const struct mmap_region uniphier_bl2_mmap[] = {
	/* for SCP, BL33 */
	MAP_REGION_FLAT(UNIPHIER_NS_DRAM_BASE, UNIPHIER_NS_DRAM_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	{ .size = 0 },
};

void bl2_plat_arch_setup(void)
{
	unsigned int soc;
	int skip_scp = 0;
	int ret;

	uniphier_mmap_setup(UNIPHIER_SEC_DRAM_BASE, UNIPHIER_SEC_DRAM_SIZE,
			    uniphier_bl2_mmap);
	enable_mmu_el1(0);

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

	switch (uniphier_get_boot_master(soc)) {
	case UNIPHIER_BOOT_MASTER_THIS:
		INFO("Booting from this SoC\n");
		skip_scp = 1;
		break;
	case UNIPHIER_BOOT_MASTER_SCP:
		INFO("Booting from on-chip SCP\n");
		if (uniphier_scp_is_running()) {
			INFO("SCP is already running. SCP_BL2 load will be skipped.\n");
			skip_scp = 1;
		}

		/*
		 * SCP must be kicked every time even if it is already running
		 * because it polls this event after the reboot of the backend.
		 */
		uniphier_bl2_kick_scp = 1;
		break;
	case UNIPHIER_BOOT_MASTER_EXT:
		INFO("Booting from external SCP\n");
		skip_scp = 1;
		break;
	default:
		plat_error_handler(-ENOTSUP);
	}

	if (!skip_scp) {
		ret = uniphier_check_image(SCP_BL2_IMAGE_ID);
		if (ret) {
			WARN("SCP_BL2 image not found. SCP_BL2 load will be skipped.\n");
			WARN("You must setup SCP by other means.\n");
			skip_scp = 1;
			uniphier_bl2_kick_scp = 0;
		}
	}

	if (skip_scp)
		uniphier_image_descs_fixup();
}

void bl2_platform_setup(void)
{
}

void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	if (image_id == SCP_BL2_IMAGE_ID && uniphier_bl2_kick_scp)
		uniphier_scp_start();

	return 0;
}

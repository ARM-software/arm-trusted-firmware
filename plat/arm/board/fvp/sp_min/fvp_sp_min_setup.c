/*
 * Copyright (c) 2016-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <bl32/sp_min/platform_sp_min.h>
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>

#include "../fvp_private.h"

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	const struct dyn_cfg_dtb_info_t *tos_fw_config_info __unused;

	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

#if !RESET_TO_SP_MIN && !BL2_AT_EL3

	INFO("SP_MIN FCONF: FW_CONFIG address = %lx\n", (uintptr_t)arg1);
	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", arg1);

	tos_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, TOS_FW_CONFIG_ID);
	if (tos_fw_config_info != NULL) {
		arg1 = tos_fw_config_info->config_addr;
	}
#endif /* !RESET_TO_SP_MIN && !BL2_AT_EL3 */

	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * FVP PSCI code will enable coherency for other clusters.
	 */
	fvp_interconnect_enable();
}

void sp_min_plat_arch_setup(void)
{
	int rc __unused;
	const struct dyn_cfg_dtb_info_t *hw_config_info __unused;
	uintptr_t hw_config_base_align __unused;
	size_t mapped_size_align __unused;

	arm_sp_min_plat_arch_setup();

	/*
	 * For RESET_TO_SP_MIN systems, SP_MIN(BL32) is the first bootloader
	 * to run. So there is no BL2 to load the HW_CONFIG dtb into memory
	 * before control is passed to SP_MIN.
	 * Also, BL2 skips loading HW_CONFIG dtb for BL2_AT_EL3 builds.
	 * The code below relies on dynamic mapping capability, which is not
	 * supported by xlat tables lib V1.
	 * TODO: remove the ARM_XLAT_TABLES_LIB_V1 check when its support
	 * gets deprecated.
	 */
#if !RESET_TO_SP_MIN && !BL2_AT_EL3 && !ARM_XLAT_TABLES_LIB_V1
	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);
	assert(hw_config_info->config_addr != 0UL);

	INFO("SP_MIN FCONF: HW_CONFIG address = %p\n",
	     (void *)hw_config_info->config_addr);

	/*
	 * Preferrably we expect this address and size are page aligned,
	 * but if they are not then align it.
	 */
	hw_config_base_align = page_align(hw_config_info->config_addr, DOWN);
	mapped_size_align = page_align(hw_config_info->config_max_size, UP);

	if ((hw_config_info->config_addr != hw_config_base_align) &&
	    (hw_config_info->config_max_size == mapped_size_align)) {
		mapped_size_align += PAGE_SIZE;
	}

	/*
	 * map dynamically HW config region with its aligned base address and
	 * size
	 */
	rc = mmap_add_dynamic_region((unsigned long long)hw_config_base_align,
				     hw_config_base_align,
				     mapped_size_align,
				     MT_RO_DATA);
	if (rc != 0) {
		ERROR("Error while mapping HW_CONFIG device tree (%d).\n", rc);
		panic();
	}

	/* Populate HW_CONFIG device tree with the mapped address */
	fconf_populate("HW_CONFIG", hw_config_info->config_addr);

	/* unmap the HW_CONFIG memory region */
	rc = mmap_remove_dynamic_region(hw_config_base_align, mapped_size_align);
	if (rc != 0) {
		ERROR("Error while unmapping HW_CONFIG device tree (%d).\n",
		      rc);
		panic();
	}
#endif /* !RESET_TO_SP_MIN && !BL2_AT_EL3 && !ARM_XLAT_TABLES_LIB_V1 */
}

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>

#include <plat/arm/common/plat_arm.h>

void __init bl31_early_platform_setup2(u_register_t arg0,
		u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	const struct dyn_cfg_dtb_info_t *soc_fw_config_info;

	INFO("BL31 FCONF: FW_CONFIG address = %lx\n", (uintptr_t)arg1);

	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", arg1);

	soc_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, SOC_FW_CONFIG_ID);
	if (soc_fw_config_info != NULL) {
		arg1 = soc_fw_config_info->config_addr;
	}

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_arm_interconnect_enter_coherency();
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();

	/* HW_CONFIG was also loaded by BL2 */
	const struct dyn_cfg_dtb_info_t *hw_config_info;

	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	fconf_populate("HW_CONFIG", hw_config_info->config_addr);
}

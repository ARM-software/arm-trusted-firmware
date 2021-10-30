/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/arm/smmu_v3.h>
#include <fconf_hw_config_getter.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/mmio.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "fvp_private.h"

void __init bl31_early_platform_setup2(u_register_t arg0,
		u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
#if !RESET_TO_BL31 && !BL2_AT_EL3
	const struct dyn_cfg_dtb_info_t *soc_fw_config_info;

	INFO("BL31 FCONF: FW_CONFIG address = %lx\n", (uintptr_t)arg1);
	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", arg1);

	soc_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, SOC_FW_CONFIG_ID);
	if (soc_fw_config_info != NULL) {
		arg1 = soc_fw_config_info->config_addr;
	}
#endif /* !RESET_TO_BL31 && !BL2_AT_EL3 */

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

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

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();

	/* On FVP RevC, initialize SMMUv3 */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U)
		smmuv3_init(PLAT_FVP_SMMUV3_BASE);
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();

	/*
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run.
	 * So there is no BL2 to load the HW_CONFIG dtb into memory before
	 * control is passed to BL31.
	 */
#if !RESET_TO_BL31 && !BL2_AT_EL3
	/* HW_CONFIG was also loaded by BL2 */
	const struct dyn_cfg_dtb_info_t *hw_config_info;

	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	fconf_populate("HW_CONFIG", hw_config_info->config_addr);
#endif
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;

#if !RESET_TO_BL31 && !BL2_AT_EL3
	/* Get the frequency through FCONF API for HW_CONFIG */
	counter_base_frequency = FCONF_GET_PROPERTY(hw_config, cpu_timer, clock_freq);
	if (counter_base_frequency > 0U) {
		return counter_base_frequency;
	}
#endif

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(ARM_SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	if (counter_base_frequency == 0U) {
		panic();
	}

	return counter_base_frequency;
}

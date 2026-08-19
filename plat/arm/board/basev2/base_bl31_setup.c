/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/dsu.h>
#include <drivers/arm/sbsa.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <psa/error.h>

#include <base_plat.h>

static scmi_channel_plat_info_t base_scmi_plat_info = {
	.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
	.db_reg_addr = PLAT_CSS_MHU_BASE + MHU_V3_SENDER_REG_SET(0),
	.db_preserve_mask = 0xfffffffe,
	.db_modify_mask = 0x1,
	.ring_doorbell = &mhu_ring_doorbell,
};

const dsu_driver_data_t plat_dsu_data = {
	.clusterpwrdwn_pwrdn = false,
	.clusterpwrdwn_memret = false,
	.clusterpwrctlr_cachepwr = CLUSTERPWRCTLR_CACHEPWR_RESET,
	.clusterpwrctlr_funcret = CLUSTERPWRCTLR_FUNCRET_RESET
};

void bl31_platform_setup(void)
{
	base_bl31_common_platform_setup();

#if TRUSTED_BOARD_BOOT || MEASURED_BOOT
	/* Initialise RSE communication channel */
	psa_status_t status;

	status = base_rse_comms_init();
	if (status != PSA_SUCCESS) {
		ERROR("Failed to initialize RSE communication channel - psa_status = %d\n", status);
	}
#endif
}

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id __unused)
{

	return &base_scmi_plat_info;

}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	arm_console_boot_init();
	arm_bl31_early_platform_setup(arg0, arg1, arg2, arg3);

#if !TRANSFER_LIST
	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", arg1);
#endif
}

void base_bl31_common_platform_setup(void)
{
	arm_bl31_platform_setup();
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();

	gic_set_gicr_frames(arm_gicr_base_addrs);

#if !TRANSFER_LIST
	/* HW_CONFIG was also loaded by BL2 */
	const struct dyn_cfg_dtb_info_t *hw_config_info;

	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	fconf_populate("HW_CONFIG", hw_config_info->config_addr);
#endif /* TRANSFER_LIST */
}

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
void base_bl31_plat_runtime_setup(void)
{
	/* Start secure watchdog timer. */
	plat_arm_secure_wdt_start();

	arm_bl31_plat_runtime_setup();
}

void bl31_plat_runtime_setup(void)
{
	base_bl31_plat_runtime_setup();
}

/*
 * Platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/* Trusted Watchdog timer is the only source of Group0 interrupt now. */
	if (intid == SBSA_SECURE_WDOG_INTID) {
		/* Refresh the timer. */
		plat_arm_secure_wdt_refresh();

		return 0;
	}

	return -1;
}
#endif /*defined(SPD_spmd) && (SPMC_AT_EL3 == 0)*/

/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/generic_delay_timer.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>
#include <plat/common/platform.h>

#include <nrd_ras.h>
#include <nrd_variant.h>

nrd_platform_info_t nrd_plat_info;

static scmi_channel_plat_info_t plat_rd_scmi_info[] = {
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
	},
	#if (NRD_CHIP_COUNT > 1)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
			NRD_REMOTE_CHIP_MEM_OFFSET(1),
		.db_reg_addr = PLAT_CSS_MHU_BASE
			+ NRD_REMOTE_CHIP_MEM_OFFSET(1) + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
	},
	#endif
	#if (NRD_CHIP_COUNT > 2)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
			NRD_REMOTE_CHIP_MEM_OFFSET(2),
		.db_reg_addr = PLAT_CSS_MHU_BASE +
			NRD_REMOTE_CHIP_MEM_OFFSET(2) + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
	},
	#endif
	#if (NRD_CHIP_COUNT > 3)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
			NRD_REMOTE_CHIP_MEM_OFFSET(3),
		.db_reg_addr = PLAT_CSS_MHU_BASE +
			NRD_REMOTE_CHIP_MEM_OFFSET(3) + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
	},
	#endif
};

static scmi_channel_plat_info_t plat3_rd_scmi_info[] = {
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
	#if (NRD_CHIP_COUNT > 1)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(1),
		.db_reg_addr = PLAT_CSS_MHU_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(1) +
					MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
	#endif
	#if (NRD_CHIP_COUNT > 2)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(2),
		.db_reg_addr = PLAT_CSS_MHU_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(2) +
					MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
	#endif
	#if (NRD_CHIP_COUNT > 3)
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(3),
		.db_reg_addr = PLAT_CSS_MHU_BASE +
					NRD_REMOTE_CHIP_MEM_OFFSET(3) +
					MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
	#endif
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id)
{
	if (nrd_plat_info.platform_id == RD_N2_SID_VER_PART_NUM ||
		nrd_plat_info.platform_id == RD_V2_SID_VER_PART_NUM ||
		nrd_plat_info.platform_id == RD_N2_CFG1_SID_VER_PART_NUM ||
		nrd_plat_info.platform_id == RD_N2_CFG3_SID_VER_PART_NUM) {
		if (channel_id >= ARRAY_SIZE(plat_rd_scmi_info)) {
			panic();
		}
		return &plat_rd_scmi_info[channel_id];
	} else if (nrd_plat_info.platform_id == RD_V3_SID_VER_PART_NUM ||
		nrd_plat_info.platform_id == RD_V3_CFG1_SID_VER_PART_NUM ||
		nrd_plat_info.platform_id == RD_V3_CFG2_SID_VER_PART_NUM) {
		if (channel_id >= ARRAY_SIZE(plat3_rd_scmi_info)) {
			panic();
		}
		return &plat3_rd_scmi_info[channel_id];
	} else {
		panic();
	}
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	nrd_plat_info.platform_id = plat_arm_nrd_get_platform_id();
	nrd_plat_info.config_id = plat_arm_nrd_get_config_id();
	nrd_plat_info.multi_chip_mode = plat_arm_nrd_get_multi_chip_mode();

#if RESET_TO_BL31
#if (ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_GENERIC_TIMER)
	/* Set the counter frequency for the generic timer */
	write_cntfrq_el0(plat_get_syscnt_freq2());
#endif
#endif /* RESET_TO_BL31 */

	/* Initialize generic timer */
	generic_delay_timer_init();

#if SPMD_SPM_AT_SEL2 && !RESET_TO_BL31
	INFO("BL31 FCONF: FW_CONFIG address = 0x%lx\n", (uintptr_t)arg1);
	/* Initialize BL31's copy of the DTB registry because SPMD needs the
	 * TOS_FW_CONFIG's addresses to make a copy.
	 */
	fconf_populate("FW_CONFIG", arg1);

	/* arg1 is supposed to point to SOC_FW_CONFIG */
	const struct dyn_cfg_dtb_info_t *soc_fw_config_info;

	soc_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, SOC_FW_CONFIG_ID);
	if (soc_fw_config_info != NULL) {
		arg1 = soc_fw_config_info->config_addr;
	}
#endif /* SPMD_SPM_AT_SEL2 && !RESET_TO_BL31 */
	arm_bl31_early_platform_setup(arg0, arg1, arg2, arg3);
}

/*******************************************************************************
 * This function inserts platform information via device tree nodes as,
 * system-id {
 *    platform-id = <0>;
 *    config-id = <0>;
 * }
 ******************************************************************************/
#if RESET_TO_BL31
static int append_config_node(uintptr_t fdt_base_addr, uintptr_t fdt_base_size)
{
	void *fdt;
	int nodeoffset, err;
	unsigned int platid = 0, platcfg = 0;

	if (fdt_base_addr == 0) {
		ERROR("NT_FW CONFIG base address is NULL\n");
		return -1;
	}

	fdt = (void *)fdt_base_addr;

	/* Check the validity of the fdt */
	if (fdt_check_header(fdt) != 0) {
		ERROR("Invalid NT_FW_CONFIG DTB passed\n");
		return -1;
	}

	nodeoffset = fdt_subnode_offset(fdt, 0, "system-id");
	if (nodeoffset < 0) {
		ERROR("Failed to get system-id node offset\n");
		return -1;
	}

	platid = plat_arm_nrd_get_platform_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "platform-id", platid);
	if (err < 0) {
		ERROR("Failed to set platform-id\n");
		return -1;
	}

	platcfg = plat_arm_nrd_get_config_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "config-id", platcfg);
	if (err < 0) {
		ERROR("Failed to set config-id\n");
		return -1;
	}

	platcfg = plat_arm_nrd_get_multi_chip_mode();
	err = fdt_setprop_u32(fdt, nodeoffset, "multi-chip-mode", platcfg);
	if (err < 0) {
		ERROR("Failed to set multi-chip-mode\n");
		return -1;
	}

	flush_dcache_range((uintptr_t)fdt, fdt_base_size);
	return 0;
}
#endif

void nrd_bl31_common_platform_setup(void)
{

	arm_bl31_platform_setup();

	/* Configure the warm reboot SGI for primary core */
	css_setup_cpu_pwr_down_intr();

#if CSS_SYSTEM_GRACEFUL_RESET
	/* Register priority level handlers for reboot */
	ehf_register_priority_handler(PLAT_REBOOT_PRI,
			css_reboot_interrupt_handler);
#endif

#if RESET_TO_BL31
	int ret = append_config_node(NRD_CSS_BL31_PRELOAD_DTB_BASE,
			NRD_CSS_BL31_PRELOAD_DTB_SIZE);

	if (ret != 0) {
		panic();
	}
#endif
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

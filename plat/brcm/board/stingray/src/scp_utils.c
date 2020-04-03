/*
 * Copyright (c) 2017-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <bcm_elog_ddr.h>
#include <brcm_mhu.h>
#include <brcm_scpi.h>
#include <chimp.h>
#include <cmn_plat_util.h>
#include <ddr_init.h>
#include <scp.h>
#include <scp_cmd.h>
#include <scp_utils.h>

#include "m0_cfg.h"
#include "m0_ipc.h"

#ifdef BCM_ELOG
static void prepare_elog(void)
{
#if (CLEAN_DDR && !defined(MMU_DISABLED))
	/*
	 * Now DDR has been initialized. We want to copy all the logs in SRAM
	 * into DDR so we will have much more space to store the logs in the
	 * next boot stage
	 */
	bcm_elog_copy_log((void *)BCM_ELOG_BL31_BASE,
			   MIN(BCM_ELOG_BL2_SIZE, BCM_ELOG_BL31_SIZE)
			 );

	/*
	 * We are almost at the end of BL2, and we can stop log here so we do
	 * not need to add 'bcm_elog_exit' to the standard BL2 code. The
	 * benefit of capturing BL2 logs after this is very minimal in a
	 * production system.
	 */
	bcm_elog_exit();
#endif

	/*
	 * Notify CRMU that now it should pull logs from DDR instead of from
	 * FS4 SRAM.
	 */
	SCP_WRITE_CFG(flash_log.can_use_ddr, 1);
}
#endif

bool is_crmu_alive(void)
{
	return (scp_send_cmd(MCU_IPC_MCU_CMD_NOP, 0, SCP_CMD_DEFAULT_TIMEOUT_US)
		== 0);
}

bool bcm_scp_issue_sys_reset(void)
{
	return (scp_send_cmd(MCU_IPC_MCU_CMD_L1_RESET, 0,
			     SCP_CMD_DEFAULT_TIMEOUT_US));
}

/*
 * Note that this is just a temporary implementation until
 * channels are introduced
 */

int plat_bcm_bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int scp_patch_activated, scp_patch_version;
#ifndef EMULATION_SETUP
	uint8_t active_ch_bitmap, i;
#endif
	uint32_t reset_state = 0;
	uint32_t mcu_ap_init_param = 0;

	/*
	 * First check if SCP patch has already been loaded
	 * Send NOP command and see if there is a valid response
	 */
	scp_patch_activated =
		(scp_send_cmd(MCU_IPC_MCU_CMD_NOP, 0,
		SCP_CMD_DEFAULT_TIMEOUT_US) == 0);
	if (scp_patch_activated) {
		INFO("SCP Patch is already active.\n");

		reset_state =  SCP_READ_CFG(board_cfg.reset_state);
		mcu_ap_init_param = SCP_READ_CFG(board_cfg.mcu_init_param);

		/* Clear reset state, it's been already read */
		SCP_WRITE_CFG(board_cfg.reset_state, 0);

		if (mcu_ap_init_param & MCU_PATCH_LOADED_BY_NITRO) {
			/*
			 * Reset "MCU_PATCH_LOADED_BY_NITRO" flag, but
			 * Preserve any other flags we don't deal with here
			 */
			INFO("AP booted by Nitro\n");
			SCP_WRITE_CFG(
					board_cfg.mcu_init_param,
					mcu_ap_init_param &
						~MCU_PATCH_LOADED_BY_NITRO
				      );
		}
	} else {
		/*
		 * MCU Patch not loaded, so load it.
		 * MCU patch stamps critical points in REG9 (debug test-point)
		 * Display its last content here. This helps to locate
		 * where crash occurred if a CRMU watchdog kicked in.
		 */
		int ret;

		INFO("MCU Patch Point: 0x%x\n",
			mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG9));

		ret = download_scp_patch((void *)scp_bl2_image_info->image_base,
				scp_bl2_image_info->image_size);
		if (ret != 0)
			return ret;

		VERBOSE("SCP Patch loaded OK.\n");

		ret = scp_send_cmd(MCU_IPC_MCU_CMD_INIT,
				MCU_PATCH_LOADED_BY_AP,
				SCP_CMD_SCP_BOOT_TIMEOUT_US);
		if (ret) {
			ERROR("SCP Patch could not initialize; error %d\n",
				ret);
			return ret;
		}

		INFO("SCP Patch successfully initialized.\n");
	}

	scp_patch_version = scp_send_cmd(MCU_IPC_MCU_CMD_GET_FW_VERSION, 0,
				SCP_CMD_DEFAULT_TIMEOUT_US);
	INFO("SCP Patch version :0x%x\n", scp_patch_version);

	/* Next block just reports current AVS voltages (if applicable) */
	{
		uint16_t vcore_mv, ihost03_mv, ihost12_mv;

		vcore_mv = SCP_READ_CFG16(vcore.millivolts) +
				SCP_READ_CFG8(vcore.avs_cfg.additive_margin);
		ihost03_mv = SCP_READ_CFG16(ihost03.millivolts) +
				SCP_READ_CFG8(ihost03.avs_cfg.additive_margin);
		ihost12_mv = SCP_READ_CFG16(ihost12.millivolts) +
				SCP_READ_CFG8(ihost12.avs_cfg.additive_margin);

		if (vcore_mv || ihost03_mv || ihost12_mv) {
			INFO("AVS voltages from cfg (including margin)\n");
			if (vcore_mv > 0)
				INFO("%s\tVCORE: %dmv\n",
					SCP_READ_CFG8(vcore.avs_cfg.avs_set) ?
					"*" : "n/a", vcore_mv);
			if (ihost03_mv > 0)
				INFO("%s\tIHOST03: %dmv\n",
				SCP_READ_CFG8(ihost03.avs_cfg.avs_set) ?
					"*" : "n/a", ihost03_mv);
			if (ihost12_mv > 0)
				INFO("%s\tIHOST12: %dmv\n",
				SCP_READ_CFG8(ihost12.avs_cfg.avs_set) ?
					"*" : "n/a", ihost12_mv);
		} else {
			INFO("AVS settings not applicable\n");
		}
	}

#if (CLEAN_DDR && !defined(MMU_DISABLED) && !defined(EMULATION_SETUP))
	/* This will clean the DDR and enable ECC if set */
	check_ddr_clean();
#endif

#if (WARMBOOT_DDR_S3_SUPPORT && ELOG_STORE_MEDIA_DDR)
	elog_init_ddr_log();
#endif

#ifdef BCM_ELOG
	/* Prepare ELOG to use DDR */
	prepare_elog();
#endif

#ifndef EMULATION_SETUP
	/* Ask ddr_init to save obtained DDR information into DDR */
	ddr_info_save();
#endif

	/*
	 * Configure TMON DDR address.
	 * This cfg is common for all cases
	 */
	SCP_WRITE_CFG(tmon_cfg.ddr_desc, TMON_SHARED_DDR_ADDRESS);

	if (reset_state == SOFT_RESET_L3 && !mcu_ap_init_param) {
		INFO("SCP configuration after L3 RESET done.\n");
		return 0;
	}

	if (bcm_chimp_is_nic_mode())
		/* Configure AP WDT to not reset the NIC interface */
		SCP_WRITE_CFG(board_cfg.apwdt_reset_type, SOFT_RESET_L3);

#if (WARMBOOT_DDR_S3_SUPPORT && ELOG_STORE_MEDIA_DDR)
	/* When AP WDog triggers perform L3 reset if DDR err logging enabled */
	SCP_WRITE_CFG(board_cfg.apwdt_reset_type, SOFT_RESET_L3);
#endif

#ifndef EMULATION_SETUP

#ifdef DDR_SCRUB_ENA
	ddr_scrub_enable();
#endif
	/* Fill the Active channel information */
	active_ch_bitmap = get_active_ddr_channel();
	for (i = 0; i < MAX_NR_DDR_CH; i++)
		SCP_WRITE_CFG(ddr_cfg.ddr_cfg[i],
			      (active_ch_bitmap & BIT(i)) ? 1 : 0);
#endif
	return 0;
}

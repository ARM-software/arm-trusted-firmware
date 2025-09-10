/*
 * Copyright (c) 2022-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/scmi.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include "scmi_private.h"

#include <platform_def.h>
#include "rcar_def.h"
#include "rcar_private.h"

/*
 * This file implements the SCP helper functions using SCMI protocol.
 */
static void __section(".system_ram")

/*
 * The handles for invoking the SCMI driver APIs after the driver
 * has been initialized.
 */
static void *scmi_handle __section(".system_ram_data");

/* The SCMI channel info */
static scmi_channel_t sscmi_channel __section(".system_ram_data");

/*
 * Allow use of channel specific lock instead of using a single lock for
 * all the channels.
 */
static RCAR_SCMI_INSTANTIATE_LOCK __section(".system_ram_data");

/* The SCMI platform info holder in system ram */
static scmi_channel_plat_info_t scmi_plat_info __section(".system_ram_data");

static void __section(".system_ram")
rcar_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	u_register_t reg = plat_info->db_reg_addr;

	mmio_setbits_32(reg, plat_info->db_modify_mask);
}

/*
 * Helper function to shutdown the system via SCMI.
 */
void rcar_scmi_sys_shutdown(void)
{
	int ret = scmi_sys_pwr_state_set(scmi_handle, SCMI_SYS_PWR_GRACEFUL_REQ,
					 SCMI_SYS_PWR_SHUTDOWN);

	if (ret == SCMI_E_SUCCESS)
		return;

	ERROR("SCMI system power state set (SHUTDOWN) returns 0x%x\n",
	      ret);
	panic();
}

/*
 * Helper function to reset the system via SCMI.
 */
void rcar_scmi_sys_reboot(void)
{
	int ret = scmi_sys_pwr_state_set(scmi_handle, SCMI_SYS_PWR_GRACEFUL_REQ,
					 SCMI_SYS_PWR_COLD_RESET);
	if (ret == SCMI_E_SUCCESS)
		return;

	ERROR("SCMI system power state set (COLD RESET) returns 0x%x\n",
	      ret);
	panic();
}

/*
 * Helper function to suspend the system via SCMI.
 */
void __section(".system_ram") rcar_scmi_sys_suspend(void)
{
	int ret = scmi_sys_pwr_state_set(scmi_handle, SCMI_SYS_PWR_GRACEFUL_REQ,
					 SCMI_SYS_PWR_SUSPEND);

	if (ret == SCMI_E_SUCCESS)
		return;

	/* Do not out logs and go panic when DRAM backup mode */
	while (true)
		;
}

void __init plat_rcar_scmi_setup(void)
{
	mailbox_mem_t *mbx_mem;

	INFO("Initializing SCMI driver on Channel\n");

	/* Preset info data to sram variable */
	scmi_plat_info.scmi_mbx_mem = RCAR_SCMI_CHANNEL_BASE;
	scmi_plat_info.db_reg_addr = RCAR_SCMI_MFIS_ADDR;
	scmi_plat_info.db_preserve_mask = RCAR_SCMI_MFIS_PRV_MASK;
	scmi_plat_info.db_modify_mask = RCAR_SCMI_MFIS_MOD_MASK;
	scmi_plat_info.ring_doorbell = rcar_ring_doorbell;

	/* Clean-up and initialize channel memory */
	mbx_mem = (mailbox_mem_t *)(scmi_plat_info.scmi_mbx_mem);
	(void)memset(mbx_mem, 0x0, RCAR_SCMI_CHANNEL_SIZE);

	/* Initialize channel status to free at first */
	mbx_mem->status |= (uint32_t)SCMI_CH_STATUS_FREE_MASK;

	/* Setup and initialize channel */
	sscmi_channel.info = &scmi_plat_info;
	sscmi_channel.lock = RCAR_SCMI_LOCK_GET_INSTANCE;

	scmi_handle = scmi_init(&sscmi_channel);
	if (scmi_handle)
		return;

	ERROR("SCMI Initialization failed on channel\n");
	panic();
}

const plat_psci_ops_t *plat_rcar_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	/* Check if the SCMI SYSTEM_POWER_STATE_SET message is supported */
	uint32_t msg_attr;
	int ret = scmi_proto_msg_attr(scmi_handle, SCMI_SYS_PWR_PROTO_ID,
				      SCMI_SYS_PWR_STATE_SET_MSG, &msg_attr);

	if (ret == SCMI_E_SUCCESS) {
		msg_attr &= SCMI_SYS_PWR_SUSPEND_SUPPORTED;
		if (msg_attr != SCMI_SYS_PWR_SUSPEND_SUPPORTED) {
			/*
			 * System power management protocol is available,
			 * but it does not support SYSTEM SUSPEND.
			 */
			ops->get_sys_suspend_power_state = NULL;
		}
	} else {
		/* System power management operations are not supported */
		ops->system_off = NULL;
		ops->system_reset = NULL;
		ops->get_sys_suspend_power_state = NULL;
	}

	return ops;
}

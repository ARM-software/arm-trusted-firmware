/*
 * Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
 * k3low SoC specific bl31_setup
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>
#include <ti_sci_transport.h>

#include <board_def.h>
#include <plat_private.h>

/* Table of regions to map using the MMU */
const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(K3_USART_BASE,       K3_USART_SIZE,       MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GIC_BASE,         K3_GIC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GTC_BASE,         K3_GTC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(TI_MAILBOX_TX_BASE,  TI_MAILBOX_RX_TX_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(WKUP_CTRL_MMR0_BASE, WKUP_CTRL_MMR0_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(MAILBOX_SHMEM_REGION_BASE, MAILBOX_SHMEM_REGION_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

int ti_soc_init(void)
{
	struct ti_sci_msg_version version;
	int ret;

	generic_delay_timer_init();

	ret = ti_sci_boot_notification();
	if (ret != 0) {
		ERROR("%s: Failed to receive boot notification (%d)\n", __func__, ret);
		return ret;
	}

	ret = ti_sci_get_revision(&version);
	if (ret != 0) {
		ERROR("%s: Failed to get revision (%d)\n", __func__, ret);
		return ret;
	}

	NOTICE("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	       version.abi_major, version.abi_minor,
	       version.firmware_revision,
	       version.firmware_description);

	ret = ti_sci_proc_request(PLAT_PROC_START_ID);
	if (ret != 0) {
		ERROR("Unable to request host (%d)\n", ret);
		return ret;
	}

	/* Enable ACP interface */
	ret = ti_sci_proc_set_boot_ctrl(PLAT_PROC_START_ID, 0,
					PROC_BOOT_CTRL_FLAG_ARMV8_AINACTS);
	if (ret != 0) {
		ERROR("Unable to set boot control (%d)\n", ret);
		return ret;
	}

	return 0;
}

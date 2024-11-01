/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "mmup_common.h"
#include "vcp_helper.h"
#include "vcp_reg.h"

#define MODULE_TAG "[MMUP]"

bool mmup_smc_rstn_set(bool boot_ok)
{
	if (mmio_read_32(VCP_GPR_CORE1_REBOOT) != 0 &&
	    mmio_read_32(VCP_R_CORE1_STATUS) != 0 &&
	    (mmio_read_32(VCP_R_GIPC_IN_SET) & B_GIPC3_SETCLR_1) == 0 &&
	    (mmio_read_32(VCP_R_GIPC_IN_CLR) & B_GIPC3_SETCLR_1) == 0 &&
	    mmio_read_32(VCP_GPR_CORE1_REBOOT) != VCP_CORE_RDY_TO_REBOOT) {
		ERROR("%s: [%s] mmup reset set fail!GIPC 0x%x 0x%x REBOOT 0x%x\n",
		      MODULE_TAG, __func__, mmio_read_32(VCP_R_GIPC_IN_SET),
		      mmio_read_32(VCP_R_GIPC_IN_CLR),
		      mmio_read_32(VCP_GPR_CORE1_REBOOT));
		return false;
	}

	mmio_write_32(VCP_R_CORE1_SW_RSTN_SET, BIT(0));

	/* reset sec control */
	mmio_write_32(VCP_R_SEC_CTRL_2, 0);

	/* reset domain setting */
	mmio_write_32(VCP_R_S_DOM_EN0_31, 0x0);
	mmio_write_32(VCP_R_S_DOM_EN32_63, 0x0);
	mmio_write_32(VCP_R_NS_DOM_EN0_31, 0x0);
	mmio_write_32(VCP_R_NS_DOM_EN32_63, 0x0);

	/* reset sec setting */
	mmio_clrbits_32(VCP_R_DYN_SECURE,
			RESET_NS_SECURE_B_REGION << VCP_NS_SECURE_B_REGION_ENABLE);

	if (boot_ok)
		mmio_write_32(VCP_GPR_CORE1_REBOOT, VCP_CORE_REBOOT_OK);

	dsbsy();
	return true;
}

bool mmup_smc_rstn_clr(void)
{
	if ((mmio_read_32(VCP_R_CORE1_SW_RSTN_SET) & BIT(0)) == 1) {
		ERROR("%s: [%s] mmup not reset set !\n", MODULE_TAG, __func__);
		return false;
	}

	if ((get_mmup_fw_size() == 0) || get_mmup_l2tcm_offset() == 0) {
		ERROR("%s: [%s] mmup no enough l2tcm to run !\n", MODULE_TAG, __func__);
		return false;
	}

	mmio_write_32(VCP_R_SEC_DOMAIN_MMPC, VCP_DOMAIN_SET_MMPC);

	/* enable IOVA Mode */
	mmio_write_32(VCP_R_AXIOMMUEN_DEV_APC, BIT(0));

	/* reset secure setting */
	mmio_setbits_32(VCP_R_SEC_CTRL_2, CORE1_SEC_BIT_SEL);

	/* l2tcm offset*/
	mmio_setbits_32(VCP_R_SEC_CTRL, VCP_OFFSET_ENABLE_P | VCP_OFFSET_ENABLE_B);
	mmio_write_32(R_L2TCM_OFFSET_RANGE_0_LOW, 0x0);
	mmio_write_32(R_L2TCM_OFFSET_RANGE_0_HIGH, round_up(get_mmup_fw_size(), PAGE_SIZE));
	mmio_write_32(R_L2TCM_OFFSET, get_mmup_l2tcm_offset());

	/* start vcp-mmup */
	mmio_write_32(VCP_R_CORE1_SW_RSTN_CLR, BIT(0));
	dsbsy();
	return true;
}

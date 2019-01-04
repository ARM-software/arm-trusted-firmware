/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/marvell/ccu.h>
#include <drivers/marvell/mochi/cp110_setup.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <marvell_plat_priv.h> /* timer functionality */

#include "mss_scp_bootloader.h"

/* IO windows configuration */
#define IOW_GCR_OFFSET		(0x70)

/* MSS windows configuration */
#define MSS_AEBR(base)			(base + 0x160)
#define MSS_AIBR(base)			(base + 0x164)
#define MSS_AEBR_MASK			0xFFF
#define MSS_AIBR_MASK			0xFFF

#define MSS_EXTERNAL_SPACE		0x50000000
#define MSS_EXTERNAL_ACCESS_BIT		28
#define MSS_EXTERNAL_ADDR_MASK		0xfffffff
#define MSS_INTERNAL_ACCESS_BIT		28

struct addr_map_win ccu_mem_map[] = {
	{MVEBU_CP_REGS_BASE(0), 0x4000000, IO_0_TID}
};

/* Since the scp_bl2 image can contain firmware for cp1 and cp0 coprocessors,
 * the access to cp0 and cp1 need to be provided. More precisely it is
 * required to:
 *  - get the information about device id which is stored in CP0 registers
 *    (to distinguish between cases where we have cp0 and cp1 or standalone cp0)
 *  - get the access to cp which is needed for loading fw for cp0/cp1
 *    coprocessors
 * This function configures ccu windows accordingly.
 *
 * Note: there is no need to restore previous ccu configuration, since in next
 * phase (BL31) the init_ccu will be called (via apn806_init/
 * bl31_plat_arch_setu) and therefore the ccu configuration will be overwritten.
 */
static int bl2_plat_mmap_init(void)
{
	int cfg_num, win_id, cfg_idx;

	cfg_num =  ARRAY_SIZE(ccu_mem_map);

	/* CCU window-0 should not be counted - it's already used */
	if (cfg_num > (MVEBU_CCU_MAX_WINS - 1)) {
		ERROR("BL2: %s: trying to open too many windows\n", __func__);
		return -1;
	}

	/* Enable required CCU windows
	 * Do not touch CCU window 0,
	 * it's used for the internal registers access
	 */
	for (cfg_idx = 0, win_id = 1; cfg_idx < cfg_num; cfg_idx++, win_id++) {
		/* Enable required CCU windows */
		ccu_win_check(&ccu_mem_map[cfg_idx]);
		ccu_enable_win(MVEBU_AP0, &ccu_mem_map[cfg_idx], win_id);
	}

	/* Set the default target id to PIDI */
	mmio_write_32(MVEBU_IO_WIN_BASE(MVEBU_AP0) + IOW_GCR_OFFSET, PIDI_TID);

	return 0;
}

/*****************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 *****************************************************************************
 */
int bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int ret;

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");

	/* initialize time (for delay functionality) */
	plat_delay_timer_init();

	ret = bl2_plat_mmap_init();
	if (ret != 0)
		return ret;

	ret = scp_bootloader_transfer((void *)scp_bl2_image_info->image_base,
		scp_bl2_image_info->image_size);

	if (ret == 0)
		INFO("BL2: SCP_BL2 transferred to SCP\n");
	else
		ERROR("BL2: SCP_BL2 transfer failure\n");

	return ret;
}

uintptr_t bl2_plat_get_cp_mss_regs(int ap_idx, int cp_idx)
{
	return MVEBU_CP_REGS_BASE(cp_idx) + 0x280000;
}

uintptr_t bl2_plat_get_ap_mss_regs(int ap_idx)
{
	return MVEBU_REGS_BASE + 0x580000;
}

uint32_t bl2_plat_get_cp_count(int ap_idx)
{
	uint32_t revision = cp110_device_id_get(MVEBU_CP_REGS_BASE(0));
	/* A8040: two CPs.
	 * A7040: one CP.
	 */
	if (revision == MVEBU_80X0_DEV_ID ||
	    revision == MVEBU_80X0_CP115_DEV_ID)
		return 2;
	else
		return 1;
}

uint32_t bl2_plat_get_ap_count(void)
{
	/* A8040 and A7040 have only one AP */
	return 1;
}

void bl2_plat_configure_mss_windows(uintptr_t mss_regs)
{
	/* set AXI External and Internal Address Bus extension */
	mmio_write_32(MSS_AEBR(mss_regs),
		      ((0x0 >> MSS_EXTERNAL_ACCESS_BIT) & MSS_AEBR_MASK));
	mmio_write_32(MSS_AIBR(mss_regs),
		      ((mss_regs >> MSS_INTERNAL_ACCESS_BIT) & MSS_AIBR_MASK));
}

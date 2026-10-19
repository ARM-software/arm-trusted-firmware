/*
 * Copyright 2022-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/mmio.h>

#include <ele_api.h>
#include <platform_def.h>

#include "trdc_config.h"

#define BLK_CTRL_NS_ANOMIX_BASE	U(0x44210000)

static void trdc_fuse_init(void)
{
	uint32_t val, i;

	val = mmio_read_32(BLK_CTRL_NS_ANOMIX_BASE + 0x28U);
	for (i = 0U; i < fuse_data_num; i++) {
		if ((val & BIT(0)) != 0U) { /* OSCCA enabled */
			fuse_data[i].value = ele_read_common_fuse(fuse_data[i].fsb_index);
		} else {
			fuse_data[i].value = mmio_read_32(FSB_BASE + FSB_SHADOW_OFF
				+ (fuse_data[i].fsb_index << 2));
		}
	}
}

uint32_t trdc_fuse_read(uint8_t word_index)
{
	uint32_t i;

	for (i = 0U; i < fuse_data_num; i++) {
		if (fuse_data[i].fsb_index == word_index) {
			return fuse_data[i].value;
		}
	}

	return 0U;
}

/*
 * Apply the 4-phase TRDC configuration (MGR/MC setup, user config,
 * fused-slot protection, lockup) for blocks matching base. Pass
 * base == 0U to apply to every TRDC instance.
 */
static void trdc_reinit_base(uintptr_t base)
{
	unsigned int i;

	/* Config the access permission for the TRDC MGR and MC slot */
	for (i = 0U; i < trdc_mgr_num; i++) {
		if (base == 0U || trdc_mgr_blks[i].trdc_base == base) {
			trdc_mgr_mbc_setup(&trdc_mgr_blks[i]);
		}
	}

	/* Config the TRDC user setting from the config table */
	for (i = 0U; i < trdc_cfg_info_num; i++) {
		if (base == 0U || trdc_cfg_info[i].trdc_base == base) {
			trdc_setup(&trdc_cfg_info[i]);
		}
	}

	/* Configure the access permission for fused slots */
	for (i = 0U; i < fuse_info_num; i++) {
		if (base == 0U || fuse_info[i].trdc_base == base) {
			trdc_mgr_fused_slot_setup(&fuse_info[i]);
		}
	}

	/* Try to lock up TRDC MBC/MRC according to user settings */
	for (i = 0U; i < trdc_cfg_info_num; i++) {
		if (base == 0U || trdc_cfg_info[i].trdc_base == base) {
			trdc_try_lockup(&trdc_cfg_info[i]);
		}
	}
}

void trdc_common_init(void)
{
	trdc_fuse_init();
	trdc_reinit_base(0U);
}

/* Wakeup mix TRDC init */
void trdc_w_reinit(void)
{
	trdc_reinit_base(TRDC_W_BASE);
}

/* Nic mix TRDC init */
void trdc_n_reinit(void)
{
	trdc_reinit_base(TRDC_N_BASE);
}

void trdc_config(void)
{
	/* Set MTR to DID1 */
	trdc_mda_set_noncpu(TRDC_A_BASE, 4, false, 0x2, 0x2, 0x1, false);

#if defined(PLAT_imx93)
	/* Set M33 to DID2 */
	trdc_mda_set_cpu(TRDC_A_BASE, 1, 0, 0x2, 0x0, 0x2,
			 0x0, 0x0, 0x0, false);
#endif /* PLAT_imx93 */

	trdc_common_init();

	NOTICE("TRDC init done\n");
}

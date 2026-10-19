/*
 * Copyright 2022-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/mmio.h>

#include <platform_def.h>

#include "trdc_config.h"

void trdc_config(void)
{
	unsigned int i;

	/* Set MTR to DID1 */
	trdc_mda_set_noncpu(TRDC_A_BASE, 4, false, 0x2, 0x2, 0x1);

#if defined(PLAT_imx93)
	/* Set M33 to DID2 */
	trdc_mda_set_cpu(TRDC_A_BASE, 1, 0, 0x2, 0x0, 0x2, 0x0, 0x0, 0x0);
#endif /* PLAT_imx93 */

	/* Configure the access permission for TRDC MGR and MC slots */
	for (i = 0U; i < ARRAY_SIZE(trdc_mgr_blks); i++) {
		trdc_mgr_mbc_setup(&trdc_mgr_blks[i]);
	}

	/* Configure TRDC user settings from config table */
	for (i = 0U; i < ARRAY_SIZE(trdc_cfg_info); i++) {
		trdc_setup(&trdc_cfg_info[i]);
	}

	NOTICE("TRDC init done\n");
}

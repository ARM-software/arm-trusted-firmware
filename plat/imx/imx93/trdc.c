/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include "trdc_config.h"

struct trdc_mgr_info trdc_mgr_blks[] = {
	{ TRDC_A_BASE, 0, 0, 39, 40 },
	{ TRDC_W_BASE, 0, 0, 70, 71 },
	{ TRDC_M_BASE, 1, 0, 1, 2 },
	{ TRDC_N_BASE, 0, 1, 1, 2 },
};

unsigned int trdc_mgr_num = ARRAY_SIZE(trdc_mgr_blks);

struct trdc_config_info trdc_cfg_info[] = {
	{	TRDC_A_BASE,
		trdc_a_mbc_glbac, ARRAY_SIZE(trdc_a_mbc_glbac),
		trdc_a_mbc, ARRAY_SIZE(trdc_a_mbc),
		trdc_a_mrc_glbac, ARRAY_SIZE(trdc_a_mrc_glbac),
		trdc_a_mrc, ARRAY_SIZE(trdc_a_mrc)
	}, /* TRDC_A */
	{	TRDC_W_BASE,
		trdc_w_mbc_glbac, ARRAY_SIZE(trdc_w_mbc_glbac),
		trdc_w_mbc, ARRAY_SIZE(trdc_w_mbc),
		trdc_w_mrc_glbac, ARRAY_SIZE(trdc_w_mrc_glbac),
		trdc_w_mrc, ARRAY_SIZE(trdc_w_mrc)
	}, /* TRDC_W */
	{	TRDC_N_BASE,
		trdc_n_mbc_glbac, ARRAY_SIZE(trdc_n_mbc_glbac),
		trdc_n_mbc, ARRAY_SIZE(trdc_n_mbc),
		trdc_n_mrc_glbac, ARRAY_SIZE(trdc_n_mrc_glbac),
		trdc_n_mrc, ARRAY_SIZE(trdc_n_mrc)
	}, /* TRDC_N */
};

void trdc_config(void)
{
	unsigned int i;

	/* Set MTR to DID1 */
	trdc_mda_set_noncpu(TRDC_A_BASE, 4, false, 0x2, 0x2, 0x1);

	/* Set M33 to DID2*/
	trdc_mda_set_cpu(TRDC_A_BASE, 1, 0, 0x2, 0x0, 0x2, 0x0, 0x0, 0x0);

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

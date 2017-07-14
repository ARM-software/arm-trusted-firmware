/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dram.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <secure.h>
#include <soc.h>

__pmusramdata struct rk3399_sdram_params sdram_config;

void dram_init(void)
{
	uint32_t os_reg2_val, i;

	os_reg2_val = mmio_read_32(PMUGRF_BASE + PMUGRF_OSREG(2));
	sdram_config.dramtype = SYS_REG_DEC_DDRTYPE(os_reg2_val);
	sdram_config.num_channels = SYS_REG_DEC_NUM_CH(os_reg2_val);
	sdram_config.stride = (mmio_read_32(SGRF_BASE + SGRF_SOC_CON3_7(4)) >>
				10) & 0x1f;

	for (i = 0; i < 2; i++) {
		struct rk3399_sdram_channel *ch = &sdram_config.ch[i];
		struct rk3399_msch_timings *noc = &ch->noc_timings;

		if (!(SYS_REG_DEC_CHINFO(os_reg2_val, i)))
			continue;

		ch->rank = SYS_REG_DEC_RANK(os_reg2_val, i);
		ch->col = SYS_REG_DEC_COL(os_reg2_val, i);
		ch->bk = SYS_REG_DEC_BK(os_reg2_val, i);
		ch->bw = SYS_REG_DEC_BW(os_reg2_val, i);
		ch->dbw = SYS_REG_DEC_DBW(os_reg2_val, i);
		ch->row_3_4 = SYS_REG_DEC_ROW_3_4(os_reg2_val, i);
		ch->cs0_row = SYS_REG_DEC_CS0_ROW(os_reg2_val, i);
		ch->cs1_row = SYS_REG_DEC_CS1_ROW(os_reg2_val, i);
		ch->ddrconfig = mmio_read_32(MSCH_BASE(i) + MSCH_DEVICECONF);

		noc->ddrtiminga0.d32 = mmio_read_32(MSCH_BASE(i) +
				MSCH_DDRTIMINGA0);
		noc->ddrtimingb0.d32 = mmio_read_32(MSCH_BASE(i) +
				MSCH_DDRTIMINGB0);
		noc->ddrtimingc0.d32 = mmio_read_32(MSCH_BASE(i) +
				MSCH_DDRTIMINGC0);
		noc->devtodev0.d32 = mmio_read_32(MSCH_BASE(i) +
				MSCH_DEVTODEV0);
		noc->ddrmode.d32 = mmio_read_32(MSCH_BASE(i) + MSCH_DDRMODE);
		noc->agingx0 = mmio_read_32(MSCH_BASE(i) + MSCH_AGINGX0);
	}
}

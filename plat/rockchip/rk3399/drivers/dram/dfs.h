/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_ROCKCHIP_RK3399_DFS_H__
#define __SOC_ROCKCHIP_RK3399_DFS_H__

struct rk3399_sdram_default_config {
	unsigned char bl;
	/* 1:auto precharge, 0:never auto precharge */
	unsigned char ap;
	/* dram driver strength */
	unsigned char dramds;
	/* dram ODT, if odt=0, this parameter invalid */
	unsigned char dramodt;
	/* ca ODT, if odt=0, this parameter invalid
	 * only used by LPDDR4
	 */
	unsigned char caodt;
	unsigned char burst_ref_cnt;
	/* zqcs period, unit(s) */
	unsigned char zqcsi;
};

struct drv_odt_lp_config {
	uint32_t pd_idle;
	uint32_t sr_idle;
	uint32_t sr_mc_gate_idle;
	uint32_t srpd_lite_idle;
	uint32_t standby_idle;
	uint32_t odt_en;

	uint32_t dram_side_drv;
	uint32_t dram_side_dq_odt;
	uint32_t dram_side_ca_odt;
};

uint32_t ddr_set_rate(uint32_t hz);
uint32_t ddr_round_rate(uint32_t hz);
uint32_t ddr_get_rate(void);
uint32_t dram_set_odt_pd(uint32_t arg0, uint32_t arg1, uint32_t arg2);
void dram_dfs_init(void);
void ddr_prepare_for_sys_suspend(void);
void ddr_prepare_for_sys_resume(void);

#endif

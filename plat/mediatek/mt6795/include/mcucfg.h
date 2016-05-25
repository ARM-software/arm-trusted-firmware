/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MCUCFG_H__
#define __MCUCFG_H__

#include <platform_def.h>
#include <stdint.h>

struct mt6795_mcucfg_regs {
	uint32_t mp0_ca7l_cache_config;
	struct {
		uint32_t mem_delsel0;
		uint32_t mem_delsel1;
	} mp0_cpu[4];
	uint32_t mp0_cache_mem_delsel0;
	uint32_t mp0_cache_mem_delsel1;
	uint32_t mp0_axi_config;
	uint32_t mp0_misc_config[2];
	struct {
		uint32_t rv_addr_lw;
		uint32_t rv_addr_hw;
	} mp0_rv_addr[4];
	uint32_t mp0_ca7l_cfg_dis;
	uint32_t mp0_ca7l_clken_ctrl;
	uint32_t mp0_ca7l_rst_ctrl;
	uint32_t mp0_ca7l_misc_config;
	uint32_t mp0_ca7l_dbg_pwr_ctrl;
	uint32_t mp0_rw_rsvd0;
	uint32_t mp0_rw_rsvd1;
	uint32_t mp0_ro_rsvd;
	uint32_t reserved0_0[100];
	uint32_t mp1_cpucfg;
	uint32_t mp1_miscdbg;
	uint32_t reserved0_1[13];
	uint32_t mp1_rst_ctl;
	uint32_t mp1_clkenm_div;
	uint32_t reserved0_2[7];
	uint32_t mp1_config_res;
	uint32_t reserved0_3[13];
	struct {
		uint32_t rv_addr_lw;
		uint32_t rv_addr_hw;
	} mp1_rv_addr[2];
	uint32_t reserved0_4[84];
	uint32_t mp0_rst_status;		/* 0x400 */
	uint32_t mp0_dbg_ctrl;
	uint32_t mp0_dbg_flag;
	uint32_t mp0_ca7l_ir_mon;
	struct {
		uint32_t pc_lw;
		uint32_t pc_hw;
		uint32_t fp_arch32;
		uint32_t sp_arch32;
		uint32_t fp_arch64_lw;
		uint32_t fp_arch64_hw;
		uint32_t sp_arch64_lw;
		uint32_t sp_arch64_hw;
	} mp0_dbg_core[4];
	uint32_t dfd_ctrl;
	uint32_t dfd_cnt_l;
	uint32_t dfd_cnt_h;
	uint32_t misccfg_mp0_rw_rsvd;
	uint32_t misccfg_sec_vio_status0;
	uint32_t misccfg_sec_vio_status1;
	uint32_t reserved1[22];
	uint32_t misccfg_rw_rsvd;		/* 0x500 */
	uint32_t mcusys_dbg_mon_sel_a;
	uint32_t mcusys_dbg_mon;
	uint32_t reserved2[61];
	uint32_t mcusys_config_a;		/* 0x600 */
	uint32_t mcusys_config1_a;
	uint32_t mcusys_gic_peribase_a;
	uint32_t reserved3;
	uint32_t sec_range0_start;		/* 0x610 */
	uint32_t sec_range0_end;
	uint32_t sec_range_enable;
	uint32_t reserved4;
	uint32_t int_pol_ctl[8];		/* 0x620 */
	uint32_t aclken_div;			/* 0x640 */
	uint32_t pclken_div;
	uint32_t l2c_sram_ctrl;
	uint32_t armpll_jit_ctrl;
	uint32_t cci_addrmap;			/* 0x650 */
	uint32_t cci_config;
	uint32_t cci_periphbase;
	uint32_t cci_nevntcntovfl;
	uint32_t cci_clk_ctrl;			/* 0x660 */
	uint32_t cci_acel_s1_ctrl;
	uint32_t bus_fabric_dcm_ctrl;
	uint32_t reserved5;
	uint32_t xgpt_ctl;			/* 0x670 */
	uint32_t xgpt_idx;
	uint32_t ptpod2_ctl0;
	uint32_t ptpod2_ctl1;
	uint32_t mcusys_revid;
	uint32_t mcusys_rw_rsvd0;
	uint32_t mcusys_rw_rsvd1;
};

static struct mt6795_mcucfg_regs *const mt6795_mcucfg = (void *)MCUCFG_BASE;

/* cpu boot mode */
enum {
	MP0_CPUCFG_64BIT_SHIFT = 12,
	MP1_CPUCFG_64BIT_SHIFT = 28,
	MP0_CPUCFG_64BIT = 0xf << MP0_CPUCFG_64BIT_SHIFT,
	MP1_CPUCFG_64BIT = 0xf << MP1_CPUCFG_64BIT_SHIFT
};

/* scu related */
enum {
	MP0_ACINACTM_SHIFT = 4,
	MP1_ACINACTM_SHIFT = 0,
	MP0_ACINACTM = 1 << MP0_ACINACTM_SHIFT,
	MP1_ACINACTM = 1 << MP1_ACINACTM_SHIFT
};

enum {
	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT = 0,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT = 4,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT = 8,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT = 12,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT = 16,

	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT
};

enum {
	MP1_AINACTS_SHIFT = 4,
	MP1_AINACTS = 1 << MP1_AINACTS_SHIFT
};

enum {
	MP1_SW_CG_GEN_SHIFT = 12,
	MP1_SW_CG_GEN = 1 << MP1_SW_CG_GEN_SHIFT
};

enum {
	MP1_L2RSTDISABLE_SHIFT = 14,
	MP1_L2RSTDISABLE = 1 << MP1_L2RSTDISABLE_SHIFT
};

#endif  /* __MCUCFG_H__ */

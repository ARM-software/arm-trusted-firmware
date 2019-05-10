/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT8183_MCUCFG_H
#define MT8183_MCUCFG_H

#include <platform_def.h>
#include <stdint.h>

struct mt8183_mcucfg_regs {
	uint32_t mp0_ca7l_cache_config;		/* 0x0 */
	struct {
		uint32_t mem_delsel0;
		uint32_t mem_delsel1;
	} mp0_cpu[4];				/* 0x4 */
	uint32_t mp0_cache_mem_delsel0;		/* 0x24 */
	uint32_t mp0_cache_mem_delsel1;		/* 0x28 */
	uint32_t mp0_axi_config;		/* 0x2C */
	uint32_t mp0_misc_config[10];		/* 0x30 */
	uint32_t mp0_ca7l_cfg_dis;		/* 0x58 */
	uint32_t mp0_ca7l_clken_ctrl;		/* 0x5C */
	uint32_t mp0_ca7l_rst_ctrl;		/* 0x60 */
	uint32_t mp0_ca7l_misc_config;		/* 0x64 */
	uint32_t mp0_ca7l_dbg_pwr_ctrl;		/* 0x68 */
	uint32_t mp0_rw_rsvd0;			/* 0x6C */
	uint32_t mp0_rw_rsvd1;			/* 0x70 */
	uint32_t mp0_ro_rsvd;			/* 0x74 */
	uint32_t reserved0_0[98];		/* 0x78 */
	uint32_t mp1_ca7l_cache_config;		/* 0x200 */
	uint32_t mp1_miscdbg;			/* 0x204 */
	uint32_t reserved0_1[9];		/* 0x208 */
	uint32_t mp1_axi_config;		/* 0x22C */
	uint32_t mp1_misc_config[10];		/* 0x230 */
	uint32_t reserved0_2[3];		/* 0x258 */
	uint32_t mp1_ca7l_misc_config;		/* 0x264 */
	uint32_t reserved0_3[310];		/* 0x268 */
	uint32_t cci_adb400_dcm_config;		/* 0x740 */
	uint32_t sync_dcm_config;		/* 0x744 */
	uint32_t reserved0_4[16];		/* 0x748 */
	uint32_t mp0_cputop_spmc_ctl;		/* 0x788 */
	uint32_t mp1_cputop_spmc_ctl;		/* 0x78C */
	uint32_t mp1_cputop_spmc_sram_ctl;	/* 0x790 */
	uint32_t reserved0_5[23];		/* 0x794 */
	uint32_t l2_cfg_mp0;			/* 0x7F0 */
	uint32_t l2_cfg_mp1;			/* 0x7F4 */
	uint32_t reserved0_6[1282];		/* 0x7F8 */
	uint32_t cpusys0_sparkvretcntrl;	/* 0x1C00 */
	uint32_t cpusys0_sparken;		/* 0x1C04 */
	uint32_t cpusys0_amuxsel;		/* 0x1C08 */
	uint32_t reserved0_7[9];		/* 0x1C0C */
	uint32_t cpusys0_cpu0_spmc_ctl;		/* 0x1C30 */
	uint32_t cpusys0_cpu1_spmc_ctl;		/* 0x1C34 */
	uint32_t cpusys0_cpu2_spmc_ctl;		/* 0x1C38 */
	uint32_t cpusys0_cpu3_spmc_ctl;		/* 0x1C3C */
	uint32_t reserved0_8[370];		/* 0x1C40 */
	uint32_t mp2_cpucfg;			/* 0x2208 */
	uint32_t mp2_axi_config;		/* 0x220C */
	uint32_t reserved0_9[36];		/* 0x2210 */
	uint32_t mp2_cputop_spm_ctl;		/* 0x22A0 */
	uint32_t mp2_cputop_spm_sta;		/* 0x22A4 */
	uint32_t reserved0_10[98];		/* 0x22A8 */
	uint32_t cpusys2_cpu0_spmc_ctl;		/* 0x2430 */
	uint32_t cpusys2_cpu0_spmc_sta;		/* 0x2434 */
	uint32_t cpusys2_cpu1_spmc_ctl;		/* 0x2438 */
	uint32_t cpusys2_cpu1_spmc_sta;		/* 0x243C */
	uint32_t reserved0_11[176];		/* 0x2440 */
	uint32_t spark2ld0;			/* 0x2700 */
	uint32_t reserved0_12[1355];		/* 0x2704 */
	uint32_t cpusys1_cpu0_spmc_ctl;		/* 0x3C30 */
	uint32_t cpusys1_cpu1_spmc_ctl;		/* 0x3C34 */
	uint32_t cpusys1_cpu2_spmc_ctl;		/* 0x3C38 */
	uint32_t cpusys1_cpu3_spmc_ctl;		/* 0x3C3C */
};

static struct mt8183_mcucfg_regs *const mt8183_mcucfg = (void *)MCUCFG_BASE;

enum {
	SW_SPARK_EN = 1 << 0,
	SW_NO_WAIT_FOR_Q_CHANNEL = 1 << 1,
	SW_FSM_OVERRIDE = 1 << 2,
	SW_LOGIC_PRE1_PDB = 1 << 3,
	SW_LOGIC_PRE2_PDB = 1 << 4,
	SW_LOGIC_PDB = 1 << 5,
	SW_ISO = 1 << 6,
	SW_SRAM_SLEEPB = 0x3f << 7,
	SW_SRAM_ISOINTB = 1 << 13,
	SW_CLK_DIS = 1 << 14,
	SW_CKISO = 1 << 15,
	SW_PD = 0x3f << 16,
	SW_HOT_PLUG_RESET = 1 << 22,
	SW_PWR_ON_OVERRIDE_EN = 1 << 23,
	SW_PWR_ON = 1 << 24,
	SW_COQ_DIS = 1 << 25,
	LOGIC_PDBO_ALL_OFF_ACK = 1 << 26,
	LOGIC_PDBO_ALL_ON_ACK = 1 << 27,
	LOGIC_PRE2_PDBO_ALL_ON_ACK = 1 << 28,
	LOGIC_PRE1_PDBO_ALL_ON_ACK = 1 << 29
};

enum {
	CPU_SW_SPARK_EN = 1 << 0,
	CPU_SW_NO_WAIT_FOR_Q_CHANNEL = 1 << 1,
	CPU_SW_FSM_OVERRIDE = 1 << 2,
	CPU_SW_LOGIC_PRE1_PDB = 1 << 3,
	CPU_SW_LOGIC_PRE2_PDB = 1 << 4,
	CPU_SW_LOGIC_PDB = 1 << 5,
	CPU_SW_ISO = 1 << 6,
	CPU_SW_SRAM_SLEEPB = 1 << 7,
	CPU_SW_SRAM_ISOINTB = 1 << 8,
	CPU_SW_CLK_DIS = 1 << 9,
	CPU_SW_CKISO = 1 << 10,
	CPU_SW_PD = 0x1f << 11,
	CPU_SW_HOT_PLUG_RESET = 1 << 16,
	CPU_SW_POWR_ON_OVERRIDE_EN = 1 << 17,
	CPU_SW_PWR_ON = 1 << 18,
	CPU_SPARK2LDO_ALLSWOFF = 1 << 19,
	CPU_PDBO_ALL_ON_ACK = 1 << 20,
	CPU_PRE2_PDBO_ALLON_ACK = 1 << 21,
	CPU_PRE1_PDBO_ALLON_ACK = 1 << 22
};

enum {
	MP2_AXI_CONFIG_ACINACTM = 1 << 0,
	MPx_AXI_CONFIG_ACINACTM = 1 << 4,
	MPX_CA7_MISC_CONFIG_STANDBYWFIL2 = 1 << 28
};

enum {
	MP0_CPU0_STANDBYWFE = 1 << 20,
	MP0_CPU1_STANDBYWFE = 1 << 21,
	MP0_CPU2_STANDBYWFE = 1 << 22,
	MP0_CPU3_STANDBYWFE = 1 << 23
};

enum {
	MP1_CPU0_STANDBYWFE = 1 << 20,
	MP1_CPU1_STANDBYWFE = 1 << 21,
	MP1_CPU2_STANDBYWFE = 1 << 22,
	MP1_CPU3_STANDBYWFE = 1 << 23
};

enum {
	B_SW_HOT_PLUG_RESET = 1 << 30,
	B_SW_PD_OFFSET = 18,
	B_SW_PD = 0x3f << B_SW_PD_OFFSET,
	B_SW_SRAM_SLEEPB_OFFSET = 12,
	B_SW_SRAM_SLEEPB = 0x3f << B_SW_SRAM_SLEEPB_OFFSET
};

enum {
	B_SW_SRAM_ISOINTB = 1 << 9,
	B_SW_ISO = 1 << 8,
	B_SW_LOGIC_PDB = 1 << 7,
	B_SW_LOGIC_PRE2_PDB = 1 << 6,
	B_SW_LOGIC_PRE1_PDB = 1 << 5,
	B_SW_FSM_OVERRIDE = 1 << 4,
	B_SW_PWR_ON = 1 << 3,
	B_SW_PWR_ON_OVERRIDE_EN = 1 << 2
};

enum {
	B_FSM_STATE_OUT_OFFSET = 6,
	B_FSM_STATE_OUT_MASK = 0x1f << B_FSM_STATE_OUT_OFFSET,
	B_SW_LOGIC_PDBO_ALL_OFF_ACK = 1 << 5,
	B_SW_LOGIC_PDBO_ALL_ON_ACK = 1 << 4,
	B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = 1 << 3,
	B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = 1 << 2,
	B_FSM_OFF = 0 << B_FSM_STATE_OUT_OFFSET,
	B_FSM_ON = 1 << B_FSM_STATE_OUT_OFFSET,
	B_FSM_RET = 2 << B_FSM_STATE_OUT_OFFSET
};

/* APB Module infracfg_ao */
enum {
	INFRA_TOPAXI_PROTECTEN_1 = INFRACFG_AO_BASE + 0x250,
	INFRA_TOPAXI_PROTECTSTA1_1 = INFRACFG_AO_BASE + 0x258,
	INFRA_TOPAXI_PROTECTEN_1_SET = INFRACFG_AO_BASE + 0x2A8,
	INFRA_TOPAXI_PROTECTEN_1_CLR = INFRACFG_AO_BASE + 0x2AC
};

enum {
	IDX_PROTECT_MP0_CACTIVE = 10,
	IDX_PROTECT_MP1_CACTIVE = 11,
	IDX_PROTECT_ICC0_CACTIVE = 12,
	IDX_PROTECT_ICD0_CACTIVE = 13,
	IDX_PROTECT_ICC1_CACTIVE = 14,
	IDX_PROTECT_ICD1_CACTIVE = 15,
	IDX_PROTECT_L2C0_CACTIVE = 26,
	IDX_PROTECT_L2C1_CACTIVE = 27
};

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
	MP1_ACINACTM_SHIFT = 4,
	MP2_ACINACTM_SHIFT = 0,
	MP0_ACINACTM = 1 << MP0_ACINACTM_SHIFT,
	MP1_ACINACTM = 1 << MP1_ACINACTM_SHIFT,
	MP2_ACINACTM = 1 << MP2_ACINACTM_SHIFT
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

#endif  /* MT8183_MCUCFG_H */

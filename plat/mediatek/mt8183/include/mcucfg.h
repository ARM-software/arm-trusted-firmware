/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
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

#define sw_spark_en			(1<<0)
#define sw_no_wait_for_q_channel	(1<<1)
#define sw_fsm_override			(1<<2)
#define sw_logic_pre1_pdb		(1<<3)
#define sw_logic_pre2_pdb		(1<<4)
#define sw_logic_pdb			(1<<5)
#define sw_iso				(1<<6)
#define sw_sram_sleepb			(0x3F<<7)
#define sw_sram_isointb			(1<<13)
#define sw_clk_dis			(1<<14)
#define sw_ckiso			(1<<15)
#define sw_pd				(0x3F<<16)
#define sw_hot_plug_reset		(1<<22)
#define sw_pwr_on_override_en		(1<<23)
#define sw_pwr_on			(1<<24)
#define sw_coq_dis			(1<<25)
#define logic_pdbo_all_off_ack		(1<<26)
#define logic_pdbo_all_on_ack		(1<<27)
#define logic_pre2_pdbo_all_on_ack	(1<<28)
#define logic_pre1_pdbo_all_on_ack	(1<<29)

#define cpu_sw_spark_en			(1<<0)
#define cpu_sw_no_wait_for_q_channel	(1<<1)
#define cpu_sw_fsm_override		(1<<2)
#define cpu_sw_logic_pre1_pdb		(1<<3)
#define cpu_sw_logic_pre2_pdb		(1<<4)
#define cpu_sw_logic_pdb		(1<<5)
#define cpu_sw_iso			(1<<6)
#define cpu_sw_sram_sleepb		(1<<7)
#define cpu_sw_sram_isointb		(1<<8)
#define cpu_sw_clk_dis			(1<<9)
#define cpu_sw_ckiso			(1<<10)
#define cpu_sw_pd			(0x1F<<11)
#define cpu_sw_hot_plug_reset		(1<<16)
#define cpu_sw_powr_on_override_en	(1<<17)
#define cpu_sw_pwr_on			(1<<18)
#define cpu_spark2ldo_allswoff		(1<<19)
#define cpu_pdbo_all_on_ack		(1<<20)
#define cpu_pre2_pdbo_allon_ack		(1<<21)
#define cpu_pre1_pdbo_allon_ack		(1<<22)

#define MP2_AXI_CONFIG_acinactm		(1<<0)

#define MPx_AXI_CONFIG_acinactm		(1<<4)

#define MPx_CA7_MISC_CONFIG_standbywfil2	(1<<28)


#define MP0_CPU0_STANDBYWFE		(1<<20)
#define MP0_CPU1_STANDBYWFE		(1<<21)
#define MP0_CPU2_STANDBYWFE		(1<<22)
#define MP0_CPU3_STANDBYWFE		(1<<23)

#define MP1_CPU0_STANDBYWFE		(1<<20)
#define MP1_CPU1_STANDBYWFE		(1<<21)
#define MP1_CPU2_STANDBYWFE		(1<<22)
#define MP1_CPU3_STANDBYWFE		(1<<23)

#define B_SW_HOT_PLUG_RESET		(1<<30)
#define B_SW_PD_OFFSET			(18)
#define B_SW_PD				(0x3f<<B_SW_PD_OFFSET)

#define B_SW_SRAM_SLEEPB_OFFSET		(12)
#define B_SW_SRAM_SLEEPB		(0x3f<<B_SW_SRAM_SLEEPB_OFFSET)

#define B_SW_SRAM_ISOINTB		(1<<9)
#define B_SW_ISO			(1<<8)
#define B_SW_LOGIC_PDB			(1<<7)
#define B_SW_LOGIC_PRE2_PDB		(1<<6)
#define B_SW_LOGIC_PRE1_PDB		(1<<5)
#define B_SW_FSM_OVERRIDE		(1<<4)
#define B_SW_PWR_ON			(1<<3)
#define B_SW_PWR_ON_OVERRIDE_EN		(1<<2)



#define B_FSM_STATE_OUT_OFFSET		(6)
#define B_FSM_STATE_OUT_MASK		(0x1f << B_FSM_STATE_OUT_OFFSET)
#define B_SW_LOGIC_PDBO_ALL_OFF_ACK	(1<<5)
#define B_SW_LOGIC_PDBO_ALL_ON_ACK	(1<<4)
#define B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK	(1<<3)
#define B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK	(1<<2)


#define B_FSM_OFF				(0<<B_FSM_STATE_OUT_OFFSET)
#define B_FSM_ON				(1<<B_FSM_STATE_OUT_OFFSET)
#define B_FSM_RET				(2<<B_FSM_STATE_OUT_OFFSET)

/* APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN_1	(INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x258)
#define INFRA_TOPAXI_PROTECTEN_1_SET	(INFRACFG_AO_BASE + 0x2A8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR	(INFRACFG_AO_BASE + 0x2AC)

#define IDX_PROTECT_MP0_CACTIVE		10
#define IDX_PROTECT_MP1_CACTIVE		11
#define IDX_PROTECT_ICC0_CACTIVE	12
#define IDX_PROTECT_ICD0_CACTIVE	13
#define IDX_PROTECT_ICC1_CACTIVE	14
#define IDX_PROTECT_ICD1_CACTIVE	15
#define IDX_PROTECT_L2C0_CACTIVE	26
#define IDX_PROTECT_L2C1_CACTIVE	27

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

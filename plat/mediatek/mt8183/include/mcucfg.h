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
	uint32_t reserved0_0;			/* 0x78 */
	uint32_t mp0_l2_cache_parity1_rdata;	/* 0x7C */
	uint32_t mp0_l2_cache_parity2_rdata;	/* 0x80 */
	uint32_t reserved0_1;			/* 0x84 */
	uint32_t mp0_rgu_dcm_config;		/* 0x88 */
	uint32_t mp0_ca53_specific_ctrl;	/* 0x8C */
	uint32_t mp0_esr_case;			/* 0x90 */
	uint32_t mp0_esr_mask;			/* 0x94 */
	uint32_t mp0_esr_trig_en;		/* 0x98 */
	uint32_t reserved_0_2;			/* 0x9C */
	uint32_t mp0_ses_cg_en;			/* 0xA0 */
	uint32_t reserved0_3[216];		/* 0xA4 */
	uint32_t mp_dbg_ctrl;			/* 0x404 */
	uint32_t reserved0_4[34];		/* 0x408 */
	uint32_t mp_dfd_ctrl;			/* 0x490 */
	uint32_t dfd_cnt_l;			/* 0x494 */
	uint32_t dfd_cnt_h;			/* 0x498 */
	uint32_t misccfg_ro_rsvd;		/* 0x49C */
	uint32_t reserved0_5[24];		/* 0x4A0 */
	uint32_t mp1_rst_status;		/* 0x500 */
	uint32_t mp1_dbg_ctrl;			/* 0x504 */
	uint32_t mp1_dbg_flag;			/* 0x508 */
	uint32_t mp1_ca7l_ir_mon;		/* 0x50C */
	uint32_t reserved0_6[32];		/* 0x510 */
	uint32_t mcusys_dbg_mon_sel_a;		/* 0x590 */
	uint32_t mcucys_dbg_mon;		/* 0x594 */
	uint32_t misccfg_sec_voi_status0;	/* 0x598 */
	uint32_t misccfg_sec_vio_status1;	/* 0x59C */
	uint32_t reserved0_7[18];		/* 0x5A0 */
	uint32_t gic500_int_mask;		/* 0x5E8 */
	uint32_t core_rst_en_latch;		/* 0x5EC */
	uint32_t reserved0_8[3];		/* 0x5F0 */
	uint32_t dbg_core_ret;			/* 0x5FC */
	uint32_t mcusys_config_a;		/* 0x600 */
	uint32_t mcusys_config1_a;		/* 0x604 */
	uint32_t mcusys_gic_prebase_a;		/* 0x608 */
	uint32_t mcusys_pinmux;			/* 0x60C */
	uint32_t sec_range0_start;		/* 0x610 */
	uint32_t sec_range0_end;		/* 0x614 */
	uint32_t sec_range_enable;		/* 0x618 */
	uint32_t l2c_mm_base;			/* 0x61C */
	uint32_t reserved0_9[8];		/* 0x620 */
	uint32_t aclken_div;			/* 0x640 */
	uint32_t pclken_div;			/* 0x644 */
	uint32_t l2c_sram_ctrl;			/* 0x648 */
	uint32_t armpll_jit_ctrl;		/* 0x64C */
	uint32_t cci_addrmap;			/* 0x650 */
	uint32_t cci_config;			/* 0x654 */
	uint32_t cci_periphbase;		/* 0x658 */
	uint32_t cci_nevntcntovfl;		/* 0x65C */
	uint32_t cci_clk_ctrl;			/* 0x660 */
	uint32_t cci_acel_s1_ctrl;		/* 0x664 */
	uint32_t mcusys_bus_fabric_dcm_ctrl;	/* 0x668 */
	uint32_t mcu_misc_dcm_ctrl;		/* 0x66C */
	uint32_t xgpt_ctl;			/* 0x670 */
	uint32_t xgpt_idx;			/* 0x674 */
	uint32_t reserved0_10[3];		/* 0x678 */
	uint32_t mcusys_rw_rsvd0;		/* 0x684 */
	uint32_t mcusys_rw_rsvd1;		/* 0x688 */
	uint32_t reserved0_11[13];		/* 0x68C */
	uint32_t gic_500_delsel_ctl;		/* 0x6C0 */
	uint32_t etb_delsel_ctl;		/* 0x6C4 */
	uint32_t etb_rst_ctl;			/* 0x6C8 */
	uint32_t reserved0_12[29];		/* 0x6CC */
	uint32_t cci_adb400_dcm_config;		/* 0x740 */
	uint32_t sync_dcm_config;		/* 0x744 */
	uint32_t reserved0_13;			/* 0x748 */
	uint32_t sync_dcm_cluster_config;	/* 0x74C */
	uint32_t sw_udi;			/* 0x750 */
	uint32_t reserved0_14;			/* 0x754 */
	uint32_t gic_sync_dcm;			/* 0x758 */
	uint32_t big_dbg_pwr_ctrl;		/* 0x75C */
	uint32_t gic_cpu_periphbase;		/* 0x760 */
	uint32_t axi_cpu_config;		/* 0x764 */
	uint32_t reserved0_15[2];		/* 0x768 */
	uint32_t mcsib_sys_ctrl1;		/* 0x770 */
	uint32_t mcsib_sys_ctrl2;		/* 0x774 */
	uint32_t mcsib_sys_ctrl3;		/* 0x778 */
	uint32_t mcsib_sys_ctrl4;		/* 0x77C */
	uint32_t mcsib_dbg_ctrl1;		/* 0x780 */
	uint32_t pwrmcu_apb2to1;		/* 0x784 */
	uint32_t mp0_spmc;			/* 0x788 */
	uint32_t reserved0_16;			/* 0x78C */
	uint32_t mp0_spmc_sram_ctl;		/* 0x790 */
	uint32_t reserved0_17;			/* 0x794 */
	uint32_t mp0_sw_rst_wait_cycle;		/* 0x798 */
	uint32_t reserved0_18;			/* 0x79C */
	uint32_t mp0_pll_divider_cfg;		/* 0x7A0 */
	uint32_t reserved0_19;			/* 0x7A4 */
	uint32_t mp2_pll_divider_cfg;		/* 0x7A8 */
	uint32_t reserved0_20[5];		/* 0x7AC */
	uint32_t bus_pll_divider_cfg;		/* 0x7C0 */
	uint32_t reserved0_21[7];		/* 0x7C4 */
	uint32_t clusterid_aff1;		/* 0x7E0 */
	uint32_t clusterid_aff2;		/* 0x7E4 */
	uint32_t reserved0_22[2];		/* 0x7E8 */
	uint32_t l2_cfg_mp0;			/* 0x7F0 */
	uint32_t l2_cfg_mp1;			/* 0x7F4 */
	uint32_t reserved0_23[218];		/* 0x7F8 */
	uint32_t mscib_dcm_en;			/* 0xB60 */
	uint32_t reserved0_24[1063];		/* 0xB64 */
	uint32_t cpusys0_sparkvretcntrl;	/* 0x1C00 */
	uint32_t cpusys0_sparken;		/* 0x1C04 */
	uint32_t cpusys0_amuxsel;		/* 0x1C08 */
	uint32_t reserved0_25[9];		/* 0x1C0C */
	uint32_t cpusys0_cpu0_spmc_ctl;		/* 0x1C30 */
	uint32_t cpusys0_cpu1_spmc_ctl;		/* 0x1C34 */
	uint32_t cpusys0_cpu2_spmc_ctl;		/* 0x1C38 */
	uint32_t cpusys0_cpu3_spmc_ctl;		/* 0x1C3C */
	uint32_t reserved0_26[8];		/* 0x1C40 */
	uint32_t mp0_sync_dcm_cgavg_ctrl;	/* 0x1C60 */
	uint32_t mp0_sync_dcm_cgavg_fact;	/* 0x1C64 */
	uint32_t mp0_sync_dcm_cgavg_rfact;	/* 0x1C68 */
	uint32_t mp0_sync_dcm_cgavg;		/* 0x1C6C */
	uint32_t mp0_l2_parity_clr;		/* 0x1C70 */
	uint32_t reserved0_27[357];		/* 0x1C74 */
	uint32_t mp2_cpucfg;			/* 0x2208 */
	uint32_t mp2_axi_config;		/* 0x220C */
	uint32_t reserved0_28[25];		/* 0x2210 */
	uint32_t mp2_sync_dcm;			/* 0x2274 */
	uint32_t reserved0_29[10];		/* 0x2278 */
	uint32_t ptp3_cputop_spmc0;		/* 0x22A0 */
	uint32_t ptp3_cputop_spmc1;		/* 0x22A4 */
	uint32_t reserved0_30[98];		/* 0x22A8 */
	uint32_t ptp3_cpu0_spmc0;		/* 0x2430 */
	uint32_t ptp3_cpu0_spmc1;		/* 0x2434 */
	uint32_t ptp3_cpu1_spmc0;		/* 0x2438 */
	uint32_t ptp3_cpu1_spmc1;		/* 0x243C */
	uint32_t ptp3_cpu2_spmc0;		/* 0x2440 */
	uint32_t ptp3_cpu2_spmc1;		/* 0x2444 */
	uint32_t ptp3_cpu3_spmc0;		/* 0x2448 */
	uint32_t ptp3_cpu3_spmc1;		/* 0x244C */
	uint32_t ptp3_cpux_spmc;		/* 0x2450 */
	uint32_t reserved0_31[171];		/* 0x2454 */
	uint32_t spark2ld0;			/* 0x2700 */
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
	MP1_CPUCFG_64BIT = 0xfu << MP1_CPUCFG_64BIT_SHIFT
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

/* bus pll divider dcm related */
enum {
	BUS_PLLDIVIDER_DCM_DBC_CNT_0_SHIFT = 11,
	BUS_PLLDIV_ARMWFI_DCM_EN_SHIFT = 24,
	BUS_PLLDIV_ARMWFE_DCM_EN_SHIFT = 25,

	BUS_PLLDIV_DCM = (1 << BUS_PLLDIVIDER_DCM_DBC_CNT_0_SHIFT) |
			 (1 << BUS_PLLDIV_ARMWFI_DCM_EN_SHIFT) |
			 (1 << BUS_PLLDIV_ARMWFE_DCM_EN_SHIFT)
};

/* mp0 pll divider dcm related */
enum {
	MP0_PLLDIV_DCM_DBC_CNT_0_SHIFT = 11,
	MP0_PLLDIV_ARMWFI_DCM_EN_SHIFT = 24,
	MP0_PLLDIV_ARMWFE_DCM_EN_SHIFT = 25,
	MP0_PLLDIV_LASTCORE_IDLE_EN_SHIFT = 31,
	MP0_PLLDIV_DCM = (1 << MP0_PLLDIV_DCM_DBC_CNT_0_SHIFT) |
			 (1 << MP0_PLLDIV_ARMWFI_DCM_EN_SHIFT) |
			 (1 << MP0_PLLDIV_ARMWFE_DCM_EN_SHIFT) |
			 (1u << MP0_PLLDIV_LASTCORE_IDLE_EN_SHIFT)
};

/* mp2 pll divider dcm related */
enum {
	MP2_PLLDIV_DCM_DBC_CNT_0_SHIFT = 11,
	MP2_PLLDIV_ARMWFI_DCM_EN_SHIFT = 24,
	MP2_PLLDIV_ARMWFE_DCM_EN_SHIFT = 25,
	MP2_PLLDIV_LASTCORE_IDLE_EN_SHIFT = 31,
	MP2_PLLDIV_DCM = (1 << MP2_PLLDIV_DCM_DBC_CNT_0_SHIFT) |
			 (1 << MP2_PLLDIV_ARMWFI_DCM_EN_SHIFT) |
			 (1 << MP2_PLLDIV_ARMWFE_DCM_EN_SHIFT) |
			 (1u << MP2_PLLDIV_LASTCORE_IDLE_EN_SHIFT)
};

/* mcsib dcm related */
enum {
	MCSIB_CACTIVE_SEL_SHIFT = 0,
	MCSIB_DCM_EN_SHIFT = 16,

	MCSIB_CACTIVE_SEL_MASK = 0xffff << MCSIB_CACTIVE_SEL_SHIFT,
	MCSIB_CACTIVE_SEL = 0xffff << MCSIB_CACTIVE_SEL_SHIFT,

	MCSIB_DCM_MASK = 0xffffu << MCSIB_DCM_EN_SHIFT,
	MCSIB_DCM = 0xffffu << MCSIB_DCM_EN_SHIFT,
};

/* cci adb400 dcm related */
enum {
	CCI_M0_ADB400_DCM_EN_SHIFT = 0,
	CCI_M1_ADB400_DCM_EN_SHIFT = 1,
	CCI_M2_ADB400_DCM_EN_SHIFT = 2,
	CCI_S2_ADB400_DCM_EN_SHIFT = 3,
	CCI_S3_ADB400_DCM_EN_SHIFT = 4,
	CCI_S4_ADB400_DCM_EN_SHIFT = 5,
	CCI_S5_ADB400_DCM_EN_SHIFT = 6,
	ACP_S3_ADB400_DCM_EN_SHIFT = 11,

	CCI_ADB400_DCM_MASK = (1 << CCI_M0_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_M1_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_M2_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_S2_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_S4_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_S4_ADB400_DCM_EN_SHIFT) |
			      (1 << CCI_S5_ADB400_DCM_EN_SHIFT) |
			      (1 << ACP_S3_ADB400_DCM_EN_SHIFT),
	CCI_ADB400_DCM = (1 << CCI_M0_ADB400_DCM_EN_SHIFT) |
			 (1 << CCI_M1_ADB400_DCM_EN_SHIFT) |
			 (1 << CCI_M2_ADB400_DCM_EN_SHIFT) |
			 (0 << CCI_S2_ADB400_DCM_EN_SHIFT) |
			 (0 << CCI_S4_ADB400_DCM_EN_SHIFT) |
			 (0 << CCI_S4_ADB400_DCM_EN_SHIFT) |
			 (0 << CCI_S5_ADB400_DCM_EN_SHIFT) |
			 (1 << ACP_S3_ADB400_DCM_EN_SHIFT)
};

/* sync dcm related */
enum {
	CCI_SYNC_DCM_DIV_EN_SHIFT = 0,
	CCI_SYNC_DCM_UPDATE_TOG_SHIFT = 1,
	CCI_SYNC_DCM_DIV_SEL_SHIFT = 2,
	MP0_SYNC_DCM_DIV_EN_SHIFT = 10,
	MP0_SYNC_DCM_UPDATE_TOG_SHIFT = 11,
	MP0_SYNC_DCM_DIV_SEL_SHIFT = 12,

	SYNC_DCM_MASK = (1 << CCI_SYNC_DCM_DIV_EN_SHIFT) |
			(1 << CCI_SYNC_DCM_UPDATE_TOG_SHIFT) |
			(0x7f << CCI_SYNC_DCM_DIV_SEL_SHIFT) |
			(1 << MP0_SYNC_DCM_DIV_EN_SHIFT) |
			(1 << MP0_SYNC_DCM_UPDATE_TOG_SHIFT) |
			(0x7f << MP0_SYNC_DCM_DIV_SEL_SHIFT),
	SYNC_DCM = (1 << CCI_SYNC_DCM_DIV_EN_SHIFT) |
		   (1 << CCI_SYNC_DCM_UPDATE_TOG_SHIFT) |
		   (0 << CCI_SYNC_DCM_DIV_SEL_SHIFT) |
		   (1 << MP0_SYNC_DCM_DIV_EN_SHIFT) |
		   (1 << MP0_SYNC_DCM_UPDATE_TOG_SHIFT) |
		   (0 << MP0_SYNC_DCM_DIV_SEL_SHIFT)
};

/* mcu bus dcm related */
enum {
	MCU_BUS_DCM_EN_SHIFT = 8,
	MCU_BUS_DCM = 1 << MCU_BUS_DCM_EN_SHIFT
};

/* mcusys bus fabric dcm related */
enum {
	ACLK_INFRA_DYNAMIC_CG_EN_SHIFT = 0,
	EMI2_ADB400_S_DCM_CTRL_SHIFT = 1,
	ACLK_GPU_DYNAMIC_CG_EN_SHIFT = 2,
	ACLK_PSYS_DYNAMIC_CG_EN_SHIFT = 3,
	MP0_ADB400_S_DCM_CTRL_SHIFT = 4,
	MP0_ADB400_M_DCM_CTRL_SHIFT = 5,
	MP1_ADB400_S_DCM_CTRL_SHIFT = 6,
	MP1_ADB400_M_DCM_CTRL_SHIFT = 7,
	EMICLK_EMI_DYNAMIC_CG_EN_SHIFT = 8,
	INFRACLK_INFRA_DYNAMIC_CG_EN_SHIFT = 9,
	EMICLK_GPU_DYNAMIC_CG_EN_SHIFT = 10,
	INFRACLK_PSYS_DYNAMIC_CG_EN_SHIFT = 11,
	EMICLK_EMI1_DYNAMIC_CG_EN_SHIFT = 12,
	EMI1_ADB400_S_DCM_CTRL_SHIFT = 16,
	MP2_ADB400_M_DCM_CTRL_SHIFT = 17,
	MP0_ICC_AXI_STREAM_ARCH_CG_SHIFT = 18,
	MP1_ICC_AXI_STREAM_ARCH_CG_SHIFT = 19,
	MP2_ICC_AXI_STREAM_ARCH_CG_SHIFT = 20,
	L2_SHARE_ADB400_DCM_CTRL_SHIFT = 21,
	MP1_AGGRESS_DCM_CTRL_SHIFT = 22,
	MP0_AGGRESS_DCM_CTRL_SHIFT = 23,
	MP0_ADB400_ACP_S_DCM_CTRL_SHIFT = 24,
	MP0_ADB400_ACP_M_DCM_CTRL_SHIFT = 25,
	MP1_ADB400_ACP_S_DCM_CTRL_SHIFT = 26,
	MP1_ADB400_ACP_M_DCM_CTRL_SHIFT = 27,
	MP3_ADB400_M_DCM_CTRL_SHIFT = 28,
	MP3_ICC_AXI_STREAM_ARCH_CG_SHIFT = 29,

	MCUSYS_BUS_FABRIC_DCM_MASK = (1 << ACLK_INFRA_DYNAMIC_CG_EN_SHIFT) |
				     (1 << EMI2_ADB400_S_DCM_CTRL_SHIFT) |
				     (1 << ACLK_GPU_DYNAMIC_CG_EN_SHIFT) |
				     (1 << ACLK_PSYS_DYNAMIC_CG_EN_SHIFT) |
				     (1 << MP0_ADB400_S_DCM_CTRL_SHIFT) |
				     (1 << MP0_ADB400_M_DCM_CTRL_SHIFT) |
				     (1 << MP1_ADB400_S_DCM_CTRL_SHIFT) |
				     (1 << MP1_ADB400_M_DCM_CTRL_SHIFT) |
				     (1 << EMICLK_EMI_DYNAMIC_CG_EN_SHIFT) |
				     (1 << INFRACLK_INFRA_DYNAMIC_CG_EN_SHIFT) |
				     (1 << EMICLK_GPU_DYNAMIC_CG_EN_SHIFT) |
				     (1 << INFRACLK_PSYS_DYNAMIC_CG_EN_SHIFT) |
				     (1 << EMICLK_EMI1_DYNAMIC_CG_EN_SHIFT) |
				     (1 << EMI1_ADB400_S_DCM_CTRL_SHIFT) |
				     (1 << MP2_ADB400_M_DCM_CTRL_SHIFT) |
				     (1 << MP0_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				     (1 << MP1_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				     (1 << MP2_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				     (1 << L2_SHARE_ADB400_DCM_CTRL_SHIFT) |
				     (1 << MP1_AGGRESS_DCM_CTRL_SHIFT) |
				     (1 << MP0_AGGRESS_DCM_CTRL_SHIFT) |
				     (1 << MP0_ADB400_ACP_S_DCM_CTRL_SHIFT) |
				     (1 << MP0_ADB400_ACP_M_DCM_CTRL_SHIFT) |
				     (1 << MP1_ADB400_ACP_S_DCM_CTRL_SHIFT) |
				     (1 << MP1_ADB400_ACP_M_DCM_CTRL_SHIFT) |
				     (1 << MP3_ADB400_M_DCM_CTRL_SHIFT) |
				     (1 << MP3_ICC_AXI_STREAM_ARCH_CG_SHIFT),

	MCUSYS_BUS_FABRIC_DCM = (1 << ACLK_INFRA_DYNAMIC_CG_EN_SHIFT) |
				(1 << EMI2_ADB400_S_DCM_CTRL_SHIFT) |
				(1 << ACLK_GPU_DYNAMIC_CG_EN_SHIFT) |
				(1 << ACLK_PSYS_DYNAMIC_CG_EN_SHIFT) |
				(0 << MP0_ADB400_S_DCM_CTRL_SHIFT) |
				(0 << MP0_ADB400_M_DCM_CTRL_SHIFT) |
				(1 << MP1_ADB400_S_DCM_CTRL_SHIFT) |
				(1 << MP1_ADB400_M_DCM_CTRL_SHIFT) |
				(1 << EMICLK_EMI_DYNAMIC_CG_EN_SHIFT) |
				(1 << INFRACLK_INFRA_DYNAMIC_CG_EN_SHIFT) |
				(1 << EMICLK_GPU_DYNAMIC_CG_EN_SHIFT) |
				(1 << INFRACLK_PSYS_DYNAMIC_CG_EN_SHIFT) |
				(1 << EMICLK_EMI1_DYNAMIC_CG_EN_SHIFT) |
				(1 << EMI1_ADB400_S_DCM_CTRL_SHIFT) |
				(0 << MP2_ADB400_M_DCM_CTRL_SHIFT) |
				(1 << MP0_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				(1 << MP1_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				(1 << MP2_ICC_AXI_STREAM_ARCH_CG_SHIFT) |
				(1 << L2_SHARE_ADB400_DCM_CTRL_SHIFT) |
				(1 << MP1_AGGRESS_DCM_CTRL_SHIFT) |
				(1 << MP0_AGGRESS_DCM_CTRL_SHIFT) |
				(1 << MP0_ADB400_ACP_S_DCM_CTRL_SHIFT) |
				(1 << MP0_ADB400_ACP_M_DCM_CTRL_SHIFT) |
				(1 << MP1_ADB400_ACP_S_DCM_CTRL_SHIFT) |
				(1 << MP1_ADB400_ACP_M_DCM_CTRL_SHIFT) |
				(1 << MP3_ADB400_M_DCM_CTRL_SHIFT) |
				(1 << MP3_ICC_AXI_STREAM_ARCH_CG_SHIFT)
};

/* l2c_sram dcm related */
enum {
	L2C_SRAM_DCM_EN_SHIFT = 0,
	L2C_SRAM_DCM = 1 << L2C_SRAM_DCM_EN_SHIFT
};

/* mcu misc dcm related */
enum {
	MP0_CNTVALUEB_DCM_EN_SHIFT = 0,
	MP_CNTVALUEB_DCM_EN = 8,

	CNTVALUEB_DCM = (1 << MP0_CNTVALUEB_DCM_EN_SHIFT) |
			(1 << MP_CNTVALUEB_DCM_EN)
};

/* sync dcm cluster config related */
enum {
	MP0_SYNC_DCM_STALL_WR_EN_SHIFT = 7,
	MCUSYS_MAX_ACCESS_LATENCY_SHIFT = 24,

	MCU0_SYNC_DCM_STALL_WR_EN = 1 << MP0_SYNC_DCM_STALL_WR_EN_SHIFT,

	MCUSYS_MAX_ACCESS_LATENCY_MASK = 0xf << MCUSYS_MAX_ACCESS_LATENCY_SHIFT,
	MCUSYS_MAX_ACCESS_LATENCY = 0x5 << MCUSYS_MAX_ACCESS_LATENCY_SHIFT
};

/* cpusys rgu dcm related */
enum {
	CPUSYS_RGU_DCM_CONFIG_SHIFT = 0,

	CPUSYS_RGU_DCM_CINFIG = 1 << CPUSYS_RGU_DCM_CONFIG_SHIFT
};

/* mp2 sync dcm related */
enum {
	MP2_DCM_EN_SHIFT = 0,

	MP2_DCM_EN = 1 << MP2_DCM_EN_SHIFT
};
#endif  /* MT8183_MCUCFG_H */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef __PLAT_ROCKCHIP_DMC_RV1126B_H__
#define __PLAT_ROCKCHIP_DMC_RV1126B_H__

#include <plat_private.h>

enum {
	DLL_ON_2_ON,
	DLL_ON_2_OFF,
	DLL_OFF_2_ON,
	DLL_OFF_2_OFF,
};

/* DDR PHY Register */
#define PHY_BYTE_REG_OFS(n)		((n) * 0x180 + 0x400)
/* PHY_REG0x0 */
#define PHY_MAX_BYTE			(4)
#define PHY_DQ_WIDTH_MASK		(0xf)
#define PHY_DQ_WIDTH_SHIFT		(8)
/* PHY_REG0x44 */
#define PHY_FREQ_CHOOSE_OP_T_SHIFT	30
/* PHY_REG0x84 */
#define PHY_TRAIN_REG_UPDATE_EN		BIT(18)
/* PHY_REG0xd0 */
#define PHY_POSTDIV_EN_SHIFT(fsp)	(3 + 8 * (fsp))
#define PHY_POSTDIV_EN_MASK		(1)
#define PHY_POSTDIV_SHIFT(fsp)		(4 + 8 * (fsp))
#define PHY_POSTDIV_MASK		(0x7)

/* TOPCRU*/
#define TOPCRU_GATE_CON06		0x818
/* TOPCRU_GATE_CON06 */
#define DCLK_VOP_SRC_EN			BIT(12)

/* DDRGRF Register */
/* DDR_GRF_CON0 */
#define GRF_CON_DFI_INIT_START(n)	BITS_WITH_WMASK(n, 0x1, 1)
#define DFI_INIT_START_SEL_DDRCTL	BIT_CLR_WMSK(0)
#define DFI_INIT_START_SEL_GRF		BIT_SET_WMSK(0)
/* DDR_GRF_CON21 */
#define CSYSREQ_DDRC_PMU		(((0x1 << 12) << 16) | (0x1 << 12))
#define CSYSREQ_DDRC_CTL		(((0x1 << 12) << 16) | (0x0 << 12))

/* DDR_CRU */
#define DDR_CRU_DDR_SOFTRST_CON00	0xa00
/* DDR_CRU_DDR_SOFTRST_CON00 */
#define PRESETN_DDRPHY(n)		((0x1 << (8 + 16)) | ((n) << 8))
#define PRESETN_DDRC(n)			((0x1 << (2 + 16)) | ((n) << 2))
#define PRESETN_DDR_BIU(n)		((0x1 << (1 + 16)) | ((n) << 1))

/* SUBDDR_CRU */
#define SUBDDR_CRU_DPLL_CON(n)		((n) * 4)
#define SUBDDR_CLKSEL_CON00		0x300
#define SUBDDR_GATE_CON00		0x800
#define SUBDDR_SOFTRST_CON00		0xa00
/* SUBDDR_CRU_DPLL_CON0 0x0000 */
#define PB(n)				((0x1 << (15 + 16)) | ((n) << 15))
#define POSTDIV1_SHIFT			12
#define POSTDIV1_MASK			0x7
#define POSTDIV1(n)			((0x7 << (12 + 16)) | ((n) << 12))
#define FBDIV_SHIFT			0
#define FBDIV_MASK			0xfff
#define FBDIV(n)			((0xfff << 16) | (n))
/* SUBDDR_CRU_DPLL_CON1 0x0004 */
#define PLLPDSEL(n)			((0x1 << (15 + 16)) | ((n) << 15))
#define PLLPD1(n)			((0x1 << (14 + 16)) | ((n) << 14))
#define PLLPD0(n)			((0x1 << (13 + 16)) | ((n) << 13))
#define DSMPD(n)			((0x1 << (12 + 16)) | ((n) << 12))
#define PLL_LOCK(n)			(((n) >> 10) & 0x1)
#define POSTDIV2_SHIFT			6
#define POSTDIV2_MASK			0x7
#define POSTDIV2(n)			((0x7 << (6 + 16)) | ((n) << 6))
#define REFDIV_SHIFT			0
#define REFDIV_MASK			0x3f
#define REFDIV(n)			((0x3F << 16) | (n))
/* SUBDDR_CRU_DPLL_CON3 0x000c */
#define SSMOD_SPREAD(n)			((0x1f << (8 + 16)) | ((n) << 8))
#define SSMOD_DIVVAL(n)			((0xf << (4 + 16)) | ((n) << 4))
#define SSMOD_DOWNSPREAD(n)		((0x1 << (3 + 16)) | ((n) << 3))
#define SSMOD_RESET(n)			((0x1 << (2 + 16)) | ((n) << 2))
#define SSMOD_DIS_SSCG(n)		((0x1 << (1 + 16)) | ((n) << 1))
#define SSMOD_BP(n)			((0x1 << (0 + 16)) | ((n) << 0))
/* SUBDDR_CLKSEL_CON00 0x0300 */
#define CLK_DPLL			0
#define ACLK_SYSMEM_SRC			1
#define CLK1X_PLL_ROOT_SEL(n)		((0x1 << (1 + 16)) | ((n) << 1))
/* SUBDDR_SOFTRST_CON00	0x0a00 */
#define RESETN_DDRPHY(n)		((0x1 << (13 + 16)) | ((n) << 13))
#define RESETN_CORE_DDRC(n)		((0x1 << (7 + 16)) | ((n) << 7))
#define ARESETN_DDRSCH(n)		((0xf << (3 + 16)) | (((n) == 0 ? 0x0 : 0xf) << 3))
#define ARESETN_DDR_BIU(n)		((0x1 << (2 + 16)) | ((n) << 2))

/* PMU1 registers */
#define PMU_DDR_PWR_SFTCON		0x110
#define PMU_DDR_STS			0x144
/* PMU_DDR_PWR_SFTCON */
#define SW_DDRIO_RSTIOV_EXIT		(BIT(4 + 16) | BIT(4))
#define SW_DDRIO_RSTIOV_EN		(BIT(3 + 16) | BIT(3))
#define SW_DDRIO_RSTIOV_DIS		(BIT(3 + 16))
#define SW_DDRIO_RET_EXIT		(BIT(2 + 16) | BIT(2))
#define SW_DDRIO_RET_EN			(BIT(1 + 16) | BIT(1))
#define SW_DDRIO_RET_DIS		(BIT(1 + 16))
#define DDR_SREF_C_SFTENA		(BIT(0 + 16) | BIT(0))
#define DDR_SREF_C_SFTENA_DIS		(BIT(0 + 16))
/* PMU_DDR_STS */
#define DDRCTL_C_SYSACTIVE		BIT(3)
#define DDRCTL_C_SYSACK			BIT(2)
#define DDRIO_RSTIOV			BIT(1)
#define DDRIO_RETON			BIT(0)

/* PMU2 registers */
#define PMU2_BIU_IDLE_SFTCON0		0x0110
#define PMU2_BIU_IDLE_ACK_STS0		0x0120
#define PMU2_BIU_IDLE_STS0		0x0128
#define PMU2_BIU_GATEMASK_CON0		0x0130
#define PMU2_PWR_GATE_STS		0x0230
/* PMU2_BIU_IDLE_SFTCON0 */
#define IDLE_REQ_DDR(set)		((set) << 0)
#define IDLE_REQ_DDR_MASK		(0x1 << (0 + 16))
#define IDLE_REQ_VEPU(set)		((set) << 4)
#define IDLE_REQ_VEPU_MASK		(0x1 << (4 + 16))
#define IDLE_REQ_VCP(set)		((set) << 5)
#define IDLE_REQ_VCP_MASK		(0x1 << (5 + 16))
#define IDLE_REQ_VDO(set)		((set) << 9)
#define IDLE_REQ_VDO_MASK		(0x1 << (9 + 16))
/* PMU2_BIU_IDLE_STS0 */
#define IDLE_DDR			(0x1 << 0)
#define IDLE_VEPU			(0x1 << 4)
#define IDLE_VCP			(0x1 << 5)
#define IDLE_VDO			(0x1 << 9)
/* PMU2_BIU_GATEMASK_CON0 */
#define BIT_AUTO_VDO_ENA		((0x1 << 9) | (0x1 << (9 + 16)))
#define BIT_AUTO_VCP_ENA		((0x1 << 5) | (0x1 << (5 + 16)))
#define BIT_AUTO_VEPU_ENA		((0x1 << 4) | (0x1 << (4 + 16)))
/* PMU2_PWR_GATE_STS */
#define PD_VDO_DWN_STAT			(0x1 << 1)

/* SYSGRG */
#define GRF_BIU_CON0			0x0030
#define GRF_BIU_CON1			0x0034

/* SYS_SGRF */
#define SYS_SGRF_AXI_SECURE_SGRF_CON0	0x0018
/* SYS_SGRF_AXI_SECURE_SGRF_CON0 */
#define SECURE_CTRL_AWPROT_DCF_SHIFT	3
#define SECURE_CTRL_ARPROT_DCF_SHIFT	2

/* GRF_BIU_CONx */
#define ERROR_RESPONSE			0
#define STALL_RESPONSE			1

/* PMUGRF_SOC_CON0 */
#define PMUGRF_DDRPHY_BUFFEREN_EN	BITS_WITH_WMASK(0x1, 0x3, 9)
#define PMUGRF_DDRPHY_BUFFEREN_DIS	BITS_WITH_WMASK(0x3, 0x3, 9)
#define PMUGRF_DDRPHY_BUFFEREN_SEL_PMU	((0x3 << (16 + 9)) | (0x2 << 9))

/* BUSCRU_BASE Register */
#define BUSCRU_BUS_GATE_CON02		0x808
/* BUSCRU_BUS_GATE_CON02 */
#define ACLK_DCF_EN_SHIFT		1
#define PCLK_DCF_EN_SHIFT		0

/* VDOCRU registers */
#define VDO_GATE_CON00			0x800
/* VDO_GATE_CON00 */
#define HCLK_VOP_EN			BIT(11)
#define ACLK_VOP_EN			BIT(10)

/* VOP_LITE registers */
#define VOP_SYS_CTRL2			(0x18)
#define VOP_LINE_FLAG			(0x30)
#define VOP_INTR_CLEAR			(0x38)
#define VOP_INTR_STATUS			(0x3c)
#define VOP_DSP_HTOTAL_HS_END		(0x100)
#define VOP_DSP_VTOTAL_HS_END		(0x108)
#define VOP_WIN2_CTRL0			(0x190)
#define VOP_SCAN_LINE_NUM		(0x1f0)
/* VOP_SYS_CTRL2 */
#define IMD_VOP_STANDBY_EN		BIT(1)
/* VOP_INTR_CLEAR */
#define LINE_FLAG0_INTR_CLR		BIT_SET_WMSK(3)
/* VOP_INTR_STATUS */
#define LINE_FLAG0_INTR_RAW_STS		BIT(19)
#define LINE_FLAG0_INTR_STS		BIT(3)
/* VOP_WIN2_CTRL0 */
#define WIN_EN				BIT(0)

struct rv1126b_sdram_channel {
	unsigned char rank;
	unsigned char col;
	/* 3:8bank, 2:4bank */
	unsigned char bk;
	/* channel buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned char bw;
	/* die buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned char dbw;
	unsigned char row_3_4;
	unsigned char cs0_row;
	unsigned char cs1_row;
	unsigned char cs0_high16bit_row;
	unsigned char cs1_high16bit_row;
	unsigned int ddrconfig;
};

struct rv1126b_ddr_pctl_regs {
	uint32_t pctl[95][2];
	/* for pctl_f1, pctl_f2, pctl_f3 */
	uint32_t pctl_f[3][28][2];
};

struct rv1126b_ddr_phy_regs {
	uint32_t phy[55][2];
};

struct rv1126b_ddr_related_regs {
	uint32_t noc_devicesize;
	uint32_t ddrgrf[15][2];
	uint32_t cru_dpll_con[4];
	uint32_t subddrcru_subddrgate0;
	uint32_t sgrf_soc_con2;
};

struct rv1126b_ddr_skew {
	uint8_t ca_skew[32];
	uint8_t cs1_ca_skew[12];
	uint8_t cs2_3_ca_skew[2][12];
	uint8_t cs_dq_skew[4][4][22];
	uint8_t cs_rx_dqs_cyc[4][4];
	uint8_t cs_rx_dqs_oph[4][4];
	uint8_t cs_rx_dqs_dll[4][4];
};

struct rv1126b_dmc_config {
	unsigned int dramtype;
};

/*
 * Same as struct rv1126b_fsp_param in
 * u-boot-ddr/arch/arm/include/asm/arch-rockchip/sdram_rv1126b.h
 */
struct rv1126b_fsp_param {
	uint32_t flag;
	uint32_t freq_mhz;

	/* DPLL */
	uint32_t dpll_con[5];

	/* dram size */
	uint32_t dq_odt;
	uint32_t ca_odt;
	uint32_t ds_pdds;
	uint32_t vref_ca[2];
	uint32_t vref_dq[2];

	/* phy side, [0] for pull up, [1] for pull down */
	uint32_t wr_dq_drv[2];
	uint32_t wr_ca_drv[2];
	uint32_t wr_ck_drv[2];
	uint32_t wr_cs_drv[2];
	uint32_t rd_odt[2];
	uint32_t vref_inner;
	uint32_t vref_out;
	uint32_t lp4_drv_pd_en;
	uint32_t rx_rcv_mode;
	uint32_t phy_wdqsoff_on_length;
	uint32_t phy_hold_timing;

	uint32_t phy_pllpostdiv;
	uint32_t phy_pllpostdiven;
	uint32_t phy_al;
	uint32_t phy_cl;
	uint32_t phy_cwl;
	uint32_t reserved[2];

	struct rv1126b_ddr_skew skew;
};

#define MAX_IDX				4
#define FSP_FLAG			0xfead0001
/*
 * Borrow share memory space to temporarily store FSP parame.
 * In the stage of DDR init write FSP parame to this space.
 * In the stage of trust init move FSP parame to SRAM space from share memory space.
 */
#define FSP_PARAM_BASE			SHARE_MEM_BASE

uint32_t dmc_get_dramtype(void);
__pmusramfunc void dmc_enter_lp(void);
void dmc_save(void);
__pmusramfunc void dmc_restore(void);
void dmc_init(void);

#endif	/* __PLAT_ROCKCHIP_DMC_RV1126B_H__ */

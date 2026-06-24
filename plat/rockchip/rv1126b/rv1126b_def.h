/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

#define SIZE_K(n)			((n) * 1024)
#define SIZE_M(n)			((n) * 1024 * 1024)

#define WITH_16BITS_WMSK(bits)		(0xffff0000 | (bits))
#define BITS_WMSK(msk, shift)		((msk) << ((shift) + REG_MSK_SHIFT))

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

#define RK_DRAM_BASE			0x40000000
#define RK_DRAM_RGN_BASE		0x0

#define RV1126B_DEV_RNG0_BASE		0x00000000
#define RV1126B_DEV_RNG0_SIZE		0x40000000

/* All slave base address declare below */
#define TOPCRU_BASE			0x20000000
#define BUSCRU_BASE			0x20010000
#define PERICRU_BASE			0x20020000
#define CORECRU_BASE			0x20030000
#define PMU0CRU_BASE			0x20040000
#define PMU1CRU_BASE			0x20050000
#define DDRCRU_BASE			0x20060000
#define SUBDDRCRU_BASE			0x20068000
#define VICRU_BASE			0x20070000
#define VEPUCRU_BASE			0x20080000
#define NPUCRU_BASE			0x20090000
#define VDOCRU_BASE			0x200a0000
#define VCPCRU_BASE			0x200b0000

#define SYSGRF_BASE			0x20100000
#define PERIGRF_BASE			0x20110000
#define U3PHYGRF_BASE			0x20118000
#define COREGRF_BASE			0x20120000
#define PMUGRF_BASE			0x20130000
#define DDRGRF_BASE			0x20140000
#define VIGRF_BASE			0x20150000
#define VEPUGRF_BASE			0x20160000
#define NPUGRF_BASE			0x20170000
#define VDOGRF_BASE			0x20180000
#define VCPGRF_BASE			0x20190000
#define PMU0IOC_BASE			0x201a0000
#define PMU1IOC_BASE			0x201a8000
#define IOC1_BASE			0x201b0000
#define IOC2_BASE			0x201b8000
#define IOC3_BASE			0x201c0000
#define IOC4_BASE			0x201c8000
#define IOC5_BASE			0x201d0000
#define IOC6_BASE			0x201d8000
#define IOC7_BASE			0x201e0000

#define BUSSCRU_BASE			0x20200000
#define PMUSCRU_BASE			0x20210000

#define SYSSGRF_BASE			0x20220000
#define PMUSGRF_BASE			0x20230000
#define PVTPLL_CORE_BASE		0x20480000
#define PVTPLL_VI_BASE			0x21C60000
#define PVTPLL_VEPU_BASE		0x21F00000
#define PVTPLL_VCP_BASE			0x21FC0000
#define PVTPLL_NPU_BASE			0x22080000

#define GPIO0_BASE			0x20600000
#define PWM1_BASE			0x20700000
#define I2C2_BASE			0x20810000
#define UART0_BASE			0x20810000
#define HPTIMER_BASE			0x20820000
#define PMU_BASE			0x20830000
#define PMU0_BASE			0x20830000
#define PMU1_BASE			0x20834000
#define PMU2_BASE			0x20838000
#define PMU_WDT_BASE			0x20850000
#define RC_OSC_CTRL_BASE		0x20860000
#define STIMER_BASE			0x20a00000
#define WDT_S_BASE			0x20b00000
#define OTP_S_BASE			0x20b10000
#define WDT_NS_BASE			0x20b60000
#define DCF_BASE			0x20b70000
#define OTP_NS_BASE			0x20b90000
#define OTP_MASK_BASE			0x20ba0000
#define TSADC_BASE			0x20bb0000
#define NSTIMER_BASE			0x20c00000

#define UART1_BASE			0x21160000
#define UART2_BASE			0x21170000
#define UART3_BASE			0x21180000
#define UART4_BASE			0x21190000
#define UART5_BASE			0x211a0000
#define UART6_BASE			0x211b0000
#define UART7_BASE			0x211c0000
#define GIC400_BASE			0x21200000

#define GPIO1_BASE			0x21300000
#define DDRCTL_BASE			0x21600000
#define DDRPHY_BASE			0x21650000
#define FW_DDR_BASE			0x21660000
#define GPIO2_BASE			0x21700000
#define GPIO4_BASE			0x21800000
#define GPIO5_BASE			0x21900000
#define GPIO6_BASE			0x21a00000
#define GPIO7_BASE			0x21b00000
#define GPIO3_BASE			0x21e00000
#define VOP_BASE			0x22150000

#define PMUSRAM_RSIZE			SIZE_K(8)
#define PMUSRAM_BASE			0x3ff1e000
#define AOASRAM_BASE			0x3ff20000
#define SRAM_BASE			0x3ffb0000

#define MAX_MEM_OS_REG_NUM		4
#define MEM_OS_REG_BASE			\
	(PMUSRAM_BASE + PMUSRAM_RSIZE - MAX_MEM_OS_REG_NUM * 4)

#define PSRAM_SP_TOP			MEM_OS_REG_BASE

#define STIMER_CHN_BASE(i)		(STIMER_BASE + 0x10000 * (i))
#define NSTIMER_CHN_BASE(i)		(NSTIMER_BASE + 0x10000 * (i))

/**************************************************************************
 * share mem region allocation: 1M~2M
 **************************************************************************/
#define DDR_SHARE_MEM			(RK_DRAM_BASE + SIZE_K(1024))
#define DDR_SHARE_SIZE			SIZE_K(64)

#define SHARE_MEM_BASE			DDR_SHARE_MEM
#define SHARE_MEM_PAGE_NUM		15
#define SHARE_MEM_SIZE			SIZE_K(SHARE_MEM_PAGE_NUM * 4)

#define	SCMI_SHARE_MEM_BASE		(SHARE_MEM_BASE + SHARE_MEM_SIZE)
#define	SCMI_SHARE_MEM_SIZE		SIZE_K(4)

#define SMT_BUFFER_BASE			SCMI_SHARE_MEM_BASE
#define SMT_BUFFER0_BASE		SMT_BUFFER_BASE

#define ROCKCHIP_PM_REG_REGION_MEM_SIZE		SIZE_K(8)

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK_DBG_UART_BASE		UART0_BASE
#define RK_DBG_UART_BAUDRATE		1500000
#define RK_DBG_UART_CLOCK		24000000

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define PLAT_GICD_BASE			(GIC400_BASE + 0x1000)
#define PLAT_GICC_BASE			(GIC400_BASE + 0x2000)
#define PLAT_GICR_BASE			0

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define RK_IRQ_SEC_SGI_0		8
#define RK_IRQ_SEC_SGI_1		9
#define RK_IRQ_SEC_SGI_2		10
#define RK_IRQ_SEC_SGI_3		11
#define RK_IRQ_SEC_SGI_4		12
#define RK_IRQ_SEC_SGI_5		13
#define RK_IRQ_SEC_SGI_6		14
#define RK_IRQ_SEC_SGI_7		15
#define RK_IRQ_SEC_PHY_TIMER		29

/*
 * Define a list of Group 0 interrupts.
 */
#define PLAT_RK_GICV2_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),		\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL)
#endif /* __PLAT_DEF_H__ */

/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

#define SIZE_K(n)		((n) * 1024)

#define WITH_16BITS_WMSK(bits)	(0xffff0000 | (bits))

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define UMCTL0_BASE		0xf7000000
#define UMCTL1_BASE		0xf8000000
#define UMCTL2_BASE		0xf9000000
#define UMCTL3_BASE		0xfa000000

#define GIC600_BASE		0xfe600000
#define GIC600_SIZE		SIZE_K(64)

#define DAPLITE_BASE		0xfd100000
#define PMU0SGRF_BASE		0xfd580000
#define PMU1SGRF_BASE		0xfd582000
#define BUSSGRF_BASE		0xfd586000
#define DSUSGRF_BASE		0xfd587000
#define PMU0GRF_BASE		0xfd588000
#define PMU1GRF_BASE		0xfd58a000

#define SYSGRF_BASE		0xfd58c000
#define BIGCORE0GRF_BASE	0xfd590000
#define BIGCORE1GRF_BASE	0xfd592000
#define LITCOREGRF_BASE		0xfd594000
#define DSUGRF_BASE		0xfd598000
#define DDR01GRF_BASE		0xfd59c000
#define DDR23GRF_BASE		0xfd59d000
#define CENTERGRF_BASE		0xfd59e000
#define GPUGRF_BASE		0xfd5a0000
#define NPUGRF_BASE		0xfd5a2000
#define USBGRF_BASE		0xfd5ac000
#define PHPGRF_BASE		0xfd5b0000
#define PCIE3PHYGRF_BASE	0xfd5b8000
#define USB2PHY0_GRF_BASE	0xfd5d0000
#define USB2PHY1_GRF_BASE	0xfd5d4000
#define USB2PHY2_GRF_BASE	0xfd5d8000
#define USB2PHY3_GRF_BASE	0xfd5dc000

#define PMU0IOC_BASE		0xfd5f0000
#define PMU1IOC_BASE		0xfd5f4000
#define BUSIOC_BASE		0xfd5f8000
#define VCCIO1_4_IOC_BASE	0xfd5f9000
#define VCCIO3_5_IOC_BASE	0xfd5fa000
#define VCCIO2_IOC_BASE		0xfd5fb000
#define VCCIO6_IOC_BASE		0xfd5fc000

#define SRAM_BASE		0xff000000
#define PMUSRAM_BASE		0xff100000
#define PMUSRAM_SIZE		SIZE_K(128)
#define PMUSRAM_RSIZE		SIZE_K(64)

#define CRU_BASE		0xfd7c0000
#define PHP_CRU_BASE		0xfd7c8000
#define SCRU_BASE		0xfd7d0000
#define BUSSCRU_BASE		0xfd7d8000
#define PMU1SCRU_BASE		0xfd7e0000
#define PMU1CRU_BASE		0xfd7f0000

#define DDR0CRU_BASE		0xfd800000
#define DDR1CRU_BASE		0xfd804000
#define DDR2CRU_BASE		0xfd808000
#define DDR3CRU_BASE		0xfd80c000

#define BIGCORE0CRU_BASE	0xfd810000
#define BIGCORE1CRU_BASE	0xfd812000
#define LITCRU_BASE		0xfd814000
#define DSUCRU_BASE		0xfd818000

#define I2C0_BASE		0xfd880000
#define UART0_BASE		0xfd890000
#define GPIO0_BASE		0xfd8a0000
#define PWM0_BASE		0xfd8b0000
#define PMUPVTM_BASE		0xfd8c0000
#define TIMER_HP_BASE		0xfd8c8000
#define PMU0_BASE		0xfd8d0000
#define PMU1_BASE		0xfd8d4000
#define PMU2_BASE		0xfd8d8000
#define PMU_BASE		PMU0_BASE
#define PMUWDT_BASE		0xfd8e0000
#define PMUTIMER_BASE		0xfd8f0000
#define OSC_CHK_BASE		0xfd9b0000
#define VOP_BASE		0xfdd90000
#define HDMIRX_BASE		0xfdee0000

#define MSCH0_BASE		0xfe000000
#define MSCH1_BASE		0xfe002000
#define MSCH2_BASE		0xfe004000
#define MSCH3_BASE		0xfe006000
#define FIREWALL_DSU_BASE	0xfe010000
#define FIREWALL_DDR_BASE	0xfe030000
#define FIREWALL_SYSMEM_BASE	0xfe038000
#define DDRPHY0_BASE		0xfe0c0000
#define DDRPHY1_BASE		0xfe0d0000
#define DDRPHY2_BASE		0xfe0e0000
#define DDRPHY3_BASE		0xfe0f0000
#define TIMER_DDR_BASE		0xfe118000
#define KEYLADDER_BASE		0xfe380000
#define CRYPTO_S_BASE		0xfe390000
#define OTP_S_BASE		0xfe3a0000
#define DCF_BASE		0xfe3c0000
#define STIMER0_BASE		0xfe3d0000
#define WDT_S_BASE		0xfe3e0000
#define CRYPTO_S_BY_KEYLAD_BASE	0xfe420000
#define NSTIMER0_BASE		0xfeae0000
#define NSTIMER1_BASE		0xfeae8000
#define WDT_NS_BASE		0xfeaf0000

#define UART1_BASE		0xfeb40000
#define UART2_BASE		0xfeb50000
#define UART3_BASE		0xfeb60000
#define UART4_BASE		0xfeb70000
#define UART5_BASE		0xfeb80000
#define UART6_BASE		0xfeb90000
#define UART7_BASE		0xfeba0000
#define UART8_BASE		0xfebb0000
#define UART9_BASE		0xfebc0000

#define GPIO1_BASE		0xfec20000
#define GPIO2_BASE		0xfec30000
#define GPIO3_BASE		0xfec40000
#define GPIO4_BASE		0xfec50000

#define MAILBOX1_BASE		0xfec70000
#define OTP_NS_BASE		0xfecc0000
#define INTMUX0_DDR_BASE	0Xfecf8000
#define INTMUX1_DDR_BASE	0Xfecfc000
#define STIMER1_BASE		0xfed30000

/**************************************************************************
 * sys sram allocation
 **************************************************************************/
#define SRAM_ENTRY_BASE		SRAM_BASE
#define SRAM_PMUM0_SHMEM_BASE	(SRAM_ENTRY_BASE + SIZE_K(3))
#define SRAM_LD_BASE		(SRAM_ENTRY_BASE + SIZE_K(4))
#define SRAM_LD_SIZE		SIZE_K(64)

#define SRAM_LD_SP		(SRAM_LD_BASE + SRAM_LD_SIZE -\
				 128)

/**************************************************************************
 * share mem region allocation: 1M~2M
 **************************************************************************/
#define DDR_SHARE_MEM		SIZE_K(1024)
#define DDR_SHARE_SIZE		SIZE_K(64)

#define SHARE_MEM_BASE		DDR_SHARE_MEM
#define SHARE_MEM_PAGE_NUM	15
#define SHARE_MEM_SIZE		SIZE_K(SHARE_MEM_PAGE_NUM * 4)

#define	SCMI_SHARE_MEM_BASE	(SHARE_MEM_BASE + SHARE_MEM_SIZE)
#define	SCMI_SHARE_MEM_SIZE	SIZE_K(4)

#define SMT_BUFFER_BASE		SCMI_SHARE_MEM_BASE
#define SMT_BUFFER0_BASE	SMT_BUFFER_BASE

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK_DBG_UART_BASE		UART2_BASE
#define RK_DBG_UART_BAUDRATE		1500000
#define RK_DBG_UART_CLOCK		24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define PLAT_GICD_BASE			GIC600_BASE
#define PLAT_GICC_BASE			0
#define PLAT_GICR_BASE			(GIC600_BASE + 0x80000)
#define PLAT_GICITS0_BASE		0xfe640000
#define PLAT_GICITS1_BASE		0xfe660000

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
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */

#define PLAT_RK_GICV3_G1S_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		       INTR_GROUP1S, GIC_INTR_CFG_LEVEL)

#define PLAT_RK_GICV3_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       INTR_GROUP0, GIC_INTR_CFG_LEVEL)

/******************************************************************************
 * pm reg region memory
 ******************************************************************************/
#define ROCKCHIP_PM_REG_REGION_MEM_SIZE		SIZE_K(4)

#endif /* __PLAT_DEF_H__ */

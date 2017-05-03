/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT8173_DEF_H__
#define __MT8173_DEF_H__

#if RESET_TO_BL31
#error "MT8173 is incompatible with RESET_TO_BL31!"
#endif

#define MT8173_PRIMARY_CPU	0x0

/* Register base address */
#define IO_PHYS			(0x10000000)
#define INFRACFG_AO_BASE	(IO_PHYS + 0x1000)
#define SRAMROM_SEC_BASE	(IO_PHYS + 0x1800)
#define PERI_CON_BASE		(IO_PHYS + 0x3000)
#define GPIO_BASE		(IO_PHYS + 0x5000)
#define SPM_BASE		(IO_PHYS + 0x6000)
#define RGU_BASE		(IO_PHYS + 0x7000)
#define PMIC_WRAP_BASE		(IO_PHYS + 0xD000)
#define DEVAPC0_BASE		(IO_PHYS + 0xE000)
#define MCUCFG_BASE		(IO_PHYS + 0x200000)
#define APMIXED_BASE		(IO_PHYS + 0x209000)
#define TRNG_BASE		(IO_PHYS + 0x20F000)
#define CRYPT_BASE		(IO_PHYS + 0x210000)
#define MT_GIC_BASE		(IO_PHYS + 0x220000)
#define PLAT_MT_CCI_BASE	(IO_PHYS + 0x390000)

/* Aggregate of all devices in the first GB */
#define MTK_DEV_RNG0_BASE	IO_PHYS
#define MTK_DEV_RNG0_SIZE	0x400000
#define MTK_DEV_RNG1_BASE	(IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE	0x4000000

/* SRAMROM related registers */
#define SRAMROM_SEC_CTRL	(SRAMROM_SEC_BASE + 0x4)
#define SRAMROM_SEC_ADDR	(SRAMROM_SEC_BASE + 0x8)

/* DEVAPC0 related registers */
#define DEVAPC0_MAS_SEC_0	(DEVAPC0_BASE + 0x500)
#define DEVAPC0_APC_CON		(DEVAPC0_BASE + 0xF00)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define MT8173_UART0_BASE	(IO_PHYS + 0x01002000)
#define MT8173_UART1_BASE	(IO_PHYS + 0x01003000)
#define MT8173_UART2_BASE	(IO_PHYS + 0x01004000)
#define MT8173_UART3_BASE	(IO_PHYS + 0x01005000)

#define MT8173_BAUDRATE		(115200)
#define MT8173_UART_CLOCK	(26000000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	13000000

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE + 0x1000)
#define BASE_GICC_BASE		(MT_GIC_BASE + 0x2000)
#define BASE_GICR_BASE		0	/* no GICR in GIC-400 */
#define BASE_GICH_BASE		(MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE		(MT_GIC_BASE + 0x6000)
#define INT_POL_CTL0		0x10200620

#define GIC_PRIVATE_SIGNALS	(32)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX	3

/*******************************************************************************
 * WDT related constants
 ******************************************************************************/
#define MTK_WDT_BASE		(RGU_BASE + 0)
#define MTK_WDT_SWRST		(MTK_WDT_BASE + 0x0014)

#define MTK_WDT_MODE_DUAL_MODE	0x0040
#define MTK_WDT_MODE_IRQ	0x0008
#define MTK_WDT_MODE_KEY	0x22000000
#define MTK_WDT_MODE_EXTEN	0x0004
#define MTK_WDT_SWRST_KEY	0x1209

/* FIQ platform related define */
#define MT_IRQ_SEC_SGI_0	8
#define MT_IRQ_SEC_SGI_1	9
#define MT_IRQ_SEC_SGI_2	10
#define MT_IRQ_SEC_SGI_3	11
#define MT_IRQ_SEC_SGI_4	12
#define MT_IRQ_SEC_SGI_5	13
#define MT_IRQ_SEC_SGI_6	14
#define MT_IRQ_SEC_SGI_7	15

/*
 *  Macros for local power states in MTK platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define MTK_LOCAL_STATE_RUN     0
/* Local power state for retention. Valid only for CPU power domains */
#define MTK_LOCAL_STATE_RET     1
/* Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains
 */
#define MTK_LOCAL_STATE_OFF     2

#if PSCI_EXTENDED_STATE_ID
/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define MTK_LOCAL_PSTATE_WIDTH		4
#define MTK_LOCAL_PSTATE_MASK		((1 << MTK_LOCAL_PSTATE_WIDTH) - 1)

/* Macros to construct the composite power state */

/* Make composite power state parameter till power level 0 */

#define mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
	(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))
#else
#define mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		((type) << PSTATE_TYPE_SHIFT))

#endif /* __PSCI_EXTENDED_STATE_ID__ */

/* Make composite power state parameter till power level 1 */
#define mtk_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << MTK_LOCAL_PSTATE_WIDTH) | \
		mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/* Make composite power state parameter till power level 2 */
#define mtk_make_pwrstate_lvl2( \
		lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl2_state) << (MTK_LOCAL_PSTATE_WIDTH * 2)) | \
		mtk_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type))


#endif /* __MT8173_DEF_H__ */

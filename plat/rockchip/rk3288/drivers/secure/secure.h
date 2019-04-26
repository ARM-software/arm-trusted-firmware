/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_H
#define SECURE_H

/******************************************************************************
 * TZPC TrustZone controller
 ******************************************************************************/

#define TZPC_R0SIZE			0x0
#define TZPC_SRAM_SECURE_4K(n)		((n) > 0x200 ? 0x200 : (n))
#define TZPC_DECPROT1STAT		0x80c
#define TZPC_DECPROT1SET		0x810
#define TZPC_DECPROT1CLR		0x814
#define TZPC_DECPROT2STAT		0x818
#define TZPC_DECPROT2SET		0x818
#define TZPC_DECPROT2CLR		0x820

/**************************************************
 * sgrf reg, offset
 **************************************************/
/*
 * soc_con0-5 start at 0x0, soc_con6-... start art 0x50
 * adjusted for the 5 lower registers
 */
#define SGRF_SOC_CON(n)			((((n) < 6) ? 0x0 : 0x38) + (n) * 4)
#define SGRF_BUSDMAC_CON(n)		(0x20 + (n) * 4)
#define SGRF_CPU_CON(n)			(0x40 + (n) * 4)
#define SGRF_SOC_STATUS(n)		(0x100 + (n) * 4)
#define SGRF_FAST_BOOT_ADDR		0x120

/* SGRF_SOC_CON0 */
#define SGRF_FAST_BOOT_ENA		BIT_WITH_WMSK(8)
#define SGRF_FAST_BOOT_DIS		WMSK_BIT(8)
#define SGRF_PCLK_WDT_GATE		BIT_WITH_WMSK(6)
#define SGRF_PCLK_WDT_UNGATE		WMSK_BIT(6)
#define SGRF_PCLK_STIMER_GATE		BIT_WITH_WMSK(4)

#define SGRF_SOC_CON2_MST_NS		0xffe0ffe0
#define SGRF_SOC_CON3_MST_NS		0x003f003f

/* SGRF_SOC_CON4 */
#define SGRF_SOC_CON4_SECURE_WMSK	0xffff0000
#define SGRF_DDRC1_SECURE		BIT_WITH_WMSK(12)
#define SGRF_DDRC0_SECURE		BIT_WITH_WMSK(11)
#define SGRF_PMUSRAM_SECURE		BIT_WITH_WMSK(8)
#define SGRF_WDT_SECURE			BIT_WITH_WMSK(7)
#define SGRF_STIMER_SECURE		BIT_WITH_WMSK(6)

/* SGRF_SOC_CON5 */
#define SGRF_SLV_SEC_BYPS		BIT_WITH_WMSK(15)
#define SGRF_SLV_SEC_NO_BYPS		WMSK_BIT(15)
#define SGRF_SOC_CON5_SECURE_WMSK	0x00ff0000

/* ddr regions in SGRF_SOC_CON6 and following */
#define SGRF_DDR_RGN_SECURE_SEL		BIT_WITH_WMSK(15)
#define SGRF_DDR_RGN_SECURE_EN		BIT_WITH_WMSK(14)
#define SGRF_DDR_RGN_ADDR_WMSK		0x0fff

/* SGRF_SOC_CON21 */
/* All security of the DDR RGNs are bypassed */
#define SGRF_DDR_RGN_BYPS		BIT_WITH_WMSK(15)
#define SGRF_DDR_RGN_NO_BYPS		WMSK_BIT(15)

/* SGRF_CPU_CON0 */
#define SGRF_DAPDEVICE_ENA		BIT_WITH_WMSK(0)
#define SGRF_DAPDEVICE_MSK		WMSK_BIT(0)

/*****************************************************************************
 * core-axi
 *****************************************************************************/
#define CORE_AXI_SECURITY0		0x08
#define AXI_SECURITY0_GIC		BIT(0)

/*****************************************************************************
 * secure timer
 *****************************************************************************/
#define TIMER_LOAD_COUNT0		0x00
#define TIMER_LOAD_COUNT1		0x04
#define TIMER_CURRENT_VALUE0		0x08
#define TIMER_CURRENT_VALUE1		0x0C
#define TIMER_CONTROL_REG		0x10
#define TIMER_INTSTATUS			0x18

#define TIMER_EN			0x1

#define STIMER1_BASE			(STIME_BASE + 0x20)

/* export secure operating APIs */
void secure_watchdog_gate(void);
void secure_watchdog_ungate(void);
void secure_gic_init(void);
void secure_timer_init(void);
void secure_sgrf_init(void);
void secure_sgrf_ddr_rgn_init(void);
__pmusramfunc void sram_secure_timer_init(void);

#endif /* SECURE_H */

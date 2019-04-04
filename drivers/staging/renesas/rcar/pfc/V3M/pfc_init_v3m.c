/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>		/* for uint32_t */
#include <lib/mmio.h>
#include "pfc_init_v3m.h"
#include "include/rcar_def.h"
#include "rcar_private.h"

#define	RST_MODEMR		0xE6160060		// Mode Monitor Register

/* GPIO base address */
#define	GPIO_BASE		(0xE6050000U)

/* GPIO registers */
#define	GPIO_IOINTSEL0		(GPIO_BASE + 0x0000U)
#define	GPIO_INOUTSEL0		(GPIO_BASE + 0x0004U)
#define	GPIO_OUTDT0			(GPIO_BASE + 0x0008U)
#define	GPIO_INDT0			(GPIO_BASE + 0x000CU)
#define	GPIO_INTDT0			(GPIO_BASE + 0x0010U)
#define	GPIO_INTCLR0		(GPIO_BASE + 0x0014U)
#define	GPIO_INTMSK0		(GPIO_BASE + 0x0018U)
#define	GPIO_MSKCLR0		(GPIO_BASE + 0x001CU)
#define	GPIO_POSNEG0		(GPIO_BASE + 0x0020U)
#define	GPIO_EDGLEVEL0		(GPIO_BASE + 0x0024U)
#define	GPIO_FILONOFF0		(GPIO_BASE + 0x0028U)
#define	GPIO_INTMSKS0		(GPIO_BASE + 0x0038U)
#define	GPIO_MSKCLRS0		(GPIO_BASE + 0x003CU)
#define	GPIO_OUTDTSEL0		(GPIO_BASE + 0x0040U)
#define	GPIO_OUTDTH0		(GPIO_BASE + 0x0044U)
#define	GPIO_OUTDTL0		(GPIO_BASE + 0x0048U)
#define	GPIO_BOTHEDGE0		(GPIO_BASE + 0x004CU)
#define	GPIO_IOINTSEL1		(GPIO_BASE + 0x1000U)
#define	GPIO_INOUTSEL1		(GPIO_BASE + 0x1004U)
#define	GPIO_OUTDT1			(GPIO_BASE + 0x1008U)
#define	GPIO_INDT1			(GPIO_BASE + 0x100CU)
#define	GPIO_INTDT1			(GPIO_BASE + 0x1010U)
#define	GPIO_INTCLR1		(GPIO_BASE + 0x1014U)
#define	GPIO_INTMSK1		(GPIO_BASE + 0x1018U)
#define	GPIO_MSKCLR1		(GPIO_BASE + 0x101CU)
#define	GPIO_POSNEG1		(GPIO_BASE + 0x1020U)
#define	GPIO_EDGLEVEL1		(GPIO_BASE + 0x1024U)
#define	GPIO_FILONOFF1		(GPIO_BASE + 0x1028U)
#define	GPIO_INTMSKS1		(GPIO_BASE + 0x1038U)
#define	GPIO_MSKCLRS1		(GPIO_BASE + 0x103CU)
#define	GPIO_OUTDTSEL1		(GPIO_BASE + 0x1040U)
#define	GPIO_OUTDTH1		(GPIO_BASE + 0x1044U)
#define	GPIO_OUTDTL1		(GPIO_BASE + 0x1048U)
#define	GPIO_BOTHEDGE1		(GPIO_BASE + 0x104CU)
#define	GPIO_IOINTSEL2		(GPIO_BASE + 0x2000U)
#define	GPIO_INOUTSEL2		(GPIO_BASE + 0x2004U)
#define	GPIO_OUTDT2			(GPIO_BASE + 0x2008U)
#define	GPIO_INDT2			(GPIO_BASE + 0x200CU)
#define	GPIO_INTDT2			(GPIO_BASE + 0x2010U)
#define	GPIO_INTCLR2		(GPIO_BASE + 0x2014U)
#define	GPIO_INTMSK2		(GPIO_BASE + 0x2018U)
#define	GPIO_MSKCLR2		(GPIO_BASE + 0x201CU)
#define	GPIO_POSNEG2		(GPIO_BASE + 0x2020U)
#define	GPIO_EDGLEVEL2		(GPIO_BASE + 0x2024U)
#define	GPIO_FILONOFF2		(GPIO_BASE + 0x2028U)
#define	GPIO_INTMSKS2		(GPIO_BASE + 0x2038U)
#define	GPIO_MSKCLRS2		(GPIO_BASE + 0x203CU)
#define	GPIO_OUTDTSEL2		(GPIO_BASE + 0x2040U)
#define	GPIO_OUTDTH2		(GPIO_BASE + 0x2044U)
#define	GPIO_OUTDTL2		(GPIO_BASE + 0x2048U)
#define	GPIO_BOTHEDGE2		(GPIO_BASE + 0x204CU)
#define	GPIO_IOINTSEL3		(GPIO_BASE + 0x3000U)
#define	GPIO_INOUTSEL3		(GPIO_BASE + 0x3004U)
#define	GPIO_OUTDT3			(GPIO_BASE + 0x3008U)
#define	GPIO_INDT3			(GPIO_BASE + 0x300CU)
#define	GPIO_INTDT3			(GPIO_BASE + 0x3010U)
#define	GPIO_INTCLR3		(GPIO_BASE + 0x3014U)
#define	GPIO_INTMSK3		(GPIO_BASE + 0x3018U)
#define	GPIO_MSKCLR3		(GPIO_BASE + 0x301CU)
#define	GPIO_POSNEG3		(GPIO_BASE + 0x3020U)
#define	GPIO_EDGLEVEL3		(GPIO_BASE + 0x3024U)
#define	GPIO_FILONOFF3		(GPIO_BASE + 0x3028U)
#define	GPIO_INTMSKS3		(GPIO_BASE + 0x3038U)
#define	GPIO_MSKCLRS3		(GPIO_BASE + 0x303CU)
#define	GPIO_OUTDTSEL3		(GPIO_BASE + 0x3040U)
#define	GPIO_OUTDTH3		(GPIO_BASE + 0x3044U)
#define	GPIO_OUTDTL3		(GPIO_BASE + 0x3048U)
#define	GPIO_BOTHEDGE3		(GPIO_BASE + 0x304CU)
#define	GPIO_IOINTSEL4		(GPIO_BASE + 0x4000U)
#define	GPIO_INOUTSEL4		(GPIO_BASE + 0x4004U)
#define	GPIO_OUTDT4			(GPIO_BASE + 0x4008U)
#define	GPIO_INDT4			(GPIO_BASE + 0x400CU)
#define	GPIO_INTDT4			(GPIO_BASE + 0x4010U)
#define	GPIO_INTCLR4		(GPIO_BASE + 0x4014U)
#define	GPIO_INTMSK4		(GPIO_BASE + 0x4018U)
#define	GPIO_MSKCLR4		(GPIO_BASE + 0x401CU)
#define	GPIO_POSNEG4		(GPIO_BASE + 0x4020U)
#define	GPIO_EDGLEVEL4		(GPIO_BASE + 0x4024U)
#define	GPIO_FILONOFF4		(GPIO_BASE + 0x4028U)
#define	GPIO_INTMSKS4		(GPIO_BASE + 0x4038U)
#define	GPIO_MSKCLRS4		(GPIO_BASE + 0x403CU)
#define	GPIO_OUTDTSEL4		(GPIO_BASE + 0x4040U)
#define	GPIO_OUTDTH4		(GPIO_BASE + 0x4044U)
#define	GPIO_OUTDTL4		(GPIO_BASE + 0x4048U)
#define	GPIO_BOTHEDGE4		(GPIO_BASE + 0x404CU)
#define	GPIO_IOINTSEL5		(GPIO_BASE + 0x5000U)
#define	GPIO_INOUTSEL5		(GPIO_BASE + 0x5004U)
#define	GPIO_OUTDT5			(GPIO_BASE + 0x5008U)
#define	GPIO_INDT5			(GPIO_BASE + 0x500CU)
#define	GPIO_INTDT5			(GPIO_BASE + 0x5010U)
#define	GPIO_INTCLR5		(GPIO_BASE + 0x5014U)
#define	GPIO_INTMSK5		(GPIO_BASE + 0x5018U)
#define	GPIO_MSKCLR5		(GPIO_BASE + 0x501CU)
#define	GPIO_POSNEG5		(GPIO_BASE + 0x5020U)
#define	GPIO_EDGLEVEL5		(GPIO_BASE + 0x5024U)
#define	GPIO_FILONOFF5		(GPIO_BASE + 0x5028U)
#define	GPIO_INTMSKS5		(GPIO_BASE + 0x5038U)
#define	GPIO_MSKCLRS5		(GPIO_BASE + 0x503CU)
#define	GPIO_OUTDTSEL5		(GPIO_BASE + 0x5040U)
#define	GPIO_OUTDTH5		(GPIO_BASE + 0x5044U)
#define	GPIO_OUTDTL5		(GPIO_BASE + 0x5048U)
#define	GPIO_BOTHEDGE5		(GPIO_BASE + 0x504CU)

/* Pin functon base address */
#define	PFC_BASE		(0xE6060000U)

/* Pin functon registers */
#define	PFC_PMMR		(PFC_BASE + 0x0000U)
#define PFC_GPSR0		(PFC_BASE + 0x0100U)
#define PFC_GPSR1		(PFC_BASE + 0x0104U)
#define PFC_GPSR2		(PFC_BASE + 0x0108U)
#define PFC_GPSR3		(PFC_BASE + 0x010CU)
#define PFC_GPSR4		(PFC_BASE + 0x0110U)
#define	PFC_GPSR5		(PFC_BASE + 0x0114U)
#define	PFC_IPSR0		(PFC_BASE + 0x0200U)
#define	PFC_IPSR1		(PFC_BASE + 0x0204U)
#define	PFC_IPSR2		(PFC_BASE + 0x0208U)
#define	PFC_IPSR3		(PFC_BASE + 0x020CU)
#define	PFC_IPSR4		(PFC_BASE + 0x0210U)
#define	PFC_IPSR5		(PFC_BASE + 0x0214U)
#define	PFC_IPSR6		(PFC_BASE + 0x0218U)
#define	PFC_IPSR7		(PFC_BASE + 0x021CU)
#define	PFC_IPSR8		(PFC_BASE + 0x0220U)
#define	PFC_IOCTRL30	(PFC_BASE + 0x0380U)
#define	PFC_IOCTRL31	(PFC_BASE + 0x0384U)
#define	PFC_IOCTRL32	(PFC_BASE + 0x0388U)
#define	PFC_IOCTRL40	(PFC_BASE + 0x03C0U)
#define	PFC_PUEN0		(PFC_BASE + 0x0400U)
#define	PFC_PUEN1		(PFC_BASE + 0x0404U)
#define	PFC_PUEN2		(PFC_BASE + 0x0408U)
#define	PFC_PUEN3		(PFC_BASE + 0x040CU)
#define	PFC_PUD0		(PFC_BASE + 0x0440U)
#define	PFC_PUD1		(PFC_BASE + 0x0444U)
#define	PFC_PUD2		(PFC_BASE + 0x0448U)
#define	PFC_PUD3		(PFC_BASE + 0x044CU)
#define	PFC_MOD_SEL0	(PFC_BASE + 0x0500U)

/* Pin functon bit */
#define GPSR0_DU_EXODDF_DU_ODDF_DISP_CDE			((uint32_t)1U << 21U)
#define GPSR0_DU_EXVSYNC_DU_VSYNC					((uint32_t)1U << 20U)
#define GPSR0_DU_EXHSYNC_DU_HSYNC					((uint32_t)1U << 19U)
#define GPSR0_DU_DOTCLKOUT							((uint32_t)1U << 18U)
#define GPSR0_DU_DB7								((uint32_t)1U << 17U)
#define GPSR0_DU_DB6								((uint32_t)1U << 16U)
#define GPSR0_DU_DB5								((uint32_t)1U << 15U)
#define GPSR0_DU_DB4								((uint32_t)1U << 14U)
#define GPSR0_DU_DB3								((uint32_t)1U << 13U)
#define GPSR0_DU_DB2								((uint32_t)1U << 12U)
#define GPSR0_DU_DG7								((uint32_t)1U << 11U)
#define GPSR0_DU_DG6								((uint32_t)1U << 10U)
#define GPSR0_DU_DG5								((uint32_t)1U << 9U)
#define GPSR0_DU_DG4								((uint32_t)1U << 8U)
#define GPSR0_DU_DG3								((uint32_t)1U << 7U)
#define GPSR0_DU_DG2								((uint32_t)1U << 6U)
#define GPSR0_DU_DR7								((uint32_t)1U << 5U)
#define GPSR0_DU_DR6								((uint32_t)1U << 4U)
#define GPSR0_DU_DR5								((uint32_t)1U << 3U)
#define GPSR0_DU_DR4								((uint32_t)1U << 2U)
#define GPSR0_DU_DR3								((uint32_t)1U << 1U)
#define GPSR0_DU_DR2								((uint32_t)1U << 0U)

#define GPSR1_DIGRF_CLKOUT							((uint32_t)1U << 27U)
#define GPSR1_DIGRF_CLKIN							((uint32_t)1U << 26U)
#define GPSR1_CANFD_CLK								((uint32_t)1U << 25U)
#define GPSR1_CANFD1_RX								((uint32_t)1U << 24U)
#define GPSR1_CANFD1_TX								((uint32_t)1U << 23U)
#define GPSR1_CANFD0_RX								((uint32_t)1U << 22U)
#define GPSR1_CANFD0_TX								((uint32_t)1U << 21U)
#define GPSR1_AVB0_AVTP_CAPTURE						((uint32_t)1U << 20U)
#define GPSR1_AVB0_AVTP_MATCH						((uint32_t)1U << 19U)
#define GPSR1_AVB0_LINK								((uint32_t)1U << 18U)
#define GPSR1_AVB0_PHY_INT							((uint32_t)1U << 17U)
#define GPSR1_AVB0_MAGIC							((uint32_t)1U << 16U)
#define GPSR1_AVB0_MDC								((uint32_t)1U << 15U)
#define GPSR1_AVB0_MDIO								((uint32_t)1U << 14U)
#define GPSR1_AVB0_TXCREFCLK						((uint32_t)1U << 13U)
#define GPSR1_AVB0_TD3								((uint32_t)1U << 12U)
#define GPSR1_AVB0_TD2								((uint32_t)1U << 11U)
#define GPSR1_AVB0_TD1								((uint32_t)1U << 10U)
#define GPSR1_AVB0_TD0								((uint32_t)1U << 9U)
#define GPSR1_AVB0_TXC								((uint32_t)1U << 8U)
#define GPSR1_AVB0_TX_CTL							((uint32_t)1U << 7U)
#define GPSR1_AVB0_RD3								((uint32_t)1U << 6U)
#define GPSR1_AVB0_RD2								((uint32_t)1U << 5U)
#define GPSR1_AVB0_RD1								((uint32_t)1U << 4U)
#define GPSR1_AVB0_RD0								((uint32_t)1U << 3U)
#define GPSR1_AVB0_RXC								((uint32_t)1U << 2U)
#define GPSR1_AVB0_RX_CTL							((uint32_t)1U << 1U)
#define GPSR1_IRQ0									((uint32_t)1U << 0U)

#define GPSR2_VI0_FIELD								((uint32_t)1U << 16U)
#define GPSR2_VI0_DATA11							((uint32_t)1U << 15U)
#define GPSR2_VI0_DATA10							((uint32_t)1U << 14U)
#define GPSR2_VI0_DATA9								((uint32_t)1U << 13U)
#define GPSR2_VI0_DATA8								((uint32_t)1U << 12U)
#define GPSR2_VI0_DATA7								((uint32_t)1U << 11U)
#define GPSR2_VI0_DATA6								((uint32_t)1U << 10U)
#define GPSR2_VI0_DATA5								((uint32_t)1U << 9U)
#define GPSR2_VI0_DATA4								((uint32_t)1U << 8U)
#define GPSR2_VI0_DATA3								((uint32_t)1U << 7U)
#define GPSR2_VI0_DATA2								((uint32_t)1U << 6U)
#define GPSR2_VI0_DATA1								((uint32_t)1U << 5U)
#define GPSR2_VI0_DATA0								((uint32_t)1U << 4U)
#define GPSR2_VI0_VSYNC_N							((uint32_t)1U << 3U)
#define GPSR2_VI0_HSYNC_N							((uint32_t)1U << 2U)
#define GPSR2_VI0_CLKENB							((uint32_t)1U << 1U)
#define GPSR2_VI0_CLK								((uint32_t)1U << 0U)

#define GPSR3_VI1_FIELD								((uint32_t)1U << 16U)
#define GPSR3_VI1_DATA11							((uint32_t)1U << 15U)
#define GPSR3_VI1_DATA10							((uint32_t)1U << 14U)
#define GPSR3_VI1_DATA9								((uint32_t)1U << 13U)
#define GPSR3_VI1_DATA8								((uint32_t)1U << 12U)
#define GPSR3_VI1_DATA7								((uint32_t)1U << 11U)
#define GPSR3_VI1_DATA6								((uint32_t)1U << 10U)
#define GPSR3_VI1_DATA5								((uint32_t)1U << 9U)
#define GPSR3_VI1_DATA4								((uint32_t)1U << 8U)
#define GPSR3_VI1_DATA3								((uint32_t)1U << 7U)
#define GPSR3_VI1_DATA2								((uint32_t)1U << 6U)
#define GPSR3_VI1_DATA1								((uint32_t)1U << 5U)
#define GPSR3_VI1_DATA0								((uint32_t)1U << 4U)
#define GPSR3_VI1_VSYNC_N							((uint32_t)1U << 3U)
#define GPSR3_VI1_HSYNC_N							((uint32_t)1U << 2U)
#define GPSR3_VI1_CLKENB							((uint32_t)1U << 1U)
#define GPSR3_VI1_CLK								((uint32_t)1U << 0U)

#define GPSR4_SDA2									((uint32_t)1U << 5U)
#define GPSR4_SCL2									((uint32_t)1U << 4U)
#define GPSR4_SDA1									((uint32_t)1U << 3U)
#define GPSR4_SCL1									((uint32_t)1U << 2U)
#define GPSR4_SDA0									((uint32_t)1U << 1U)
#define GPSR4_SCL0									((uint32_t)1U << 0U)

#define GPSR5_RPC_INT_N								((uint32_t)1U << 14U)
#define GPSR5_RPC_WP_N								((uint32_t)1U << 13U)
#define GPSR5_RPC_RESET_N							((uint32_t)1U << 12U)
#define GPSR5_QSPI1_SSL								((uint32_t)1U << 11U)
#define GPSR5_QSPI1_IO3								((uint32_t)1U << 10U)
#define GPSR5_QSPI1_IO2								((uint32_t)1U << 9U)
#define GPSR5_QSPI1_MISO_IO1						((uint32_t)1U << 8U)
#define GPSR5_QSPI1_MOSI_IO0						((uint32_t)1U << 7U)
#define GPSR5_QSPI1_SPCLK							((uint32_t)1U << 6U)
#define GPSR5_QSPI0_SSL								((uint32_t)1U << 5U)
#define GPSR5_QSPI0_IO3								((uint32_t)1U << 4U)
#define GPSR5_QSPI0_IO2								((uint32_t)1U << 3U)
#define GPSR5_QSPI0_MISO_IO1						((uint32_t)1U << 2U)
#define GPSR5_QSPI0_MOSI_IO0						((uint32_t)1U << 1U)
#define GPSR5_QSPI0_SPCLK							((uint32_t)1U << 0U)

#define	IPSR_28_FUNC(x)								((uint32_t)(x) << 28U)
#define	IPSR_24_FUNC(x)								((uint32_t)(x) << 24U)
#define	IPSR_20_FUNC(x)								((uint32_t)(x) << 20U)
#define	IPSR_16_FUNC(x)								((uint32_t)(x) << 16U)
#define	IPSR_12_FUNC(x)								((uint32_t)(x) << 12U)
#define	IPSR_8_FUNC(x)								((uint32_t)(x) << 8U)
#define	IPSR_4_FUNC(x)								((uint32_t)(x) << 4U)
#define	IPSR_0_FUNC(x)								((uint32_t)(x) << 0U)

#define IOCTRL30_POC_VI0_DATA5						((uint32_t)1U << 31U)
#define IOCTRL30_POC_VI0_DATA4						((uint32_t)1U << 30U)
#define IOCTRL30_POC_VI0_DATA3						((uint32_t)1U << 29U)
#define IOCTRL30_POC_VI0_DATA2						((uint32_t)1U << 28U)
#define IOCTRL30_POC_VI0_DATA1						((uint32_t)1U << 27U)
#define IOCTRL30_POC_VI0_DATA0						((uint32_t)1U << 26U)
#define IOCTRL30_POC_VI0_VSYNC_N					((uint32_t)1U << 25U)
#define IOCTRL30_POC_VI0_HSYNC_N					((uint32_t)1U << 24U)
#define IOCTRL30_POC_VI0_CLKENB						((uint32_t)1U << 23U)
#define IOCTRL30_POC_VI0_CLK						((uint32_t)1U << 22U)
#define IOCTRL30_POC_DU_EXODDF_DU_ODDF_DISP_CDE		((uint32_t)1U << 21U)
#define IOCTRL30_POC_DU_EXVSYNC_DU_VSYNC			((uint32_t)1U << 20U)
#define IOCTRL30_POC_DU_EXHSYNC_DU_HSYNC			((uint32_t)1U << 19U)
#define IOCTRL30_POC_DU_DOTCLKOUT					((uint32_t)1U << 18U)
#define IOCTRL30_POC_DU_DB7							((uint32_t)1U << 17U)
#define IOCTRL30_POC_DU_DB6							((uint32_t)1U << 16U)
#define IOCTRL30_POC_DU_DB5							((uint32_t)1U << 15U)
#define IOCTRL30_POC_DU_DB4							((uint32_t)1U << 14U)
#define IOCTRL30_POC_DU_DB3							((uint32_t)1U << 13U)
#define IOCTRL30_POC_DU_DB2							((uint32_t)1U << 12U)
#define IOCTRL30_POC_DU_DG7							((uint32_t)1U << 11U)
#define IOCTRL30_POC_DU_DG6							((uint32_t)1U << 10U)
#define IOCTRL30_POC_DU_DG5							((uint32_t)1U << 9U)
#define IOCTRL30_POC_DU_DG4							((uint32_t)1U << 8U)
#define IOCTRL30_POC_DU_DG3							((uint32_t)1U << 7U)
#define IOCTRL30_POC_DU_DG2							((uint32_t)1U << 6U)
#define IOCTRL30_POC_DU_DR7							((uint32_t)1U << 5U)
#define IOCTRL30_POC_DU_DR6							((uint32_t)1U << 4U)
#define IOCTRL30_POC_DU_DR5							((uint32_t)1U << 3U)
#define IOCTRL30_POC_DU_DR4							((uint32_t)1U << 2U)
#define IOCTRL30_POC_DU_DR3							((uint32_t)1U << 1U)
#define IOCTRL30_POC_DU_DR2							((uint32_t)1U << 0U)

#define IOCTRL31_POC_DUMMY_31						((uint32_t)1U << 31U)
#define IOCTRL31_POC_DUMMY_30						((uint32_t)1U << 30U)
#define IOCTRL31_POC_DUMMY_29						((uint32_t)1U << 29U)
#define IOCTRL31_POC_DUMMY_28						((uint32_t)1U << 28U)
#define IOCTRL31_POC_DUMMY_27						((uint32_t)1U << 27U)
#define IOCTRL31_POC_DUMMY_26						((uint32_t)1U << 26U)
#define IOCTRL31_POC_DUMMY_25						((uint32_t)1U << 25U)
#define IOCTRL31_POC_DUMMY_24						((uint32_t)1U << 24U)
#define IOCTRL31_POC_VI1_FIELD						((uint32_t)1U << 23U)
#define IOCTRL31_POC_VI1_DATA11						((uint32_t)1U << 22U)
#define IOCTRL31_POC_VI1_DATA10						((uint32_t)1U << 21U)
#define IOCTRL31_POC_VI1_DATA9						((uint32_t)1U << 20U)
#define IOCTRL31_POC_VI1_DATA8						((uint32_t)1U << 19U)
#define IOCTRL31_POC_VI1_DATA7						((uint32_t)1U << 18U)
#define IOCTRL31_POC_VI1_DATA6						((uint32_t)1U << 17U)
#define IOCTRL31_POC_VI1_DATA5						((uint32_t)1U << 16U)
#define IOCTRL31_POC_VI1_DATA4						((uint32_t)1U << 15U)
#define IOCTRL31_POC_VI1_DATA3						((uint32_t)1U << 14U)
#define IOCTRL31_POC_VI1_DATA2						((uint32_t)1U << 13U)
#define IOCTRL31_POC_VI1_DATA1						((uint32_t)1U << 12U)
#define IOCTRL31_POC_VI1_DATA0						((uint32_t)1U << 11U)
#define IOCTRL31_POC_VI1_VSYNC_N					((uint32_t)1U << 10U)
#define IOCTRL31_POC_VI1_HSYNC_N					((uint32_t)1U << 9U)
#define IOCTRL31_POC_VI1_CLKENB						((uint32_t)1U << 8U)
#define IOCTRL31_POC_VI1_CLK						((uint32_t)1U << 7U)
#define IOCTRL31_POC_VI0_FIELD						((uint32_t)1U << 6U)
#define IOCTRL31_POC_VI0_DATA11						((uint32_t)1U << 5U)
#define IOCTRL31_POC_VI0_DATA10						((uint32_t)1U << 4U)
#define IOCTRL31_POC_VI0_DATA9						((uint32_t)1U << 3U)
#define IOCTRL31_POC_VI0_DATA8						((uint32_t)1U << 2U)
#define IOCTRL31_POC_VI0_DATA7						((uint32_t)1U << 1U)
#define IOCTRL31_POC_VI0_DATA6						((uint32_t)1U << 0U)
#define IOCTRL32_POC2_VREF							((uint32_t)1U << 0U)
#define IOCTRL40_SD0TDSEL1							((uint32_t)1U << 1U)
#define IOCTRL40_SD0TDSEL0							((uint32_t)1U << 0U)

#define PUEN0_PUEN_VI0_CLK							((uint32_t)1U << 31U)
#define PUEN0_PUEN_TDI								((uint32_t)1U << 30U)
#define PUEN0_PUEN_TMS								((uint32_t)1U << 29U)
#define PUEN0_PUEN_TCK								((uint32_t)1U << 28U)
#define PUEN0_PUEN_TRST_N							((uint32_t)1U << 27U)
#define PUEN0_PUEN_IRQ0								((uint32_t)1U << 26U)
#define PUEN0_PUEN_FSCLKST_N						((uint32_t)1U << 25U)
#define PUEN0_PUEN_EXTALR							((uint32_t)1U << 24U)
#define PUEN0_PUEN_PRESETOUT_N						((uint32_t)1U << 23U)
#define PUEN0_PUEN_DU_DOTCLKIN						((uint32_t)1U << 22U)
#define PUEN0_PUEN_DU_EXODDF_DU_ODDF_DISP_CDE		((uint32_t)1U << 21U)
#define PUEN0_PUEN_DU_EXVSYNC_DU_VSYNC				((uint32_t)1U << 20U)
#define PUEN0_PUEN_DU_EXHSYNC_DU_HSYNC				((uint32_t)1U << 19U)
#define PUEN0_PUEN_DU_DOTCLKOUT						((uint32_t)1U << 18U)
#define PUEN0_PUEN_DU_DB7							((uint32_t)1U << 17U)
#define PUEN0_PUEN_DU_DB6							((uint32_t)1U << 16U)
#define PUEN0_PUEN_DU_DB5							((uint32_t)1U << 15U)
#define PUEN0_PUEN_DU_DB4							((uint32_t)1U << 14U)
#define PUEN0_PUEN_DU_DB3							((uint32_t)1U << 13U)
#define PUEN0_PUEN_DU_DB2							((uint32_t)1U << 12U)
#define PUEN0_PUEN_DU_DG7							((uint32_t)1U << 11U)
#define PUEN0_PUEN_DU_DG6							((uint32_t)1U << 10U)
#define PUEN0_PUEN_DU_DG5							((uint32_t)1U << 9U)
#define PUEN0_PUEN_DU_DG4							((uint32_t)1U << 8U)
#define PUEN0_PUEN_DU_DG3							((uint32_t)1U << 7U)
#define PUEN0_PUEN_DU_DG2							((uint32_t)1U << 6U)
#define PUEN0_PUEN_DU_DR7							((uint32_t)1U << 5U)
#define PUEN0_PUEN_DU_DR6							((uint32_t)1U << 4U)
#define PUEN0_PUEN_DU_DR5							((uint32_t)1U << 3U)
#define PUEN0_PUEN_DU_DR4							((uint32_t)1U << 2U)
#define PUEN0_PUEN_DU_DR3							((uint32_t)1U << 1U)
#define PUEN0_PUEN_DU_DR2							((uint32_t)1U << 0U)

#define PUEN1_PUEN_VI1_DATA11						((uint32_t)1U << 31U)
#define PUEN1_PUEN_VI1_DATA10						((uint32_t)1U << 30U)
#define PUEN1_PUEN_VI1_DATA9						((uint32_t)1U << 29U)
#define PUEN1_PUEN_VI1_DATA8						((uint32_t)1U << 28U)
#define PUEN1_PUEN_VI1_DATA7						((uint32_t)1U << 27U)
#define PUEN1_PUEN_VI1_DATA6						((uint32_t)1U << 26U)
#define PUEN1_PUEN_VI1_DATA5						((uint32_t)1U << 25U)
#define PUEN1_PUEN_VI1_DATA4						((uint32_t)1U << 24U)
#define PUEN1_PUEN_VI1_DATA3						((uint32_t)1U << 23U)
#define PUEN1_PUEN_VI1_DATA2						((uint32_t)1U << 22U)
#define PUEN1_PUEN_VI1_DATA1						((uint32_t)1U << 21U)
#define PUEN1_PUEN_VI1_DATA0						((uint32_t)1U << 20U)
#define PUEN1_PUEN_VI1_VSYNC_N						((uint32_t)1U << 19U)
#define PUEN1_PUEN_VI1_HSYNC_N						((uint32_t)1U << 18U)
#define PUEN1_PUEN_VI1_CLKENB						((uint32_t)1U << 17U)
#define PUEN1_PUEN_VI1_CLK							((uint32_t)1U << 16U)
#define PUEN1_PUEN_VI0_FIELD						((uint32_t)1U << 15U)
#define PUEN1_PUEN_VI0_DATA11						((uint32_t)1U << 14U)
#define PUEN1_PUEN_VI0_DATA10						((uint32_t)1U << 13U)
#define PUEN1_PUEN_VI0_DATA9						((uint32_t)1U << 12U)
#define PUEN1_PUEN_VI0_DATA8						((uint32_t)1U << 11U)
#define PUEN1_PUEN_VI0_DATA7						((uint32_t)1U << 10U)
#define PUEN1_PUEN_VI0_DATA6						((uint32_t)1U << 9U)
#define PUEN1_PUEN_VI0_DATA5						((uint32_t)1U << 8U)
#define PUEN1_PUEN_VI0_DATA4						((uint32_t)1U << 7U)
#define PUEN1_PUEN_VI0_DATA3						((uint32_t)1U << 6U)
#define PUEN1_PUEN_VI0_DATA2						((uint32_t)1U << 5U)
#define PUEN1_PUEN_VI0_DATA1						((uint32_t)1U << 4U)
#define PUEN1_PUEN_VI0_DATA0						((uint32_t)1U << 3U)
#define PUEN1_PUEN_VI0_VSYNC_N						((uint32_t)1U << 2U)
#define PUEN1_PUEN_VI0_HSYNC_N						((uint32_t)1U << 1U)
#define PUEN1_PUEN_VI0_CLKENB						((uint32_t)1U << 0U)

#define PUEN2_PUEN_CANFD_CLK						((uint32_t)1U << 31U)
#define PUEN2_PUEN_CANFD1_RX						((uint32_t)1U << 30U)
#define PUEN2_PUEN_CANFD1_TX						((uint32_t)1U << 29U)
#define PUEN2_PUEN_CANFD0_RX						((uint32_t)1U << 28U)
#define PUEN2_PUEN_CANFD0_TX						((uint32_t)1U << 27U)
#define PUEN2_PUEN_AVB0_AVTP_CAPTURE				((uint32_t)1U << 26U)
#define PUEN2_PUEN_AVB0_AVTP_MATCH					((uint32_t)1U << 25U)
#define PUEN2_PUEN_AVB0_LINK						((uint32_t)1U << 24U)
#define PUEN2_PUEN_AVB0_PHY_INT						((uint32_t)1U << 23U)
#define PUEN2_PUEN_AVB0_MAGIC						((uint32_t)1U << 22U)
#define PUEN2_PUEN_AVB0_MDC							((uint32_t)1U << 21U)
#define PUEN2_PUEN_AVB0_MDIO						((uint32_t)1U << 20U)
#define PUEN2_PUEN_AVB0_TXCREFCLK					((uint32_t)1U << 19U)
#define PUEN2_PUEN_AVB0_TD3							((uint32_t)1U << 18U)
#define PUEN2_PUEN_AVB0_TD2							((uint32_t)1U << 17U)
#define PUEN2_PUEN_AVB0_TD1							((uint32_t)1U << 16U)
#define PUEN2_PUEN_AVB0_TD0							((uint32_t)1U << 15U)
#define PUEN2_PUEN_AVB0_TXC							((uint32_t)1U << 14U)
#define PUEN2_PUEN_AVB0_TX_CTL						((uint32_t)1U << 13U)
#define PUEN2_PUEN_AVB0_RD3							((uint32_t)1U << 12U)
#define PUEN2_PUEN_AVB0_RD2							((uint32_t)1U << 11U)
#define PUEN2_PUEN_AVB0_RD1							((uint32_t)1U << 10U)
#define PUEN2_PUEN_AVB0_RD0							((uint32_t)1U << 9U)
#define PUEN2_PUEN_AVB0_RXC							((uint32_t)1U << 8U)
#define PUEN2_PUEN_AVB0_RX_CTL						((uint32_t)1U << 7U)
#define PUEN2_PUEN_SDA2								((uint32_t)1U << 6U)
#define PUEN2_PUEN_SCL2								((uint32_t)1U << 5U)
#define PUEN2_PUEN_SDA1								((uint32_t)1U << 4U)
#define PUEN2_PUEN_SCL1								((uint32_t)1U << 3U)
#define PUEN2_PUEN_SDA0								((uint32_t)1U << 2U)
#define PUEN2_PUEN_SCL0								((uint32_t)1U << 1U)
#define PUEN2_PUEN_VI1_FIELD						((uint32_t)1U << 0U)

#define PUEN3_PUEN_DIGRF_CLKOUT						((uint32_t)1U << 16U)
#define PUEN3_PUEN_DIGRF_CLKIN						((uint32_t)1U << 15U)
#define PUEN3_PUEN_RPC_INT_N						((uint32_t)1U << 14U)
#define PUEN3_PUEN_RPC_WP_N							((uint32_t)1U << 13U)
#define PUEN3_PUEN_RPC_RESET_N						((uint32_t)1U << 12U)
#define PUEN3_PUEN_QSPI1_SSL						((uint32_t)1U << 11U)
#define PUEN3_PUEN_QSPI1_IO3						((uint32_t)1U << 10U)
#define PUEN3_PUEN_QSPI1_IO2						((uint32_t)1U << 9U)
#define PUEN3_PUEN_QSPI1_MISO_IO1					((uint32_t)1U << 8U)
#define PUEN3_PUEN_QSPI1_MOSI_IO0					((uint32_t)1U << 7U)
#define PUEN3_PUEN_QSPI1_SPCLK						((uint32_t)1U << 6U)
#define PUEN3_PUEN_QSPI0_SSL						((uint32_t)1U << 5U)
#define PUEN3_PUEN_QSPI0_IO3						((uint32_t)1U << 4U)
#define PUEN3_PUEN_QSPI0_IO2						((uint32_t)1U << 3U)
#define PUEN3_PUEN_QSPI0_MISO_IO1					((uint32_t)1U << 2U)
#define PUEN3_PUEN_QSPI0_MOSI_IO0					((uint32_t)1U << 1U)
#define PUEN3_PUEN_QSPI0_SPCLK						((uint32_t)1U << 0U)

#define PUD0_PUD_VI0_CLK							((uint32_t)1U << 31U)
#define PUD0_PUD_IRQ0								((uint32_t)1U << 26U)
#define PUD0_PUD_FSCLKST_N							((uint32_t)1U << 25U)
#define PUD0_PUD_PRESETOUT_N						((uint32_t)1U << 23U)
#define PUD0_PUD_DU_EXODDF_DU_ODDF_DISP_CDE			((uint32_t)1U << 21U)
#define PUD0_PUD_DU_EXVSYNC_DU_VSYNC				((uint32_t)1U << 20U)
#define PUD0_PUD_DU_EXHSYNC_DU_HSYNC				((uint32_t)1U << 19U)
#define PUD0_PUD_DU_DOTCLKOUT						((uint32_t)1U << 18U)
#define PUD0_PUD_DU_DB7								((uint32_t)1U << 17U)
#define PUD0_PUD_DU_DB6								((uint32_t)1U << 16U)
#define PUD0_PUD_DU_DB5								((uint32_t)1U << 15U)
#define PUD0_PUD_DU_DB4								((uint32_t)1U << 14U)
#define PUD0_PUD_DU_DB3								((uint32_t)1U << 13U)
#define PUD0_PUD_DU_DB2								((uint32_t)1U << 12U)
#define PUD0_PUD_DU_DG7								((uint32_t)1U << 11U)
#define PUD0_PUD_DU_DG6								((uint32_t)1U << 10U)
#define PUD0_PUD_DU_DG5								((uint32_t)1U << 9U)
#define PUD0_PUD_DU_DG4								((uint32_t)1U << 8U)
#define PUD0_PUD_DU_DG3								((uint32_t)1U << 7U)
#define PUD0_PUD_DU_DG2								((uint32_t)1U << 6U)
#define PUD0_PUD_DU_DR7								((uint32_t)1U << 5U)
#define PUD0_PUD_DU_DR6								((uint32_t)1U << 4U)
#define PUD0_PUD_DU_DR5								((uint32_t)1U << 3U)
#define PUD0_PUD_DU_DR4								((uint32_t)1U << 2U)
#define PUD0_PUD_DU_DR3								((uint32_t)1U << 1U)
#define PUD0_PUD_DU_DR2								((uint32_t)1U << 0U)

#define PUD1_PUD_VI1_DATA11							((uint32_t)1U << 31U)
#define PUD1_PUD_VI1_DATA10							((uint32_t)1U << 30U)
#define PUD1_PUD_VI1_DATA9 							((uint32_t)1U << 29U)
#define PUD1_PUD_VI1_DATA8 							((uint32_t)1U << 28U)
#define PUD1_PUD_VI1_DATA7 							((uint32_t)1U << 27U)
#define PUD1_PUD_VI1_DATA6 							((uint32_t)1U << 26U)
#define PUD1_PUD_VI1_DATA5 							((uint32_t)1U << 25U)
#define PUD1_PUD_VI1_DATA4 							((uint32_t)1U << 24U)
#define PUD1_PUD_VI1_DATA3 							((uint32_t)1U << 23U)
#define PUD1_PUD_VI1_DATA2 							((uint32_t)1U << 22U)
#define PUD1_PUD_VI1_DATA1 							((uint32_t)1U << 21U)
#define PUD1_PUD_VI1_DATA0 							((uint32_t)1U << 20U)
#define PUD1_PUD_VI1_VSYNC_N						((uint32_t)1U << 19U)
#define PUD1_PUD_VI1_HSYNC_N						((uint32_t)1U << 18U)
#define PUD1_PUD_VI1_CLKENB							((uint32_t)1U << 17U)
#define PUD1_PUD_VI1_CLK							((uint32_t)1U << 16U)
#define PUD1_PUD_VI0_FIELD 							((uint32_t)1U << 15U)
#define PUD1_PUD_VI0_DATA11							((uint32_t)1U << 14U)
#define PUD1_PUD_VI0_DATA10							((uint32_t)1U << 13U)
#define PUD1_PUD_VI0_DATA9 							((uint32_t)1U << 12U)
#define PUD1_PUD_VI0_DATA8 							((uint32_t)1U << 11U)
#define PUD1_PUD_VI0_DATA7 							((uint32_t)1U << 10U)
#define PUD1_PUD_VI0_DATA6 							((uint32_t)1U << 9U)
#define PUD1_PUD_VI0_DATA5 							((uint32_t)1U << 8U)
#define PUD1_PUD_VI0_DATA4 							((uint32_t)1U << 7U)
#define PUD1_PUD_VI0_DATA3 							((uint32_t)1U << 6U)
#define PUD1_PUD_VI0_DATA2 							((uint32_t)1U << 5U)
#define PUD1_PUD_VI0_DATA1 							((uint32_t)1U << 4U)
#define PUD1_PUD_VI0_DATA0 							((uint32_t)1U << 3U)
#define PUD1_PUD_VI0_VSYNC_N						((uint32_t)1U << 2U)
#define PUD1_PUD_VI0_HSYNC_N						((uint32_t)1U << 1U)
#define PUD1_PUD_VI0_CLKENB							((uint32_t)1U << 0U)

#define PUD2_PUD_CANFD_CLK							((uint32_t)1U << 31U)
#define PUD2_PUD_CANFD1_RX							((uint32_t)1U << 30U)
#define PUD2_PUD_CANFD1_TX							((uint32_t)1U << 29U)
#define PUD2_PUD_CANFD0_RX							((uint32_t)1U << 28U)
#define PUD2_PUD_CANFD0_TX							((uint32_t)1U << 27U)
#define PUD2_PUD_AVB0_AVTP_CAPTURE					((uint32_t)1U << 26U)
#define PUD2_PUD_AVB0_AVTP_MATCH					((uint32_t)1U << 25U)
#define PUD2_PUD_AVB0_LINK							((uint32_t)1U << 24U)
#define PUD2_PUD_AVB0_PHY_INT						((uint32_t)1U << 23U)
#define PUD2_PUD_AVB0_MAGIC							((uint32_t)1U << 22U)
#define PUD2_PUD_AVB0_MDC							((uint32_t)1U << 21U)
#define PUD2_PUD_AVB0_MDIO							((uint32_t)1U << 20U)
#define PUD2_PUD_AVB0_TXCREFCLK						((uint32_t)1U << 19U)
#define PUD2_PUD_AVB0_TD3							((uint32_t)1U << 18U)
#define PUD2_PUD_AVB0_TD2							((uint32_t)1U << 17U)
#define PUD2_PUD_AVB0_TD1							((uint32_t)1U << 16U)
#define PUD2_PUD_AVB0_TD0							((uint32_t)1U << 15U)
#define PUD2_PUD_AVB0_TXC							((uint32_t)1U << 14U)
#define PUD2_PUD_AVB0_TX_CTL						((uint32_t)1U << 13U)
#define PUD2_PUD_AVB0_RD3							((uint32_t)1U << 12U)
#define PUD2_PUD_AVB0_RD2							((uint32_t)1U << 11U)
#define PUD2_PUD_AVB0_RD1							((uint32_t)1U << 10U)
#define PUD2_PUD_AVB0_RD0							((uint32_t)1U << 9U)
#define PUD2_PUD_AVB0_RXC							((uint32_t)1U << 8U)
#define PUD2_PUD_AVB0_RX_CTL						((uint32_t)1U << 7U)
#define PUD2_PUD_SDA2								((uint32_t)1U << 6U)
#define PUD2_PUD_SCL2								((uint32_t)1U << 5U)
#define PUD2_PUD_SDA1								((uint32_t)1U << 4U)
#define PUD2_PUD_SCL1								((uint32_t)1U << 3U)
#define PUD2_PUD_SDA0								((uint32_t)1U << 2U)
#define PUD2_PUD_SCL0								((uint32_t)1U << 1U)
#define PUD2_PUD_VI1_FIELD							((uint32_t)1U << 0U)

#define PUD3_PUD_DIGRF_CLKOUT						((uint32_t)1U << 16U)
#define PUD3_PUD_DIGRF_CLKIN						((uint32_t)1U << 15U)
#define PUD3_PUD_RPC_INT_N							((uint32_t)1U << 14U)
#define PUD3_PUD_RPC_WP_N							((uint32_t)1U << 13U)
#define PUD3_PUD_RPC_RESET_N						((uint32_t)1U << 12U)
#define PUD3_PUD_QSPI1_SSL							((uint32_t)1U << 11U)
#define PUD3_PUD_QSPI1_IO3							((uint32_t)1U << 10U)
#define PUD3_PUD_QSPI1_IO2							((uint32_t)1U << 9U)
#define PUD3_PUD_QSPI1_MISO_IO1						((uint32_t)1U << 8U)
#define PUD3_PUD_QSPI1_MOSI_IO0						((uint32_t)1U << 7U)
#define PUD3_PUD_QSPI1_SPCLK						((uint32_t)1U << 6U)
#define PUD3_PUD_QSPI0_SSL							((uint32_t)1U << 5U)
#define PUD3_PUD_QSPI0_IO3							((uint32_t)1U << 4U)
#define PUD3_PUD_QSPI0_IO2							((uint32_t)1U << 3U)
#define PUD3_PUD_QSPI0_MISO_IO1						((uint32_t)1U << 2U)
#define PUD3_PUD_QSPI0_MOSI_IO0						((uint32_t)1U << 1U)
#define PUD3_PUD_QSPI0_SPCLK						((uint32_t)1U << 0U)

#define MOD_SEL0_sel_hscif0							((uint32_t)1U << 10U)
#define MOD_SEL0_sel_scif1							((uint32_t)1U << 9U)
#define MOD_SEL0_sel_canfd0							((uint32_t)1U << 8U)
#define MOD_SEL0_sel_pwm4							((uint32_t)1U << 7U)
#define MOD_SEL0_sel_pwm3							((uint32_t)1U << 6U)
#define MOD_SEL0_sel_pwm2							((uint32_t)1U << 5U)
#define MOD_SEL0_sel_pwm1							((uint32_t)1U << 4U)
#define MOD_SEL0_sel_pwm0							((uint32_t)1U << 3U)
#define MOD_SEL0_sel_rfso							((uint32_t)1U << 2U)
#define MOD_SEL0_sel_rsp							((uint32_t)1U << 1U)
#define MOD_SEL0_sel_tmu							((uint32_t)1U << 0U)

/* SCIF3 Registers for Dummy write */
#define SCIF3_BASE		(0xE6C50000U)
#define SCIF3_SCFCR		(SCIF3_BASE + 0x0018U)
#define SCIF3_SCFDR		(SCIF3_BASE + 0x001CU)
#define SCFCR_DATA		(0x0000U)

/* Realtime module stop control */
#define	CPG_BASE		(0xE6150000U)
#define CPG_MSTPSR0		(CPG_BASE + 0x0030U)
#define CPG_RMSTPCR0		(CPG_BASE + 0x0110U)
#define RMSTPCR0_RTDMAC		(0x00200000U)

/* RT-DMAC Registers */
#define RTDMAC_CH		(0U)		/* choose 0 to 15 */

#define RTDMAC_BASE		(0xFFC10000U)
#define RTDMAC_RDMOR		(RTDMAC_BASE + 0x0060U)
#define RTDMAC_RDMCHCLR		(RTDMAC_BASE + 0x0080U)
#define RTDMAC_RDMSAR(x)	(RTDMAC_BASE + 0x8000U + (0x80U * (x)))
#define RTDMAC_RDMDAR(x)	(RTDMAC_BASE + 0x8004U + (0x80U * (x)))
#define RTDMAC_RDMTCR(x)	(RTDMAC_BASE + 0x8008U + (0x80U * (x)))
#define RTDMAC_RDMCHCR(x)	(RTDMAC_BASE + 0x800CU + (0x80U * (x)))
#define RTDMAC_RDMCHCRB(x)	(RTDMAC_BASE + 0x801CU + (0x80U * (x)))
#define RTDMAC_RDMDPBASE(x)	(RTDMAC_BASE + 0x8050U + (0x80U * (x)))
#define RTDMAC_DESC_BASE	(RTDMAC_BASE + 0xA000U)
#define RTDMAC_DESC_RDMSAR	(RTDMAC_DESC_BASE + 0x0000U)
#define RTDMAC_DESC_RDMDAR	(RTDMAC_DESC_BASE + 0x0004U)
#define RTDMAC_DESC_RDMTCR	(RTDMAC_DESC_BASE + 0x0008U)

#define RDMOR_DME		(0x0001U)	/* DMA Master Enable */
#define RDMCHCR_DPM_INFINITE	(0x30000000U)	/* Infinite repeat mode */
#define RDMCHCR_RPT_TCR		(0x02000000U)	/* enable to update TCR */
#define RDMCHCR_TS_2		(0x00000008U)	/* Word(2byte) units transfer */
#define RDMCHCR_RS_AUTO		(0x00000400U)	/* Auto request */
#define RDMCHCR_DE		(0x00000001U)	/* DMA Enable */
#define RDMCHCRB_DRST		(0x00008000U)	/* Descriptor reset */
#define RDMCHCRB_SLM_256	(0x00000080U)	/* once in 256 clock cycle */
#define RDMDPBASE_SEL_EXT	(0x00000001U)	/* External memory use */

static void pfc_reg_write(uint32_t addr, uint32_t data);
static void StartRtDma0_Descriptor(void);

static void pfc_reg_write(uint32_t addr, uint32_t data)
{
	mmio_write_32(PFC_PMMR, ~data);
	mmio_write_32((uintptr_t)addr, data);
}

static void StartRtDma0_Descriptor(void)
{
	uint32_t reg;

	/* Module stop clear */
	while((mmio_read_32(CPG_MSTPSR0) & RMSTPCR0_RTDMAC) != 0U) {
		reg = mmio_read_32(CPG_RMSTPCR0);
		reg &= ~RMSTPCR0_RTDMAC;
		cpg_write(CPG_RMSTPCR0, reg);
	}

	/* Initialize ch0, Reset Descriptor */
	mmio_write_32(RTDMAC_RDMCHCLR, ((uint32_t)1U << RTDMAC_CH));
	mmio_write_32(RTDMAC_RDMCHCRB(RTDMAC_CH), RDMCHCRB_DRST);

	/* Enable DMA */
	mmio_write_16(RTDMAC_RDMOR, RDMOR_DME);

	/* Set first transfer */
	mmio_write_32(RTDMAC_RDMSAR(RTDMAC_CH), RCAR_PRR);
	mmio_write_32(RTDMAC_RDMDAR(RTDMAC_CH), SCIF3_SCFDR);
	mmio_write_32(RTDMAC_RDMTCR(RTDMAC_CH), 0x00000001U);

	/* Set descriptor */
	mmio_write_32(RTDMAC_DESC_RDMSAR, 0x00000000U);
	mmio_write_32(RTDMAC_DESC_RDMDAR, 0x00000000U);
	mmio_write_32(RTDMAC_DESC_RDMTCR, 0x00200000U);
	mmio_write_32(RTDMAC_RDMCHCRB(RTDMAC_CH), RDMCHCRB_SLM_256);
	mmio_write_32(RTDMAC_RDMDPBASE(RTDMAC_CH), RTDMAC_DESC_BASE
						 | RDMDPBASE_SEL_EXT);

	/* Set transfer parameter, Start transfer */
	mmio_write_32(RTDMAC_RDMCHCR(RTDMAC_CH), RDMCHCR_DPM_INFINITE
					       | RDMCHCR_RPT_TCR
					       | RDMCHCR_TS_2
					       | RDMCHCR_RS_AUTO
					       | RDMCHCR_DE);
}

void pfc_init_v3m(void)
{
	/* Work around for PFC eratta */
	StartRtDma0_Descriptor();

	// pin function
	// md[4:1]!=0000
	/* initialize GPIO/perihperal function select */

	pfc_reg_write(PFC_GPSR0, 0x00000000);

	pfc_reg_write(PFC_GPSR1, GPSR1_CANFD_CLK);

	pfc_reg_write(PFC_GPSR2, 0x00000000);

	pfc_reg_write(PFC_GPSR3, 0x00000000);

	pfc_reg_write(PFC_GPSR4, GPSR4_SDA2
				   | GPSR4_SCL2);

	pfc_reg_write(PFC_GPSR5, GPSR5_QSPI1_SSL
				   | GPSR5_QSPI1_IO3
				   | GPSR5_QSPI1_IO2
				   | GPSR5_QSPI1_MISO_IO1
				   | GPSR5_QSPI1_MOSI_IO0
				   | GPSR5_QSPI1_SPCLK
				   | GPSR5_QSPI0_SSL
				   | GPSR5_QSPI0_IO3
				   | GPSR5_QSPI0_IO2
				   | GPSR5_QSPI0_MISO_IO1
				   | GPSR5_QSPI0_MOSI_IO0
				   | GPSR5_QSPI0_SPCLK);


	/* initialize peripheral function select */
	pfc_reg_write(PFC_IPSR0, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR1, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR2, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR3, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR4, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR5, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR6, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(0)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR7, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(4)
				   | IPSR_20_FUNC(4)
				   | IPSR_16_FUNC(4)
				   | IPSR_12_FUNC(4)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR8, IPSR_28_FUNC(0)
				   | IPSR_24_FUNC(0)
				   | IPSR_20_FUNC(0)
				   | IPSR_16_FUNC(4)
				   | IPSR_12_FUNC(0)
				   | IPSR_8_FUNC(0)
				   | IPSR_4_FUNC(0)
				   | IPSR_0_FUNC(0));

	/* initialize POC Control */

	pfc_reg_write(PFC_IOCTRL30, IOCTRL30_POC_VI0_DATA5
				   | IOCTRL30_POC_VI0_DATA4
				   | IOCTRL30_POC_VI0_DATA3
				   | IOCTRL30_POC_VI0_DATA2
				   | IOCTRL30_POC_VI0_DATA1
				   | IOCTRL30_POC_VI0_DATA0
				   | IOCTRL30_POC_VI0_VSYNC_N
				   | IOCTRL30_POC_VI0_HSYNC_N
				   | IOCTRL30_POC_VI0_CLKENB
				   | IOCTRL30_POC_VI0_CLK
				   | IOCTRL30_POC_DU_EXODDF_DU_ODDF_DISP_CDE
				   | IOCTRL30_POC_DU_EXVSYNC_DU_VSYNC
				   | IOCTRL30_POC_DU_EXHSYNC_DU_HSYNC
				   | IOCTRL30_POC_DU_DOTCLKOUT
				   | IOCTRL30_POC_DU_DB7
				   | IOCTRL30_POC_DU_DB6
				   | IOCTRL30_POC_DU_DB5
				   | IOCTRL30_POC_DU_DB4
				   | IOCTRL30_POC_DU_DB3
				   | IOCTRL30_POC_DU_DB2
				   | IOCTRL30_POC_DU_DG7
				   | IOCTRL30_POC_DU_DG6
				   | IOCTRL30_POC_DU_DG5
				   | IOCTRL30_POC_DU_DG4
				   | IOCTRL30_POC_DU_DG3
				   | IOCTRL30_POC_DU_DG2
				   | IOCTRL30_POC_DU_DR7
				   | IOCTRL30_POC_DU_DR6
				   | IOCTRL30_POC_DU_DR5
				   | IOCTRL30_POC_DU_DR4
				   | IOCTRL30_POC_DU_DR3
				   | IOCTRL30_POC_DU_DR2);

	pfc_reg_write(PFC_IOCTRL31, IOCTRL31_POC_DUMMY_31
				   | IOCTRL31_POC_DUMMY_30
				   | IOCTRL31_POC_DUMMY_29
				   | IOCTRL31_POC_DUMMY_28
				   | IOCTRL31_POC_DUMMY_27
				   | IOCTRL31_POC_DUMMY_26
				   | IOCTRL31_POC_DUMMY_25
				   | IOCTRL31_POC_DUMMY_24
				   | IOCTRL31_POC_VI1_FIELD
				   | IOCTRL31_POC_VI1_DATA11
				   | IOCTRL31_POC_VI1_DATA10
				   | IOCTRL31_POC_VI1_DATA9
				   | IOCTRL31_POC_VI1_DATA8
				   | IOCTRL31_POC_VI1_DATA7
				   | IOCTRL31_POC_VI1_DATA6
				   | IOCTRL31_POC_VI1_DATA5
				   | IOCTRL31_POC_VI1_DATA4
				   | IOCTRL31_POC_VI1_DATA3
				   | IOCTRL31_POC_VI1_DATA2
				   | IOCTRL31_POC_VI1_DATA1
				   | IOCTRL31_POC_VI1_DATA0
				   | IOCTRL31_POC_VI1_VSYNC_N
				   | IOCTRL31_POC_VI1_HSYNC_N
				   | IOCTRL31_POC_VI1_CLKENB
				   | IOCTRL31_POC_VI1_CLK
				   | IOCTRL31_POC_VI0_FIELD
				   | IOCTRL31_POC_VI0_DATA11
				   | IOCTRL31_POC_VI0_DATA10
				   | IOCTRL31_POC_VI0_DATA9
				   | IOCTRL31_POC_VI0_DATA8
				   | IOCTRL31_POC_VI0_DATA7
				   | IOCTRL31_POC_VI0_DATA6);

	pfc_reg_write(PFC_IOCTRL32,0x00000000);

	pfc_reg_write(PFC_IOCTRL40,0x00000000);

	/* initialize Pull enable */
	pfc_reg_write(PFC_PUEN0,PUEN0_PUEN_VI0_CLK
				   | PUEN0_PUEN_TDI
				   | PUEN0_PUEN_TMS
				   | PUEN0_PUEN_TCK
				   | PUEN0_PUEN_TRST_N
				   | PUEN0_PUEN_IRQ0
				   | PUEN0_PUEN_FSCLKST_N
				   | PUEN0_PUEN_DU_EXHSYNC_DU_HSYNC
				   | PUEN0_PUEN_DU_DOTCLKOUT
				   | PUEN0_PUEN_DU_DB7
				   | PUEN0_PUEN_DU_DB6
				   | PUEN0_PUEN_DU_DB5
				   | PUEN0_PUEN_DU_DB4
				   | PUEN0_PUEN_DU_DB3
				   | PUEN0_PUEN_DU_DB2
				   | PUEN0_PUEN_DU_DG7
				   | PUEN0_PUEN_DU_DG6
				   | PUEN0_PUEN_DU_DG5
				   | PUEN0_PUEN_DU_DG4
				   | PUEN0_PUEN_DU_DG3
				   | PUEN0_PUEN_DU_DG2
				   | PUEN0_PUEN_DU_DR7
				   | PUEN0_PUEN_DU_DR6
				   | PUEN0_PUEN_DU_DR5
				   | PUEN0_PUEN_DU_DR4
				   | PUEN0_PUEN_DU_DR3
				   | PUEN0_PUEN_DU_DR2);

	pfc_reg_write(PFC_PUEN1,PUEN1_PUEN_VI1_DATA11
				   | PUEN1_PUEN_VI1_DATA10
				   | PUEN1_PUEN_VI1_DATA9
				   | PUEN1_PUEN_VI1_DATA8
				   | PUEN1_PUEN_VI1_DATA7
				   | PUEN1_PUEN_VI1_DATA6
				   | PUEN1_PUEN_VI1_DATA5
				   | PUEN1_PUEN_VI1_DATA4
				   | PUEN1_PUEN_VI1_DATA3
				   | PUEN1_PUEN_VI1_DATA2
				   | PUEN1_PUEN_VI1_DATA1
				   | PUEN1_PUEN_VI1_DATA0
				   | PUEN1_PUEN_VI1_VSYNC_N
				   | PUEN1_PUEN_VI1_HSYNC_N
				   | PUEN1_PUEN_VI1_CLKENB
				   | PUEN1_PUEN_VI1_CLK
				   | PUEN1_PUEN_VI0_DATA11
				   | PUEN1_PUEN_VI0_DATA10
				   | PUEN1_PUEN_VI0_DATA9
				   | PUEN1_PUEN_VI0_DATA8
				   | PUEN1_PUEN_VI0_DATA7
				   | PUEN1_PUEN_VI0_DATA6
				   | PUEN1_PUEN_VI0_DATA5
				   | PUEN1_PUEN_VI0_DATA4
				   | PUEN1_PUEN_VI0_DATA3
				   | PUEN1_PUEN_VI0_DATA2
				   | PUEN1_PUEN_VI0_DATA1);

	pfc_reg_write(PFC_PUEN2,PUEN2_PUEN_CANFD_CLK
				   | PUEN2_PUEN_CANFD1_RX
				   | PUEN2_PUEN_CANFD1_TX
				   | PUEN2_PUEN_CANFD0_RX
				   | PUEN2_PUEN_CANFD0_TX
				   | PUEN2_PUEN_AVB0_AVTP_CAPTURE
				   | PUEN2_PUEN_AVB0_AVTP_MATCH
				   | PUEN2_PUEN_AVB0_LINK
				   | PUEN2_PUEN_AVB0_PHY_INT
				   | PUEN2_PUEN_AVB0_MAGIC
				   | PUEN2_PUEN_AVB0_TXCREFCLK
				   | PUEN2_PUEN_AVB0_TD3
				   | PUEN2_PUEN_AVB0_TD2
				   | PUEN2_PUEN_AVB0_TD1
				   | PUEN2_PUEN_AVB0_TD0
				   | PUEN2_PUEN_AVB0_TXC
				   | PUEN2_PUEN_AVB0_TX_CTL
				   | PUEN2_PUEN_AVB0_RD3
				   | PUEN2_PUEN_AVB0_RD2
				   | PUEN2_PUEN_AVB0_RD1
				   | PUEN2_PUEN_AVB0_RD0
				   | PUEN2_PUEN_AVB0_RXC
				   | PUEN2_PUEN_AVB0_RX_CTL
				   | PUEN2_PUEN_VI1_FIELD);

	pfc_reg_write(PFC_PUEN3,PUEN3_PUEN_DIGRF_CLKOUT
				   | PUEN3_PUEN_DIGRF_CLKIN);

	/* initialize PUD Control */
	pfc_reg_write(PFC_PUD0,PUD0_PUD_VI0_CLK
				   | PUD0_PUD_IRQ0
				   | PUD0_PUD_FSCLKST_N
				   | PUD0_PUD_DU_EXODDF_DU_ODDF_DISP_CDE
				   | PUD0_PUD_DU_EXVSYNC_DU_VSYNC
				   | PUD0_PUD_DU_EXHSYNC_DU_HSYNC
				   | PUD0_PUD_DU_DOTCLKOUT
				   | PUD0_PUD_DU_DB7
				   | PUD0_PUD_DU_DB6
				   | PUD0_PUD_DU_DB5
				   | PUD0_PUD_DU_DB4
				   | PUD0_PUD_DU_DB3
				   | PUD0_PUD_DU_DB2
				   | PUD0_PUD_DU_DG7
				   | PUD0_PUD_DU_DG6
				   | PUD0_PUD_DU_DG5
				   | PUD0_PUD_DU_DG4
				   | PUD0_PUD_DU_DG3
				   | PUD0_PUD_DU_DG2
				   | PUD0_PUD_DU_DR7
				   | PUD0_PUD_DU_DR6
				   | PUD0_PUD_DU_DR5
				   | PUD0_PUD_DU_DR4
				   | PUD0_PUD_DU_DR3
				   | PUD0_PUD_DU_DR2);

	pfc_reg_write(PFC_PUD1,PUD1_PUD_VI1_DATA11
				   | PUD1_PUD_VI1_DATA10
				   | PUD1_PUD_VI1_DATA9
				   | PUD1_PUD_VI1_DATA8
				   | PUD1_PUD_VI1_DATA7
				   | PUD1_PUD_VI1_DATA6
				   | PUD1_PUD_VI1_DATA5
				   | PUD1_PUD_VI1_DATA4
				   | PUD1_PUD_VI1_DATA3
				   | PUD1_PUD_VI1_DATA2
				   | PUD1_PUD_VI1_DATA1
				   | PUD1_PUD_VI1_DATA0
				   | PUD1_PUD_VI1_VSYNC_N
				   | PUD1_PUD_VI1_HSYNC_N
				   | PUD1_PUD_VI1_CLKENB
				   | PUD1_PUD_VI1_CLK
				   | PUD1_PUD_VI0_DATA11
				   | PUD1_PUD_VI0_DATA10
				   | PUD1_PUD_VI0_DATA9
				   | PUD1_PUD_VI0_DATA8
				   | PUD1_PUD_VI0_DATA7
				   | PUD1_PUD_VI0_DATA6
				   | PUD1_PUD_VI0_DATA5
				   | PUD1_PUD_VI0_DATA4
				   | PUD1_PUD_VI0_DATA3
				   | PUD1_PUD_VI0_DATA2
				   | PUD1_PUD_VI0_DATA1
				   | PUD1_PUD_VI0_DATA0
				   | PUD1_PUD_VI0_VSYNC_N
				   | PUD1_PUD_VI0_HSYNC_N
				   | PUD1_PUD_VI0_CLKENB);

	pfc_reg_write(PFC_PUD2,PUD2_PUD_CANFD_CLK
				   | PUD2_PUD_CANFD1_RX
				   | PUD2_PUD_CANFD1_TX
				   | PUD2_PUD_CANFD0_RX
				   | PUD2_PUD_CANFD0_TX
				   | PUD2_PUD_AVB0_AVTP_CAPTURE
				   | PUD2_PUD_VI1_FIELD);

	pfc_reg_write(PFC_PUD3,PUD3_PUD_DIGRF_CLKOUT
				   | PUD3_PUD_DIGRF_CLKIN);

	/* initialize Module Select */
	pfc_reg_write(PFC_MOD_SEL0,0x00000000);

	// gpio
	/* initialize positive/negative logic select */
	mmio_write_32(GPIO_POSNEG0, 0x00000000U);
	mmio_write_32(GPIO_POSNEG1, 0x00000000U);
	mmio_write_32(GPIO_POSNEG2, 0x00000000U);
	mmio_write_32(GPIO_POSNEG3, 0x00000000U);
	mmio_write_32(GPIO_POSNEG4, 0x00000000U);
	mmio_write_32(GPIO_POSNEG5, 0x00000000U);

	/* initialize general IO/interrupt switching */
	mmio_write_32(GPIO_IOINTSEL0, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL1, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL2, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL3, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL4, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL5, 0x00000000U);

	/* initialize general output register */
	mmio_write_32(GPIO_OUTDT0, 0x00000000U);
	mmio_write_32(GPIO_OUTDT1, 0x00000000U);
	mmio_write_32(GPIO_OUTDT2, 0x00000000U);
	mmio_write_32(GPIO_OUTDT3, 0x00000000U);
	mmio_write_32(GPIO_OUTDT4, 0x00000000U);
	mmio_write_32(GPIO_OUTDT5, 0x00000000U);

	/* initialize general input/output switching */
	mmio_write_32(GPIO_INOUTSEL0, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL1, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL2, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL3, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL4, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL5, 0x00000000U);
}

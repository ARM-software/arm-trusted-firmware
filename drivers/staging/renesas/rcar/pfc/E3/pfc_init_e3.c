/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>		/* for uint32_t */

#include <lib/mmio.h>

#include "pfc_init_e3.h"
#include "rcar_def.h"

/* GPIO base address */
#define	GPIO_BASE		(0xE6050000U)

/* GPIO registers */
#define	GPIO_IOINTSEL0		(GPIO_BASE + 0x0000U)
#define	GPIO_INOUTSEL0		(GPIO_BASE + 0x0004U)
#define	GPIO_OUTDT0		(GPIO_BASE + 0x0008U)
#define	GPIO_INDT0		(GPIO_BASE + 0x000CU)
#define	GPIO_INTDT0		(GPIO_BASE + 0x0010U)
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
#define	GPIO_OUTDT1		(GPIO_BASE + 0x1008U)
#define	GPIO_INDT1		(GPIO_BASE + 0x100CU)
#define	GPIO_INTDT1		(GPIO_BASE + 0x1010U)
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
#define	GPIO_OUTDT2		(GPIO_BASE + 0x2008U)
#define	GPIO_INDT2		(GPIO_BASE + 0x200CU)
#define	GPIO_INTDT2		(GPIO_BASE + 0x2010U)
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
#define	GPIO_OUTDT3		(GPIO_BASE + 0x3008U)
#define	GPIO_INDT3		(GPIO_BASE + 0x300CU)
#define	GPIO_INTDT3		(GPIO_BASE + 0x3010U)
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
#define	GPIO_OUTDT4		(GPIO_BASE + 0x4008U)
#define	GPIO_INDT4		(GPIO_BASE + 0x400CU)
#define	GPIO_INTDT4		(GPIO_BASE + 0x4010U)
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
#define	GPIO_OUTDT5		(GPIO_BASE + 0x5008U)
#define	GPIO_INDT5		(GPIO_BASE + 0x500CU)
#define	GPIO_INTDT5		(GPIO_BASE + 0x5010U)
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
#define	GPIO_IOINTSEL6		(GPIO_BASE + 0x5400U)
#define	GPIO_INOUTSEL6		(GPIO_BASE + 0x5404U)
#define	GPIO_OUTDT6		(GPIO_BASE + 0x5408U)
#define	GPIO_INDT6		(GPIO_BASE + 0x540CU)
#define	GPIO_INTDT6		(GPIO_BASE + 0x5410U)
#define	GPIO_INTCLR6		(GPIO_BASE + 0x5414U)
#define	GPIO_INTMSK6		(GPIO_BASE + 0x5418U)
#define	GPIO_MSKCLR6		(GPIO_BASE + 0x541CU)
#define	GPIO_POSNEG6		(GPIO_BASE + 0x5420U)
#define	GPIO_EDGLEVEL6		(GPIO_BASE + 0x5424U)
#define	GPIO_FILONOFF6		(GPIO_BASE + 0x5428U)
#define	GPIO_INTMSKS6		(GPIO_BASE + 0x5438U)
#define	GPIO_MSKCLRS6		(GPIO_BASE + 0x543CU)
#define	GPIO_OUTDTSEL6		(GPIO_BASE + 0x5440U)
#define	GPIO_OUTDTH6		(GPIO_BASE + 0x5444U)
#define	GPIO_OUTDTL6		(GPIO_BASE + 0x5448U)
#define	GPIO_BOTHEDGE6		(GPIO_BASE + 0x544CU)

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
#define	PFC_GPSR6		(PFC_BASE + 0x0118U)
#define	PFC_IPSR0		(PFC_BASE + 0x0200U)
#define	PFC_IPSR1		(PFC_BASE + 0x0204U)
#define	PFC_IPSR2		(PFC_BASE + 0x0208U)
#define	PFC_IPSR3		(PFC_BASE + 0x020CU)
#define	PFC_IPSR4		(PFC_BASE + 0x0210U)
#define	PFC_IPSR5		(PFC_BASE + 0x0214U)
#define	PFC_IPSR6		(PFC_BASE + 0x0218U)
#define	PFC_IPSR7		(PFC_BASE + 0x021CU)
#define	PFC_IPSR8		(PFC_BASE + 0x0220U)
#define	PFC_IPSR9		(PFC_BASE + 0x0224U)
#define	PFC_IPSR10		(PFC_BASE + 0x0228U)
#define	PFC_IPSR11		(PFC_BASE + 0x022CU)
#define	PFC_IPSR12		(PFC_BASE + 0x0230U)
#define	PFC_IPSR13		(PFC_BASE + 0x0234U)
#define	PFC_IPSR14		(PFC_BASE + 0x0238U)
#define	PFC_IPSR15		(PFC_BASE + 0x023CU)
#define PFC_IOCTRL30		(PFC_BASE + 0x0380U)
#define PFC_IOCTRL32		(PFC_BASE + 0x0388U)
#define PFC_IOCTRL40		(PFC_BASE + 0x03C0U)
#define	PFC_PUEN0		(PFC_BASE + 0x0400U)
#define	PFC_PUEN1		(PFC_BASE + 0x0404U)
#define	PFC_PUEN2		(PFC_BASE + 0x0408U)
#define	PFC_PUEN3		(PFC_BASE + 0x040CU)
#define	PFC_PUEN4		(PFC_BASE + 0x0410U)
#define	PFC_PUEN5		(PFC_BASE + 0x0414U)
#define	PFC_PUD0		(PFC_BASE + 0x0440U)
#define	PFC_PUD1		(PFC_BASE + 0x0444U)
#define	PFC_PUD2		(PFC_BASE + 0x0448U)
#define	PFC_PUD3		(PFC_BASE + 0x044CU)
#define	PFC_PUD4		(PFC_BASE + 0x0450U)
#define	PFC_PUD5		(PFC_BASE + 0x0454U)
#define	PFC_MOD_SEL0		(PFC_BASE + 0x0500U)
#define	PFC_MOD_SEL1		(PFC_BASE + 0x0504U)

#define GPSR0_SDA4		((uint32_t)1U << 17U)
#define GPSR0_SCL4		((uint32_t)1U << 16U)
#define	GPSR0_D15		((uint32_t)1U << 15U)
#define	GPSR0_D14		((uint32_t)1U << 14U)
#define	GPSR0_D13		((uint32_t)1U << 13U)
#define	GPSR0_D12		((uint32_t)1U << 12U)
#define	GPSR0_D11		((uint32_t)1U << 11U)
#define	GPSR0_D10		((uint32_t)1U << 10U)
#define	GPSR0_D9		((uint32_t)1U << 9U)
#define	GPSR0_D8		((uint32_t)1U << 8U)
#define	GPSR0_D7		((uint32_t)1U << 7U)
#define	GPSR0_D6		((uint32_t)1U << 6U)
#define	GPSR0_D5		((uint32_t)1U << 5U)
#define	GPSR0_D4		((uint32_t)1U << 4U)
#define	GPSR0_D3		((uint32_t)1U << 3U)
#define	GPSR0_D2		((uint32_t)1U << 2U)
#define	GPSR0_D1		((uint32_t)1U << 1U)
#define	GPSR0_D0		((uint32_t)1U << 0U)
#define	GPSR1_WE0		((uint32_t)1U << 22U)
#define	GPSR1_CS0		((uint32_t)1U << 21U)
#define	GPSR1_CLKOUT		((uint32_t)1U << 20U)
#define	GPSR1_A19		((uint32_t)1U << 19U)
#define	GPSR1_A18		((uint32_t)1U << 18U)
#define	GPSR1_A17		((uint32_t)1U << 17U)
#define	GPSR1_A16		((uint32_t)1U << 16U)
#define	GPSR1_A15		((uint32_t)1U << 15U)
#define	GPSR1_A14		((uint32_t)1U << 14U)
#define	GPSR1_A13		((uint32_t)1U << 13U)
#define	GPSR1_A12		((uint32_t)1U << 12U)
#define	GPSR1_A11		((uint32_t)1U << 11U)
#define	GPSR1_A10		((uint32_t)1U << 10U)
#define	GPSR1_A9		((uint32_t)1U << 9U)
#define	GPSR1_A8		((uint32_t)1U << 8U)
#define	GPSR1_A7		((uint32_t)1U << 7U)
#define	GPSR1_A6		((uint32_t)1U << 6U)
#define	GPSR1_A5		((uint32_t)1U << 5U)
#define	GPSR1_A4		((uint32_t)1U << 4U)
#define	GPSR1_A3		((uint32_t)1U << 3U)
#define	GPSR1_A2		((uint32_t)1U << 2U)
#define	GPSR1_A1		((uint32_t)1U << 1U)
#define	GPSR1_A0		((uint32_t)1U << 0U)
#define	GPSR2_BIT27_REVERCED 	((uint32_t)1U << 27U)
#define	GPSR2_BIT26_REVERCED 	((uint32_t)1U << 26U)
#define	GPSR2_EX_WAIT0		((uint32_t)1U << 25U)
#define	GPSR2_RD_WR		((uint32_t)1U << 24U)
#define	GPSR2_RD		((uint32_t)1U << 23U)
#define	GPSR2_BS		((uint32_t)1U << 22U)
#define	GPSR2_AVB_PHY_INT	((uint32_t)1U << 21U)
#define GPSR2_AVB_TXCREFCLK	((uint32_t)1U << 20U)
#define	GPSR2_AVB_RD3		((uint32_t)1U << 19U)
#define	GPSR2_AVB_RD2		((uint32_t)1U << 18U)
#define	GPSR2_AVB_RD1		((uint32_t)1U << 17U)
#define	GPSR2_AVB_RD0		((uint32_t)1U << 16U)
#define	GPSR2_AVB_RXC		((uint32_t)1U << 15U)
#define	GPSR2_AVB_RX_CTL	((uint32_t)1U << 14U)
#define GPSR2_RPC_RESET		((uint32_t)1U << 13U)
#define GPSR2_RPC_RPC_INT	((uint32_t)1U << 12U)
#define GPSR2_QSPI1_SSL		((uint32_t)1U << 11U)
#define GPSR2_QSPI1_IO3		((uint32_t)1U << 10U)
#define GPSR2_QSPI1_IO2		((uint32_t)1U << 9U)
#define GPSR2_QSPI1_MISO_IO1	((uint32_t)1U << 8U)
#define GPSR2_QSPI1_MOSI_IO0	((uint32_t)1U << 7U)
#define GPSR2_QSPI1_SPCLK	((uint32_t)1U << 6U)
#define GPSR2_QSPI0_SSL		((uint32_t)1U << 5U)
#define GPSR2_QSPI0_IO3		((uint32_t)1U << 4U)
#define GPSR2_QSPI0_IO2		((uint32_t)1U << 3U)
#define GPSR2_QSPI0_MISO_IO1	((uint32_t)1U << 2U)
#define GPSR2_QSPI0_MOSI_IO0	((uint32_t)1U << 1U)
#define GPSR2_QSPI0_SPCLK	((uint32_t)1U << 0U)
#define	GPSR3_SD1_WP		((uint32_t)1U << 15U)
#define	GPSR3_SD1_CD		((uint32_t)1U << 14U)
#define	GPSR3_SD0_WP		((uint32_t)1U << 13U)
#define	GPSR3_SD0_CD		((uint32_t)1U << 12U)
#define	GPSR3_SD1_DAT3		((uint32_t)1U << 11U)
#define	GPSR3_SD1_DAT2		((uint32_t)1U << 10U)
#define	GPSR3_SD1_DAT1		((uint32_t)1U << 9U)
#define	GPSR3_SD1_DAT0		((uint32_t)1U << 8U)
#define	GPSR3_SD1_CMD		((uint32_t)1U << 7U)
#define	GPSR3_SD1_CLK		((uint32_t)1U << 6U)
#define	GPSR3_SD0_DAT3		((uint32_t)1U << 5U)
#define	GPSR3_SD0_DAT2		((uint32_t)1U << 4U)
#define	GPSR3_SD0_DAT1		((uint32_t)1U << 3U)
#define	GPSR3_SD0_DAT0		((uint32_t)1U << 2U)
#define	GPSR3_SD0_CMD		((uint32_t)1U << 1U)
#define	GPSR3_SD0_CLK		((uint32_t)1U << 0U)
#define	GPSR4_SD3_DS		((uint32_t)1U << 10U)
#define	GPSR4_SD3_DAT7		((uint32_t)1U << 9U)
#define	GPSR4_SD3_DAT6		((uint32_t)1U << 8U)
#define	GPSR4_SD3_DAT5		((uint32_t)1U << 7U)
#define	GPSR4_SD3_DAT4		((uint32_t)1U << 6U)
#define	GPSR4_SD3_DAT3		((uint32_t)1U << 5U)
#define	GPSR4_SD3_DAT2		((uint32_t)1U << 4U)
#define	GPSR4_SD3_DAT1		((uint32_t)1U << 3U)
#define	GPSR4_SD3_DAT0		((uint32_t)1U << 2U)
#define	GPSR4_SD3_CMD		((uint32_t)1U << 1U)
#define	GPSR4_SD3_CLK		((uint32_t)1U << 0U)
#define	GPSR5_MLB_DAT		((uint32_t)1U << 19U)
#define	GPSR5_MLB_SIG		((uint32_t)1U << 18U)
#define	GPSR5_MLB_CLK		((uint32_t)1U << 17U)
#define	GPSR5_SSI_SDATA9	((uint32_t)1U << 16U)
#define	GPSR5_MSIOF0_SS2	((uint32_t)1U << 15U)
#define	GPSR5_MSIOF0_SS1	((uint32_t)1U << 14U)
#define	GPSR5_MSIOF0_SYNC	((uint32_t)1U << 13U)
#define	GPSR5_MSIOF0_TXD	((uint32_t)1U << 12U)
#define	GPSR5_MSIOF0_RXD	((uint32_t)1U << 11U)
#define	GPSR5_MSIOF0_SCK	((uint32_t)1U << 10U)
#define	GPSR5_RX2_A		((uint32_t)1U << 9U)
#define	GPSR5_TX2_A		((uint32_t)1U << 8U)
#define	GPSR5_SCK2_A		((uint32_t)1U << 7U)
#define	GPSR5_TX1		((uint32_t)1U << 6U)
#define	GPSR5_RX1		((uint32_t)1U << 5U)
#define	GPSR5_RTS0_TANS_A	((uint32_t)1U << 4U)
#define	GPSR5_CTS0_A		((uint32_t)1U << 3U)
#define	GPSR5_TX0_A		((uint32_t)1U << 2U)
#define	GPSR5_RX0_A		((uint32_t)1U << 1U)
#define	GPSR5_SCK0_A		((uint32_t)1U << 0U)
#define	GPSR6_USB30_PWEN	((uint32_t)1U << 17U)
#define	GPSR6_SSI_SDATA6	((uint32_t)1U << 16U)
#define	GPSR6_SSI_WS6		((uint32_t)1U << 15U)
#define	GPSR6_SSI_SCK6		((uint32_t)1U << 14U)
#define	GPSR6_SSI_SDATA5	((uint32_t)1U << 13U)
#define	GPSR6_SSI_WS5		((uint32_t)1U << 12U)
#define	GPSR6_SSI_SCK5		((uint32_t)1U << 11U)
#define	GPSR6_SSI_SDATA4	((uint32_t)1U << 10U)
#define	GPSR6_USB30_OVC		((uint32_t)1U << 9U)
#define	GPSR6_AUDIO_CLKA	((uint32_t)1U << 8U)
#define	GPSR6_SSI_SDATA3	((uint32_t)1U << 7U)
#define	GPSR6_SSI_WS349		((uint32_t)1U << 6U)
#define	GPSR6_SSI_SCK349	((uint32_t)1U << 5U)
#define	GPSR6_SSI_SDATA2	((uint32_t)1U << 4U)
#define	GPSR6_SSI_SDATA1	((uint32_t)1U << 3U)
#define	GPSR6_SSI_SDATA0	((uint32_t)1U << 2U)
#define	GPSR6_SSI_WS01239	((uint32_t)1U << 1U)
#define	GPSR6_SSI_SCK01239	((uint32_t)1U << 0U)

#define	IPSR_28_FUNC(x)		((uint32_t)(x) << 28U)
#define	IPSR_24_FUNC(x)		((uint32_t)(x) << 24U)
#define	IPSR_20_FUNC(x)		((uint32_t)(x) << 20U)
#define	IPSR_16_FUNC(x)		((uint32_t)(x) << 16U)
#define	IPSR_12_FUNC(x)		((uint32_t)(x) << 12U)
#define	IPSR_8_FUNC(x)		((uint32_t)(x) << 8U)
#define	IPSR_4_FUNC(x)		((uint32_t)(x) << 4U)
#define	IPSR_0_FUNC(x)		((uint32_t)(x) << 0U)

#define IOCTRL30_MASK		(0x0007F000U)
#define	POC_SD3_DS_33V		((uint32_t)1U << 29U)
#define	POC_SD3_DAT7_33V	((uint32_t)1U << 28U)
#define	POC_SD3_DAT6_33V	((uint32_t)1U << 27U)
#define	POC_SD3_DAT5_33V	((uint32_t)1U << 26U)
#define	POC_SD3_DAT4_33V	((uint32_t)1U << 25U)
#define	POC_SD3_DAT3_33V	((uint32_t)1U << 24U)
#define	POC_SD3_DAT2_33V	((uint32_t)1U << 23U)
#define	POC_SD3_DAT1_33V	((uint32_t)1U << 22U)
#define	POC_SD3_DAT0_33V	((uint32_t)1U << 21U)
#define	POC_SD3_CMD_33V		((uint32_t)1U << 20U)
#define	POC_SD3_CLK_33V		((uint32_t)1U << 19U)
#define	POC_SD1_DAT3_33V	((uint32_t)1U << 11U)
#define	POC_SD1_DAT2_33V	((uint32_t)1U << 10U)
#define	POC_SD1_DAT1_33V	((uint32_t)1U << 9U)
#define	POC_SD1_DAT0_33V	((uint32_t)1U << 8U)
#define	POC_SD1_CMD_33V		((uint32_t)1U << 7U)
#define	POC_SD1_CLK_33V		((uint32_t)1U << 6U)
#define	POC_SD0_DAT3_33V	((uint32_t)1U << 5U)
#define	POC_SD0_DAT2_33V	((uint32_t)1U << 4U)
#define	POC_SD0_DAT1_33V	((uint32_t)1U << 3U)
#define	POC_SD0_DAT0_33V	((uint32_t)1U << 2U)
#define	POC_SD0_CMD_33V		((uint32_t)1U << 1U)
#define	POC_SD0_CLK_33V		((uint32_t)1U << 0U)

#define IOCTRL32_MASK		(0xFFFFFFFEU)
#define POC2_VREF_33V		((uint32_t)1U << 0U)

#define	MOD_SEL0_ADGB_A		((uint32_t)0U << 29U)
#define	MOD_SEL0_ADGB_B		((uint32_t)1U << 29U)
#define	MOD_SEL0_ADGB_C		((uint32_t)2U << 29U)
#define	MOD_SEL0_DRIF0_A	((uint32_t)0U << 28U)
#define	MOD_SEL0_DRIF0_B	((uint32_t)1U << 28U)
#define	MOD_SEL0_FM_A		((uint32_t)0U << 26U)
#define	MOD_SEL0_FM_B		((uint32_t)1U << 26U)
#define	MOD_SEL0_FM_C		((uint32_t)2U << 26U)
#define	MOD_SEL0_FSO_A		((uint32_t)0U << 25U)
#define	MOD_SEL0_FSO_B		((uint32_t)1U << 25U)
#define	MOD_SEL0_HSCIF0_A	((uint32_t)0U << 24U)
#define	MOD_SEL0_HSCIF0_B	((uint32_t)1U << 24U)
#define	MOD_SEL0_HSCIF1_A	((uint32_t)0U << 23U)
#define	MOD_SEL0_HSCIF1_B	((uint32_t)1U << 23U)
#define	MOD_SEL0_HSCIF2_A	((uint32_t)0U << 22U)
#define	MOD_SEL0_HSCIF2_B	((uint32_t)1U << 22U)
#define	MOD_SEL0_I2C1_A		((uint32_t)0U << 20U)
#define	MOD_SEL0_I2C1_B		((uint32_t)1U << 20U)
#define	MOD_SEL0_I2C1_C		((uint32_t)2U << 20U)
#define	MOD_SEL0_I2C1_D		((uint32_t)3U << 20U)
#define	MOD_SEL0_I2C2_A		((uint32_t)0U << 17U)
#define	MOD_SEL0_I2C2_B		((uint32_t)1U << 17U)
#define	MOD_SEL0_I2C2_C		((uint32_t)2U << 17U)
#define	MOD_SEL0_I2C2_D		((uint32_t)3U << 17U)
#define	MOD_SEL0_I2C2_E		((uint32_t)4U << 17U)
#define	MOD_SEL0_NDFC_A		((uint32_t)0U << 16U)
#define	MOD_SEL0_NDFC_B		((uint32_t)1U << 16U)
#define	MOD_SEL0_PWM0_A		((uint32_t)0U << 15U)
#define	MOD_SEL0_PWM0_B		((uint32_t)1U << 15U)
#define	MOD_SEL0_PWM1_A		((uint32_t)0U << 14U)
#define	MOD_SEL0_PWM1_B		((uint32_t)1U << 14U)
#define	MOD_SEL0_PWM2_A		((uint32_t)0U << 12U)
#define	MOD_SEL0_PWM2_B		((uint32_t)1U << 12U)
#define	MOD_SEL0_PWM2_C		((uint32_t)2U << 12U)
#define	MOD_SEL0_PWM3_A		((uint32_t)0U << 10U)
#define	MOD_SEL0_PWM3_B		((uint32_t)1U << 10U)
#define	MOD_SEL0_PWM3_C		((uint32_t)2U << 10U)
#define	MOD_SEL0_PWM4_A		((uint32_t)0U << 9U)
#define	MOD_SEL0_PWM4_B		((uint32_t)1U << 9U)
#define	MOD_SEL0_PWM5_A		((uint32_t)0U << 8U)
#define	MOD_SEL0_PWM5_B		((uint32_t)1U << 8U)
#define	MOD_SEL0_PWM6_A		((uint32_t)0U << 7U)
#define	MOD_SEL0_PWM6_B		((uint32_t)1U << 7U)
#define	MOD_SEL0_REMOCON_A	((uint32_t)0U << 5U)
#define	MOD_SEL0_REMOCON_B	((uint32_t)1U << 5U)
#define	MOD_SEL0_REMOCON_C	((uint32_t)2U << 5U)
#define	MOD_SEL0_SCIF_A		((uint32_t)0U << 4U)
#define	MOD_SEL0_SCIF_B		((uint32_t)1U << 4U)
#define	MOD_SEL0_SCIF0_A	((uint32_t)0U << 3U)
#define	MOD_SEL0_SCIF0_B	((uint32_t)1U << 3U)
#define	MOD_SEL0_SCIF2_A	((uint32_t)0U << 2U)
#define	MOD_SEL0_SCIF2_B	((uint32_t)1U << 2U)
#define	MOD_SEL0_SPEED_PULSE_IF_A	((uint32_t)0U << 0U)
#define	MOD_SEL0_SPEED_PULSE_IF_B	((uint32_t)1U << 0U)
#define	MOD_SEL0_SPEED_PULSE_IF_C	((uint32_t)2U << 0U)
#define	MOD_SEL1_SIMCARD_A	((uint32_t)0U << 31U)
#define	MOD_SEL1_SIMCARD_B	((uint32_t)1U << 31U)
#define	MOD_SEL1_SSI2_A		((uint32_t)0U << 30U)
#define	MOD_SEL1_SSI2_B		((uint32_t)1U << 30U)
#define	MOD_SEL1_TIMER_TMU_A	((uint32_t)0U << 29U)
#define	MOD_SEL1_TIMER_TMU_B	((uint32_t)1U << 29U)
#define MOD_SEL1_USB20_CH0_A	((uint32_t)0U << 28U)
#define MOD_SEL1_USB20_CH0_B	((uint32_t)1U << 28U)
#define	MOD_SEL1_DRIF2_A	((uint32_t)0U << 26U)
#define	MOD_SEL1_DRIF2_B	((uint32_t)1U << 26U)
#define	MOD_SEL1_DRIF3_A	((uint32_t)0U << 25U)
#define	MOD_SEL1_DRIF3_B	((uint32_t)1U << 25U)
#define	MOD_SEL1_HSCIF3_A	((uint32_t)0U << 22U)
#define	MOD_SEL1_HSCIF3_B	((uint32_t)1U << 22U)
#define	MOD_SEL1_HSCIF3_C	((uint32_t)2U << 22U)
#define	MOD_SEL1_HSCIF3_D	((uint32_t)3U << 22U)
#define	MOD_SEL1_HSCIF3_E	((uint32_t)4U << 22U)
#define	MOD_SEL1_HSCIF4_A	((uint32_t)0U << 19U)
#define	MOD_SEL1_HSCIF4_B	((uint32_t)1U << 19U)
#define	MOD_SEL1_HSCIF4_C	((uint32_t)2U << 19U)
#define	MOD_SEL1_HSCIF4_D	((uint32_t)3U << 19U)
#define	MOD_SEL1_HSCIF4_E	((uint32_t)4U << 19U)
#define	MOD_SEL1_I2C6_A		((uint32_t)0U << 18U)
#define	MOD_SEL1_I2C6_B		((uint32_t)1U << 18U)
#define	MOD_SEL1_I2C7_A		((uint32_t)0U << 17U)
#define	MOD_SEL1_I2C7_B		((uint32_t)1U << 17U)
#define	MOD_SEL1_MSIOF2_A	((uint32_t)0U << 16U)
#define	MOD_SEL1_MSIOF2_B	((uint32_t)1U << 16U)
#define	MOD_SEL1_MSIOF3_A	((uint32_t)0U << 15U)
#define	MOD_SEL1_MSIOF3_B	((uint32_t)1U << 15U)
#define	MOD_SEL1_SCIF3_A	((uint32_t)0U << 13U)
#define	MOD_SEL1_SCIF3_B	((uint32_t)1U << 13U)
#define	MOD_SEL1_SCIF3_C	((uint32_t)2U << 13U)
#define	MOD_SEL1_SCIF4_A	((uint32_t)0U << 11U)
#define	MOD_SEL1_SCIF4_B	((uint32_t)1U << 11U)
#define	MOD_SEL1_SCIF4_C	((uint32_t)2U << 11U)
#define	MOD_SEL1_SCIF5_A	((uint32_t)0U << 9U)
#define	MOD_SEL1_SCIF5_B	((uint32_t)1U << 9U)
#define	MOD_SEL1_SCIF5_C	((uint32_t)2U << 9U)
#define	MOD_SEL1_VIN4_A		((uint32_t)0U << 8U)
#define	MOD_SEL1_VIN4_B		((uint32_t)1U << 8U)
#define	MOD_SEL1_VIN5_A		((uint32_t)0U << 7U)
#define	MOD_SEL1_VIN5_B		((uint32_t)1U << 7U)
#define	MOD_SEL1_ADGC_A		((uint32_t)0U << 5U)
#define	MOD_SEL1_ADGC_B		((uint32_t)1U << 5U)
#define	MOD_SEL1_ADGC_C		((uint32_t)2U << 5U)
#define	MOD_SEL1_SSI9_A		((uint32_t)0U << 4U)
#define	MOD_SEL1_SSI9_B		((uint32_t)1U << 4U)

static void pfc_reg_write(uint32_t addr, uint32_t data);

static void pfc_reg_write(uint32_t addr, uint32_t data)
{
	mmio_write_32(PFC_PMMR, ~data);
	mmio_write_32((uintptr_t) addr, data);
}

void pfc_init_e3(void)
{
	uint32_t reg;

	/* initialize module select */
	pfc_reg_write(PFC_MOD_SEL0, MOD_SEL0_ADGB_A
		      | MOD_SEL0_DRIF0_A
		      | MOD_SEL0_FM_A
		      | MOD_SEL0_FSO_A
		      | MOD_SEL0_HSCIF0_A
		      | MOD_SEL0_HSCIF1_A
		      | MOD_SEL0_HSCIF2_A
		      | MOD_SEL0_I2C1_A
		      | MOD_SEL0_I2C2_A
		      | MOD_SEL0_NDFC_A
		      | MOD_SEL0_PWM0_A
		      | MOD_SEL0_PWM1_A
		      | MOD_SEL0_PWM2_A
		      | MOD_SEL0_PWM3_A
		      | MOD_SEL0_PWM4_A
		      | MOD_SEL0_PWM5_A
		      | MOD_SEL0_PWM6_A
		      | MOD_SEL0_REMOCON_A
		      | MOD_SEL0_SCIF_A
		      | MOD_SEL0_SCIF0_A
		      | MOD_SEL0_SCIF2_A
		      | MOD_SEL0_SPEED_PULSE_IF_A);
	pfc_reg_write(PFC_MOD_SEL1, MOD_SEL1_SIMCARD_A
		      | MOD_SEL1_SSI2_A
		      | MOD_SEL1_TIMER_TMU_A
		      | MOD_SEL1_USB20_CH0_B
		      | MOD_SEL1_DRIF2_A
		      | MOD_SEL1_DRIF3_A
		      | MOD_SEL1_HSCIF3_A
		      | MOD_SEL1_HSCIF4_A
		      | MOD_SEL1_I2C6_A
		      | MOD_SEL1_I2C7_A
		      | MOD_SEL1_MSIOF2_A
		      | MOD_SEL1_MSIOF3_A
		      | MOD_SEL1_SCIF3_A
		      | MOD_SEL1_SCIF4_A
		      | MOD_SEL1_SCIF5_A
		      | MOD_SEL1_VIN4_A
		      | MOD_SEL1_VIN5_A
		      | MOD_SEL1_ADGC_A
		      | MOD_SEL1_SSI9_A);

	/* initialize peripheral function select */
	pfc_reg_write(PFC_IPSR0, IPSR_28_FUNC(0)	/* QSPI1_MISO/IO1 */
		      | IPSR_24_FUNC(0)	/* QSPI1_MOSI/IO0 */
		      | IPSR_20_FUNC(0)	/* QSPI1_SPCLK */
		      | IPSR_16_FUNC(0)	/* QSPI0_IO3 */
		      | IPSR_12_FUNC(0)	/* QSPI0_IO2 */
		      | IPSR_8_FUNC(0)	/* QSPI0_MISO/IO1 */
		      | IPSR_4_FUNC(0)	/* QSPI0_MOSI/IO0 */
		      | IPSR_0_FUNC(0));	/* QSPI0_SPCLK */
	pfc_reg_write(PFC_IPSR1, IPSR_28_FUNC(0)	/* AVB_RD2 */
		      | IPSR_24_FUNC(0)	/* AVB_RD1 */
		      | IPSR_20_FUNC(0)	/* AVB_RD0 */
		      | IPSR_16_FUNC(0)	/* RPC_RESET# */
		      | IPSR_12_FUNC(0)	/* RPC_INT# */
		      | IPSR_8_FUNC(0)	/* QSPI1_SSL */
		      | IPSR_4_FUNC(0)	/* QSPI1_IO3 */
		      | IPSR_0_FUNC(0));	/* QSPI1_IO2 */
	pfc_reg_write(PFC_IPSR2, IPSR_28_FUNC(1)	/* IRQ0 */
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(2)	/* AVB_LINK */
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)	/* AVB_MDC */
		      | IPSR_4_FUNC(0)	/* AVB_MDIO */
		      | IPSR_0_FUNC(0));	/* AVB_TXCREFCLK */
	pfc_reg_write(PFC_IPSR3, IPSR_28_FUNC(5)	/* DU_HSYNC */
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(5)	/* DU_DG4 */
		      | IPSR_8_FUNC(5)	/* DU_DOTCLKOUT0 */
		      | IPSR_4_FUNC(5)	/* DU_DISP */
		      | IPSR_0_FUNC(1));	/* IRQ1 */
	pfc_reg_write(PFC_IPSR4, IPSR_28_FUNC(5)	/* DU_DB5 */
		      | IPSR_24_FUNC(5)	/* DU_DB4 */
		      | IPSR_20_FUNC(5)	/* DU_DB3 */
		      | IPSR_16_FUNC(5)	/* DU_DB2 */
		      | IPSR_12_FUNC(5)	/* DU_DG6 */
		      | IPSR_8_FUNC(5)	/* DU_VSYNC */
		      | IPSR_4_FUNC(5)	/* DU_DG5 */
		      | IPSR_0_FUNC(5));	/* DU_DG7 */
	pfc_reg_write(PFC_IPSR5, IPSR_28_FUNC(5)	/* DU_DR3 */
		      | IPSR_24_FUNC(5)	/* DU_DB7 */
		      | IPSR_20_FUNC(5)	/* DU_DR2 */
		      | IPSR_16_FUNC(5)	/* DU_DR1 */
		      | IPSR_12_FUNC(5)	/* DU_DR0 */
		      | IPSR_8_FUNC(5)	/* DU_DB1 */
		      | IPSR_4_FUNC(5)	/* DU_DB0 */
		      | IPSR_0_FUNC(5));	/* DU_DB6 */
	pfc_reg_write(PFC_IPSR6, IPSR_28_FUNC(5)	/* DU_DG1 */
		      | IPSR_24_FUNC(5)	/* DU_DG0 */
		      | IPSR_20_FUNC(5)	/* DU_DR7 */
		      | IPSR_16_FUNC(2)	/* IRQ5 */
		      | IPSR_12_FUNC(5)	/* DU_DR6 */
		      | IPSR_8_FUNC(5)	/* DU_DR5 */
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(5));	/* DU_DR4 */
	pfc_reg_write(PFC_IPSR7, IPSR_28_FUNC(0)	/* SD0_CLK */
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(5)	/* DU_DOTCLKIN0 */
		      | IPSR_16_FUNC(5)	/* DU_DG3 */
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(5));	/* DU_DG2 */
	pfc_reg_write(PFC_IPSR8, IPSR_28_FUNC(0)	/* SD1_DAT0 */
		      | IPSR_24_FUNC(0)	/* SD1_CMD */
		      | IPSR_20_FUNC(0)	/* SD1_CLK */
		      | IPSR_16_FUNC(0)	/* SD0_DAT3 */
		      | IPSR_12_FUNC(0)	/* SD0_DAT2 */
		      | IPSR_8_FUNC(0)	/* SD0_DAT1 */
		      | IPSR_4_FUNC(0)	/* SD0_DAT0 */
		      | IPSR_0_FUNC(0));	/* SD0_CMD */
	pfc_reg_write(PFC_IPSR9, IPSR_28_FUNC(0)	/* SD3_DAT2 */
		      | IPSR_24_FUNC(0)	/* SD3_DAT1 */
		      | IPSR_20_FUNC(0)	/* SD3_DAT0 */
		      | IPSR_16_FUNC(0)	/* SD3_CMD */
		      | IPSR_12_FUNC(0)	/* SD3_CLK */
		      | IPSR_8_FUNC(0)	/* SD1_DAT3 */
		      | IPSR_4_FUNC(0)	/* SD1_DAT2 */
		      | IPSR_0_FUNC(0));	/* SD1_DAT1 */
	pfc_reg_write(PFC_IPSR10, IPSR_28_FUNC(0)	/* SD0_WP */
		      | IPSR_24_FUNC(0)	/* SD0_CD */
		      | IPSR_20_FUNC(0)	/* SD3_DS */
		      | IPSR_16_FUNC(0)	/* SD3_DAT7 */
		      | IPSR_12_FUNC(0)	/* SD3_DAT6 */
		      | IPSR_8_FUNC(0)	/* SD3_DAT5 */
		      | IPSR_4_FUNC(0)	/* SD3_DAT4 */
		      | IPSR_0_FUNC(0));	/* SD3_DAT3 */
	pfc_reg_write(PFC_IPSR11, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(2)	/* AUDIO_CLKOUT1_A */
		      | IPSR_16_FUNC(2)	/* AUDIO_CLKOUT_A */
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)	/* SD1_WP */
		      | IPSR_0_FUNC(0));	/* SD1_CD */
	pfc_reg_write(PFC_IPSR12, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)	/* RX2_A */
		      | IPSR_8_FUNC(0)	/* TX2_A */
		      | IPSR_4_FUNC(2)	/* AUDIO_CLKB_A */
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR13, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(2)	/* AUDIO_CLKC_A */
		      | IPSR_4_FUNC(1)	/* HTX2_A */
		      | IPSR_0_FUNC(1));	/* HRX2_A */
	pfc_reg_write(PFC_IPSR14, IPSR_28_FUNC(3)	/* USB0_PWEN_B */
		      | IPSR_24_FUNC(0)	/* SSI_SDATA4 */
		      | IPSR_20_FUNC(0)	/* SSI_SDATA3 */
		      | IPSR_16_FUNC(0)	/* SSI_WS349 */
		      | IPSR_12_FUNC(0)	/* SSI_SCK349 */
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)	/* SSI_SDATA1 */
		      | IPSR_0_FUNC(0));	/* SSI_SDATA0 */
	pfc_reg_write(PFC_IPSR15, IPSR_28_FUNC(0)	/* USB30_OVC */
		      | IPSR_24_FUNC(0)	/* USB30_PWEN */
		      | IPSR_20_FUNC(0)	/* AUDIO_CLKA */
		      | IPSR_16_FUNC(1)	/* HRTS2#_A */
		      | IPSR_12_FUNC(1)	/* HCTS2#_A */
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(3));	/* USB0_OVC_B */

	/* initialize GPIO/perihperal function select */
	pfc_reg_write(PFC_GPSR0, GPSR0_SCL4
		      | GPSR0_D15
		      | GPSR0_D11
		      | GPSR0_D10
		      | GPSR0_D9
		      | GPSR0_D8
		      | GPSR0_D7
		      | GPSR0_D6
		      | GPSR0_D5
		      | GPSR0_D3
		      | GPSR0_D2
		      | GPSR0_D1
		      | GPSR0_D0);
	pfc_reg_write(PFC_GPSR1, GPSR1_WE0
		      | GPSR1_CS0
		      | GPSR1_A19
		      | GPSR1_A18
		      | GPSR1_A17
		      | GPSR1_A16
		      | GPSR1_A15
		      | GPSR1_A14
		      | GPSR1_A13
		      | GPSR1_A12
		      | GPSR1_A11
		      | GPSR1_A10
		      | GPSR1_A9
		      | GPSR1_A8
		      | GPSR1_A4
		      | GPSR1_A3
		      | GPSR1_A2
		      | GPSR1_A1
		      | GPSR1_A0);
	pfc_reg_write(PFC_GPSR2, GPSR2_BIT27_REVERCED
		      | GPSR2_BIT26_REVERCED
		      | GPSR2_RD
		      | GPSR2_AVB_PHY_INT
		      | GPSR2_AVB_TXCREFCLK
		      | GPSR2_AVB_RD3
		      | GPSR2_AVB_RD2
		      | GPSR2_AVB_RD1
		      | GPSR2_AVB_RD0
		      | GPSR2_AVB_RXC
		      | GPSR2_AVB_RX_CTL
		      | GPSR2_RPC_RESET
		      | GPSR2_RPC_RPC_INT
		      | GPSR2_QSPI1_SSL
		      | GPSR2_QSPI1_IO3
		      | GPSR2_QSPI1_IO2
		      | GPSR2_QSPI1_MISO_IO1
		      | GPSR2_QSPI1_MOSI_IO0
		      | GPSR2_QSPI1_SPCLK
		      | GPSR2_QSPI0_SSL
		      | GPSR2_QSPI0_IO3
		      | GPSR2_QSPI0_IO2
		      | GPSR2_QSPI0_MISO_IO1
		      | GPSR2_QSPI0_MOSI_IO0
		      | GPSR2_QSPI0_SPCLK);
	pfc_reg_write(PFC_GPSR3, GPSR3_SD1_WP
		      | GPSR3_SD1_CD
		      | GPSR3_SD0_WP
		      | GPSR3_SD0_CD
		      | GPSR3_SD1_DAT3
		      | GPSR3_SD1_DAT2
		      | GPSR3_SD1_DAT1
		      | GPSR3_SD1_DAT0
		      | GPSR3_SD1_CMD
		      | GPSR3_SD1_CLK
		      | GPSR3_SD0_DAT3
		      | GPSR3_SD0_DAT2
		      | GPSR3_SD0_DAT1
		      | GPSR3_SD0_DAT0
		      | GPSR3_SD0_CMD
		      | GPSR3_SD0_CLK);
	pfc_reg_write(PFC_GPSR4, GPSR4_SD3_DS
		      | GPSR4_SD3_DAT7
		      | GPSR4_SD3_DAT6
		      | GPSR4_SD3_DAT5
		      | GPSR4_SD3_DAT4
		      | GPSR4_SD3_DAT3
		      | GPSR4_SD3_DAT2
		      | GPSR4_SD3_DAT1
		      | GPSR4_SD3_DAT0
		      | GPSR4_SD3_CMD
		      | GPSR4_SD3_CLK);
	pfc_reg_write(PFC_GPSR5, GPSR5_SSI_SDATA9
		      | GPSR5_MSIOF0_SS2
		      | GPSR5_MSIOF0_SS1
		      | GPSR5_RX2_A
		      | GPSR5_TX2_A
		      | GPSR5_SCK2_A
		      | GPSR5_RTS0_TANS_A
		      | GPSR5_CTS0_A);
	pfc_reg_write(PFC_GPSR6, GPSR6_USB30_PWEN
		      | GPSR6_SSI_SDATA6
		      | GPSR6_SSI_WS6
		      | GPSR6_SSI_WS5
		      | GPSR6_SSI_SCK5
		      | GPSR6_SSI_SDATA4
		      | GPSR6_USB30_OVC
		      | GPSR6_AUDIO_CLKA
		      | GPSR6_SSI_SDATA3
		      | GPSR6_SSI_WS349
		      | GPSR6_SSI_SCK349
		      | GPSR6_SSI_SDATA1
		      | GPSR6_SSI_SDATA0
		      | GPSR6_SSI_WS01239
		      | GPSR6_SSI_SCK01239);

	/* initialize POC control */
	reg = mmio_read_32(PFC_IOCTRL30);
	reg = ((reg & IOCTRL30_MASK) | POC_SD1_DAT3_33V
	       | POC_SD1_DAT2_33V
	       | POC_SD1_DAT1_33V
	       | POC_SD1_DAT0_33V
	       | POC_SD1_CMD_33V
	       | POC_SD1_CLK_33V
	       | POC_SD0_DAT3_33V
	       | POC_SD0_DAT2_33V
	       | POC_SD0_DAT1_33V
	       | POC_SD0_DAT0_33V
	       | POC_SD0_CMD_33V
	       | POC_SD0_CLK_33V);
	pfc_reg_write(PFC_IOCTRL30, reg);
	reg = mmio_read_32(PFC_IOCTRL32);
	reg = (reg & IOCTRL32_MASK);
	pfc_reg_write(PFC_IOCTRL32, reg);

	/* initialize LSI pin pull-up/down control */
	pfc_reg_write(PFC_PUD0, 0xFDF80000U);
	pfc_reg_write(PFC_PUD1, 0xCE298464U);
	pfc_reg_write(PFC_PUD2, 0xA4C380F4U);
	pfc_reg_write(PFC_PUD3, 0x0000079FU);
	pfc_reg_write(PFC_PUD4, 0xFFF0FFFFU);
	pfc_reg_write(PFC_PUD5, 0x40000000U);

	/* initialize LSI pin pull-enable register */
	pfc_reg_write(PFC_PUEN0, 0xFFF00000U);
	pfc_reg_write(PFC_PUEN1, 0x00000000U);
	pfc_reg_write(PFC_PUEN2, 0x00000004U);
	pfc_reg_write(PFC_PUEN3, 0x00000000U);
	pfc_reg_write(PFC_PUEN4, 0x07800010U);
	pfc_reg_write(PFC_PUEN5, 0x00000000U);

	/* initialize positive/negative logic select */
	mmio_write_32(GPIO_POSNEG0, 0x00000000U);
	mmio_write_32(GPIO_POSNEG1, 0x00000000U);
	mmio_write_32(GPIO_POSNEG2, 0x00000000U);
	mmio_write_32(GPIO_POSNEG3, 0x00000000U);
	mmio_write_32(GPIO_POSNEG4, 0x00000000U);
	mmio_write_32(GPIO_POSNEG5, 0x00000000U);
	mmio_write_32(GPIO_POSNEG6, 0x00000000U);

	/* initialize general IO/interrupt switching */
	mmio_write_32(GPIO_IOINTSEL0, 0x00020000U);
	mmio_write_32(GPIO_IOINTSEL1, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL2, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL3, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL4, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL5, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL6, 0x00000000U);

	/* initialize general output register */
	mmio_write_32(GPIO_OUTDT0, 0x00000010U);
	mmio_write_32(GPIO_OUTDT1, 0x00100000U);
	mmio_write_32(GPIO_OUTDT2, 0x00000000U);
	mmio_write_32(GPIO_OUTDT3, 0x00008000U);
	mmio_write_32(GPIO_OUTDT5, 0x00060000U);
	mmio_write_32(GPIO_OUTDT6, 0x00000000U);

	/* initialize general input/output switching */
	mmio_write_32(GPIO_INOUTSEL0, 0x00000010U);
	mmio_write_32(GPIO_INOUTSEL1, 0x00100020U);
	mmio_write_32(GPIO_INOUTSEL2, 0x03000000U);
	mmio_write_32(GPIO_INOUTSEL3, 0x00008000U);
	mmio_write_32(GPIO_INOUTSEL4, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL5, 0x00060000U);
	mmio_write_32(GPIO_INOUTSEL6, 0x00004000U);
}

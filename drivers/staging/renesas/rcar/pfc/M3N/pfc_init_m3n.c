/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>		/* for uint32_t */

#include <lib/mmio.h>

#include "pfc_init_m3n.h"
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
#define	GPIO_IOINTSEL7		(GPIO_BASE + 0x5800U)
#define	GPIO_INOUTSEL7		(GPIO_BASE + 0x5804U)
#define	GPIO_OUTDT7		(GPIO_BASE + 0x5808U)
#define	GPIO_INDT7		(GPIO_BASE + 0x580CU)
#define	GPIO_INTDT7		(GPIO_BASE + 0x5810U)
#define	GPIO_INTCLR7		(GPIO_BASE + 0x5814U)
#define	GPIO_INTMSK7		(GPIO_BASE + 0x5818U)
#define	GPIO_MSKCLR7		(GPIO_BASE + 0x581CU)
#define	GPIO_POSNEG7		(GPIO_BASE + 0x5820U)
#define	GPIO_EDGLEVEL7		(GPIO_BASE + 0x5824U)
#define	GPIO_FILONOFF7		(GPIO_BASE + 0x5828U)
#define	GPIO_INTMSKS7		(GPIO_BASE + 0x5838U)
#define	GPIO_MSKCLRS7		(GPIO_BASE + 0x583CU)
#define	GPIO_OUTDTSEL7		(GPIO_BASE + 0x5840U)
#define	GPIO_OUTDTH7		(GPIO_BASE + 0x5844U)
#define	GPIO_OUTDTL7		(GPIO_BASE + 0x5848U)
#define	GPIO_BOTHEDGE7		(GPIO_BASE + 0x584CU)

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
#define	PFC_GPSR7		(PFC_BASE + 0x011CU)
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
#define	PFC_IPSR16		(PFC_BASE + 0x0240U)
#define	PFC_IPSR17		(PFC_BASE + 0x0244U)
#define	PFC_IPSR18		(PFC_BASE + 0x0248U)
#define PFC_DRVCTRL0		(PFC_BASE + 0x0300U)
#define PFC_DRVCTRL1		(PFC_BASE + 0x0304U)
#define PFC_DRVCTRL2		(PFC_BASE + 0x0308U)
#define PFC_DRVCTRL3		(PFC_BASE + 0x030CU)
#define PFC_DRVCTRL4		(PFC_BASE + 0x0310U)
#define PFC_DRVCTRL5		(PFC_BASE + 0x0314U)
#define PFC_DRVCTRL6		(PFC_BASE + 0x0318U)
#define PFC_DRVCTRL7		(PFC_BASE + 0x031CU)
#define PFC_DRVCTRL8		(PFC_BASE + 0x0320U)
#define PFC_DRVCTRL9		(PFC_BASE + 0x0324U)
#define PFC_DRVCTRL10		(PFC_BASE + 0x0328U)
#define PFC_DRVCTRL11		(PFC_BASE + 0x032CU)
#define PFC_DRVCTRL12		(PFC_BASE + 0x0330U)
#define PFC_DRVCTRL13		(PFC_BASE + 0x0334U)
#define PFC_DRVCTRL14		(PFC_BASE + 0x0338U)
#define PFC_DRVCTRL15		(PFC_BASE + 0x033CU)
#define PFC_DRVCTRL16		(PFC_BASE + 0x0340U)
#define PFC_DRVCTRL17		(PFC_BASE + 0x0344U)
#define PFC_DRVCTRL18		(PFC_BASE + 0x0348U)
#define PFC_DRVCTRL19		(PFC_BASE + 0x034CU)
#define PFC_DRVCTRL20		(PFC_BASE + 0x0350U)
#define PFC_DRVCTRL21		(PFC_BASE + 0x0354U)
#define PFC_DRVCTRL22		(PFC_BASE + 0x0358U)
#define PFC_DRVCTRL23		(PFC_BASE + 0x035CU)
#define PFC_DRVCTRL24		(PFC_BASE + 0x0360U)
#define PFC_POCCTRL0		(PFC_BASE + 0x0380U)
#define	PFC_TDSELCTRL0		(PFC_BASE + 0x03C0U)
#define	PFC_IOCTRL		(PFC_BASE + 0x03E0U)
#define	PFC_TSREG		(PFC_BASE + 0x03E4U)
#define	PFC_PUEN0		(PFC_BASE + 0x0400U)
#define	PFC_PUEN1		(PFC_BASE + 0x0404U)
#define	PFC_PUEN2		(PFC_BASE + 0x0408U)
#define	PFC_PUEN3		(PFC_BASE + 0x040CU)
#define	PFC_PUEN4		(PFC_BASE + 0x0410U)
#define	PFC_PUEN5		(PFC_BASE + 0x0414U)
#define	PFC_PUEN6		(PFC_BASE + 0x0418U)
#define	PFC_PUD0		(PFC_BASE + 0x0440U)
#define	PFC_PUD1		(PFC_BASE + 0x0444U)
#define	PFC_PUD2		(PFC_BASE + 0x0448U)
#define	PFC_PUD3		(PFC_BASE + 0x044CU)
#define	PFC_PUD4		(PFC_BASE + 0x0450U)
#define	PFC_PUD5		(PFC_BASE + 0x0454U)
#define	PFC_PUD6		(PFC_BASE + 0x0458U)
#define	PFC_MOD_SEL0		(PFC_BASE + 0x0500U)
#define	PFC_MOD_SEL1		(PFC_BASE + 0x0504U)
#define	PFC_MOD_SEL2		(PFC_BASE + 0x0508U)

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
#define	GPSR1_CLKOUT		((uint32_t)1U << 28U)
#define	GPSR1_EX_WAIT0_A	((uint32_t)1U << 27U)
#define	GPSR1_WE1		((uint32_t)1U << 26U)
#define	GPSR1_WE0		((uint32_t)1U << 25U)
#define	GPSR1_RD_WR		((uint32_t)1U << 24U)
#define	GPSR1_RD		((uint32_t)1U << 23U)
#define	GPSR1_BS		((uint32_t)1U << 22U)
#define	GPSR1_CS1_A26		((uint32_t)1U << 21U)
#define	GPSR1_CS0		((uint32_t)1U << 20U)
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
#define	GPSR2_AVB_AVTP_CAPTURE_A	((uint32_t)1U << 14U)
#define	GPSR2_AVB_AVTP_MATCH_A	((uint32_t)1U << 13U)
#define	GPSR2_AVB_LINK		((uint32_t)1U << 12U)
#define	GPSR2_AVB_PHY_INT	((uint32_t)1U << 11U)
#define	GPSR2_AVB_MAGIC		((uint32_t)1U << 10U)
#define	GPSR2_AVB_MDC		((uint32_t)1U << 9U)
#define	GPSR2_PWM2_A		((uint32_t)1U << 8U)
#define	GPSR2_PWM1_A		((uint32_t)1U << 7U)
#define	GPSR2_PWM0		((uint32_t)1U << 6U)
#define	GPSR2_IRQ5		((uint32_t)1U << 5U)
#define	GPSR2_IRQ4		((uint32_t)1U << 4U)
#define	GPSR2_IRQ3		((uint32_t)1U << 3U)
#define	GPSR2_IRQ2		((uint32_t)1U << 2U)
#define	GPSR2_IRQ1		((uint32_t)1U << 1U)
#define	GPSR2_IRQ0		((uint32_t)1U << 0U)
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
#define	GPSR4_SD3_DS		((uint32_t)1U << 17U)
#define	GPSR4_SD3_DAT7		((uint32_t)1U << 16U)
#define	GPSR4_SD3_DAT6		((uint32_t)1U << 15U)
#define	GPSR4_SD3_DAT5		((uint32_t)1U << 14U)
#define	GPSR4_SD3_DAT4		((uint32_t)1U << 13U)
#define	GPSR4_SD3_DAT3		((uint32_t)1U << 12U)
#define	GPSR4_SD3_DAT2		((uint32_t)1U << 11U)
#define	GPSR4_SD3_DAT1		((uint32_t)1U << 10U)
#define	GPSR4_SD3_DAT0		((uint32_t)1U << 9U)
#define	GPSR4_SD3_CMD		((uint32_t)1U << 8U)
#define	GPSR4_SD3_CLK		((uint32_t)1U << 7U)
#define	GPSR4_SD2_DS		((uint32_t)1U << 6U)
#define	GPSR4_SD2_DAT3		((uint32_t)1U << 5U)
#define	GPSR4_SD2_DAT2		((uint32_t)1U << 4U)
#define	GPSR4_SD2_DAT1		((uint32_t)1U << 3U)
#define	GPSR4_SD2_DAT0		((uint32_t)1U << 2U)
#define	GPSR4_SD2_CMD		((uint32_t)1U << 1U)
#define	GPSR4_SD2_CLK		((uint32_t)1U << 0U)
#define	GPSR5_MLB_DAT		((uint32_t)1U << 25U)
#define	GPSR5_MLB_SIG		((uint32_t)1U << 24U)
#define	GPSR5_MLB_CLK		((uint32_t)1U << 23U)
#define	GPSR5_MSIOF0_RXD	((uint32_t)1U << 22U)
#define	GPSR5_MSIOF0_SS2	((uint32_t)1U << 21U)
#define	GPSR5_MSIOF0_TXD	((uint32_t)1U << 20U)
#define	GPSR5_MSIOF0_SS1	((uint32_t)1U << 19U)
#define	GPSR5_MSIOF0_SYNC	((uint32_t)1U << 18U)
#define	GPSR5_MSIOF0_SCK	((uint32_t)1U << 17U)
#define	GPSR5_HRTS0		((uint32_t)1U << 16U)
#define	GPSR5_HCTS0		((uint32_t)1U << 15U)
#define	GPSR5_HTX0		((uint32_t)1U << 14U)
#define	GPSR5_HRX0		((uint32_t)1U << 13U)
#define	GPSR5_HSCK0		((uint32_t)1U << 12U)
#define	GPSR5_RX2_A		((uint32_t)1U << 11U)
#define	GPSR5_TX2_A		((uint32_t)1U << 10U)
#define	GPSR5_SCK2		((uint32_t)1U << 9U)
#define	GPSR5_RTS1_TANS		((uint32_t)1U << 8U)
#define	GPSR5_CTS1		((uint32_t)1U << 7U)
#define	GPSR5_TX1_A		((uint32_t)1U << 6U)
#define	GPSR5_RX1_A		((uint32_t)1U << 5U)
#define	GPSR5_RTS0_TANS		((uint32_t)1U << 4U)
#define	GPSR5_CTS0		((uint32_t)1U << 3U)
#define	GPSR5_TX0		((uint32_t)1U << 2U)
#define	GPSR5_RX0		((uint32_t)1U << 1U)
#define	GPSR5_SCK0		((uint32_t)1U << 0U)
#define	GPSR6_USB31_OVC		((uint32_t)1U << 31U)
#define	GPSR6_USB31_PWEN	((uint32_t)1U << 30U)
#define	GPSR6_USB30_OVC		((uint32_t)1U << 29U)
#define	GPSR6_USB30_PWEN	((uint32_t)1U << 28U)
#define	GPSR6_USB1_OVC		((uint32_t)1U << 27U)
#define	GPSR6_USB1_PWEN		((uint32_t)1U << 26U)
#define	GPSR6_USB0_OVC		((uint32_t)1U << 25U)
#define	GPSR6_USB0_PWEN		((uint32_t)1U << 24U)
#define	GPSR6_AUDIO_CLKB_B	((uint32_t)1U << 23U)
#define	GPSR6_AUDIO_CLKA_A	((uint32_t)1U << 22U)
#define	GPSR6_SSI_SDATA9_A	((uint32_t)1U << 21U)
#define	GPSR6_SSI_SDATA8	((uint32_t)1U << 20U)
#define	GPSR6_SSI_SDATA7	((uint32_t)1U << 19U)
#define	GPSR6_SSI_WS78		((uint32_t)1U << 18U)
#define	GPSR6_SSI_SCK78		((uint32_t)1U << 17U)
#define	GPSR6_SSI_SDATA6	((uint32_t)1U << 16U)
#define	GPSR6_SSI_WS6		((uint32_t)1U << 15U)
#define	GPSR6_SSI_SCK6		((uint32_t)1U << 14U)
#define	GPSR6_SSI_SDATA5	((uint32_t)1U << 13U)
#define	GPSR6_SSI_WS5		((uint32_t)1U << 12U)
#define	GPSR6_SSI_SCK5		((uint32_t)1U << 11U)
#define	GPSR6_SSI_SDATA4	((uint32_t)1U << 10U)
#define	GPSR6_SSI_WS4		((uint32_t)1U << 9U)
#define	GPSR6_SSI_SCK4		((uint32_t)1U << 8U)
#define	GPSR6_SSI_SDATA3	((uint32_t)1U << 7U)
#define	GPSR6_SSI_WS34		((uint32_t)1U << 6U)
#define	GPSR6_SSI_SCK34		((uint32_t)1U << 5U)
#define	GPSR6_SSI_SDATA2_A	((uint32_t)1U << 4U)
#define	GPSR6_SSI_SDATA1_A	((uint32_t)1U << 3U)
#define	GPSR6_SSI_SDATA0	((uint32_t)1U << 2U)
#define	GPSR6_SSI_WS0129	((uint32_t)1U << 1U)
#define	GPSR6_SSI_SCK0129	((uint32_t)1U << 0U)
#define	GPSR7_HDMI1_CEC		((uint32_t)1U << 3U)
#define	GPSR7_HDMI0_CEC		((uint32_t)1U << 2U)
#define	GPSR7_AVS2		((uint32_t)1U << 1U)
#define	GPSR7_AVS1		((uint32_t)1U << 0U)

#define	IPSR_28_FUNC(x)		((uint32_t)(x) << 28U)
#define	IPSR_24_FUNC(x)		((uint32_t)(x) << 24U)
#define	IPSR_20_FUNC(x)		((uint32_t)(x) << 20U)
#define	IPSR_16_FUNC(x)		((uint32_t)(x) << 16U)
#define	IPSR_12_FUNC(x)		((uint32_t)(x) << 12U)
#define	IPSR_8_FUNC(x)		((uint32_t)(x) << 8U)
#define	IPSR_4_FUNC(x)		((uint32_t)(x) << 4U)
#define	IPSR_0_FUNC(x)		((uint32_t)(x) << 0U)

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
#define	POC_SD2_DS_33V		((uint32_t)1U << 18U)
#define	POC_SD2_DAT3_33V	((uint32_t)1U << 17U)
#define	POC_SD2_DAT2_33V	((uint32_t)1U << 16U)
#define	POC_SD2_DAT1_33V	((uint32_t)1U << 15U)
#define	POC_SD2_DAT0_33V	((uint32_t)1U << 14U)
#define	POC_SD2_CMD_33V		((uint32_t)1U << 13U)
#define	POC_SD2_CLK_33V		((uint32_t)1U << 12U)
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

#define	DRVCTRL0_MASK		(0xCCCCCCCCU)
#define	DRVCTRL1_MASK		(0xCCCCCCC8U)
#define	DRVCTRL2_MASK		(0x88888888U)
#define	DRVCTRL3_MASK		(0x88888888U)
#define	DRVCTRL4_MASK		(0x88888888U)
#define	DRVCTRL5_MASK		(0x88888888U)
#define	DRVCTRL6_MASK		(0x88888888U)
#define	DRVCTRL7_MASK		(0x88888888U)
#define	DRVCTRL8_MASK		(0x88888888U)
#define	DRVCTRL9_MASK		(0x88888888U)
#define	DRVCTRL10_MASK		(0x88888888U)
#define	DRVCTRL11_MASK		(0x888888CCU)
#define	DRVCTRL12_MASK		(0xCCCFFFCFU)
#define	DRVCTRL13_MASK		(0xCC888888U)
#define	DRVCTRL14_MASK		(0x88888888U)
#define	DRVCTRL15_MASK		(0x88888888U)
#define	DRVCTRL16_MASK		(0x88888888U)
#define	DRVCTRL17_MASK		(0x88888888U)
#define	DRVCTRL18_MASK		(0x88888888U)
#define	DRVCTRL19_MASK		(0x88888888U)
#define	DRVCTRL20_MASK		(0x88888888U)
#define	DRVCTRL21_MASK		(0x88888888U)
#define	DRVCTRL22_MASK		(0x88888888U)
#define	DRVCTRL23_MASK		(0x88888888U)
#define	DRVCTRL24_MASK		(0x8888888FU)

#define	DRVCTRL0_QSPI0_SPCLK(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL0_QSPI0_MOSI_IO0(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL0_QSPI0_MISO_IO1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL0_QSPI0_IO2(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL0_QSPI0_IO3(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL0_QSPI0_SSL(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL0_QSPI1_SPCLK(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL0_QSPI1_MOSI_IO0(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL1_QSPI1_MISO_IO1(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL1_QSPI1_IO2(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL1_QSPI1_IO3(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL1_QSPI1_SS(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL1_RPC_INT(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL1_RPC_WP(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL1_RPC_RESET(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL1_AVB_RX_CTL(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL2_AVB_RXC(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL2_AVB_RD0(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL2_AVB_RD1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL2_AVB_RD2(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL2_AVB_RD3(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL2_AVB_TX_CTL(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL2_AVB_TXC(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL2_AVB_TD0(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL3_AVB_TD1(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL3_AVB_TD2(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL3_AVB_TD3(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL3_AVB_TXCREFCLK(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL3_AVB_MDIO(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL3_AVB_MDC(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL3_AVB_MAGIC(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL3_AVB_PHY_INT(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL4_AVB_LINK(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL4_AVB_AVTP_MATCH(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL4_AVB_AVTP_CAPTURE(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL4_IRQ0(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL4_IRQ1(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL4_IRQ2(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL4_IRQ3(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL4_IRQ4(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL5_IRQ5(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL5_PWM0(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL5_PWM1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL5_PWM2(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL5_A0(x)		((uint32_t)(x) << 12U)
#define	DRVCTRL5_A1(x)		((uint32_t)(x) << 8U)
#define	DRVCTRL5_A2(x)		((uint32_t)(x) << 4U)
#define	DRVCTRL5_A3(x)		((uint32_t)(x) << 0U)
#define	DRVCTRL6_A4(x)		((uint32_t)(x) << 28U)
#define	DRVCTRL6_A5(x)		((uint32_t)(x) << 24U)
#define	DRVCTRL6_A6(x)		((uint32_t)(x) << 20U)
#define	DRVCTRL6_A7(x)		((uint32_t)(x) << 16U)
#define	DRVCTRL6_A8(x)		((uint32_t)(x) << 12U)
#define	DRVCTRL6_A9(x)		((uint32_t)(x) << 8U)
#define	DRVCTRL6_A10(x)		((uint32_t)(x) << 4U)
#define	DRVCTRL6_A11(x)		((uint32_t)(x) << 0U)
#define	DRVCTRL7_A12(x)		((uint32_t)(x) << 28U)
#define	DRVCTRL7_A13(x)		((uint32_t)(x) << 24U)
#define	DRVCTRL7_A14(x)		((uint32_t)(x) << 20U)
#define	DRVCTRL7_A15(x)		((uint32_t)(x) << 16U)
#define	DRVCTRL7_A16(x)		((uint32_t)(x) << 12U)
#define	DRVCTRL7_A17(x)		((uint32_t)(x) << 8U)
#define	DRVCTRL7_A18(x)		((uint32_t)(x) << 4U)
#define	DRVCTRL7_A19(x)		((uint32_t)(x) << 0U)
#define	DRVCTRL8_CLKOUT(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL8_CS0(x)		((uint32_t)(x) << 24U)
#define	DRVCTRL8_CS1_A2(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL8_BS(x)		((uint32_t)(x) << 16U)
#define	DRVCTRL8_RD(x)		((uint32_t)(x) << 12U)
#define	DRVCTRL8_RD_W(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL8_WE0(x)		((uint32_t)(x) << 4U)
#define	DRVCTRL8_WE1(x)		((uint32_t)(x) << 0U)
#define	DRVCTRL9_EX_WAIT0(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL9_PRESETOU(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL9_D0(x)		((uint32_t)(x) << 20U)
#define	DRVCTRL9_D1(x)		((uint32_t)(x) << 16U)
#define	DRVCTRL9_D2(x)		((uint32_t)(x) << 12U)
#define	DRVCTRL9_D3(x)		((uint32_t)(x) << 8U)
#define	DRVCTRL9_D4(x)		((uint32_t)(x) << 4U)
#define	DRVCTRL9_D5(x)		((uint32_t)(x) << 0U)
#define	DRVCTRL10_D6(x)		((uint32_t)(x) << 28U)
#define	DRVCTRL10_D7(x)		((uint32_t)(x) << 24U)
#define	DRVCTRL10_D8(x)		((uint32_t)(x) << 20U)
#define	DRVCTRL10_D9(x)		((uint32_t)(x) << 16U)
#define	DRVCTRL10_D10(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL10_D11(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL10_D12(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL10_D13(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL11_D14(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL11_D15(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL11_AVS1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL11_AVS2(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL11_HDMI0_CEC(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL11_HDMI1_CEC(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL11_DU_DOTCLKIN0(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL11_DU_DOTCLKIN1(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL12_DU_DOTCLKIN2(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL12_DU_DOTCLKIN3(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL12_DU_FSCLKST(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL12_DU_TMS(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL13_TDO(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL13_ASEBRK(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL13_SD0_CLK(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL13_SD0_CMD(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL13_SD0_DAT0(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL13_SD0_DAT1(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL13_SD0_DAT2(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL13_SD0_DAT3(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL14_SD1_CLK(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL14_SD1_CMD(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL14_SD1_DAT0(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL14_SD1_DAT1(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL14_SD1_DAT2(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL14_SD1_DAT3(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL14_SD2_CLK(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL14_SD2_CMD(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL15_SD2_DAT0(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL15_SD2_DAT1(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL15_SD2_DAT2(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL15_SD2_DAT3(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL15_SD2_DS(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL15_SD3_CLK(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL15_SD3_CMD(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL15_SD3_DAT0(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL16_SD3_DAT1(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL16_SD3_DAT2(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL16_SD3_DAT3(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL16_SD3_DAT4(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL16_SD3_DAT5(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL16_SD3_DAT6(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL16_SD3_DAT7(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL16_SD3_DS(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL17_SD0_CD(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL17_SD0_WP(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL17_SD1_CD(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL17_SD1_WP(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL17_SCK0(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL17_RX0(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL17_TX0(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL17_CTS0(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL18_RTS0_TANS(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL18_RX1(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL18_TX1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL18_CTS1(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL18_RTS1_TANS(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL18_SCK2(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL18_TX2(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL18_RX2(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL19_HSCK0(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL19_HRX0(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL19_HTX0(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL19_HCTS0(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL19_HRTS0(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL19_MSIOF0_SCK(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL19_MSIOF0_SYNC(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL19_MSIOF0_SS1(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL20_MSIOF0_TXD(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL20_MSIOF0_SS2(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL20_MSIOF0_RXD(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL20_MLB_CLK(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL20_MLB_SIG(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL20_MLB_DAT(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL20_MLB_REF(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL20_SSI_SCK0129(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL21_SSI_WS0129(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL21_SSI_SDATA0(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL21_SSI_SDATA1(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL21_SSI_SDATA2(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL21_SSI_SCK34(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL21_SSI_WS34(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL21_SSI_SDATA3(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL21_SSI_SCK4(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL22_SSI_WS4(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL22_SSI_SDATA4(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL22_SSI_SCK5(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL22_SSI_WS5(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL22_SSI_SDATA5(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL22_SSI_SCK6(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL22_SSI_WS6(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL22_SSI_SDATA6(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL23_SSI_SCK78(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL23_SSI_WS78(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL23_SSI_SDATA7(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL23_SSI_SDATA8(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL23_SSI_SDATA9(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL23_AUDIO_CLKA(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL23_AUDIO_CLKB(x)	((uint32_t)(x) << 4U)
#define	DRVCTRL23_USB0_PWEN(x)	((uint32_t)(x) << 0U)
#define	DRVCTRL24_USB0_OVC(x)	((uint32_t)(x) << 28U)
#define	DRVCTRL24_USB1_PWEN(x)	((uint32_t)(x) << 24U)
#define	DRVCTRL24_USB1_OVC(x)	((uint32_t)(x) << 20U)
#define	DRVCTRL24_USB30_PWEN(x)	((uint32_t)(x) << 16U)
#define	DRVCTRL24_USB30_OVC(x)	((uint32_t)(x) << 12U)
#define	DRVCTRL24_USB31_PWEN(x)	((uint32_t)(x) << 8U)
#define	DRVCTRL24_USB31_OVC(x)	((uint32_t)(x) << 4U)

#define	MOD_SEL0_MSIOF3_A	((uint32_t)0U << 29U)
#define	MOD_SEL0_MSIOF3_B	((uint32_t)1U << 29U)
#define	MOD_SEL0_MSIOF3_C	((uint32_t)2U << 29U)
#define	MOD_SEL0_MSIOF3_D	((uint32_t)3U << 29U)
#define	MOD_SEL0_MSIOF3_E	((uint32_t)4U << 29U)
#define	MOD_SEL0_MSIOF2_A	((uint32_t)0U << 27U)
#define	MOD_SEL0_MSIOF2_B	((uint32_t)1U << 27U)
#define	MOD_SEL0_MSIOF2_C	((uint32_t)2U << 27U)
#define	MOD_SEL0_MSIOF2_D	((uint32_t)3U << 27U)
#define	MOD_SEL0_MSIOF1_A	((uint32_t)0U << 24U)
#define	MOD_SEL0_MSIOF1_B	((uint32_t)1U << 24U)
#define	MOD_SEL0_MSIOF1_C	((uint32_t)2U << 24U)
#define	MOD_SEL0_MSIOF1_D	((uint32_t)3U << 24U)
#define	MOD_SEL0_MSIOF1_E	((uint32_t)4U << 24U)
#define	MOD_SEL0_MSIOF1_F	((uint32_t)5U << 24U)
#define	MOD_SEL0_MSIOF1_G	((uint32_t)6U << 24U)
#define	MOD_SEL0_LBSC_A		((uint32_t)0U << 23U)
#define	MOD_SEL0_LBSC_B		((uint32_t)1U << 23U)
#define	MOD_SEL0_IEBUS_A	((uint32_t)0U << 22U)
#define	MOD_SEL0_IEBUS_B	((uint32_t)1U << 22U)
#define	MOD_SEL0_I2C2_A		((uint32_t)0U << 21U)
#define	MOD_SEL0_I2C2_B		((uint32_t)1U << 21U)
#define	MOD_SEL0_I2C1_A		((uint32_t)0U << 20U)
#define	MOD_SEL0_I2C1_B		((uint32_t)1U << 20U)
#define	MOD_SEL0_HSCIF4_A	((uint32_t)0U << 19U)
#define	MOD_SEL0_HSCIF4_B	((uint32_t)1U << 19U)
#define	MOD_SEL0_HSCIF3_A	((uint32_t)0U << 17U)
#define	MOD_SEL0_HSCIF3_B	((uint32_t)1U << 17U)
#define	MOD_SEL0_HSCIF3_C	((uint32_t)2U << 17U)
#define	MOD_SEL0_HSCIF3_D	((uint32_t)3U << 17U)
#define	MOD_SEL0_HSCIF1_A	((uint32_t)0U << 16U)
#define	MOD_SEL0_HSCIF1_B	((uint32_t)1U << 16U)
#define	MOD_SEL0_FSO_A		((uint32_t)0U << 15U)
#define	MOD_SEL0_FSO_B		((uint32_t)1U << 15U)
#define	MOD_SEL0_HSCIF2_A	((uint32_t)0U << 13U)
#define	MOD_SEL0_HSCIF2_B	((uint32_t)1U << 13U)
#define	MOD_SEL0_HSCIF2_C	((uint32_t)2U << 13U)
#define	MOD_SEL0_ETHERAVB_A	((uint32_t)0U << 12U)
#define	MOD_SEL0_ETHERAVB_B	((uint32_t)1U << 12U)
#define	MOD_SEL0_DRIF3_A	((uint32_t)0U << 11U)
#define	MOD_SEL0_DRIF3_B	((uint32_t)1U << 11U)
#define	MOD_SEL0_DRIF2_A	((uint32_t)0U << 10U)
#define	MOD_SEL0_DRIF2_B	((uint32_t)1U << 10U)
#define	MOD_SEL0_DRIF1_A	((uint32_t)0U << 8U)
#define	MOD_SEL0_DRIF1_B	((uint32_t)1U << 8U)
#define	MOD_SEL0_DRIF1_C	((uint32_t)2U << 8U)
#define	MOD_SEL0_DRIF0_A	((uint32_t)0U << 6U)
#define	MOD_SEL0_DRIF0_B	((uint32_t)1U << 6U)
#define	MOD_SEL0_DRIF0_C	((uint32_t)2U << 6U)
#define	MOD_SEL0_CANFD0_A	((uint32_t)0U << 5U)
#define	MOD_SEL0_CANFD0_B	((uint32_t)1U << 5U)
#define	MOD_SEL0_ADG_A_A	((uint32_t)0U << 3U)
#define	MOD_SEL0_ADG_A_B	((uint32_t)1U << 3U)
#define	MOD_SEL0_ADG_A_C	((uint32_t)2U << 3U)
#define	MOD_SEL1_TSIF1_A	((uint32_t)0U << 30U)
#define	MOD_SEL1_TSIF1_B	((uint32_t)1U << 30U)
#define	MOD_SEL1_TSIF1_C	((uint32_t)2U << 30U)
#define	MOD_SEL1_TSIF1_D	((uint32_t)3U << 30U)
#define	MOD_SEL1_TSIF0_A	((uint32_t)0U << 27U)
#define	MOD_SEL1_TSIF0_B	((uint32_t)1U << 27U)
#define	MOD_SEL1_TSIF0_C	((uint32_t)2U << 27U)
#define	MOD_SEL1_TSIF0_D	((uint32_t)3U << 27U)
#define	MOD_SEL1_TSIF0_E	((uint32_t)4U << 27U)
#define	MOD_SEL1_TIMER_TMU_A	((uint32_t)0U << 26U)
#define	MOD_SEL1_TIMER_TMU_B	((uint32_t)1U << 26U)
#define	MOD_SEL1_SSP1_1_A	((uint32_t)0U << 24U)
#define	MOD_SEL1_SSP1_1_B	((uint32_t)1U << 24U)
#define	MOD_SEL1_SSP1_1_C	((uint32_t)2U << 24U)
#define	MOD_SEL1_SSP1_1_D	((uint32_t)3U << 24U)
#define	MOD_SEL1_SSP1_0_A	((uint32_t)0U << 21U)
#define	MOD_SEL1_SSP1_0_B	((uint32_t)1U << 21U)
#define	MOD_SEL1_SSP1_0_C	((uint32_t)2U << 21U)
#define	MOD_SEL1_SSP1_0_D	((uint32_t)3U << 21U)
#define	MOD_SEL1_SSP1_0_E	((uint32_t)4U << 21U)
#define	MOD_SEL1_SSI_A		((uint32_t)0U << 20U)
#define	MOD_SEL1_SSI_B		((uint32_t)1U << 20U)
#define	MOD_SEL1_SPEED_PULSE_IF_A	((uint32_t)0U << 19U)
#define	MOD_SEL1_SPEED_PULSE_IF_B	((uint32_t)1U << 19U)
#define	MOD_SEL1_SIMCARD_A	((uint32_t)0U << 17U)
#define	MOD_SEL1_SIMCARD_B	((uint32_t)1U << 17U)
#define	MOD_SEL1_SIMCARD_C	((uint32_t)2U << 17U)
#define	MOD_SEL1_SIMCARD_D	((uint32_t)3U << 17U)
#define	MOD_SEL1_SDHI2_A	((uint32_t)0U << 16U)
#define	MOD_SEL1_SDHI2_B	((uint32_t)1U << 16U)
#define	MOD_SEL1_SCIF4_A	((uint32_t)0U << 14U)
#define	MOD_SEL1_SCIF4_B	((uint32_t)1U << 14U)
#define	MOD_SEL1_SCIF4_C	((uint32_t)2U << 14U)
#define	MOD_SEL1_SCIF3_A	((uint32_t)0U << 13U)
#define	MOD_SEL1_SCIF3_B	((uint32_t)1U << 13U)
#define	MOD_SEL1_SCIF2_A	((uint32_t)0U << 12U)
#define	MOD_SEL1_SCIF2_B	((uint32_t)1U << 12U)
#define	MOD_SEL1_SCIF1_A	((uint32_t)0U << 11U)
#define	MOD_SEL1_SCIF1_B	((uint32_t)1U << 11U)
#define	MOD_SEL1_SCIF_A		((uint32_t)0U << 10U)
#define	MOD_SEL1_SCIF_B		((uint32_t)1U << 10U)
#define	MOD_SEL1_REMOCON_A	((uint32_t)0U << 9U)
#define	MOD_SEL1_REMOCON_B	((uint32_t)1U << 9U)
#define	MOD_SEL1_RCAN0_A	((uint32_t)0U << 6U)
#define	MOD_SEL1_RCAN0_B	((uint32_t)1U << 6U)
#define	MOD_SEL1_PWM6_A		((uint32_t)0U << 5U)
#define	MOD_SEL1_PWM6_B		((uint32_t)1U << 5U)
#define	MOD_SEL1_PWM5_A		((uint32_t)0U << 4U)
#define	MOD_SEL1_PWM5_B		((uint32_t)1U << 4U)
#define	MOD_SEL1_PWM4_A		((uint32_t)0U << 3U)
#define	MOD_SEL1_PWM4_B		((uint32_t)1U << 3U)
#define	MOD_SEL1_PWM3_A		((uint32_t)0U << 2U)
#define	MOD_SEL1_PWM3_B		((uint32_t)1U << 2U)
#define	MOD_SEL1_PWM2_A		((uint32_t)0U << 1U)
#define	MOD_SEL1_PWM2_B		((uint32_t)1U << 1U)
#define	MOD_SEL1_PWM1_A		((uint32_t)0U << 0U)
#define	MOD_SEL1_PWM1_B		((uint32_t)1U << 0U)
#define	MOD_SEL2_I2C_5_A	((uint32_t)0U << 31U)
#define	MOD_SEL2_I2C_5_B	((uint32_t)1U << 31U)
#define	MOD_SEL2_I2C_3_A	((uint32_t)0U << 30U)
#define	MOD_SEL2_I2C_3_B	((uint32_t)1U << 30U)
#define	MOD_SEL2_I2C_0_A	((uint32_t)0U << 29U)
#define	MOD_SEL2_I2C_0_B	((uint32_t)1U << 29U)
#define	MOD_SEL2_FM_A		((uint32_t)0U << 27U)
#define	MOD_SEL2_FM_B		((uint32_t)1U << 27U)
#define	MOD_SEL2_FM_C		((uint32_t)2U << 27U)
#define	MOD_SEL2_FM_D		((uint32_t)3U << 27U)
#define	MOD_SEL2_SCIF5_A	((uint32_t)0U << 26U)
#define	MOD_SEL2_SCIF5_B	((uint32_t)1U << 26U)
#define	MOD_SEL2_I2C6_A		((uint32_t)0U << 23U)
#define	MOD_SEL2_I2C6_B		((uint32_t)1U << 23U)
#define	MOD_SEL2_I2C6_C		((uint32_t)2U << 23U)
#define	MOD_SEL2_NDF_A		((uint32_t)0U << 22U)
#define	MOD_SEL2_NDF_B		((uint32_t)1U << 22U)
#define	MOD_SEL2_SSI2_A		((uint32_t)0U << 21U)
#define	MOD_SEL2_SSI2_B		((uint32_t)1U << 21U)
#define	MOD_SEL2_SSI9_A		((uint32_t)0U << 20U)
#define	MOD_SEL2_SSI9_B		((uint32_t)1U << 20U)
#define	MOD_SEL2_TIMER_TMU2_A	((uint32_t)0U << 19U)
#define	MOD_SEL2_TIMER_TMU2_B	((uint32_t)1U << 19U)
#define	MOD_SEL2_ADG_B_A	((uint32_t)0U << 18U)
#define	MOD_SEL2_ADG_B_B	((uint32_t)1U << 18U)
#define	MOD_SEL2_ADG_C_A	((uint32_t)0U << 17U)
#define	MOD_SEL2_ADG_C_B	((uint32_t)1U << 17U)
#define	MOD_SEL2_VIN4_A		((uint32_t)0U << 0U)
#define	MOD_SEL2_VIN4_B		((uint32_t)1U << 0U)

static void pfc_reg_write(uint32_t addr, uint32_t data);

static void pfc_reg_write(uint32_t addr, uint32_t data)
{
	mmio_write_32(PFC_PMMR, ~data);
	mmio_write_32((uintptr_t) addr, data);
}

void pfc_init_m3n(void)
{
	uint32_t reg;

	/* initialize module select */
	pfc_reg_write(PFC_MOD_SEL0, MOD_SEL0_MSIOF3_A
		      | MOD_SEL0_MSIOF2_A
		      | MOD_SEL0_MSIOF1_A
		      | MOD_SEL0_LBSC_A
		      | MOD_SEL0_IEBUS_A
		      | MOD_SEL0_I2C2_A
		      | MOD_SEL0_I2C1_A
		      | MOD_SEL0_HSCIF4_A
		      | MOD_SEL0_HSCIF3_A
		      | MOD_SEL0_HSCIF1_A
		      | MOD_SEL0_FSO_A
		      | MOD_SEL0_HSCIF2_A
		      | MOD_SEL0_ETHERAVB_A
		      | MOD_SEL0_DRIF3_A
		      | MOD_SEL0_DRIF2_A
		      | MOD_SEL0_DRIF1_A
		      | MOD_SEL0_DRIF0_A
		      | MOD_SEL0_CANFD0_A
		      | MOD_SEL0_ADG_A_A);
	pfc_reg_write(PFC_MOD_SEL1, MOD_SEL1_TSIF1_A
		      | MOD_SEL1_TSIF0_A
		      | MOD_SEL1_TIMER_TMU_A
		      | MOD_SEL1_SSP1_1_A
		      | MOD_SEL1_SSP1_0_A
		      | MOD_SEL1_SSI_A
		      | MOD_SEL1_SPEED_PULSE_IF_A
		      | MOD_SEL1_SIMCARD_A
		      | MOD_SEL1_SDHI2_A
		      | MOD_SEL1_SCIF4_A
		      | MOD_SEL1_SCIF3_A
		      | MOD_SEL1_SCIF2_A
		      | MOD_SEL1_SCIF1_A
		      | MOD_SEL1_SCIF_A
		      | MOD_SEL1_REMOCON_A
		      | MOD_SEL1_RCAN0_A
		      | MOD_SEL1_PWM6_A
		      | MOD_SEL1_PWM5_A
		      | MOD_SEL1_PWM4_A
		      | MOD_SEL1_PWM3_A
		      | MOD_SEL1_PWM2_A
		      | MOD_SEL1_PWM1_A);
	pfc_reg_write(PFC_MOD_SEL2, MOD_SEL2_I2C_5_A
		      | MOD_SEL2_I2C_3_A
		      | MOD_SEL2_I2C_0_A
		      | MOD_SEL2_FM_A
		      | MOD_SEL2_SCIF5_A
		      | MOD_SEL2_I2C6_A
		      | MOD_SEL2_NDF_A
		      | MOD_SEL2_SSI2_A
		      | MOD_SEL2_SSI9_A
		      | MOD_SEL2_TIMER_TMU2_A
		      | MOD_SEL2_ADG_B_A
		      | MOD_SEL2_ADG_C_A
		      | MOD_SEL2_VIN4_A);

	/* initialize peripheral function select */
	pfc_reg_write(PFC_IPSR0, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR1, IPSR_28_FUNC(6)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(3)
		      | IPSR_8_FUNC(3)
		      | IPSR_4_FUNC(3)
		      | IPSR_0_FUNC(3));
	pfc_reg_write(PFC_IPSR2, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(6)
		      | IPSR_20_FUNC(6)
		      | IPSR_16_FUNC(6)
		      | IPSR_12_FUNC(6)
		      | IPSR_8_FUNC(6)
		      | IPSR_4_FUNC(6)
		      | IPSR_0_FUNC(6));
	pfc_reg_write(PFC_IPSR3, IPSR_28_FUNC(6)
		      | IPSR_24_FUNC(6)
		      | IPSR_20_FUNC(6)
		      | IPSR_16_FUNC(6)
		      | IPSR_12_FUNC(6)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR4, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(6)
		      | IPSR_4_FUNC(6)
		      | IPSR_0_FUNC(6));
	pfc_reg_write(PFC_IPSR5, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(6)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR6, IPSR_28_FUNC(6)
		      | IPSR_24_FUNC(6)
		      | IPSR_20_FUNC(6)
		      | IPSR_16_FUNC(6)
		      | IPSR_12_FUNC(6)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR7, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(6)
		      | IPSR_4_FUNC(6)
		      | IPSR_0_FUNC(6));
	pfc_reg_write(PFC_IPSR8, IPSR_28_FUNC(1)
		      | IPSR_24_FUNC(1)
		      | IPSR_20_FUNC(1)
		      | IPSR_16_FUNC(1)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR9, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR10, IPSR_28_FUNC(1)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR11, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(4)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(1));
	pfc_reg_write(PFC_IPSR12, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(4)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR13, IPSR_28_FUNC(8)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(3)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR14, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(3)
		      | IPSR_0_FUNC(8));
	pfc_reg_write(PFC_IPSR15, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR16, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR17, IPSR_28_FUNC(0)
		      | IPSR_24_FUNC(0)
		      | IPSR_20_FUNC(0)
		      | IPSR_16_FUNC(0)
		      | IPSR_12_FUNC(0)
		      | IPSR_8_FUNC(0)
		      | IPSR_4_FUNC(1)
		      | IPSR_0_FUNC(0));
	pfc_reg_write(PFC_IPSR18, IPSR_4_FUNC(0)
		      | IPSR_0_FUNC(0));

	/* initialize GPIO/perihperal function select */
	pfc_reg_write(PFC_GPSR0, GPSR0_D15
		      | GPSR0_D14
		      | GPSR0_D13
		      | GPSR0_D12
		      | GPSR0_D11
		      | GPSR0_D10
		      | GPSR0_D9
		      | GPSR0_D8);
	pfc_reg_write(PFC_GPSR1, GPSR1_CLKOUT
		      | GPSR1_EX_WAIT0_A
		      | GPSR1_A19
		      | GPSR1_A18
		      | GPSR1_A17
		      | GPSR1_A16
		      | GPSR1_A15
		      | GPSR1_A14
		      | GPSR1_A13
		      | GPSR1_A12
		      | GPSR1_A7
		      | GPSR1_A6
		      | GPSR1_A5
		      | GPSR1_A4
		      | GPSR1_A3
		      | GPSR1_A2
		      | GPSR1_A1
		      | GPSR1_A0);
	pfc_reg_write(PFC_GPSR2, GPSR2_AVB_AVTP_CAPTURE_A
		      | GPSR2_AVB_AVTP_MATCH_A
		      | GPSR2_AVB_LINK
		      | GPSR2_AVB_PHY_INT
		      | GPSR2_AVB_MDC
		      | GPSR2_PWM2_A
		      | GPSR2_PWM1_A
		      | GPSR2_IRQ5
		      | GPSR2_IRQ4
		      | GPSR2_IRQ3
		      | GPSR2_IRQ2
		      | GPSR2_IRQ1
		      | GPSR2_IRQ0);
	pfc_reg_write(PFC_GPSR3, GPSR3_SD0_WP
		      | GPSR3_SD0_CD
		      | GPSR3_SD1_DAT3
		      | GPSR3_SD1_DAT2
		      | GPSR3_SD1_DAT1
		      | GPSR3_SD1_DAT0
		      | GPSR3_SD0_DAT3
		      | GPSR3_SD0_DAT2
		      | GPSR3_SD0_DAT1
		      | GPSR3_SD0_DAT0
		      | GPSR3_SD0_CMD
		      | GPSR3_SD0_CLK);
	pfc_reg_write(PFC_GPSR4, GPSR4_SD3_DAT7
		      | GPSR4_SD3_DAT6
		      | GPSR4_SD3_DAT3
		      | GPSR4_SD3_DAT2
		      | GPSR4_SD3_DAT1
		      | GPSR4_SD3_DAT0
		      | GPSR4_SD3_CMD
		      | GPSR4_SD3_CLK
		      | GPSR4_SD2_DS
		      | GPSR4_SD2_DAT3
		      | GPSR4_SD2_DAT2
		      | GPSR4_SD2_DAT1
		      | GPSR4_SD2_DAT0
		      | GPSR4_SD2_CMD
		      | GPSR4_SD2_CLK);
	pfc_reg_write(PFC_GPSR5, GPSR5_MSIOF0_SS2
		      | GPSR5_MSIOF0_SS1
		      | GPSR5_MSIOF0_SYNC
		      | GPSR5_HRTS0
		      | GPSR5_HCTS0
		      | GPSR5_HTX0
		      | GPSR5_HRX0
		      | GPSR5_HSCK0
		      | GPSR5_RX2_A
		      | GPSR5_TX2_A
		      | GPSR5_SCK2
		      | GPSR5_RTS1_TANS
		      | GPSR5_CTS1
		      | GPSR5_TX1_A
		      | GPSR5_RX1_A
		      | GPSR5_RTS0_TANS
		      | GPSR5_SCK0);
	pfc_reg_write(PFC_GPSR6, GPSR6_USB30_OVC
		      | GPSR6_USB30_PWEN
		      | GPSR6_USB1_OVC
		      | GPSR6_USB1_PWEN
		      | GPSR6_USB0_OVC
		      | GPSR6_USB0_PWEN
		      | GPSR6_AUDIO_CLKB_B
		      | GPSR6_AUDIO_CLKA_A
		      | GPSR6_SSI_SDATA8
		      | GPSR6_SSI_SDATA7
		      | GPSR6_SSI_WS78
		      | GPSR6_SSI_SCK78
		      | GPSR6_SSI_WS6
		      | GPSR6_SSI_SCK6
		      | GPSR6_SSI_SDATA4
		      | GPSR6_SSI_WS4
		      | GPSR6_SSI_SCK4
		      | GPSR6_SSI_SDATA1_A
		      | GPSR6_SSI_SDATA0
		      | GPSR6_SSI_WS0129
		      | GPSR6_SSI_SCK0129);
	pfc_reg_write(PFC_GPSR7, GPSR7_HDMI1_CEC
		      | GPSR7_HDMI0_CEC
		      | GPSR7_AVS2
		      | GPSR7_AVS1);

	/* initialize POC control register */
	pfc_reg_write(PFC_POCCTRL0, POC_SD3_DS_33V
		      | POC_SD3_DAT7_33V
		      | POC_SD3_DAT6_33V
		      | POC_SD3_DAT5_33V
		      | POC_SD3_DAT4_33V
		      | POC_SD3_DAT3_33V
		      | POC_SD3_DAT2_33V
		      | POC_SD3_DAT1_33V
		      | POC_SD3_DAT0_33V
		      | POC_SD3_CMD_33V
		      | POC_SD3_CLK_33V
		      | POC_SD0_DAT3_33V
		      | POC_SD0_DAT2_33V
		      | POC_SD0_DAT1_33V
		      | POC_SD0_DAT0_33V
		      | POC_SD0_CMD_33V
		      | POC_SD0_CLK_33V);

	/* initialize DRV control register */
	reg = mmio_read_32(PFC_DRVCTRL0);
	reg = ((reg & DRVCTRL0_MASK) | DRVCTRL0_QSPI0_SPCLK(3)
	       | DRVCTRL0_QSPI0_MOSI_IO0(3)
	       | DRVCTRL0_QSPI0_MISO_IO1(3)
	       | DRVCTRL0_QSPI0_IO2(3)
	       | DRVCTRL0_QSPI0_IO3(3)
	       | DRVCTRL0_QSPI0_SSL(3)
	       | DRVCTRL0_QSPI1_SPCLK(3)
	       | DRVCTRL0_QSPI1_MOSI_IO0(3));
	pfc_reg_write(PFC_DRVCTRL0, reg);
	reg = mmio_read_32(PFC_DRVCTRL1);
	reg = ((reg & DRVCTRL1_MASK) | DRVCTRL1_QSPI1_MISO_IO1(3)
	       | DRVCTRL1_QSPI1_IO2(3)
	       | DRVCTRL1_QSPI1_IO3(3)
	       | DRVCTRL1_QSPI1_SS(3)
	       | DRVCTRL1_RPC_INT(3)
	       | DRVCTRL1_RPC_WP(3)
	       | DRVCTRL1_RPC_RESET(3)
	       | DRVCTRL1_AVB_RX_CTL(7));
	pfc_reg_write(PFC_DRVCTRL1, reg);
	reg = mmio_read_32(PFC_DRVCTRL2);
	reg = ((reg & DRVCTRL2_MASK) | DRVCTRL2_AVB_RXC(7)
	       | DRVCTRL2_AVB_RD0(7)
	       | DRVCTRL2_AVB_RD1(7)
	       | DRVCTRL2_AVB_RD2(7)
	       | DRVCTRL2_AVB_RD3(7)
	       | DRVCTRL2_AVB_TX_CTL(3)
	       | DRVCTRL2_AVB_TXC(3)
	       | DRVCTRL2_AVB_TD0(3));
	pfc_reg_write(PFC_DRVCTRL2, reg);
	reg = mmio_read_32(PFC_DRVCTRL3);
	reg = ((reg & DRVCTRL3_MASK) | DRVCTRL3_AVB_TD1(3)
	       | DRVCTRL3_AVB_TD2(3)
	       | DRVCTRL3_AVB_TD3(3)
	       | DRVCTRL3_AVB_TXCREFCLK(7)
	       | DRVCTRL3_AVB_MDIO(7)
	       | DRVCTRL3_AVB_MDC(7)
	       | DRVCTRL3_AVB_MAGIC(7)
	       | DRVCTRL3_AVB_PHY_INT(7));
	pfc_reg_write(PFC_DRVCTRL3, reg);
	reg = mmio_read_32(PFC_DRVCTRL4);
	reg = ((reg & DRVCTRL4_MASK) | DRVCTRL4_AVB_LINK(7)
	       | DRVCTRL4_AVB_AVTP_MATCH(7)
	       | DRVCTRL4_AVB_AVTP_CAPTURE(7)
	       | DRVCTRL4_IRQ0(7)
	       | DRVCTRL4_IRQ1(7)
	       | DRVCTRL4_IRQ2(7)
	       | DRVCTRL4_IRQ3(7)
	       | DRVCTRL4_IRQ4(7));
	pfc_reg_write(PFC_DRVCTRL4, reg);
	reg = mmio_read_32(PFC_DRVCTRL5);
	reg = ((reg & DRVCTRL5_MASK) | DRVCTRL5_IRQ5(7)
	       | DRVCTRL5_PWM0(7)
	       | DRVCTRL5_PWM1(7)
	       | DRVCTRL5_PWM2(7)
	       | DRVCTRL5_A0(3)
	       | DRVCTRL5_A1(3)
	       | DRVCTRL5_A2(3)
	       | DRVCTRL5_A3(3));
	pfc_reg_write(PFC_DRVCTRL5, reg);
	reg = mmio_read_32(PFC_DRVCTRL6);
	reg = ((reg & DRVCTRL6_MASK) | DRVCTRL6_A4(3)
	       | DRVCTRL6_A5(3)
	       | DRVCTRL6_A6(3)
	       | DRVCTRL6_A7(3)
	       | DRVCTRL6_A8(7)
	       | DRVCTRL6_A9(7)
	       | DRVCTRL6_A10(7)
	       | DRVCTRL6_A11(7));
	pfc_reg_write(PFC_DRVCTRL6, reg);
	reg = mmio_read_32(PFC_DRVCTRL7);
	reg = ((reg & DRVCTRL7_MASK) | DRVCTRL7_A12(3)
	       | DRVCTRL7_A13(3)
	       | DRVCTRL7_A14(3)
	       | DRVCTRL7_A15(3)
	       | DRVCTRL7_A16(3)
	       | DRVCTRL7_A17(3)
	       | DRVCTRL7_A18(3)
	       | DRVCTRL7_A19(3));
	pfc_reg_write(PFC_DRVCTRL7, reg);
	reg = mmio_read_32(PFC_DRVCTRL8);
	reg = ((reg & DRVCTRL8_MASK) | DRVCTRL8_CLKOUT(7)
	       | DRVCTRL8_CS0(7)
	       | DRVCTRL8_CS1_A2(7)
	       | DRVCTRL8_BS(7)
	       | DRVCTRL8_RD(7)
	       | DRVCTRL8_RD_W(7)
	       | DRVCTRL8_WE0(7)
	       | DRVCTRL8_WE1(7));
	pfc_reg_write(PFC_DRVCTRL8, reg);
	reg = mmio_read_32(PFC_DRVCTRL9);
	reg = ((reg & DRVCTRL9_MASK) | DRVCTRL9_EX_WAIT0(7)
	       | DRVCTRL9_PRESETOU(7)
	       | DRVCTRL9_D0(7)
	       | DRVCTRL9_D1(7)
	       | DRVCTRL9_D2(7)
	       | DRVCTRL9_D3(7)
	       | DRVCTRL9_D4(7)
	       | DRVCTRL9_D5(7));
	pfc_reg_write(PFC_DRVCTRL9, reg);
	reg = mmio_read_32(PFC_DRVCTRL10);
	reg = ((reg & DRVCTRL10_MASK) | DRVCTRL10_D6(7)
	       | DRVCTRL10_D7(7)
	       | DRVCTRL10_D8(3)
	       | DRVCTRL10_D9(3)
	       | DRVCTRL10_D10(3)
	       | DRVCTRL10_D11(3)
	       | DRVCTRL10_D12(3)
	       | DRVCTRL10_D13(3));
	pfc_reg_write(PFC_DRVCTRL10, reg);
	reg = mmio_read_32(PFC_DRVCTRL11);
	reg = ((reg & DRVCTRL11_MASK) | DRVCTRL11_D14(3)
	       | DRVCTRL11_D15(3)
	       | DRVCTRL11_AVS1(7)
	       | DRVCTRL11_AVS2(7)
	       | DRVCTRL11_HDMI0_CEC(7)
	       | DRVCTRL11_HDMI1_CEC(7)
	       | DRVCTRL11_DU_DOTCLKIN0(3)
	       | DRVCTRL11_DU_DOTCLKIN1(3));
	pfc_reg_write(PFC_DRVCTRL11, reg);
	reg = mmio_read_32(PFC_DRVCTRL12);
	reg = ((reg & DRVCTRL12_MASK) | DRVCTRL12_DU_DOTCLKIN2(3)
	       | DRVCTRL12_DU_DOTCLKIN3(3)
	       | DRVCTRL12_DU_FSCLKST(3)
	       | DRVCTRL12_DU_TMS(3));
	pfc_reg_write(PFC_DRVCTRL12, reg);
	reg = mmio_read_32(PFC_DRVCTRL13);
	reg = ((reg & DRVCTRL13_MASK) | DRVCTRL13_TDO(3)
	       | DRVCTRL13_ASEBRK(3)
	       | DRVCTRL13_SD0_CLK(7)
	       | DRVCTRL13_SD0_CMD(7)
	       | DRVCTRL13_SD0_DAT0(7)
	       | DRVCTRL13_SD0_DAT1(7)
	       | DRVCTRL13_SD0_DAT2(7)
	       | DRVCTRL13_SD0_DAT3(7));
	pfc_reg_write(PFC_DRVCTRL13, reg);
	reg = mmio_read_32(PFC_DRVCTRL14);
	reg = ((reg & DRVCTRL14_MASK) | DRVCTRL14_SD1_CLK(7)
	       | DRVCTRL14_SD1_CMD(7)
	       | DRVCTRL14_SD1_DAT0(5)
	       | DRVCTRL14_SD1_DAT1(5)
	       | DRVCTRL14_SD1_DAT2(5)
	       | DRVCTRL14_SD1_DAT3(5)
	       | DRVCTRL14_SD2_CLK(5)
	       | DRVCTRL14_SD2_CMD(5));
	pfc_reg_write(PFC_DRVCTRL14, reg);
	reg = mmio_read_32(PFC_DRVCTRL15);
	reg = ((reg & DRVCTRL15_MASK) | DRVCTRL15_SD2_DAT0(5)
	       | DRVCTRL15_SD2_DAT1(5)
	       | DRVCTRL15_SD2_DAT2(5)
	       | DRVCTRL15_SD2_DAT3(5)
	       | DRVCTRL15_SD2_DS(5)
	       | DRVCTRL15_SD3_CLK(7)
	       | DRVCTRL15_SD3_CMD(7)
	       | DRVCTRL15_SD3_DAT0(7));
	pfc_reg_write(PFC_DRVCTRL15, reg);
	reg = mmio_read_32(PFC_DRVCTRL16);
	reg = ((reg & DRVCTRL16_MASK) | DRVCTRL16_SD3_DAT1(7)
	       | DRVCTRL16_SD3_DAT2(7)
	       | DRVCTRL16_SD3_DAT3(7)
	       | DRVCTRL16_SD3_DAT4(7)
	       | DRVCTRL16_SD3_DAT5(7)
	       | DRVCTRL16_SD3_DAT6(7)
	       | DRVCTRL16_SD3_DAT7(7)
	       | DRVCTRL16_SD3_DS(7));
	pfc_reg_write(PFC_DRVCTRL16, reg);
	reg = mmio_read_32(PFC_DRVCTRL17);
	reg = ((reg & DRVCTRL17_MASK) | DRVCTRL17_SD0_CD(7)
	       | DRVCTRL17_SD0_WP(7)
	       | DRVCTRL17_SD1_CD(7)
	       | DRVCTRL17_SD1_WP(7)
	       | DRVCTRL17_SCK0(7)
	       | DRVCTRL17_RX0(7)
	       | DRVCTRL17_TX0(7)
	       | DRVCTRL17_CTS0(7));
	pfc_reg_write(PFC_DRVCTRL17, reg);
	reg = mmio_read_32(PFC_DRVCTRL18);
	reg = ((reg & DRVCTRL18_MASK) | DRVCTRL18_RTS0_TANS(7)
	       | DRVCTRL18_RX1(7)
	       | DRVCTRL18_TX1(7)
	       | DRVCTRL18_CTS1(7)
	       | DRVCTRL18_RTS1_TANS(7)
	       | DRVCTRL18_SCK2(7)
	       | DRVCTRL18_TX2(7)
	       | DRVCTRL18_RX2(7));
	pfc_reg_write(PFC_DRVCTRL18, reg);
	reg = mmio_read_32(PFC_DRVCTRL19);
	reg = ((reg & DRVCTRL19_MASK) | DRVCTRL19_HSCK0(7)
	       | DRVCTRL19_HRX0(7)
	       | DRVCTRL19_HTX0(7)
	       | DRVCTRL19_HCTS0(7)
	       | DRVCTRL19_HRTS0(7)
	       | DRVCTRL19_MSIOF0_SCK(7)
	       | DRVCTRL19_MSIOF0_SYNC(7)
	       | DRVCTRL19_MSIOF0_SS1(7));
	pfc_reg_write(PFC_DRVCTRL19, reg);
	reg = mmio_read_32(PFC_DRVCTRL20);
	reg = ((reg & DRVCTRL20_MASK) | DRVCTRL20_MSIOF0_TXD(7)
	       | DRVCTRL20_MSIOF0_SS2(7)
	       | DRVCTRL20_MSIOF0_RXD(7)
	       | DRVCTRL20_MLB_CLK(7)
	       | DRVCTRL20_MLB_SIG(7)
	       | DRVCTRL20_MLB_DAT(7)
	       | DRVCTRL20_MLB_REF(7)
	       | DRVCTRL20_SSI_SCK0129(7));
	pfc_reg_write(PFC_DRVCTRL20, reg);
	reg = mmio_read_32(PFC_DRVCTRL21);
	reg = ((reg & DRVCTRL21_MASK) | DRVCTRL21_SSI_WS0129(7)
	       | DRVCTRL21_SSI_SDATA0(7)
	       | DRVCTRL21_SSI_SDATA1(7)
	       | DRVCTRL21_SSI_SDATA2(7)
	       | DRVCTRL21_SSI_SCK34(7)
	       | DRVCTRL21_SSI_WS34(7)
	       | DRVCTRL21_SSI_SDATA3(7)
	       | DRVCTRL21_SSI_SCK4(7));
	pfc_reg_write(PFC_DRVCTRL21, reg);
	reg = mmio_read_32(PFC_DRVCTRL22);
	reg = ((reg & DRVCTRL22_MASK) | DRVCTRL22_SSI_WS4(7)
	       | DRVCTRL22_SSI_SDATA4(7)
	       | DRVCTRL22_SSI_SCK5(7)
	       | DRVCTRL22_SSI_WS5(7)
	       | DRVCTRL22_SSI_SDATA5(7)
	       | DRVCTRL22_SSI_SCK6(7)
	       | DRVCTRL22_SSI_WS6(7)
	       | DRVCTRL22_SSI_SDATA6(7));
	pfc_reg_write(PFC_DRVCTRL22, reg);
	reg = mmio_read_32(PFC_DRVCTRL23);
	reg = ((reg & DRVCTRL23_MASK) | DRVCTRL23_SSI_SCK78(7)
	       | DRVCTRL23_SSI_WS78(7)
	       | DRVCTRL23_SSI_SDATA7(7)
	       | DRVCTRL23_SSI_SDATA8(7)
	       | DRVCTRL23_SSI_SDATA9(7)
	       | DRVCTRL23_AUDIO_CLKA(7)
	       | DRVCTRL23_AUDIO_CLKB(7)
	       | DRVCTRL23_USB0_PWEN(7));
	pfc_reg_write(PFC_DRVCTRL23, reg);
	reg = mmio_read_32(PFC_DRVCTRL24);
	reg = ((reg & DRVCTRL24_MASK) | DRVCTRL24_USB0_OVC(7)
	       | DRVCTRL24_USB1_PWEN(7)
	       | DRVCTRL24_USB1_OVC(7)
	       | DRVCTRL24_USB30_PWEN(7)
	       | DRVCTRL24_USB30_OVC(7)
	       | DRVCTRL24_USB31_PWEN(7)
	       | DRVCTRL24_USB31_OVC(7));
	pfc_reg_write(PFC_DRVCTRL24, reg);

	/* initialize LSI pin pull-up/down control */
	pfc_reg_write(PFC_PUD0, 0x00005FBFU);
	pfc_reg_write(PFC_PUD1, 0x00300FFEU);
	pfc_reg_write(PFC_PUD2, 0x330001E6U);
	pfc_reg_write(PFC_PUD3, 0x000002E0U);
	pfc_reg_write(PFC_PUD4, 0xFFFFFF00U);
	pfc_reg_write(PFC_PUD5, 0x7F5FFF87U);
	pfc_reg_write(PFC_PUD6, 0x00000055U);

	/* initialize LSI pin pull-enable register */
	pfc_reg_write(PFC_PUEN0, 0x00000FFFU);
	pfc_reg_write(PFC_PUEN1, 0x00100234U);
	pfc_reg_write(PFC_PUEN2, 0x000004C4U);
	pfc_reg_write(PFC_PUEN3, 0x00000200U);
	pfc_reg_write(PFC_PUEN4, 0x3E000000U);
	pfc_reg_write(PFC_PUEN5, 0x1F000805U);
	pfc_reg_write(PFC_PUEN6, 0x00000006U);

	/* initialize positive/negative logic select */
	mmio_write_32(GPIO_POSNEG0, 0x00000000U);
	mmio_write_32(GPIO_POSNEG1, 0x00000000U);
	mmio_write_32(GPIO_POSNEG2, 0x00000000U);
	mmio_write_32(GPIO_POSNEG3, 0x00000000U);
	mmio_write_32(GPIO_POSNEG4, 0x00000000U);
	mmio_write_32(GPIO_POSNEG5, 0x00000000U);
	mmio_write_32(GPIO_POSNEG6, 0x00000000U);

	/* initialize general IO/interrupt switching */
	mmio_write_32(GPIO_IOINTSEL0, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL1, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL2, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL3, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL4, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL5, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL6, 0x00000000U);

	/* initialize general output register */
	mmio_write_32(GPIO_OUTDT1, 0x00000000U);
	mmio_write_32(GPIO_OUTDT2, 0x00000400U);
	mmio_write_32(GPIO_OUTDT3, 0x0000C000U);
	mmio_write_32(GPIO_OUTDT5, 0x00000006U);
	mmio_write_32(GPIO_OUTDT6, 0x00003880U);

	/* initialize general input/output switching */
	mmio_write_32(GPIO_INOUTSEL0, 0x00000000U);
	mmio_write_32(GPIO_INOUTSEL1, 0x01000A00U);
	mmio_write_32(GPIO_INOUTSEL2, 0x00000400U);
	mmio_write_32(GPIO_INOUTSEL3, 0x0000C000U);
	mmio_write_32(GPIO_INOUTSEL4, 0x00000000U);
#if (RCAR_GEN3_ULCB == 1)
	mmio_write_32(GPIO_INOUTSEL5, 0x0000000EU);
#else
	mmio_write_32(GPIO_INOUTSEL5, 0x0000020EU);
#endif
	mmio_write_32(GPIO_INOUTSEL6, 0x00013880U);
}

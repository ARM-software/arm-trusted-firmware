/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include "pfc_init_g2e.h"
#include "rcar_def.h"

#include "../pfc_regs.h"

/* PFC */
#define GPSR0_SDA4		BIT(17)
#define GPSR0_SCL4		BIT(16)
#define GPSR0_D15		BIT(15)
#define GPSR0_D14		BIT(14)
#define GPSR0_D13		BIT(13)
#define GPSR0_D12		BIT(12)
#define GPSR0_D11		BIT(11)
#define GPSR0_D10		BIT(10)
#define GPSR0_D9		BIT(9)
#define GPSR0_D8		BIT(8)
#define GPSR0_D7		BIT(7)
#define GPSR0_D6		BIT(6)
#define GPSR0_D5		BIT(5)
#define GPSR0_D4		BIT(4)
#define GPSR0_D3		BIT(3)
#define GPSR0_D2		BIT(2)
#define GPSR0_D1		BIT(1)
#define GPSR0_D0		BIT(0)
#define GPSR1_WE0		BIT(22)
#define GPSR1_CS0		BIT(21)
#define GPSR1_CLKOUT		BIT(20)
#define GPSR1_A19		BIT(19)
#define GPSR1_A18		BIT(18)
#define GPSR1_A17		BIT(17)
#define GPSR1_A16		BIT(16)
#define GPSR1_A15		BIT(15)
#define GPSR1_A14		BIT(14)
#define GPSR1_A13		BIT(13)
#define GPSR1_A12		BIT(12)
#define GPSR1_A11		BIT(11)
#define GPSR1_A10		BIT(10)
#define GPSR1_A9		BIT(9)
#define GPSR1_A8		BIT(8)
#define GPSR1_A7		BIT(7)
#define GPSR1_A6		BIT(6)
#define GPSR1_A5		BIT(5)
#define GPSR1_A4		BIT(4)
#define GPSR1_A3		BIT(3)
#define GPSR1_A2		BIT(2)
#define GPSR1_A1		BIT(1)
#define GPSR1_A0		BIT(0)
#define GPSR2_BIT27_REVERSED	BIT(27)
#define GPSR2_BIT26_REVERSED	BIT(26)
#define GPSR2_EX_WAIT0		BIT(25)
#define GPSR2_RD_WR		BIT(24)
#define GPSR2_RD		BIT(23)
#define GPSR2_BS		BIT(22)
#define GPSR2_AVB_PHY_INT	BIT(21)
#define GPSR2_AVB_TXCREFCLK	BIT(20)
#define GPSR2_AVB_RD3		BIT(19)
#define GPSR2_AVB_RD2		BIT(18)
#define GPSR2_AVB_RD1		BIT(17)
#define GPSR2_AVB_RD0		BIT(16)
#define GPSR2_AVB_RXC		BIT(15)
#define GPSR2_AVB_RX_CTL	BIT(14)
#define GPSR2_RPC_RESET		BIT(13)
#define GPSR2_RPC_RPC_INT	BIT(12)
#define GPSR2_QSPI1_SSL		BIT(11)
#define GPSR2_QSPI1_IO3		BIT(10)
#define GPSR2_QSPI1_IO2		BIT(9)
#define GPSR2_QSPI1_MISO_IO1	BIT(8)
#define GPSR2_QSPI1_MOSI_IO0	BIT(7)
#define GPSR2_QSPI1_SPCLK	BIT(6)
#define GPSR2_QSPI0_SSL		BIT(5)
#define GPSR2_QSPI0_IO3		BIT(4)
#define GPSR2_QSPI0_IO2		BIT(3)
#define GPSR2_QSPI0_MISO_IO1	BIT(2)
#define GPSR2_QSPI0_MOSI_IO0	BIT(1)
#define GPSR2_QSPI0_SPCLK	BIT(0)
#define GPSR3_SD1_WP		BIT(15)
#define GPSR3_SD1_CD		BIT(14)
#define GPSR3_SD0_WP		BIT(13)
#define GPSR3_SD0_CD		BIT(12)
#define GPSR3_SD1_DAT3		BIT(11)
#define GPSR3_SD1_DAT2		BIT(10)
#define GPSR3_SD1_DAT1		BIT(9)
#define GPSR3_SD1_DAT0		BIT(8)
#define GPSR3_SD1_CMD		BIT(7)
#define GPSR3_SD1_CLK		BIT(6)
#define GPSR3_SD0_DAT3		BIT(5)
#define GPSR3_SD0_DAT2		BIT(4)
#define GPSR3_SD0_DAT1		BIT(3)
#define GPSR3_SD0_DAT0		BIT(2)
#define GPSR3_SD0_CMD		BIT(1)
#define GPSR3_SD0_CLK		BIT(0)
#define GPSR4_SD3_DS		BIT(10)
#define GPSR4_SD3_DAT7		BIT(9)
#define GPSR4_SD3_DAT6		BIT(8)
#define GPSR4_SD3_DAT5		BIT(7)
#define GPSR4_SD3_DAT4		BIT(6)
#define GPSR4_SD3_DAT3		BIT(5)
#define GPSR4_SD3_DAT2		BIT(4)
#define GPSR4_SD3_DAT1		BIT(3)
#define GPSR4_SD3_DAT0		BIT(2)
#define GPSR4_SD3_CMD		BIT(1)
#define GPSR4_SD3_CLK		BIT(0)
#define GPSR5_MLB_DAT		BIT(19)
#define GPSR5_MLB_SIG		BIT(18)
#define GPSR5_MLB_CLK		BIT(17)
#define GPSR5_SSI_SDATA9	BIT(16)
#define GPSR5_MSIOF0_SS2	BIT(15)
#define GPSR5_MSIOF0_SS1	BIT(14)
#define GPSR5_MSIOF0_SYNC	BIT(13)
#define GPSR5_MSIOF0_TXD	BIT(12)
#define GPSR5_MSIOF0_RXD	BIT(11)
#define GPSR5_MSIOF0_SCK	BIT(10)
#define GPSR5_RX2_A		BIT(9)
#define GPSR5_TX2_A		BIT(8)
#define GPSR5_SCK2_A		BIT(7)
#define GPSR5_TX1		BIT(6)
#define GPSR5_RX1		BIT(5)
#define GPSR5_RTS0_A		BIT(4)
#define GPSR5_CTS0_A		BIT(3)
#define GPSR5_TX0_A		BIT(2)
#define GPSR5_RX0_A		BIT(1)
#define GPSR5_SCK0_A		BIT(0)
#define GPSR6_USB30_PWEN	BIT(17)
#define GPSR6_SSI_SDATA6	BIT(16)
#define GPSR6_SSI_WS6		BIT(15)
#define GPSR6_SSI_SCK6		BIT(14)
#define GPSR6_SSI_SDATA5	BIT(13)
#define GPSR6_SSI_WS5		BIT(12)
#define GPSR6_SSI_SCK5		BIT(11)
#define GPSR6_SSI_SDATA4	BIT(10)
#define GPSR6_USB30_OVC		BIT(9)
#define GPSR6_AUDIO_CLKA	BIT(8)
#define GPSR6_SSI_SDATA3	BIT(7)
#define GPSR6_SSI_WS349		BIT(6)
#define GPSR6_SSI_SCK349	BIT(5)
#define GPSR6_SSI_SDATA2	BIT(4)
#define GPSR6_SSI_SDATA1	BIT(3)
#define GPSR6_SSI_SDATA0	BIT(2)
#define GPSR6_SSI_WS01239	BIT(1)
#define GPSR6_SSI_SCK01239	BIT(0)

#define IPSR_28_FUNC(x)		((uint32_t)(x) << 28U)
#define IPSR_24_FUNC(x)		((uint32_t)(x) << 24U)
#define IPSR_20_FUNC(x)		((uint32_t)(x) << 20U)
#define IPSR_16_FUNC(x)		((uint32_t)(x) << 16U)
#define IPSR_12_FUNC(x)		((uint32_t)(x) << 12U)
#define IPSR_8_FUNC(x)		((uint32_t)(x) << 8U)
#define IPSR_4_FUNC(x)		((uint32_t)(x) << 4U)
#define IPSR_0_FUNC(x)		((uint32_t)(x) << 0U)

#define POCCTRL0_MASK		(0x0007F000U)
#define POC_SD3_DS_33V		BIT(29)
#define POC_SD3_DAT7_33V	BIT(28)
#define POC_SD3_DAT6_33V	BIT(27)
#define POC_SD3_DAT5_33V	BIT(26)
#define POC_SD3_DAT4_33V	BIT(25)
#define POC_SD3_DAT3_33V	BIT(24)
#define POC_SD3_DAT2_33V	BIT(23)
#define POC_SD3_DAT1_33V	BIT(22)
#define POC_SD3_DAT0_33V	BIT(21)
#define POC_SD3_CMD_33V		BIT(20)
#define POC_SD3_CLK_33V		BIT(19)
#define POC_SD1_DAT3_33V	BIT(11)
#define POC_SD1_DAT2_33V	BIT(10)
#define POC_SD1_DAT1_33V	BIT(9)
#define POC_SD1_DAT0_33V	BIT(8)
#define POC_SD1_CMD_33V		BIT(7)
#define POC_SD1_CLK_33V		BIT(6)
#define POC_SD0_DAT3_33V	BIT(5)
#define POC_SD0_DAT2_33V	BIT(4)
#define POC_SD0_DAT1_33V	BIT(3)
#define POC_SD0_DAT0_33V	BIT(2)
#define POC_SD0_CMD_33V		BIT(1)
#define POC_SD0_CLK_33V		BIT(0)

#define POCCTRL2_MASK		(0xFFFFFFFEU)
#define POC2_VREF_33V		BIT(0)

#define MOD_SEL0_ADGB_A			((uint32_t)0U << 29U)
#define MOD_SEL0_ADGB_B			((uint32_t)1U << 29U)
#define MOD_SEL0_ADGB_C			((uint32_t)2U << 29U)
#define MOD_SEL0_DRIF0_A		((uint32_t)0U << 28U)
#define MOD_SEL0_DRIF0_B		((uint32_t)1U << 28U)
#define MOD_SEL0_FM_A			((uint32_t)0U << 26U)
#define MOD_SEL0_FM_B			((uint32_t)1U << 26U)
#define MOD_SEL0_FM_C			((uint32_t)2U << 26U)
#define MOD_SEL0_FSO_A			((uint32_t)0U << 25U)
#define MOD_SEL0_FSO_B			((uint32_t)1U << 25U)
#define MOD_SEL0_HSCIF0_A		((uint32_t)0U << 24U)
#define MOD_SEL0_HSCIF0_B		((uint32_t)1U << 24U)
#define MOD_SEL0_HSCIF1_A		((uint32_t)0U << 23U)
#define MOD_SEL0_HSCIF1_B		((uint32_t)1U << 23U)
#define MOD_SEL0_HSCIF2_A		((uint32_t)0U << 22U)
#define MOD_SEL0_HSCIF2_B		((uint32_t)1U << 22U)
#define MOD_SEL0_I2C1_A			((uint32_t)0U << 20U)
#define MOD_SEL0_I2C1_B			((uint32_t)1U << 20U)
#define MOD_SEL0_I2C1_C			((uint32_t)2U << 20U)
#define MOD_SEL0_I2C1_D			((uint32_t)3U << 20U)
#define MOD_SEL0_I2C2_A			((uint32_t)0U << 17U)
#define MOD_SEL0_I2C2_B			((uint32_t)1U << 17U)
#define MOD_SEL0_I2C2_C			((uint32_t)2U << 17U)
#define MOD_SEL0_I2C2_D			((uint32_t)3U << 17U)
#define MOD_SEL0_I2C2_E			((uint32_t)4U << 17U)
#define MOD_SEL0_NDFC_A			((uint32_t)0U << 16U)
#define MOD_SEL0_NDFC_B			((uint32_t)1U << 16U)
#define MOD_SEL0_PWM0_A			((uint32_t)0U << 15U)
#define MOD_SEL0_PWM0_B			((uint32_t)1U << 15U)
#define MOD_SEL0_PWM1_A			((uint32_t)0U << 14U)
#define MOD_SEL0_PWM1_B			((uint32_t)1U << 14U)
#define MOD_SEL0_PWM2_A			((uint32_t)0U << 12U)
#define MOD_SEL0_PWM2_B			((uint32_t)1U << 12U)
#define MOD_SEL0_PWM2_C			((uint32_t)2U << 12U)
#define MOD_SEL0_PWM3_A			((uint32_t)0U << 10U)
#define MOD_SEL0_PWM3_B			((uint32_t)1U << 10U)
#define MOD_SEL0_PWM3_C			((uint32_t)2U << 10U)
#define MOD_SEL0_PWM4_A			((uint32_t)0U << 9U)
#define MOD_SEL0_PWM4_B			((uint32_t)1U << 9U)
#define MOD_SEL0_PWM5_A			((uint32_t)0U << 8U)
#define MOD_SEL0_PWM5_B			((uint32_t)1U << 8U)
#define MOD_SEL0_PWM6_A			((uint32_t)0U << 7U)
#define MOD_SEL0_PWM6_B			((uint32_t)1U << 7U)
#define MOD_SEL0_REMOCON_A		((uint32_t)0U << 5U)
#define MOD_SEL0_REMOCON_B		((uint32_t)1U << 5U)
#define MOD_SEL0_REMOCON_C		((uint32_t)2U << 5U)
#define MOD_SEL0_SCIF_A			((uint32_t)0U << 4U)
#define MOD_SEL0_SCIF_B			((uint32_t)1U << 4U)
#define MOD_SEL0_SCIF0_A		((uint32_t)0U << 3U)
#define MOD_SEL0_SCIF0_B		((uint32_t)1U << 3U)
#define MOD_SEL0_SCIF2_A		((uint32_t)0U << 2U)
#define MOD_SEL0_SCIF2_B		((uint32_t)1U << 2U)
#define MOD_SEL0_SPEED_PULSE_IF_A	((uint32_t)0U << 0U)
#define MOD_SEL0_SPEED_PULSE_IF_B	((uint32_t)1U << 0U)
#define MOD_SEL0_SPEED_PULSE_IF_C	((uint32_t)2U << 0U)
#define MOD_SEL1_SIMCARD_A		((uint32_t)0U << 31U)
#define MOD_SEL1_SIMCARD_B		((uint32_t)1U << 31U)
#define MOD_SEL1_SSI2_A			((uint32_t)0U << 30U)
#define MOD_SEL1_SSI2_B			((uint32_t)1U << 30U)
#define MOD_SEL1_TIMER_TMU_A		((uint32_t)0U << 29U)
#define MOD_SEL1_TIMER_TMU_B		((uint32_t)1U << 29U)
#define MOD_SEL1_USB20_CH0_A		((uint32_t)0U << 28U)
#define MOD_SEL1_USB20_CH0_B		((uint32_t)1U << 28U)
#define MOD_SEL1_DRIF2_A		((uint32_t)0U << 26U)
#define MOD_SEL1_DRIF2_B		((uint32_t)1U << 26U)
#define MOD_SEL1_DRIF3_A		((uint32_t)0U << 25U)
#define MOD_SEL1_DRIF3_B		((uint32_t)1U << 25U)
#define MOD_SEL1_HSCIF3_A		((uint32_t)0U << 22U)
#define MOD_SEL1_HSCIF3_B		((uint32_t)1U << 22U)
#define MOD_SEL1_HSCIF3_C		((uint32_t)2U << 22U)
#define MOD_SEL1_HSCIF3_D		((uint32_t)3U << 22U)
#define MOD_SEL1_HSCIF3_E		((uint32_t)4U << 22U)
#define MOD_SEL1_HSCIF4_A		((uint32_t)0U << 19U)
#define MOD_SEL1_HSCIF4_B		((uint32_t)1U << 19U)
#define MOD_SEL1_HSCIF4_C		((uint32_t)2U << 19U)
#define MOD_SEL1_HSCIF4_D		((uint32_t)3U << 19U)
#define MOD_SEL1_HSCIF4_E		((uint32_t)4U << 19U)
#define MOD_SEL1_I2C6_A			((uint32_t)0U << 18U)
#define MOD_SEL1_I2C6_B			((uint32_t)1U << 18U)
#define MOD_SEL1_I2C7_A			((uint32_t)0U << 17U)
#define MOD_SEL1_I2C7_B			((uint32_t)1U << 17U)
#define MOD_SEL1_MSIOF2_A		((uint32_t)0U << 16U)
#define MOD_SEL1_MSIOF2_B		((uint32_t)1U << 16U)
#define MOD_SEL1_MSIOF3_A		((uint32_t)0U << 15U)
#define MOD_SEL1_MSIOF3_B		((uint32_t)1U << 15U)
#define MOD_SEL1_SCIF3_A		((uint32_t)0U << 13U)
#define MOD_SEL1_SCIF3_B		((uint32_t)1U << 13U)
#define MOD_SEL1_SCIF3_C		((uint32_t)2U << 13U)
#define MOD_SEL1_SCIF4_A		((uint32_t)0U << 11U)
#define MOD_SEL1_SCIF4_B		((uint32_t)1U << 11U)
#define MOD_SEL1_SCIF4_C		((uint32_t)2U << 11U)
#define MOD_SEL1_SCIF5_A		((uint32_t)0U << 9U)
#define MOD_SEL1_SCIF5_B		((uint32_t)1U << 9U)
#define MOD_SEL1_SCIF5_C		((uint32_t)2U << 9U)
#define MOD_SEL1_VIN4_A			((uint32_t)0U << 8U)
#define MOD_SEL1_VIN4_B			((uint32_t)1U << 8U)
#define MOD_SEL1_VIN5_A			((uint32_t)0U << 7U)
#define MOD_SEL1_VIN5_B			((uint32_t)1U << 7U)
#define MOD_SEL1_ADGC_A			((uint32_t)0U << 5U)
#define MOD_SEL1_ADGC_B			((uint32_t)1U << 5U)
#define MOD_SEL1_ADGC_C			((uint32_t)2U << 5U)
#define MOD_SEL1_SSI9_A			((uint32_t)0U << 4U)
#define MOD_SEL1_SSI9_B			((uint32_t)1U << 4U)

static void pfc_reg_write(uint32_t addr, uint32_t data)
{
	mmio_write_32(PFC_PMMR, ~data);
	mmio_write_32((uintptr_t)addr, data);
}

void pfc_init_g2e(void)
{
	uint32_t reg;

	/* initialize module select */
	pfc_reg_write(PFC_MOD_SEL0,
		      MOD_SEL0_ADGB_A |
		      MOD_SEL0_DRIF0_A |
		      MOD_SEL0_FM_A |
		      MOD_SEL0_FSO_A |
		      MOD_SEL0_HSCIF0_A |
		      MOD_SEL0_HSCIF1_A |
		      MOD_SEL0_HSCIF2_A |
		      MOD_SEL0_I2C1_A |
		      MOD_SEL0_I2C2_A |
		      MOD_SEL0_NDFC_A |
		      MOD_SEL0_PWM0_A |
		      MOD_SEL0_PWM1_A |
		      MOD_SEL0_PWM2_A |
		      MOD_SEL0_PWM3_A |
		      MOD_SEL0_PWM4_A |
		      MOD_SEL0_PWM5_A |
		      MOD_SEL0_PWM6_A |
		      MOD_SEL0_REMOCON_A |
		      MOD_SEL0_SCIF_A |
		      MOD_SEL0_SCIF0_A |
		      MOD_SEL0_SCIF2_A |
		      MOD_SEL0_SPEED_PULSE_IF_A);

	pfc_reg_write(PFC_MOD_SEL1,
		      MOD_SEL1_SIMCARD_A |
		      MOD_SEL1_SSI2_A |
		      MOD_SEL1_TIMER_TMU_A |
		      MOD_SEL1_USB20_CH0_B |
		      MOD_SEL1_DRIF2_A |
		      MOD_SEL1_DRIF3_A |
		      MOD_SEL1_HSCIF3_C |
		      MOD_SEL1_HSCIF4_B |
		      MOD_SEL1_I2C6_A |
		      MOD_SEL1_I2C7_A |
		      MOD_SEL1_MSIOF2_A |
		      MOD_SEL1_MSIOF3_A |
		      MOD_SEL1_SCIF3_A |
		      MOD_SEL1_SCIF4_A |
		      MOD_SEL1_SCIF5_A |
		      MOD_SEL1_VIN4_A |
		      MOD_SEL1_VIN5_A |
		      MOD_SEL1_ADGC_A |
		      MOD_SEL1_SSI9_A);

	/* initialize peripheral function select */
	pfc_reg_write(PFC_IPSR0,
		      IPSR_28_FUNC(2) |	/* HRX4_B */
		      IPSR_24_FUNC(2) |	/* HTX4_B */
		      IPSR_20_FUNC(0) |	/* QSPI1_SPCLK */
		      IPSR_16_FUNC(0) |	/* QSPI0_IO3 */
		      IPSR_12_FUNC(0) |	/* QSPI0_IO2 */
		      IPSR_8_FUNC(0) |	/* QSPI0_MISO/IO1 */
		      IPSR_4_FUNC(0) |	/* QSPI0_MOSI/IO0 */
		      IPSR_0_FUNC(0));	/* QSPI0_SPCLK */

	pfc_reg_write(PFC_IPSR1,
		      IPSR_28_FUNC(0) |	/* AVB_RD2 */
		      IPSR_24_FUNC(0) |	/* AVB_RD1 */
		      IPSR_20_FUNC(0) |	/* AVB_RD0 */
		      IPSR_16_FUNC(0) |	/* RPC_RESET# */
		      IPSR_12_FUNC(0) |	/* RPC_INT# */
		      IPSR_8_FUNC(0) |	/* QSPI1_SSL */
		      IPSR_4_FUNC(2) |	/* HRX3_C */
		      IPSR_0_FUNC(2));	/* HTX3_C */

	pfc_reg_write(PFC_IPSR2,
		      IPSR_28_FUNC(1) |	/* IRQ0 */
		      IPSR_24_FUNC(0) |
		      IPSR_20_FUNC(0) |
		      IPSR_16_FUNC(2) |	/* AVB_LINK */
		      IPSR_12_FUNC(0) |
		      IPSR_8_FUNC(0) |	/* AVB_MDC */
		      IPSR_4_FUNC(0) |	/* AVB_MDIO */
		      IPSR_0_FUNC(0));	/* AVB_TXCREFCLK */

	pfc_reg_write(PFC_IPSR3,
		      IPSR_28_FUNC(5) |	/* DU_HSYNC */
		      IPSR_24_FUNC(0) |
		      IPSR_20_FUNC(0) |
		      IPSR_16_FUNC(0) |
		      IPSR_12_FUNC(5) |	/* DU_DG4 */
		      IPSR_8_FUNC(5) |	/* DU_DOTCLKOUT0 */
		      IPSR_4_FUNC(5) |	/* DU_DISP */
		      IPSR_0_FUNC(1));	/* IRQ1 */

	pfc_reg_write(PFC_IPSR4,
		      IPSR_28_FUNC(5) |	/* DU_DB5 */
		      IPSR_24_FUNC(5) |	/* DU_DB4 */
		      IPSR_20_FUNC(5) |	/* DU_DB3 */
		      IPSR_16_FUNC(5) |	/* DU_DB2 */
		      IPSR_12_FUNC(5) |	/* DU_DG6 */
		      IPSR_8_FUNC(5) |	/* DU_VSYNC */
		      IPSR_4_FUNC(5) |	/* DU_DG5 */
		      IPSR_0_FUNC(5));	/* DU_DG7 */

	pfc_reg_write(PFC_IPSR5,
		      IPSR_28_FUNC(5) |	/* DU_DR3 */
		      IPSR_24_FUNC(5) |	/* DU_DB7 */
		      IPSR_20_FUNC(5) |	/* DU_DR2 */
		      IPSR_16_FUNC(5) |	/* DU_DR1 */
		      IPSR_12_FUNC(5) |	/* DU_DR0 */
		      IPSR_8_FUNC(5) |	/* DU_DB1 */
		      IPSR_4_FUNC(5) |	/* DU_DB0 */
		      IPSR_0_FUNC(5));	/* DU_DB6 */

	pfc_reg_write(PFC_IPSR6,
		      IPSR_28_FUNC(5) |	/* DU_DG1 */
		      IPSR_24_FUNC(5) |	/* DU_DG0 */
		      IPSR_20_FUNC(5) |	/* DU_DR7 */
		      IPSR_16_FUNC(1) |	/* CANFD1_RX */
		      IPSR_12_FUNC(5) |	/* DU_DR6 */
		      IPSR_8_FUNC(5) |	/* DU_DR5 */
		      IPSR_4_FUNC(1) |	/* CANFD1_TX */
		      IPSR_0_FUNC(5));	/* DU_DR4 */

	pfc_reg_write(PFC_IPSR7,
		      IPSR_28_FUNC(0) |	/* SD0_CLK */
		      IPSR_24_FUNC(0) |
		      IPSR_20_FUNC(5) |	/* DU_DOTCLKIN0 */
		      IPSR_16_FUNC(5) |	/* DU_DG3 */
		      IPSR_12_FUNC(1) |	/* CAN_CLK */
		      IPSR_8_FUNC(1) |	/* CANFD0_RX */
		      IPSR_4_FUNC(1) |	/* CANFD0_TX */
		      IPSR_0_FUNC(5));	/* DU_DG2 */

	pfc_reg_write(PFC_IPSR8,
		      IPSR_28_FUNC(0) |	/* SD1_DAT0 */
		      IPSR_24_FUNC(0) |	/* SD1_CMD */
		      IPSR_20_FUNC(0) |	/* SD1_CLK */
		      IPSR_16_FUNC(0) |	/* SD0_DAT3 */
		      IPSR_12_FUNC(0) |	/* SD0_DAT2 */
		      IPSR_8_FUNC(0) |	/* SD0_DAT1 */
		      IPSR_4_FUNC(0) |	/* SD0_DAT0 */
		      IPSR_0_FUNC(0));	/* SD0_CMD */

	pfc_reg_write(PFC_IPSR9,
		      IPSR_28_FUNC(0) |	/* SD3_DAT2 */
		      IPSR_24_FUNC(0) |	/* SD3_DAT1 */
		      IPSR_20_FUNC(0) |	/* SD3_DAT0 */
		      IPSR_16_FUNC(0) |	/* SD3_CMD */
		      IPSR_12_FUNC(0) |	/* SD3_CLK */
		      IPSR_8_FUNC(0) |	/* SD1_DAT3 */
		      IPSR_4_FUNC(0) |	/* SD1_DAT2 */
		      IPSR_0_FUNC(0));	/* SD1_DAT1 */

	pfc_reg_write(PFC_IPSR10,
		      IPSR_24_FUNC(0) |	/* SD0_CD */
		      IPSR_20_FUNC(0) |	/* SD3_DS */
		      IPSR_16_FUNC(0) |	/* SD3_DAT7 */
		      IPSR_12_FUNC(0) |	/* SD3_DAT6 */
		      IPSR_8_FUNC(0) |	/* SD3_DAT5 */
		      IPSR_4_FUNC(0) |	/* SD3_DAT4 */
		      IPSR_0_FUNC(0));	/* SD3_DAT3 */

	pfc_reg_write(PFC_IPSR11,
		      IPSR_28_FUNC(0) |
		      IPSR_24_FUNC(8) |	/* USB0_ID */
		      IPSR_20_FUNC(2) |	/* AUDIO_CLKOUT1_A */
		      IPSR_16_FUNC(0) |	/* CTS0#_A */
		      IPSR_12_FUNC(0) |
		      IPSR_8_FUNC(0) |
		      IPSR_4_FUNC(0) |	/* SD1_WP */
		      IPSR_0_FUNC(0));	/* SD1_CD */

	pfc_reg_write(PFC_IPSR12,
		      IPSR_28_FUNC(0) |
		      IPSR_24_FUNC(0) |
		      IPSR_20_FUNC(0) |
		      IPSR_16_FUNC(0) |
		      IPSR_12_FUNC(0) |	/* RX2_A */
		      IPSR_8_FUNC(0) |	/* TX2_A */
		      IPSR_4_FUNC(0) |	/* SCK2_A */
		      IPSR_0_FUNC(0));

	pfc_reg_write(PFC_IPSR13,
		      IPSR_28_FUNC(0) |
		      IPSR_24_FUNC(0) |
		      IPSR_20_FUNC(0) |
		      IPSR_16_FUNC(4) |	/* SDA1_B */
		      IPSR_12_FUNC(4) |	/* SCL1_B */
		      IPSR_8_FUNC(0) |	/* SSI_SDATA9 */
		      IPSR_4_FUNC(1) |	/* HTX2_A */
		      IPSR_0_FUNC(1));	/* HRX2_A */

	pfc_reg_write(PFC_IPSR14,
		      IPSR_28_FUNC(0) |	/* SSI_SCK5 */
		      IPSR_24_FUNC(0) |	/* SSI_SDATA4 */
		      IPSR_20_FUNC(0) |	/* SSI_SDATA3 */
		      IPSR_16_FUNC(0) |	/* SSI_WS349 */
		      IPSR_12_FUNC(0) |	/* SSI_SCK349 */
		      IPSR_8_FUNC(0) |
		      IPSR_4_FUNC(0) |	/* SSI_SDATA1 */
		      IPSR_0_FUNC(0));/* SSI_SDATA0 */

	pfc_reg_write(PFC_IPSR15,
		      IPSR_28_FUNC(0) |	/* USB30_OVC */
		      IPSR_24_FUNC(0) |	/* USB30_PWEN */
		      IPSR_20_FUNC(0) |	/* AUDIO_CLKA */
		      IPSR_16_FUNC(1) |	/* HRTS2#_A */
		      IPSR_12_FUNC(1) |	/* HCTS2#_A */
		      IPSR_8_FUNC(3) |	/* TPU0TO1 */
		      IPSR_4_FUNC(3) |	/* TPU0TO0 */
		      IPSR_0_FUNC(0));	/* SSI_WS5 */

	/* initialize GPIO/peripheral function select */
	pfc_reg_write(PFC_GPSR0,
		      GPSR0_SCL4 |
		      GPSR0_D15 |
		      GPSR0_D14 |
		      GPSR0_D13 |
		      GPSR0_D12 |
		      GPSR0_D11 |
		      GPSR0_D10 |
		      GPSR0_D9 |
		      GPSR0_D8 |
		      GPSR0_D7 |
		      GPSR0_D6 |
		      GPSR0_D5 |
		      GPSR0_D4 |
		      GPSR0_D3 |
		      GPSR0_D2 |
		      GPSR0_D1 |
		      GPSR0_D0);

	pfc_reg_write(PFC_GPSR1,
		      GPSR1_WE0 |
		      GPSR1_CS0 |
		      GPSR1_A19 |
		      GPSR1_A18 |
		      GPSR1_A17 |
		      GPSR1_A16 |
		      GPSR1_A15 |
		      GPSR1_A14 |
		      GPSR1_A13 |
		      GPSR1_A12 |
		      GPSR1_A11 |
		      GPSR1_A10 |
		      GPSR1_A9 |
		      GPSR1_A8 |
		      GPSR1_A4 |
		      GPSR1_A3 |
		      GPSR1_A2 |
		      GPSR1_A1 |
		      GPSR1_A0);

	pfc_reg_write(PFC_GPSR2,
		      GPSR2_BIT27_REVERSED |
		      GPSR2_BIT26_REVERSED |
		      GPSR2_AVB_PHY_INT |
		      GPSR2_AVB_TXCREFCLK |
		      GPSR2_AVB_RD3 |
		      GPSR2_AVB_RD2 |
		      GPSR2_AVB_RD1 |
		      GPSR2_AVB_RD0 |
		      GPSR2_AVB_RXC |
		      GPSR2_AVB_RX_CTL |
		      GPSR2_RPC_RESET |
		      GPSR2_RPC_RPC_INT |
		      GPSR2_QSPI1_IO3 |
		      GPSR2_QSPI1_IO2 |
		      GPSR2_QSPI1_MISO_IO1 |
		      GPSR2_QSPI1_MOSI_IO0 |
		      GPSR2_QSPI0_SSL |
		      GPSR2_QSPI0_IO3 |
		      GPSR2_QSPI0_IO2 |
		      GPSR2_QSPI0_MISO_IO1 |
		      GPSR2_QSPI0_MOSI_IO0 |
		      GPSR2_QSPI0_SPCLK);

	pfc_reg_write(PFC_GPSR3,
		      GPSR3_SD0_CD |
		      GPSR3_SD1_DAT3 |
		      GPSR3_SD1_DAT2 |
		      GPSR3_SD1_DAT1 |
		      GPSR3_SD1_DAT0 |
		      GPSR3_SD1_CMD |
		      GPSR3_SD1_CLK |
		      GPSR3_SD0_DAT3 |
		      GPSR3_SD0_DAT2 |
		      GPSR3_SD0_DAT1 |
		      GPSR3_SD0_DAT0 |
		      GPSR3_SD0_CMD |
		      GPSR3_SD0_CLK);

	pfc_reg_write(PFC_GPSR4,
		      GPSR4_SD3_DAT3 |
		      GPSR4_SD3_DAT2 |
		      GPSR4_SD3_DAT1 |
		      GPSR4_SD3_DAT0 |
		      GPSR4_SD3_CMD |
		      GPSR4_SD3_CLK);

	pfc_reg_write(PFC_GPSR5,
		      GPSR5_MLB_SIG |
		      GPSR5_MLB_CLK |
		      GPSR5_SSI_SDATA9 |
		      GPSR5_MSIOF0_SS2 |
		      GPSR5_MSIOF0_SS1 |
		      GPSR5_MSIOF0_SYNC |
		      GPSR5_MSIOF0_TXD |
		      GPSR5_MSIOF0_RXD |
		      GPSR5_MSIOF0_SCK |
		      GPSR5_RX2_A |
		      GPSR5_TX2_A |
		      GPSR5_RTS0_A |
		      GPSR5_SCK0_A);

	pfc_reg_write(PFC_GPSR6,
		      GPSR6_USB30_PWEN |
		      GPSR6_SSI_SDATA6 |
		      GPSR6_SSI_WS6 |
		      GPSR6_SSI_SCK6 |
		      GPSR6_SSI_SDATA5 |
		      GPSR6_SSI_SCK5 |
		      GPSR6_SSI_SDATA4 |
		      GPSR6_USB30_OVC |
		      GPSR6_AUDIO_CLKA |
		      GPSR6_SSI_SDATA3 |
		      GPSR6_SSI_WS349 |
		      GPSR6_SSI_SCK349 |
		      GPSR6_SSI_SDATA0 |
		      GPSR6_SSI_WS01239 |
		      GPSR6_SSI_SCK01239);

	/* initialize POC control */
	reg = mmio_read_32(PFC_POCCTRL0);
	reg = (reg & POCCTRL0_MASK) |
	      POC_SD1_DAT3_33V |
	      POC_SD1_DAT2_33V |
	      POC_SD1_DAT1_33V |
	      POC_SD1_DAT0_33V |
	      POC_SD1_CMD_33V |
	      POC_SD1_CLK_33V |
	      POC_SD0_DAT3_33V |
	      POC_SD0_DAT2_33V |
	      POC_SD0_DAT1_33V |
	      POC_SD0_DAT0_33V |
	      POC_SD0_CMD_33V |
	      POC_SD0_CLK_33V;
	pfc_reg_write(PFC_POCCTRL0, reg);

	reg = mmio_read_32(PFC_POCCTRL2);
	reg = ((reg & POCCTRL2_MASK) & ~POC2_VREF_33V);
	pfc_reg_write(PFC_POCCTRL2, reg);

	/* initialize LSI pin pull-up/down control */
	pfc_reg_write(PFC_PUD0, 0x00080000U);
	pfc_reg_write(PFC_PUD1, 0xCE398464U);
	pfc_reg_write(PFC_PUD2, 0xA4C380F4U);
	pfc_reg_write(PFC_PUD3, 0x0000079FU);
	pfc_reg_write(PFC_PUD4, 0xFFF0FFFFU);
	pfc_reg_write(PFC_PUD5, 0x40000000U);

	/* initialize LSI pin pull-enable register */
	pfc_reg_write(PFC_PUEN0, 0x00000000U);
	pfc_reg_write(PFC_PUEN1, 0x00300000U);
	pfc_reg_write(PFC_PUEN2, 0x00400074U);
	pfc_reg_write(PFC_PUEN3, 0x00000000U);
	pfc_reg_write(PFC_PUEN4, 0x07900600U);
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
	mmio_write_32(GPIO_IOINTSEL0, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL1, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL2, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL3, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL4, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL5, 0x00000000U);
	mmio_write_32(GPIO_IOINTSEL6, 0x00000000U);

	/* initialize general output register */
	mmio_write_32(GPIO_OUTDT0, 0x00000000U);
	mmio_write_32(GPIO_OUTDT1, 0x00000000U);
	mmio_write_32(GPIO_OUTDT2, 0x00000000U);
	mmio_write_32(GPIO_OUTDT3, 0x00006000U);
	mmio_write_32(GPIO_OUTDT5, 0x00000000U);
	mmio_write_32(GPIO_OUTDT6, 0x00000000U);

	/* initialize general input/output switching */
	mmio_write_32(GPIO_INOUTSEL0, 0x00020000U);
	mmio_write_32(GPIO_INOUTSEL1, 0x00100000U);
	mmio_write_32(GPIO_INOUTSEL2, 0x03000000U);
	mmio_write_32(GPIO_INOUTSEL3, 0x0000E000U);
	mmio_write_32(GPIO_INOUTSEL4, 0x00000440U);
	mmio_write_32(GPIO_INOUTSEL5, 0x00080000U);
	mmio_write_32(GPIO_INOUTSEL6, 0x00000010U);
}

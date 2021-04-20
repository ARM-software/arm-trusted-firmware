/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <mvebu.h>
#include <stdbool.h>
#include "dfx.h"

/* #define DEBUG_DFX */
#ifdef DEBUG_DFX
#define debug(format...) NOTICE(format)
#else
#define debug(format, arg...)
#endif

#define TSEN_CTRL0			0xf06f8084
 #define TSEN_CTRL0_START		BIT(0)
 #define TSEN_CTRL0_RESET		BIT(1)
 #define TSEN_CTRL0_ENABLE		BIT(2)
 #define TSEN_CTRL0_AVG_BYPASS		BIT(6)
 #define TSEN_CTRL0_CHAN_SHIFT		13
 #define TSEN_CTRL0_CHAN_MASK		0xF
 #define TSEN_CTRL0_OSR_SHIFT		24
 #define TSEN_CTRL0_OSR_MAX		0x3
 #define TSEN_CTRL0_MODE_SHIFT		30
 #define TSEN_CTRL0_MODE_EXTERNAL	0x2U
 #define TSEN_CTRL0_MODE_MASK		0x3U

#define TSEN_CTRL1			0xf06f8088
 #define TSEN_CTRL1_INT_EN		BIT(25)
 #define TSEN_CTRL1_HYST_SHIFT		19
 #define TSEN_CTRL1_HYST_MASK		(0x3 << TSEN_CTRL1_HYST_SHIFT)
 #define TSEN_CTRL1_THRESH_SHIFT	3
 #define TSEN_CTRL1_THRESH_MASK		(0x3ff << TSEN_CTRL1_THRESH_SHIFT)

#define TSEN_STATUS			0xf06f808c
 #define TSEN_STATUS_VALID_OFFSET	16
 #define TSEN_STATUS_VALID_MASK		(0x1 << TSEN_STATUS_VALID_OFFSET)
 #define TSEN_STATUS_TEMP_OUT_OFFSET	0
 #define TSEN_STATUS_TEMP_OUT_MASK	(0x3FF << TSEN_STATUS_TEMP_OUT_OFFSET)

#define DFX_SERVER_IRQ_SUM_MASK_REG	0xf06f8104
 #define DFX_SERVER_IRQ_EN		BIT(1)

#define DFX_IRQ_CAUSE_REG		0xf06f8108

#define DFX_IRQ_MASK_REG		0xf06f810c
 #define DFX_IRQ_TSEN_OVERHEAT_OFFSET	BIT(22)

#define THERMAL_SEN_OUTPUT_MSB		512
#define THERMAL_SEN_OUTPUT_COMP		1024

#define COEF_M 423
#define COEF_B -150000LL

static void armada_ap806_thermal_read(u_register_t *temp)
{
	uint32_t reg;

	reg = mmio_read_32(TSEN_STATUS);

	reg = ((reg & TSEN_STATUS_TEMP_OUT_MASK) >>
	      TSEN_STATUS_TEMP_OUT_OFFSET);

	/*
	 * TSEN output format is signed as a 2s complement number
	 * ranging from-512 to +511. when MSB is set, need to
	 * calculate the complement number
	 */
	if (reg >= THERMAL_SEN_OUTPUT_MSB)
		reg -= THERMAL_SEN_OUTPUT_COMP;

	*temp = ((COEF_M * ((signed int)reg)) - COEF_B);
}

static void armada_ap806_thermal_irq(void)
{
	/* Dummy read, register ROC */
	mmio_read_32(DFX_IRQ_CAUSE_REG);
}

static void armada_ap806_thermal_overheat_irq_init(void)
{
	uint32_t reg;

	/* Clear DFX temperature IRQ cause */
	reg = mmio_read_32(DFX_IRQ_CAUSE_REG);

	/* Enable DFX Temperature IRQ */
	reg = mmio_read_32(DFX_IRQ_MASK_REG);
	reg |= DFX_IRQ_TSEN_OVERHEAT_OFFSET;
	mmio_write_32(DFX_IRQ_MASK_REG, reg);

	/* Enable DFX server IRQ */
	reg = mmio_read_32(DFX_SERVER_IRQ_SUM_MASK_REG);
	reg |= DFX_SERVER_IRQ_EN;
	mmio_write_32(DFX_SERVER_IRQ_SUM_MASK_REG, reg);

	/* Enable overheat interrupt */
	reg = mmio_read_32(TSEN_CTRL1);
	reg |= TSEN_CTRL1_INT_EN;
	mmio_write_32(TSEN_CTRL1, reg);
}

static unsigned int armada_mc_to_reg_temp(unsigned int temp_mc)
{
	unsigned int sample;

	sample = (temp_mc + COEF_B) / COEF_M;

	return sample & 0x3ff;
}

/*
 * The documentation states:
 * high/low watermark = threshold +/- 0.4761 * 2^(hysteresis + 2)
 * which is the mathematical derivation for:
 * 0x0 <=> 1.9°C, 0x1 <=> 3.8°C, 0x2 <=> 7.6°C, 0x3 <=> 15.2°C
 */
static unsigned int hyst_levels_mc[] = {1900, 3800, 7600, 15200};

static unsigned int armada_mc_to_reg_hyst(int hyst_mc)
{
	int i;

	/*
	 * We will always take the smallest possible hysteresis to avoid risking
	 * the hardware integrity by enlarging the threshold by +8°C in the
	 * worst case.
	 */
	for (i = ARRAY_SIZE(hyst_levels_mc) - 1; i > 0; i--)
		if (hyst_mc >= hyst_levels_mc[i])
			break;

	return i;
}

static void armada_ap806_thermal_threshold(int thresh_mc, int hyst_mc)
{
	uint32_t ctrl1;
	unsigned int threshold = armada_mc_to_reg_temp(thresh_mc);
	unsigned int hysteresis = armada_mc_to_reg_hyst(hyst_mc);

	ctrl1 = mmio_read_32(TSEN_CTRL1);
	/* Set Threshold */
	if (thresh_mc >= 0) {
		ctrl1 &= ~(TSEN_CTRL1_THRESH_MASK);
		ctrl1 |= threshold << TSEN_CTRL1_THRESH_SHIFT;
	}

	/* Set Hysteresis */
	if (hyst_mc >= 0) {
		ctrl1 &= ~(TSEN_CTRL1_HYST_MASK);
		ctrl1 |= hysteresis << TSEN_CTRL1_HYST_SHIFT;
	}

	mmio_write_32(TSEN_CTRL1, ctrl1);
}

static void armada_select_channel(int channel)
{
	uint32_t ctrl0;

	/* Stop the measurements */
	ctrl0 = mmio_read_32(TSEN_CTRL0);
	ctrl0 &= ~TSEN_CTRL0_START;
	mmio_write_32(TSEN_CTRL0, ctrl0);

	/* Reset the mode, internal sensor will be automatically selected */
	ctrl0 &= ~(TSEN_CTRL0_MODE_MASK << TSEN_CTRL0_MODE_SHIFT);

	/* Other channels are external and should be selected accordingly */
	if (channel) {
		/* Change the mode to external */
		ctrl0 |= TSEN_CTRL0_MODE_EXTERNAL <<
			 TSEN_CTRL0_MODE_SHIFT;
		/* Select the sensor */
		ctrl0 &= ~(TSEN_CTRL0_CHAN_MASK << TSEN_CTRL0_CHAN_SHIFT);
		ctrl0 |= (channel - 1) << TSEN_CTRL0_CHAN_SHIFT;
	}

	/* Actually set the mode/channel */
	mmio_write_32(TSEN_CTRL0, ctrl0);

	/* Re-start the measurements */
	ctrl0 |= TSEN_CTRL0_START;
	mmio_write_32(TSEN_CTRL0, ctrl0);
}

static void armada_ap806_thermal_init(void)
{
	uint32_t reg;

	reg = mmio_read_32(TSEN_CTRL0);
	reg &= ~TSEN_CTRL0_RESET;
	reg |= TSEN_CTRL0_START | TSEN_CTRL0_ENABLE;

	/* Sample every ~2ms */
	reg |= TSEN_CTRL0_OSR_MAX << TSEN_CTRL0_OSR_SHIFT;

	/* Enable average (2 samples by default) */
	reg &= ~TSEN_CTRL0_AVG_BYPASS;

	mmio_write_32(TSEN_CTRL0, reg);

	debug("thermal: Initialization done\n");
}

static void armada_is_valid(u_register_t *read)
{
	*read = (mmio_read_32(TSEN_STATUS) & TSEN_STATUS_VALID_MASK);
}

int mvebu_dfx_thermal_handle(u_register_t func, u_register_t *read,
			     u_register_t x2, u_register_t x3)
{
	debug_enter();

	switch (func) {
	case MV_SIP_DFX_THERMAL_INIT:
		armada_ap806_thermal_init();
		break;
	case MV_SIP_DFX_THERMAL_READ:
		armada_ap806_thermal_read(read);
		break;
	case MV_SIP_DFX_THERMAL_IRQ:
		armada_ap806_thermal_irq();
		break;
	case MV_SIP_DFX_THERMAL_THRESH:
		armada_ap806_thermal_threshold(x2, x3);
		armada_ap806_thermal_overheat_irq_init();
		break;
	case MV_SIP_DFX_THERMAL_IS_VALID:
		armada_is_valid(read);
		break;
	case MV_SIP_DFX_THERMAL_SEL_CHANNEL:
		armada_select_channel(x2);
		break;
	default:
		ERROR("unsupported dfx func\n");
		return -EINVAL;
	}

	debug_exit();

	return 0;
}

/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <hi3660.h>
#include "hikey960_private.h"

#define ADC_ADCIN0				0
#define ADC_ADCIN1				1
#define ADC_ADCIN2				2

#define HKADC_DATA_GRADE0			0
#define HKADC_DATA_GRADE1			100
#define HKADC_DATA_GRADE2			300
#define HKADC_DATA_GRADE3			500
#define HKADC_DATA_GRADE4			700
#define HKADC_DATA_GRADE5			900
#define HKADC_DATA_GRADE6			1100
#define HKADC_DATA_GRADE7			1300
#define HKADC_DATA_GRADE8			1500
#define HKADC_DATA_GRADE9			1700
#define HKADC_DATA_GRADE10			1800

#define BOARDID_VALUE0				0
#define BOARDID_VALUE1				1
#define BOARDID_VALUE2				2
#define BOARDID_VALUE3				3
#define BOARDID_VALUE4				4
#define BOARDID_VALUE5				5
#define BOARDID_VALUE6				6
#define BOARDID_VALUE7				7
#define BOARDID_VALUE8				8
#define BOARDID_VALUE9				9
#define BOARDID_UNKNOWN				0xF

#define BOARDID3_BASE				5


static void init_adc(void)
{
	/* reset hkadc */
	mmio_write_32(CRG_PERRSTEN2_REG, PERRSTEN2_HKADCSSI);
	/* wait a few clock cycles */
	udelay(2);
	mmio_write_32(CRG_PERRSTDIS2_REG, PERRSTEN2_HKADCSSI);
	udelay(2);
	/* enable hkadc clock */
	mmio_write_32(CRG_PERDIS2_REG, PEREN2_HKADCSSI);
	udelay(2);
	mmio_write_32(CRG_PEREN2_REG, PEREN2_HKADCSSI);
	udelay(2);
}

static int get_adc(unsigned int channel, unsigned int *value)
{
	unsigned int	data, value1, value0;

	if (channel > HKADC_CHANNEL_MAX) {
		WARN("invalid channel:%d\n", channel);
		return -EFAULT;
	}
	/* configure the read/write operation for external HKADC */
	mmio_write_32(HKADC_WR01_DATA_REG, HKADC_WR01_VALUE | channel);
	mmio_write_32(HKADC_WR23_DATA_REG, HKADC_WR23_VALUE);
	mmio_write_32(HKADC_WR45_DATA_REG, HKADC_WR45_VALUE);
	/* configure the number of accessing registers */
	mmio_write_32(HKADC_WR_NUM_REG, HKADC_WR_NUM_VALUE);
	/* configure delay of accessing registers */
	mmio_write_32(HKADC_DELAY01_REG, HKADC_CHANNEL0_DELAY01_VALUE);
	mmio_write_32(HKADC_DELAY23_REG, HKADC_DELAY23_VALUE);

	/* start HKADC */
	mmio_write_32(HKADC_DSP_START_REG, 1);
	do {
		data = mmio_read_32(HKADC_DSP_START_REG);
	} while (data & 1);

	/* convert AD result */
	value1 = mmio_read_32(HKADC_DSP_RD2_DATA_REG) & 0xffff;
	value0 = mmio_read_32(HKADC_DSP_RD3_DATA_REG) & 0xffff;

	data = ((value1 << 4) & HKADC_VALUE_HIGH) |
	       ((value0 >> 4) & HKADC_VALUE_LOW);
	*value = data;
	return 0;
}

static int get_value(unsigned int channel, unsigned int *value)
{
	int ret;

	ret = get_adc(channel, value);
	if (ret)
		return ret;

	/* convert ADC value to micro-volt */
	ret = ((*value & HKADC_VALID_VALUE) * HKADC_VREF_1V8) / HKADC_ACCURACY;
	*value = ret;
	return 0;
}

static int adcin_data_remap(unsigned int adcin_value)
{
	int	ret;

	if (adcin_value < HKADC_DATA_GRADE1)
		ret = BOARDID_VALUE0;
	else if (adcin_value < HKADC_DATA_GRADE2)
		ret = BOARDID_VALUE1;
	else if (adcin_value < HKADC_DATA_GRADE3)
		ret = BOARDID_VALUE2;
	else if (adcin_value < HKADC_DATA_GRADE4)
		ret = BOARDID_VALUE3;
	else if (adcin_value < HKADC_DATA_GRADE5)
		ret = BOARDID_VALUE4;
	else if (adcin_value < HKADC_DATA_GRADE6)
		ret = BOARDID_VALUE5;
	else if (adcin_value < HKADC_DATA_GRADE7)
		ret = BOARDID_VALUE6;
	else if (adcin_value < HKADC_DATA_GRADE8)
		ret = BOARDID_VALUE7;
	else if (adcin_value < HKADC_DATA_GRADE9)
		ret = BOARDID_VALUE8;
	else if (adcin_value < HKADC_DATA_GRADE10)
		ret = BOARDID_VALUE9;
	else
		ret = BOARDID_UNKNOWN;
	return ret;
}

int hikey960_read_boardid(unsigned int *id)
{
	unsigned int	adcin0, adcin1, adcin2;
	unsigned int	adcin0_remap, adcin1_remap, adcin2_remap;

	assert(id != NULL);

	init_adc();

	/* read ADC channel0 data */
	get_value(ADC_ADCIN0, &adcin0);
	adcin0_remap = adcin_data_remap(adcin0);
	if (adcin0_remap == BOARDID_UNKNOWN)
		return -EINVAL;
	/* read ADC channel1 data */
	get_value(ADC_ADCIN1, &adcin1);
	adcin1_remap = adcin_data_remap(adcin1);
	if (adcin1_remap == BOARDID_UNKNOWN)
		return -EINVAL;
	/* read ADC channel2 data */
	get_value(ADC_ADCIN2, &adcin2);
	adcin2_remap = adcin_data_remap(adcin2);
	if (adcin2_remap == BOARDID_UNKNOWN)
		return -EINVAL;
	*id = BOARDID3_BASE * 1000 + (adcin2_remap * 100) +
		(adcin1_remap * 10) + adcin0_remap;
	return 0;
}

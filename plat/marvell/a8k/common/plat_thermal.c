/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/marvell/thermal.h>
#include <lib/mmio.h>

#include <mvebu_def.h>

#define THERMAL_TIMEOUT					1200

#define THERMAL_SEN_CTRL_LSB_STRT_OFFSET		0
#define THERMAL_SEN_CTRL_LSB_STRT_MASK			\
				(0x1 << THERMAL_SEN_CTRL_LSB_STRT_OFFSET)
#define THERMAL_SEN_CTRL_LSB_RST_OFFSET			1
#define THERMAL_SEN_CTRL_LSB_RST_MASK			\
				(0x1 << THERMAL_SEN_CTRL_LSB_RST_OFFSET)
#define THERMAL_SEN_CTRL_LSB_EN_OFFSET			2
#define THERMAL_SEN_CTRL_LSB_EN_MASK			\
				(0x1 << THERMAL_SEN_CTRL_LSB_EN_OFFSET)

#define THERMAL_SEN_CTRL_STATS_VALID_OFFSET		16
#define THERMAL_SEN_CTRL_STATS_VALID_MASK		\
				(0x1 << THERMAL_SEN_CTRL_STATS_VALID_OFFSET)
#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET		0
#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK		\
			(0x3FF << THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET)

#define THERMAL_SEN_OUTPUT_MSB				512
#define THERMAL_SEN_OUTPUT_COMP				1024

struct tsen_regs {
	uint32_t ext_tsen_ctrl_lsb;
	uint32_t ext_tsen_ctrl_msb;
	uint32_t ext_tsen_status;
};

static int ext_tsen_probe(struct tsen_config *tsen_cfg)
{
	uint32_t reg, timeout = 0;
	struct tsen_regs *base;

	if (tsen_cfg == NULL && tsen_cfg->regs_base == NULL) {
		ERROR("initial thermal sensor configuration is missing\n");
		return -1;
	}
	base = (struct tsen_regs *)tsen_cfg->regs_base;

	INFO("initializing thermal sensor\n");

	/* initialize thermal sensor hardware reset once */
	reg = mmio_read_32((uintptr_t)&base->ext_tsen_ctrl_lsb);
	reg &= ~THERMAL_SEN_CTRL_LSB_RST_OFFSET; /* de-assert TSEN_RESET */
	reg |= THERMAL_SEN_CTRL_LSB_EN_MASK; /* set TSEN_EN to 1 */
	reg |= THERMAL_SEN_CTRL_LSB_STRT_MASK; /* set TSEN_START to 1 */
	mmio_write_32((uintptr_t)&base->ext_tsen_ctrl_lsb, reg);

	reg = mmio_read_32((uintptr_t)&base->ext_tsen_status);
	while ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0 &&
	       timeout < THERMAL_TIMEOUT) {
		udelay(100);
		reg = mmio_read_32((uintptr_t)&base->ext_tsen_status);
		timeout++;
	}

	if ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0) {
		ERROR("thermal sensor is not ready\n");
		return -1;
	}

	tsen_cfg->tsen_ready = 1;

	VERBOSE("thermal sensor was initialized\n");

	return 0;
}

static int ext_tsen_read(struct tsen_config *tsen_cfg, int *temp)
{
	uint32_t reg;
	struct tsen_regs *base;

	if (tsen_cfg == NULL && !tsen_cfg->tsen_ready) {
		ERROR("thermal sensor was not initialized\n");
		return -1;
	}
	base = (struct tsen_regs *)tsen_cfg->regs_base;

	reg = mmio_read_32((uintptr_t)&base->ext_tsen_status);
	reg = ((reg & THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK) >>
		THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET);

	/*
	 * TSEN output format is signed as a 2s complement number
	 * ranging from-512 to +511. when MSB is set, need to
	 * calculate the complement number
	 */
	if (reg >= THERMAL_SEN_OUTPUT_MSB)
		reg -= THERMAL_SEN_OUTPUT_COMP;

	if (tsen_cfg->tsen_divisor == 0) {
		ERROR("thermal sensor divisor cannot be zero\n");
		return -1;
	}

	*temp = ((tsen_cfg->tsen_gain * ((int)reg)) +
		 tsen_cfg->tsen_offset) / tsen_cfg->tsen_divisor;

	return 0;
}

static struct tsen_config tsen_cfg = {
	.tsen_offset = 153400,
	.tsen_gain = 425,
	.tsen_divisor = 1000,
	.tsen_ready = 0,
	.regs_base = (void *)MVEBU_AP_EXT_TSEN_BASE,
	.ptr_tsen_probe = ext_tsen_probe,
	.ptr_tsen_read = ext_tsen_read
};

struct tsen_config *marvell_thermal_config_get(void)
{
	return &tsen_cfg;
}

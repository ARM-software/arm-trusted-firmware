/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mcucfg.h>
#include <mt_timer.h>
#include <platform_def.h>

static void enable_systimer_compensation(void)
{
	unsigned int reg;

	reg = mmio_read_32(CNTCR_REG);
	reg &= ~COMP_15_EN;
	reg |= COMP_20_EN;
	mmio_write_32(CNTCR_REG, reg);

	NOTICE("[systimer] CNTCR_REG(0x%x)\n", mmio_read_32(CNTCR_REG));
}

void mt_systimer_init(void)
{
	/* systimer is default on, so we only enable systimer compensation */
	enable_systimer_compensation();
}

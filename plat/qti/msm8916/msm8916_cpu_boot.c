/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <msm8916_mmap.h>
#include "msm8916_pm.h"

#define CPU_PWR_CTL			0x4
#define APC_PWR_GATE_CTL		0x14

#define CPU_PWR_CTL_CLAMP		BIT_32(0)
#define CPU_PWR_CTL_CORE_MEM_CLAMP	BIT_32(1)
#define CPU_PWR_CTL_L1_RST_DIS		BIT_32(2)
#define CPU_PWR_CTL_CORE_MEM_HS		BIT_32(3)
#define CPU_PWR_CTL_CORE_RST		BIT_32(4)
#define CPU_PWR_CTL_COREPOR_RST		BIT_32(5)
#define CPU_PWR_CTL_GATE_CLK		BIT_32(6)
#define CPU_PWR_CTL_CORE_PWRD_UP	BIT_32(7)

#define APC_PWR_GATE_CTL_GHDS_EN	BIT_32(0)
#define APC_PWR_GATE_CTL_GHDS_CNT(cnt)	((cnt) << 24)

/* Boot a secondary CPU core for the first time. */
void msm8916_cpu_boot(unsigned int core)
{
	uintptr_t acs = APCS_ALIAS_ACS(core);
	uint32_t pwr_ctl;

	pwr_ctl = CPU_PWR_CTL_CLAMP | CPU_PWR_CTL_CORE_MEM_CLAMP |
		  CPU_PWR_CTL_CORE_RST | CPU_PWR_CTL_COREPOR_RST;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();

	mmio_write_32(acs + APC_PWR_GATE_CTL, APC_PWR_GATE_CTL_GHDS_EN |
		      APC_PWR_GATE_CTL_GHDS_CNT(16));
	dsb();
	udelay(2);

	pwr_ctl &= ~CPU_PWR_CTL_CORE_MEM_CLAMP;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();

	pwr_ctl |= CPU_PWR_CTL_CORE_MEM_HS;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();
	udelay(2);

	pwr_ctl &= ~CPU_PWR_CTL_CLAMP;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();
	udelay(2);

	pwr_ctl &= ~(CPU_PWR_CTL_CORE_RST | CPU_PWR_CTL_COREPOR_RST);
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();

	pwr_ctl |= CPU_PWR_CTL_CORE_PWRD_UP;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();
}

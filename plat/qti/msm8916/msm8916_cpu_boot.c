/*
 * Copyright (c) 2021-2022, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

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

#define PWR_CTL_OVERRIDE		0xc
#define L2_PWR_CTL			0x14
#define L2_PWR_STATUS			0x18
#define CORE_CBCR			0x58

#define PWR_CTL_OVERRIDE_PRESETDBG	BIT_32(22)

#define L2_PWR_CTL_L2_ARRAY_HS		BIT_32(0)
#define L2_PWR_CTL_SCU_ARRAY_HS		BIT_32(1)
#define L2_PWR_CTL_L2_RST_DIS		BIT_32(2)
#define L2_PWR_CTL_L2_HS_CLAMP		BIT_32(8)
#define L2_PWR_CTL_L2_HS_EN		BIT_32(9)
#define L2_PWR_CTL_L2_HS_RST		BIT_32(10)
#define L2_PWR_CTL_L2_SLEEP_STATE	BIT_32(11)
#define L2_PWR_CTL_SYS_RESET		BIT_32(12)
#define L2_PWR_CTL_L2_RET_SLP		BIT_32(13)
#define L2_PWR_CTL_SCU_ARRAY_HS_CLAMP	BIT_32(14)
#define L2_PWR_CTL_L2_ARRAY_HS_CLAMP	BIT_32(15)
#define L2_PWR_CTL_L2_HS_CNT(cnt)	((cnt) << 16)
#define L2_PWR_CTL_PMIC_APC_ON		BIT_32(28)

#define L2_PWR_STATUS_L2_HS_STS		BIT_32(9)

#define CORE_CBCR_CLK_ENABLE		BIT_32(0)
#define CORE_CBCR_HW_CTL		BIT_32(1)

/* Boot a secondary CPU core for the first time. */
void msm8916_cpu_boot(uintptr_t acs)
{
	uint32_t pwr_ctl;

	VERBOSE("PSCI: Powering on CPU @ 0x%08lx\n", acs);

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

/* Power on cluster L2 cache for the first time. */
void msm8916_l2_boot(uintptr_t base)
{
	uint32_t pwr_ctl, cbcr, ovr;

	/* Skip if cluster L2 is already powered on */
	if (mmio_read_32(base + L2_PWR_STATUS) & L2_PWR_STATUS_L2_HS_STS) {
		VERBOSE("PSCI: L2 cache @ 0x%08lx is already powered on\n", base);
		return;
	}

	VERBOSE("PSCI: Powering on L2 cache @ 0x%08lx\n", base);

	pwr_ctl = L2_PWR_CTL_L2_HS_CLAMP | L2_PWR_CTL_L2_HS_EN |
		  L2_PWR_CTL_L2_HS_RST | L2_PWR_CTL_SYS_RESET |
		  L2_PWR_CTL_SCU_ARRAY_HS_CLAMP | L2_PWR_CTL_L2_ARRAY_HS_CLAMP |
		  L2_PWR_CTL_L2_HS_CNT(16);
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);

	ovr = PWR_CTL_OVERRIDE_PRESETDBG;
	mmio_write_32(base + PWR_CTL_OVERRIDE, ovr);
	dsb();
	udelay(2);

	pwr_ctl &= ~(L2_PWR_CTL_SCU_ARRAY_HS_CLAMP |
		     L2_PWR_CTL_L2_ARRAY_HS_CLAMP);
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);

	pwr_ctl |= (L2_PWR_CTL_L2_ARRAY_HS | L2_PWR_CTL_SCU_ARRAY_HS);
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);
	dsb();
	udelay(2);

	cbcr = CORE_CBCR_CLK_ENABLE;
	mmio_write_32(base + CORE_CBCR, cbcr);

	pwr_ctl &= ~L2_PWR_CTL_L2_HS_CLAMP;
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);
	dsb();
	udelay(2);

	ovr &= ~PWR_CTL_OVERRIDE_PRESETDBG;
	mmio_write_32(base + PWR_CTL_OVERRIDE, ovr);

	pwr_ctl &= ~(L2_PWR_CTL_L2_HS_RST | L2_PWR_CTL_SYS_RESET);
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);
	dsb();
	udelay(54);

	pwr_ctl |= L2_PWR_CTL_PMIC_APC_ON;
	mmio_write_32(base + L2_PWR_CTL, pwr_ctl);

	cbcr |= CORE_CBCR_HW_CTL;
	mmio_write_32(base + CORE_CBCR, cbcr);
	dsb();
}

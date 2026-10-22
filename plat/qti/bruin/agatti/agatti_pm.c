/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_plat.h>

/* Per-core power controller (APSS_ACS) registers */
#define CPU_PWR_CTL			0x4
#define APC_PWR_GATE_CTL		0x14

#define CPU_PWR_CTL_CLAMP		BIT_32(0)
#define CPU_PWR_CTL_CORE_MEM_CLAMP	BIT_32(1)
#define CPU_PWR_CTL_CORE_MEM_HS		BIT_32(3)
#define CPU_PWR_CTL_CORE_RST		BIT_32(4)
#define CPU_PWR_CTL_COREPOR_RST		BIT_32(5)
#define CPU_PWR_CTL_CORE_PWRD_UP	BIT_32(7)
#define CPU_PWR_CTL_CORE_MEM_RET_N	BIT_32(9)

#define APC_PWR_GATE_CTL_HS_EN		BIT_32(0)
#define APC_PWR_GATE_CTL_HS_CNT(cnt)	((cnt) << 24)

static void agatti_cpu_boot(uintptr_t acs)
{
	uint32_t pwr_ctl;

	pwr_ctl = mmio_read_32(acs + CPU_PWR_CTL);
	if ((pwr_ctl & CPU_PWR_CTL_CORE_PWRD_UP) != 0U) {
		/* The core is still powered: pulse its resets. */
		pwr_ctl |= CPU_PWR_CTL_CORE_RST | CPU_PWR_CTL_COREPOR_RST;
		mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
		dsb();
		udelay(6);

		pwr_ctl &= ~(CPU_PWR_CTL_CORE_RST | CPU_PWR_CTL_COREPOR_RST);
		mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
		dsb();
		return;
	}

	pwr_ctl = CPU_PWR_CTL_CLAMP | CPU_PWR_CTL_CORE_MEM_CLAMP |
		  CPU_PWR_CTL_CORE_RST | CPU_PWR_CTL_COREPOR_RST;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();

	mmio_write_32(acs + APC_PWR_GATE_CTL, APC_PWR_GATE_CTL_HS_EN |
		      APC_PWR_GATE_CTL_HS_CNT(16));
	dsb();
	udelay(2);

	pwr_ctl &= ~CPU_PWR_CTL_CORE_MEM_CLAMP;
	mmio_write_32(acs + CPU_PWR_CTL, pwr_ctl);
	dsb();

	pwr_ctl |= CPU_PWR_CTL_CORE_MEM_HS | CPU_PWR_CTL_CORE_MEM_RET_N;
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

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)mpidr;

	agatti_cpu_boot(AGATTI_APCS_ALIAS_ACS(core_pos));

	return PSCI_E_SUCCESS;
}

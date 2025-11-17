// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#include <common/debug.h>

#include <dmc_ddrc_rv1126b.h>
#include <dmc_rv1126b.h>
#include <plat_private.h>
#include <platform_def.h>
#include <pmu.h>
#include <soc.h>

/* ddr phy registers define */
#define PHY_REG(n)			(DDRPHY_BASE + n)

static __pmusramdata uint32_t save_lp_stat;

static __pmusramfunc uint32_t low_power_update(uint32_t en)
{
	uint32_t lp_stat = 0;

	if (en == 1) {
		mmio_clrbits_32(PHY_REG(0x84), BIT(15));
		mmio_setbits_32(DDRCTL_BASE + DDRC_PWRCTL, en & 0xf);
	} else {
		lp_stat = mmio_read_32(DDRCTL_BASE + DDRC_PWRCTL) & 0xf;
		mmio_clrbits_32(DDRCTL_BASE + DDRC_PWRCTL, 0xf);
		mmio_setbits_32(PHY_REG(0x84), BIT(15));
	}

	return lp_stat;
}

__pmusramfunc void dmc_enter_lp(void)
{
	save_lp_stat = low_power_update(0);

	/* into self_refresh */
	mmio_write_32(PMU1_BASE + PMU_DDR_PWR_SFTCON, DDR_SREF_C_SFTENA);
	while (!(mmio_read_32(PMU1_BASE + PMU_DDR_STS) & DDRCTL_C_SYSACK))
		continue;
}

__pmusramfunc void dmc_restore(void)
{
	/* exit self_refresh */
	mmio_write_32(DDRGRF_BASE + DDRGRF_CON(21), CSYSREQ_DDRC_PMU);
	mmio_write_32(PMU1_BASE + PMU_DDR_PWR_SFTCON, DDR_SREF_C_SFTENA_DIS);
	while (mmio_read_32(PMU1_BASE + PMU_DDR_STS) & DDRCTL_C_SYSACK)
		continue;

	low_power_update(save_lp_stat);
}

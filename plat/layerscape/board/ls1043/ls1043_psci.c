/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <endian.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#define LS_SCFG_BASE			0x01570000
/* register to store warm boot entry, big endian, higher 32bit */
#define LS_SCFG_SCRATCHRW0_OFFSET	     0x600
/* register to store warm boot entry, big endian, lower 32bit */
#define LS_SCFG_SCRATCHRW1_OFFSET	     0x604
#define LS_SCFG_COREBCR_OFFSET		     0x680

#define LS_DCFG_BASE			0x01EE0000
#define LS_DCFG_RSTCR_OFFSET		     0x0B0
#define LS_DCFG_RSTRQMR1_OFFSET		     0x0C0
#define LS_DCFG_BRR_OFFSET		     0x0E4

#define LS_SCFG_CORE0_SFT_RST_OFFSET		0x130
#define LS_SCFG_CORE1_SFT_RST_OFFSET		0x134
#define LS_SCFG_CORE2_SFT_RST_OFFSET		0x138
#define LS_SCFG_CORE3_SFT_RST_OFFSET		0x13C

#define LS_SCFG_CORESRENCR_OFFSET		0x204

#define LS_SCFG_RVBAR0_0_OFFSET			0x220
#define LS_SCFG_RVBAR0_1_OFFSET			0x224

#define LS_SCFG_RVBAR1_0_OFFSET			0x228
#define LS_SCFG_RVBAR1_1_OFFSET			0x22C

#define LS_SCFG_RVBAR2_0_OFFSET			0x230
#define LS_SCFG_RVBAR2_1_OFFSET			0x234

#define LS_SCFG_RVBAR3_0_OFFSET			0x238
#define LS_SCFG_RVBAR3_1_OFFSET			0x23C

/* the entry for core warm boot */
static uintptr_t warmboot_entry;

/* warm reset single core */
static void ls1043_reset_core(int core_pos)
{
	assert(core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT);

	/* set 0 in RVBAR, boot from bootrom at 0x0 */
	mmio_write_32(LS_SCFG_BASE + LS_SCFG_RVBAR0_0_OFFSET + core_pos * 8,
		      0);
	mmio_write_32(LS_SCFG_BASE + LS_SCFG_RVBAR0_1_OFFSET + core_pos * 8,
		      0);

	dsb();
	/* enable core soft reset */
	mmio_write_32(LS_SCFG_BASE + LS_SCFG_CORESRENCR_OFFSET,
		      htobe32(1U << 31));
	dsb();
	isb();
	/* reset core */
	mmio_write_32(LS_SCFG_BASE + LS_SCFG_CORE0_SFT_RST_OFFSET +
			core_pos * 4, htobe32(1U << 31));
	mdelay(10);
}

static void __dead2 ls1043_system_reset(void)
{
	/* clear reset request mask bits */
	mmio_write_32(LS_DCFG_BASE + LS_DCFG_RSTRQMR1_OFFSET, 0);

	/* set reset request bit */
	mmio_write_32(LS_DCFG_BASE + LS_DCFG_RSTCR_OFFSET,
		      htobe32((uint32_t)0x2));

	/* system will reset; if fail, enter wfi */
	dsb();
	isb();
	wfi();

	panic();
}


static int ls1043_pwr_domain_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos_by_mpidr(mpidr);
	uint32_t core_mask, brr;

	assert(core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT);
	core_mask = 1 << core_pos;

	/* set warm boot entry */
	mmio_write_32(LS_SCFG_BASE + LS_SCFG_SCRATCHRW0_OFFSET,
		htobe32((uint32_t)(warmboot_entry >> 32)));

	mmio_write_32(LS_SCFG_BASE + LS_SCFG_SCRATCHRW1_OFFSET,
		htobe32((uint32_t)warmboot_entry));

	dsb();

	brr = be32toh(mmio_read_32(LS_DCFG_BASE + LS_DCFG_BRR_OFFSET));
	if (brr & core_mask) {
		/* core has been released, must reset it to restart */
		ls1043_reset_core(core_pos);

		/* set bit in core boot control register to enable boot */
		mmio_write_32(LS_SCFG_BASE + LS_SCFG_COREBCR_OFFSET,
			htobe32(core_mask));

	} else {
		/* set bit in core boot control register to enable boot */
		mmio_write_32(LS_SCFG_BASE + LS_SCFG_COREBCR_OFFSET,
			htobe32(core_mask));

		/* release core */
		mmio_write_32(LS_DCFG_BASE + LS_DCFG_BRR_OFFSET,
			      htobe32(brr | core_mask));
	}

	mdelay(20);

	/* wake core in case it is in wfe */
	dsb();
	isb();
	sev();

	return PSCI_E_SUCCESS;
}

static void ls1043_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* Per cpu gic distributor setup */
	gicv2_pcpu_distif_init();

	/* Enable the gic CPU interface */
	gicv2_cpuif_enable();
}

static void ls1043_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Disable the gic CPU interface */
	gicv2_cpuif_disable();
}

static plat_psci_ops_t ls1043_psci_pm_ops = {
	.system_reset = ls1043_system_reset,
	.pwr_domain_on = ls1043_pwr_domain_on,
	.pwr_domain_on_finish = ls1043_pwr_domain_on_finish,
	.pwr_domain_off = ls1043_pwr_domain_off,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	warmboot_entry = sec_entrypoint;
	*psci_ops = &ls1043_psci_pm_ops;
	return 0;
}

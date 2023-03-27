/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <sunxi_cpucfg.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#ifndef SUNXI_C0_CPU_CTRL_REG
#define SUNXI_C0_CPU_CTRL_REG(n)	0
#define SUNXI_CPU_UNK_REG(n)		0
#define SUNXI_CPU_CTRL_REG(n)		0
#endif

static void sunxi_cpu_disable_power(unsigned int cluster, unsigned int core)
{
	if (mmio_read_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core)) == 0xff)
		return;

	VERBOSE("PSCI: Disabling power to cluster %d core %d\n", cluster, core);

	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0xff);
}

static void sunxi_cpu_enable_power(unsigned int cluster, unsigned int core)
{
	if (mmio_read_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core)) == 0)
		return;

	VERBOSE("PSCI: Enabling power to cluster %d core %d\n", cluster, core);

	/* Power enable sequence from original Allwinner sources */
	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0xfe);
	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0xf8);
	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0xe0);
	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0x80);
	mmio_write_32(SUNXI_CPU_POWER_CLAMP_REG(cluster, core), 0x00);
	udelay(1);
}

/* We can't turn ourself off like this, but it works for other cores. */
static void sunxi_cpu_off(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int core    = MPIDR_AFFLVL0_VAL(mpidr);

	VERBOSE("PSCI: Powering off cluster %d core %d\n", cluster, core);

	if (sunxi_cpucfg_has_per_cluster_regs()) {
		/* Deassert DBGPWRDUP */
		mmio_clrbits_32(SUNXI_CPUCFG_DBG_REG0, BIT(core));
		/* Activate the core output clamps, but not for core 0. */
		if (core != 0) {
			mmio_setbits_32(SUNXI_POWEROFF_GATING_REG(cluster),
					BIT(core));
		}
		/* Assert CPU power-on reset */
		mmio_clrbits_32(SUNXI_POWERON_RST_REG(cluster), BIT(core));
		/* Remove power from the CPU */
		sunxi_cpu_disable_power(cluster, core);
	} else {
		/* power down(?) debug core */
		mmio_clrbits_32(SUNXI_C0_CPU_CTRL_REG(core), BIT(8));
		/* ??? Activate the core output clamps, but not for core 0 */
		if (core != 0) {
			mmio_setbits_32(SUNXI_CPU_UNK_REG(core), BIT(1));
		}
		/* ??? Assert CPU power-on reset ??? */
		mmio_clrbits_32(SUNXI_CPU_UNK_REG(core), BIT(0));
		/* Remove power from the CPU */
		sunxi_cpu_disable_power(cluster, core);
	}
}

void sunxi_cpu_on(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int core    = MPIDR_AFFLVL0_VAL(mpidr);

	VERBOSE("PSCI: Powering on cluster %d core %d\n", cluster, core);

	if (sunxi_cpucfg_has_per_cluster_regs()) {
		/* Assert CPU core reset */
		mmio_clrbits_32(SUNXI_CPUCFG_RST_CTRL_REG(cluster), BIT(core));
		/* Assert CPU power-on reset */
		mmio_clrbits_32(SUNXI_POWERON_RST_REG(cluster), BIT(core));
		/* Set CPU to start in AArch64 mode */
		mmio_setbits_32(SUNXI_AA64nAA32_REG(cluster),
				BIT(SUNXI_AA64nAA32_OFFSET + core));
		/* Apply power to the CPU */
		sunxi_cpu_enable_power(cluster, core);
		/* Release the core output clamps */
		mmio_clrbits_32(SUNXI_POWEROFF_GATING_REG(cluster), BIT(core));
		/* Deassert CPU power-on reset */
		mmio_setbits_32(SUNXI_POWERON_RST_REG(cluster), BIT(core));
		/* Deassert CPU core reset */
		mmio_setbits_32(SUNXI_CPUCFG_RST_CTRL_REG(cluster), BIT(core));
		/* Assert DBGPWRDUP */
		mmio_setbits_32(SUNXI_CPUCFG_DBG_REG0, BIT(core));
	} else {
		/* Assert CPU core reset */
		mmio_clrbits_32(SUNXI_C0_CPU_CTRL_REG(core), BIT(0));
		/* ??? Assert CPU power-on reset ??? */
		mmio_clrbits_32(SUNXI_CPU_UNK_REG(core), BIT(0));

		/* Set CPU to start in AArch64 mode */
		mmio_setbits_32(SUNXI_CPU_CTRL_REG(core), BIT(0));

		/* Apply power to the CPU */
		sunxi_cpu_enable_power(cluster, core);

		/* ??? Release the core output clamps ??? */
		mmio_clrbits_32(SUNXI_CPU_UNK_REG(core), BIT(1));
		/* ??? Deassert CPU power-on reset ??? */
		mmio_setbits_32(SUNXI_CPU_UNK_REG(core), BIT(0));
		/* Deassert CPU core reset */
		mmio_setbits_32(SUNXI_C0_CPU_CTRL_REG(core), BIT(0));
		/* power up(?) debug core */
		mmio_setbits_32(SUNXI_C0_CPU_CTRL_REG(core), BIT(8));
	}
}

void sunxi_cpu_power_off_others(void)
{
	u_register_t self = read_mpidr();
	unsigned int cluster;
	unsigned int core;

	for (cluster = 0; cluster < PLATFORM_CLUSTER_COUNT; ++cluster) {
		for (core = 0; core < PLATFORM_MAX_CPUS_PER_CLUSTER; ++core) {
			u_register_t mpidr = (cluster << MPIDR_AFF1_SHIFT) |
					     (core    << MPIDR_AFF0_SHIFT) |
					     BIT(31);
			if (mpidr != self)
				sunxi_cpu_off(mpidr);
		}
	}
}

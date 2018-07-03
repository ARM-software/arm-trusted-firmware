/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <sunxi_mmap.h>
#include <sunxi_cpucfg.h>
#include <utils_def.h>

#include "sunxi_private.h"

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
}

void sunxi_cpu_off(unsigned int cluster, unsigned int core)
{
	VERBOSE("PSCI: Powering off cluster %d core %d\n", cluster, core);

	/* Deassert DBGPWRDUP */
	mmio_clrbits_32(SUNXI_CPUCFG_DBG_REG0, BIT(core));
	/* Activate the core output clamps */
	mmio_setbits_32(SUNXI_POWEROFF_GATING_REG(cluster), BIT(core));
	/* Assert CPU power-on reset */
	mmio_clrbits_32(SUNXI_POWERON_RST_REG(cluster), BIT(core));
	/* Remove power from the CPU */
	sunxi_cpu_disable_power(cluster, core);
}

void sunxi_cpu_on(unsigned int cluster, unsigned int core)
{
	VERBOSE("PSCI: Powering on cluster %d core %d\n", cluster, core);

	/* Assert CPU core reset */
	mmio_clrbits_32(SUNXI_CPUCFG_RST_CTRL_REG(cluster), BIT(core));
	/* Assert CPU power-on reset */
	mmio_clrbits_32(SUNXI_POWERON_RST_REG(cluster), BIT(core));
	/* Set CPU to start in AArch64 mode */
	mmio_setbits_32(SUNXI_CPUCFG_CLS_CTRL_REG0(cluster), BIT(24 + core));
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
}

void sunxi_disable_secondary_cpus(unsigned int primary_cpu)
{
	for (unsigned int cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu += 1) {
		if (cpu == primary_cpu)
			continue;
		sunxi_cpu_off(cpu / PLATFORM_MAX_CPUS_PER_CLUSTER,
			       cpu % PLATFORM_MAX_CPUS_PER_CLUSTER);
	}
}

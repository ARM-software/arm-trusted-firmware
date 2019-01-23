/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/marvell/cache_llc.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#define CCU_HTC_ASET			(MVEBU_CCU_BASE(MVEBU_AP0) + 0x264)
#define MVEBU_IO_AFFINITY		(0xF00)
#define MVEBU_SF_REG			(MVEBU_REGS_BASE + 0x40)
#define MVEBU_SF_EN			BIT(8)

#ifdef MVEBU_SOC_AP807
static void plat_enable_snoop_filter(void)
{
	int cpu_id = plat_my_core_pos();

	/* Snoop filter needs to be enabled once per cluster */
	if (cpu_id % 2)
		return;

	mmio_setbits_32(MVEBU_SF_REG, MVEBU_SF_EN);
}
#endif

static void plat_enable_affinity(void)
{
	int cluster_id;
	int affinity;

	/* set CPU Affinity */
	cluster_id = plat_my_core_pos() / PLAT_MARVELL_CLUSTER_CORE_COUNT;
	affinity = (MVEBU_IO_AFFINITY | (1 << cluster_id));
	mmio_write_32(CCU_HTC_ASET, affinity);

	/* set barier */
	isb();
}

void marvell_psci_arch_init(int die_index)
{
#if LLC_ENABLE
	/* check if LLC is in exclusive mode
	 * as L2 is configured to UniqueClean eviction
	 * (in a8k reset handler)
	 */
	if (llc_is_exclusive(0) == 0)
		ERROR("LLC should be configured to exclusice mode\n");
#endif

	/* Enable Affinity */
	plat_enable_affinity();

#ifdef MVEBU_SOC_AP807
	plat_enable_snoop_filter();
#endif
}

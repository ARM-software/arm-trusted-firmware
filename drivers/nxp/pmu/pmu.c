/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <dcfg.h>
#include <lib/mmio.h>
#include <pmu.h>

void enable_timer_base_to_cluster(uintptr_t nxp_pmu_addr)
{
	uint32_t *cltbenr = NULL;
	uint32_t cltbenr_val = 0U;

	cltbenr = (uint32_t *)(nxp_pmu_addr
				+ CLUST_TIMER_BASE_ENBL_OFFSET);

	cltbenr_val = mmio_read_32((uintptr_t)cltbenr);

	cltbenr_val = cltbenr_val
			| (1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	mmio_write_32((uintptr_t)cltbenr, cltbenr_val);

	VERBOSE("Enable cluster time base\n");
}

/*
 * Enable core timebase.  In certain Layerscape SoCs, the clock for each core's
 * has an enable bit in the PMU Physical Core Time Base Enable
 * Register (PCTBENR), which allows the watchdog to operate.
 */

void enable_core_tb(uintptr_t nxp_pmu_addr)
{
	uint32_t *pctbenr = (uint32_t *) (nxp_pmu_addr +
					  CORE_TIMEBASE_ENBL_OFFSET);

	mmio_write_32((uintptr_t)pctbenr, 0xff);
}

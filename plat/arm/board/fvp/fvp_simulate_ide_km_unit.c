/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <plat/common/platform.h>

/*
 * This not a full IDE unit simulation.
 *
 * blocking mode:
 * FVP_SIMULATE_IDE_KM_UNIT == 1
 * Simulate blocking IDE KM operations. In this mode, these fvp_simulate_* call
 * randomnly returns -EBUSY to simulate an access to MMIO status register is
 * BUSY or not ready. So the caller can re-try the operation after some delay
 */

#define FVP_KMU_MAX		(PLAT_PCIE_ROOT_COMPLEX_MAX)

/*
 * This timeout is a heuristic value so that the caller can retry after adding
 * delay.
 */
#define IDE_KMU_BUSY_TIMEOUT_MS	(20)

struct fvp_km_unit {
	uint64_t call_count;
	uint64_t busy_cntpct;
};

struct fvp_km_unit fvp_km_units[FVP_KMU_MAX];

static inline uint64_t timeout_ms2cnt(uint32_t ms)
{
	return ((uint64_t)ms * (uint64_t)read_cntfrq_el0()) / 1000ULL;
}

static bool simulate_km_unit_busy(int rc_idx)
{
	if (read_cntpct_el0() < fvp_km_units[rc_idx].busy_cntpct) {
		return true;
	}

	/* update busy timeout by 50 ms for every 10th iteration */
	if ((fvp_km_units[rc_idx].call_count % 10) == 0) {
		fvp_km_units[rc_idx].busy_cntpct = read_cntpct_el0() +
			timeout_ms2cnt(IDE_KMU_BUSY_TIMEOUT_MS);

		VERBOSE("FVP IDE KM [%d]: set busy_timeout: 0x%lx\n",
			rc_idx, fvp_km_units[rc_idx].busy_cntpct);
	}

	return false;
}

static int fvp_simulate_ide_km_keyset_action(uint64_t ecam_address,
					     uint64_t flags, uint64_t keyset_id)
{
	int rc_idx;
	int rc = 0;

	rc_idx = plat_get_root_complex_index(ecam_address);
	if (rc_idx < 0) {
		return -EINVAL;
	}

	fvp_km_units[rc_idx].call_count++;

	if (simulate_km_unit_busy(rc_idx)) {
		return -EBUSY;
	}

	return rc;
}

int fvp_simulate_ide_km_keyset_prog(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id, uint64_t keyqw0,
				    uint64_t keyqw1, uint64_t keyqw2,
				    uint64_t keyqw3)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

int fvp_simulate_ide_km_keyset_go(uint64_t ecam_address, uint64_t flags,
				  uint64_t keyset_id)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

int fvp_simulate_ide_km_keyset_stop(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

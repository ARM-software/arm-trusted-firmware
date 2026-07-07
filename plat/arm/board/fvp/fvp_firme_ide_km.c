/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <plat/common/platform.h>

#if FVP_SIMULATE_IDE_KM_UNIT
int fvp_simulate_ide_km_keyset_prog(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id, uint64_t keyqw0,
				    uint64_t keyqw1, uint64_t keyqw2,
				    uint64_t keyqw3);
int fvp_simulate_ide_km_keyset_go(uint64_t ecam_address, uint64_t flags,
				  uint64_t keyset_id);
int fvp_simulate_ide_km_keyset_stop(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id);
#endif

/*
 * Return Root Complex index for the given 'ecam address'.
 *
 * Returns:
 *  < 0	- On error, else index of the Root Complex for the given ecam address
 */
int plat_get_root_complex_index(uint64_t ecam_address)
{
	if (ecam_address != PCIE_EXP_BASE) {
		return -EINVAL;
	}

	/* FVP default PCIe topology supports one Root Complex */
	return 0;
}

int plat_ide_km_keyset_prog(uint64_t ecam_address, uint64_t flags,
			    uint64_t keyset_id, uint64_t keyqw0,
			    uint64_t keyqw1, uint64_t keyqw2, uint64_t keyqw3)
{
#if FVP_SIMULATE_IDE_KM_UNIT
	return fvp_simulate_ide_km_keyset_prog(ecam_address, flags, keyset_id,
					       keyqw0, keyqw1, keyqw2, keyqw3);
#else
	return 0;
#endif
}


int plat_ide_km_keyset_go(uint64_t ecam_address, uint64_t flags,
			  uint64_t keyset_id)
{
#if FVP_SIMULATE_IDE_KM_UNIT
	return fvp_simulate_ide_km_keyset_go(ecam_address, flags, keyset_id);
#else
	return 0;
#endif
}

int plat_ide_km_keyset_stop(uint64_t ecam_address, uint64_t flags,
			    uint64_t keyset_id)
{
#if FVP_SIMULATE_IDE_KM_UNIT
	return fvp_simulate_ide_km_keyset_stop(ecam_address, flags, keyset_id);
#else
	return 0;
#endif
}

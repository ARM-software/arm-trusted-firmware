/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_H
#define MPMM_H

#include <stdbool.h>

#include <platform_def.h>

/*
 * Enable the Maximum Power Mitigation Mechanism.
 *
 * This function will enable MPMM for the current core. The AMU counters
 * representing the MPMM gears must have been configured and enabled prior to
 * calling this function.
 */
void mpmm_enable(void);

/*
 * MPMM core data.
 *
 * This structure represents per-core data retrieved from the hardware
 * configuration device tree.
 */
struct mpmm_core {
	/*
	 * Whether MPMM is supported.
	 *
	 * Cores with support for MPMM offer one or more auxiliary AMU counters
	 * representing MPMM gears.
	 */
	bool supported;
};

/*
 * MPMM topology.
 *
 * This topology structure describes the system-wide representation of the
 * information retrieved from the hardware configuration device tree.
 */
struct mpmm_topology {
	struct mpmm_core cores[PLATFORM_CORE_COUNT]; /* Per-core data */
};

#if !ENABLE_MPMM_FCONF
/*
 * Retrieve the platform's MPMM topology. A `NULL` return value is treated as a
 * non-fatal error, in which case MPMM will not be enabled for any core.
 */
const struct mpmm_topology *plat_mpmm_topology(void);
#endif /* ENABLE_MPMM_FCONF */

#endif /* MPMM_H */

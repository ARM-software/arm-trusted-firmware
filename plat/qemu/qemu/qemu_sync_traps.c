/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 * Portions copyright (c) 2026, Jason Wright. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file just contains demonstration code, to "handle" RNG traps.
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/sync_handle.h>
#include <context.h>

#define RANDOM_VALUE ((u_register_t) 3288484550995823360ULL)

/*
 * Emulation for testing purposes. If FEAT_RNG is available, just use that
 * for simplicity. If not, quickly make up a number that can pass for being
 * random. This is insecure, however, as a test platform, this is not
 * relevant on QEMU.
 */
#if ENABLE_FEAT_RNG_TRAP
int plat_handle_rng_trap(u_register_t *data, bool rndrrs)
{
	if (is_feat_rng_supported()) {
		/*
		 * Architecturally these can fail. In practice QEMU's
		 * RNDR/RNDRRS read always succeeds via
		 * qemu_guest_getrandom().
		 */
		if (rndrrs) {
			*data = read_rndrrs();
		} else {
			*data = read_rndr();
		}
	} else {
		*data = RANDOM_VALUE ^ read_cntpct_el0();
	}

	return TRAP_RET_CONTINUE;
}
#endif

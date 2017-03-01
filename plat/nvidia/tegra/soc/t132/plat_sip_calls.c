/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>

#include <tegra_private.h>

#define NS_SWITCH_AARCH32	1
#define SCR_RW_BITPOS		__builtin_ctz(SCR_RW_BIT)

/*******************************************************************************
 * Tegra132 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_AARCH_SWITCH			0x82000004

/*******************************************************************************
 * SPSR settings for AARCH32/AARCH64 modes
 ******************************************************************************/
#define SPSR32		SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE, \
			DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT)
#define SPSR64		SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS)

/*******************************************************************************
 * This function is responsible for handling all T132 SiP calls
 ******************************************************************************/
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags)
{
	switch (smc_fid) {

	case TEGRA_SIP_AARCH_SWITCH:

		/* clean up the high bits */
		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;

		if (!x1 || x2 > NS_SWITCH_AARCH32) {
			ERROR("%s: invalid parameters\n", __func__);
			return -EINVAL;
		}

		/* x1 = ns entry point */
		cm_set_elr_spsr_el3(NON_SECURE, x1,
			(x2 == NS_SWITCH_AARCH32) ? SPSR32 : SPSR64);

		/* switch NS world mode */
		cm_write_scr_el3_bit(NON_SECURE, SCR_RW_BITPOS, !x2);

		INFO("CPU switched to AARCH%s mode\n",
			(x2 == NS_SWITCH_AARCH32) ? "32" : "64");
		return 0;

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	return -ENOTSUP;
}

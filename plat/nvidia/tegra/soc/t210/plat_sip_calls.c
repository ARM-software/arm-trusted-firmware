/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <memctrl.h>
#include <pmc.h>
#include <tegra_private.h>
#include <tegra_platform.h>
#include <tegra_def.h>

/*******************************************************************************
 * PMC parameters
 ******************************************************************************/
#define PMC_READ 			U(0xaa)
#define PMC_WRITE 			U(0xbb)

/*******************************************************************************
 * Tegra210 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_PMC_COMMANDS		U(0xC2FFFE00)

/*******************************************************************************
 * This function is responsible for handling all T210 SiP calls
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
	uint32_t val, ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns)
		SMC_RET1(handle, SMC_UNK);

	switch (smc_fid) {
	case TEGRA_SIP_PMC_COMMANDS:

		/* check the address is within PMC range and is 4byte aligned */
		if ((x2 >= TEGRA_PMC_SIZE) || (x2 & 0x3))
			return -EINVAL;

		/* pmc_secure_scratch registers are not accessible */
		if (((x2 >= PMC_SECURE_SCRATCH0) && (x2 <= PMC_SECURE_SCRATCH5)) ||
		    ((x2 >= PMC_SECURE_SCRATCH6) && (x2 <= PMC_SECURE_SCRATCH7)) ||
		    ((x2 >= PMC_SECURE_SCRATCH8) && (x2 <= PMC_SECURE_SCRATCH79)) ||
		    ((x2 >= PMC_SECURE_SCRATCH80) && (x2 <= PMC_SECURE_SCRATCH119)))
				return -EFAULT;

		/* PMC secure-only registers are not accessible */
		if ((x2 == PMC_DPD_ENABLE_0) || (x2 == PMC_FUSE_CONTROL_0) ||
		    (x2 == PMC_CRYPTO_OP_0))
			return -EFAULT;

		/* Perform PMC read/write */
		if (x1 == PMC_READ) {
			val = mmio_read_32((uint32_t)(TEGRA_PMC_BASE + x2));
			write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X1, val);
		} else if (x1 == PMC_WRITE) {
			mmio_write_32((uint32_t)(TEGRA_PMC_BASE + x2), (uint32_t)x3);
		} else {
			return -EINVAL;
		}

		break;

	default:
		ERROR("%s: unsupported function ID\n", __func__);
		return -ENOTSUP;
	}

	return 0;
}

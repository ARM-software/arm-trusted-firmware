/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <errno.h>
#include <mce.h>
#include <mce_private.h>
#include <memctrl.h>
#include <common/runtime_svc.h>
#include <tegra_private.h>
#include <tegra_platform.h>
#include <smmu.h>
#include <stdbool.h>

/*******************************************************************************
 * Tegra194 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_GET_SMMU_PER		0xC200FF00U
#define TEGRA_SIP_CLEAR_RAS_CORRECTED_ERRORS	0xC200FF01U

/*******************************************************************************
 * This function is responsible for handling all T194 SiP calls
 ******************************************************************************/
int32_t plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags)
{
	int32_t ret = 0;
	uint32_t i, smmu_per[6] = {0};
	uint32_t num_smmu_devices = plat_get_num_smmu_devices();
	uint64_t per[3] = {0ULL};

	(void)x1;
	(void)x4;
	(void)cookie;
	(void)flags;

	switch (smc_fid) {
	case TEGRA_SIP_GET_SMMU_PER:

		/* make sure we dont go past the array length */
		assert(num_smmu_devices <= ARRAY_SIZE(smmu_per));

		/* read all supported SMMU_PER records */
		for (i = 0U; i < num_smmu_devices; i++) {
			smmu_per[i] = tegra_smmu_read_32(i, SMMU_GSR0_PER);
		}

		/* pack results into 3 64bit variables. */
		per[0] = smmu_per[0] | ((uint64_t)smmu_per[1] << 32U);
		per[1] = smmu_per[2] | ((uint64_t)smmu_per[3] << 32U);
		per[2] = smmu_per[4] | ((uint64_t)smmu_per[5] << 32U);

		/* provide the results via X1-X3 CPU registers */
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X1, per[0]);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X2, per[1]);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X3, per[2]);

		break;

#if RAS_EXTENSION
	case TEGRA_SIP_CLEAR_RAS_CORRECTED_ERRORS:
	{
		/*
		 * clear all RAS error records for corrected errors at first.
		 * x1 shall be 0 for first SMC call after FHI is asserted.
		 * */
		uint64_t local_x1 = x1;

		tegra194_ras_corrected_err_clear(&local_x1);
		if (local_x1 == 0ULL) {
			/* clear HSM corrected error status after all corrected
			 * RAS errors are cleared.
			 */
			mce_clear_hsm_corr_status();
		}

		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X1, local_x1);

		break;
	}
#endif

	default:
		ret = -ENOTSUP;
		break;
	}

	return ret;
}

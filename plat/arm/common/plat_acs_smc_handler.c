/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <plat/arm/common/plat_acs_smc_handler.h>

/*
 * Placeholder function for handling ACS SMC calls.
 * return 0  till the handling is done.
 */
uintptr_t plat_arm_acs_smc_handler(unsigned int smc_fid, uint64_t services,
		 uint64_t arg0, uint64_t arg1, uint64_t arg2, void *handle)
{
	WARN("Unimplemented ACS Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

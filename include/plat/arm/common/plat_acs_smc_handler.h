/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACS_SMC_HANDLER_H
#define ACS_SMC_HANDLER_H

#include <common/runtime_svc.h>
#include <lib/utils_def.h>

/* ARM ACS SMC service call */
#define ARM_VEN_EL3_ACS_SMC_HANDLER     U(0xC7000030)
#define is_acs_fid(smc_fid)             (smc_fid == ARM_VEN_EL3_ACS_SMC_HANDLER)

uintptr_t plat_arm_acs_smc_handler(unsigned int smc_fid,
				   uint64_t services,
				   uint64_t arg0,
				   uint64_t arg1,
				   uint64_t arg2,
				   void *handle);
#endif /* ACS_SMC_HANDLER_H */

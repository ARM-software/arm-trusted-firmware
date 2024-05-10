/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMF_H
#define PMF_H

#include <lib/cassert.h>
#include <lib/pmf/pmf_helpers.h>
#include <lib/utils_def.h>

/*
 * Constants used for/by PMF services.
 */
#define PMF_ARM_TIF_IMPL_ID	UL(0x41)
#define PMF_TID_SHIFT		0
#define PMF_TID_MASK		(UL(0xFF) << PMF_TID_SHIFT)
#define PMF_SVC_ID_SHIFT	10
#define PMF_SVC_ID_MASK		(UL(0x3F) << PMF_SVC_ID_SHIFT)
#define PMF_IMPL_ID_SHIFT	24
#define PMF_IMPL_ID_MASK	(UL(0xFF) << PMF_IMPL_ID_SHIFT)

/*
 * Flags passed to PMF_REGISTER_SERVICE
 */
#define PMF_STORE_ENABLE	(1 << 0)
#define PMF_DUMP_ENABLE		(1 << 1)

/*
 * Flags passed to PMF_GET_TIMESTAMP_XXX
 * and PMF_CAPTURE_TIMESTAMP
 */
#define PMF_CACHE_MAINT		(U(1) << 0)
#define PMF_NO_CACHE_MAINT	U(0)

/*
 * Defines for PMF SMC function ids used with arm-sip
 * range, this is now deprecated and will be removed.
 */
#define PMF_SMC_GET_TIMESTAMP_32_DEP	U(0x82000010)
#define PMF_SMC_GET_TIMESTAMP_64_DEP	U(0xC2000010)

#define PMF_FID_VALUE_DEPRECATED	U(0x10)
#define is_pmf_fid_deprecated(_fid) \
	(GET_SMC_NUM(_fid) == PMF_FID_VALUE_DEPRECATED)

/*
 * Defines for PMF SMC function ids used with Vendor-Specific
 * EL3 range.
 */
#define PMF_SMC_GET_TIMESTAMP_32	U(0x87000020)
#define PMF_SMC_GET_TIMESTAMP_64	U(0xC7000020)
#define PMF_NUM_SMC_CALLS		2

#define PMF_SMC_GET_VERSION_32		U(0x87000021)
#define PMF_SMC_GET_VERSION_64		U(0xC7000021)

#define PMF_SMC_VERSION			U(0x00000001)

/*
 * The macros below are used to identify
 * PMF calls from the SMC function ID.
 */
#define PMF_FID_VALUE		U(0x20)
#define PMF_ID_MASK		(FUNCID_NUM_MASK & ~(0xf))
#define is_pmf_fid(_fid)	((GET_SMC_NUM(_fid) & PMF_ID_MASK) == PMF_FID_VALUE)

/* Following are the supported PMF service IDs */
#define PMF_PSCI_STAT_SVC_ID	0
#define PMF_RT_INSTR_SVC_ID	1

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
/* PMF common functions */
int pmf_get_timestamp_smc(unsigned int tid,
		u_register_t mpidr,
		unsigned int flags,
		unsigned long long *ts_value);
int pmf_setup(void);
uintptr_t pmf_smc_handler(unsigned int smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags);

#endif /* PMF_H */

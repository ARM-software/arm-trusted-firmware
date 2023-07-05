/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_ABI_SVC_H
#define ERRATA_ABI_SVC_H

#include <lib/smccc.h>

#define ARM_EM_VERSION			U(0x840000F0)
#define ARM_EM_FEATURES			U(0x840000F1)
#define ARM_EM_CPU_ERRATUM_FEATURES	U(0x840000F2)

/* EM version numbers */
#define EM_VERSION_MAJOR		(0x1)
#define EM_VERSION_MINOR		(0x0)

/* EM CPU_ERRATUM_FEATURES return codes */
#define EM_HIGHER_EL_MITIGATION		(3)
#define EM_NOT_AFFECTED			(2)
#define EM_AFFECTED			(1)
#define EM_SUCCESS			(0)
#define EM_NOT_SUPPORTED		(-1)
#define EM_INVALID_PARAMETERS		(-2)
#define EM_UNKNOWN_ERRATUM		(-3)

#if ERRATA_ABI_SUPPORT
bool is_errata_fid(uint32_t smc_fid);
#else
static inline bool is_errata_fid(uint32_t smc_fid)
{
	return false;
}
#endif /* ERRATA_ABI_SUPPORT */
uintptr_t errata_abi_smc_handler(
	uint32_t smc_fid,
	u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *cookie,
	void *handle,
	u_register_t flags
);
#endif /* ERRATA_ABI_SVC_H */


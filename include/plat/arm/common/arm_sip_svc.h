/*
 * Copyright (c) 2016-2019,2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_SIP_SVC_H
#define ARM_SIP_SVC_H

#include <lib/utils_def.h>

/* SMC function IDs for SiP Service queries */

#define ARM_SIP_SVC_CALL_COUNT		U(0x8200ff00)
#define ARM_SIP_SVC_UID			U(0x8200ff01)
/*					U(0x8200ff02) is reserved */
#define ARM_SIP_SVC_VERSION		U(0x8200ff03)

/* PMF_SMC_GET_TIMESTAMP_32		0x82000010 */
/* PMF_SMC_GET_TIMESTAMP_64		0xC2000010 */

/* Function ID for requesting state switch of lower EL */
#define ARM_SIP_SVC_EXE_STATE_SWITCH	U(0x82000020)

/* Deprecated FID's Range and will be removed */
/* DEBUGFS_SMC_32			0x82000030U */
/* DEBUGFS_SMC_64			0xC2000030U */

/*
 * Arm(R) Ethos(TM)-N NPU SiP SMC function IDs
 * 0xC2000050-0xC200005F
 * 0x82000050-0x8200005F
 */

/* ARM SiP Service Calls version numbers */
#define ARM_SIP_SVC_VERSION_MAJOR		U(0x1)
#define ARM_SIP_SVC_VERSION_MINOR		U(0x0)

/*
 * Arm SiP SMC calls that are primarily used for testing purposes.
 */
#if PLAT_TEST_SPM
#define ARM_SIP_SET_INTERRUPT_PENDING	U(0x82000100)
#endif

/**
 * Arm SiP Service Call for the SPM to leverage RME to protect a give memory range.
 * Protected memory range is one whose PAS was made secure.
 * Unprotect relates to reverting a protect operation.
 */
#if SPMD_SPM_AT_SEL2 && ENABLE_RME
#define PLAT_PROTECT_MEM_SMC64 0xC2000101
#define PLAT_UNPROTECT_MEM_SMC64 0xC2000102
#endif

/* SiP handler specific to each Arm platform. */
uintptr_t plat_arm_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags);

#endif /* ARM_SIP_SVC_H */

/*
 * Copyright (c) 2026, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PFDI_SVC_H
#define PFDI_SVC_H

#include <stdint.h>

#include <lib/utils_def.h>
#include <smccc_helpers.h>

/*
 * SMC function IDs for PFDI Service
 * Upper word bits set: Fast call, SMC64, Standard Secure SVC. Call (OEN = 4)
 */
#define PFDI_FID(func_num)			\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |	\
	 (SMC_64 << FUNCID_CC_SHIFT) |		\
	 (OEN_STD_START << FUNCID_OEN_SHIFT) |	\
	 ((func_num) << FUNCID_NUM_SHIFT))

/* Platform Fault Detection Interface(PFDI) functions */
#define PFDI_FNUM_VERSION		U(0x2D0)
#define PFDI_FNUM_FEATURES		U(0x2D1)
#define PFDI_FNUM_PE_TEST_ID		U(0x2D2)
#define PFDI_FNUM_PE_TEST_PART_COUNT	U(0x2D3)
#define PFDI_FNUM_PE_TEST_RUN		U(0x2D4)
#define PFDI_FNUM_PE_TEST_RESULT	U(0x2D5)
#define PFDI_FNUM_FW_CHECK		U(0x2D6)
#define PFDI_FNUM_FORCE_ERROR		U(0x2D7)

#define PFDI_VERSION			PFDI_FID(PFDI_FNUM_VERSION)
#define PFDI_FEATURES			PFDI_FID(PFDI_FNUM_FEATURES)
#define PFDI_PE_TEST_ID			PFDI_FID(PFDI_FNUM_PE_TEST_ID)
#define PFDI_PE_TEST_PART_COUNT		PFDI_FID(PFDI_FNUM_PE_TEST_PART_COUNT)
#define PFDI_PE_TEST_RUN		PFDI_FID(PFDI_FNUM_PE_TEST_RUN)
#define PFDI_PE_TEST_RESULT		PFDI_FID(PFDI_FNUM_PE_TEST_RESULT)
#define PFDI_FW_CHECK			PFDI_FID(PFDI_FNUM_FW_CHECK)
#define PFDI_FORCE_ERROR		PFDI_FID(PFDI_FNUM_FORCE_ERROR)
#define PFDI_FID_MIN			PFDI_VERSION
#define PFDI_FID_MAX			PFDI_FID(U(0x2DF))

/*
 * The macros below are used to identify (Platform Fault Detection Interface)
 * PFDI calls from the SMC function ID
 */
static inline bool is_pfdi_fid(uint32_t fid)
{
	uint32_t type = (fid >> FUNCID_TYPE_SHIFT) & FUNCID_TYPE_MASK;
	uint32_t cc = (fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK;
	uint32_t oen = (fid >> FUNCID_OEN_SHIFT) & FUNCID_OEN_MASK;

	return (type == SMC_TYPE_FAST) &&
		(cc == SMC_64) &&
		(oen == OEN_STD_START) &&
		(fid >= PFDI_FID_MIN) &&
		(fid <= PFDI_FID_MAX);
}

uint64_t pfdi_smc_handler(uint32_t smc_fid,
			  u_register_t x1, u_register_t x2,
			  u_register_t x3, u_register_t x4,
			  void *cookie, void *handle,
			  u_register_t flags);

#endif /* PFDI_SVC_H */

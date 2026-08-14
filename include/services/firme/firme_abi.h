/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_ABI_H
#define FIRME_ABI_H

#include <lib/smccc.h>
#include <lib/utils_def.h>

/* FIRME SMC return codes */
#define FIRME_SUCCESS			0
#define FIRME_NOT_SUPPORTED		-1
#define FIRME_INVALID_PARAMETERS	-2
#define FIRME_ABORTED			-3
#define FIRME_INCOMPLETE		-4
#define FIRME_DENIED			-5
#define FIRME_BUSY			-6
#define FIRME_OP_CONFLICT		-7
#define FIRME_EXISTS			-8
#define FIRME_NO_ENTRY			-9
#define FIRME_NO_MEMORY			-10
#define FIRME_BAD_DATA			-11

/* Range of function IDs used by FIRME interface */
#define FIRME_FNUM_MIN_VALUE		U(0x400)
#define FIRME_FNUM_MAX_VALUE		U(0x412)

/* Construct a FIRME fastcall std FID from a service-local function number. */
#define FIRME_FID(_offset)                                                    \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | (SMC_64 << FUNCID_CC_SHIFT) | \
	 (OEN_STD_START << FUNCID_OEN_SHIFT) |                                \
	 (((FIRME_FNUM_MIN_VALUE + (_offset)) & FUNCID_NUM_MASK)              \
	  << FUNCID_NUM_SHIFT))

/* Compatibility alias for existing FIRME service headers. */
#define SMC64_FIRME_FID(_offset)	FIRME_FID(_offset)

#define is_firme_fid(fid)                                       \
	__extension__({                                         \
		__typeof__(fid) _fid = (fid);                   \
		((GET_SMC_NUM(_fid) >= FIRME_FNUM_MIN_VALUE) && \
		 (GET_SMC_NUM(_fid) <= FIRME_FNUM_MAX_VALUE) && \
		 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST) &&       \
		 (GET_SMC_CC(_fid) == SMC_64) &&                \
		 (GET_SMC_OEN(_fid) == OEN_STD_START) &&        \
		 ((_fid & 0x00FE0000) == 0U));                  \
	})

#endif /* FIRME_ABI_H */

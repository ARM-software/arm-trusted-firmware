/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPRT_SVC_H
#define SPRT_SVC_H

#include <lib/smccc.h>
#include <lib/utils_def.h>

/* SPRT_VERSION helpers */

#define SPRT_VERSION_MAJOR		U(0)
#define SPRT_VERSION_MAJOR_SHIFT	16
#define SPRT_VERSION_MAJOR_MASK		U(0x7FFF)
#define SPRT_VERSION_MINOR		U(1)
#define SPRT_VERSION_MINOR_SHIFT	0
#define SPRT_VERSION_MINOR_MASK		U(0xFFFF)
#define SPRT_VERSION_FORM(major, minor)	((((major) & SPRT_VERSION_MAJOR_MASK)  \
						<< SPRT_VERSION_MAJOR_SHIFT) | \
					((minor) & SPRT_VERSION_MINOR_MASK))
#define SPRT_VERSION_COMPILED		SPRT_VERSION_FORM(SPRT_VERSION_MAJOR, \
							  SPRT_VERSION_MINOR)

/* SPRT function IDs */

#define SPRT_FID_VERSION		U(0x0)
#define SPRT_FID_PUT_RESPONSE		U(0x1)
#define SPRT_FID_YIELD			U(0x5)
#define SPRT_FID_PANIC			U(0x7)
#define SPRT_FID_MEMORY_PERM_ATTR_GET	U(0xB)
#define SPRT_FID_MEMORY_PERM_ATTR_SET	U(0xC)

#define SPRT_FID_MASK			U(0xFF)

/* Definitions to build the complete SMC ID */

#define OEN_SPRT_START			U(0x20)
#define OEN_SPRT_END			U(0x2F)

#define SPRT_SMC_64(sprt_fid)	((OEN_SPRT_START << FUNCID_OEN_SHIFT) | \
				 (U(1) << 31) | ((sprt_fid) & SPRT_FID_MASK) | \
				 (SMC_64 << FUNCID_CC_SHIFT))
#define SPRT_SMC_32(sprt_fid)	((OEN_SPRT_START << FUNCID_OEN_SHIFT) | \
				 (U(1) << 31) | ((sprt_fid) & SPRT_FID_MASK) | \
				 (SMC_32 << FUNCID_CC_SHIFT))

/* Complete SMC IDs */

#define SPRT_VERSION				SPRT_SMC_32(SPRT_FID_VERSION)
#define SPRT_PUT_RESPONSE_AARCH64		SPRT_SMC_64(SPRT_FID_PUT_RESPONSE)
#define SPRT_YIELD_AARCH64			SPRT_SMC_64(SPRT_FID_YIELD)
#define SPRT_PANIC_AARCH64			SPRT_SMC_64(SPRT_FID_PANIC)
#define SPRT_MEMORY_PERM_ATTR_GET_AARCH64	SPRT_SMC_64(SPRT_FID_MEMORY_PERM_ATTR_GET)
#define SPRT_MEMORY_PERM_ATTR_SET_AARCH64	SPRT_SMC_64(SPRT_FID_MEMORY_PERM_ATTR_SET)

/* Defines used by SPRT_MEMORY_PERM_ATTR_{GET,SET}_AARCH64 */

#define SPRT_MEMORY_PERM_ATTR_RO	U(0)
#define SPRT_MEMORY_PERM_ATTR_RW	U(1)
#define SPRT_MEMORY_PERM_ATTR_RO_EXEC	U(2)
/* U(3) is reserved */
#define SPRT_MEMORY_PERM_ATTR_MASK	U(3)
#define SPRT_MEMORY_PERM_ATTR_SHIFT	3

/* SPRT error codes. */

#define SPRT_SUCCESS		 0
#define SPRT_NOT_SUPPORTED	-1
#define SPRT_INVALID_PARAMETER	-2

#endif /* SPRT_SVC_H */

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SPM_SVC_H__
#define __SPM_SVC_H__

#include <utils_def.h>

#define SPM_VERSION_MAJOR	U(0)
#define SPM_VERSION_MINOR	U(1)
#define SPM_VERSION_FORM(major, minor)	((major << 16) | (minor))
#define SPM_VERSION_COMPILED	SPM_VERSION_FORM(SPM_VERSION_MAJOR, SPM_VERSION_MINOR)

#define SP_VERSION_MAJOR	U(1)
#define SP_VERSION_MINOR	U(0)
#define SP_VERSION_FORM(major, minor)	((major << 16) | (minor))
#define SP_VERSION_COMPILED	SP_VERSION_FORM(SP_VERSION_MAJOR, SP_VERSION_MINOR)

/* The macros below are used to identify SPM calls from the SMC function ID */
#define SPM_FID_MASK			U(0xffff)
#define SPM_FID_MIN_VALUE		U(0x40)
#define SPM_FID_MAX_VALUE		U(0x7f)
#define is_spm_fid(_fid)						\
		((((_fid) & SPM_FID_MASK) >= SPM_FID_MIN_VALUE) &&	\
		 (((_fid) & SPM_FID_MASK) <= SPM_FID_MAX_VALUE))

/*
 * SMC IDs defined for accessing services implemented by the Secure Partition
 * Manager from the Secure Partition(s). These services enable a partition to
 * handle delegated events and request privileged operations from the manager.
 */
#define SPM_VERSION_AARCH32			U(0x84000060)
#define SP_EVENT_COMPLETE_AARCH64		U(0xC4000061)
#define SP_MEMORY_ATTRIBUTES_GET_AARCH64	U(0xC4000064)
#define SP_MEMORY_ATTRIBUTES_SET_AARCH64	U(0xC4000065)

/*
 * Macros used by SP_MEMORY_ATTRIBUTES_SET_AARCH64.
 */

#define SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS	U(0)
#define SP_MEMORY_ATTRIBUTES_ACCESS_RW		U(1)
/* Value U(2) is reserved. */
#define SP_MEMORY_ATTRIBUTES_ACCESS_RO		U(3)
#define SP_MEMORY_ATTRIBUTES_ACCESS_MASK	U(3)
#define SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT	0

#define SP_MEMORY_ATTRIBUTES_EXEC		(U(0) << 2)
#define SP_MEMORY_ATTRIBUTES_NON_EXEC		(U(1) << 2)

/*
 * SMC IDs defined in [1] for accessing secure partition services from the
 * Non-secure world. These FIDs occupy the range 0x40 - 0x5f
 * [1] DEN0060A_ARM_MM_Interface_Specification.pdf
 */
#define SP_VERSION_AARCH64		U(0xC4000040)
#define SP_VERSION_AARCH32		U(0x84000040)

#define MM_COMMUNICATE_AARCH64		U(0xC4000041)
#define MM_COMMUNICATE_AARCH32		U(0x84000041)

/* SPM error codes. */
#define SPM_SUCCESS		0
#define SPM_NOT_SUPPORTED	-1
#define SPM_INVALID_PARAMETER	-2
#define SPM_DENIED		-3
#define SPM_NO_MEMORY		-5

#ifndef __ASSEMBLY__

#include <stdint.h>

int32_t spm_setup(void);

uint64_t spm_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags);

#endif /* __ASSEMBLY__ */

#endif /* __SPM_SVC_H__ */

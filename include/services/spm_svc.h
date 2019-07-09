/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_SVC_H
#define SPM_SVC_H

#if SPM_MM

#include <lib/utils_def.h>

#define SPM_VERSION_MAJOR	U(0)
#define SPM_VERSION_MAJOR_SHIFT	16
#define SPM_VERSION_MAJOR_MASK	U(0x7FFF)
#define SPM_VERSION_MINOR	U(1)
#define SPM_VERSION_MINOR_SHIFT	0
#define SPM_VERSION_MINOR_MASK	U(0xFFFF)
#define SPM_VERSION_FORM(major, minor)	((major << SPM_VERSION_MAJOR_SHIFT) | (minor))
#define SPM_VERSION_COMPILED	SPM_VERSION_FORM(SPM_VERSION_MAJOR, SPM_VERSION_MINOR)

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
 * They occupy the range 0x60-0x7f.
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


/* SPM error codes. */
#define SPM_SUCCESS		0
#define SPM_NOT_SUPPORTED	-1
#define SPM_INVALID_PARAMETER	-2
#define SPM_DENIED		-3
#define SPM_NO_MEMORY		-5

#endif /* SPM_MM */

#ifndef __ASSEMBLER__

#include <stdint.h>

int32_t spm_setup(void);

#if SPM_MM

uint64_t spm_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags);

/* Helper to enter a Secure Partition */
uint64_t spm_sp_call(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3);

#endif /* SPM_MM */

#endif /* __ASSEMBLER__ */

#endif /* SPM_SVC_H */

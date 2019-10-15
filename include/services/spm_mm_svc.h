/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_MM_SVC_H
#define SPM_MM_SVC_H

#include <lib/utils_def.h>

/*
 * The MM_VERSION_XXX definitions are used when responding to the
 * MM_VERSION_AARCH32 service request. The version returned is different between
 * this request and the SPM_MM_VERSION_AARCH32 request - both have been retained
 * for compatibility.
 */
#define MM_VERSION_MAJOR	U(1)
#define MM_VERSION_MAJOR_SHIFT	16
#define MM_VERSION_MAJOR_MASK	U(0x7FFF)
#define MM_VERSION_MINOR	U(0)
#define MM_VERSION_MINOR_SHIFT	0
#define MM_VERSION_MINOR_MASK	U(0xFFFF)
#define MM_VERSION_FORM(major, minor) ((major << MM_VERSION_MAJOR_SHIFT) | \
				       (minor))
#define MM_VERSION_COMPILED	MM_VERSION_FORM(MM_VERSION_MAJOR, \
						MM_VERSION_MINOR)

#define SPM_MM_VERSION_MAJOR		  U(0)
#define SPM_MM_VERSION_MAJOR_SHIFT	  16
#define SPM_MM_VERSION_MAJOR_MASK	  U(0x7FFF)
#define SPM_MM_VERSION_MINOR		  U(1)
#define SPM_MM_VERSION_MINOR_SHIFT	  0
#define SPM_MM_VERSION_MINOR_MASK	  U(0xFFFF)
#define SPM_MM_VERSION_FORM(major, minor) ((major << \
					    SPM_MM_VERSION_MAJOR_SHIFT) | \
					   (minor))
#define SPM_MM_VERSION_COMPILED	SPM_MM_VERSION_FORM(SPM_MM_VERSION_MAJOR, \
						    SPM_MM_VERSION_MINOR)

/* These macros are used to identify SPM-MM calls using the SMC function ID */
#define SPM_MM_FID_MASK			U(0xffff)
#define SPM_MM_FID_MIN_VALUE		U(0x40)
#define SPM_MM_FID_MAX_VALUE		U(0x7f)
#define is_spm_mm_fid(_fid)						 \
		((((_fid) & SPM_MM_FID_MASK) >= SPM_MM_FID_MIN_VALUE) && \
		 (((_fid) & SPM_MM_FID_MASK) <= SPM_MM_FID_MAX_VALUE))

/*
 * SMC IDs defined in [1] for accessing MM services from the Non-secure world.
 * These FIDs occupy the range 0x40 - 0x5f.
 * [1] DEN0060A_ARM_MM_Interface_Specification.pdf
 */
#define MM_VERSION_AARCH32		U(0x84000040)
#define MM_COMMUNICATE_AARCH64		U(0xC4000041)
#define MM_COMMUNICATE_AARCH32		U(0x84000041)

/*
 * SMC IDs defined for accessing services implemented by the Secure Partition
 * Manager from the Secure Partition(s). These services enable a partition to
 * handle delegated events and request privileged operations from the manager.
 * They occupy the range 0x60-0x7f.
 */
#define SPM_MM_VERSION_AARCH32			U(0x84000060)
#define MM_SP_EVENT_COMPLETE_AARCH64		U(0xC4000061)
#define MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64	U(0xC4000064)
#define MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64	U(0xC4000065)

/*
 * Macros used by MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64.
 */

#define MM_SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS	U(0)
#define MM_SP_MEMORY_ATTRIBUTES_ACCESS_RW	U(1)
/* Value U(2) is reserved. */
#define MM_SP_MEMORY_ATTRIBUTES_ACCESS_RO	U(3)
#define MM_SP_MEMORY_ATTRIBUTES_ACCESS_MASK	U(3)
#define MM_SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT	0

#define MM_SP_MEMORY_ATTRIBUTES_EXEC		(U(0) << 2)
#define MM_SP_MEMORY_ATTRIBUTES_NON_EXEC	(U(1) << 2)


/* SPM error codes. */
#define SPM_MM_SUCCESS		  0
#define SPM_MM_NOT_SUPPORTED	 -1
#define SPM_MM_INVALID_PARAMETER -2
#define SPM_MM_DENIED		 -3
#define SPM_MM_NO_MEMORY	 -5

#ifndef __ASSEMBLER__

#include <stdint.h>

int32_t spm_mm_setup(void);

uint64_t spm_mm_smc_handler(uint32_t smc_fid,
			    uint64_t x1,
			    uint64_t x2,
			    uint64_t x3,
			    uint64_t x4,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

/* Helper to enter a secure partition */
uint64_t spm_mm_sp_call(uint32_t smc_fid,
			uint64_t x1,
			uint64_t x2,
			uint64_t x3);

#endif /* __ASSEMBLER__ */

#endif /* SPM_MM_SVC_H */

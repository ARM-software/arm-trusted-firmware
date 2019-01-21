/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MM_SVC_H
#define MM_SVC_H

#if SPM_MM

#include <lib/utils_def.h>

#define MM_VERSION_MAJOR	U(1)
#define MM_VERSION_MAJOR_SHIFT	16
#define MM_VERSION_MAJOR_MASK	U(0x7FFF)
#define MM_VERSION_MINOR	U(0)
#define MM_VERSION_MINOR_SHIFT	0
#define MM_VERSION_MINOR_MASK	U(0xFFFF)
#define MM_VERSION_FORM(major, minor)	((major << MM_VERSION_MAJOR_SHIFT) | (minor))
#define MM_VERSION_COMPILED	MM_VERSION_FORM(MM_VERSION_MAJOR, MM_VERSION_MINOR)

/*
 * SMC IDs defined in [1] for accessing MM services from the Non-secure world.
 * These FIDs occupy the range 0x40 - 0x5f.
 * [1] DEN0060A_ARM_MM_Interface_Specification.pdf
 */
#define MM_VERSION_AARCH32		U(0x84000040)

#define MM_COMMUNICATE_AARCH64		U(0xC4000041)
#define MM_COMMUNICATE_AARCH32		U(0x84000041)

#endif /* SPM_MM */

#endif /* MM_SVC_H */

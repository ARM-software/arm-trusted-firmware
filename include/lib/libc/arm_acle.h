/*
 * Copyright (c) 2021 ARM Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The definitions below are a subset of what we would normally get by using
 * the compiler's version of arm_acle.h. We can't use that directly because
 * we specify -nostdinc in the Makefiles.
 *
 * We just define the functions we need so far.
 */

#ifndef ARM_ACLE_H
#define ARM_ACLE_H

#if !defined(__aarch64__) || defined(__clang__)
#	define __crc32b __builtin_arm_crc32b
#	define __crc32w __builtin_arm_crc32w
#else
#	define __crc32b __builtin_aarch64_crc32b
#	define __crc32w __builtin_aarch64_crc32w
#endif

#endif	/* ARM_ACLE_H */

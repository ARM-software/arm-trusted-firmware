/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_ARCH_SVC_H
#define ARM_ARCH_SVC_H

#define SMCCC_VERSION			U(0x80000000)
#define SMCCC_ARCH_FEATURES		U(0x80000001)
#define SMCCC_ARCH_WORKAROUND_1		U(0x80008000)
#define SMCCC_ARCH_WORKAROUND_2		U(0x80007FFF)

#define SMCCC_ARCH_NOT_REQUIRED		-2

#endif /* ARM_ARCH_SVC_H */

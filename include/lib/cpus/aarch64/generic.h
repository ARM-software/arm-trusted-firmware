/*
 * Copyright (c) 2020, Arm Limited. All rights reserverd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AARCH64_GENERIC_H
#define AARCH64_GENERIC_H

#include <lib/utils_def.h>

/*
 * 0x0 value on the MIDR implementer value is reserved for software use,
 * so use an MIDR value of 0 for a default CPU library.
 */
#define AARCH64_GENERIC_MIDR			U(0)

#endif /* AARCH64_GENERIC_H */

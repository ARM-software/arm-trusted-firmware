/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QEMU_MAX_H
#define QEMU_MAX_H

#include <lib/utils_def.h>

/*
 *  QEMU MAX midr for revision 0
 *  00   - Reserved for software use
 *  0    - Variant
 *  F    - Architectural features identified in ID_* registers
 *  051  - 'Q', in a 12-bit field.
 *  0    - Revision
 */
#define QEMU_MAX_MIDR		U(0x000F0510)

#endif /* QEMU_MAX_H */

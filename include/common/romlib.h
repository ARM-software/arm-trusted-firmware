/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ROMLIB_H
#define ROMLIB_H

#define ROMLIB_MAJOR   0
#define ROMLIB_MINOR   1
#define ROMLIB_VERSION ((ROMLIB_MAJOR << 8) | ROMLIB_MINOR)

int rom_lib_init(int version);

#endif /* ROMLIB_H */

/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <cdefs.h>
#include <stdlib.h>

static inline void plat_panic_handler(void)
{
	printf("TF-A aborted by calling %s\n", __func__);
	abort();
}

#endif

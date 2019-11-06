/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dev.h"

static const dirtab_t blobtab[] = {
	{"ctl", DEV_ROOT_QBLOBCTL, 0, O_READ},
	{"fip.bin", DEV_ROOT_QBLOBCTL + 1, 0x100000, O_READ, (void *)0x8000000}
};

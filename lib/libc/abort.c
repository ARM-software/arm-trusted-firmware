/*
 * Copyright (c) 2013-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <common/debug.h>

void abort(void)
{
	ERROR("ABORT\n");
	panic();
}

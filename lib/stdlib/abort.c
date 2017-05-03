/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>

/*
 * This is a basic implementation. This could be improved.
 */
void abort (void)
{
	ERROR("ABORT\n");
	panic();
}

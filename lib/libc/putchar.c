/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

int __putchar(int c)
{
	return c;
}

int putchar(int c) __attribute__((weak,alias("__putchar")));

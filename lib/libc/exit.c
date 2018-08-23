/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

static void (*exitfun)(void);

void exit(int status)
{
	if (exitfun != NULL)
		(*exitfun)();
	for (;;)
		;
}

int atexit(void (*fun)(void))
{
	if (exitfun != NULL)
		return -1;
	exitfun = fun;

	return 0;
}

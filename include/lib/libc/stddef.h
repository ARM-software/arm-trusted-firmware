/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STDDEF_H
#define _STDDEF_H

#include <arch/stddef.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define offsetof(st, m) ((size_t)&(((st *)0)->m))

#endif

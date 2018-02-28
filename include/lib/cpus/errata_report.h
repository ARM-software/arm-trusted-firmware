/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ERRATA_H__
#define __ERRATA_H__

#ifndef __ASSEMBLY__

#include <arch.h>
#include <arch_helpers.h>
#include <spinlock.h>
#include <utils_def.h>

#if DEBUG
void print_errata_status(void);
#else
static inline void print_errata_status(void) {}
#endif

void errata_print_msg(unsigned int status, const char *cpu, const char *id);
int errata_needs_reporting(spinlock_t *lock, uint32_t *reported);

#endif /* __ASSEMBLY__ */

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

#endif /* __ERRATA_H__ */


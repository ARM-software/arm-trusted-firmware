/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_REPORT_H
#define ERRATA_REPORT_H

#ifndef __ASSEMBLER__

#include <arch.h>
#include <arch_helpers.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#if DEBUG
void print_errata_status(void);
#else
static inline void print_errata_status(void) {}
#endif

void errata_print_msg(unsigned int status, const char *cpu, const char *id);
int errata_needs_reporting(spinlock_t *lock, uint32_t *reported);

#endif /* __ASSEMBLER__ */

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

#endif /* ERRATA_REPORT_H */

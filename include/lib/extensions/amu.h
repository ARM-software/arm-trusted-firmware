/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_H
#define AMU_H

#include <stdbool.h>
#include <context.h>

#if __aarch64__
void amu_enable(bool el2_unused, cpu_context_t *ctx);
#else
void amu_enable(bool el2_unused);
#endif

#endif /* AMU_H */

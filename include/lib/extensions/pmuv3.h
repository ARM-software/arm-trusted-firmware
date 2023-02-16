/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMUV3_H
#define PMUV3_H

#include <context.h>

void pmuv3_init_el3(void);

#ifdef __aarch64__
void pmuv3_enable(cpu_context_t *ctx);
void pmuv3_init_el2_unused(void);
#endif /* __aarch64__ */

#endif /* PMUV3_H */

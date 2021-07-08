/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SVE_H
#define SVE_H

#include <context.h>

void sve_enable(cpu_context_t *context);
void sve_disable(cpu_context_t *context);

#endif /* SVE_H */

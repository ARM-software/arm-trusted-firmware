/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SVE_H
#define SVE_H

#include <stdbool.h>

bool sve_supported(void);
void sve_enable(bool el2_unused);

#endif /* SVE_H */

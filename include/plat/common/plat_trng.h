/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_TRNG_H
#define PLAT_TRNG_H

#include <tools_share/uuid.h>

/* TRNG platform functions */

extern uuid_t plat_trng_uuid;
void plat_entropy_setup(void);
bool plat_get_entropy(uint64_t *out);

#endif /* PLAT_TRNG_H */

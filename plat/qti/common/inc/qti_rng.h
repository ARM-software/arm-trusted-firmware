/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RNG_H
#define QTI_RNG_H

#include <stdint.h>

int qti_rng_get_data(uint8_t *out, uint32_t out_len);

#endif /* QTI_RNG_H */

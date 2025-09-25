/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RNG_H
#define QTI_RNG_H

#include <stdint.h>

#include <platform_def.h>

#define SEC_PRNG_STATUS				(QTI_SEC_PRNG_BASE + 0x1004)
#define SEC_PRNG_DATA_OUT			(QTI_SEC_PRNG_BASE + 0x1000)
#define SEC_PRNG_STATUS_DATA_AVAIL_BMSK		0x1

int qti_rng_get_data(uint8_t *out, uint32_t out_len);

#endif /* QTI_RNG_H */

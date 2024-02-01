/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_H
#define FWU_H

#include <stdbool.h>

#define FWU_BANK_STATE_ACCEPTED		0xFCU
#define FWU_BANK_STATE_VALID		0xFEU
#define FWU_BANK_STATE_INVALID		0xFFU

void fwu_init(void);
uint32_t fwu_get_active_bank_state(void);
const struct fwu_metadata *fwu_get_metadata(void);

#endif /* FWU_H */

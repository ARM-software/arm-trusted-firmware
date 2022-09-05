/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_DCM_H
#define MTK_DCM_H

#include <stdbool.h>

bool dcm_check_state(uintptr_t addr, unsigned int mask, unsigned int compare);
int dcm_set_init(void);

#endif /* #ifndef MTK_DCM_H */

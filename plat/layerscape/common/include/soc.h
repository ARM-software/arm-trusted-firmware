/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

#include <stdint.h>

#define SVR_WO_E		0xFFFFFE
#define SVR_LS1043A		0x879204
#define SVR_LS1043AE		0x879200

void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base);

#endif /* SOC_H */

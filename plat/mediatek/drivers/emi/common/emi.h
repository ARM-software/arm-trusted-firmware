/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_H
#define EMI_H

#include <stdint.h>

#define EMI_MPU_ALIGN_BITS	12

uint64_t sip_emi_mpu_set_protection(u_register_t start, u_register_t end, u_register_t region);

#endif /* EMI_H */

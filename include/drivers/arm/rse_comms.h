/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSE_COMMS_H
#define RSE_COMMS_H

#include <stdint.h>

int rse_comms_init(uintptr_t mhu_sender_base, uintptr_t mhu_receiver_base);

#endif /* RSE_COMMS_H */

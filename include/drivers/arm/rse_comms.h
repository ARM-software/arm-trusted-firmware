/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSS_COMMS_H
#define RSS_COMMS_H

#include <stdint.h>

int rss_comms_init(uintptr_t mhu_sender_base, uintptr_t mhu_receiver_base);

#endif /* RSS_COMMS_H */

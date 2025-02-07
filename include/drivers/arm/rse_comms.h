/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSE_COMMS_H
#define RSE_COMMS_H

#include <stddef.h>
#include <stdint.h>

size_t rse_mbx_get_max_message_size(void);
int rse_mbx_send_data(const uint8_t *send_buffer, size_t size);
int rse_mbx_receive_data(uint8_t *receive_buffer, size_t *size);
int rse_mbx_init(const void *init_data);

#endif /* RSE_COMMS_H */

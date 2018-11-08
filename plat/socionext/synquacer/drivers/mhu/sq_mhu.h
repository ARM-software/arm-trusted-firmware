/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SQ_MHU_H
#define SQ_MHU_H

#include <stdint.h>

void mhu_secure_message_start(unsigned int slot_id);
void mhu_secure_message_send(unsigned int slot_id);
uint32_t mhu_secure_message_wait(void);
void mhu_secure_message_end(unsigned int slot_id);

void mhu_secure_init(void);

#endif /* SQ_MHU_H */

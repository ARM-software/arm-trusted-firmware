/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRCM_MHU_H
#define BRCM_MHU_H

#include <stdint.h>

void mhu_secure_message_start(unsigned int slot_id);
void mhu_secure_message_send(unsigned int slot_id);
uint32_t mhu_secure_message_wait(void);
void mhu_secure_message_end(unsigned int slot_id);

void mhu_secure_init(void);

#endif	/* BRCM_MHU_H */

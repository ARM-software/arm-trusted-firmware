/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __CRYPT_H__
#define __CRYPT_H__

#include <stdint.h>

/* crypt function prototype */
uint64_t crypt_set_hdcp_key_ex(uint64_t x1, uint64_t x2, uint64_t x3);
uint64_t crypt_set_hdcp_key_num(uint32_t num);
uint64_t crypt_clear_hdcp_key(void);

#endif /* __CRYPT_H__ */

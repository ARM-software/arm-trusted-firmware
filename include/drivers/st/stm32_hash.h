/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_HASH_H
#define STM32_HASH_H

enum stm32_hash_algo_mode {
	HASH_MD5SUM,
	HASH_SHA1,
	HASH_SHA224,
	HASH_SHA256
};

int stm32_hash_update(const uint8_t *buffer, size_t length);
int stm32_hash_final(uint8_t *digest);
int stm32_hash_final_update(const uint8_t *buffer, uint32_t buf_length,
			    uint8_t *digest);
void stm32_hash_init(enum stm32_hash_algo_mode mode);
int stm32_hash_register(void);

#endif /* STM32_HASH_H */

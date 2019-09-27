/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_AUTH_H
#define STM32MP_AUTH_H

struct stm32mp_auth_ops {
	uint32_t (*check_key)(uint8_t *pubkey_in, uint8_t *pubkey_out);
	uint32_t (*verify_signature)(uint8_t *hash_in, uint8_t *pubkey_in,
				     uint8_t *signature, uint32_t ecc_algo);
};

void stm32mp_init_auth(struct stm32mp_auth_ops *init_ptr);
int stm32mp_auth_image(boot_api_image_header_t *header, uintptr_t buffer);

#endif /* STM32MP_AUTH_H */

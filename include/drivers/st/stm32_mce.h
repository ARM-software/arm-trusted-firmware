/*
 * Copyright (c) 2020-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_MCE_H
#define STM32_MCE_H

#include <stdbool.h>
#include <stdint.h>

#include <dt-bindings/soc/stm32mp13-mce.h>

/* MCE encryption modes */
#define MCE_BYPASS_MODE			MCE_PLAINTEXT
#define MCE_ENCRYPT_MODE		MCE_ENCRYPT
#define MCE_ENCRYPTION_MODE_MAX		MCE_ENCRYPT_MODE

/* IP configuration */
#define MCE_IP_MAX_REGION_NB		1U

struct stm32_mce_region_s {
	uint32_t encrypt_mode;	/*
				 * Specifies the region encryption mode.
				 * This parameter can be a value of
				 * @ref MCE_*_MODE (in driver header file).
				 */
	uint32_t start_address;	/* Specifies the region start address */
	uint32_t end_address;	/* Specifies the region end address */
};

void stm32_mce_init(void);

int stm32_mce_write_master_key(uint8_t *mkey);
void stm32_mce_lock_master_key(void);
bool stm32_mce_is_master_key_locked(void);

void stm32_mce_lock_global(void);
bool stm32_mce_is_globally_locked(void);
bool stm32_mce_is_hw_encryption_functional(void);

int stm32_mce_get_address_encryption_state(uint32_t address, uint32_t *state);
#endif /* STM32_MCE_H */

/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_SAES_H
#define STM32_SAES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DT_SAES_COMPAT		"st,stm32-saes"

struct stm32_saes_platdata {
	uintptr_t base;
	unsigned long clock_id;
	unsigned int reset_id;
};

enum stm32_saes_chaining_mode {
	STM32_SAES_MODE_ECB,
	STM32_SAES_MODE_CBC,
	STM32_SAES_MODE_CTR,
	STM32_SAES_MODE_GCM,
	STM32_SAES_MODE_CCM, /* Not use in TF-A */
};

enum stm32_saes_key_selection {
	STM32_SAES_KEY_SOFT,
	STM32_SAES_KEY_DHU,           /* Derived HW unique key */
	STM32_SAES_KEY_BH,            /* Boot HW key */
	STM32_SAES_KEY_BHU_XOR_BH,    /* XOR of DHUK and BHK */
	STM32_SAES_KEY_WRAPPED
};

struct stm32_saes_context {
	uintptr_t base;
	uint32_t cr;
	uint32_t assoc_len;
	uint32_t load_len;
	uint32_t key[8]; /* In HW byte order */
	uint32_t iv[4];  /* In HW byte order */
};

int stm32_saes_driver_init(void);

int stm32_saes_init(struct stm32_saes_context *ctx, bool is_decrypt,
		    enum stm32_saes_chaining_mode ch_mode, enum stm32_saes_key_selection key_select,
		    const void *key, size_t key_len, const void *iv, size_t iv_len);
int stm32_saes_update(struct stm32_saes_context *ctx, bool last_block,
		      uint8_t *data_in, uint8_t *data_out, size_t data_len);
int stm32_saes_update_assodata(struct stm32_saes_context *ctx, bool last_block,
			       uint8_t *data, size_t data_len);
int stm32_saes_update_load(struct stm32_saes_context *ctx, bool last_block,
			   uint8_t *data_in, uint8_t *data_out, size_t data_len);
int stm32_saes_final(struct stm32_saes_context *ctx, uint8_t *tag, size_t tag_len);
#endif

/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_PKA_H
#define STM32_PKA_H

#include <stdint.h>

#if !PKA_USE_NIST_P256 && !PKA_USE_BRAINPOOL_P256R1 && !PKA_USE_BRAINPOOL_P256T1 && \
	!PKA_USE_NIST_P521
#error "At least one ECDSA curve needs to be selected"
#endif

enum stm32_pka_ecdsa_curve_id {
#if PKA_USE_NIST_P256
	PKA_NIST_P256,
#endif
#if PKA_USE_BRAINPOOL_P256R1
	PKA_BRAINPOOL_P256R1,
#endif
#if PKA_USE_BRAINPOOL_P256T1
	PKA_BRAINPOOL_P256T1,
#endif
#if PKA_USE_NIST_P521
	PKA_NIST_P521,
#endif
};

struct stm32_pka_platdata {
	uintptr_t base;
	unsigned long clock_id;
	unsigned int reset_id;
};

int stm32_pka_init(void);
int stm32_pka_ecdsa_verif(void *hash, unsigned int hash_size,
			  void *sig_r_ptr, unsigned int sig_r_size,
			  void *sig_s_ptr, unsigned int sig_s_size,
			  void *pk_x_ptr, unsigned int pk_x_size,
			  void *pk_y_ptr, unsigned int pk_y_size,
			  enum stm32_pka_ecdsa_curve_id cid);

#endif /* STM32_PKA_H */

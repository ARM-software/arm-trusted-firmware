/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _RSA_H__
#define _RSA_H__

/* RSA key size defines */
#define RSA_4K_KEY_SZ       4096
#define RSA_4K_KEY_SZ_BYTES (RSA_4K_KEY_SZ/8)
#define RSA_2K_KEY_SZ       2048
#define RSA_2K_KEY_SZ_BYTES (RSA_2K_KEY_SZ/8)
#define RSA_1K_KEY_SZ       1024
#define RSA_1K_KEY_SZ_BYTES (RSA_1K_KEY_SZ/8)

#define SHA256_BYTES        (256/8)

struct pk_in_params {
	uint8_t *e;
	uint32_t e_siz;
	uint8_t *n;
	uint32_t n_siz;
	uint8_t *a;
	uint32_t a_siz;
	uint8_t *b;
	uint32_t b_siz;
};

struct rsa_context {
	struct pk_in_params pkin;
};

int rsa_verify_signature(void *hash_ptr, unsigned int hash_len,
			 void *sig_ptr, unsigned int sig_len,
			 void *pk_ptr, unsigned int pk_len);

#endif

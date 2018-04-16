/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CRYPTO_MOD_H__
#define __CRYPTO_MOD_H__

/* Return values */
enum crypto_ret_value {
	CRYPTO_SUCCESS = 0,
	CRYPTO_ERR_INIT,
	CRYPTO_ERR_HASH,
	CRYPTO_ERR_SIGNATURE,
	CRYPTO_ERR_UNKNOWN
};

/* List of hash algorithms */
enum hash_algo {
	SHA1 = 0,
	SHA256,
	SHA512
};

/*
 * Cryptographic library descriptor
 */
typedef struct crypto_lib_desc_s {
	const char *name;

	/* Initialize library. This function is not expected to fail. All errors
	 * must be handled inside the function, asserting or panicing in case of
	 * a non-recoverable error */
	void (*init)(void);

	/* Verify a digital signature. Return one of the
	 * 'enum crypto_ret_value' options */
	int (*verify_signature)(void *data_ptr, unsigned int data_len,
				void *sig_ptr, unsigned int sig_len,
				void *sig_alg, unsigned int sig_alg_len,
				void *pk_ptr, unsigned int pk_len);

	/* Verify a hash. Return one of the 'enum crypto_ret_value' options */
	int (*verify_hash)(void *data_ptr, unsigned int data_len,
			   void *digest_info_ptr, unsigned int digest_info_len);

	/* Initilaize the context for hash function */
	int (*hash_init)(enum hash_algo algo, void **ctx);

	/* Add chunk of data to be hashed */
	int (*hash_update)(enum hash_algo algo, void *ctx,
			   void *data_ptr, unsigned int data_len);

	/* Place the final digest in hash_ptr */
	int (*hash_final)(enum hash_algo algo, void *ctx, void *hash_ptr,
			  unsigned int hash_len);
} crypto_lib_desc_t;

/* Public functions */
void crypto_mod_init(void);
int crypto_mod_verify_signature(void *data_ptr, unsigned int data_len,
				void *sig_ptr, unsigned int sig_len,
				void *sig_alg_ptr, unsigned int sig_alg_len,
				void *pk_ptr, unsigned int pk_len);
int crypto_mod_verify_hash(void *data_ptr, unsigned int data_len,
			   void *digest_info_ptr, unsigned int digest_info_len);
int crypto_hash_init(enum hash_algo algo, void **ctx);
int crypto_hash_update(enum hash_algo algo, void *ctx,
		       void *data_ptr, unsigned int data_len);
int crypto_hash_final(enum hash_algo algo, void *ctx, void *hash_ptr,
		      unsigned int hash_len);

/* Macro to register a cryptographic library */
#define REGISTER_CRYPTO_LIB(_name, _init, _verify_signature, _verify_hash, \
			    _hash_init, _hash_update, _hash_final) \
	const crypto_lib_desc_t crypto_lib_desc = { \
		.name = _name, \
		.init = _init, \
		.verify_signature = _verify_signature, \
		.verify_hash = _verify_hash, \
		.hash_init = _hash_init, \
		.hash_update = _hash_update, \
		.hash_final = _hash_final \
	}

extern const crypto_lib_desc_t crypto_lib_desc;

#endif /* __CRYPTO_MOD_H__ */

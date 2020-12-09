/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch_helpers.h>
#include "caam.h"
#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>

#include "jobdesc.h"
#include "rsa.h"
#include "sec_hw_specific.h"

/* This array contains DER value for SHA-256 */
static const uint8_t hash_identifier[] = {
	0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60,
	0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00,
	0x04, 0x20
};

static void rsa_done(uint32_t *desc, uint32_t status, void *arg,
		     void *job_ring)
{
	INFO("RSA Desc SUCCESS with status %x\n", status);
}

static int rsa_public_verif_sec(uint8_t *sign, uint8_t *to,
				uint8_t *rsa_pub_key, uint32_t klen)
{
	int ret = 0;
	struct rsa_context ctx __aligned(CACHE_WRITEBACK_GRANULE);
	struct job_descriptor jobdesc __aligned(CACHE_WRITEBACK_GRANULE);

	jobdesc.arg = NULL;
	jobdesc.callback = rsa_done;

	memset(&ctx, 0, sizeof(struct rsa_context));

	ctx.pkin.a = sign;
	ctx.pkin.a_siz = klen;
	ctx.pkin.n = rsa_pub_key;
	ctx.pkin.n_siz = klen;
	ctx.pkin.e = rsa_pub_key + klen;
	ctx.pkin.e_siz = klen;

	cnstr_jobdesc_pkha_rsaexp(jobdesc.desc, &ctx.pkin, to, klen);

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	flush_dcache_range((uintptr_t)sign, klen);
	flush_dcache_range((uintptr_t)rsa_pub_key, 2 * klen);
	flush_dcache_range((uintptr_t)&ctx.pkin, sizeof(ctx.pkin));
	inv_dcache_range((uintptr_t)to, klen);

	dmbsy();
	dsbsy();
	isb();
#endif

	/* Finally, generate the requested random data bytes */
	ret = run_descriptor_jr(&jobdesc);
	if (ret != 0) {
		ERROR("Error in running descriptor\n");
		ret = -1;
	}
#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	inv_dcache_range((uintptr_t)to, klen);
	dmbsy();
	dsbsy();
	isb();
#endif
	return ret;
}

/*
 * Construct encoded hash EM' wrt PKCSv1.5. This function calculates the
 * pointers for padding, DER value and hash. And finally, constructs EM'
 * which includes hash of complete CSF header and ESBC image. If SG flag
 * is on, hash of SG table and entries is also included.
 */
static int construct_img_encoded_hash_second(uint8_t *hash, uint8_t hash_len,
					     uint8_t *encoded_hash_second,
					     unsigned int key_len)
{
	/*
	 * RSA PKCSv1.5 encoding format for encoded message is below
	 * EM = 0x0 || 0x1 || PS || 0x0 || DER || Hash
	 * PS is Padding String
	 * DER is DER value for SHA-256
	 * Hash is SHA-256 hash
	 * *********************************************************
	 * representative points to first byte of EM initially and is
	 * filled with 0x0
	 * representative is incremented by 1 and second byte is filled
	 * with 0x1
	 * padding points to third byte of EM
	 * digest points to full length of EM - 32 bytes
	 * hash_id (DER value) points to 19 bytes before pDigest
	 * separator is one byte which separates padding and DER
	 */

	unsigned int len;
	uint8_t *representative;
	uint8_t *padding, *digest;
	uint8_t *hash_id, *separator;
	int i;
	int ret = 0;

	if (hash_len != SHA256_BYTES) {
		return -1;
	}

	/* Key length = Modulus length */
	len = (key_len / 2U) - 1U;
	representative = encoded_hash_second;
	representative[0] = 0U;
	representative[1] = 1U;	/* block type 1 */

	padding = &representative[2];
	digest = &representative[1] + len - 32;
	hash_id = digest - sizeof(hash_identifier);
	separator = hash_id - 1;

	/* fill padding area pointed by padding with 0xff */
	memset(padding, 0xff, separator - padding);

	/* fill byte pointed by separator */
	*separator = 0U;

	/* fill SHA-256 DER value  pointed by HashId */
	memcpy(hash_id, hash_identifier, sizeof(hash_identifier));

	/* fill hash pointed by Digest */
	for (i = 0; i < SHA256_BYTES; i++) {
		digest[i] = hash[i];
	}

	return ret;
}

int rsa_verify_signature(void *hash_ptr, unsigned int hash_len,
			 void *sig_ptr, unsigned int sig_len,
			 void *pk_ptr, unsigned int pk_len)
{
	uint8_t img_encoded_hash_second[RSA_4K_KEY_SZ_BYTES];
	uint8_t encoded_hash[RSA_4K_KEY_SZ_BYTES] __aligned(CACHE_WRITEBACK_GRANULE);
	int ret = 0;

	ret = construct_img_encoded_hash_second(hash_ptr, hash_len,
						img_encoded_hash_second,
						pk_len);
	if (ret != 0) {
		ERROR("Encoded Hash Failure\n");
		return CRYPTO_ERR_SIGNATURE;
	}

	ret = rsa_public_verif_sec(sig_ptr, encoded_hash, pk_ptr, pk_len / 2);
	if (ret != 0) {
		ERROR("RSA signature Failure\n");
		return CRYPTO_ERR_SIGNATURE;
	}

	ret = memcmp(img_encoded_hash_second, encoded_hash, sig_len);
	if (ret != 0) {
		ERROR("Comparison Failure\n");
		return CRYPTO_ERR_SIGNATURE;
	}

	return CRYPTO_SUCCESS;
}

/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <delegated_attestation.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

psa_status_t
rss_delegated_attest_get_delegated_key(uint8_t   ecc_curve,
				       uint32_t  key_bits,
				       uint8_t  *key_buf,
				       size_t    key_buf_size,
				       size_t   *key_size,
				       uint32_t  hash_algo)
{
	psa_status_t status;
	psa_invec in_vec[] = {
		{&ecc_curve, sizeof(ecc_curve)},
		{&key_bits,  sizeof(key_bits)},
		{&hash_algo, sizeof(hash_algo)}
	};
	psa_outvec out_vec[] = {
		{key_buf, key_buf_size}
	};

	if (key_size == NULL) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	status = psa_call(RSS_DELEGATED_SERVICE_HANDLE,
			  RSS_DELEGATED_ATTEST_GET_DELEGATED_KEY,
			  in_vec,  IOVEC_LEN(in_vec),
			  out_vec, IOVEC_LEN(out_vec));
	if (status == PSA_SUCCESS) {
		*key_size = out_vec[0].len;
	}

	return status;
}

psa_status_t
rss_delegated_attest_get_token(const uint8_t *dak_pub_hash,
			       size_t         dak_pub_hash_size,
			       uint8_t       *token_buf,
			       size_t         token_buf_size,
			       size_t        *token_size)
{
	psa_status_t status;
	psa_invec in_vec[] = {
		{dak_pub_hash, dak_pub_hash_size}
	};
	psa_outvec out_vec[] = {
		{token_buf, token_buf_size}
	};

	if (token_size == NULL) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	status = psa_call(RSS_DELEGATED_SERVICE_HANDLE,
			  RSS_DELEGATED_ATTEST_GET_PLATFORM_TOKEN,
			  in_vec, IOVEC_LEN(in_vec),
			  out_vec, IOVEC_LEN(out_vec));
	if (status == PSA_SUCCESS) {
		*token_size = out_vec[0].len;
	}

	return status;
}

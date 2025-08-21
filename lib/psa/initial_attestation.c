/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <initial_attestation.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

psa_status_t rse_initial_attest_get_token(const uint8_t *challenge_buf,
					  size_t challenge_size,
					  uint8_t *token_buf,
					  size_t token_buf_size,
					  size_t *token_size)
{
	psa_status_t status;
	psa_invec in_vec[] = { { challenge_buf, challenge_size } };
	psa_outvec out_vec[] = { { token_buf, token_buf_size } };

	if (token_size == NULL) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	status = psa_call(RSE_INITIAL_ATTESTATION_SERVICE_HANDLE,
			  RSE_INITIAL_ATTESTATION_GET_TOKEN, in_vec,
			  IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));
	if (status == PSA_SUCCESS) {
		*token_size = out_vec[0].len;
	}

	if (status == PSA_OPERATION_INCOMPLETE) {
		*token_size = 0;
	}

	return status;
}

psa_status_t rse_initial_attest_get_token_size(size_t challenge_size,
					       size_t *token_size)
{
	psa_status_t status;
	psa_invec in_vec[] = { { &challenge_size, sizeof(challenge_size) } };
	psa_outvec out_vec[1];

	if (token_size == NULL) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	out_vec[0].base = token_size;
	out_vec[0].len = sizeof(*token_size);

	status = psa_call(RSE_INITIAL_ATTESTATION_SERVICE_HANDLE,
			  RSE_INITIAL_ATTESTATION_GET_TOKEN_SIZE, in_vec,
			  IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));
	if (status == PSA_SUCCESS) {
		*token_size = out_vec[0].len;
	}

	if (status == PSA_OPERATION_INCOMPLETE) {
		*token_size = 0;
	}

	return status;
}

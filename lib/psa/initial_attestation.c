/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <initial_attestation.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

psa_status_t
psa_initial_attest_get_token(const uint8_t *auth_challenge,
			     size_t         challenge_size,
			     uint8_t       *token_buf,
			     size_t         token_buf_size,
			     size_t        *token_size)
{
	psa_status_t status;
	psa_invec in_vec[] = {
		{auth_challenge, challenge_size}
	};
	psa_outvec out_vec[] = {
		{token_buf, token_buf_size},
	};

	status = psa_call(RSS_ATTESTATION_SERVICE_HANDLE, RSS_ATTEST_GET_TOKEN,
			  in_vec, IOVEC_LEN(in_vec),
			  out_vec, IOVEC_LEN(out_vec));

	if (status == PSA_SUCCESS) {
		*token_size = out_vec[0].len;
	}

	return status;
}

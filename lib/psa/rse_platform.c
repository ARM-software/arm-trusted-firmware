/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <psa/client.h>
#include <psa_manifest/sid.h>
#if CRYPTO_SUPPORT
#include <rse_crypto_defs.h>
#endif
#include <rse_platform_api.h>

psa_status_t
rse_platform_nv_counter_increment(uint32_t counter_id)
{
	struct psa_invec in_vec[1];

	in_vec[0].base = &counter_id;
	in_vec[0].len = sizeof(counter_id);

	return psa_call(RSE_PLATFORM_SERVICE_HANDLE,
			RSE_PLATFORM_API_ID_NV_INCREMENT,
			in_vec, 1, NULL, 0);
}

psa_status_t
rse_platform_nv_counter_read(uint32_t counter_id,
		uint32_t size, uint8_t *val)
{
	struct psa_invec in_vec[1];
	struct psa_outvec out_vec[1];

	in_vec[0].base = &counter_id;
	in_vec[0].len = sizeof(counter_id);

	out_vec[0].base = val;
	out_vec[0].len = size;

	return psa_call(RSE_PLATFORM_SERVICE_HANDLE,
			RSE_PLATFORM_API_ID_NV_READ,
			in_vec, 1, out_vec, 1);
}

#if CRYPTO_SUPPORT
psa_status_t
rse_platform_get_entropy(uint8_t *data, size_t data_size)
{
	psa_status_t status;

	struct rse_crypto_pack_iovec iov = {
		.function_id = RSE_CRYPTO_GENERATE_RANDOM_SID,
	};

	psa_invec in_vec[] = {
		{.base = &iov, .len = sizeof(struct rse_crypto_pack_iovec)},
	};
	psa_outvec out_vec[] = {
		{.base = data, .len = data_size}
	};

	status = psa_call(RSE_CRYPTO_HANDLE, PSA_IPC_CALL,
			  in_vec, IOVEC_LEN(in_vec),
			  out_vec, IOVEC_LEN(out_vec));

	return status;
}
#endif

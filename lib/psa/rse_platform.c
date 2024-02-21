/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <psa/client.h>
#include <psa_manifest/sid.h>
#include <rss_crypto_defs.h>
#include <rss_platform_api.h>

psa_status_t
rss_platform_nv_counter_increment(uint32_t counter_id)
{
	struct psa_invec in_vec[1];

	in_vec[0].base = &counter_id;
	in_vec[0].len = sizeof(counter_id);

	return psa_call(RSS_PLATFORM_SERVICE_HANDLE,
			RSS_PLATFORM_API_ID_NV_INCREMENT,
			in_vec, 1, NULL, 0);
}

psa_status_t
rss_platform_nv_counter_read(uint32_t counter_id,
		uint32_t size, uint8_t *val)
{
	struct psa_invec in_vec[1];
	struct psa_outvec out_vec[1];

	in_vec[0].base = &counter_id;
	in_vec[0].len = sizeof(counter_id);

	out_vec[0].base = val;
	out_vec[0].len = size;

	return psa_call(RSS_PLATFORM_SERVICE_HANDLE,
			RSS_PLATFORM_API_ID_NV_READ,
			in_vec, 1, out_vec, 1);
}

psa_status_t
rss_platform_key_read(enum rss_key_id_builtin_t key, uint8_t *data,
		size_t data_size, size_t *data_length)
{
	psa_status_t status;

	struct rss_crypto_pack_iovec iov = {
		.function_id = RSS_CRYPTO_EXPORT_PUBLIC_KEY_SID,
		.key_id = key,
	};

	psa_invec in_vec[] = {
		{.base = &iov, .len = sizeof(struct rss_crypto_pack_iovec)},
	};
	psa_outvec out_vec[] = {
		{.base = data, .len = data_size}
	};

	status = psa_call(RSS_CRYPTO_HANDLE, PSA_IPC_CALL,
			in_vec, IOVEC_LEN(in_vec),
			out_vec, IOVEC_LEN(out_vec));

	*data_length = out_vec[0].len;

	return status;
}

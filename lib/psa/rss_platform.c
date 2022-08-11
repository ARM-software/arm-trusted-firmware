/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>

#include <psa/client.h>
#include <psa_manifest/sid.h>
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

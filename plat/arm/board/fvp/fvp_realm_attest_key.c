/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <delegated_attestation.h>
#include <psa/error.h>
#include <services/rmmd_svc.h>

/*
 * Get the delegated realm attestation key through the PSA delegated
 * attestation layer.
 *
 * FVP cannot support RSS hardware at the moment, but it can still mock
 * the RSS implementation of the PSA interface (see PLAT_RSS_NOT_SUPPORTED).
 */
int plat_rmmd_get_cca_realm_attest_key(uintptr_t buf, size_t *len,
				       unsigned int type)
{
	psa_status_t ret;

	assert(type == ATTEST_KEY_CURVE_ECC_SECP384R1);

	ret = rss_delegated_attest_get_delegated_key(0U, 0U, (uint8_t *)buf,
						     *len, len, 0U);

	return ret;
}

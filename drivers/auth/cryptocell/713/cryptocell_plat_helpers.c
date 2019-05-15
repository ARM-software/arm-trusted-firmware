/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>

#include <lib/libc/endian.h>
#include <drivers/arm/cryptocell/713/bsv_api.h>
#include <drivers/arm/cryptocell/713/bsv_error.h>

/*
 * Return the ROTPK hash
 *
 * Return: 0 = success, Otherwise = error
 */
int cc_get_rotpk_hash(unsigned char *dst, unsigned int len, unsigned int *flags)
{
	CCError_t error;
	uint32_t lcs;
	int i;
	uint32_t *key = (uint32_t *)dst;

	assert(dst != NULL);
	assert(len >= HASH_RESULT_SIZE_IN_WORDS);
	assert(flags != NULL);

	error = CC_BsvLcsGet(PLAT_CRYPTOCELL_BASE, &lcs);
	if (error != CC_OK)
		return 1;

	if ((lcs == CC_BSV_CHIP_MANUFACTURE_LCS) || (lcs == CC_BSV_RMA_LCS)) {
		*flags = ROTPK_NOT_DEPLOYED;
		return 0;
	}

	error = CC_BsvPubKeyHashGet(PLAT_CRYPTOCELL_BASE,
				    CC_SB_HASH_BOOT_KEY_256B,
				    key, HASH_RESULT_SIZE_IN_WORDS);

	if (error == CC_BSV_HASH_NOT_PROGRAMMED_ERR) {
		*flags = ROTPK_NOT_DEPLOYED;
		return 0;
	}

	if (error == CC_OK) {

		/* Keys are stored in OTP in little-endian format */
		for (i = 0; i < HASH_RESULT_SIZE_IN_WORDS; i++)
			key[i] = le32toh(key[i]);

		*flags = ROTPK_IS_HASH;
		return 0;
	}

	return 1;
}

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * specifies the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	CCError_t error = CC_FAIL;

	if (strcmp(cookie, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = CC_BsvSwVersionGet(PLAT_CRYPTOCELL_BASE,
					   CC_SW_VERSION_TRUSTED, nv_ctr);
	} else if (strcmp(cookie, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = CC_BsvSwVersionGet(PLAT_CRYPTOCELL_BASE,
					   CC_SW_VERSION_NON_TRUSTED, nv_ctr);
	}

	return (error != CC_OK);
}

/*
 * Store a new non-volatile counter value in the counter specified by the OID
 * in the cookie. This function is not expected to be called if the Lifecycle
 * state is RMA as the values in the certificate are expected to always match
 * the nvcounter values. But if called when the LCS is RMA, the underlying
 * helper functions will return success but without updating the counter.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	CCError_t error = CC_FAIL;

	if (strcmp(cookie, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = CC_BsvSwVersionSet(PLAT_CRYPTOCELL_BASE,
					   CC_SW_VERSION_TRUSTED, nv_ctr);
	} else if (strcmp(cookie, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = CC_BsvSwVersionSet(PLAT_CRYPTOCELL_BASE,
					   CC_SW_VERSION_NON_TRUSTED, nv_ctr);
	}

	return (error != CC_OK);
}


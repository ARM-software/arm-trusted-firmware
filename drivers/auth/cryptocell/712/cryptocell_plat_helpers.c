/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>

#include <common/debug.h>
#include <drivers/arm/cryptocell/712/sbrom_bsv_api.h>
#include <drivers/arm/cryptocell/712/nvm.h>
#include <drivers/arm/cryptocell/712/nvm_otp.h>

/*
 * Return the ROTPK hash
 *
 * dst:   buffer into which the ROTPK hash will be copied into
 * len:   length of the provided buffer, which must be at least enough for a
 *        SHA256 hash
 * flags: a pointer to integer that will be set to indicate the ROTPK status
 *
 * Return: 0 = success, Otherwise = error
 */
int cc_get_rotpk_hash(unsigned char *dst, unsigned int len, unsigned int *flags)
{
	CCError_t error;
	uint32_t lcs;

	assert(dst != NULL);
	assert(len >= HASH_RESULT_SIZE_IN_WORDS);
	assert(flags != NULL);

	error = NVM_GetLCS(PLAT_CRYPTOCELL_BASE, &lcs);
	if (error != CC_OK)
		return 1;

	/* If the lifecycle state is `SD`, return failure */
	if (lcs == CC_BSV_SECURITY_DISABLED_LCS)
		return 1;

	/*
	 * If the lifecycle state is `CM` or `DM`, ROTPK shouldn't be verified.
	 * Return success after setting ROTPK_NOT_DEPLOYED flag
	 */
	if ((lcs == CC_BSV_CHIP_MANUFACTURE_LCS) ||
			(lcs == CC_BSV_DEVICE_MANUFACTURE_LCS)) {
		*flags = ROTPK_NOT_DEPLOYED;
		return 0;
	}

	/* Copy the DER header */
	error = NVM_ReadHASHPubKey(PLAT_CRYPTOCELL_BASE,
			CC_SB_HASH_BOOT_KEY_256B,
			(uint32_t *)dst, HASH_RESULT_SIZE_IN_WORDS);
	if (error != CC_OK)
		return 1;

	*flags = ROTPK_IS_HASH;
	return 0;
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
		error = NVM_GetSwVersion(PLAT_CRYPTOCELL_BASE,
				CC_SW_VERSION_COUNTER1, nv_ctr);
	} else if (strcmp(cookie, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = NVM_GetSwVersion(PLAT_CRYPTOCELL_BASE,
				CC_SW_VERSION_COUNTER2, nv_ctr);
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
		error = NVM_SetSwVersion(PLAT_CRYPTOCELL_BASE,
				CC_SW_VERSION_COUNTER1, nv_ctr);
	} else if (strcmp(cookie, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		error = NVM_SetSwVersion(PLAT_CRYPTOCELL_BASE,
				CC_SW_VERSION_COUNTER2, nv_ctr);
	}

	return (error != CC_OK);
}


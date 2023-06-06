/*
 * Copyright (c) 2016-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/fconf/fconf.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/fconf_nv_cntr_getter.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/cca_oid.h>

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	return arm_get_rotpk_info(cookie, key_ptr, key_len, flags);
}

/*
 * Return the non-volatile counter address stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
static int plat_get_nv_ctr_addr(void *cookie, uintptr_t *nv_ctr_addr)
{
	const char *oid = (const char *)cookie;

	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		*nv_ctr_addr = FCONF_GET_PROPERTY(cot, nv_cntr_addr,
						TRUSTED_NV_CTR_ID);
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		*nv_ctr_addr = FCONF_GET_PROPERTY(cot, nv_cntr_addr,
						NON_TRUSTED_NV_CTR_ID);
	} else if (strcmp(oid, CCA_FW_NVCOUNTER_OID) == 0) {
		/* FVP does not support the CCA NV Counter so use the Trusted NV */
		*nv_ctr_addr = FCONF_GET_PROPERTY(cot, nv_cntr_addr,
						TRUSTED_NV_CTR_ID);
	} else {
		return 1;
	}

	return 0;
}

/*
 * Store a new non-volatile counter value.
 *
 * On some FVP versions, the non-volatile counters are read-only so this
 * function will always fail.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	uintptr_t nv_ctr_addr;
	int rc;

	assert(cookie != NULL);

	rc = plat_get_nv_ctr_addr(cookie, &nv_ctr_addr);
	if (rc != 0) {
		return rc;
	}

	mmio_write_32(nv_ctr_addr, nv_ctr);

	/*
	 * If the FVP models a locked counter then its value cannot be updated
	 * and the above write operation has been silently ignored.
	 */
	return (mmio_read_32(nv_ctr_addr) == nv_ctr) ? 0 : 1;
}

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	uintptr_t nv_ctr_addr;
	int rc;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	rc = plat_get_nv_ctr_addr(cookie, &nv_ctr_addr);
	if (rc != 0) {
		return rc;
	}

	*nv_ctr = *((unsigned int *)nv_ctr_addr);

	return 0;
}

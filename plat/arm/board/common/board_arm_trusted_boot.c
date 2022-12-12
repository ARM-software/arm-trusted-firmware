/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/arm/cryptocell/cc_rotpk.h>
#include <drivers/delay_timer.h>
#include <lib/cassert.h>
#include <lib/fconf/fconf.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#if defined(ARM_COT_cca)
#include <tools_share/cca_oid.h>
#elif defined(ARM_COT_dualroot)
#include <tools_share/dualroot_oid.h>
#elif defined(ARM_COT_tbbr)
#include <tools_share/tbbr_oid.h>
#endif

#include <plat/arm/common/fconf_nv_cntr_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#if !ARM_CRYPTOCELL_INTEG
#if !ARM_ROTPK_LOCATION_ID
  #error "ARM_ROTPK_LOCATION_ID not defined"
#endif
#endif

#if COT_DESC_IN_DTB && defined(IMAGE_BL2)
uintptr_t nv_cntr_base_addr[MAX_NV_CTR_IDS];
#else
uintptr_t nv_cntr_base_addr[MAX_NV_CTR_IDS] = {
	TFW_NVCTR_BASE,
	NTFW_CTR_BASE
};
#endif


/* Weak definition may be overridden in specific platform */
#pragma weak plat_get_nv_ctr
#pragma weak plat_set_nv_ctr

extern unsigned char arm_rotpk_header[], arm_rotpk_key[], arm_rotpk_hash_end[],
       arm_rotpk_key_end[];

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID) || ARM_CRYPTOCELL_INTEG
static unsigned char rotpk_hash_der[ARM_ROTPK_HEADER_LEN + ARM_ROTPK_HASH_LEN];
#endif

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID)
/*
 * Return the ROTPK hash stored in dedicated registers.
 */
int arm_get_rotpk_info_regs(void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	uint8_t *dst;
	uint32_t *src, tmp;
	unsigned int words, i;

	assert(key_ptr != NULL);
	assert(key_len != NULL);
	assert(flags != NULL);

	/* Copy the DER header */

	memcpy(rotpk_hash_der, arm_rotpk_header, ARM_ROTPK_HEADER_LEN);
	dst = (uint8_t *)&rotpk_hash_der[ARM_ROTPK_HEADER_LEN];

	words = ARM_ROTPK_HASH_LEN >> 2;

	src = (uint32_t *)TZ_PUB_KEY_HASH_BASE;
	for (i = 0 ; i < words ; i++) {
		tmp = src[words - 1 - i];
		/* Words are read in little endian */
		*dst++ = (uint8_t)(tmp & 0xFF);
		*dst++ = (uint8_t)((tmp >> 8) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 16) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 24) & 0xFF);
	}

	*key_ptr = (void *)rotpk_hash_der;
	*key_len = (unsigned int)sizeof(rotpk_hash_der);
	*flags = ROTPK_IS_HASH;
	return 0;
}
#endif

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) || \
    (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID) || \
    (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_FULL_DEV_RSA_KEY_ID)
int arm_get_rotpk_info_dev(void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_FULL_DEV_RSA_KEY_ID) {
		*key_ptr = arm_rotpk_key;
		*key_len = arm_rotpk_key_end - arm_rotpk_key;
		*flags = 0;
	} else {
		*key_ptr = arm_rotpk_header;
		*key_len = arm_rotpk_hash_end - arm_rotpk_header;
		*flags = ROTPK_IS_HASH;
	}
	return 0;
}
#endif

#if ARM_CRYPTOCELL_INTEG
/*
 * Return ROTPK hash from CryptoCell.
 */
int arm_get_rotpk_info_cc(void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	unsigned char *dst;

	assert(key_ptr != NULL);
	assert(key_len != NULL);
	assert(flags != NULL);

	/* Copy the DER header */
	memcpy(rotpk_hash_der, arm_rotpk_header, ARM_ROTPK_HEADER_LEN);
	dst = &rotpk_hash_der[ARM_ROTPK_HEADER_LEN];
	*key_ptr = rotpk_hash_der;
	*key_len = sizeof(rotpk_hash_der);
	return cc_get_rotpk_hash(dst, ARM_ROTPK_HASH_LEN, flags);
}
#endif

/*
 * Wrapper function for most Arm platforms to get ROTPK info.
 */
static int get_rotpk_info(void **key_ptr, unsigned int *key_len,
				unsigned int *flags)
{
#if ARM_CRYPTOCELL_INTEG
	return arm_get_rotpk_info_cc(key_ptr, key_len, flags);
#else

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) || \
    (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID) || \
    (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_FULL_DEV_RSA_KEY_ID)
	return arm_get_rotpk_info_dev(key_ptr, key_len, flags);
#elif (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID)
	return arm_get_rotpk_info_regs(key_ptr, key_len, flags);
#else
	return 1;
#endif
#endif /* ARM_CRYPTOCELL_INTEG */
}

#if defined(ARM_COT_tbbr)

int arm_get_rotpk_info(void *cookie __unused, void **key_ptr,
		       unsigned int *key_len, unsigned int *flags)
{
	return get_rotpk_info(key_ptr, key_len, flags);
}

#elif defined(ARM_COT_dualroot)

int arm_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
		       unsigned int *flags)
{
	/*
	 * Return the right root of trust key hash based on the cookie value:
	 *  - NULL means the primary ROTPK.
	 *  - Otherwise, interpret cookie as the OID of the certificate
	 *    extension containing the key.
	 */
	if (cookie == NULL) {
		return get_rotpk_info(key_ptr, key_len, flags);
	} else if (strcmp(cookie, PROT_PK_OID) == 0) {
		extern unsigned char arm_protpk_hash[];
		extern unsigned char arm_protpk_hash_end[];
		*key_ptr = arm_protpk_hash;
		*key_len = arm_protpk_hash_end - arm_protpk_hash;
		*flags = ROTPK_IS_HASH;
		return 0;
	} else {
		/* Invalid key ID. */
		return 1;
	}
}

#elif defined(ARM_COT_cca)

int arm_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
		       unsigned int *flags)
{
	/*
	 * Return the right root of trust key hash based on the cookie value:
	 *  - NULL means the primary ROTPK.
	 *  - Otherwise, interpret cookie as the OID of the certificate
	 *    extension containing the key.
	 */
	if (cookie == NULL) {
		return get_rotpk_info(key_ptr, key_len, flags);
	} else if (strcmp(cookie, PROT_PK_OID) == 0) {
		extern unsigned char arm_protpk_hash[];
		extern unsigned char arm_protpk_hash_end[];
		*key_ptr = arm_protpk_hash;
		*key_len = arm_protpk_hash_end - arm_protpk_hash;
		*flags = ROTPK_IS_HASH;
		return 0;
	} else if (strcmp(cookie, SWD_ROT_PK_OID) == 0) {
		extern unsigned char arm_swd_rotpk_hash[];
		extern unsigned char arm_swd_rotpk_hash_end[];
		*key_ptr = arm_swd_rotpk_hash;
		*key_len = arm_swd_rotpk_hash_end - arm_swd_rotpk_hash;
		*flags = ROTPK_IS_HASH;
		return 0;
	} else {
		/* Invalid key ID. */
		return 1;
	}
}

#endif

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	const char *oid;
	uint32_t *nv_ctr_addr;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = (uint32_t *)FCONF_GET_PROPERTY(cot, nv_cntr_addr,
							TRUSTED_NV_CTR_ID);
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = (uint32_t *)FCONF_GET_PROPERTY(cot, nv_cntr_addr,
							NON_TRUSTED_NV_CTR_ID);
	} else {
		return 1;
	}

	*nv_ctr = (unsigned int)(*nv_ctr_addr);

	return 0;
}

/*
 * Store a new non-volatile counter value. By default on ARM development
 * platforms, the non-volatile counters are RO and cannot be modified. We expect
 * the values in the certificates to always match the RO values so that this
 * function is never called.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 1;
}

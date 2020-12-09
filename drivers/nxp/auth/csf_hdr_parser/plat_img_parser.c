/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2017-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <csf_hdr.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/auth/img_parser_mod.h>
#include <lib/utils.h>
#include <sfp.h>

/* Temporary variables to speed up the authentication parameters search. These
 * variables are assigned once during the integrity check and used any time an
 * authentication parameter is requested, so we do not have to parse the image
 * again.
 */

/* Hash of Image + CSF Header + SRK table */
uint8_t img_hash[SHA256_BYTES] __aligned(CACHE_WRITEBACK_GRANULE);
uint32_t hash_len;

/* Key being used for authentication
 * Points to the key in CSF header copied in DDR
 * ESBC client key
 */
void *img_key;
uint32_t key_len;

/* ESBC client signature */
void *img_sign;
uint32_t sign_len;
enum sig_alg alg;

/* Maximum OID string length ("a.b.c.d.e.f ...") */
#define MAX_OID_STR_LEN			64

#define LIB_NAME	"NXP CSFv2"

/*
 * Clear all static temporary variables.
 */
static void clear_temp_vars(void)
{
#define ZERO_AND_CLEAN(x)					\
	do {							\
		zeromem(&x, sizeof(x));				\
		clean_dcache_range((uintptr_t)&x, sizeof(x));	\
	} while (0)

	ZERO_AND_CLEAN(img_key);
	ZERO_AND_CLEAN(img_sign);
	ZERO_AND_CLEAN(img_hash);
	ZERO_AND_CLEAN(key_len);
	ZERO_AND_CLEAN(hash_len);
	ZERO_AND_CLEAN(sign_len);

#undef ZERO_AND_CLEAN
}

/* Exported functions */

static void init(void)
{
	clear_temp_vars();
}

/*
 * This function would check the integrity of the CSF header
 */
static int check_integrity(void *img, unsigned int img_len)
{
	int ret;

	/*
	 * The image file has been successfully loaded till here.
	 *
	 * Flush the image to main memory so that it can be authenticated
	 * by CAAM, a HW accelerator regardless of cache and MMU state.
	 */
	flush_dcache_range((uintptr_t) img, img_len);

	/*
	 * Image is appended at an offset of 16K (IMG_OFFSET) to the header.
	 * So the size in header should be equal to img_len - IMG_OFFSET
	 */
	VERBOSE("Barker code is %x\n", *(unsigned int *)img);
	ret = validate_esbc_header(img, &img_key, &key_len, &img_sign,
				   &sign_len, &alg);
	if (ret < 0) {
		ERROR("Header authentication failed\n");
		clear_temp_vars();
		return IMG_PARSER_ERR;
	}
	/* Calculate the hash of various components from the image */
	ret = calc_img_hash(img, (uint8_t *)img + CSF_HDR_SZ,
			    img_len - CSF_HDR_SZ, img_hash, &hash_len);
	if (ret != 0) {
		ERROR("Issue in hash calculation %d\n", ret);
		clear_temp_vars();
		return IMG_PARSER_ERR;
	}

	return IMG_PARSER_OK;
}

/*
 * Extract an authentication parameter from CSF header
 *
 * CSF header has already been parsed and the required information like
 * hash of data, signature, length stored in global variables has been
 * extracted in chek_integrity function.  This data
 * is returned back to the caller.
 */
static int get_auth_param(const auth_param_type_desc_t *type_desc,
		void *img, unsigned int img_len,
		void **param, unsigned int *param_len)
{
	int rc = IMG_PARSER_OK;

	/* We do not use img because the check_integrity function has already
	 * extracted the relevant data ( pk, sig_alg, etc)
	 */

	switch (type_desc->type) {

	/* Hash will be returned for comparison with signature */
	case AUTH_PARAM_HASH:
		*param = (void *)img_hash;
		*param_len = (unsigned int)SHA256_BYTES;
		break;

	/* Return the public key used for signature extracted from the SRK table
	 * after checks with key revocation
	 */
	case AUTH_PARAM_PUB_KEY:
		/* Get the subject public key */
		/* For a 1K key - the length would be 2k/8 = 0x100 bytes
		 * 2K RSA key - 0x200 , 4K RSA - 0x400
		 */
		*param = img_key;
		*param_len = (unsigned int)key_len;
		break;

	/* Call a function to tell if signature is RSA or ECDSA. ECDSA to be
	 * supported in later platforms like LX2 etc
	 */
	case AUTH_PARAM_SIG_ALG:
		/* Algo will be signature - RSA or ECDSA  on hash */
		*param = (void *)&alg;
		*param_len = 4U;
		break;

	/* Return the signature */
	case AUTH_PARAM_SIG:
		*param = img_sign;
		*param_len = (unsigned int)sign_len;
		break;

	case AUTH_PARAM_NV_CTR:

	default:
		rc = IMG_PARSER_ERR_NOT_FOUND;
		break;
	}

	return rc;
}

REGISTER_IMG_PARSER_LIB(IMG_PLAT, LIB_NAME, init,
			check_integrity, get_auth_param);

/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * X509 parser based on mbed TLS
 *
 * This module implements functions to check the integrity of a X509v3
 * certificate ASN.1 structure and extract authentication parameters from the
 * extensions field, such as an image hash or a public key.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* mbed TLS headers */
#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#include <mbedtls/platform.h>

#include <arch_helpers.h>
#include <drivers/auth/img_parser_mod.h>
#include <drivers/auth/mbedtls/mbedtls_common.h>
#include <lib/utils.h>

/* Maximum OID string length ("a.b.c.d.e.f ...") */
#define MAX_OID_STR_LEN			64

#define LIB_NAME	"mbed TLS X509v3"

/* Temporary variables to speed up the authentication parameters search. These
 * variables are assigned once during the integrity check and used any time an
 * authentication parameter is requested, so we do not have to parse the image
 * again */
static mbedtls_asn1_buf tbs;
static mbedtls_asn1_buf v3_ext;
static mbedtls_asn1_buf pk;
static mbedtls_asn1_buf sig_alg;
static mbedtls_asn1_buf signature;

/*
 * Clear all static temporary variables.
 */
static void clear_temp_vars(void)
{
#define ZERO_AND_CLEAN(x)					\
	do {							\
		zeromem(&x, sizeof(x));				\
		clean_dcache_range((uintptr_t)&x, sizeof(x));	\
	} while (0);

	ZERO_AND_CLEAN(tbs)
	ZERO_AND_CLEAN(v3_ext);
	ZERO_AND_CLEAN(pk);
	ZERO_AND_CLEAN(sig_alg);
	ZERO_AND_CLEAN(signature);

#undef ZERO_AND_CLEAN
}

/*
 * Get X509v3 extension
 *
 * Global variable 'v3_ext' must point to the extensions region
 * in the certificate.  OID may be NULL to request that get_ext()
 * is only being called for integrity checking.
 */
static int get_ext(const char *oid, void **ext, unsigned int *ext_len)
{
	int oid_len, ret, is_critical;
	size_t len;
	unsigned char *p;
	const unsigned char *end;
	char oid_str[MAX_OID_STR_LEN];
	mbedtls_asn1_buf extn_oid;

	p = v3_ext.p;
	end = v3_ext.p + v3_ext.len;

	/*
	 * Check extensions integrity.  At least one extension is
	 * required: the ASN.1 specifies a minimum size of 1, and at
	 * least one extension is needed to authenticate the next stage
	 * in the boot chain.
	 */
	do {
		unsigned char *end_ext_data;

		ret = mbedtls_asn1_get_tag(&p, end, &len,
					   MBEDTLS_ASN1_CONSTRUCTED |
					   MBEDTLS_ASN1_SEQUENCE);
		if (ret != 0) {
			return IMG_PARSER_ERR_FORMAT;
		}
		end_ext_data = p + len;

		/* Get extension ID */
		ret = mbedtls_asn1_get_tag(&p, end_ext_data, &extn_oid.len,
					   MBEDTLS_ASN1_OID);
		if (ret != 0) {
			return IMG_PARSER_ERR_FORMAT;
		}
		extn_oid.tag = MBEDTLS_ASN1_OID;
		extn_oid.p = p;
		p += extn_oid.len;

		/* Get optional critical */
		ret = mbedtls_asn1_get_bool(&p, end_ext_data, &is_critical);
		if ((ret != 0) && (ret != MBEDTLS_ERR_ASN1_UNEXPECTED_TAG)) {
			return IMG_PARSER_ERR_FORMAT;
		}

		/*
		 * Data should be octet string type and must use all bytes in
		 * the Extension.
		 */
		ret = mbedtls_asn1_get_tag(&p, end_ext_data, &len,
					   MBEDTLS_ASN1_OCTET_STRING);
		if ((ret != 0) || ((p + len) != end_ext_data)) {
			return IMG_PARSER_ERR_FORMAT;
		}

		/* Detect requested extension */
		oid_len = mbedtls_oid_get_numeric_string(oid_str,
							 MAX_OID_STR_LEN,
							 &extn_oid);
		if ((oid_len == MBEDTLS_ERR_OID_BUF_TOO_SMALL) || (oid_len < 0)) {
			return IMG_PARSER_ERR;
		}

		if ((oid != NULL) &&
		    ((size_t)oid_len == strlen(oid_str)) &&
		    (strcmp(oid, oid_str) == 0)) {
			/* Extension must be ASN.1 DER */
			if (len < 2) {
				/* too short */
				return IMG_PARSER_ERR_FORMAT;
			}

			if ((p[0] & 0x1F) == 0x1F) {
				/* multi-byte ASN.1 DER tag, not allowed */
				return IMG_PARSER_ERR_FORMAT;
			}

			if ((p[0] & 0xDF) == 0) {
				/* UNIVERSAL 0 tag, not allowed */
				return IMG_PARSER_ERR_FORMAT;
			}

			*ext = (void *)p;
			*ext_len = (unsigned int)len;

			/* Advance past the tag byte */
			p++;

			if (mbedtls_asn1_get_len(&p, end_ext_data, &len)) {
				/* not valid DER */
				return IMG_PARSER_ERR_FORMAT;
			}

			if (p + len != end_ext_data) {
				/* junk after ASN.1 object */
				return IMG_PARSER_ERR_FORMAT;
			}

			return IMG_PARSER_OK;
		}

		/* Next */
		p = end_ext_data;
	} while (p < end);

	return (oid == NULL) ? IMG_PARSER_OK : IMG_PARSER_ERR_NOT_FOUND;
}


/*
 * Check the integrity of the certificate ASN.1 structure.
 *
 * Extract the relevant data that will be used later during authentication.
 *
 * This function doesn't clear the static variables located on the top of this
 * file in case of an error. It is only called from check_integrity(), which
 * performs the cleanup if necessary.
 */
static int cert_parse(void *img, unsigned int img_len)
{
	int ret;
	size_t len;
	unsigned char *p, *end, *crt_end, *pk_end;
	mbedtls_asn1_buf sig_alg1;
	/*
	 * The unique ASN.1 DER encoding of [0] EXPLICIT INTEGER { v3(2} }.
	 */
	static const char v3[] = {
		/* The outer CONTEXT SPECIFIC 0 tag */
		MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_CONTEXT_SPECIFIC | 0,
		/* The number bytes used to encode the inner INTEGER */
		3,
		/* The tag of the inner INTEGER */
		MBEDTLS_ASN1_INTEGER,
		/* The number of bytes needed to represent 2 */
		1,
		/* The actual value 2 */
		2,
	};

	p = (unsigned char *)img;
	len = img_len;
	crt_end = p + len;
	end = crt_end;

	/*
	 * Certificate  ::=  SEQUENCE  {
	 *      tbsCertificate       TBSCertificate,
	 *      signatureAlgorithm   AlgorithmIdentifier,
	 *      signatureValue       BIT STRING  }
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if ((ret != 0) || ((p + len) != end)) {
		return IMG_PARSER_ERR_FORMAT;
	}

	/*
	 * TBSCertificate  ::=  SEQUENCE  {
	 */
	tbs.p = p;
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	end = p + len;
	tbs.len = end - tbs.p;

	/*
	 * Version  ::=  [0] EXPLICIT INTEGER {  v1(0), v2(1), v3(2)  }
	 * -- only v3 accepted
	 */
	if (((end - p) <= (ptrdiff_t)sizeof(v3)) ||
	    (memcmp(p, v3, sizeof(v3)) != 0)) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += sizeof(v3);

	/*
	 * CertificateSerialNumber  ::=  INTEGER
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_INTEGER);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += len;

	/*
	 * signature            AlgorithmIdentifier
	 */
	sig_alg1.p = p;
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	sig_alg1.len = (p + len) - sig_alg1.p;
	p += len;

	/*
	 * issuer               Name
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += len;

	/*
	 * Validity ::= SEQUENCE {
	 *      notBefore      Time,
	 *      notAfter       Time }
	 *
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += len;

	/*
	 * subject              Name
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += len;

	/*
	 * SubjectPublicKeyInfo
	 */
	pk.p = p;
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	pk_end = p + len;
	pk.len = pk_end - pk.p;

	/* algorithm */
	ret = mbedtls_asn1_get_tag(&p, pk_end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += len;

	/* Key is a BIT STRING and must use all bytes in SubjectPublicKeyInfo */
	ret = mbedtls_asn1_get_bitstring_null(&p, pk_end, &len);
	if ((ret != 0) || (p + len != pk_end)) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p = pk_end;

	/*
	 * issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
	 * subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
	 * -- technically these contain BIT STRINGs but that is not worth
	 * -- validating
	 */
	for (int i = 1; i < 3; i++) {
		ret = mbedtls_asn1_get_tag(&p, end, &len,
					   MBEDTLS_ASN1_CONTEXT_SPECIFIC |
					   MBEDTLS_ASN1_CONSTRUCTED | i);
		/*
		 * Unique IDs are obsolete, so MBEDTLS_ERR_ASN1_UNEXPECTED_TAG
		 * is the common case.
		 */
		if (ret != MBEDTLS_ERR_ASN1_UNEXPECTED_TAG) {
			if (ret != 0) {
				return IMG_PARSER_ERR_FORMAT;
			}
			p += len;
		}
	}

	/*
	 * extensions      [3]  EXPLICIT Extensions OPTIONAL
	 * }
	 *
	 * X.509 and RFC5280 allow omitting the extensions entirely.
	 * However, in TF-A, a certificate with no extensions would
	 * always fail later on, as the extensions contain the
	 * information needed to authenticate the next stage in the
	 * boot chain.  Furthermore, get_ext() assumes that the
	 * extensions have been parsed into v3_ext, and allowing
	 * there to be no extensions would pointlessly complicate
	 * the code.  Therefore, just reject certificates without
	 * extensions.  This is also why version 1 and 2 certificates
	 * are rejected above.
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len,
				   MBEDTLS_ASN1_CONTEXT_SPECIFIC |
				   MBEDTLS_ASN1_CONSTRUCTED | 3);
	if ((ret != 0) || (len != (size_t)(end - p))) {
		return IMG_PARSER_ERR_FORMAT;
	}

	/*
	 * Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
	 * -- must use all remaining bytes in TBSCertificate
	 */
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if ((ret != 0) || (len != (size_t)(end - p))) {
		return IMG_PARSER_ERR_FORMAT;
	}
	v3_ext.p = p;
	v3_ext.len = len;
	p += len;

	/* Check extensions integrity */
	ret = get_ext(NULL, NULL, NULL);
	if (ret != IMG_PARSER_OK) {
		return ret;
	}

	end = crt_end;

	/*
	 *  }
	 *  -- end of TBSCertificate
	 *
	 *  signatureAlgorithm   AlgorithmIdentifier
	 *  -- Does not need to be parsed.  Ensuring it is bitwise
	 *  -- identical (including the tag!) with the first signature
	 *  -- algorithm is sufficient.
	 */
	if ((sig_alg1.len >= (size_t)(end - p)) ||
	    (0 != memcmp(sig_alg1.p, p, sig_alg1.len))) {
		return IMG_PARSER_ERR_FORMAT;
	}
	p += sig_alg1.len;
	memcpy(&sig_alg, &sig_alg1, sizeof(sig_alg));

	/*
	 * signatureValue       BIT STRING
	 * } -- must consume all bytes
	 */
	signature.p = p;
	ret = mbedtls_asn1_get_bitstring_null(&p, end, &len);
	if ((ret != 0) || ((p + len) != end)) {
		return IMG_PARSER_ERR_FORMAT;
	}
	signature.len = end - signature.p;

	return IMG_PARSER_OK;
}


/* Exported functions */

static void init(void)
{
	mbedtls_init();
}

/*
 * Wrapper for cert_parse() that clears the static variables used by it in case
 * of an error.
 */
static int check_integrity(void *img, unsigned int img_len)
{
	int rc = cert_parse(img, img_len);

	if (rc != IMG_PARSER_OK)
		clear_temp_vars();

	return rc;
}

/*
 * Extract an authentication parameter from an X509v3 certificate
 *
 * This function returns a pointer to the extracted data and its length.
 * Depending on the type of parameter, a pointer to the data stored in the
 * certificate may be returned (i.e. an octet string containing a hash). Other
 * data may need to be copied and formatted (i.e. integers). In the later case,
 * a buffer of the correct type needs to be statically allocated, filled and
 * returned.
 */
static int get_auth_param(const auth_param_type_desc_t *type_desc,
		void *img, unsigned int img_len,
		void **param, unsigned int *param_len)
{
	int rc = IMG_PARSER_OK;

	/* We do not use img because the check_integrity function has already
	 * extracted the relevant data (v3_ext, pk, sig_alg, etc) */

	switch (type_desc->type) {
	case AUTH_PARAM_RAW_DATA:
		/* Data to be signed */
		*param = (void *)tbs.p;
		*param_len = (unsigned int)tbs.len;
		break;
	case AUTH_PARAM_HASH:
	case AUTH_PARAM_NV_CTR:
		/* All these parameters are included as X509v3 extensions */
		rc = get_ext(type_desc->cookie, param, param_len);
		break;
	case AUTH_PARAM_PUB_KEY:
		if (type_desc->cookie != NULL) {
			/* Get public key from extension */
			rc = get_ext(type_desc->cookie, param, param_len);
		} else {
			/* Get the subject public key */
			*param = (void *)pk.p;
			*param_len = (unsigned int)pk.len;
		}
		break;
	case AUTH_PARAM_SIG_ALG:
		/* Get the certificate signature algorithm */
		*param = (void *)sig_alg.p;
		*param_len = (unsigned int)sig_alg.len;
		break;
	case AUTH_PARAM_SIG:
		/* Get the certificate signature */
		*param = (void *)signature.p;
		*param_len = (unsigned int)signature.len;
		break;
	default:
		rc = IMG_PARSER_ERR_NOT_FOUND;
		break;
	}

	return rc;
}

REGISTER_IMG_PARSER_LIB(IMG_CERT, LIB_NAME, init,
		       check_integrity, get_auth_param);

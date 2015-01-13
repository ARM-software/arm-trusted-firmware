/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Authentication module based on PolarSSL */

#include <stddef.h>

#include <assert.h>
#include <auth.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <platform_oid.h>

#include <polarssl/memory_buffer_alloc.h>
#include <polarssl/oid.h>
#include <polarssl/platform.h>
#include <polarssl/sha256.h>
#include <polarssl/x509_crt.h>

/*
 * At each authentication stage, the module is responsible for extracting and
 * storing those elements (keys, hashes, etc.) that will be needed later on
 * during the Trusted Boot process.
 */

/* SHA256 algorithm */
#define SHA_BYTES			32

/*
 * An 8 KB stack has been proven to be enough for the current Trusted Boot
 * process
 */
#define POLARSSL_HEAP_SIZE		(8*1024)
static unsigned char heap[POLARSSL_HEAP_SIZE];

/*
 * RSA public keys:
 *  SubjectPublicKeyInfo  ::=  SEQUENCE  {          1 + 3
 *       algorithm            AlgorithmIdentifier,  1 + 1 (sequence)
 *                                                + 1 + 1 + 9 (rsa oid)
 *                                                + 1 + 1 (params null)
 *       subjectPublicKey     BIT STRING }          1 + 3 + (1 + below)
 *  RSAPublicKey ::= SEQUENCE {                     1 + 3
 *      modulus           INTEGER,  -- n            1 + 3 + MPI_MAX + 1
 *      publicExponent    INTEGER   -- e            1 + 3 + MPI_MAX + 1
 *  }
 *
 * POLARSSL_MPI_MAX_SIZE is set to 256 bytes (RSA-2048 bit keys) in the
 * configuration file
 */
#define RSA_PUB_DER_MAX_BYTES   38 + 2 * POLARSSL_MPI_MAX_SIZE

/*
 * Buffer for storing public keys extracted from certificates while they are
 * verified
 */
static unsigned char pk_buf[RSA_PUB_DER_MAX_BYTES];

/* We use this variable to parse and authenticate the certificates */
static x509_crt cert;

/* BL specific variables */
#if IMAGE_BL1
static unsigned char sha_bl2[SHA_BYTES];
#elif IMAGE_BL2
/* Buffers to store the hash of BL3-x images */
static unsigned char sha_bl30[SHA_BYTES];
static unsigned char sha_bl31[SHA_BYTES];
static unsigned char sha_bl32[SHA_BYTES];
static unsigned char sha_bl33[SHA_BYTES];
/* Buffers to store the Trusted and Non-Trusted world public keys */
static unsigned char tz_world_pk[RSA_PUB_DER_MAX_BYTES];
static unsigned char ntz_world_pk[RSA_PUB_DER_MAX_BYTES];
static size_t tz_world_pk_len, ntz_world_pk_len;
/* Buffer to store the BL3-x public keys */
static unsigned char content_pk[RSA_PUB_DER_MAX_BYTES];
static size_t content_pk_len;
#endif


static int x509_get_crt_ext_data(const unsigned char **ext_data,
				 size_t *ext_len,
				 x509_crt *crt,
				 const char *oid)
{
	int ret;
	size_t len;
	unsigned char *end_ext_data, *end_ext_octet;
	unsigned char *p;
	const unsigned char *end;
	char oid_str[64];

	p = crt->v3_ext.p;
	end = crt->v3_ext.p + crt->v3_ext.len;

	ret = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE);
	if (ret != 0)
		return POLARSSL_ERR_X509_INVALID_EXTENSIONS + ret;

	if (end != p + len)
		return POLARSSL_ERR_X509_INVALID_EXTENSIONS +
				POLARSSL_ERR_ASN1_LENGTH_MISMATCH;

	while (p < end) {
		/*
		 * Extension  ::=  SEQUENCE  {
		 *      extnID      OBJECT IDENTIFIER,
		 *      critical    BOOLEAN DEFAULT FALSE,
		 *      extnValue   OCTET STRING  }
		 */
		x509_buf extn_oid = {0, 0, NULL};
		int is_critical = 0; /* DEFAULT FALSE */

		ret = asn1_get_tag(&p, end, &len,
				ASN1_CONSTRUCTED | ASN1_SEQUENCE);
		if (ret != 0)
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS + ret;

		end_ext_data = p + len;

		/* Get extension ID */
		extn_oid.tag = *p;

		ret = asn1_get_tag(&p, end, &extn_oid.len, ASN1_OID);
		if (ret != 0)
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS + ret;

		extn_oid.p = p;
		p += extn_oid.len;

		if ((end - p) < 1)
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS +
					POLARSSL_ERR_ASN1_OUT_OF_DATA;

		/* Get optional critical */
		ret = asn1_get_bool(&p, end_ext_data, &is_critical);
		if (ret != 0 && (ret != POLARSSL_ERR_ASN1_UNEXPECTED_TAG))
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS + ret;

		/* Data should be octet string type */
		ret = asn1_get_tag(&p, end_ext_data, &len, ASN1_OCTET_STRING);
		if (ret != 0)
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS + ret;

		end_ext_octet = p + len;

		if (end_ext_octet != end_ext_data)
			return POLARSSL_ERR_X509_INVALID_EXTENSIONS +
					POLARSSL_ERR_ASN1_LENGTH_MISMATCH;

		/* Detect requested extension */
		oid_get_numeric_string(oid_str, 64, &extn_oid);
		if (memcmp(oid, oid_str, sizeof(oid)) == 0) {
			*ext_data = p;
			*ext_len = len;
			return 0;
		}

		/* Next */
		p = end_ext_octet;
	}

	if (p != end)
		return POLARSSL_ERR_X509_INVALID_EXTENSIONS +
				POLARSSL_ERR_ASN1_LENGTH_MISMATCH;

	return POLARSSL_ERR_X509_UNKNOWN_OID;
}

#if IMAGE_BL1
/*
 * Parse and verify the BL2 certificate
 *
 * This function verifies the integrity of the BL2 certificate, checks that it
 * has been signed with the ROT key and extracts the BL2 hash stored in the
 * certificate so it can be matched later against the calculated hash.
 *
 * Return: 0 = success, Otherwise = error
 */
static int check_bl2_cert(unsigned char *buf, size_t len)
{
	const unsigned char *p;
	size_t sz;
	int err, flags;

	x509_crt_init(&cert);

	/* Parse the BL2 certificate */
	err = x509_crt_parse(&cert, buf, len);
	if (err) {
		ERROR("BL2 certificate parse error %d.\n", err);
		goto error;
	}

	/* Check that it has been signed with the ROT key */
	err = pk_write_pubkey_der(&cert.pk, pk_buf, sizeof(pk_buf));
	if (err < 0) {
		ERROR("Error loading ROT key in DER format %d.\n", err);
		goto error;
	}

	sz = (size_t)err;
	p = pk_buf + sizeof(pk_buf) - sz;

	err = plat_match_rotpk(p, sz);
	if (err) {
		ERROR("ROT and BL2 certificate key mismatch\n");
		goto error;
	}

	/* Verify certificate */
	err = x509_crt_verify(&cert, &cert, NULL, NULL, &flags, NULL, NULL);
	if (err) {
		ERROR("BL2 certificate verification error %d. Flags: 0x%x.\n",
				err, flags);
		goto error;
	}

	/* Extract BL2 image hash from certificate */
	err = x509_get_crt_ext_data(&p, &sz, &cert, BL2_HASH_OID);
	if (err) {
		ERROR("Cannot read BL2 hash from certificate\n");
		goto error;
	}

	assert(sz == SHA_BYTES + 2);

	/* Skip the tag and length bytes and copy the hash */
	p += 2;
	memcpy(sha_bl2, p, SHA_BYTES);

error:
	x509_crt_free(&cert);

	return err;
}
#endif /* IMAGE_BL1 */

#if IMAGE_BL2
static int check_trusted_key_cert(unsigned char *buf, size_t len)
{
	const unsigned char *p;
	size_t sz;
	int err, flags;

	x509_crt_init(&cert);

	/* Parse the Trusted Key certificate */
	err = x509_crt_parse(&cert, buf, len);
	if (err) {
		ERROR("Trusted Key certificate parse error %d.\n", err);
		goto error;
	}

	/* Verify Trusted Key certificate */
	err = x509_crt_verify(&cert, &cert, NULL, NULL, &flags, NULL, NULL);
	if (err) {
		ERROR("Trusted Key certificate verification error %d. Flags: "
				"0x%x.\n", err, flags);
		goto error;
	}

	/* Check that it has been signed with the ROT key */
	err = pk_write_pubkey_der(&cert.pk, pk_buf, sizeof(pk_buf));
	if (err < 0) {
		ERROR("Error loading ROT key in DER format %d.\n", err);
		goto error;
	}

	sz = (size_t)err;
	p = pk_buf + sizeof(pk_buf) - sz;

	if (plat_match_rotpk(p, sz)) {
		ERROR("ROT and Trusted Key certificate key mismatch\n");
		goto error;
	}

	/* Extract Trusted World key from extensions */
	err = x509_get_crt_ext_data(&p, &tz_world_pk_len,
			&cert, TZ_WORLD_PK_OID);
	if (err) {
		ERROR("Cannot read Trusted World key\n");
		goto error;
	}

	assert(tz_world_pk_len <= RSA_PUB_DER_MAX_BYTES);
	memcpy(tz_world_pk, p, tz_world_pk_len);

	/* Extract Non-Trusted World key from extensions */
	err = x509_get_crt_ext_data(&p, &ntz_world_pk_len,
			&cert, NTZ_WORLD_PK_OID);
	if (err) {
		ERROR("Cannot read Non-Trusted World key\n");
		goto error;
	}

	assert(tz_world_pk_len <= RSA_PUB_DER_MAX_BYTES);
	memcpy(ntz_world_pk, p, ntz_world_pk_len);

error:
	x509_crt_free(&cert);

	return err;
}

static int check_bl3x_key_cert(const unsigned char *buf, size_t len,
			       const unsigned char *i_key, size_t i_key_len,
			       unsigned char *s_key, size_t *s_key_len,
			       const char *key_oid)
{
	const unsigned char *p;
	size_t sz;
	int err, flags;

	x509_crt_init(&cert);

	/* Parse key certificate */
	err = x509_crt_parse(&cert, buf, len);
	if (err) {
		ERROR("Key certificate parse error %d.\n", err);
		goto error;
	}

	/* Verify certificate */
	err = x509_crt_verify(&cert, &cert, NULL, NULL, &flags, NULL, NULL);
	if (err) {
		ERROR("Key certificate verification error %d. Flags: "
				"0x%x.\n", err, flags);
		goto error;
	}

	/* Check that the certificate has been signed by the issuer */
	err = pk_write_pubkey_der(&cert.pk, pk_buf, sizeof(pk_buf));
	if (err < 0) {
		ERROR("Error loading key in DER format %d.\n", err);
		goto error;
	}

	sz = (size_t)err;
	p = pk_buf + sizeof(pk_buf) - sz;
	if ((sz != i_key_len) || memcmp(p, i_key, sz)) {
		ERROR("Key certificate not signed with issuer key\n");
		err = 1;
		goto error;
	}

	/* Get the content certificate key */
	err = x509_get_crt_ext_data(&p, &sz, &cert, key_oid);
	if (err) {
		ERROR("Extension %s not found in Key certificate\n", key_oid);
		goto error;
	}

	assert(sz <= RSA_PUB_DER_MAX_BYTES);
	memcpy(s_key, p, sz);
	*s_key_len = sz;

error:
	x509_crt_free(&cert);

	return err;
}

static int check_bl3x_cert(unsigned char *buf, size_t len,
		       const unsigned char *i_key, size_t i_key_len,
		       const char *hash_oid, unsigned char *sha)
{
	const unsigned char *p;
	size_t sz;
	int err, flags;

	x509_crt_init(&cert);

	/* Parse BL31 content certificate */
	err = x509_crt_parse(&cert, buf, len);
	if (err) {
		ERROR("Content certificate parse error %d.\n", err);
		goto error;
	}

	/* Verify certificate */
	err = x509_crt_verify(&cert, &cert, NULL, NULL, &flags, NULL, NULL);
	if (err) {
		ERROR("Content certificate verification error %d. Flags: "
				"0x%x.\n", err, flags);
		goto error;
	}

	/* Check that content certificate has been signed with the content
	 * certificate key corresponding to this image */
	sz = pk_write_pubkey_der(&cert.pk, pk_buf, sizeof(pk_buf));
	p = pk_buf + sizeof(pk_buf) - sz;

	if ((sz != i_key_len) || memcmp(p, i_key, sz)) {
		ERROR("Content certificate not signed with content "
				"certificate key\n");
		err = 1;
		goto error;
	}

	/* Extract image hash from certificate */
	err = x509_get_crt_ext_data(&p, &sz, &cert, hash_oid);
	if (err) {
		ERROR("Cannot read hash from certificate\n");
		goto error;
	}

	assert(sz == SHA_BYTES + 2);

	/* Skip the tag and length bytes and copy the hash */
	p += 2;
	memcpy(sha, p, SHA_BYTES);

error:
	x509_crt_free(&cert);

	return err;
}
#endif /* IMAGE_BL2 */

/*
 * Calculate the hash of the image and check it against the hash extracted
 * previously from the certificate
 *
 * Parameters:
 *   buf: buffer where image is loaded
 *   len: size of the image
 *   sha: matching hash (extracted from the image certificate)
 *
 * Return: 0 = match, Otherwise = mismatch
 */
static int check_bl_img(unsigned char *buf, size_t len,
			const unsigned char *sha)
{
	unsigned char img_sha[SHA_BYTES];

	/* Calculate the hash of the image */
	sha256(buf, len, img_sha, 0);

	/* Match the hash with the one extracted from the certificate */
	if (memcmp(img_sha, sha, SHA_BYTES)) {
		ERROR("Image hash mismatch\n");
		return 1;
	}

	return 0;
}

/*
 * Object verification function
 *
 * The id parameter will indicate the expected format of the object
 * (certificate, image, etc).
 *
 * Return: 0 = success, Otherwise = error
 */
static int polarssl_mod_verify(unsigned int id, uintptr_t obj, size_t len)
{
	int ret;

	switch (id) {
#if IMAGE_BL1
	case AUTH_BL2_IMG_CERT:
		ret = check_bl2_cert((unsigned char *)obj, len);
		break;
	case AUTH_BL2_IMG:
		ret = check_bl_img((unsigned char *)obj, len, sha_bl2);
		break;
#endif /* IMAGE_BL1 */

#if IMAGE_BL2
	case AUTH_TRUSTED_KEY_CERT:
		ret = check_trusted_key_cert((unsigned char *)obj, len);
		break;
	case AUTH_BL30_KEY_CERT:
		ret = check_bl3x_key_cert((unsigned char *)obj, len,
				tz_world_pk, tz_world_pk_len,
				content_pk, &content_pk_len,
				BL30_CONTENT_CERT_PK_OID);
		break;
	case AUTH_BL31_KEY_CERT:
		ret = check_bl3x_key_cert((unsigned char *)obj, len,
				tz_world_pk, tz_world_pk_len,
				content_pk, &content_pk_len,
				BL31_CONTENT_CERT_PK_OID);
		break;
	case AUTH_BL32_KEY_CERT:
		ret = check_bl3x_key_cert((unsigned char *)obj, len,
				tz_world_pk, tz_world_pk_len,
				content_pk, &content_pk_len,
				BL32_CONTENT_CERT_PK_OID);
		break;
	case AUTH_BL33_KEY_CERT:
		ret = check_bl3x_key_cert((unsigned char *)obj, len,
				ntz_world_pk, ntz_world_pk_len,
				content_pk, &content_pk_len,
				BL33_CONTENT_CERT_PK_OID);
		break;
	case AUTH_BL30_IMG_CERT:
		ret = check_bl3x_cert((unsigned char *)obj, len,
				content_pk, content_pk_len,
				BL30_HASH_OID, sha_bl30);
		break;
	case AUTH_BL31_IMG_CERT:
		ret = check_bl3x_cert((unsigned char *)obj, len,
				content_pk, content_pk_len,
				BL31_HASH_OID, sha_bl31);
		break;
	case AUTH_BL32_IMG_CERT:
		ret = check_bl3x_cert((unsigned char *)obj, len,
				content_pk, content_pk_len,
				BL32_HASH_OID, sha_bl32);
		break;
	case AUTH_BL33_IMG_CERT:
		ret = check_bl3x_cert((unsigned char *)obj, len,
				content_pk, content_pk_len,
				BL33_HASH_OID, sha_bl33);
		break;
	case AUTH_BL30_IMG:
		ret = check_bl_img((unsigned char *)obj, len, sha_bl30);
		break;
	case AUTH_BL31_IMG:
		ret = check_bl_img((unsigned char *)obj, len, sha_bl31);
		break;
	case AUTH_BL32_IMG:
		ret = check_bl_img((unsigned char *)obj, len, sha_bl32);
		break;
	case AUTH_BL33_IMG:
		ret = check_bl_img((unsigned char *)obj, len, sha_bl33);
		break;
#endif /* IMAGE_BL2 */
	default:
		ret = -1;
		break;
	}

	return ret;
}

/*
 * Module initialization function
 *
 * Return: 0 = success, Otherwise = error
 */
static int polarssl_mod_init(void)
{
	/* Initialize the PolarSSL heap */
	return memory_buffer_alloc_init(heap, POLARSSL_HEAP_SIZE);
}

const auth_mod_t auth_mod = {
	.name = "PolarSSL",
	.init = polarssl_mod_init,
	.verify = polarssl_mod_verify
};

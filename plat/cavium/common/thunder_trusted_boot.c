/*
 * Copyright (c) 2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018, Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "thunder_dt.h"
#include "thunder_private.h"
#include <mbedtls/sha256.h>

#define ROTPK_BYTES			64

/* SHA256 algorithm */
#define SHA256_BYTES			32

#define FUSF_ROTPK_MAX			4

/* ROTPK locations */
#define ARM_ROTPK_REGS_ID		1
#define ARM_ROTPK_DEVEL_RSA_ID		2

#if !ARM_ROTPK_LOCATION_ID
  #error "ARM_ROTPK_LOCATION_ID not defined"
#endif

#define AES_KEY_BYTES			16

static const unsigned char rotpk_hash_hdr[] =		\
		"\x30\x31\x30\x0D\x06\x09\x60\x86\x48"	\
		"\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20";
static const unsigned int rotpk_hash_hdr_len = sizeof(rotpk_hash_hdr) - 1;
static unsigned char rotpk_hash_der[sizeof(rotpk_hash_hdr) - 1 + SHA256_BYTES];

static const unsigned char rotpk_asn1_hdr[] = \
		"\x30\x59" \
		"\x30\x13" \
		"\x06\x07" \
		"\x2a\x86\x48\xce\x3d\x02\x01" \
		"\x06\x08" \
		"\x2a\x86\x48\xce\x3d\x03\x01\x07" \
		"\x03\x42" \
		"\x00\x04";
static const unsigned int rotpk_asn1_hdr_len = sizeof(rotpk_asn1_hdr) - 1;
static unsigned char rotpk[sizeof(rotpk_asn1_hdr) - 1 + ROTPK_BYTES];

static unsigned char aes_key[AES_KEY_BYTES];

unsigned int nv_ctr_val = 0;

/*
 * This function is used to verify if data received from TRUST-ROT-ADDR
 * FDT property is actually the ROTPK.
 *
 * @param sha256                     - SHA256(ROTPK)
 * @param len                        - length of SHA256(ROTPK) in bytes
 *
 * return                            - 0 if hash from FUSF_ROTPK matches
 *                                     sha256_rotpk, -1 otherwise
 */
int thunder_verify_rotpk(const unsigned char * sha256, unsigned int len)
{
	union cavm_fusf_rotpkx fusf_rotpkx;
	uint64_t hash[FUSF_ROTPK_MAX];
	unsigned char *dst;
	int i, j, rc = 0;

	assert(sha256 != NULL);

	dst = (unsigned char *)hash;
	for (i = 0; i < FUSF_ROTPK_MAX; i++) {
		/* Convert sha256 to FUSF_ROTPK format */
		for (j = 0; j < 8; j++) {
			*dst++ = sha256[8*i + j] & 0xFF;
		}

		/* Get hash value from FUSF_ROTPK */
		fusf_rotpkx.u = CSR_READ_PA(0, CAVM_FUSF_ROTPKX(i));

		/* Compare hashes */
		if (fusf_rotpkx.s.dat != hash[i]) {
			rc = -1;
		}
	}

	return rc;
}

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
	uint8_t *dst;
	unsigned int i, words;
	uint64_t *src, tmp;
	unsigned char *ptr;

	assert(key_ptr != NULL);
	assert(key_len != NULL);
	assert(flags != NULL);
	assert(bfdt.trust_rot_addr != 0);

	/* Copy the DER header of ROTPK into rotpk structure */
	memcpy(rotpk, rotpk_asn1_hdr, rotpk_asn1_hdr_len);

	/* Move the pointer a header_len forward */
	ptr = &rotpk[rotpk_asn1_hdr_len];

	/* Copy memory from trust_rot_addr to global table indicated by ptr */
	memcpy(ptr, (unsigned char *)bfdt.trust_rot_addr, ROTPK_BYTES);

	/* Calculate the hash of ROTPK */
	mbedtls_sha256(ptr, ROTPK_BYTES, rotpk_hash_der, 0);

	/* Verify if the hash matches FUSF_ROTPK registers */
	if (thunder_verify_rotpk(rotpk_hash_der, SHA256_BYTES) != 0) {
		printf("ERROR:   Hash of ROTPK from FDT and hash from FUSF registers do not match\n");
		return -1;
	}

	/* Convert ROTPK to big endian partly Qx/Qy */
	words = 4;
	src = (uint64_t *)bfdt.trust_rot_addr;
	for (i = 0; i < words; i++) {
		tmp = src[words - 1 - i];
		*ptr++ = (unsigned char)((tmp >> 56) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 48) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 40) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 32) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 24) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 16) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 8) & 0xFF);
		*ptr++ = (unsigned char)(tmp & 0xFF);
	}
	src = (uint64_t *)(bfdt.trust_rot_addr + ROTPK_BYTES/2);
	for (i = 0; i < words; i++) {
		tmp = src[words - 1 - i];
		*ptr++ = (unsigned char)((tmp >> 56) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 48) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 40) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 32) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 24) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 16) & 0xFF);
		*ptr++ = (unsigned char)((tmp >> 8) & 0xFF);
		*ptr++ = (unsigned char)(tmp & 0xFF);
	}

	/* Copy the DER header of ROTPK_HASH */
	memcpy(rotpk_hash_der, rotpk_hash_hdr, rotpk_hash_hdr_len);
	dst = (uint8_t *)&rotpk_hash_der[rotpk_hash_hdr_len];

	/* Finally, calculate SHA256 on ROTPK and store it in dst */
	mbedtls_sha256(rotpk, sizeof(rotpk), dst, 0);

	/* Assign outputs */
	*key_ptr = (void *)rotpk_hash_der;
	*key_len = (unsigned int)sizeof(rotpk_hash_der);
	*flags = ROTPK_IS_HASH;
	return 0;
}

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	const char *oid;
	union cavm_fusf_ctl fusf_ctl;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	oid = (const char *)cookie;

	/* Cavium platform uses the same address for both counters */
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0 ||
	    strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		fusf_ctl.u = CSR_READ_PA(0, CAVM_FUSF_CTL);
		nv_ctr_val = 0;
		/* Convert value from rom_t_cnt to unsigned int */
		if (fusf_ctl.s.rom_t_cnt)
			nv_ctr_val = 32 - __builtin_clz(fusf_ctl.s.rom_t_cnt);

		*nv_ctr = nv_ctr_val;
	} else {
		return 1;
	}

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

/*
 * Get the AES key stored at either FDT address or at FUSF_SSKX(0..1) registers
 */
int plat_get_crypt_key(unsigned char **key, unsigned int *key_len)
{
	unsigned char *ptr;
	uint64_t ssk[2];
	int i;

	ptr = &aes_key[0];
	if (bfdt.trust_key_addr != 0) {
		/* Try to get the BSSK key */
		memcpy(ptr, (unsigned char *)bfdt.trust_key_addr, AES_KEY_BYTES);
	} else {
		/*
		 * Otherwise, copy content from FUSF_SSKX regs into aes_key.
		 * SSK will be used to decrypt images.
		 */
		for (i = 0; i < 2; i++)
			ssk[i] = CSR_READ_PA(0, CAVM_FUSF_SSKX(i));
		memcpy(ptr, ssk, AES_KEY_BYTES);
	}

	/* Assign outputs */
	*key = aes_key;
	*key_len = (unsigned int)sizeof(aes_key);
	return 0;
}

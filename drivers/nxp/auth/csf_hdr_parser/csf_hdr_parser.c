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
#include <cassert.h>
#include <common/debug.h>
#include <csf_hdr.h>
#include <dcfg.h>
#include <drivers/auth/crypto_mod.h>
#include <lib/utils.h>
#include <sfp.h>

/* Maximum OID string length ("a.b.c.d.e.f ...") */
#define MAX_OID_STR_LEN			64

#define LIB_NAME	"NXP CSFv2"

#ifdef CSF_HDR_CH3
/* Barker Code for LS Ch3 ESBC Header */
static const uint8_t barker_code[CSF_BARKER_LEN] = { 0x12, 0x19, 0x20, 0x01 };
#else
static const uint8_t barker_code[CSF_BARKER_LEN] = { 0x68, 0x39, 0x27, 0x81 };
#endif

#define CHECK_KEY_LEN(key_len)	(((key_len) == 2 * RSA_1K_KEY_SZ_BYTES) || \
				 ((key_len) == 2 * RSA_2K_KEY_SZ_BYTES) || \
				 ((key_len) == 2 * RSA_4K_KEY_SZ_BYTES))

/* Flag to indicate if values are there in rotpk_hash_table */
bool rotpk_not_dpld =  true;
uint8_t rotpk_hash_table[MAX_KEY_ENTRIES][SHA256_BYTES] __aligned(CACHE_WRITEBACK_GRANULE);
uint32_t num_rotpk_hash_entries;

/*
 * This function deploys the hashes of the various platform keys in
 * rotpk_hash_table. This is done in case of secure boot after comparison
 * of table's hash with the hash in SFP fuses. This installation is done
 * only in the first header parsing.
 */
static int deploy_rotpk_hash_table(void *srk_buffer, uint16_t num_srk)
{
	void *ctx;
	int ret = 0;
	int i, j = 0;
	unsigned int digest_size = SHA256_BYTES;
	enum hash_algo algo = SHA256;
	uint8_t hash[SHA256_BYTES];
	uint32_t srk_hash[SHA256_BYTES/4] __aligned(CACHE_WRITEBACK_GRANULE);
	struct srk_table *srktbl = (void *)srk_buffer;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(get_sfp_addr()
							+ SFP_FUSE_REGS_OFFSET);


	if (num_srk > MAX_KEY_ENTRIES) {
		return -1;
	}

	ret = hash_init(algo, &ctx);
	if (ret != 0) {
		return -1;
	}

	/* Update hash with that of SRK table */
	ret = hash_update(algo, ctx, (uint8_t *)((uint8_t *)srk_buffer),
			  num_srk * sizeof(struct srk_table));
	if (ret != 0) {
		return -1;
	}

	/* Copy hash at destination buffer */
	ret = hash_final(algo, ctx, hash, digest_size);
	if (ret != 0) {
		return -1;
	}

	/* Add comparison of hash with SFP hash here */
	for (i = 0; i < SHA256_BYTES/4; i++) {
		srk_hash[i] =
			mmio_read_32((uintptr_t)&sfp_ccsr_regs->srk_hash[i]);
	}

	VERBOSE("SRK table HASH\n");
	for (i = 0; i < 8; i++) {
		VERBOSE("%x\n", *((uint32_t *)hash + i));
	}

	if (memcmp(hash, srk_hash, SHA256_BYTES) != 0) {
		ERROR("Error in installing ROTPK table\n");
		ERROR("SRK hash doesn't match the fuse hash\n");
		return -1;
	}

	/* Hash table already deployed */
	if (rotpk_not_dpld == false) {
		return 0;
	}

	for (i = 0; i < num_srk; i++) {
		ret = hash_init(algo, &ctx);
		if (ret != 0) {
			return -1;
		}

		/* Update hash with that of SRK table */
		ret = hash_update(algo, ctx, srktbl[i].pkey, srktbl[i].key_len);
		if (ret != 0) {
			return -1;
		}

		/* Copy hash at destination buffer */
		ret = hash_final(algo, ctx, rotpk_hash_table[i], digest_size);
		if (ret != 0) {
			return -1;
		}
		VERBOSE("Table key %d HASH\n", i);
		for (j = 0; j < 8; j++) {
			VERBOSE("%x\n", *((uint32_t *)rotpk_hash_table[i] + j));
		}
	}
	rotpk_not_dpld = false;
	num_rotpk_hash_entries = num_srk;

	return 0;
}

/*
 * Calculate hash of ESBC hdr and ESBC. This function calculates the
 * single hash of ESBC header and ESBC image
 */
int calc_img_hash(struct csf_hdr *hdr,
		  void *img_addr, uint32_t img_size,
		  uint8_t *img_hash, uint32_t *hash_len)
{
	void *ctx;
	int ret = 0;
	unsigned int digest_size = SHA256_BYTES;
	enum hash_algo algo = SHA256;

	ret = hash_init(algo, &ctx);
	/* Copy hash at destination buffer */
	if (ret != 0) {
		return -1;
	}

	/* Update hash for CSF Header */
	ret = hash_update(algo, ctx, (uint8_t *)hdr, sizeof(struct csf_hdr));
	if (ret != 0) {
		return -1;
	}

	/* Update hash with that of SRK table */
	ret = hash_update(algo, ctx,
			  (uint8_t *)((uint8_t *)hdr + hdr->srk_tbl_off),
			  hdr->len_kr.num_srk * sizeof(struct srk_table));
	if (ret != 0) {
		return -1;
	}

	/* Update hash for actual Image */
	ret = hash_update(algo, ctx, (uint8_t *)(img_addr), img_size);
	if (ret != 0) {
		return -1;
	}

	/* Copy hash at destination buffer */
	ret = hash_final(algo, ctx, img_hash, digest_size);
	if (ret != 0) {
		return -1;
	}

	*hash_len = digest_size;

	VERBOSE("IMG encoded HASH\n");
	for (int i = 0; i < 8; i++) {
		VERBOSE("%x\n", *((uint32_t *)img_hash + i));
	}

	return 0;
}

/* This function checks if selected key is revoked or not.*/
static uint32_t is_key_revoked(uint32_t keynum, uint32_t rev_flag)
{
	if (keynum == UNREVOCABLE_KEY) {
		return 0;
	}

	if (((uint32_t)(1 << (REVOC_KEY_ALIGN - keynum)) & rev_flag) != 0) {
		return 1;
	}

	return 0;
}

/* Parse the header to extract the type of key,
 * Check if key is not revoked
 * and return the key , key length and key_type
 */
static int32_t get_key(struct csf_hdr *hdr, uint8_t **key, uint32_t *len,
			enum sig_alg *key_type)
{
	int i = 0;
	uint32_t ret = 0U;
	uint32_t key_num, key_revoc_flag;
	void *esbc = hdr;
	struct srk_table *srktbl = (void *)((uint8_t *)esbc + hdr->srk_tbl_off);
	bool sb;
	uint32_t mode;

	/* We currently support only RSA keys and signature */
	*key_type = RSA;

	/* Check for number of SRK entries */
	if ((hdr->len_kr.num_srk == 0) ||
	    (hdr->len_kr.num_srk > MAX_KEY_ENTRIES)) {
		ERROR("Error in NUM entries in SRK Table\n");
		return -1;
	}

	/*
	 * Check the key number field. It should be not greater than
	 * number of entries in SRK table.
	 */
	key_num = hdr->len_kr.srk_sel;
	if ((key_num == 0) || (key_num > hdr->len_kr.num_srk)) {
		ERROR("Invalid Key number\n");
		return -1;
	}

	/* Get revoc key from sfp */
	key_revoc_flag = get_key_revoc();

	/* Check if selected key has been revoked */
	ret = is_key_revoked(key_num, key_revoc_flag);
	if (ret != 0) {
		ERROR("Selected key has been revoked\n");
		return -1;
	}

	/* Check for valid key length - allowed key sized 1k, 2k and 4K */
	for (i = 0; i < hdr->len_kr.num_srk; i++) {
		if (CHECK_KEY_LEN(srktbl[i].key_len) == 0) {
			ERROR("Invalid key length\n");
			return -1;
		}
	}

	/* We don't return error from here. While parsing we just try to
	 * install the srk table. Failure needs to be taken care of in
	 * case of secure boot. This failure will be handled at the time
	 * of rotpk comparison in plat_get_rotpk_info function
	 */
	sb = check_boot_mode_secure(&mode);
	if (sb) {
		ret = deploy_rotpk_hash_table(srktbl, hdr->len_kr.num_srk);
		if (ret != 0) {
			ERROR("ROTPK FAILURE\n");
			/* For ITS =1 , return failure */
			if (mode != 0) {
				return -1;
			}
			ERROR("SECURE BOOT DEV-ENV MODE:\n");
			ERROR("\tCHECK ROTPK !\n");
			ERROR("\tCONTINUING ON FAILURE...\n");
		}
	}

	/* Return the length of the selected key */
	*len = srktbl[key_num - 1].key_len;

	/* Point key to the selected key */
	*key =  (uint8_t *)&(srktbl[key_num - 1].pkey);

	return 0;
}

/*
 * This function would parse the CSF header and do the following:
 * 1. Basic integrity checks
 * 2. Key checks and extract the key from SRK/IE Table
 * 3. Key hash comparison with SRKH in fuses in case of SRK Table
 * 4. OEM/UID checks - To be added
 * 5. Hash calculation for various components used in signature
 * 6. Signature integrity checks
 * return -> 0 on success, -1 on failure
 */
int validate_esbc_header(void *img_hdr, void **img_key, uint32_t *key_len,
			 void **img_sign, uint32_t *sign_len,
			 enum sig_alg *algo)
{
	struct csf_hdr *hdr = img_hdr;
	uint8_t *s;
	int32_t ret = 0;
	void *esbc = (uint8_t *)img_hdr;
	uint8_t *key;
	uint32_t klen;

	/* check barker code */
	if (memcmp(hdr->barker, barker_code, CSF_BARKER_LEN) != 0) {
		ERROR("Wrong barker code in header\n");
		return -1;
	}

	ret = get_key(hdr, &key, &klen, algo);
	if (ret != 0) {
		return -1;
	}

	/* check signaure */
	if (klen == (2 * hdr->sign_len)) {
		/* check signature length */
		if (((hdr->sign_len == RSA_1K_KEY_SZ_BYTES) ||
		    (hdr->sign_len == RSA_2K_KEY_SZ_BYTES) ||
		    (hdr->sign_len == RSA_4K_KEY_SZ_BYTES)) == 0) {
			ERROR("Wrong Signature length in header\n");
			return -1;
		}
	} else {
		ERROR("RSA key length not twice the signature length\n");
		return -1;
	}

	/* modulus most significant bit should be set */

	if ((key[0] & 0x80) == 0U) {
		ERROR("RSA Public key MSB not set\n");
		return -1;
	}

	/* modulus value should be odd */
	if ((key[klen / 2 - 1] & 0x1) == 0U) {
		ERROR("Public key Modulus in header not odd\n");
		return -1;
	}

	/* Check signature value < modulus value */
	s =  (uint8_t *)(esbc + hdr->psign);

	if (!(memcmp(s, key, hdr->sign_len) < 0)) {
		ERROR("Signature not less than modulus");
		return -1;
	}

	/* Populate the return addresses */
	*img_sign = (void *)(s);

	/* Save the length of signature */
	*sign_len = hdr->sign_len;

	*img_key = (uint8_t *)key;

	*key_len = klen;

	return ret;
}

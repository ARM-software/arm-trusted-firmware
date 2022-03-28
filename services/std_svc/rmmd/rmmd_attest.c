/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <string.h>
#include <common/debug.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include "rmmd_private.h"
#include <services/rmmd_svc.h>

static spinlock_t lock;

/* For printing Realm attestation token hash */
#define DIGITS_PER_BYTE				2UL
#define LENGTH_OF_TERMINATING_ZERO_IN_BYTES	1UL
#define BYTES_PER_LINE_BASE			4UL

static void print_challenge(uint8_t *hash, size_t hash_size)
{
	size_t leftover;
	/*
	 * bytes_per_line is always a power of two, so it can be used to
	 * construct mask with it when it is necessary to count remainder.
	 *
	 */
	const size_t bytes_per_line = 1 << BYTES_PER_LINE_BASE;
	char hash_text[(1 << BYTES_PER_LINE_BASE) * DIGITS_PER_BYTE +
		LENGTH_OF_TERMINATING_ZERO_IN_BYTES];
	const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
				  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	unsigned int i;

	for (i = 0U; i < hash_size; ++i) {
		hash_text[(i & (bytes_per_line - 1)) * DIGITS_PER_BYTE] =
			hex_chars[hash[i] >> 4];
		hash_text[(i & (bytes_per_line - 1)) * DIGITS_PER_BYTE + 1] =
			hex_chars[hash[i] & 0x0f];
		if (((i + 1) & (bytes_per_line - 1)) == 0U) {
			hash_text[bytes_per_line * DIGITS_PER_BYTE] = '\0';
			VERBOSE("hash part %u = %s\n",
				(i >> BYTES_PER_LINE_BASE) + 1, hash_text);
		}
	}

	leftover = (size_t)i & (bytes_per_line - 1);

	if (leftover != 0UL) {
		hash_text[leftover * DIGITS_PER_BYTE] = '\0';
		VERBOSE("hash part %u = %s\n", (i >> BYTES_PER_LINE_BASE) + 1,
			hash_text);
	}
}

/*
 * TODO: Have different error codes for different errors so that the caller can
 * differentiate various error cases.
 */
int rmmd_attest_get_platform_token(uint64_t buf_pa, uint64_t *buf_len, uint64_t challenge_hash_len)
{
	int err;
	uintptr_t va;
	uint8_t temp_buf[SHA512_DIGEST_SIZE];

	/*
	 * TODO: Currently we don't validate incoming buf_pa. This is a
	 * prototype and we will need to allocate static buffer for EL3-RMM
	 * communication.
	 */

	/* We need a page of buffer to pass data */
	if (*buf_len != PAGE_SIZE) {
		ERROR("Invalid buffer length\n");
		return RMMD_ERR_INVAL;
	}

	if ((challenge_hash_len != SHA256_DIGEST_SIZE) &&
	    (challenge_hash_len != SHA384_DIGEST_SIZE) &&
	    (challenge_hash_len != SHA512_DIGEST_SIZE)) {
		ERROR("Invalid hash size: %lu\n", challenge_hash_len);
		return RMMD_ERR_INVAL;
	}

	spin_lock(&lock);

	/* Map the buffer that was provided by the RMM. */
	err = mmap_add_dynamic_region_alloc_va(buf_pa, &va, PAGE_SIZE,
					       MT_RW_DATA | MT_REALM);
	if (err != 0) {
		ERROR("mmap_add_dynamic_region_alloc_va failed: %d (%p).\n"
		      , err, (void *)buf_pa);
		spin_unlock(&lock);
		return RMMD_ERR_NOMEM;
	}

	(void)memcpy(temp_buf, (void *)va, challenge_hash_len);

	print_challenge((uint8_t *)temp_buf, challenge_hash_len);

	/* Get the platform token. */
	err = plat_get_cca_attest_token(va,
		buf_len, (uintptr_t)temp_buf, challenge_hash_len);

	if (err != 0) {
		ERROR("Failed to get platform token: %d.\n", err);
		err = RMMD_ERR_UNK;
	}

	/* Unmap RMM memory. */
	(void)mmap_remove_dynamic_region(va, PAGE_SIZE);
	spin_unlock(&lock);

	return err;
}

int rmmd_attest_get_signing_key(uint64_t buf_pa, uint64_t *buf_len,
				uint64_t ecc_curve)
{
	int err;
	uintptr_t va;

	/*
	 * TODO: Currently we don't validate incoming buf_pa. This is a
	 * prototype and we will need to allocate static buffer for EL3-RMM
	 * communication.
	 */

	/* We need a page of buffer to pass data */
	if (*buf_len != PAGE_SIZE) {
		ERROR("Invalid buffer length\n");
		return RMMD_ERR_INVAL;
	}

	if (ecc_curve != ATTEST_KEY_CURVE_ECC_SECP384R1) {
		ERROR("Invalid ECC curve specified\n");
		return RMMD_ERR_INVAL;
	}

	spin_lock(&lock);

	/* Map the buffer that was provided by the RMM. */
	err = mmap_add_dynamic_region_alloc_va(buf_pa, &va, PAGE_SIZE,
					       MT_RW_DATA | MT_REALM);
	if (err != 0) {
		ERROR("mmap_add_dynamic_region_alloc_va failed: %d (%p).\n"
		      , err, (void *)buf_pa);
		spin_unlock(&lock);
		return RMMD_ERR_NOMEM;
	}

	/* Get the Realm attestation key. */
	err = plat_get_cca_realm_attest_key(va, buf_len, (unsigned int)ecc_curve);
	if (err != 0) {
		ERROR("Failed to get attestation key: %d.\n", err);
		err =  RMMD_ERR_UNK;
	}

	/* Unmap RMM memory. */
	(void)mmap_remove_dynamic_region(va, PAGE_SIZE);
	spin_unlock(&lock);

	return err;
}

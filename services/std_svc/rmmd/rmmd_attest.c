/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2024, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include "rmmd_private.h"
#include <services/rmm_el3_token_sign.h>
#include <smccc_helpers.h>

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
 * Helper function to validate that the buffer base and length are
 * within range.
 */
static int validate_buffer_params(uint64_t buf_pa, uint64_t buf_len)
{
	unsigned long shared_buf_page;
	uintptr_t shared_buf_base;

	(void)plat_rmmd_get_el3_rmm_shared_mem(&shared_buf_base);

	shared_buf_page = shared_buf_base & ~PAGE_SIZE_MASK;

	/* Validate the buffer pointer */
	if ((buf_pa & ~PAGE_SIZE_MASK) != shared_buf_page) {
		ERROR("Buffer PA out of range\n");
		return E_RMM_BAD_ADDR;
	}

	/* Validate the size of the shared area */
	if (((buf_pa + buf_len - 1UL) & ~PAGE_SIZE_MASK) != shared_buf_page) {
		ERROR("Invalid buffer length\n");
		return E_RMM_INVAL;
	}

	return 0; /* No error */
}

int rmmd_attest_get_platform_token(uint64_t buf_pa, uint64_t *buf_size,
				   uint64_t c_size,
				   uint64_t *remaining_len)
{
	int err;
	uint8_t temp_buf[SHA512_DIGEST_SIZE];

	err = validate_buffer_params(buf_pa, *buf_size);
	if (err != 0) {
		return err;
	}

	if ((c_size != SHA256_DIGEST_SIZE) &&
	    (c_size != SHA384_DIGEST_SIZE) &&
	    (c_size != SHA512_DIGEST_SIZE)) {
		ERROR("Invalid hash size: %lu\n", c_size);
		return E_RMM_INVAL;
	}

	spin_lock(&lock);

	(void)memcpy(temp_buf, (void *)buf_pa, c_size);

	print_challenge((uint8_t *)temp_buf, c_size);

	/* Get the platform token. */
	err = plat_rmmd_get_cca_attest_token((uintptr_t)buf_pa,
		buf_size, (uintptr_t)temp_buf, c_size, remaining_len);

	switch (err) {
	case 0:
		err = E_RMM_OK;
		break;
	case -EAGAIN:
		err = E_RMM_AGAIN;
		break;
	case -EINVAL:
		err = E_RMM_INVAL;
		break;
	default:
		ERROR("Failed to get platform token: %d.\n", err);
		err = E_RMM_UNK;
	}

	spin_unlock(&lock);

	return err;
}

int rmmd_attest_get_signing_key(uint64_t buf_pa, uint64_t *buf_size,
				uint64_t ecc_curve)
{
	int err;

	err = validate_buffer_params(buf_pa, *buf_size);
	if (err != 0) {
		return err;
	}

	if (ecc_curve != ATTEST_KEY_CURVE_ECC_SECP384R1) {
		ERROR("Invalid ECC curve specified\n");
		return E_RMM_INVAL;
	}

	spin_lock(&lock);

	/* Get the Realm attestation key. */
	err = plat_rmmd_get_cca_realm_attest_key((uintptr_t)buf_pa, buf_size,
						 (unsigned int)ecc_curve);
	if (err != 0) {
		ERROR("Failed to get attestation key: %d.\n", err);
		err = E_RMM_UNK;
	}

	spin_unlock(&lock);

	return err;
}

static int rmmd_el3_token_sign_push_req(uint64_t buf_pa, uint64_t buf_size)
{
	int err;

	err = validate_buffer_params(buf_pa, buf_size);
	if (err != 0) {
		return err;
	}

	if (buf_size < sizeof(struct el3_token_sign_request)) {
		return E_RMM_INVAL;
	}

	spin_lock(&lock);

	/* Call platform port to handle attestation toekn signing request. */
	err = plat_rmmd_el3_token_sign_push_req((struct el3_token_sign_request *)buf_pa);

	spin_unlock(&lock);

	return err;
}

static int rmmd_el3_token_sign_pull_resp(uint64_t buf_pa, uint64_t buf_size)
{
	int err;

	err = validate_buffer_params(buf_pa, buf_size);
	if (err != 0) {
		return err;
	}


	if (buf_size < sizeof(struct el3_token_sign_response)) {
		return E_RMM_INVAL;
	}

	spin_lock(&lock);

	/* Pull attestation signing response from HES. */
	err = plat_rmmd_el3_token_sign_pull_resp(
			(struct el3_token_sign_response *)buf_pa);

	spin_unlock(&lock);

	return err;
}

static int rmmd_attest_get_attest_pub_key(uint64_t buf_pa, uint64_t *buf_size,
				   uint64_t ecc_curve)
{
	int err;

	err = validate_buffer_params(buf_pa, *buf_size);
	if (err != 0) {
		return err;
	}

	if (ecc_curve != ATTEST_KEY_CURVE_ECC_SECP384R1) {
		ERROR("Invalid ECC curve specified\n");
		return E_RMM_INVAL;
	}

	spin_lock(&lock);

	/* Get the Realm attestation public key from platform port. */
	err = plat_rmmd_el3_token_sign_get_rak_pub(
		(uintptr_t)buf_pa, buf_size, (unsigned int)ecc_curve);

	spin_unlock(&lock);
	if (err != 0) {
		ERROR("Failed to get attestation public key from HES: %d.\n",
		      err);
		err = E_RMM_UNK;
	}


	return err;
}

uint64_t rmmd_el3_token_sign(void *handle, uint64_t opcode, uint64_t x2,
				    uint64_t x3, uint64_t x4)
{
	int ret;

	switch (opcode) {
	case RMM_EL3_TOKEN_SIGN_PUSH_REQ_OP:
		ret = rmmd_el3_token_sign_push_req(x2, x3);
		SMC_RET1(handle, ret);
	case RMM_EL3_TOKEN_SIGN_PULL_RESP_OP:
		ret = rmmd_el3_token_sign_pull_resp(x2, x3);
		SMC_RET1(handle, ret);
	case RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP:
		ret = rmmd_attest_get_attest_pub_key(x2, &x3, x4);
		SMC_RET2(handle, ret, x3);
	default:
		SMC_RET1(handle, SMC_UNK);
	}
}

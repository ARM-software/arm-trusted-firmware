/*
 * Copyright (c) 2024-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <delegated_attestation.h>
#include <initial_attestation.h>
#include <lib/gpt_rme/gpc_fault.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/common_def.h>
#include <psa/error.h>
#include <services/firme/firme_attestation.h>

#if FIRME_SUPPORT

#define PLAT_ARM_MAX_TOKEN_SIZE SZ_4K

static uint8_t platform_token_buf[PLAT_ARM_MAX_TOKEN_SIZE];
static uint64_t platform_token_offset;
static size_t platform_token_len;
static bool rse_request_pending;

typedef struct {
	spinlock_t lock;
	bool active;
	bool in_call;
	firme_instance_e owner;
} rdv3_attest_request_t;

static rdv3_attest_request_t rdv3_attest_request;

int32_t firme_attest_plat_begin(firme_instance_e instance, bool is_new_request)
{
	int32_t rc = FIRME_SUCCESS;

	spin_lock(&rdv3_attest_request.lock);

	if (rdv3_attest_request.in_call) {
		rc = FIRME_BUSY;
	} else if (rdv3_attest_request.active) {
		if (is_new_request) {
			rc = FIRME_BUSY;
		} else if (rdv3_attest_request.owner != instance) {
			rc = FIRME_DENIED;
		} else {
			rdv3_attest_request.in_call = true;
		}
	} else if (!is_new_request) {
		rc = FIRME_INVALID_PARAMETERS;
	} else {
		rdv3_attest_request.active = true;
		rdv3_attest_request.in_call = true;
		rdv3_attest_request.owner = instance;
	}

	spin_unlock(&rdv3_attest_request.lock);

	return rc;
}

void firme_attest_plat_finish(firme_instance_e instance, int32_t status)
{
	(void)instance;

	spin_lock(&rdv3_attest_request.lock);
	rdv3_attest_request.in_call = false;
	if ((status != FIRME_OP_CONFLICT) && (status != FIRME_INCOMPLETE)) {
		rdv3_attest_request.active = false;
	}
	spin_unlock(&rdv3_attest_request.lock);
}

/*
 * Note: This implementation caters for retrieval of the platform token
 * in hunks to facilitate EL3-EL1 interface testing. For most platforms,
 * since the shared buffer size is known, the implementation can be more
 * optimized.
 *
 * Caller allocates a single page to ensure the buffer is contiguous
 * in the PA range.
 *
 * RSE supports sending the entire token in a single operation only; it does not
 * support chunked transfers. If the token size exceeds a single page (4 KB),
 * a sufficiently large local buffer must be allocated, and RSE writes the
 * token into that buffer.
 *
 * FIRME supports chunked transfers, so the caller can retrieve the token
 * from the local buffer in chunks.
 */
int32_t firme_attest_plat_get_token(firme_instance_e instance,
				    uintptr_t shared_buf, size_t shared_buf_sz,
				    uintptr_t challenge, size_t challenge_size,
				    size_t *write_size, size_t *remaining_size)
{
	psa_status_t ret;
	size_t hunk_len;

	if ((write_size == NULL) || (remaining_size == NULL)) {
		return FIRME_INVALID_PARAMETERS;
	}

	/*
	 * If a challenge is supplied, retrieve a fresh token. Otherwise,
	 * continue returning chunks from the current staged token.
	 */
	if (challenge_size != 0U) {
		platform_token_len = 0U;
		platform_token_offset = 0U;
		rse_request_pending = false;
	}

	if ((challenge_size != 0U) || rse_request_pending) {
		if (instance == FIRME_REALM) {
			ret = rse_delegated_attest_get_token(
				(const uint8_t *)challenge, challenge_size,
				platform_token_buf, sizeof(platform_token_buf),
				&platform_token_len);
		} else {
			ret = rse_initial_attest_get_token(
				(const uint8_t *)challenge, challenge_size,
				platform_token_buf, sizeof(platform_token_buf),
				&platform_token_len);
		}

		if (ret != PSA_SUCCESS) {
			switch (ret) {
			case PSA_ERROR_INVALID_ARGUMENT:
				ERROR("Invalid parameters to RSE initial attestation service (%d)\n",
				      ret);
				return FIRME_INVALID_PARAMETERS;
			case PSA_OPERATION_INCOMPLETE:
				rse_request_pending = true;
				*write_size = 0U;
				*remaining_size = 0U;
				return FIRME_OP_CONFLICT;
			case PSA_ERROR_CONNECTION_BUSY:
				ERROR("Connection busy\n");
				return FIRME_BUSY;
			default:
				return FIRME_NOT_SUPPORTED;
			}
		}

		rse_request_pending = false;
	} else if (platform_token_len == 0U) {
		return FIRME_INVALID_PARAMETERS;
	}

	/*
	 * If the remaining size of the internal buffer is larger than the shared buffer, truncate
	 * how much we write to the output buffer
	 */

	hunk_len = platform_token_len - platform_token_offset;
	if (hunk_len >= shared_buf_sz) {
		hunk_len = shared_buf_sz;
	}

	/* Update remaining bytes according to hunk size */
#if ENABLE_FEAT_RME
	if (rme_gpf_safe_write((void *)shared_buf,
			       &platform_token_buf[platform_token_offset],
			       hunk_len) != hunk_len) {
		platform_token_len = 0U;
		platform_token_offset = 0U;
		*write_size = 0U;
		*remaining_size = 0U;
		return FIRME_INVALID_PARAMETERS;
	}
#else
	(void)memcpy((void *)shared_buf,
		     &platform_token_buf[platform_token_offset], hunk_len);
#endif

	platform_token_offset += hunk_len;

	*write_size = hunk_len;
	*remaining_size = platform_token_len - platform_token_offset;

	/* If all the chunks are transferred then reset global state */
	if (*remaining_size == 0U) {
		platform_token_len = 0U;
		platform_token_offset = 0U;
	}

	return (*remaining_size == 0U) ? FIRME_SUCCESS : FIRME_INCOMPLETE;
}

uintptr_t firme_plat_shared_buf_addr(firme_instance_e instance,
				     size_t *shared_buf_pg_cnt)
{
	uintptr_t shared_buf = 0U;

#if ENABLE_RMM
	if (instance == FIRME_REALM) {
		*shared_buf_pg_cnt = (size_t)(RMM_SHARED_SIZE / PAGE_SIZE);
		shared_buf = RMM_SHARED_BASE;
	}
#else
	*shared_buf_pg_cnt = 0U;
#endif

	return shared_buf;
}

#endif /* FIRME_SUPPORT */

int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len, uintptr_t hash,
				   size_t hash_size, size_t *remaining_size)
{
	size_t token_len = 0U;
	psa_status_t ret;

	assert(*len == SZ_4K);

	ret = rse_delegated_attest_get_token((const uint8_t *)hash, hash_size,
					     (uint8_t *)buf, *len, &token_len);
	if (ret == PSA_OPERATION_INCOMPLETE) {
		return -EAGAIN;
	}

	if (ret != PSA_SUCCESS) {
		ERROR("Unable to fetch CCA attestation token\n");
		return -1;
	}

	*len = token_len;
	assert(*len <= SZ_4K);

	*remaining_size = 0;

	return 0;
}

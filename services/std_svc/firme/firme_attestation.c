/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "firme_private.h"

#include <common/debug.h>
#include <common/smc_validation_framework.h>
#include <lib/gpt_rme/gpc_fault.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/firme/firme_attestation.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

#define ATTEST_INSTANCE_SUPPORT (BIT(FIRME_NONSECURE) | BIT(FIRME_REALM))

CASSERT(FIRME_ATTEST_MAX_PAT_PG_CNT <=
		FIRME_ATTEST_FEAT_REG1_MAX_PAT_PG_CNT_MASK,
	assert_firme_attest_max_pat_pg_cnt_too_large);

static uint64_t registers[FIRME_ATTESTATION_FEATURE_REG_COUNT] = {
	FIRME_ATTEST_FEAT_REG0_PAT_GET_BIT,
	(((uint64_t)FIRME_ATTEST_MAX_PAT_PG_CNT &
	  FIRME_ATTEST_FEAT_REG1_MAX_PAT_PG_CNT_MASK)
	 << FIRME_ATTEST_FEAT_REG1_MAX_PAT_PG_CNT_SHIFT)
};

static int copy_challenge(void *dst, const void *src, size_t size)
{
#if ENABLE_FEAT_RME
	if (rme_gpf_safe_read(dst, src, size) != size) {
		return FIRME_INVALID_PARAMETERS;
	}
#else
	memcpy(dst, src, size);
#endif

	return 0;
}

static int32_t
firme_attestation_service_version(firme_instance_e instance __unused)
{
	return FIRME_VERSION(FIRME_ATTESTATION_VERSION_MAJOR,
			     FIRME_ATTESTATION_VERSION_MINOR);
}

static bool firme_attestation_service_is_supported(firme_instance_e instance)
{
	return (ATTEST_INSTANCE_SUPPORT & BIT(instance)) != 0U;
}

static int32_t
firme_attestation_service_get_feature_reg(firme_instance_e instance __unused,
					  uint8_t reg_index, uint64_t *reg)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (reg_index >= FIRME_ATTESTATION_FEATURE_REG_COUNT) {
		return FIRME_NOT_SUPPORTED;
	}

	*reg = registers[reg_index];
	return FIRME_SUCCESS;
}

bool firme_is_valid_shared_buffer(firme_instance_e instance,
				  uintptr_t shared_buf_addr,
				  size_t buf_page_count)
{
	uintptr_t rmm_shared_buf_addr;
	size_t min_sh_buf_sz;
	size_t max_sh_buf_pg_cnt;
	size_t plat_buf_page_cnt;
	size_t shared_buf_size;

	if (buf_page_count == 0U) {
		return false;
	}

	if (instance == FIRME_REALM) {
		rmm_shared_buf_addr = firme_plat_shared_buf_addr(
			instance, &plat_buf_page_cnt);
		if ((rmm_shared_buf_addr & ~PAGE_SIZE_MASK) !=
			    shared_buf_addr ||
		    buf_page_count > plat_buf_page_cnt) {
			return false;
		}
	}

	switch (PAGE_SIZE) {
	case PAGE_SIZE_4KB:
		min_sh_buf_sz = SZ_4K;
		break;
	case PAGE_SIZE_16KB:
		min_sh_buf_sz = SZ_16K;
		break;
	case PAGE_SIZE_64KB:
		min_sh_buf_sz = SZ_64K;
		break;
	default:
		return false;
	}

	max_sh_buf_pg_cnt = FIRME_BASE_MAX_SH_BUF_PG_CNT + U(1);
	if (buf_page_count > max_sh_buf_pg_cnt) {
		return false;
	}

	shared_buf_size = buf_page_count * PAGE_SIZE;
	if ((instance == FIRME_NONSECURE) &&
	    (smc_validate_mem_range(shared_buf_addr, shared_buf_size) !=
	     SMC_OK)) {
		return false;
	}

	return shared_buf_size >= min_sh_buf_sz;
}

static int32_t firme_attest_pat_get(firme_instance_e instance,
				    uintptr_t shared_buf_addr, size_t offset,
				    size_t page_count, size_t challenge_size,
				    size_t *write_size, size_t *remaining_size)
{
	uint64_t challenge[SHA512_DIGEST_SIZE / sizeof(uint64_t)];
	size_t buf_page_count;
	size_t token_size, shared_buf_size;
	uintptr_t token_addr, challenge_addr = 0U;
	int32_t rc, mmap_rc;
	bool is_new_request = challenge_size > 0U;

	if (page_count > FIRME_BASE_MAX_SH_BUF_PG_CNT) {
		ERROR("Invalid max buffer size: %lu\n", page_count);
		return FIRME_INVALID_PARAMETERS;
	}

	/* The ABI encodes N pages as N - 1. */
	buf_page_count = page_count + U(1);
	shared_buf_size = buf_page_count * PAGE_SIZE;

	if (!firme_is_valid_shared_buffer(instance, shared_buf_addr,
					  buf_page_count) ||
	    offset >= shared_buf_size) {
		ERROR("Invalid max buffer size: %lu\n", page_count);
		return FIRME_INVALID_PARAMETERS;
	}

	token_addr = shared_buf_addr + offset;
	token_size = shared_buf_size - offset;

	if ((challenge_size != 0U) && (challenge_size != SHA256_DIGEST_SIZE) &&
	    (challenge_size != SHA384_DIGEST_SIZE) &&
	    (challenge_size != SHA512_DIGEST_SIZE)) {
		ERROR("Invalid challenge size: %lu\n", challenge_size);
		return FIRME_INVALID_PARAMETERS;
	}

	if (instance == FIRME_NONSECURE) {
		mmap_rc = mmap_add_dynamic_region(shared_buf_addr,
						  shared_buf_addr,
						  shared_buf_size,
						  MT_MEMORY | MT_RW | MT_NS);
		if (mmap_rc != 0) {
			ERROR("mmap_add_dynamic_region failed rc=%d\n",
			      mmap_rc);
			return FIRME_INVALID_PARAMETERS;
		}
	}

	/*
	 * Copy the challenge into a private buffer before passing its
	 * contents to a platform for consumption.
	 *
	 */
	if (is_new_request) {
		rc = copy_challenge(challenge, (const void *)shared_buf_addr,
				    challenge_size);
		if (rc != 0) {
			goto unmap;
		}

		challenge_addr = (uintptr_t)challenge;
	}

	rc = firme_attest_plat_get_token(instance, token_addr, token_size,
					 challenge_addr, challenge_size,
					 write_size, remaining_size);

unmap:
	if (instance == FIRME_NONSECURE) {
		mmap_rc = mmap_remove_dynamic_region(shared_buf_addr,
						     shared_buf_size);
		if (mmap_rc != 0U) {
			if (mmap_rc != -ENOMEM) {
				ERROR("%s(): mmap_remove_dynamic_region() failed unexpectedly rc=%d\n",
				      __func__, mmap_rc);
				return FIRME_NO_MEMORY;
			}

			return FIRME_INVALID_PARAMETERS;
		}
	}

	return rc;
}

static u_register_t firme_attestation_service_handler(
	firme_instance_e instance, uint32_t smc_fid, uint64_t x1, uint64_t x2,
	uint64_t x3, uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	size_t write_size = 0U, remaining_size = 0U;
	int32_t rc;

	(void)cookie;
	(void)flags;

	switch (smc_fid) {
	case FIRME_ATTEST_PAT_GET_FID:
		rc = firme_attest_pat_get(instance, x1, x2, x3, x4, &write_size,
					  &remaining_size);
		switch (rc) {
		case FIRME_SUCCESS:
			SMC_RET2(handle, rc, write_size);
		case FIRME_INCOMPLETE:
			SMC_RET3(handle, rc, write_size, remaining_size);
		default:
			SMC_RET1(handle, rc);
		}
	default:
		VERBOSE("FIRME Attestation Service FID 0x%X not implemented\n",
			smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}
}

const struct firme_service firme_attestation_service = {
	.id = FIRME_ATTESTATION_ID,
	.init = NULL,
	.version = firme_attestation_service_version,
	.is_supported = firme_attestation_service_is_supported,
	.get_feature_reg = firme_attestation_service_get_feature_reg,
	.call = firme_attestation_service_handler,
};

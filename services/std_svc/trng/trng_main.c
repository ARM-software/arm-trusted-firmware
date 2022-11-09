/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_features.h>
#include <lib/smccc.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

#include <plat/common/plat_trng.h>

#include "trng_entropy_pool.h"

static const uuid_t uuid_null;

/* handle the RND call in SMC 32 bit mode */
static uintptr_t trng_rnd32(uint32_t nbits, void *handle)
{
	uint32_t mask = ~0U;
	uint64_t ent[2] = {0};

	if (nbits == 0U || nbits > TRNG_RND32_ENTROPY_MAXBITS) {
		SMC_RET1(handle, TRNG_E_INVALID_PARAMS);
	}

	if (!trng_pack_entropy(nbits, &ent[0])) {
		SMC_RET1(handle, TRNG_E_NO_ENTROPY);
	}

	if ((nbits % 32U) != 0U) {
		mask >>= 32U - (nbits % 32U);
	}

	switch ((nbits - 1U) / 32U) {
	case 0:
		SMC_RET4(handle, TRNG_E_SUCCESS, 0, 0, ent[0] & mask);
		break; /* unreachable */
	case 1:
		SMC_RET4(handle, TRNG_E_SUCCESS, 0, (ent[0] >> 32) & mask,
			 ent[0] & 0xFFFFFFFF);
		break; /* unreachable */
	case 2:
		SMC_RET4(handle, TRNG_E_SUCCESS, ent[1] & mask,
			 (ent[0] >> 32) & 0xFFFFFFFF, ent[0] & 0xFFFFFFFF);
		break; /* unreachable */
	default:
		SMC_RET1(handle, TRNG_E_INVALID_PARAMS);
		break; /* unreachable */
	}
}

/* handle the RND call in SMC 64 bit mode */
static uintptr_t trng_rnd64(uint32_t nbits, void *handle)
{
	uint64_t mask = ~0ULL;
	uint64_t ent[3] = {0};

	if (nbits == 0U || nbits > TRNG_RND64_ENTROPY_MAXBITS) {
		SMC_RET1(handle, TRNG_E_INVALID_PARAMS);
	}

	if (!trng_pack_entropy(nbits, &ent[0])) {
		SMC_RET1(handle, TRNG_E_NO_ENTROPY);
	}

	/* Mask off higher bits if only part of register requested */
	if ((nbits % 64U) != 0U) {
		mask >>= 64U - (nbits % 64U);
	}

	switch ((nbits - 1U) / 64U) {
	case 0:
		SMC_RET4(handle, TRNG_E_SUCCESS, 0, 0, ent[0] & mask);
		break; /* unreachable */
	case 1:
		SMC_RET4(handle, TRNG_E_SUCCESS, 0, ent[1] & mask, ent[0]);
		break; /* unreachable */
	case 2:
		SMC_RET4(handle, TRNG_E_SUCCESS, ent[2] & mask, ent[1], ent[0]);
		break; /* unreachable */
	default:
		SMC_RET1(handle, TRNG_E_INVALID_PARAMS);
		break; /* unreachable */
	}
}

void trng_setup(void)
{
	trng_entropy_pool_setup();
	plat_entropy_setup();
}

/* Predicate indicating that a function id is part of TRNG */
bool is_trng_fid(uint32_t smc_fid)
{
	return ((smc_fid == ARM_TRNG_VERSION) ||
		(smc_fid == ARM_TRNG_FEATURES) ||
		(smc_fid == ARM_TRNG_GET_UUID) ||
		(smc_fid == ARM_TRNG_RND32) ||
		(smc_fid == ARM_TRNG_RND64));
}

uintptr_t trng_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			   u_register_t x3, u_register_t x4, void *cookie,
			   void *handle, u_register_t flags)
{
	if (!memcmp(&plat_trng_uuid, &uuid_null, sizeof(uuid_t))) {
		SMC_RET1(handle, TRNG_E_NOT_IMPLEMENTED);
	}

	switch (smc_fid) {
	case ARM_TRNG_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			TRNG_VERSION_MAJOR, TRNG_VERSION_MINOR));
		break; /* unreachable */

	case ARM_TRNG_FEATURES:
		if (is_trng_fid((uint32_t)x1)) {
			SMC_RET1(handle, TRNG_E_SUCCESS);
		} else {
			SMC_RET1(handle, TRNG_E_NOT_SUPPORTED);
		}
		break; /* unreachable */

	case ARM_TRNG_GET_UUID:
		SMC_UUID_RET(handle, plat_trng_uuid);
		break; /* unreachable */

	case ARM_TRNG_RND32:
		return trng_rnd32((uint32_t)x1, handle);

	case ARM_TRNG_RND64:
		return trng_rnd64((uint32_t)x1, handle);

	default:
		WARN("Unimplemented TRNG Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, TRNG_E_NOT_IMPLEMENTED);
		break; /* unreachable */
	}
}

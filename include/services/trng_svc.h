/*
 * Copyright (c) 2021-2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRNG_SVC_H
#define TRNG_SVC_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/smccc.h>

/* SMC function IDs for TRNG queries */
#define ARM_TRNG_VERSION	U(0x84000050)
#define ARM_TRNG_FEATURES	U(0x84000051)
#define ARM_TRNG_GET_UUID	U(0x84000052)
#define ARM_TRNG_RND32		U(0x84000053)
#define ARM_TRNG_RND64		U(0xC4000053)

/* TRNG version numbers */
#define TRNG_VERSION_MAJOR	(0x1)
#define TRNG_VERSION_MINOR	(0x0)

/* TRNG Error Numbers */
#define TRNG_E_SUCCESS		(0)
#define TRNG_E_NOT_SUPPORTED	(-1)
#define TRNG_E_INVALID_PARAMS	(-2)
#define TRNG_E_NO_ENTROPY	(-3)
#define TRNG_E_NOT_IMPLEMENTED	(-4)

/* TRNG Entropy Bit Numbers */
#define TRNG_RND32_ENTROPY_MAXBITS	(96U)
#define TRNG_RND64_ENTROPY_MAXBITS	(192U)

/* Public API to perform the initial TRNG entropy setup */
void trng_setup(void);

/* Public API to verify function id is part of TRNG */
bool is_trng_fid(uint32_t smc_fid);

/* Handler to be called to handle TRNG smc calls */
uintptr_t trng_smc_handler(
	uint32_t smc_fid,
	u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *cookie,
	void *handle,
	u_register_t flags
);

#endif /* TRNG_SVC_H */

/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ETHOSN_H
#define ETHOSN_H

#include <lib/smccc.h>

/* Function numbers */
#define ETHOSN_FNUM_VERSION		U(0x50)
#define ETHOSN_FNUM_IS_SEC		U(0x51)
#define ETHOSN_FNUM_HARD_RESET		U(0x52)
#define ETHOSN_FNUM_SOFT_RESET		U(0x53)
/* 0x54-0x5F reserved for future use */

/* SMC64 function IDs */
#define ETHOSN_FID_64(func_num)		U(0xC2000000 | func_num)
#define ETHOSN_FID_VERSION_64		ETHOSN_FID_64(ETHOSN_FNUM_VERSION)
#define ETHOSN_FID_IS_SEC_64		ETHOSN_FID_64(ETHOSN_FNUM_IS_SEC)
#define ETHOSN_FID_HARD_RESET_64	ETHOSN_FID_64(ETHOSN_FNUM_HARD_RESET)
#define ETHOSN_FID_SOFT_RESET_64	ETHOSN_FID_64(ETHOSN_FNUM_SOFT_RESET)

/* SMC32 function IDs */
#define ETHOSN_FID_32(func_num)		U(0x82000000 | func_num)
#define ETHOSN_FID_VERSION_32		ETHOSN_FID_32(ETHOSN_FNUM_VERSION)
#define ETHOSN_FID_IS_SEC_32		ETHOSN_FID_32(ETHOSN_FNUM_IS_SEC)
#define ETHOSN_FID_HARD_RESET_32	ETHOSN_FID_32(ETHOSN_FNUM_HARD_RESET)
#define ETHOSN_FID_SOFT_RESET_32	ETHOSN_FID_32(ETHOSN_FNUM_SOFT_RESET)

#define ETHOSN_NUM_SMC_CALLS	8

/* Macro to identify function calls */
#define ETHOSN_FID_MASK		U(0xFFF0)
#define ETHOSN_FID_VALUE	U(0x50)
#define is_ethosn_fid(_fid) (((_fid) & ETHOSN_FID_MASK) == ETHOSN_FID_VALUE)

/* Service version  */
#define ETHOSN_VERSION_MAJOR U(2)
#define ETHOSN_VERSION_MINOR U(0)

/* Return codes for function calls */
#define ETHOSN_SUCCESS			 0
#define ETHOSN_NOT_SUPPORTED		-1
/* -2 Reserved for NOT_REQUIRED */
/* -3 Reserved for INVALID_PARAMETER */
#define ETHOSN_FAILURE			-4
#define ETHOSN_UNKNOWN_CORE_ADDRESS	-5
#define ETHOSN_UNKNOWN_ALLOCATOR_IDX	-6

uintptr_t ethosn_smc_handler(uint32_t smc_fid,
			     u_register_t core_addr,
			     u_register_t asset_alloc_idx,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags);

#endif  /* ETHOSN_H */

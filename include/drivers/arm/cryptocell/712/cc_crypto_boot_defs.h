/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_CRYPTO_BOOT_DEFS_H
#define _CC_CRYPTO_BOOT_DEFS_H

/*! @file
@brief This file contains SBROM definitions
*/

/*! Version counters value. */
typedef enum {

	CC_SW_VERSION_COUNTER1 = 1,	/*!< Counter 1 - trusted version. */
	CC_SW_VERSION_COUNTER2,		/*!< Counter 2 - non trusted version. */

	CC_SW_VERSION_MAX      = 0x7FFFFFFF

} CCSbSwVersionId_t;

/* HASH boot key definition */
typedef enum {
	CC_SB_HASH_BOOT_KEY_0_128B 	= 0,		/*!< 128-bit truncated SHA256 digest of public key 0. */
	CC_SB_HASH_BOOT_KEY_1_128B	= 1,		/*!< 128-bit truncated SHA256 digest of public key 1. */
	CC_SB_HASH_BOOT_KEY_256B	= 2,		/*!< 256-bit SHA256 digest of public key. */
	CC_SB_HASH_BOOT_NOT_USED	= 0xFF,
	CC_SB_HASH_MAX_NUM 		= 0x7FFFFFFF,	/*!\internal use external 128-bit truncated SHA256 digest */
} CCSbPubKeyIndexType_t;


#endif

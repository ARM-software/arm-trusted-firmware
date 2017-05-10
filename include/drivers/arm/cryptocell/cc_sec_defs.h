/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_SEC_DEFS_H
#define _CC_SEC_DEFS_H

/*!
@file
@brief This file contains general hash definitions and types.
*/


#ifdef __cplusplus
extern "C"
{
#endif

/*! The hashblock size in words. */
#define HASH_BLOCK_SIZE_IN_WORDS             16
/*! The hash - SHA2 results in words. */
#define HASH_RESULT_SIZE_IN_WORDS            8
#define HASH_RESULT_SIZE_IN_BYTES            32

/*! Definition for hash result array. */
typedef uint32_t CCHashResult_t[HASH_RESULT_SIZE_IN_WORDS];

#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_SEC_DEFS_H
#define _CC_SEC_DEFS_H

/*!
@file
@brief This file contains general definitions and types.
*/


#ifdef __cplusplus
extern "C"
{
#endif

#include "cc_pal_types.h"

/*! Hashblock size in words. */
#define HASH_BLOCK_SIZE_IN_WORDS             16
/*! Hash - SHA2 results in words. */
#define HASH_RESULT_SIZE_IN_WORDS            8
/*! Hash - SHA2 results in bytes. */
#define HASH_RESULT_SIZE_IN_BYTES            32

/*! Definition for hash result array. */
typedef uint32_t CCHashResult_t[HASH_RESULT_SIZE_IN_WORDS];

/*! Definition for converting pointer to Host address. */
#define CONVERT_TO_ADDR(ptr) 	(unsigned long)ptr

/*! Definition for converting pointer to SRAM address. */
#define CONVERT_TO_SRAM_ADDR(ptr)    (0xFFFFFFFF & ptr)

/*! The data size of the signed SW image, in bytes. */
/*!\internal ContentCertImageRecord_t includes:  HS(8W) + 64-b dstAddr(2W) + imgSize(1W) + isCodeEncUsed(1W) */
#define SW_REC_SIGNED_DATA_SIZE_IN_BYTES            48

/*! The data size of the unsigned SW image, in bytes. */
/*!\internal CCSbSwImgAddData_t includes: 64-b srcAddr(2W)*/
#define SW_REC_NONE_SIGNED_DATA_SIZE_IN_BYTES       8

/*! The additional data size - storage address and length of the unsigned SW image, in words. */
#define SW_REC_NONE_SIGNED_DATA_SIZE_IN_WORDS       SW_REC_NONE_SIGNED_DATA_SIZE_IN_BYTES/CC_32BIT_WORD_SIZE

/*! The additional data section size, in bytes. */
#define CC_SB_MAX_SIZE_ADDITIONAL_DATA_BYTES    128

/*! Indication of whether or not to load the SW image to memory. */
#define CC_SW_COMP_NO_MEM_LOAD_INDICATION       0xFFFFFFFFFFFFFFFFUL

/*! Indication of product version, stored in certificate version field. */
#define CC_SB_CERT_VERSION_PROJ_PRD             0x713

#ifdef __cplusplus
}
#endif

#endif

/**
@}
 */




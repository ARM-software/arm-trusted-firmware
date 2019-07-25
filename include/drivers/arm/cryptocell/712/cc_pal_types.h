/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CC_PAL_TYPES_H
#define CC_PAL_TYPES_H

/*!
@file
@brief This file contains platform-dependent definitions and types.
*/

#include "cc_pal_types_plat.h"

typedef enum {
	CC_FALSE = 0,
	CC_TRUE = 1
} CCBool;

#define CC_SUCCESS		0UL
#define CC_FAIL			1UL

#define CC_1K_SIZE_IN_BYTES	1024
#define CC_BITS_IN_BYTE		8
#define CC_BITS_IN_32BIT_WORD	32
#define CC_32BIT_WORD_SIZE	(sizeof(uint32_t))

#define CC_OK			CC_SUCCESS

#define CC_UNUSED_PARAM(prm)	((void)prm)

#define CC_MAX_UINT32_VAL	(0xFFFFFFFF)

#define CALC_FULL_BYTES(numBits)		(((numBits) + (CC_BITS_IN_BYTE - 1))/CC_BITS_IN_BYTE)
#define CALC_FULL_32BIT_WORDS(numBits)		(((numBits) + (CC_BITS_IN_32BIT_WORD - 1))/CC_BITS_IN_32BIT_WRD)
#define CALC_32BIT_WORDS_FROM_BYTES(sizeBytes)	(((sizeBytes) + CC_32BIT_WORD_SIZE - 1)/CC_32BIT_WORD_SIZE)

#endif

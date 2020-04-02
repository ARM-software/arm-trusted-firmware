/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CC_PAL_TYPES_H
#define CC_PAL_TYPES_H

/*!
@file
@brief This file contains platform-dependent definitions and types of the PAL layer.

@defgroup cc_pal_types CryptoCell platform-dependent PAL layer definitions and types
@{
@ingroup cc_pal

 @{
 @ingroup cc_pal
 @}
*/

#include "cc_pal_types_plat.h"

/*! Definition of Boolean type.*/
typedef enum {
    /*! Boolean false.*/
    CC_FALSE = 0,
    /*! Boolean true.*/
    CC_TRUE =  1
} CCBool_t;

/*! Success. */
#define CC_SUCCESS  0UL
/*! Failure. */
#define CC_FAIL     1UL

/*! Success (OK). */
#define CC_OK       0

/*! This macro handles unused parameters in the code, to avoid compilation warnings.  */
#define CC_UNUSED_PARAM(prm)  ((void)prm)

/*! The maximal uint32 value.*/
#define CC_MAX_UINT32_VAL   (0xFFFFFFFF)


/* Minimal and Maximal macros */
#ifdef  min
/*! Definition for minimal calculation. */
#define CC_MIN(a,b) min( a , b )
#else
/*! Definition for minimal calculation. */
#define CC_MIN( a , b ) ( ( (a) < (b) ) ? (a) : (b) )
#endif

#ifdef max
/*! Definition for maximal calculation. */
#define CC_MAX(a,b) max( a , b )
#else
/*! Definition for maximal calculation.. */
#define CC_MAX( a , b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif

/*! This macro calculates the number of full Bytes from bits, where seven bits are one Byte. */
#define CALC_FULL_BYTES(numBits)        ((numBits)/CC_BITS_IN_BYTE + (((numBits) & (CC_BITS_IN_BYTE-1)) > 0))
/*! This macro calculates the number of full 32-bit words from bits where 31 bits are one word. */
#define CALC_FULL_32BIT_WORDS(numBits)      ((numBits)/CC_BITS_IN_32BIT_WORD +  (((numBits) & (CC_BITS_IN_32BIT_WORD-1)) > 0))
/*! This macro calculates the number of full 32-bit words from Bytes where three Bytes are one word. */
#define CALC_32BIT_WORDS_FROM_BYTES(sizeBytes)  ((sizeBytes)/CC_32BIT_WORD_SIZE + (((sizeBytes) & (CC_32BIT_WORD_SIZE-1)) > 0))
/*! This macro calculates the number of full 32-bit words from 64-bits dwords. */
#define CALC_32BIT_WORDS_FROM_64BIT_DWORD(sizeWords)  (sizeWords * CC_32BIT_WORD_IN_64BIT_DWORD)
/*! This macro rounds up bits to 32-bit words. */
#define ROUNDUP_BITS_TO_32BIT_WORD(numBits) 	(CALC_FULL_32BIT_WORDS(numBits) * CC_BITS_IN_32BIT_WORD)
/*! This macro rounds up bits to Bytes. */
#define ROUNDUP_BITS_TO_BYTES(numBits) 		(CALC_FULL_BYTES(numBits) * CC_BITS_IN_BYTE)
/*! This macro rounds up bytes to 32-bit words. */
#define ROUNDUP_BYTES_TO_32BIT_WORD(sizeBytes) 	(CALC_32BIT_WORDS_FROM_BYTES(sizeBytes) * CC_32BIT_WORD_SIZE)
/*! This macro calculates the number Bytes from words. */
#define CALC_WORDS_TO_BYTES(numwords) 		((numwords)*CC_32BIT_WORD_SIZE)
/*! Definition of 1 KB in Bytes. */
#define CC_1K_SIZE_IN_BYTES 1024
/*! Definition of number of bits in a Byte. */
#define CC_BITS_IN_BYTE     8
/*! Definition of number of bits in a 32-bits word. */
#define CC_BITS_IN_32BIT_WORD   32
/*! Definition of number of Bytes in a 32-bits word. */
#define CC_32BIT_WORD_SIZE  4
/*! Definition of number of 32-bits words in a 64-bits dword. */
#define CC_32BIT_WORD_IN_64BIT_DWORD 2


#endif

/**
@}
 */




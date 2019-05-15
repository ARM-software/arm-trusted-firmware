/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_ADDRESS_DEFS_H
#define _CC_ADDRESS_DEFS_H

/*!
@file
@brief This file contains general definitions.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cc_pal_types.h"

/************************ Defines ******************************/

/**
 * Address types within CC
 */
/*! Definition of DMA address type, can be 32 bits or 64 bits according to CryptoCell's HW. */
typedef uint64_t  CCDmaAddr_t;
/*! Definition of CryptoCell address type, can be 32 bits or 64 bits according to platform. */
typedef uint64_t  CCAddr_t;
/*! Definition of CC SRAM address type, can be 32 bits according to CryptoCell's HW. */
typedef uint32_t  CCSramAddr_t;

/*
 * CCSramAddr_t is being cast into pointer type which can be 64 bit.
 */
/*! Definition of MACRO that casts SRAM addresses to pointer types. */
#define CCSramAddr2Ptr(sramAddr) ((uintptr_t)sramAddr)

#ifdef __cplusplus
}
#endif

#endif

/**
 @}
 */



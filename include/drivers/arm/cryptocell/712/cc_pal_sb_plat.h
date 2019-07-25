/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
@file
@brief This file contains the platform-dependent definitions that are used in the SBROM code.
*/

#ifndef _CC_PAL_SB_PLAT_H
#define _CC_PAL_SB_PLAT_H

#include "cc_pal_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*! Definition of DMA address type, can be 32 bits or 64 bits according to CryptoCell's HW. */
typedef uint64_t		CCDmaAddr_t;
/*! Definition of CryptoCell address type, can be 32 bits or 64 bits according to platform. */
typedef uintptr_t		CCAddr_t;


#ifdef __cplusplus
}
#endif

#endif

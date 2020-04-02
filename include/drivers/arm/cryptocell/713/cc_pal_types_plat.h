/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*! @file
@brief This file contains basic type definitions that are platform-dependent.
*/
#ifndef _CC_PAL_TYPES_PLAT_H
#define _CC_PAL_TYPES_PLAT_H
/* Host specific types for standard (ISO-C99) compilant platforms */

#include <stddef.h>
#include <stdint.h>

typedef uint32_t CCStatus;

#define CCError_t	CCStatus
#define CC_INFINITE	0xFFFFFFFF

#define CEXPORT_C
#define CIMPORT_C

#endif /*_CC_PAL_TYPES_PLAT_H*/

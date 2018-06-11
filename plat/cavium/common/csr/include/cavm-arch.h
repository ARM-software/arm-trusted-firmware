/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __CAVM_ARCH_H__
#define __CAVM_ARCH_H__

/**
 * @file
 *
 * Master include file for architecture support. Use cavm.h
 * instead of including this file directly.
 *
 * <hr>$Revision: 49448 $<hr>
 */

#ifndef __BYTE_ORDER
    #if !defined(__ORDER_BIG_ENDIAN__) || !defined(__ORDER_LITTLE_ENDIAN__) || !defined(__BYTE_ORDER__)
        #error Unable to determine Endian mode
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define __BYTE_ORDER __ORDER_BIG_ENDIAN__
    #elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define __BYTE_ORDER __ORDER_LITTLE_ENDIAN__
    #else
        #error Unable to determine Endian mode
    #endif
    #define __BIG_ENDIAN    __ORDER_BIG_ENDIAN__
    #define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#endif

#define WEAK __attribute__((weak))

#include "cavm-swap.h"
#ifndef CAVM_BUILD_HOST
#include "cavm-asm.h"
#endif
#include "cavm-model.h"
#include "cavm-numa.h"
#include "cavm-csr.h"
#include "cavm-csr-plat.h"
#include "cavm-csrs.h"
#ifndef CAVM_BUILD_HOST
#include "cavm-fuse.h"
#endif

#endif

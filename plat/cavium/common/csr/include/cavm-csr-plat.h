/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __CAVM_CSR_PLAT__
#define __CAVM_CSR_PLAT__
/**
 * @file
 *
 * Functions and macros for the taget platform.
 *
 * @defgroup csr CSR support
 * @{
 */

#ifdef __KERNEL__

#include <linux/types.h>
#include <asm/io.h>

#define __cavm_csr_fatal(name, num_args, arg1, arg2, arg3, arg4) BUG_ON(0)

#else

#include <stdint.h>

#define __cavm_csr_fatal(name, num_args, arg1, arg2, arg3, arg4) while(1)

#endif

#endif /* __CAVM_CSR_PLAT__ */

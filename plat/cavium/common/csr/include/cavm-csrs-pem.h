#ifndef __CAVM_CSRS_PEM_H__
#define __CAVM_CSRS_PEM_H__
/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * Cavium PEM.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration pem_bar_e
 *
 * PEM Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_PEM_BAR_E_PEMX_PF_BAR0(a) (0x87e0c0000000ll + 0x1000000ll * (a))
#define CAVM_PEM_BAR_E_PEMX_PF_BAR0_SIZE 0x800000ull
#define CAVM_PEM_BAR_E_PEMX_PF_BAR4(a) (0x87e0c0f00000ll + 0x1000000ll * (a))
#define CAVM_PEM_BAR_E_PEMX_PF_BAR4_SIZE 0x100000ull

#endif /* __CAVM_CSRS_PEM_H__ */

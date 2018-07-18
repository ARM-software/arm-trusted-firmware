#ifndef __CAVM_CSRS_BGX_H__
#define __CAVM_CSRS_BGX_H__
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
 * Cavium BGX.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration bgx_bar_e
 *
 * BGX Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_BGX_BAR_E_BGXX_PF_BAR0(a) (0x87e0e0000000ll + 0x1000000ll * (a))
#define CAVM_BGX_BAR_E_BGXX_PF_BAR0_SIZE 0x400000ull
#define CAVM_BGX_BAR_E_BGXX_PF_BAR4(a) (0x87e0e0400000ll + 0x1000000ll * (a))
#define CAVM_BGX_BAR_E_BGXX_PF_BAR4_SIZE 0x400000ull

#endif /* __CAVM_CSRS_BGX_H__ */

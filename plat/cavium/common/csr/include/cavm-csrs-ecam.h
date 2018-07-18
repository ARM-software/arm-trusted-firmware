#ifndef __CAVM_CSRS_ECAM_H__
#define __CAVM_CSRS_ECAM_H__
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
 * Cavium ECAM.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration ecam_bar_e
 *
 * ECAM Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_ECAM_BAR_E_ECAMX_PF_BAR0(a) (0x87e048000000ll + 0x1000000ll * (a))
#define CAVM_ECAM_BAR_E_ECAMX_PF_BAR0_SIZE 0x100000ull
#define CAVM_ECAM_BAR_E_ECAMX_PF_BAR2(a) (0x848000000000ll + 0x1000000000ll * (a))
#define CAVM_ECAM_BAR_E_ECAMX_PF_BAR2_SIZE 0x1000000000ull

#endif /* __CAVM_CSRS_ECAM_H__ */

#ifndef __CAVM_CSRS_SLI_H__
#define __CAVM_CSRS_SLI_H__
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
 * Cavium SLI.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration sli_bar_e
 *
 * SLI Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN81XX(a) (0x874000000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN81XX_SIZE 0x2000000ull
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN88XX(a) (0x874000000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN88XX_SIZE 0x2000000ull
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN83XX(a) (0x874000000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR0_CN83XX_SIZE 0x800000000ull
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN81XX(a) (0x874010000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN81XX_SIZE 0x100000ull
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN88XX(a) (0x874010000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN88XX_SIZE 0x100000ull
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN83XX(a) (0x874c00000000ll + 0x1000000000ll * (a))
#define CAVM_SLI_BAR_E_SLIX_PF_BAR4_CN83XX_SIZE 0x100000ull

#endif /* __CAVM_CSRS_SLI_H__ */

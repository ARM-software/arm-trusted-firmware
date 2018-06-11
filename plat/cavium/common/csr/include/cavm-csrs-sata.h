#ifndef __CAVM_CSRS_SATA_H__
#define __CAVM_CSRS_SATA_H__
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
 * Cavium SATA.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration sata_bar_e
 *
 * SATA Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_SATA_BAR_E_SATAX_PF_BAR0(a) (0x810000000000ll + 0x1000000000ll * (a))
#define CAVM_SATA_BAR_E_SATAX_PF_BAR0_SIZE 0x200000ull
#define CAVM_SATA_BAR_E_SATAX_PF_BAR4(a) (0x810000200000ll + 0x1000000000ll * (a))
#define CAVM_SATA_BAR_E_SATAX_PF_BAR4_SIZE 0x100000ull

#endif /* __CAVM_CSRS_SATA_H__ */

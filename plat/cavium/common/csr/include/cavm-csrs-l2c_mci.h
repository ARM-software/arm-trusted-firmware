#ifndef __CAVM_CSRS_L2C_MCI_H__
#define __CAVM_CSRS_L2C_MCI_H__
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
 * Cavium L2C_MCI.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration l2c_mci_bar_e
 *
 * L2C_MCI Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_L2C_MCI_BAR_E_L2C_MCIX_PF_BAR0(a) (0x87e05c000000ll + 0x1000000ll * (a))
#define CAVM_L2C_MCI_BAR_E_L2C_MCIX_PF_BAR0_SIZE 0x800000ull
#define CAVM_L2C_MCI_BAR_E_L2C_MCIX_PF_BAR4(a) (0x87e05cf00000ll + 0x1000000ll * (a))
#define CAVM_L2C_MCI_BAR_E_L2C_MCIX_PF_BAR4_SIZE 0x100000ull

#endif /* __CAVM_CSRS_L2C_MCI_H__ */

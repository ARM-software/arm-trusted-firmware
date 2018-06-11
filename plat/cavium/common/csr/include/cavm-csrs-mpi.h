#ifndef __CAVM_CSRS_MPI_H__
#define __CAVM_CSRS_MPI_H__
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
 * Cavium MPI.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration mpi_bar_e
 *
 * MPI Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_MPI_BAR_E_MPI_PF_BAR0 (0x804000000000ll)
#define CAVM_MPI_BAR_E_MPI_PF_BAR0_SIZE 0x800000ull
#define CAVM_MPI_BAR_E_MPI_PF_BAR4 (0x804000f00000ll)
#define CAVM_MPI_BAR_E_MPI_PF_BAR4_SIZE 0x100000ull

#endif /* __CAVM_CSRS_MPI_H__ */

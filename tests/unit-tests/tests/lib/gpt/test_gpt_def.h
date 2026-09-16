/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_GPT_DEF_H
#define TEST_GPT_DEF_H

#define SIZE_4KB		(4096)
#define SIZE_16KB		(16384)
#define SIZE_64KB		(65536)
#define SIZE_1MB		(0x10000U)
#define SIZE_3MB		(0x30000U)
#define SIZE_4MB		(0x40000U)
#define SIZE_32MB		(0x2000000U)
#define SIZE_64MB		(0x4000000U)
#define SIZE_1GB		(0x40000000U)
#define SIZE_2GB		(0x80000000U)
#define SIZE_4GB		(0x100000000UL)
#define SIZE_16GB		(0x400000000UL)
#define SIZE_64GB		(0x1000000000UL)
#define SIZE_512GB		(0x8000000000UL)
#define SIZE_1TB		(0x10000000000UL)
#define SIZE_4TB		(0x40000000000UL)
#define SIZE_16TB		(0x100000000000UL)
#define SIZE_256TB		(0x1000000000000UL)
#define SIZE_4PB		(0x10000000000000UL)

#define PPS_COUNT 		7
#define L0GPTSZ_COUNT 		4
#define PGS_COUNT 		3

#define PAS_1_BASE		(U(0))
#define PAS_1_SIZE 		(SIZE_2GB)
#define PAS_1_GPI		GPT_GPI_ANY
#define PAS_2_BASE		(PAS_1_BASE + PAS_1_SIZE)
#define PAS_2_SIZE		(SIZE_2GB - SIZE_64MB)
#define PAS_2_GPI		GPT_GPI_NS
#define PAS_3_BASE		(PAS_2_BASE + PAS_2_SIZE)
#define PAS_3_SIZE		(SIZE_64MB - SIZE_32MB - SIZE_4MB)
#define PAS_3_GPI		GPT_GPI_SECURE
#define PAS_4_BASE		(PAS_3_BASE + PAS_3_SIZE)
#define PAS_4_SIZE		(SIZE_32MB)
#define PAS_4_GPI		GPT_GPI_REALM
#define PAS_5_BASE		(PAS_4_BASE + PAS_4_SIZE)
#define PAS_5_SIZE		(SIZE_3MB)
#define PAS_5_GPI		GPT_GPI_ROOT
#define PAS_6_BASE		(PAS_5_BASE + PAS_5_SIZE)
#define PAS_6_SIZE		(SIZE_1MB)
#define PAS_6_GPI		GPT_GPI_ROOT

#endif

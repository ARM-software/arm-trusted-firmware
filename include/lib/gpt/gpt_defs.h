/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_DEFS_H
#define GPT_DEFS_H

#include <arch.h>
#include <lib/utils_def.h>

#include "gpt.h"

/* GPI values */
#define GPI_NO_ACCESS			U(0x0)
#define GPI_SECURE			U(0x8)
#define GPI_NS				U(0x9)
#define GPI_ROOT			U(0xa)
#define GPI_REALM			U(0xb)
#define GPI_ANY				U(0xf)
#define GPI_VAL_MASK			ULL(0xf)

/* GPT descriptor bit definitions */
#define GPT_L1_INDEX_MASK		ULL(0xf)
#define GPT_L1_INDEX_SHIFT		ULL(0x0)

#define GPT_TBL_DESC			ULL(0x3)
#define GPT_BLK_DESC			ULL(0x1)

#define GPT_TBL_DESC_ADDR_SHIFT		ULL(12)
#define GPT_TBL_DESC_ADDR_MASK		(((ULL(1) <<			      \
					 (51 - GPT_TBL_DESC_ADDR_SHIFT)) - 1) \
					 << GPT_TBL_DESC_ADDR_SHIFT)

#define GPT_BLOCK_DESC_GPI_VAL_SHIFT	ULL(4)

/* Each descriptor is 8 bytes long. */
#define GPT_DESC_SIZE			ULL(8)

#define PPS_MAX_VAL			PSTCR_EL3_PPS_4PB
#define PPS_NUM_1GB_ENTRIES		ULL(1024)
#define PGS_4K_1GB_L1_TABLE_SZ		(U(2) << 17)

/* 2 << LOG2_8K = Bytes in 8K */
#define LOG2_8K                         U(13)

#define GPT_L1_SIZE			ULL(0x40000)		/* 256K */
#define SZ_1G				(ULL(0x1) << 30)	/* 1GB */

#define GPT_MIN_PGS_SHIFT		U(12)			/* 4K */

#define L1_GPT_INDEX_MASK		U(0x3fffffff)
#define GPT_GRAN_DESC_NUM_GPIS		U(4)

#define PAS_REG_GPI_SHIFT		U(0)
#define PAS_REG_GPI_MASK		U(0xf)

/* .attrs field definitions */
#define PAS_REG_DESC_TYPE_ANY		U(0)
#define PAS_REG_DESC_TYPE_BLK		U(1)
#define PAS_REG_DESC_TYPE_TBL		U(2)
#define PAS_REG_DESC_TYPE_SHIFT		U(4)
#define PAS_REG_DESC_TYPE_MASK		U(0x3)
#define PAS_REG_DESC_TYPE(_attrs)	(((_attrs)			\
					  >> PAS_REG_DESC_TYPE_SHIFT)	\
					 & PAS_REG_DESC_TYPE_MASK)

#define PAS_REG_GPI(_attrs)		(((_attrs)			\
					  >> PAS_REG_GPI_SHIFT)		\
					 & PAS_REG_GPI_MASK)

#define SZ_1G_MASK			(SZ_1G - U(1))
#define IS_1GB_ALIGNED(addr)		(((addr) & SZ_1G_MASK) == U(0))

#endif /* GPT_DEFS */

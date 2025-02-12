/*
 * Copyright (c) 2024-2025, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QEMU_PAS_DEF_H
#define QEMU_PAS_DEF_H

#include <lib/gpt_rme/gpt_rme.h>
#include "platform_def.h"

/*****************************************************************************
 * PAS regions used to initialize the Granule Protection Table (GPT)
 ****************************************************************************/

/* EL3 SRAM */
#define QEMU_PAS_ROOT_BASE	(BL32_MEM_BASE + BL32_MEM_SIZE)
#define QEMU_PAS_ROOT_SIZE	(BL_RAM_SIZE - \
				 (BL32_MEM_SIZE + RME_GPT_DRAM_SIZE))

/* Secure DRAM */
#define QEMU_PAS_SEC_BASE	BL32_MEM_BASE /* BL32_SRAM_BASE */
#define QEMU_PAS_SEC_SIZE	BL32_MEM_SIZE

/* GPTs */
#define QEMU_PAS_GPT_BASE	RME_GPT_DRAM_BASE /* PLAT_QEMU_L0_GPT_BASE */
#define QEMU_PAS_GPT_SIZE	RME_GPT_DRAM_SIZE

/* RMM */
#define QEMU_PAS_RMM_BASE	RMM_BASE
#define QEMU_PAS_RMM_SIZE	PLAT_QEMU_RMM_SIZE

/* Shared area between EL3 and RMM */
#define QEMU_PAS_RMM_SHARED_BASE	RMM_SHARED_BASE
#define QEMU_PAS_RMM_SHARED_SIZE	RMM_SHARED_SIZE

#define QEMU_PAS_ROOT		GPT_MAP_REGION_GRANULE(QEMU_PAS_ROOT_BASE, \
						       QEMU_PAS_ROOT_SIZE, \
						       GPT_GPI_ROOT)

#define QEMU_PAS_SECURE		GPT_MAP_REGION_GRANULE(QEMU_PAS_SEC_BASE, \
						       QEMU_PAS_SEC_SIZE, \
						       GPT_GPI_SECURE)

#define QEMU_PAS_GPTS		GPT_MAP_REGION_GRANULE(QEMU_PAS_GPT_BASE, \
						       QEMU_PAS_GPT_SIZE, \
						       GPT_GPI_ROOT)

/*
 * NS0 base address and size are fetched from the DT at runtime.
 * See bl31_adjust_pas_regions() for details
 */
#define QEMU_PAS_NS0		GPT_MAP_REGION_GRANULE(0, 0, GPT_GPI_NS)

#define QEMU_PAS_REALM		GPT_MAP_REGION_GRANULE(QEMU_PAS_RMM_BASE, \
					       QEMU_PAS_RMM_SIZE + \
					       QEMU_PAS_RMM_SHARED_SIZE, \
					       GPT_GPI_REALM)

/* Cover 4TB with L0GTP */
#define PLAT_QEMU_GPCCR_PPS	GPCCR_PPS_4TB
#define PLAT_QEMU_PPS		SZ_4T

/* GPT Configuration options */
#define PLATFORM_L0GPTSZ	GPCCR_L0GPTSZ_30BITS

#endif /* QEMU_PAS_DEF_H */

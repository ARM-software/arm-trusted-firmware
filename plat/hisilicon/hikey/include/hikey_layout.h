/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HIKEY_LAYOUT_H
#define HIKEY_LAYOUT_H

/*
 * Platform memory map related constants
 */
#define XG2RAM0_BASE		0xF9800000
#define XG2RAM0_SIZE		0x00400000

/*
 * BL1 is stored in XG2RAM0_HIRQ that is 784KB large (0xF980_0000~0xF98C_4000).
 */
#define ONCHIPROM_PARAM_BASE		(XG2RAM0_BASE + 0x700)
#define LOADER_RAM_BASE			(XG2RAM0_BASE + 0x800)
#define BL1_XG2RAM0_OFFSET		0x1000

/*
 * BL1 specific defines.
 *
 * Both loader and BL1_RO region stay in SRAM since they are used to simulate
 * ROM.
 * Loader is used to switch Hi6220 SoC from 32-bit to 64-bit mode.
 *
 * ++++++++++  0xF980_0000
 * + loader +
 * ++++++++++  0xF980_1000
 * + BL1_RO +
 * ++++++++++  0xF981_8000
 * + BL1_RW +
 * ++++++++++  0xF989_8000
 */
#define BL1_RO_BASE			(XG2RAM0_BASE + BL1_XG2RAM0_OFFSET)
#define BL1_RO_LIMIT			(XG2RAM0_BASE + 0x18000)
#define BL1_RW_BASE			(BL1_RO_LIMIT)	/* 0xf981_8000 */
#define BL1_RW_SIZE			(0x00080000)
#define BL1_RW_LIMIT			(0xF9898000)

/*
 * Non-Secure BL1U specific defines.
 */
#define NS_BL1U_BASE			(0xf9828000)
#define NS_BL1U_SIZE			(0x00010000)
#define NS_BL1U_LIMIT			(NS_BL1U_BASE + NS_BL1U_SIZE)

/*
 * BL2 specific defines.
 *
 * Both loader and BL2 region stay in SRAM.
 * Loader is used to switch Hi6220 SoC from 32-bit to 64-bit mode.
 *
 * ++++++++++ 0xF980_0000
 * + loader +
 * ++++++++++ 0xF980_1000
 * +  BL2   +
 * ++++++++++ 0xF983_0000
 */
#define BL2_BASE			(BL1_RO_BASE)		/* 0xf980_1000 */
#define BL2_LIMIT			(0xF9830000)		/* 0xf983_0000 */

/*
 * SCP_BL2 specific defines.
 * In HiKey, SCP_BL2 means MCU firmware. It's loaded into the temporary buffer
 * at 0x0100_0000. Then BL2 will parse the sections and loaded them into
 * predefined separated buffers.
 */
#define SCP_BL2_BASE			(DDR_BASE + 0x01000000)
#define SCP_BL2_LIMIT			(SCP_BL2_BASE + 0x00100000)
#define SCP_BL2_SIZE			(SCP_BL2_LIMIT - SCP_BL2_BASE)

/*
 * BL31 specific defines.
 */
#define BL31_BASE			(0xF9858000)		/* 0xf985_8000 */
#define BL31_LIMIT			(0xF9898000)

/*
 * BL3-2 specific defines.
 */

/*
 * The TSP currently executes from TZC secured area of DRAM or SRAM.
 */
#define BL32_SRAM_BASE			BL31_LIMIT
#define BL32_SRAM_LIMIT			(BL31_LIMIT+0x80000) /* 512K */

#define BL32_DRAM_BASE			DDR_SEC_BASE
#define BL32_DRAM_LIMIT			(DDR_SEC_BASE+DDR_SEC_SIZE)

#ifdef SPD_opteed
/* Load pageable part of OP-TEE at end of allocated DRAM space for BL32 */
#define HIKEY_OPTEE_PAGEABLE_LOAD_BASE	(BL32_DRAM_LIMIT - HIKEY_OPTEE_PAGEABLE_LOAD_SIZE) /* 0x3FC0_0000 */
#define HIKEY_OPTEE_PAGEABLE_LOAD_SIZE	0x400000 /* 4MB */
#endif

#if (HIKEY_TSP_RAM_LOCATION_ID == HIKEY_DRAM_ID)
#define TSP_SEC_MEM_BASE		BL32_DRAM_BASE
#define TSP_SEC_MEM_SIZE		(BL32_DRAM_LIMIT - BL32_DRAM_BASE)
#define BL32_BASE			BL32_DRAM_BASE
#define BL32_LIMIT			BL32_DRAM_LIMIT
#elif (HIKEY_TSP_RAM_LOCATION_ID == HIKEY_SRAM_ID)
#define TSP_SEC_MEM_BASE		BL32_SRAM_BASE
#define TSP_SEC_MEM_SIZE		(BL32_SRAM_LIMIT - BL32_SRAM_BASE)
#define BL32_BASE			BL32_SRAM_BASE
#define BL32_LIMIT			BL32_SRAM_LIMIT
#else
#error "Currently unsupported HIKEY_TSP_LOCATION_ID value"
#endif

/* BL32 is mandatory in AArch32 */
#ifdef __aarch64__
#ifdef SPD_none
#undef BL32_BASE
#endif /* SPD_none */
#endif

#endif /* HIKEY_LAYOUT_H */

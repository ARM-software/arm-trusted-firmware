/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_DEFAULT_DEF_H
#define PLAT_DEFAULT_DEF_H

/*
 * Platform binary types for linking
 */
#ifdef __aarch64__
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64
#else
#define PLATFORM_LINKER_FORMAT          "elf32-littlearm"
#define PLATFORM_LINKER_ARCH            arm
#endif /* __aarch64__ */

#define LS_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

/* NXP Platforms have DRAM divided into banks.
 * DRAM0 Bank:	Maximum size of this bank is fixed to 2GB
 * DRAM1 Bank:	Greater than 2GB belongs to bank1 and size of bank1 varies from
 *		one platform to other platform.
 * DRAMn Bank:
 *
 * Except a few, all the platforms have 2GB size as DRAM0 BANK.
 * Hence common for all the platforms.
 * For platforms where DRAM0 Size is < 2GB, it is defined in platform_def.h
 */
#ifndef PLAT_DEF_DRAM0_SIZE
#define PLAT_DEF_DRAM0_SIZE	0x80000000	/*  2G */
#endif

/* This is common for all platforms where: */
#ifndef NXP_NS_DRAM_ADDR
#define NXP_NS_DRAM_ADDR	NXP_DRAM0_ADDR
#endif

/* 1 MB is reserved for dma of sd */
#ifndef NXP_SD_BLOCK_BUF_SIZE
#define NXP_SD_BLOCK_BUF_SIZE	(1 * 1024 * 1024)
#endif

/* 64MB is reserved for Secure memory */
#ifndef NXP_SECURE_DRAM_SIZE
#define NXP_SECURE_DRAM_SIZE	(64 * 1024 * 1024)
#endif

/* 2M Secure EL1 Payload Shared Memory */
#ifndef NXP_SP_SHRD_DRAM_SIZE
#define NXP_SP_SHRD_DRAM_SIZE	(2 * 1024 * 1024)
#endif

#ifndef NXP_NS_DRAM_SIZE
/* Non secure memory */
#define NXP_NS_DRAM_SIZE	(PLAT_DEF_DRAM0_SIZE - \
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE))
#endif

#ifndef NXP_SD_BLOCK_BUF_ADDR
#define NXP_SD_BLOCK_BUF_ADDR	(NXP_NS_DRAM_ADDR)
#endif

#ifndef NXP_SECURE_DRAM_ADDR
#ifdef TEST_BL31
#define NXP_SECURE_DRAM_ADDR 0
#else
#define NXP_SECURE_DRAM_ADDR	(NXP_NS_DRAM_ADDR + PLAT_DEF_DRAM0_SIZE - \
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE))
#endif
#endif

#ifndef NXP_SP_SHRD_DRAM_ADDR
#define NXP_SP_SHRD_DRAM_ADDR	(NXP_NS_DRAM_ADDR + PLAT_DEF_DRAM0_SIZE - \
				NXP_SP_SHRD_DRAM_SIZE)
#endif

#ifndef BL31_BASE
/* 2 MB reserved in secure memory for DDR */
#define BL31_BASE		NXP_SECURE_DRAM_ADDR
#endif

#ifndef BL31_SIZE
#define BL31_SIZE		(0x200000)
#endif

#ifndef BL31_LIMIT
#define BL31_LIMIT		(BL31_BASE + BL31_SIZE)
#endif

/* Put BL32 in secure memory */
#ifndef BL32_BASE
#define BL32_BASE		(NXP_SECURE_DRAM_ADDR + BL31_SIZE)
#endif

#ifndef BL32_LIMIT
#define BL32_LIMIT		(NXP_SECURE_DRAM_ADDR + \
				NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)
#endif

/* BL33 memory region */
/* Hardcoded based on current address in u-boot */
#ifndef BL33_BASE
#define BL33_BASE		0x82000000
#endif

#ifndef BL33_LIMIT
#define BL33_LIMIT		(NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE)
#endif

/*
 * FIP image defines - Offset at which FIP Image would be present
 * Image would include Bl31 , Bl33 and Bl32 (optional)
 */
#ifdef POLICY_FUSE_PROVISION
#ifndef FUSE_BUF
#define FUSE_BUF		ULL(0x81000000)
#endif

#ifndef FUSE_SZ
#define FUSE_SZ			0x80000
#endif
#endif

#ifndef MAX_FIP_DEVICES
#define MAX_FIP_DEVICES		2
#endif

#ifndef PLAT_FIP_OFFSET
#define PLAT_FIP_OFFSET		0x100000
#endif

#ifndef PLAT_FIP_MAX_SIZE
#define PLAT_FIP_MAX_SIZE	0x400000
#endif

/* Check if this size can be determined from array size */
#if defined(IMAGE_BL2)
#ifndef MAX_MMAP_REGIONS
#define MAX_MMAP_REGIONS	8
#endif
#ifndef MAX_XLAT_TABLES
#define MAX_XLAT_TABLES		6
#endif
#elif defined(IMAGE_BL31)
#ifndef MAX_MMAP_REGIONS
#define MAX_MMAP_REGIONS	9
#endif
#ifndef MAX_XLAT_TABLES
#define MAX_XLAT_TABLES		9
#endif
#elif defined(IMAGE_BL32)
#ifndef MAX_MMAP_REGIONS
#define MAX_MMAP_REGIONS	8
#endif
#ifndef MAX_XLAT_TABLES
#define MAX_XLAT_TABLES		9
#endif
#endif

/*
 * ID of the secure physical generic timer interrupt used by the BL32.
 */
#ifndef BL32_IRQ_SEC_PHY_TIMER
#define BL32_IRQ_SEC_PHY_TIMER	29
#endif

#endif	/*	PLAT_DEFAULT_DEF_H	*/

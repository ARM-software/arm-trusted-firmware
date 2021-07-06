/*
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_STM32IMAGE_DEF_H
#define STM32MP1_STM32IMAGE_DEF_H

#ifdef AARCH32_SP_OPTEE
#define STM32MP_DDR_S_SIZE		U(0x01E00000)	/* 30 MB */
#define STM32MP_DDR_SHMEM_SIZE		U(0x00200000)	/* 2 MB */
#else
#define STM32MP_DDR_S_SIZE		U(0)
#define STM32MP_DDR_SHMEM_SIZE		U(0)
#endif

#define STM32MP_BL2_SIZE		U(0x0001C000)	/* 112 KB for BL2 */
#define STM32MP_DTB_SIZE		U(0x00006000)	/* 24 KB for DTB */

#ifdef AARCH32_SP_OPTEE
#define STM32MP_BL32_BASE		STM32MP_SEC_SYSRAM_BASE

#define STM32MP_BL2_BASE		(STM32MP_SEC_SYSRAM_BASE + \
					 STM32MP_SEC_SYSRAM_SIZE - \
					 STM32MP_BL2_SIZE)

/* OP-TEE loads from SYSRAM base to BL2 DTB start address */
#define STM32MP_OPTEE_BASE		STM32MP_BL32_BASE
#define STM32MP_OPTEE_SIZE		(STM32MP_SEC_SYSRAM_SIZE -  \
					 STM32MP_BL2_SIZE - STM32MP_DTB_SIZE)
#define STM32MP_BL32_SIZE		STM32MP_OPTEE_SIZE
#else /* AARCH32_SP_OPTEE */
#define STM32MP_BL32_SIZE		U(0x00019000)	/* 96 KB for BL32 */

#define STM32MP_BL32_BASE		(STM32MP_SEC_SYSRAM_BASE + \
					 STM32MP_SEC_SYSRAM_SIZE - \
					 STM32MP_BL32_SIZE)

#define STM32MP_BL2_BASE		(STM32MP_BL32_BASE - \
					 STM32MP_BL2_SIZE)
#endif /* AARCH32_SP_OPTEE */

/* DTB initialization value */
#define STM32MP_DTB_BASE		(STM32MP_BL2_BASE -	\
					 STM32MP_DTB_SIZE)

/*
 * MAX_MMAP_REGIONS is usually:
 * BL stm32mp1_mmap size + mmap regions in *_plat_arch_setup
 */
#if defined(IMAGE_BL32)
#define MAX_MMAP_REGIONS		6
#endif

/*******************************************************************************
 * STM32MP1 RAW partition offset for MTD devices
 ******************************************************************************/
#define STM32MP_NOR_BL33_OFFSET		U(0x00080000)
#ifdef AARCH32_SP_OPTEE
#define STM32MP_NOR_TEEH_OFFSET		U(0x00280000)
#define STM32MP_NOR_TEED_OFFSET		U(0x002C0000)
#define STM32MP_NOR_TEEX_OFFSET		U(0x00300000)
#endif

#define STM32MP_NAND_BL33_OFFSET	U(0x00200000)
#ifdef AARCH32_SP_OPTEE
#define STM32MP_NAND_TEEH_OFFSET	U(0x00600000)
#define STM32MP_NAND_TEED_OFFSET	U(0x00680000)
#define STM32MP_NAND_TEEX_OFFSET	U(0x00700000)
#endif

#endif /* STM32MP1_STM32IMAGE_DEF_H */

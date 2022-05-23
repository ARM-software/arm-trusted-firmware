/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/* CPU topology */
#define PLAT_MAX_CORES_PER_CLUSTER	U(2)
#define PLAT_CLUSTER_COUNT		U(12)
#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER_COUNT *	\
					 PLAT_MAX_CORES_PER_CLUSTER)

/* Macros to read the SQ power domain state */
#define SQ_PWR_LVL0		MPIDR_AFFLVL0
#define SQ_PWR_LVL1		MPIDR_AFFLVL1
#define SQ_PWR_LVL2		MPIDR_AFFLVL2

#define SQ_CORE_PWR_STATE(state)	(state)->pwr_domain_state[SQ_PWR_LVL0]
#define SQ_CLUSTER_PWR_STATE(state)	(state)->pwr_domain_state[SQ_PWR_LVL1]
#define SQ_SYSTEM_PWR_STATE(state)	((PLAT_MAX_PWR_LVL > SQ_PWR_LVL1) ?\
				(state)->pwr_domain_state[SQ_PWR_LVL2] : 0)

#define PLAT_MAX_PWR_LVL		U(1)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

#define SQ_LOCAL_STATE_RUN		0
#define SQ_LOCAL_STATE_RET		1
#define SQ_LOCAL_STATE_OFF		2

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES			8
#define MAX_MMAP_REGIONS		8

#if TRUSTED_BOARD_BOOT
#define PLATFORM_STACK_SIZE		0x1000
#else
#define PLATFORM_STACK_SIZE		0x400
#endif

#if !RESET_TO_BL31

/* A mailbox page will be mapped from BL2 and BL31 */
#define BL2_MAILBOX_BASE		0x0403f000
#define BL2_MAILBOX_SIZE		0x1000

#define PLAT_SQ_BOOTIDX_BASE		0x08510000
#define PLAT_SQ_MAX_BOOT_INDEX		2

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES	U(1)

#define BL2_BASE			0x04000000
#define BL2_SIZE			(256 * 1024)
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

/* If BL2 is enabled, the BL31 is loaded on secure DRAM */
#define BL31_BASE			0xfbe00000
#define BL31_SIZE			0x00100000
#else

#define BL31_BASE			0x04000000
#define BL31_SIZE			0x00080000
#endif

#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0xfc000000
#define BL32_SIZE			0x03c00000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

/* Alternative BL33 */
#define PLAT_SQ_BL33_BASE		0xe0000000
#define PLAT_SQ_BL33_SIZE		0x00100000

/* FWU FIP IO base */
#define PLAT_SQ_FIP_IOBASE		0x08600000
#define PLAT_SQ_FIP_MAXSIZE		0x00400000

#define PLAT_SQ_CCN_BASE		0x32000000
#define PLAT_SQ_CLUSTER_TO_CCN_ID_MAP					\
					0,	/* Cluster 0 */		\
					18,	/* Cluster 1 */		\
					11,	/* Cluster 2 */		\
					29,	/* Cluster 3 */		\
					35,	/* Cluster 4 */		\
					17,	/* Cluster 5 */		\
					12,	/* Cluster 6 */		\
					30,	/* Cluster 7 */		\
					14,	/* Cluster 8 */		\
					32,	/* Cluster 9 */		\
					15,	/* Cluster 10 */	\
					33	/* Cluster 11 */

/* UART related constants */
#define PLAT_SQ_BOOT_UART_BASE		0x2A400000
#define PLAT_SQ_BOOT_UART_CLK_IN_HZ	62500000
#define SQ_CONSOLE_BAUDRATE		115200

#define SQ_SYS_CNTCTL_BASE		0x2a430000

#define SQ_SYS_TIMCTL_BASE		0x2a810000
#define PLAT_SQ_NSTIMER_FRAME_ID	0
#define SQ_SYS_CNT_BASE_NS		0x2a830000

#define DRAMINFO_BASE			0x2E00FFC0

#define PLAT_SQ_MHU_BASE		0x45000000

#define PLAT_SQ_SCP_COM_SHARED_MEM_BASE		0x45400000
#define SCPI_CMD_GET_DRAMINFO			0x1

#define SQ_BOOT_CFG_ADDR			0x45410000
#define PLAT_SQ_PRIMARY_CPU_SHIFT		8
#define PLAT_SQ_PRIMARY_CPU_BIT_WIDTH		6

#define PLAT_SQ_GICD_BASE		0x30000000
#define PLAT_SQ_GICR_BASE		0x30400000

#define PLAT_SQ_GPIO_BASE		0x51000000

#define PLAT_SPM_BUF_BASE		(BL32_LIMIT - 32 * PLAT_SPM_BUF_SIZE)
#define PLAT_SPM_BUF_SIZE		ULL(0x10000)
#define PLAT_SPM_SPM_BUF_EL0_MMAP	MAP_REGION2(PLAT_SPM_BUF_BASE, \
						    PLAT_SPM_BUF_BASE, \
						    PLAT_SPM_BUF_SIZE, \
						    MT_RO_DATA | MT_SECURE | \
						    MT_USER, PAGE_SIZE)

#define PLAT_SP_IMAGE_NS_BUF_BASE	BL32_LIMIT
#define PLAT_SP_IMAGE_NS_BUF_SIZE	ULL(0x200000)
#define PLAT_SP_IMAGE_NS_BUF_MMAP	MAP_REGION2(PLAT_SP_IMAGE_NS_BUF_BASE, \
						    PLAT_SP_IMAGE_NS_BUF_BASE, \
						    PLAT_SP_IMAGE_NS_BUF_SIZE, \
						    MT_RW_DATA | MT_NS | \
						    MT_USER, PAGE_SIZE)

#define PLAT_SP_IMAGE_STACK_PCPU_SIZE	ULL(0x10000)
#define PLAT_SP_IMAGE_STACK_SIZE	(32 * PLAT_SP_IMAGE_STACK_PCPU_SIZE)
#define PLAT_SP_IMAGE_STACK_BASE	(PLAT_SQ_SP_HEAP_BASE + PLAT_SQ_SP_HEAP_SIZE)

#define PLAT_SQ_SP_IMAGE_SIZE		ULL(0x200000)
#define PLAT_SQ_SP_IMAGE_MMAP		MAP_REGION2(BL32_BASE, BL32_BASE, \
						    PLAT_SQ_SP_IMAGE_SIZE, \
						    MT_CODE | MT_SECURE | \
						    MT_USER, PAGE_SIZE)

#define PLAT_SQ_SP_HEAP_BASE		(BL32_BASE + PLAT_SQ_SP_IMAGE_SIZE)
#define PLAT_SQ_SP_HEAP_SIZE		ULL(0x800000)

#define PLAT_SQ_SP_IMAGE_RW_MMAP	MAP_REGION2(PLAT_SQ_SP_HEAP_BASE, \
						    PLAT_SQ_SP_HEAP_BASE, \
						    (PLAT_SQ_SP_HEAP_SIZE + \
						     PLAT_SP_IMAGE_STACK_SIZE), \
						    MT_RW_DATA | MT_SECURE | \
						    MT_USER, PAGE_SIZE)

#define PLAT_SQ_SP_PRIV_BASE		(PLAT_SP_IMAGE_STACK_BASE + \
					 PLAT_SP_IMAGE_STACK_SIZE)
#define PLAT_SQ_SP_PRIV_SIZE		ULL(0x40000)

#define PLAT_SP_PRI			0x20
#define PLAT_PRI_BITS			2
#define PLAT_SPM_COOKIE_0		ULL(0)
#define PLAT_SPM_COOKIE_1		ULL(0)

/* Total number of memory regions with distinct properties */
#define PLAT_SP_IMAGE_NUM_MEM_REGIONS	6

#define PLAT_SP_IMAGE_MMAP_REGIONS	30
#define PLAT_SP_IMAGE_MAX_XLAT_TABLES	20
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	"sp_xlat_table"
#define PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME	"sp_xlat_table"

#define PLAT_SQ_UART1_BASE		PLAT_SQ_BOOT_UART_BASE
#define PLAT_SQ_UART1_SIZE		ULL(0x1000)
#define PLAT_SQ_UART1_MMAP		MAP_REGION_FLAT(PLAT_SQ_UART1_BASE, \
							PLAT_SQ_UART1_SIZE, \
							MT_DEVICE | MT_RW | \
							MT_NS | MT_PRIVILEGED)

#define PLAT_SQ_PERIPH_BASE		0x50000000
#define PLAT_SQ_PERIPH_SIZE		ULL(0x8000000)
#define PLAT_SQ_PERIPH_MMAP		MAP_REGION_FLAT(PLAT_SQ_PERIPH_BASE, \
							PLAT_SQ_PERIPH_SIZE, \
							MT_DEVICE | MT_RW | \
							MT_NS | MT_USER)

#define PLAT_SQ_FLASH_BASE		0x08000000
#define PLAT_SQ_FLASH_SIZE		ULL(0x8000000)
#define PLAT_SQ_FLASH_MMAP		MAP_REGION_FLAT(PLAT_SQ_FLASH_BASE, \
							PLAT_SQ_FLASH_SIZE, \
							MT_DEVICE | MT_RW | \
							MT_NS | MT_USER)

#endif /* PLATFORM_DEF_H */

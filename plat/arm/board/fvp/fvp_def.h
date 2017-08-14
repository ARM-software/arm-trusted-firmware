/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FVP_DEF_H__
#define __FVP_DEF_H__

#ifndef FVP_CLUSTER_COUNT
#define FVP_CLUSTER_COUNT		2
#endif
#define FVP_MAX_CPUS_PER_CLUSTER	4

#ifndef FVP_MAX_PE_PER_CPU
# define FVP_MAX_PE_PER_CPU		1
#endif

#define FVP_PRIMARY_CPU			0x0

/* Defines for the Interconnect build selection */
#define FVP_CCI			1
#define FVP_CCN			2

/*******************************************************************************
 * FVP memory map related constants
 ******************************************************************************/

#define FLASH1_BASE			0x0c000000
#define FLASH1_SIZE			0x04000000

#define PSRAM_BASE			0x14000000
#define PSRAM_SIZE			0x04000000

#define VRAM_BASE			0x18000000
#define VRAM_SIZE			0x02000000

/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE			0x20000000
#define DEVICE0_SIZE			0x0c200000

/*
 *  In case of FVP models with CCN, the CCN register space overlaps into
 *  the NSRAM area.
 */
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
#define DEVICE1_BASE			0x2e000000
#define DEVICE1_SIZE			0x1A00000
#else
#define DEVICE1_BASE			0x2f000000
#define DEVICE1_SIZE			0x200000
#define NSRAM_BASE			0x2e000000
#define NSRAM_SIZE			0x10000
#endif
/* Devices in the second GB */
#define DEVICE2_BASE			0x7fe00000
#define DEVICE2_SIZE			0x00200000

#define PCIE_EXP_BASE			0x40000000
#define TZRNG_BASE			0x7fe60000

/* Non-volatile counters */
#define TRUSTED_NVCTR_BASE		0x7fe70000
#define TFW_NVCTR_BASE			(TRUSTED_NVCTR_BASE + 0x0000)
#define TFW_NVCTR_SIZE			4
#define NTFW_CTR_BASE			(TRUSTED_NVCTR_BASE + 0x0004)
#define NTFW_CTR_SIZE			4

/* Keys */
#define SOC_KEYS_BASE			0x7fe80000
#define TZ_PUB_KEY_HASH_BASE		(SOC_KEYS_BASE + 0x0000)
#define TZ_PUB_KEY_HASH_SIZE		32
#define HU_KEY_BASE			(SOC_KEYS_BASE + 0x0020)
#define HU_KEY_SIZE			16
#define END_KEY_BASE			(SOC_KEYS_BASE + 0x0044)
#define END_KEY_SIZE			32

/* Constants to distinguish FVP type */
#define HBI_BASE_FVP			0x020
#define REV_BASE_FVP_V0			0x0
#define REV_BASE_FVP_REVC		0x2

#define HBI_FOUNDATION_FVP		0x010
#define REV_FOUNDATION_FVP_V2_0		0x0
#define REV_FOUNDATION_FVP_V2_1		0x1
#define REV_FOUNDATION_FVP_v9_1		0x2
#define REV_FOUNDATION_FVP_v9_6		0x3

#define BLD_GIC_VE_MMAP			0x0
#define BLD_GIC_A53A57_MMAP		0x1

#define ARCH_MODEL			0x1

/* FVP Power controller base address*/
#define PWRC_BASE			0x1c100000

/* FVP SP804 timer frequency is 35 MHz*/
#define SP804_TIMER_CLKMULT		1
#define SP804_TIMER_CLKDIV		35

/* SP810 controller. FVP specific flags */
#define FVP_SP810_CTRL_TIM0_OV		(1 << 16)
#define FVP_SP810_CTRL_TIM1_OV		(1 << 18)
#define FVP_SP810_CTRL_TIM2_OV		(1 << 20)
#define FVP_SP810_CTRL_TIM3_OV		(1 << 22)

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
/* VE compatible GIC memory map */
#define VE_GICD_BASE			0x2c001000
#define VE_GICC_BASE			0x2c002000
#define VE_GICH_BASE			0x2c004000
#define VE_GICV_BASE			0x2c006000

/* Base FVP compatible GIC memory map */
#define BASE_GICD_BASE			0x2f000000
#define BASE_GICR_BASE			0x2f100000
#define BASE_GICC_BASE			0x2c000000
#define BASE_GICH_BASE			0x2c010000
#define BASE_GICV_BASE			0x2c02f000

#define FVP_IRQ_TZ_WDOG			56
#define FVP_IRQ_SEC_SYS_TIMER		57


/*******************************************************************************
 * TrustZone address space controller related constants
 ******************************************************************************/

/* NSAIDs used by devices in TZC filter 0 on FVP */
#define FVP_NSAID_DEFAULT		0
#define FVP_NSAID_PCI			1
#define FVP_NSAID_VIRTIO		8  /* from FVP v5.6 onwards */
#define FVP_NSAID_AP			9  /* Application Processors */
#define FVP_NSAID_VIRTIO_OLD		15 /* until FVP v5.5 */

/* NSAIDs used by devices in TZC filter 2 on FVP */
#define FVP_NSAID_HDLCD0		2
#define FVP_NSAID_CLCD			7

#endif /* __FVP_DEF_H__ */

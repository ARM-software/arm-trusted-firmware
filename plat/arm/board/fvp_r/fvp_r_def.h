/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_R_DEF_H
#define FVP_R_DEF_H

#include <lib/utils_def.h>

/******************************************************************************
 * FVP-R topology constants
 *****************************************************************************/
#define FVP_R_CLUSTER_COUNT		2
#define FVP_R_MAX_CPUS_PER_CLUSTER	4
#define FVP_R_MAX_PE_PER_CPU		1
#define FVP_R_PRIMARY_CPU		0x0

/******************************************************************************
 * Definition of platform soc id
 *****************************************************************************/
#define FVP_R_SOC_ID			0

/*******************************************************************************
 * FVP_R memory map related constants
 ******************************************************************************/

#define FLASH1_BASE			UL(0x8c000000)
#define FLASH1_SIZE			UL(0x04000000)

#define PSRAM_BASE			UL(0x94000000)
#define PSRAM_SIZE			UL(0x04000000)

#define VRAM_BASE			UL(0x98000000)
#define VRAM_SIZE			UL(0x02000000)

/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE			UL(0xa0000000)
#define DEVICE0_SIZE			UL(0x0c200000)

/*
 *  In case of FVP_R models with CCN, the CCN register space overlaps into
 *  the NSRAM area.
 */
#define DEVICE1_BASE			UL(0xae000000)
#define DEVICE1_SIZE			UL(0x1A00000)

#define NSRAM_BASE			UL(0xae000000)
#define NSRAM_SIZE			UL(0x10000)
/* Devices in the second GB */
#define DEVICE2_BASE			UL(0xffe00000)
#define DEVICE2_SIZE			UL(0x00200000)

#define PCIE_EXP_BASE			UL(0xc0000000)
#define TZRNG_BASE			UL(0x7fe60000)

/* Non-volatile counters */
#define TRUSTED_NVCTR_BASE		UL(0xffe70000)
#define TFW_NVCTR_BASE			(TRUSTED_NVCTR_BASE + UL(0x0000))
#define TFW_NVCTR_SIZE			UL(4)
#define NTFW_CTR_BASE			(TRUSTED_NVCTR_BASE + UL(0x0004))
#define NTFW_CTR_SIZE			UL(4)

/* Keys */
#define SOC_KEYS_BASE			UL(0xffe80000)
#define TZ_PUB_KEY_HASH_BASE		(SOC_KEYS_BASE + UL(0x0000))
#define TZ_PUB_KEY_HASH_SIZE		UL(32)
#define HU_KEY_BASE			(SOC_KEYS_BASE + UL(0x0020))
#define HU_KEY_SIZE			UL(16)
#define END_KEY_BASE			(SOC_KEYS_BASE + UL(0x0044))
#define END_KEY_SIZE			UL(32)

/* Constants to distinguish FVP_R type */
#define HBI_BASE_FVP_R			U(0x020)
#define REV_BASE_FVP_R_V0		U(0x0)
#define REV_BASE_FVP_R_REVC		U(0x2)

#define HBI_FOUNDATION_FVP_R		U(0x010)
#define REV_FOUNDATION_FVP_R_V2_0	U(0x0)
#define REV_FOUNDATION_FVP_R_V2_1	U(0x1)
#define REV_FOUNDATION_FVP_R_v9_1	U(0x2)
#define REV_FOUNDATION_FVP_R_v9_6	U(0x3)

#define BLD_GIC_VE_MMAP			U(0x0)
#define BLD_GIC_A53A57_MMAP		U(0x1)

#define ARCH_MODEL			U(0x1)

/* FVP_R Power controller base address*/
#define PWRC_BASE			UL(0x1c100000)

/* FVP_R SP804 timer frequency is 35 MHz*/
#define SP804_TIMER_CLKMULT		1
#define SP804_TIMER_CLKDIV		35

/* SP810 controller. FVP_R specific flags */
#define FVP_R_SP810_CTRL_TIM0_OV		BIT_32(16)
#define FVP_R_SP810_CTRL_TIM1_OV		BIT_32(18)
#define FVP_R_SP810_CTRL_TIM2_OV		BIT_32(20)
#define FVP_R_SP810_CTRL_TIM3_OV		BIT_32(22)

#endif /* FVP_R_DEF_H */

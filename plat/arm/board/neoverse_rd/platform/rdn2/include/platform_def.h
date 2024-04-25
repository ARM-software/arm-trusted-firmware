/*
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#include <nrd_css_fw_def2.h>
#include <nrd_plat_arm_def2.h>
#include <nrd_ros_fw_def2.h>
#include <nrd_sdei.h>

/* Remote chip address offset */
#define NRD_REMOTE_CHIP_MEM_OFFSET(n)					\
		((ULL(1) << NRD_ADDR_BITS_PER_CHIP) * (n))

#define NRD_MAX_CPUS_PER_CLUSTER	U(1)
#define NRD_MAX_PE_PER_CPU		U(1)

/* Boot ROM */
#define NRD_CSS_SECURE_ROM_SIZE		UL(0x00080000) /* 512KB */

/* Secure SRAM */
#define NRD_CSS_SECURE_SRAM_SIZE	UL(0x00080000) /* 512KB */

/* NS SRAM */
#define NRD_CSS_NS_SRAM_SIZE		UL(0x00080000) /* 512KB */

/* DRAM2 */
#define NRD_CSS_DRAM2_SIZE		ULL(0x180000000) /* 6GB */

#define TZC400_OFFSET			UL(0x1000000)

#if (NRD_PLATFORM_VARIANT == 1)
#define TZC400_COUNT			U(2)
#elif (NRD_PLATFORM_VARIANT == 2)
#define TZC400_COUNT			U(4)
#else
#define TZC400_COUNT			U(8)
#endif

#define TZC400_BASE(n)			(PLAT_ARM_TZC_BASE + \
						(n * TZC400_OFFSET))

#define TZC_NSAID_ALL_AP		U(0)
#define TZC_NSAID_PCI			U(1)
#define TZC_NSAID_HDLCD0		U(2)
#define TZC_NSAID_DMA			U(5)
#define TZC_NSAID_DMA2			U(8)
#define TZC_NSAID_CLCD			U(7)
#define TZC_NSAID_AP			U(9)
#define TZC_NSAID_VIRTIO		U(15)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	\
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_ALL_AP)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_HDLCD0)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_PCI))    | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_DMA))    | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_DMA2))   | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_AP))     | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_CLCD))   | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_VIRTIO))

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#if (NRD_PLATFORM_VARIANT == 2)
#define NRD_ADDR_BITS_PER_CHIP	U(46)	/* 64TB */
#else
#define NRD_ADDR_BITS_PER_CHIP	U(42)	/* 4TB */
#endif

/* GIC SPI range for multichip */
#define NRD_CHIP0_SPI_MIN		U(32)
#define NRD_CHIP0_SPI_MAX		U(511)
#if NRD_CHIP_COUNT > 1
#define NRD_CHIP1_SPI_MIN		U(512)
#define NRD_CHIP1_SPI_MAX		U(991)
#endif
#if NRD_CHIP_COUNT > 2
#define NRD_CHIP2_SPI_MIN		U(4096)
#define NRD_CHIP2_SPI_MAX		U(4575)
#endif
#if NRD_CHIP_COUNT > 3
#define NRD_CHIP3_SPI_MIN		U(4576)
#define NRD_CHIP3_SPI_MAX		U(5055)
#endif

#endif /* PLATFORM_DEF_H */

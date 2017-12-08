/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arm_def.h>
#include <arm_spm_def.h>
#include <board_arm_def.h>
#include <common_def.h>
#include <tzc400.h>
#include <utils_def.h>
#include <v2m_def.h>
#include "../fvp_def.h"

/* Required platform porting definitions */
#define PLATFORM_CORE_COUNT \
	(FVP_CLUSTER_COUNT * FVP_MAX_CPUS_PER_CLUSTER * FVP_MAX_PE_PER_CPU)

#define PLAT_NUM_PWR_DOMAINS		(FVP_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT) + 1

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT		FVP_CLUSTER_COUNT

#define PLAT_ARM_TRUSTED_ROM_BASE	0x00000000
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x04000000	/* 64 MB */

#define PLAT_ARM_TRUSTED_DRAM_BASE	0x06000000
#define PLAT_ARM_TRUSTED_DRAM_SIZE	0x02000000	/* 32 MB */

/* No SCP in FVP */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	ULL(0x0)

#define PLAT_ARM_DRAM2_SIZE		ULL(0x780000000)

/*
 * Load address of BL33 for this platform port
 */
#define PLAT_ARM_NS_IMAGE_OFFSET	(ARM_DRAM1_BASE + 0x8000000)


/*
 * PL011 related constants
 */
#define PLAT_ARM_BOOT_UART_BASE		V2M_IOFPGA_UART0_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	V2M_IOFPGA_UART0_CLK_IN_HZ

#define PLAT_ARM_BL31_RUN_UART_BASE		V2M_IOFPGA_UART1_BASE
#define PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ	V2M_IOFPGA_UART1_CLK_IN_HZ

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		V2M_IOFPGA_UART1_BASE
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	V2M_IOFPGA_UART1_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_BL31_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_TSP_UART_BASE		V2M_IOFPGA_UART2_BASE
#define PLAT_ARM_TSP_UART_CLK_IN_HZ	V2M_IOFPGA_UART2_CLK_IN_HZ

#define PLAT_FVP_SMMUV3_BASE		0x2b400000

/* CCI related constants */
#define PLAT_FVP_CCI400_BASE		0x2c090000
#define PLAT_FVP_CCI400_CLUS0_SL_PORT	3
#define PLAT_FVP_CCI400_CLUS1_SL_PORT	4

/* CCI-500/CCI-550 on Base platform */
#define PLAT_FVP_CCI5XX_BASE		0x2a000000
#define PLAT_FVP_CCI5XX_CLUS0_SL_PORT	5
#define PLAT_FVP_CCI5XX_CLUS1_SL_PORT	6

/* CCN related constants. Only CCN 502 is currently supported */
#define PLAT_ARM_CCN_BASE		0x2e000000
#define PLAT_ARM_CLUSTER_TO_CCN_ID_MAP	1, 5, 7, 11

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID		1

/* Mailbox base address */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE


/* TrustZone controller related constants
 *
 * Currently only filters 0 and 2 are connected on Base FVP.
 * Filter 0 : CPU clusters (no access to DRAM by default)
 * Filter 1 : not connected
 * Filter 2 : LCDs (access to VRAM allowed by default)
 * Filter 3 : not connected
 * Programming unconnected filters will have no effect at the
 * moment. These filter could, however, be connected in future.
 * So care should be taken not to configure the unused filters.
 *
 * Allow only non-secure access to all DRAM to supported devices.
 * Give access to the CPUs and Virtio. Some devices
 * would normally use the default ID so allow that too.
 */
#define PLAT_ARM_TZC_BASE		0x2a4a0000
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_DEFAULT)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_PCI)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_AP)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO_OLD))

/*
 * GIC related constants to cater for both GICv2 and GICv3 instances of an
 * FVP. They could be overriden at runtime in case the FVP implements the legacy
 * VE memory map.
 */
#define PLAT_ARM_GICD_BASE		BASE_GICD_BASE
#define PLAT_ARM_GICR_BASE		BASE_GICR_BASE
#define PLAT_ARM_GICC_BASE		BASE_GICC_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQS		ARM_G1S_IRQS,			\
					FVP_IRQ_TZ_WDOG,		\
					FVP_IRQ_SEC_SYS_TIMER

#define PLAT_ARM_G0_IRQS		ARM_G0_IRQS

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	ARM_G1S_IRQ_PROPS(grp), \
	INTR_PROP_DESC(FVP_IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(FVP_IRQ_SEC_SYS_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define PLAT_ARM_PRIVATE_SDEI_EVENTS	ARM_SDEI_PRIVATE_EVENTS
#define PLAT_ARM_SHARED_SDEI_EVENTS	ARM_SDEI_SHARED_EVENTS

#endif /* __PLATFORM_DEF_H__ */

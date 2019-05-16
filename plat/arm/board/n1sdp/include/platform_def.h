/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE			0x2A400000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		50000000

#define PLAT_ARM_RUN_UART_BASE		0x2A410000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	50000000

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		0x2A410000
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	50000000

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_DRAM2_BASE			ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE			ULL(0x780000000)

#if CSS_USE_SCMI_SDS_DRIVER
#define N1SDP_SCMI_PAYLOAD_BASE			0x45400000
#else
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	0x45400000
#endif

#define PLAT_ARM_TRUSTED_SRAM_SIZE		0x00080000	/* 512 KB */
#define PLAT_ARM_MAX_BL31_SIZE			0X20000


/*******************************************************************************
 * N1SDP topology related constants
 ******************************************************************************/
#define N1SDP_MAX_CPUS_PER_CLUSTER		2
#define PLAT_ARM_CLUSTER_COUNT			2
#define N1SDP_MAX_PE_PER_CPU			1

#define PLATFORM_CORE_COUNT			(PLAT_ARM_CLUSTER_COUNT *	\
						N1SDP_MAX_CPUS_PER_CLUSTER *	\
						N1SDP_MAX_PE_PER_CPU)

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL			ARM_PWR_LVL2

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#define PLAT_ARM_MMAP_ENTRIES			3
#define MAX_XLAT_TABLES				4

#define PLATFORM_STACK_SIZE			0x400

#define PLAT_ARM_NSTIMER_FRAME_ID		0
#define PLAT_CSS_MHU_BASE			0x45000000
#define PLAT_MHUV2_BASE				PLAT_CSS_MHU_BASE
#define PLAT_MAX_PWR_LVL			1

#define PLAT_ARM_G1S_IRQS			ARM_G1S_IRQS,			\
						CSS_IRQ_MHU
#define PLAT_ARM_G0_IRQS			ARM_G0_IRQS

#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)


#define N1SDP_DEVICE_BASE			(0x20000000)
#define N1SDP_DEVICE_SIZE			(0x30000000)
#define N1SDP_MAP_DEVICE			MAP_REGION_FLAT(	\
						N1SDP_DEVICE_BASE,	\
						N1SDP_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE			0x30000000
#define PLAT_ARM_GICC_BASE			0x2C000000
#define PLAT_ARM_GICR_BASE			0x300C0000

/* Platform ID address */
#define SSC_VERSION				(SSC_REG_BASE + SSC_VERSION_OFFSET)

/* Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_BASE			UL(0x2A480000)
#define SBSA_SECURE_WDOG_TIMEOUT		UL(100)

#endif /* PLATFORM_DEF_H */

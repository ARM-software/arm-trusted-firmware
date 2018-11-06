/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arm_def.h>
#include <board_css_def.h>
#include <css_def.h>

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


/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#define PLAT_ARM_MMAP_ENTRIES			3
#define MAX_XLAT_TABLES				4

#define PLATFORM_STACK_SIZE			0x400

#define PLAT_ARM_NSTIMER_FRAME_ID		0
#define PLAT_CSS_MHU_BASE			0x45000000
#define PLAT_MAX_PWR_LVL			1

#define PLAT_ARM_G1S_IRQS			ARM_G1S_IRQS,			\
						CSS_IRQ_MHU
#define PLAT_ARM_G0_IRQS			ARM_G0_IRQS

#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)


#define N1SDP_DEVICE_BASE			(0x20000000)
#define N1SDP_DEVICE_SIZE			(0x20000000)
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

#endif /* PLATFORM_DEF_H */

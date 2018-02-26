/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arm_def.h>
#include <board_arm_def.h>
#include <board_css_def.h>
#include <common_def.h>
#include <css_def.h>
#include <soc_css_def.h>

#define CSS_SGI_MAX_CORES_PER_CLUSTER	4

/* CPU topology */
#define PLAT_ARM_CLUSTER_COUNT		2
#define PLATFORM_CORE_COUNT		(PLAT_ARM_CLUSTER_COUNT *	\
					CSS_SGI_MAX_CORES_PER_CLUSTER)

#if ARM_BOARD_OPTIMISE_MEM

#if IMAGE_BL31 || IMAGE_BL32
# define PLAT_ARM_MMAP_ENTRIES		6
# define MAX_XLAT_TABLES		4
#else
# define PLAT_ARM_MMAP_ENTRIES		10
# define MAX_XLAT_TABLES		5
#endif

#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL1_RW_SIZE	0xA000
#else
# define PLAT_ARM_MAX_BL1_RW_SIZE	0x6000
#endif

#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		0x1D000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0xC000
#endif

#endif /* ARM_BOARD_OPTIMISE_MEM */

#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_CSS_MHU_BASE		0x45000000

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00080000	/* 512KB */

#define PLAT_MAX_PWR_LVL		1

#define PLAT_ARM_G1S_IRQS		ARM_G1S_IRQS,			\
					CSS_IRQ_MHU

#define PLAT_ARM_G0_IRQS		ARM_G0_IRQS

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define CSS_SGI_DEVICE_BASE	(0x20000000)
#define CSS_SGI_DEVICE_SIZE	(0x20000000)
#define CSS_SGI_MAP_DEVICE	MAP_REGION_FLAT(		\
					CSS_SGI_DEVICE_BASE,	\
					CSS_SGI_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	0x45400000
#define SGI_BOOT_CFG_ADDR		0x45410000
#define PLAT_CSS_PRIMARY_CPU_SHIFT		8
#define PLAT_CSS_PRIMARY_CPU_BIT_WIDTH		6

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		0x30000000
#define PLAT_ARM_GICC_BASE		0x2C000000
#define PLAT_ARM_GICR_BASE		0x300C0000

/* Platform ID address */
#define SSC_VERSION                     (SSC_REG_BASE + SSC_VERSION_OFFSET)
#ifndef __ASSEMBLY__
/* SSC_VERSION related accessors */
/* Returns the part number of the platform */
#define GET_SGI_PART_NUM                                       \
		GET_SSC_VERSION_PART_NUM(mmio_read_32(SSC_VERSION))
/* Returns the configuration number of the platform */
#define GET_SGI_CONFIG_NUM                                     \
		GET_SSC_VERSION_CONFIG(mmio_read_32(SSC_VERSION))
#endif /* __ASSEMBLY__ */

#endif /* __PLATFORM_DEF_H__ */

/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSS_DEF_H
#define CSS_DEF_H

#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/tzc400.h>

/*************************************************************************
 * Definitions common to all ARM Compute SubSystems (CSS)
 *************************************************************************/
#define NSROM_BASE			0x1f000000
#define NSROM_SIZE			0x00001000

/* Following covers CSS Peripherals excluding NSROM and NSRAM  */
#define CSS_DEVICE_BASE			0x20000000
#define CSS_DEVICE_SIZE			0x0e000000

/* System Security Control Registers */
#define SSC_REG_BASE			0x2a420000
#define SSC_GPRETN			(SSC_REG_BASE + 0x030)

/* System ID Registers Unit */
#define SID_REG_BASE			0x2a4a0000
#define SID_SYSTEM_ID_OFFSET		0x40
#define SID_SYSTEM_CFG_OFFSET		0x70
#define SID_NODE_ID_OFFSET		0x60
#define SID_CHIP_ID_MASK		0xFF
#define SID_MULTI_CHIP_MODE_MASK	0x100
#define SID_MULTI_CHIP_MODE_SHIFT	8

/* The slave_bootsecure controls access to GPU, DMC and CS. */
#define CSS_NIC400_SLAVE_BOOTSECURE	8

/* Interrupt handling constants */
#define CSS_IRQ_MHU			69
#define CSS_IRQ_GPU_SMMU_0		71
#define CSS_IRQ_TZC			80
#define CSS_IRQ_TZ_WDOG			86
#define CSS_IRQ_SEC_SYS_TIMER		91

/* MHU register offsets */
#define MHU_CPU_INTR_S_SET_OFFSET	0x308

/*
 * Define a list of Group 1 Secure interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the interrupts will be treated as
 * Group 0 interrupts.
 */
#define CSS_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(CSS_IRQ_MHU, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(CSS_IRQ_GPU_SMMU_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(CSS_IRQ_TZC, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(CSS_IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(CSS_IRQ_SEC_SYS_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

#if CSS_USE_SCMI_SDS_DRIVER
/* Memory region for shared data storage */
#define PLAT_ARM_SDS_MEM_BASE		ARM_SHARED_RAM_BASE
#define PLAT_ARM_SDS_MEM_SIZE_MAX	0xDC0 /* 3520 bytes */
/*
 * The SCMI Channel is placed right after the SDS region
 */
#define CSS_SCMI_PAYLOAD_BASE		(PLAT_ARM_SDS_MEM_BASE + PLAT_ARM_SDS_MEM_SIZE_MAX)
#define CSS_SCMI_MHU_DB_REG_OFF		MHU_CPU_INTR_S_SET_OFFSET

/* Trusted mailbox base address common to all CSS */
/* If SDS is present, then mailbox is at top of SRAM */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	(ARM_SHARED_RAM_BASE + ARM_SHARED_RAM_SIZE - 0x8)

/* Number of retries for SCP_RAM_READY flag */
#define CSS_SCP_READY_10US_RETRIES		1000000 /* Effective timeout of 10000 ms */

#else
/*
 * SCP <=> AP boot configuration
 *
 * The SCP/AP boot configuration is a 32-bit word located at a known offset from
 * the start of the Trusted SRAM.
 *
 * Note that the value stored at this address is only valid at boot time, before
 * the SCP_BL2 image is transferred to SCP.
 */
#define SCP_BOOT_CFG_ADDR		PLAT_CSS_SCP_COM_SHARED_MEM_BASE

/* Trusted mailbox base address common to all CSS */
/* If SDS is not present, then the mailbox is at the bottom of SRAM */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE

#endif /* CSS_USE_SCMI_SDS_DRIVER */

#define CSS_MAP_DEVICE			MAP_REGION_FLAT(		\
						CSS_DEVICE_BASE,	\
						CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define CSS_MAP_NSRAM			MAP_REGION_FLAT(		\
						NSRAM_BASE,	\
						NSRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_NS)

#if defined(IMAGE_BL2U)
#define CSS_MAP_SCP_BL2U		MAP_REGION_FLAT(		\
						SCP_BL2U_BASE,		\
						SCP_BL2U_LIMIT		\
							- SCP_BL2U_BASE,\
						MT_RW_DATA | MT_SECURE)
#endif

/* Platform ID address */
#define SSC_VERSION_OFFSET			0x040

#define SSC_VERSION_CONFIG_SHIFT		28
#define SSC_VERSION_MAJOR_REV_SHIFT		24
#define SSC_VERSION_MINOR_REV_SHIFT		20
#define SSC_VERSION_DESIGNER_ID_SHIFT		12
#define SSC_VERSION_PART_NUM_SHIFT		0x0
#define SSC_VERSION_CONFIG_MASK			0xf
#define SSC_VERSION_MAJOR_REV_MASK		0xf
#define SSC_VERSION_MINOR_REV_MASK		0xf
#define SSC_VERSION_DESIGNER_ID_MASK		0xff
#define SSC_VERSION_PART_NUM_MASK		0xfff

#define SID_SYSTEM_ID_PART_NUM_MASK		0xfff

/* SSC debug configuration registers */
#define SSC_DBGCFG_SET		0x14
#define SSC_DBGCFG_CLR		0x18

#define SPNIDEN_INT_CLR_SHIFT	4
#define SPNIDEN_SEL_SET_SHIFT	5
#define SPIDEN_INT_CLR_SHIFT	6
#define SPIDEN_SEL_SET_SHIFT	7

#ifndef __ASSEMBLER__

/* SSC_VERSION related accessors */

/* Returns the part number of the platform */
#define GET_SSC_VERSION_PART_NUM(val)				\
		(((val) >> SSC_VERSION_PART_NUM_SHIFT) &	\
		SSC_VERSION_PART_NUM_MASK)

/* Returns the configuration number of the platform */
#define GET_SSC_VERSION_CONFIG(val)				\
		(((val) >> SSC_VERSION_CONFIG_SHIFT) &		\
		SSC_VERSION_CONFIG_MASK)

#endif /* __ASSEMBLER__ */

/*************************************************************************
 * Required platform porting definitions common to all
 * ARM Compute SubSystems (CSS)
 ************************************************************************/

/*
 * The loading of SCP images(SCP_BL2 or SCP_BL2U) is done if there
 * respective base addresses are defined (i.e SCP_BL2_BASE, SCP_BL2U_BASE).
 * Hence, `CSS_LOAD_SCP_IMAGES` needs to be set to 1 if BL2 needs to load
 * an SCP_BL2/SCP_BL2U image.
 */
#if CSS_LOAD_SCP_IMAGES

#if ARM_BL31_IN_DRAM
#error "SCP_BL2 is not expected to be loaded by BL2 for ARM_BL31_IN_DRAM config"
#endif

/*
 * Load address of SCP_BL2 in CSS platform ports
 * SCP_BL2 is loaded to the same place as BL31 but it shouldn't overwrite BL1
 * rw data or BL2.  Once SCP_BL2 is transferred to the SCP, it is discarded and
 * BL31 is loaded over the top.
 */
#define SCP_BL2_BASE			(BL2_BASE - PLAT_CSS_MAX_SCP_BL2_SIZE)
#define SCP_BL2_LIMIT			BL2_BASE

#define SCP_BL2U_BASE			(BL2_BASE - PLAT_CSS_MAX_SCP_BL2U_SIZE)
#define SCP_BL2U_LIMIT			BL2_BASE
#endif /* CSS_LOAD_SCP_IMAGES */

/* Load address of Non-Secure Image for CSS platform ports */
#define PLAT_ARM_NS_IMAGE_BASE		U(0xE0000000)

/*
 * Parsing of CPU and Cluster states, as returned by 'Get CSS Power State' SCP
 * command
 */
#define CSS_CLUSTER_PWR_STATE_ON	0
#define CSS_CLUSTER_PWR_STATE_OFF	3

#define CSS_CPU_PWR_STATE_ON		1
#define CSS_CPU_PWR_STATE_OFF		0
#define CSS_CPU_PWR_STATE(state, n)	(((state) >> (n)) & 1)

#endif /* CSS_DEF_H */

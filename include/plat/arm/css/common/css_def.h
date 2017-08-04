/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CSS_DEF_H__
#define __CSS_DEF_H__

#include <arm_def.h>
#include <tzc400.h>

/*************************************************************************
 * Definitions common to all ARM Compute SubSystems (CSS)
 *************************************************************************/
#define NSROM_BASE			0x1f000000
#define NSROM_SIZE			0x00001000

/* Following covers CSS Peripherals excluding NSROM and NSRAM  */
#define CSS_DEVICE_BASE			0x20000000
#define CSS_DEVICE_SIZE			0x0e000000

#define NSRAM_BASE			0x2e000000
#define NSRAM_SIZE			0x00008000

/* System Security Control Registers */
#define SSC_REG_BASE			0x2a420000
#define SSC_GPRETN			(SSC_REG_BASE + 0x030)

/* The slave_bootsecure controls access to GPU, DMC and CS. */
#define CSS_NIC400_SLAVE_BOOTSECURE	8

/* Interrupt handling constants */
#define CSS_IRQ_MHU			69
#define CSS_IRQ_GPU_SMMU_0		71
#define CSS_IRQ_TZC			80
#define CSS_IRQ_TZ_WDOG			86
#define CSS_IRQ_SEC_SYS_TIMER		91

/*
 * Define a list of Group 1 Secure interrupts as per GICv3 terminology. On a
 * GICv2 system or mode, the interrupts will be treated as Group 0 interrupts.
 */
#define CSS_G1S_IRQS			CSS_IRQ_MHU,		\
					CSS_IRQ_GPU_SMMU_0,	\
					CSS_IRQ_TZC,		\
					CSS_IRQ_TZ_WDOG,	\
					CSS_IRQ_SEC_SYS_TIMER

/*
 * The lower Non-secure MHU channel is being used for SCMI for ARM Trusted
 * Firmware.
 * TODO: Move SCMI to Secure channel once the migration to SCMI in SCP is
 * complete.
 */
#define MHU_CPU_INTR_L_SET_OFFSET	0x108
#define MHU_CPU_INTR_H_SET_OFFSET	0x128
#define CSS_SCMI_PAYLOAD_BASE		(NSRAM_BASE + 0x500)
#define CSS_SCMI_MHU_DB_REG_OFF		MHU_CPU_INTR_L_SET_OFFSET

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

#define CSS_MAP_DEVICE			MAP_REGION_FLAT(		\
						CSS_DEVICE_BASE,	\
						CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define CSS_MAP_NSRAM			MAP_REGION_FLAT(		\
						NSRAM_BASE,	\
						NSRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

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

/* SSC debug configuration registers */
#define SSC_DBGCFG_SET		0x14
#define SSC_DBGCFG_CLR		0x18

#define SPIDEN_INT_CLR_SHIFT	6
#define SPIDEN_SEL_SET_SHIFT	7

#ifndef __ASSEMBLY__

/* SSC_VERSION related accessors */

/* Returns the part number of the platform */
#define GET_SSC_VERSION_PART_NUM(val)				\
		(((val) >> SSC_VERSION_PART_NUM_SHIFT) &	\
		SSC_VERSION_PART_NUM_MASK)

/* Returns the configuration number of the platform */
#define GET_SSC_VERSION_CONFIG(val)				\
		(((val) >> SSC_VERSION_CONFIG_SHIFT) &		\
		SSC_VERSION_CONFIG_MASK)

#endif /* __ASSEMBLY__ */

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
 * rw data.  Once SCP_BL2 is transferred to the SCP, it is discarded and BL31
 * is loaded over the top.
 */
#define SCP_BL2_BASE			(BL1_RW_BASE - PLAT_CSS_MAX_SCP_BL2_SIZE)
#define SCP_BL2_LIMIT			BL1_RW_BASE

#define SCP_BL2U_BASE			(BL1_RW_BASE - PLAT_CSS_MAX_SCP_BL2U_SIZE)
#define SCP_BL2U_LIMIT			BL1_RW_BASE
#endif /* CSS_LOAD_SCP_IMAGES */

/* Load address of Non-Secure Image for CSS platform ports */
#define PLAT_ARM_NS_IMAGE_OFFSET	0xE0000000

/* TZC related constants */
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT_ALL

/* Trusted mailbox base address common to all CSS */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE

/*
 * Parsing of CPU and Cluster states, as returned by 'Get CSS Power State' SCP
 * command
 */
#define CSS_CLUSTER_PWR_STATE_ON	0
#define CSS_CLUSTER_PWR_STATE_OFF	3

#define CSS_CPU_PWR_STATE_ON		1
#define CSS_CPU_PWR_STATE_OFF		0
#define CSS_CPU_PWR_STATE(state, n)	(((state) >> (n)) & 1)

#endif /* __CSS_DEF_H__ */

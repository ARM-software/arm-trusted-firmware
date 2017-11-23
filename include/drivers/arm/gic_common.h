/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GIC_COMMON_H__
#define __GIC_COMMON_H__

/*******************************************************************************
 * GIC Distributor interface general definitions
 ******************************************************************************/
/* Constants to categorise interrupts */
#define MIN_SGI_ID		0
#define MIN_SEC_SGI_ID		8
#define MIN_PPI_ID		16
#define MIN_SPI_ID		32
#define MAX_SPI_ID		1019

#define TOTAL_SPI_INTR_NUM	(MAX_SPI_ID - MIN_SPI_ID + 1)
#define TOTAL_PCPU_INTR_NUM	(MIN_SPI_ID - MIN_SGI_ID)

/* Mask for the priority field common to all GIC interfaces */
#define GIC_PRI_MASK			0xff

/* Mask for the configuration field common to all GIC interfaces */
#define GIC_CFG_MASK			0x3

/* Constant to indicate a spurious interrupt in all GIC versions */
#define GIC_SPURIOUS_INTERRUPT		1023

/* Interrupt configurations */
#define GIC_INTR_CFG_LEVEL		0
#define GIC_INTR_CFG_EDGE		1

/* Constants to categorise priorities */
#define GIC_HIGHEST_SEC_PRIORITY	0x0
#define GIC_LOWEST_SEC_PRIORITY		0x7f
#define GIC_HIGHEST_NS_PRIORITY		0x80
#define GIC_LOWEST_NS_PRIORITY		0xfe /* 0xff would disable all interrupts */

/*******************************************************************************
 * GIC Distributor interface register offsets that are common to GICv3 & GICv2
 ******************************************************************************/
#define GICD_CTLR		0x0
#define GICD_TYPER		0x4
#define GICD_IIDR		0x8
#define GICD_IGROUPR		0x80
#define GICD_ISENABLER		0x100
#define GICD_ICENABLER		0x180
#define GICD_ISPENDR		0x200
#define GICD_ICPENDR		0x280
#define GICD_ISACTIVER		0x300
#define GICD_ICACTIVER		0x380
#define GICD_IPRIORITYR		0x400
#define GICD_ICFGR		0xc00
#define GICD_NSACR		0xe00

/* GICD_CTLR bit definitions */
#define CTLR_ENABLE_G0_SHIFT		0
#define CTLR_ENABLE_G0_MASK		0x1
#define CTLR_ENABLE_G0_BIT		(1 << CTLR_ENABLE_G0_SHIFT)


/*******************************************************************************
 * GIC Distributor interface register constants that are common to GICv3 & GICv2
 ******************************************************************************/
#define PIDR2_ARCH_REV_SHIFT	4
#define PIDR2_ARCH_REV_MASK	0xf

/* GICv3 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV3		0x3
/* GICv2 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV2		0x2
/* GICv1 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV1		0x1

#define IGROUPR_SHIFT		5
#define ISENABLER_SHIFT		5
#define ICENABLER_SHIFT		ISENABLER_SHIFT
#define ISPENDR_SHIFT		5
#define ICPENDR_SHIFT		ISPENDR_SHIFT
#define ISACTIVER_SHIFT		5
#define ICACTIVER_SHIFT		ISACTIVER_SHIFT
#define IPRIORITYR_SHIFT	2
#define ITARGETSR_SHIFT		2
#define ICFGR_SHIFT		4
#define NSACR_SHIFT		4

/* GICD_TYPER shifts and masks */
#define TYPER_IT_LINES_NO_SHIFT	0
#define TYPER_IT_LINES_NO_MASK	0x1f

/* Value used to initialize Normal world interrupt priorities four at a time */
#define GICD_IPRIORITYR_DEF_VAL			\
	(GIC_HIGHEST_NS_PRIORITY	|	\
	(GIC_HIGHEST_NS_PRIORITY << 8)	|	\
	(GIC_HIGHEST_NS_PRIORITY << 16)	|	\
	(GIC_HIGHEST_NS_PRIORITY << 24))

#endif /* __GIC_COMMON_H__ */

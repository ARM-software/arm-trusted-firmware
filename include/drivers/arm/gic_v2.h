/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GIC_V2_H__
#define __GIC_V2_H__

/* The macros required here are additional to those in gic_common.h. */
#include <gic_common.h>

/******************************************************************************
 * THIS DRIVER IS DEPRECATED. For GICv2 systems, use the driver in gicv2.h
 * and for GICv3 systems, use the driver in gicv3.h.
 *****************************************************************************/
#if ERROR_DEPRECATED
#error " The legacy ARM GIC driver is deprecated."
#endif

#define GIC400_NUM_SPIS		U(480)
#define MAX_PPIS		U(14)
#define MAX_SGIS		U(16)


#define GRP0			U(0)
#define GRP1			U(1)
#define GIC_TARGET_CPU_MASK	U(0xff)

#define ENABLE_GRP0		(U(1) << 0)
#define ENABLE_GRP1		(U(1) << 1)

/* Distributor interface definitions */
#define GICD_ITARGETSR		U(0x800)
#define GICD_SGIR		U(0xF00)
#define GICD_CPENDSGIR		U(0xF10)
#define GICD_SPENDSGIR		U(0xF20)

#define CPENDSGIR_SHIFT		U(2)
#define SPENDSGIR_SHIFT		CPENDSGIR_SHIFT

/* GICD_TYPER bit definitions */
#define IT_LINES_NO_MASK	U(0x1f)

/* Physical CPU Interface registers */
#define GICC_CTLR		U(0x0)
#define GICC_PMR		U(0x4)
#define GICC_BPR		U(0x8)
#define GICC_IAR		U(0xC)
#define GICC_EOIR		U(0x10)
#define GICC_RPR		U(0x14)
#define GICC_HPPIR		U(0x18)
#define GICC_AHPPIR		U(0x28)
#define GICC_IIDR		U(0xFC)
#define GICC_DIR		U(0x1000)
#define GICC_PRIODROP           GICC_EOIR

/* Common CPU Interface definitions */
#define INT_ID_MASK		U(0x3ff)

/* GICC_CTLR bit definitions */
#define EOI_MODE_NS		(U(1) << 10)
#define EOI_MODE_S		(U(1) << 9)
#define IRQ_BYP_DIS_GRP1	(U(1) << 8)
#define FIQ_BYP_DIS_GRP1	(U(1) << 7)
#define IRQ_BYP_DIS_GRP0	(U(1) << 6)
#define FIQ_BYP_DIS_GRP0	(U(1) << 5)
#define CBPR			(U(1) << 4)
#define FIQ_EN			(U(1) << 3)
#define ACK_CTL			(U(1) << 2)

/* GICC_IIDR bit masks and shifts */
#define GICC_IIDR_PID_SHIFT	U(20)
#define GICC_IIDR_ARCH_SHIFT	U(16)
#define GICC_IIDR_REV_SHIFT	U(12)
#define GICC_IIDR_IMP_SHIFT	U(0)

#define GICC_IIDR_PID_MASK	U(0xfff)
#define GICC_IIDR_ARCH_MASK	U(0xf)
#define GICC_IIDR_REV_MASK	U(0xf)
#define GICC_IIDR_IMP_MASK	U(0xfff)

/* HYP view virtual CPU Interface registers */
#define GICH_CTL		U(0x0)
#define GICH_VTR		U(0x4)
#define GICH_ELRSR0		U(0x30)
#define GICH_ELRSR1		U(0x34)
#define GICH_APR0		U(0xF0)
#define GICH_LR_BASE		U(0x100)

/* Virtual CPU Interface registers */
#define GICV_CTL		U(0x0)
#define GICV_PRIMASK		U(0x4)
#define GICV_BP			U(0x8)
#define GICV_INTACK		U(0xC)
#define GICV_EOI		U(0x10)
#define GICV_RUNNINGPRI		U(0x14)
#define GICV_HIGHESTPEND	U(0x18)
#define GICV_DEACTIVATE		U(0x1000)

#ifndef __ASSEMBLY__

#include <mmio.h>
#include <stdint.h>

/*******************************************************************************
 * GIC Distributor function prototypes
 ******************************************************************************/

unsigned int gicd_read_igroupr(uintptr_t, unsigned int);
unsigned int gicd_read_isenabler(uintptr_t, unsigned int);
unsigned int gicd_read_icenabler(uintptr_t, unsigned int);
unsigned int gicd_read_ispendr(uintptr_t, unsigned int);
unsigned int gicd_read_icpendr(uintptr_t, unsigned int);
unsigned int gicd_read_isactiver(uintptr_t, unsigned int);
unsigned int gicd_read_icactiver(uintptr_t, unsigned int);
unsigned int gicd_read_ipriorityr(uintptr_t, unsigned int);
unsigned int gicd_read_itargetsr(uintptr_t, unsigned int);
unsigned int gicd_read_icfgr(uintptr_t, unsigned int);
unsigned int gicd_read_cpendsgir(uintptr_t, unsigned int);
unsigned int gicd_read_spendsgir(uintptr_t, unsigned int);
void gicd_write_igroupr(uintptr_t, unsigned int, unsigned int);
void gicd_write_isenabler(uintptr_t, unsigned int, unsigned int);
void gicd_write_icenabler(uintptr_t, unsigned int, unsigned int);
void gicd_write_ispendr(uintptr_t, unsigned int, unsigned int);
void gicd_write_icpendr(uintptr_t, unsigned int, unsigned int);
void gicd_write_isactiver(uintptr_t, unsigned int, unsigned int);
void gicd_write_icactiver(uintptr_t, unsigned int, unsigned int);
void gicd_write_ipriorityr(uintptr_t, unsigned int, unsigned int);
void gicd_write_itargetsr(uintptr_t, unsigned int, unsigned int);
void gicd_write_icfgr(uintptr_t, unsigned int, unsigned int);
void gicd_write_cpendsgir(uintptr_t, unsigned int, unsigned int);
void gicd_write_spendsgir(uintptr_t, unsigned int, unsigned int);
unsigned int gicd_get_igroupr(uintptr_t, unsigned int);
void gicd_set_igroupr(uintptr_t, unsigned int);
void gicd_clr_igroupr(uintptr_t, unsigned int);
void gicd_set_isenabler(uintptr_t, unsigned int);
void gicd_set_icenabler(uintptr_t, unsigned int);
void gicd_set_ispendr(uintptr_t, unsigned int);
void gicd_set_icpendr(uintptr_t, unsigned int);
void gicd_set_isactiver(uintptr_t, unsigned int);
void gicd_set_icactiver(uintptr_t, unsigned int);
void gicd_set_ipriorityr(uintptr_t, unsigned int, unsigned int);
void gicd_set_itargetsr(uintptr_t, unsigned int, unsigned int);


/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicd_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(uintptr_t base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_sgir(uintptr_t base)
{
	return mmio_read_32(base + GICD_SGIR);
}


/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicd_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

static inline void gicd_write_sgir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}


/*******************************************************************************
 * GIC CPU interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicc_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICC_CTLR);
}

static inline unsigned int gicc_read_pmr(uintptr_t base)
{
	return mmio_read_32(base + GICC_PMR);
}

static inline unsigned int gicc_read_BPR(uintptr_t base)
{
	return mmio_read_32(base + GICC_BPR);
}

static inline unsigned int gicc_read_IAR(uintptr_t base)
{
	return mmio_read_32(base + GICC_IAR);
}

static inline unsigned int gicc_read_EOIR(uintptr_t base)
{
	return mmio_read_32(base + GICC_EOIR);
}

static inline unsigned int gicc_read_hppir(uintptr_t base)
{
	return mmio_read_32(base + GICC_HPPIR);
}

static inline unsigned int gicc_read_ahppir(uintptr_t base)
{
	return mmio_read_32(base + GICC_AHPPIR);
}

static inline unsigned int gicc_read_dir(uintptr_t base)
{
	return mmio_read_32(base + GICC_DIR);
}

static inline unsigned int gicc_read_iidr(uintptr_t base)
{
	return mmio_read_32(base + GICC_IIDR);
}


/*******************************************************************************
 * GIC CPU interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicc_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_CTLR, val);
}

static inline void gicc_write_pmr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_PMR, val);
}

static inline void gicc_write_BPR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_BPR, val);
}


static inline void gicc_write_IAR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_IAR, val);
}

static inline void gicc_write_EOIR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_EOIR, val);
}

static inline void gicc_write_hppir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_HPPIR, val);
}

static inline void gicc_write_dir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_DIR, val);
}

/*******************************************************************************
 * Prototype of function to map an interrupt type to the interrupt line used to
 * signal it.
 ******************************************************************************/
uint32_t gicv2_interrupt_type_to_line(uint32_t cpuif_base, uint32_t type);

#endif /*__ASSEMBLY__*/

#endif /* __GIC_V2_H__ */

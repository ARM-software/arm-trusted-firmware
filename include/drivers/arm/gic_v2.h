/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GIC_V2_H__
#define __GIC_V2_H__


#define GIC400_NUM_SPIS		480
#define MAX_PPIS		14
#define MAX_SGIS		16

#define MIN_SGI_ID		0
#define MIN_PPI_ID		16
#define MIN_SPI_ID		32

#define GRP0			0
#define GRP1			1
#define GIC_PRI_MASK		0xff
#define GIC_HIGHEST_SEC_PRIORITY 0
#define GIC_LOWEST_SEC_PRIORITY	127
#define GIC_HIGHEST_NS_PRIORITY	128
#define GIC_LOWEST_NS_PRIORITY	254 /* 255 would disable an interrupt */
#define GIC_SPURIOUS_INTERRUPT	1023

#define ENABLE_GRP0		(1 << 0)
#define ENABLE_GRP1		(1 << 1)

/* Distributor interface definitions */
#define GICD_CTLR		0x0
#define GICD_TYPER		0x4
#define GICD_IGROUPR		0x80
#define GICD_ISENABLER		0x100
#define GICD_ICENABLER		0x180
#define GICD_ISPENDR		0x200
#define GICD_ICPENDR		0x280
#define GICD_ISACTIVER		0x300
#define GICD_ICACTIVER		0x380
#define GICD_IPRIORITYR		0x400
#define GICD_ITARGETSR		0x800
#define GICD_ICFGR		0xC00
#define GICD_SGIR		0xF00
#define GICD_CPENDSGIR		0xF10
#define GICD_SPENDSGIR		0xF20

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
#define CPENDSGIR_SHIFT		2
#define SPENDSGIR_SHIFT		CPENDSGIR_SHIFT

/* GICD_TYPER bit definitions */
#define IT_LINES_NO_MASK	0x1f

/* Physical CPU Interface registers */
#define GICC_CTLR		0x0
#define GICC_PMR		0x4
#define GICC_BPR		0x8
#define GICC_IAR		0xC
#define GICC_EOIR		0x10
#define GICC_RPR		0x14
#define GICC_HPPIR		0x18
#define GICC_AHPPIR		0x28
#define GICC_IIDR		0xFC
#define GICC_DIR		0x1000
#define GICC_PRIODROP           GICC_EOIR

/* GICC_CTLR bit definitions */
#define EOI_MODE_NS		(1 << 10)
#define EOI_MODE_S		(1 << 9)
#define IRQ_BYP_DIS_GRP1	(1 << 8)
#define FIQ_BYP_DIS_GRP1	(1 << 7)
#define IRQ_BYP_DIS_GRP0	(1 << 6)
#define FIQ_BYP_DIS_GRP0	(1 << 5)
#define CBPR			(1 << 4)
#define FIQ_EN			(1 << 3)
#define ACK_CTL			(1 << 2)

/* GICC_IIDR bit masks and shifts */
#define GICC_IIDR_PID_SHIFT	20
#define GICC_IIDR_ARCH_SHIFT	16
#define GICC_IIDR_REV_SHIFT	12
#define GICC_IIDR_IMP_SHIFT	0

#define GICC_IIDR_PID_MASK	0xfff
#define GICC_IIDR_ARCH_MASK	0xf
#define GICC_IIDR_REV_MASK	0xf
#define GICC_IIDR_IMP_MASK	0xfff

/* HYP view virtual CPU Interface registers */
#define GICH_CTL		0x0
#define GICH_VTR		0x4
#define GICH_ELRSR0		0x30
#define GICH_ELRSR1		0x34
#define GICH_APR0		0xF0
#define GICH_LR_BASE		0x100

/* Virtual CPU Interface registers */
#define GICV_CTL		0x0
#define GICV_PRIMASK		0x4
#define GICV_BP			0x8
#define GICV_INTACK		0xC
#define GICV_EOI		0x10
#define GICV_RUNNINGPRI		0x14
#define GICV_HIGHESTPEND	0x18
#define GICV_DEACTIVATE		0x1000

#ifndef __ASSEMBLY__

#include <mmio.h>


/*******************************************************************************
 * GIC Distributor function prototypes
 ******************************************************************************/

unsigned int gicd_read_igroupr(unsigned int, unsigned int);
unsigned int gicd_read_isenabler(unsigned int, unsigned int);
unsigned int gicd_read_icenabler(unsigned int, unsigned int);
unsigned int gicd_read_ispendr(unsigned int, unsigned int);
unsigned int gicd_read_icpendr(unsigned int, unsigned int);
unsigned int gicd_read_isactiver(unsigned int, unsigned int);
unsigned int gicd_read_icactiver(unsigned int, unsigned int);
unsigned int gicd_read_ipriorityr(unsigned int, unsigned int);
unsigned int gicd_read_itargetsr(unsigned int, unsigned int);
unsigned int gicd_read_icfgr(unsigned int, unsigned int);
unsigned int gicd_read_cpendsgir(unsigned int, unsigned int);
unsigned int gicd_read_spendsgir(unsigned int, unsigned int);
void gicd_write_igroupr(unsigned int, unsigned int, unsigned int);
void gicd_write_isenabler(unsigned int, unsigned int, unsigned int);
void gicd_write_icenabler(unsigned int, unsigned int, unsigned int);
void gicd_write_ispendr(unsigned int, unsigned int, unsigned int);
void gicd_write_icpendr(unsigned int, unsigned int, unsigned int);
void gicd_write_isactiver(unsigned int, unsigned int, unsigned int);
void gicd_write_icactiver(unsigned int, unsigned int, unsigned int);
void gicd_write_ipriorityr(unsigned int, unsigned int, unsigned int);
void gicd_write_itargetsr(unsigned int, unsigned int, unsigned int);
void gicd_write_icfgr(unsigned int, unsigned int, unsigned int);
void gicd_write_cpendsgir(unsigned int, unsigned int, unsigned int);
void gicd_write_spendsgir(unsigned int, unsigned int, unsigned int);
unsigned int gicd_get_igroupr(unsigned int, unsigned int);
void gicd_set_igroupr(unsigned int, unsigned int);
void gicd_clr_igroupr(unsigned int, unsigned int);
void gicd_set_isenabler(unsigned int, unsigned int);
void gicd_set_icenabler(unsigned int, unsigned int);
void gicd_set_ispendr(unsigned int, unsigned int);
void gicd_set_icpendr(unsigned int, unsigned int);
void gicd_set_isactiver(unsigned int, unsigned int);
void gicd_set_icactiver(unsigned int, unsigned int);
void gicd_set_ipriorityr(unsigned int, unsigned int, unsigned int);
void gicd_set_itargetsr(unsigned int, unsigned int, unsigned int);


/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicd_read_ctlr(unsigned int base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(unsigned int base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_sgir(unsigned int base)
{
	return mmio_read_32(base + GICD_SGIR);
}


/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicd_write_ctlr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

static inline void gicd_write_sgir(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}


/*******************************************************************************
 * GIC CPU interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicc_read_ctlr(unsigned int base)
{
	return mmio_read_32(base + GICC_CTLR);
}

static inline unsigned int gicc_read_pmr(unsigned int base)
{
	return mmio_read_32(base + GICC_PMR);
}

static inline unsigned int gicc_read_BPR(unsigned int base)
{
	return mmio_read_32(base + GICC_BPR);
}

static inline unsigned int gicc_read_IAR(unsigned int base)
{
	return mmio_read_32(base + GICC_IAR);
}

static inline unsigned int gicc_read_EOIR(unsigned int base)
{
	return mmio_read_32(base + GICC_EOIR);
}

static inline unsigned int gicc_read_hppir(unsigned int base)
{
	return mmio_read_32(base + GICC_HPPIR);
}

static inline unsigned int gicc_read_ahppir(unsigned int base)
{
	return mmio_read_32(base + GICC_AHPPIR);
}

static inline unsigned int gicc_read_dir(unsigned int base)
{
	return mmio_read_32(base + GICC_DIR);
}

static inline unsigned int gicc_read_iidr(unsigned int base)
{
	return mmio_read_32(base + GICC_IIDR);
}


/*******************************************************************************
 * GIC CPU interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicc_write_ctlr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_CTLR, val);
}

static inline void gicc_write_pmr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_PMR, val);
}

static inline void gicc_write_BPR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_BPR, val);
}


static inline void gicc_write_IAR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_IAR, val);
}

static inline void gicc_write_EOIR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_EOIR, val);
}

static inline void gicc_write_hppir(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_HPPIR, val);
}

static inline void gicc_write_dir(unsigned int base, unsigned int val)
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

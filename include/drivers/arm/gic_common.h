/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __GIC_COMMON_H__
#define __GIC_COMMON_H__

/*******************************************************************************
 * GIC Distributor interface general definitions
 ******************************************************************************/
/* Constants to categorise interrupts */
#define MIN_SGI_ID		0
#define MIN_PPI_ID		16
#define MIN_SPI_ID		32

/* Mask for the priority field common to all GIC interfaces */
#define GIC_PRI_MASK			0xff

/* Constant to indicate a spurious interrupt in all GIC versions */
#define GIC_SPURIOUS_INTERRUPT		1023

/* Constants to categorise priorities */
#define GIC_HIGHEST_SEC_PRIORITY	0
#define GIC_LOWEST_SEC_PRIORITY		127
#define GIC_HIGHEST_NS_PRIORITY		128
#define GIC_LOWEST_NS_PRIORITY		254 /* 255 would disable an interrupt */

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

#define IGROUPR_SHIFT		5
#define ISENABLER_SHIFT		5
#define ICENABLER_SHIFT		ISENABLER_SHIFT
#define ISPENDR_SHIFT		5
#define ICPENDR_SHIFT		ISPENDR_SHIFT
#define ISACTIVER_SHIFT		5
#define ICACTIVER_SHIFT		ISACTIVER_SHIFT
#define IPRIORITYR_SHIFT	2
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

#ifndef __ASSEMBLY__

#include <mmio.h>
#include <stdint.h>

/*******************************************************************************
 * GIC Distributor interface register accessors that are common to GICv3 & GICv2
 ******************************************************************************/
static inline unsigned int gicd_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(uintptr_t base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_iidr(uintptr_t base)
{
	return mmio_read_32(base + GICD_IIDR);
}

static inline void gicd_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

/*******************************************************************************
 * GIC Distributor function prototypes
 ******************************************************************************/
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id);
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id);
void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val);
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id);
void gicd_set_igroupr(uintptr_t base, unsigned int id);
void gicd_clr_igroupr(uintptr_t base, unsigned int id);
void gicd_set_isenabler(uintptr_t base, unsigned int id);
void gicd_set_icenabler(uintptr_t base, unsigned int id);
void gicd_set_ispendr(uintptr_t base, unsigned int id);
void gicd_set_icpendr(uintptr_t base, unsigned int id);
void gicd_set_isactiver(uintptr_t base, unsigned int id);
void gicd_set_icactiver(uintptr_t base, unsigned int id);


#endif /* __ASSEMBLY__ */
#endif /* __GIC_COMMON_H__ */

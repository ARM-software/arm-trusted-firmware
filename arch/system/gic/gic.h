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

#ifndef __GIC_H__
#define __GIC_H__

#define MAX_SPIS		480
#define MAX_PPIS		14
#define MAX_SGIS		16

#define GRP0			0
#define GRP1			1
#define MAX_PRI_VAL		0xff

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

/* GICv3 Re-distributor interface registers & shifts */
#define GICR_PCPUBASE_SHIFT	0x11
#define GICR_WAKER		0x14

/* GICR_WAKER bit definitions */
#define WAKER_CA		(1UL << 2)
#define WAKER_PS		(1UL << 1)

/* GICv3 ICC_SRE register bit definitions*/
#define ICC_SRE_EN		(1UL << 3)
#define ICC_SRE_SRE		(1UL << 0)

#ifndef __ASSEMBLY__

#include <gic_v2.h>
#include <gic_v3.h>

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

extern unsigned int gicd_read_igroupr(unsigned int, unsigned int);
extern unsigned int gicd_read_isenabler(unsigned int, unsigned int);
extern unsigned int gicd_read_icenabler(unsigned int, unsigned int);
extern unsigned int gicd_read_ispendr(unsigned int, unsigned int);
extern unsigned int gicd_read_icpendr(unsigned int, unsigned int);
extern unsigned int gicd_read_isactiver(unsigned int, unsigned int);
extern unsigned int gicd_read_icactiver(unsigned int, unsigned int);
extern unsigned int gicd_read_ipriorityr(unsigned int, unsigned int);
extern unsigned int gicd_read_itargetsr(unsigned int, unsigned int);
extern unsigned int gicd_read_icfgr(unsigned int, unsigned int);
extern unsigned int gicd_read_cpendsgir(unsigned int, unsigned int);
extern unsigned int gicd_read_spendsgir(unsigned int, unsigned int);
extern void gicd_write_igroupr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_isenabler(unsigned int, unsigned int, unsigned int);
extern void gicd_write_icenabler(unsigned int, unsigned int, unsigned int);
extern void gicd_write_ispendr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_icpendr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_isactiver(unsigned int, unsigned int, unsigned int);
extern void gicd_write_icactiver(unsigned int, unsigned int, unsigned int);
extern void gicd_write_ipriorityr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_itargetsr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_icfgr(unsigned int, unsigned int, unsigned int);
extern void gicd_write_cpendsgir(unsigned int, unsigned int, unsigned int);
extern void gicd_write_spendsgir(unsigned int, unsigned int, unsigned int);
extern unsigned int gicd_get_igroupr(unsigned int, unsigned int);
extern void gicd_set_igroupr(unsigned int, unsigned int);
extern void gicd_clr_igroupr(unsigned int, unsigned int);
extern void gicd_set_isenabler(unsigned int, unsigned int);
extern void gicd_set_icenabler(unsigned int, unsigned int);
extern void gicd_set_ispendr(unsigned int, unsigned int);
extern void gicd_set_icpendr(unsigned int, unsigned int);
extern void gicd_set_isactiver(unsigned int, unsigned int);
extern void gicd_set_icactiver(unsigned int, unsigned int);
extern void gicd_set_ipriorityr(unsigned int, unsigned int, unsigned int);
extern void gicd_set_itargetsr(unsigned int, unsigned int, unsigned int);

/* GICv3 functions */

extern unsigned int read_icc_sre_el1(void);
extern unsigned int read_icc_sre_el2(void);
extern unsigned int read_icc_sre_el3(void);
extern void write_icc_sre_el1(unsigned int);
extern void write_icc_sre_el2(unsigned int);
extern void write_icc_sre_el3(unsigned int);
extern void write_icc_pmr_el1(unsigned int);

#endif /*__ASSEMBLY__*/

#endif	/* __GIC_H__ */


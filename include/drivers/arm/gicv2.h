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

#ifndef __GICV2_H__
#define __GICV2_H__

/*******************************************************************************
 * GICv2 miscellaneous definitions
 ******************************************************************************/
/* Interrupt IDs reported by the HPPIR and IAR registers */
#define PENDING_G1_INTID	1022

/*******************************************************************************
 * GICv2 specific Distributor interface register offsets and constants.
 ******************************************************************************/
#define GICD_ITARGETSR		0x800
#define GICD_SGIR		0xF00
#define GICD_CPENDSGIR		0xF10
#define GICD_SPENDSGIR		0xF20
#define GICD_PIDR2_GICV2	0xFE8

#define ITARGETSR_SHIFT		2
#define GIC_TARGET_CPU_MASK	0xff

#define CPENDSGIR_SHIFT		2
#define SPENDSGIR_SHIFT		CPENDSGIR_SHIFT

/*******************************************************************************
 * GICv2 specific CPU interface register offsets and constants.
 ******************************************************************************/
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
#define GICC_PRIODROP		GICC_EOIR

/* GICC_CTLR bit definitions */
#define EOI_MODE_NS		(1 << 10)
#define EOI_MODE_S		(1 << 9)
#define IRQ_BYP_DIS_GRP1	(1 << 8)
#define FIQ_BYP_DIS_GRP1	(1 << 7)
#define IRQ_BYP_DIS_GRP0	(1 << 6)
#define FIQ_BYP_DIS_GRP0	(1 << 5)
#define CBPR			(1 << 4)
#define FIQ_EN_SHIFT		3
#define FIQ_EN_BIT		(1 << FIQ_EN_SHIFT)
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

/* GICD_CTLR bit definitions */
#define CTLR_ENABLE_G1_SHIFT		1
#define CTLR_ENABLE_G1_MASK		0x1
#define CTLR_ENABLE_G1_BIT		(1 << CTLR_ENABLE_G1_SHIFT)

/* Interrupt ID mask for HPPIR, AHPPIR, IAR and AIAR CPU Interface registers */
#define INT_ID_MASK		0x3ff

#ifndef __ASSEMBLY__

#include <stdint.h>

/*******************************************************************************
 * This structure describes some of the implementation defined attributes of
 * the GICv2 IP. It is used by the platform port to specify these attributes
 * in order to initialize the GICv2 driver. The attributes are described
 * below.
 *
 * 1. The 'gicd_base' field contains the base address of the Distributor
 *    interface programmer's view.
 *
 * 2. The 'gicc_base' field contains the base address of the CPU Interface
 *    programmer's view.
 *
 * 3. The 'g0_interrupt_array' field is a pointer to an array in which each
 *    entry corresponds to an ID of a Group 0 interrupt.
 *
 * 4. The 'g0_interrupt_num' field contains the number of entries in the
 *    'g0_interrupt_array'.
 ******************************************************************************/
typedef struct gicv2_driver_data {
	uintptr_t gicd_base;
	uintptr_t gicc_base;
	unsigned int g0_interrupt_num;
	const unsigned int *g0_interrupt_array;
} gicv2_driver_data_t;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void gicv2_driver_init(const gicv2_driver_data_t *plat_driver_data);
void gicv2_distif_init(void);
void gicv2_pcpu_distif_init(void);
void gicv2_cpuif_enable(void);
void gicv2_cpuif_disable(void);
unsigned int gicv2_is_fiq_enabled(void);
unsigned int gicv2_get_pending_interrupt_type(void);
unsigned int gicv2_get_pending_interrupt_id(void);
unsigned int gicv2_acknowledge_interrupt(void);
void gicv2_end_of_interrupt(unsigned int id);
unsigned int gicv2_get_interrupt_group(unsigned int id);

#endif /* __ASSEMBLY__ */
#endif /* __GICV2_H__ */

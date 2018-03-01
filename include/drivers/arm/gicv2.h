/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GICV2_H__
#define __GICV2_H__

/*******************************************************************************
 * GICv2 miscellaneous definitions
 ******************************************************************************/

/* Interrupt group definitions */
#define GICV2_INTR_GROUP0	0
#define GICV2_INTR_GROUP1	1

/* Interrupt IDs reported by the HPPIR and IAR registers */
#define PENDING_G1_INTID	1022

/* GICv2 can only target up to 8 PEs */
#define GICV2_MAX_TARGET_PE	8

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

#define SGIR_TGTLSTFLT_SHIFT	24
#define SGIR_TGTLSTFLT_MASK	0x3
#define SGIR_TGTLST_SHIFT	16
#define SGIR_TGTLST_MASK	0xff
#define SGIR_INTID_MASK		0xf

#define SGIR_TGT_SPECIFIC	0

#define GICV2_SGIR_VALUE(tgt_lst_flt, tgt, intid) \
	((((tgt_lst_flt) & SGIR_TGTLSTFLT_MASK) << SGIR_TGTLSTFLT_SHIFT) | \
	 (((tgt) & SGIR_TGTLST_MASK) << SGIR_TGTLST_SHIFT) | \
	 ((intid) & SGIR_INTID_MASK))

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

#include <interrupt_props.h>
#include <stdint.h>

/*******************************************************************************
 * This structure describes some of the implementation defined attributes of
 * the GICv2 IP. It is used by the platform port to specify these attributes
 * in order to initialize the GICv2 driver. The attributes are described
 * below.
 *
 * The 'gicd_base' field contains the base address of the Distributor interface
 * programmer's view.
 *
 * The 'gicc_base' field contains the base address of the CPU Interface
 * programmer's view.
 *
 * The 'g0_interrupt_array' field is a pointer to an array in which each entry
 * corresponds to an ID of a Group 0 interrupt. This field is ignored when
 * 'interrupt_props' field is used. This field is deprecated.
 *
 * The 'g0_interrupt_num' field contains the number of entries in the
 * 'g0_interrupt_array'. This field is ignored when 'interrupt_props' field is
 * used. This field is deprecated.
 *
 * The 'target_masks' is a pointer to an array containing 'target_masks_num'
 * elements. The GIC driver will populate the array with per-PE target mask to
 * use to when targeting interrupts.
 *
 * The 'interrupt_props' field is a pointer to an array that enumerates secure
 * interrupts and their properties. If this field is not NULL, both
 * 'g0_interrupt_array' and 'g1s_interrupt_array' fields are ignored.
 *
 * The 'interrupt_props_num' field contains the number of entries in the
 * 'interrupt_props' array. If this field is non-zero, 'g0_interrupt_num' is
 * ignored.
 ******************************************************************************/
typedef struct gicv2_driver_data {
	uintptr_t gicd_base;
	uintptr_t gicc_base;
#if !ERROR_DEPRECATED
	unsigned int g0_interrupt_num __deprecated;
	const unsigned int *g0_interrupt_array __deprecated;
#endif
	unsigned int *target_masks;
	unsigned int target_masks_num;
	const interrupt_prop_t *interrupt_props;
	unsigned int interrupt_props_num;
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
unsigned int gicv2_get_running_priority(void);
void gicv2_set_pe_target_mask(unsigned int proc_num);
unsigned int gicv2_get_interrupt_active(unsigned int id);
void gicv2_enable_interrupt(unsigned int id);
void gicv2_disable_interrupt(unsigned int id);
void gicv2_set_interrupt_priority(unsigned int id, unsigned int priority);
void gicv2_set_interrupt_type(unsigned int id, unsigned int type);
void gicv2_raise_sgi(int sgi_num, int proc_num);
void gicv2_set_spi_routing(unsigned int id, int proc_num);
void gicv2_set_interrupt_pending(unsigned int id);
void gicv2_clear_interrupt_pending(unsigned int id);
unsigned int gicv2_set_pmr(unsigned int mask);

#endif /* __ASSEMBLY__ */
#endif /* __GICV2_H__ */

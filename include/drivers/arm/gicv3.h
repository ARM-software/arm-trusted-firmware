/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GICV3_H__
#define __GICV3_H__

/*******************************************************************************
 * GICv3 miscellaneous definitions
 ******************************************************************************/
/* Interrupt group definitions */
#define INTR_GROUP1S		0
#define INTR_GROUP0		1
#define INTR_GROUP1NS		2

/* Interrupt IDs reported by the HPPIR and IAR registers */
#define PENDING_G1S_INTID	1020
#define PENDING_G1NS_INTID	1021

/* Constant to categorize LPI interrupt */
#define MIN_LPI_ID		8192

/* GICv3 can only target up to 16 PEs with SGI */
#define GICV3_MAX_SGI_TARGETS	16

/*******************************************************************************
 * GICv3 specific Distributor interface register offsets and constants.
 ******************************************************************************/
#define GICD_STATUSR		0x10
#define GICD_SETSPI_NSR		0x40
#define GICD_CLRSPI_NSR		0x48
#define GICD_SETSPI_SR		0x50
#define GICD_CLRSPI_SR		0x50
#define GICD_IGRPMODR		0xd00
/*
 * GICD_IROUTER<n> register is at 0x6000 + 8n, where n is the interrupt id and
 * n >= 32, making the effective offset as 0x6100.
 */
#define GICD_IROUTER		0x6000
#define GICD_PIDR2_GICV3	0xffe8

#define IGRPMODR_SHIFT		5

/* GICD_CTLR bit definitions */
#define CTLR_ENABLE_G1NS_SHIFT		1
#define CTLR_ENABLE_G1S_SHIFT		2
#define CTLR_ARE_S_SHIFT		4
#define CTLR_ARE_NS_SHIFT		5
#define CTLR_DS_SHIFT			6
#define CTLR_E1NWF_SHIFT		7
#define GICD_CTLR_RWP_SHIFT		31

#define CTLR_ENABLE_G1NS_MASK		0x1
#define CTLR_ENABLE_G1S_MASK		0x1
#define CTLR_ARE_S_MASK			0x1
#define CTLR_ARE_NS_MASK		0x1
#define CTLR_DS_MASK			0x1
#define CTLR_E1NWF_MASK			0x1
#define GICD_CTLR_RWP_MASK		0x1

#define CTLR_ENABLE_G1NS_BIT		(1 << CTLR_ENABLE_G1NS_SHIFT)
#define CTLR_ENABLE_G1S_BIT		(1 << CTLR_ENABLE_G1S_SHIFT)
#define CTLR_ARE_S_BIT			(1 << CTLR_ARE_S_SHIFT)
#define CTLR_ARE_NS_BIT			(1 << CTLR_ARE_NS_SHIFT)
#define CTLR_DS_BIT			(1 << CTLR_DS_SHIFT)
#define CTLR_E1NWF_BIT			(1 << CTLR_E1NWF_SHIFT)
#define GICD_CTLR_RWP_BIT		(1 << GICD_CTLR_RWP_SHIFT)

/* GICD_IROUTER shifts and masks */
#define IROUTER_SHIFT		0
#define IROUTER_IRM_SHIFT	31
#define IROUTER_IRM_MASK	0x1

#define GICV3_IRM_PE		0
#define GICV3_IRM_ANY		1

#define NUM_OF_DIST_REGS	30

/*******************************************************************************
 * GICv3 Re-distributor interface registers & constants
 ******************************************************************************/
#define GICR_PCPUBASE_SHIFT	0x11
#define GICR_SGIBASE_OFFSET	(1 << 0x10)	/* 64 KB */
#define GICR_CTLR		0x0
#define GICR_TYPER		0x08
#define GICR_WAKER		0x14
#define GICR_PROPBASER		0x70
#define GICR_PENDBASER		0x78
#define GICR_IGROUPR0		(GICR_SGIBASE_OFFSET + 0x80)
#define GICR_ISENABLER0		(GICR_SGIBASE_OFFSET + 0x100)
#define GICR_ICENABLER0		(GICR_SGIBASE_OFFSET + 0x180)
#define GICR_ISPENDR0		(GICR_SGIBASE_OFFSET + 0x200)
#define GICR_ICPENDR0		(GICR_SGIBASE_OFFSET + 0x280)
#define GICR_ISACTIVER0		(GICR_SGIBASE_OFFSET + 0x300)
#define GICR_ICACTIVER0		(GICR_SGIBASE_OFFSET + 0x380)
#define GICR_IPRIORITYR		(GICR_SGIBASE_OFFSET + 0x400)
#define GICR_ICFGR0		(GICR_SGIBASE_OFFSET + 0xc00)
#define GICR_ICFGR1		(GICR_SGIBASE_OFFSET + 0xc04)
#define GICR_IGRPMODR0		(GICR_SGIBASE_OFFSET + 0xd00)
#define GICR_NSACR		(GICR_SGIBASE_OFFSET + 0xe00)

/* GICR_CTLR bit definitions */
#define GICR_CTLR_UWP_SHIFT	31
#define GICR_CTLR_UWP_MASK	0x1
#define GICR_CTLR_UWP_BIT	(1U << GICR_CTLR_UWP_SHIFT)
#define GICR_CTLR_RWP_SHIFT	3
#define GICR_CTLR_RWP_MASK	0x1
#define GICR_CTLR_RWP_BIT	(1U << GICR_CTLR_RWP_SHIFT)
#define GICR_CTLR_EN_LPIS_BIT	(1U << 0)

/* GICR_WAKER bit definitions */
#define WAKER_CA_SHIFT		2
#define WAKER_PS_SHIFT		1

#define WAKER_CA_MASK		0x1
#define WAKER_PS_MASK		0x1

#define WAKER_CA_BIT		(1 << WAKER_CA_SHIFT)
#define WAKER_PS_BIT		(1 << WAKER_PS_SHIFT)

/* GICR_TYPER bit definitions */
#define TYPER_AFF_VAL_SHIFT	32
#define TYPER_PROC_NUM_SHIFT	8
#define TYPER_LAST_SHIFT	4

#define TYPER_AFF_VAL_MASK	0xffffffff
#define TYPER_PROC_NUM_MASK	0xffff
#define TYPER_LAST_MASK		0x1

#define TYPER_LAST_BIT		(1 << TYPER_LAST_SHIFT)

#define NUM_OF_REDIST_REGS	30

/*******************************************************************************
 * GICv3 CPU interface registers & constants
 ******************************************************************************/
/* ICC_SRE bit definitions*/
#define ICC_SRE_EN_BIT		(1 << 3)
#define ICC_SRE_DIB_BIT		(1 << 2)
#define ICC_SRE_DFB_BIT		(1 << 1)
#define ICC_SRE_SRE_BIT		(1 << 0)

/* ICC_IGRPEN1_EL3 bit definitions */
#define IGRPEN1_EL3_ENABLE_G1NS_SHIFT	0
#define IGRPEN1_EL3_ENABLE_G1S_SHIFT	1

#define IGRPEN1_EL3_ENABLE_G1NS_BIT	(1 << IGRPEN1_EL3_ENABLE_G1NS_SHIFT)
#define IGRPEN1_EL3_ENABLE_G1S_BIT	(1 << IGRPEN1_EL3_ENABLE_G1S_SHIFT)

/* ICC_IGRPEN0_EL1 bit definitions */
#define IGRPEN1_EL1_ENABLE_G0_SHIFT	0
#define IGRPEN1_EL1_ENABLE_G0_BIT	(1 << IGRPEN1_EL1_ENABLE_G0_SHIFT)

/* ICC_HPPIR0_EL1 bit definitions */
#define HPPIR0_EL1_INTID_SHIFT		0
#define HPPIR0_EL1_INTID_MASK		0xffffff

/* ICC_HPPIR1_EL1 bit definitions */
#define HPPIR1_EL1_INTID_SHIFT		0
#define HPPIR1_EL1_INTID_MASK		0xffffff

/* ICC_IAR0_EL1 bit definitions */
#define IAR0_EL1_INTID_SHIFT		0
#define IAR0_EL1_INTID_MASK		0xffffff

/* ICC_IAR1_EL1 bit definitions */
#define IAR1_EL1_INTID_SHIFT		0
#define IAR1_EL1_INTID_MASK		0xffffff

/* ICC SGI macros */
#define SGIR_TGT_MASK			0xffff
#define SGIR_AFF1_SHIFT			16
#define SGIR_INTID_SHIFT		24
#define SGIR_INTID_MASK			0xf
#define SGIR_AFF2_SHIFT			32
#define SGIR_IRM_SHIFT			40
#define SGIR_IRM_MASK			0x1
#define SGIR_AFF3_SHIFT			48
#define SGIR_AFF_MASK			0xf

#define SGIR_IRM_TO_AFF			0

#define GICV3_SGIR_VALUE(aff3, aff2, aff1, intid, irm, tgt) \
	((((uint64_t) (aff3) & SGIR_AFF_MASK) << SGIR_AFF3_SHIFT) | \
	 (((uint64_t) (irm) & SGIR_IRM_MASK) << SGIR_IRM_SHIFT) | \
	 (((uint64_t) (aff2) & SGIR_AFF_MASK) << SGIR_AFF2_SHIFT) | \
	 (((intid) & SGIR_INTID_MASK) << SGIR_INTID_SHIFT) | \
	 (((aff1) & SGIR_AFF_MASK) << SGIR_AFF1_SHIFT) | \
	 ((tgt) & SGIR_TGT_MASK))

/*****************************************************************************
 * GICv3 ITS registers and constants
 *****************************************************************************/

#define GITS_CTLR			0x0
#define GITS_IIDR			0x4
#define GITS_TYPER			0x8
#define GITS_CBASER			0x80
#define GITS_CWRITER			0x88
#define GITS_CREADR			0x90
#define GITS_BASER			0x100

/* GITS_CTLR bit definitions */
#define GITS_CTLR_ENABLED_BIT		1
#define GITS_CTLR_QUIESCENT_SHIFT	31
#define GITS_CTLR_QUIESCENT_BIT		(1U << GITS_CTLR_QUIESCENT_SHIFT)

#ifndef __ASSEMBLY__

#include <gic_common.h>
#include <interrupt_props.h>
#include <stdint.h>
#include <types.h>
#include <utils_def.h>

#define gicv3_is_intr_id_special_identifier(id)	\
	(((id) >= PENDING_G1S_INTID) && ((id) <= GIC_SPURIOUS_INTERRUPT))

/*******************************************************************************
 * Helper GICv3 macros for SEL1
 ******************************************************************************/
#define gicv3_acknowledge_interrupt_sel1()	read_icc_iar1_el1() &\
							IAR1_EL1_INTID_MASK
#define gicv3_get_pending_interrupt_id_sel1()	read_icc_hppir1_el1() &\
							HPPIR1_EL1_INTID_MASK
#define gicv3_end_of_interrupt_sel1(id)		write_icc_eoir1_el1(id)


/*******************************************************************************
 * Helper GICv3 macros for EL3
 ******************************************************************************/
#define gicv3_acknowledge_interrupt()		read_icc_iar0_el1() &\
							IAR0_EL1_INTID_MASK
#define gicv3_end_of_interrupt(id)		write_icc_eoir0_el1(id)

/*
 * This macro returns the total number of GICD registers corresponding to
 * the name.
 */
#define GICD_NUM_REGS(reg_name)	\
	DIV_ROUND_UP_2EVAL(TOTAL_SPI_INTR_NUM, (1 << reg_name ## _SHIFT))

#define GICR_NUM_REGS(reg_name)	\
	DIV_ROUND_UP_2EVAL(TOTAL_PCPU_INTR_NUM, (1 << reg_name ## _SHIFT))

/* Interrupt ID mask for HPPIR, AHPPIR, IAR and AIAR CPU Interface registers */
#define INT_ID_MASK	0xffffff

/*******************************************************************************
 * This structure describes some of the implementation defined attributes of the
 * GICv3 IP. It is used by the platform port to specify these attributes in order
 * to initialise the GICV3 driver. The attributes are described below.
 *
 * The 'gicd_base' field contains the base address of the Distributor interface
 * programmer's view.
 *
 * The 'gicr_base' field contains the base address of the Re-distributor
 * interface programmer's view.
 *
 * The 'g0_interrupt_array' field is a pointer to an array in which each entry
 * corresponds to an ID of a Group 0 interrupt. This field is ignored when
 * 'interrupt_props' field is used. This field is deprecated.
 *
 * The 'g0_interrupt_num' field contains the number of entries in the
 * 'g0_interrupt_array'. This field is ignored when 'interrupt_props' field is
 * used. This field is deprecated.
 *
 * The 'g1s_interrupt_array' field is a pointer to an array in which each entry
 * corresponds to an ID of a Group 1 interrupt. This field is ignored when
 * 'interrupt_props' field is used. This field is deprecated.
 *
 * The 'g1s_interrupt_num' field contains the number of entries in the
 * 'g1s_interrupt_array'. This field must be 0 if 'interrupt_props' field is
 * used. This field is ignored when 'interrupt_props' field is used. This field
 * is deprecated.
 *
 * The 'interrupt_props' field is a pointer to an array that enumerates secure
 * interrupts and their properties. If this field is not NULL, both
 * 'g0_interrupt_array' and 'g1s_interrupt_array' fields are ignored.
 *
 * The 'interrupt_props_num' field contains the number of entries in the
 * 'interrupt_props' array. If this field is non-zero, both 'g0_interrupt_num'
 * and 'g1s_interrupt_num' are ignored.
 *
 * The 'rdistif_num' field contains the number of Redistributor interfaces the
 * GIC implements. This is equal to the number of CPUs or CPU interfaces
 * instantiated in the GIC.
 *
 * The 'rdistif_base_addrs' field is a pointer to an array that has an entry for
 * storing the base address of the Redistributor interface frame of each CPU in
 * the system. The size of the array = 'rdistif_num'. The base addresses are
 * detected during driver initialisation.
 *
 * The 'mpidr_to_core_pos' field is a pointer to a hash function which the
 * driver will use to convert an MPIDR value to a linear core index. This index
 * will be used for accessing the 'rdistif_base_addrs' array. This is an
 * optional field. A GICv3 implementation maps each MPIDR to a linear core index
 * as well. This mapping can be found by reading the "Affinity Value" and
 * "Processor Number" fields in the GICR_TYPER. It is IMP. DEF. if the
 * "Processor Numbers" are suitable to index into an array to access core
 * specific information. If this not the case, the platform port must provide a
 * hash function. Otherwise, the "Processor Number" field will be used to access
 * the array elements.
 ******************************************************************************/
typedef unsigned int (*mpidr_hash_fn)(u_register_t mpidr);

typedef struct gicv3_driver_data {
	uintptr_t gicd_base;
	uintptr_t gicr_base;
#if !ERROR_DEPRECATED
	unsigned int g0_interrupt_num;
	unsigned int g1s_interrupt_num;
	const unsigned int *g0_interrupt_array;
	const unsigned int *g1s_interrupt_array;
#endif
	const interrupt_prop_t *interrupt_props;
	unsigned int interrupt_props_num;
	unsigned int rdistif_num;
	uintptr_t *rdistif_base_addrs;
	mpidr_hash_fn mpidr_to_core_pos;
} gicv3_driver_data_t;

typedef struct gicv3_redist_ctx {
	/* 64 bits registers */
	uint64_t gicr_propbaser;
	uint64_t gicr_pendbaser;

	/* 32 bits registers */
	uint32_t gicr_ctlr;
	uint32_t gicr_igroupr0;
	uint32_t gicr_isenabler0;
	uint32_t gicr_ispendr0;
	uint32_t gicr_isactiver0;
	uint32_t gicr_ipriorityr[GICR_NUM_REGS(IPRIORITYR)];
	uint32_t gicr_icfgr0;
	uint32_t gicr_icfgr1;
	uint32_t gicr_igrpmodr0;
	uint32_t gicr_nsacr;
} gicv3_redist_ctx_t;

typedef struct gicv3_dist_ctx {
	/* 64 bits registers */
	uint64_t gicd_irouter[TOTAL_SPI_INTR_NUM];

	/* 32 bits registers */
	uint32_t gicd_ctlr;
	uint32_t gicd_igroupr[GICD_NUM_REGS(IGROUPR)];
	uint32_t gicd_isenabler[GICD_NUM_REGS(ISENABLER)];
	uint32_t gicd_ispendr[GICD_NUM_REGS(ISPENDR)];
	uint32_t gicd_isactiver[GICD_NUM_REGS(ISACTIVER)];
	uint32_t gicd_ipriorityr[GICD_NUM_REGS(IPRIORITYR)];
	uint32_t gicd_icfgr[GICD_NUM_REGS(ICFGR)];
	uint32_t gicd_igrpmodr[GICD_NUM_REGS(IGRPMODR)];
	uint32_t gicd_nsacr[GICD_NUM_REGS(NSACR)];
} gicv3_dist_ctx_t;

typedef struct gicv3_its_ctx {
	/* 64 bits registers */
	uint64_t gits_cbaser;
	uint64_t gits_cwriter;
	uint64_t gits_baser[8];

	/* 32 bits registers */
	uint32_t gits_ctlr;
} gicv3_its_ctx_t;

/*******************************************************************************
 * GICv3 EL3 driver API
 ******************************************************************************/
void gicv3_driver_init(const gicv3_driver_data_t *plat_driver_data);
void gicv3_distif_init(void);
void gicv3_rdistif_init(unsigned int proc_num);
void gicv3_rdistif_on(unsigned int proc_num);
void gicv3_rdistif_off(unsigned int proc_num);
void gicv3_cpuif_enable(unsigned int proc_num);
void gicv3_cpuif_disable(unsigned int proc_num);
unsigned int gicv3_get_pending_interrupt_type(void);
unsigned int gicv3_get_pending_interrupt_id(void);
unsigned int gicv3_get_interrupt_type(unsigned int id,
					  unsigned int proc_num);
void gicv3_distif_init_restore(const gicv3_dist_ctx_t * const dist_ctx);
void gicv3_distif_save(gicv3_dist_ctx_t * const dist_ctx);
/*
 * gicv3_distif_post_restore and gicv3_distif_pre_save must be implemented if
 * gicv3_distif_save and gicv3_rdistif_init_restore are used. If no
 * implementation-defined sequence is needed at these steps, an empty function
 * can be provided.
 */
void gicv3_distif_post_restore(unsigned int proc_num);
void gicv3_distif_pre_save(unsigned int proc_num);
void gicv3_rdistif_init_restore(unsigned int proc_num, const gicv3_redist_ctx_t * const rdist_ctx);
void gicv3_rdistif_save(unsigned int proc_num, gicv3_redist_ctx_t * const rdist_ctx);
void gicv3_its_save_disable(uintptr_t gits_base, gicv3_its_ctx_t * const its_ctx);
void gicv3_its_restore(uintptr_t gits_base, const gicv3_its_ctx_t * const its_ctx);

unsigned int gicv3_get_running_priority(void);
unsigned int gicv3_get_interrupt_active(unsigned int id, unsigned int proc_num);
void gicv3_enable_interrupt(unsigned int id, unsigned int proc_num);
void gicv3_disable_interrupt(unsigned int id, unsigned int proc_num);
void gicv3_set_interrupt_priority(unsigned int id, unsigned int proc_num,
		unsigned int priority);
void gicv3_set_interrupt_type(unsigned int id, unsigned int proc_num,
		unsigned int group);
void gicv3_raise_secure_g0_sgi(int sgi_num, u_register_t target);
void gicv3_set_spi_routing(unsigned int id, unsigned int irm,
		u_register_t mpidr);
void gicv3_set_interrupt_pending(unsigned int id, unsigned int proc_num);
void gicv3_clear_interrupt_pending(unsigned int id, unsigned int proc_num);
unsigned int gicv3_set_pmr(unsigned int mask);

#endif /* __ASSEMBLY__ */
#endif /* __GICV3_H__ */

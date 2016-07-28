/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#define IROUTER_IRM_SHIFT	31
#define IROUTER_IRM_MASK	0x1

/*******************************************************************************
 * GICv3 Re-distributor interface registers & constants
 ******************************************************************************/
#define GICR_PCPUBASE_SHIFT	0x11
#define GICR_SGIBASE_OFFSET	(1 << 0x10)	/* 64 KB */
#define GICR_CTLR		0x0
#define GICR_TYPER		0x08
#define GICR_WAKER		0x14
#define GICR_IGROUPR0		(GICR_SGIBASE_OFFSET + 0x80)
#define GICR_ISENABLER0		(GICR_SGIBASE_OFFSET + 0x100)
#define GICR_ICENABLER0		(GICR_SGIBASE_OFFSET + 0x180)
#define GICR_IPRIORITYR		(GICR_SGIBASE_OFFSET + 0x400)
#define GICR_ICFGR0		(GICR_SGIBASE_OFFSET + 0xc00)
#define GICR_ICFGR1		(GICR_SGIBASE_OFFSET + 0xc04)
#define GICR_IGRPMODR0		(GICR_SGIBASE_OFFSET + 0xd00)

/* GICR_CTLR bit definitions */
#define GICR_CTLR_RWP_SHIFT	3
#define GICR_CTLR_RWP_MASK	0x1
#define GICR_CTLR_RWP_BIT	(1 << GICR_CTLR_RWP_SHIFT)

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

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <types.h>

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

/*******************************************************************************
 * This structure describes some of the implementation defined attributes of the
 * GICv3 IP. It is used by the platform port to specify these attributes in order
 * to initialise the GICV3 driver. The attributes are described below.
 *
 * 1. The 'gicd_base' field contains the base address of the Distributor
 *    interface programmer's view.
 *
 * 2. The 'gicr_base' field contains the base address of the Re-distributor
 *    interface programmer's view.
 *
 * 3. The 'g0_interrupt_array' field is a ponter to an array in which each
 *    entry corresponds to an ID of a Group 0 interrupt.
 *
 * 4. The 'g0_interrupt_num' field contains the number of entries in the
 *    'g0_interrupt_array'.
 *
 * 5. The 'g1s_interrupt_array' field is a ponter to an array in which each
 *    entry corresponds to an ID of a Group 1 interrupt.
 *
 * 6. The 'g1s_interrupt_num' field contains the number of entries in the
 *    'g1s_interrupt_array'.
 *
 * 7. The 'rdistif_num' field contains the number of Redistributor interfaces
 *    the GIC implements. This is equal to the number of CPUs or CPU interfaces
 *    instantiated in the GIC.
 *
 * 8. The 'rdistif_base_addrs' field is a pointer to an array that has an entry
 *    for storing the base address of the Redistributor interface frame of each
 *    CPU in the system. The size of the array = 'rdistif_num'. The base
 *    addresses are detected during driver initialisation.
 *
 * 9. The 'mpidr_to_core_pos' field is a pointer to a hash function which the
 *    driver will use to convert an MPIDR value to a linear core index. This
 *    index will be used for accessing the 'rdistif_base_addrs' array. This is
 *    an optional field. A GICv3 implementation maps each MPIDR to a linear core
 *    index as well. This mapping can be found by reading the "Affinity Value"
 *    and "Processor Number" fields in the GICR_TYPER. It is IMP. DEF. if the
 *    "Processor Numbers" are suitable to index into an array to access core
 *    specific information. If this not the case, the platform port must provide
 *    a hash function. Otherwise, the "Processor Number" field will be used to
 *    access the array elements.
 ******************************************************************************/
typedef unsigned int (*mpidr_hash_fn)(u_register_t mpidr);

typedef struct gicv3_driver_data {
	uintptr_t gicd_base;
	uintptr_t gicr_base;
	unsigned int g0_interrupt_num;
	unsigned int g1s_interrupt_num;
	const unsigned int *g0_interrupt_array;
	const unsigned int *g1s_interrupt_array;
	unsigned int rdistif_num;
	uintptr_t *rdistif_base_addrs;
	mpidr_hash_fn mpidr_to_core_pos;
} gicv3_driver_data_t;

/*******************************************************************************
 * GICv3 EL3 driver API
 ******************************************************************************/
void gicv3_driver_init(const gicv3_driver_data_t *plat_driver_data);
void gicv3_distif_init(void);
void gicv3_rdistif_init(unsigned int proc_num);
void gicv3_cpuif_enable(unsigned int proc_num);
void gicv3_cpuif_disable(unsigned int proc_num);
unsigned int gicv3_get_pending_interrupt_type(void);
unsigned int gicv3_get_pending_interrupt_id(void);
unsigned int gicv3_get_interrupt_type(unsigned int id,
					  unsigned int proc_num);


#endif /* __ASSEMBLY__ */
#endif /* __GICV3_H__ */

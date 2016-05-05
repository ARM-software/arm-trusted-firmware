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

#ifndef __GICV3_PRIVATE_H__
#define __GICV3_PRIVATE_H__

#include <gicv3.h>
#include <mmio.h>
#include <stdint.h>

/*******************************************************************************
 * GICv3 private macro definitions
 ******************************************************************************/

/* Constants to indicate the status of the RWP bit */
#define RWP_TRUE		1
#define RWP_FALSE		0

/*
 * Macro to wait for updates to :
 * GICD_CTLR[2:0] - the Group Enables
 * GICD_CTLR[5:4] - the ARE bits
 * GICD_ICENABLERn - the clearing of enable state for SPIs
 */
#define gicd_wait_for_pending_write(gicd_base)			\
	do {							\
		;						\
	} while (gicd_read_ctlr(gicd_base) & GICD_CTLR_RWP_BIT)

/*
 * Macro to convert an mpidr to a value suitable for programming into a
 * GICD_IROUTER. Bits[31:24] in the MPIDR are cleared as they are not relevant
 * to GICv3.
 */
#define gicd_irouter_val_from_mpidr(mpidr, irm)		\
	((mpidr & ~(0xff << 24)) |			\
	 (irm & IROUTER_IRM_MASK) << IROUTER_IRM_SHIFT)

/*
 * Macro to wait for updates to :
 * GICR_ICENABLER0
 * GICR_CTLR.DPG1S
 * GICR_CTLR.DPG1NS
 * GICR_CTLR.DPG0
 */
#define gicr_wait_for_pending_write(gicr_base)			\
	do {							\
		;						\
	} while (gicr_read_ctlr(gicr_base) & GICR_CTLR_RWP_BIT)

/*
 * Macro to convert a GICR_TYPER affinity value into a MPIDR value. Bits[31:24]
 * are zeroes.
 */
#ifdef AARCH32
#define mpidr_from_gicr_typer(typer_val)	(((typer_val) >> 32) & 0xffffff)
#else
#define mpidr_from_gicr_typer(typer_val)				 \
	(((((typer_val) >> 56) & MPIDR_AFFLVL_MASK) << MPIDR_AFF3_SHIFT) | \
	 (((typer_val) >> 32) & 0xffffff))
#endif

/*******************************************************************************
 * Private GICv3 function prototypes for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id);
unsigned int gicr_read_ipriorityr(uintptr_t base, unsigned int id);
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val);
void gicr_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val);

/*******************************************************************************
 * Private GICv3 function prototypes for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id);
unsigned int gicr_get_igrpmodr0(uintptr_t base, unsigned int id);
unsigned int gicr_get_igroupr0(uintptr_t base, unsigned int id);
void gicd_set_igrpmodr(uintptr_t base, unsigned int id);
void gicr_set_igrpmodr0(uintptr_t base, unsigned int id);
void gicr_set_isenabler0(uintptr_t base, unsigned int id);
void gicr_set_igroupr0(uintptr_t base, unsigned int id);
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id);
void gicr_clr_igrpmodr0(uintptr_t base, unsigned int id);
void gicr_clr_igroupr0(uintptr_t base, unsigned int id);
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri);

/*******************************************************************************
 * Private GICv3 helper function prototypes
 ******************************************************************************/
void gicv3_spis_configure_defaults(uintptr_t gicd_base);
void gicv3_ppi_sgi_configure_defaults(uintptr_t gicr_base);
void gicv3_secure_spis_configure(uintptr_t gicd_base,
				     unsigned int num_ints,
				     const unsigned int *sec_intr_list,
				     unsigned int int_grp);
void gicv3_secure_ppi_sgi_configure(uintptr_t gicr_base,
					unsigned int num_ints,
					const unsigned int *sec_intr_list,
					unsigned int int_grp);
void gicv3_rdistif_base_addrs_probe(uintptr_t *rdistif_base_addrs,
					unsigned int rdistif_num,
					uintptr_t gicr_base,
					mpidr_hash_fn mpidr_to_core_pos);
void gicv3_rdistif_mark_core_awake(uintptr_t gicr_base);
void gicv3_rdistif_mark_core_asleep(uintptr_t gicr_base);

/*******************************************************************************
 * GIC Distributor interface accessors
 ******************************************************************************/
static inline unsigned int gicd_read_pidr2(uintptr_t base)
{
	return mmio_read_32(base + GICD_PIDR2_GICV3);
}

static inline unsigned long long gicd_read_irouter(uintptr_t base, unsigned int id)
{
	assert(id >= MIN_SPI_ID);
	return mmio_read_64(base + GICD_IROUTER + (id << 3));
}

static inline void gicd_write_irouter(uintptr_t base,
				      unsigned int id,
				      unsigned long long affinity)
{
	assert(id >= MIN_SPI_ID);
	mmio_write_64(base + GICD_IROUTER + (id << 3), affinity);
}

static inline void gicd_clr_ctlr(uintptr_t base,
				 unsigned int bitmap,
				 unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) & ~bitmap);
	if (rwp)
		gicd_wait_for_pending_write(base);
}

static inline void gicd_set_ctlr(uintptr_t base,
				 unsigned int bitmap,
				 unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) | bitmap);
	if (rwp)
		gicd_wait_for_pending_write(base);
}

/*******************************************************************************
 * GIC Redistributor interface accessors
 ******************************************************************************/
static inline unsigned long long gicr_read_ctlr(uintptr_t base)
{
	return mmio_read_64(base + GICR_CTLR);
}

static inline unsigned long long gicr_read_typer(uintptr_t base)
{
	return mmio_read_64(base + GICR_TYPER);
}

static inline unsigned int gicr_read_waker(uintptr_t base)
{
	return mmio_read_32(base + GICR_WAKER);
}

static inline void gicr_write_waker(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_WAKER, val);
}

/*******************************************************************************
 * GIC Re-distributor functions for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/
static inline unsigned int gicr_read_icenabler0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ICENABLER0);
}

static inline void gicr_write_icenabler0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICENABLER0, val);
}

static inline unsigned int gicr_read_isenabler0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ISENABLER0);
}

static inline void gicr_write_isenabler0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ISENABLER0, val);
}

static inline unsigned int gicr_read_igroupr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_IGROUPR0);
}

static inline void gicr_write_igroupr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_IGROUPR0, val);
}

static inline unsigned int gicr_read_igrpmodr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_IGRPMODR0);
}

static inline void gicr_write_igrpmodr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_IGRPMODR0, val);
}

static inline unsigned int gicr_read_icfgr1(uintptr_t base)
{
	return mmio_read_32(base + GICR_ICFGR1);
}

static inline void gicr_write_icfgr1(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICFGR1, val);
}

#endif /* __GICV3_PRIVATE_H__ */

/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GICV3_PRIVATE_H
#define GICV3_PRIVATE_H

#include <assert.h>
#include <stdint.h>

#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>

#include "../common/gic_common_private.h"

/*******************************************************************************
 * GICv3 private macro definitions
 ******************************************************************************/

/* Constants to indicate the status of the RWP bit */
#define RWP_TRUE		U(1)
#define RWP_FALSE		U(0)

/* Calculate GIC register bit number corresponding to its interrupt ID */
#define	BIT_NUM(REG, id)	\
	((id) & ((1U << REG##R_SHIFT) - 1U))

/*
 * Calculate 8, 32 and 64-bit GICD register offset
 * corresponding to its interrupt ID
 */
#if GIC_EXT_INTID
	/* GICv3.1 */
#define	GICD_OFFSET_8(REG, id)				\
	(((id) <= MAX_SPI_ID) ?				\
	GICD_##REG##R + (uintptr_t)(id) :		\
	GICD_##REG##RE + (uintptr_t)(id) - MIN_ESPI_ID)

#define	GICD_OFFSET(REG, id)						\
	(((id) <= MAX_SPI_ID) ?						\
	GICD_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 2) :	\
	GICD_##REG##RE + ((((uintptr_t)(id) - MIN_ESPI_ID) >>		\
					REG##R_SHIFT) << 2))

#define	GICD_OFFSET_64(REG, id)						\
	(((id) <= MAX_SPI_ID) ?						\
	GICD_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 3) :	\
	GICD_##REG##RE + ((((uintptr_t)(id) - MIN_ESPI_ID) >>		\
					REG##R_SHIFT) << 3))

#else	/* GICv3 */
#define	GICD_OFFSET_8(REG, id)	\
	(GICD_##REG##R + (uintptr_t)(id))

#define	GICD_OFFSET(REG, id)	\
	(GICD_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 2))

#define	GICD_OFFSET_64(REG, id)	\
	(GICD_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 3))
#endif	/* GIC_EXT_INTID */

/*
 * Read/Write 8, 32 and 64-bit GIC Distributor register
 * corresponding to its interrupt ID
 */
#define GICD_READ(REG, base, id)	\
	mmio_read_32((base) + GICD_OFFSET(REG, (id)))

#define GICD_READ_64(REG, base, id)	\
	mmio_read_64((base) + GICD_OFFSET_64(REG, (id)))

#define GICD_WRITE_8(REG, base, id, val)	\
	mmio_write_8((base) + GICD_OFFSET_8(REG, (id)), (val))

#define GICD_WRITE(REG, base, id, val)	\
	mmio_write_32((base) + GICD_OFFSET(REG, (id)), (val))

#define GICD_WRITE_64(REG, base, id, val)	\
	mmio_write_64((base) + GICD_OFFSET_64(REG, (id)), (val))

/*
 * Bit operations on GIC Distributor register corresponding
 * to its interrupt ID
 */
/* Get bit in GIC Distributor register */
#define GICD_GET_BIT(REG, base, id)				\
	((mmio_read_32((base) + GICD_OFFSET(REG, (id))) >>	\
		BIT_NUM(REG, (id))) & 1U)

/* Set bit in GIC Distributor register */
#define GICD_SET_BIT(REG, base, id)				\
	mmio_setbits_32((base) + GICD_OFFSET(REG, (id)),	\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/* Clear bit in GIC Distributor register */
#define GICD_CLR_BIT(REG, base, id)				\
	mmio_clrbits_32((base) + GICD_OFFSET(REG, (id)),	\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/* Write bit in GIC Distributor register */
#define	GICD_WRITE_BIT(REG, base, id)			\
	mmio_write_32((base) + GICD_OFFSET(REG, (id)),	\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/*
 * Calculate 8 and 32-bit GICR register offset
 * corresponding to its interrupt ID
 */
#if GIC_EXT_INTID
	/* GICv3.1 */
#define	GICR_OFFSET_8(REG, id)				\
	(((id) <= MAX_PPI_ID) ?				\
	GICR_##REG##R + (uintptr_t)(id) :		\
	GICR_##REG##R + (uintptr_t)(id) - (MIN_EPPI_ID - MIN_SPI_ID))

#define GICR_OFFSET(REG, id)						\
	(((id) <= MAX_PPI_ID) ?						\
	GICR_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 2) :	\
	GICR_##REG##R + ((((uintptr_t)(id) - (MIN_EPPI_ID - MIN_SPI_ID))\
						>> REG##R_SHIFT) << 2))
#else	/* GICv3 */
#define	GICR_OFFSET_8(REG, id)	\
	(GICR_##REG##R + (uintptr_t)(id))

#define GICR_OFFSET(REG, id)	\
	(GICR_##REG##R + (((uintptr_t)(id) >> REG##R_SHIFT) << 2))
#endif /* GIC_EXT_INTID */

/* Read/Write GIC Redistributor register corresponding to its interrupt ID */
#define GICR_READ(REG, base, id)			\
	mmio_read_32((base) + GICR_OFFSET(REG, (id)))

#define GICR_WRITE_8(REG, base, id, val)		\
	mmio_write_8((base) + GICR_OFFSET_8(REG, (id)), (val))

#define GICR_WRITE(REG, base, id, val)			\
	mmio_write_32((base) + GICR_OFFSET(REG, (id)), (val))

/*
 * Bit operations on GIC Redistributor register
 * corresponding to its interrupt ID
 */
/* Get bit in GIC Redistributor register */
#define GICR_GET_BIT(REG, base, id)				\
	((mmio_read_32((base) + GICR_OFFSET(REG, (id))) >>	\
		BIT_NUM(REG, (id))) & 1U)

/* Write bit in GIC Redistributor register */
#define	GICR_WRITE_BIT(REG, base, id)				\
	mmio_write_32((base) + GICR_OFFSET(REG, (id)),		\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/* Set bit in GIC Redistributor register */
#define	GICR_SET_BIT(REG, base, id)				\
	mmio_setbits_32((base) + GICR_OFFSET(REG, (id)),	\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/* Clear bit in GIC Redistributor register */
#define	GICR_CLR_BIT(REG, base, id)				\
	mmio_clrbits_32((base) + GICR_OFFSET(REG, (id)),	\
		((uint32_t)1 << BIT_NUM(REG, (id))))

/*
 * Macro to convert an mpidr to a value suitable for programming into a
 * GICD_IROUTER. Bits[31:24] in the MPIDR are cleared as they are not relevant
 * to GICv3.
 */
static inline u_register_t gicd_irouter_val_from_mpidr(u_register_t mpidr,
						       unsigned int irm)
{
	return (mpidr & ~(U(0xff) << 24)) |
		((irm & IROUTER_IRM_MASK) << IROUTER_IRM_SHIFT);
}

/*
 * Macro to convert a GICR_TYPER affinity value into a MPIDR value. Bits[31:24]
 * are zeroes.
 */
#ifdef __aarch64__
static inline u_register_t mpidr_from_gicr_typer(uint64_t typer_val)
{
	return (((typer_val >> 56) & MPIDR_AFFLVL_MASK) << MPIDR_AFF3_SHIFT) |
		((typer_val >> 32) & U(0xffffff));
}
#else
static inline u_register_t mpidr_from_gicr_typer(uint64_t typer_val)
{
	return (((typer_val) >> 32) & U(0xffffff));
}
#endif

/*******************************************************************************
 * GICv3 private global variables declarations
 ******************************************************************************/
extern const gicv3_driver_data_t *gicv3_driver_data;

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
unsigned int gicr_get_igrpmodr(uintptr_t base, unsigned int id);
unsigned int gicr_get_igroupr(uintptr_t base, unsigned int id);
unsigned int gicr_get_isactiver(uintptr_t base, unsigned int id);
void gicd_set_igrpmodr(uintptr_t base, unsigned int id);
void gicr_set_igrpmodr(uintptr_t base, unsigned int id);
void gicr_set_isenabler(uintptr_t base, unsigned int id);
void gicr_set_icenabler(uintptr_t base, unsigned int id);
void gicr_set_ispendr(uintptr_t base, unsigned int id);
void gicr_set_icpendr(uintptr_t base, unsigned int id);
void gicr_set_igroupr(uintptr_t base, unsigned int id);
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id);
void gicr_clr_igrpmodr(uintptr_t base, unsigned int id);
void gicr_clr_igroupr(uintptr_t base, unsigned int id);
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri);
void gicr_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg);

/*******************************************************************************
 * Private GICv3 helper function prototypes
 ******************************************************************************/
void gicv3_spis_config_defaults(uintptr_t gicd_base);
void gicv3_ppi_sgi_config_defaults(uintptr_t gicr_base);
unsigned int gicv3_secure_ppi_sgi_config_props(uintptr_t gicr_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
unsigned int gicv3_secure_spis_config_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
void gicv3_rdistif_base_addrs_probe(uintptr_t *rdistif_base_addrs,
					unsigned int rdistif_num,
					uintptr_t gicr_base,
					mpidr_hash_fn mpidr_to_core_pos);
void gicv3_rdistif_mark_core_awake(uintptr_t gicr_base);
void gicv3_rdistif_mark_core_asleep(uintptr_t gicr_base);

/*******************************************************************************
 * GIC Distributor interface accessors
 ******************************************************************************/
/*
 * Wait for updates to:
 * GICD_CTLR[2:0] - the Group Enables
 * GICD_CTLR[7:4] - the ARE bits, E1NWF bit and DS bit
 * GICD_ICENABLER<n> - the clearing of enable state for SPIs
 */
static inline void gicd_wait_for_pending_write(uintptr_t gicd_base)
{
	while ((gicd_read_ctlr(gicd_base) & GICD_CTLR_RWP_BIT) != 0U) {
	}
}

static inline uint32_t gicd_read_pidr2(uintptr_t base)
{
	return mmio_read_32(base + GICD_PIDR2_GICV3);
}

static inline uint64_t gicd_read_irouter(uintptr_t base, unsigned int id)
{
	assert(id >= MIN_SPI_ID);
	return GICD_READ_64(IROUTE, base, id);
}

static inline void gicd_write_irouter(uintptr_t base,
				      unsigned int id,
				      uint64_t affinity)
{
	assert(id >= MIN_SPI_ID);
	GICD_WRITE_64(IROUTE, base, id, affinity);
}

static inline void gicd_clr_ctlr(uintptr_t base,
				 unsigned int bitmap,
				 unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) & ~bitmap);
	if (rwp != 0U) {
		gicd_wait_for_pending_write(base);
	}
}

static inline void gicd_set_ctlr(uintptr_t base,
				 unsigned int bitmap,
				 unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) | bitmap);
	if (rwp != 0U) {
		gicd_wait_for_pending_write(base);
	}
}

/*******************************************************************************
 * GIC Redistributor interface accessors
 ******************************************************************************/
static inline uint32_t gicr_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICR_CTLR);
}

static inline void gicr_write_ctlr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_CTLR, val);
}

static inline uint64_t gicr_read_typer(uintptr_t base)
{
	return mmio_read_64(base + GICR_TYPER);
}

static inline uint32_t gicr_read_waker(uintptr_t base)
{
	return mmio_read_32(base + GICR_WAKER);
}

static inline void gicr_write_waker(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_WAKER, val);
}

/*
 * Wait for updates to:
 * GICR_ICENABLER0
 * GICR_CTLR.DPG1S
 * GICR_CTLR.DPG1NS
 * GICR_CTLR.DPG0
 * GICR_CTLR, which clears EnableLPIs from 1 to 0
 */
static inline void gicr_wait_for_pending_write(uintptr_t gicr_base)
{
	while ((gicr_read_ctlr(gicr_base) & GICR_CTLR_RWP_BIT) != 0U) {
	}
}

static inline void gicr_wait_for_upstream_pending_write(uintptr_t gicr_base)
{
	while ((gicr_read_ctlr(gicr_base) & GICR_CTLR_UWP_BIT) != 0U) {
	}
}

/* Private implementation of Distributor power control hooks */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num);
void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num);

/*******************************************************************************
 * GIC Redistributor functions for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/

/*
 * Accessors to read/write GIC Redistributor ICENABLER0 register
 */
static inline unsigned int gicr_read_icenabler0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ICENABLER0);
}

static inline void gicr_write_icenabler0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICENABLER0, val);
}

/*
 * Accessors to read/write GIC Redistributor ICENABLER0 and ICENABLERE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_icenabler(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ICENABLER + (reg_num << 2));
}

static inline void gicr_write_icenabler(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICENABLER + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor ICFGR0, ICFGR1 registers
 */
static inline unsigned int gicr_read_icfgr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ICFGR0);
}

static inline unsigned int gicr_read_icfgr1(uintptr_t base)
{
	return mmio_read_32(base + GICR_ICFGR1);
}

static inline void gicr_write_icfgr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICFGR0, val);
}

static inline void gicr_write_icfgr1(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICFGR1, val);
}

/*
 * Accessors to read/write GIC Redistributor ICFGR0, ICFGR1 and ICFGRE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_icfgr(uintptr_t base, unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ICFGR + (reg_num << 2));
}

static inline void gicr_write_icfgr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICFGR + (reg_num << 2), val);
}

/*
 * Accessor to write GIC Redistributor ICPENDR0 register
 */
static inline void gicr_write_icpendr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ICPENDR0, val);
}

/*
 * Accessor to write GIC Redistributor ICPENDR0 and ICPENDRE
 * register corresponding to its number
 */
static inline void gicr_write_icpendr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICPENDR + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor IGROUPR0 register
 */
static inline unsigned int gicr_read_igroupr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_IGROUPR0);
}

static inline void gicr_write_igroupr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_IGROUPR0, val);
}

/*
 * Accessors to read/write GIC Redistributor IGROUPR0 and IGROUPRE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_igroupr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IGROUPR + (reg_num << 2));
}

static inline void gicr_write_igroupr(uintptr_t base, unsigned int reg_num,
						unsigned int val)
{
	mmio_write_32(base + GICR_IGROUPR + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor IGRPMODR0 register
 */
static inline unsigned int gicr_read_igrpmodr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_IGRPMODR0);
}

static inline void gicr_write_igrpmodr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_IGRPMODR0, val);
}

/*
 * Accessors to read/write GIC Redistributor IGRPMODR0 and IGRPMODRE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_igrpmodr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IGRPMODR + (reg_num << 2));
}

static inline void gicr_write_igrpmodr(uintptr_t base, unsigned int reg_num,
				       unsigned int val)
{
	mmio_write_32(base + GICR_IGRPMODR + (reg_num << 2), val);
}

/*
 * Accessors to read/write the GIC Redistributor IPRIORITYR(E) register
 * corresponding to its number, 4 interrupts IDs at a time.
 */
static inline unsigned int gicr_ipriorityr_read(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IPRIORITYR + (reg_num << 2));
}

static inline void gicr_ipriorityr_write(uintptr_t base, unsigned int reg_num,
						unsigned int val)
{
	mmio_write_32(base + GICR_IPRIORITYR + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor ISACTIVER0 register
 */
static inline unsigned int gicr_read_isactiver0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ISACTIVER0);
}

static inline void gicr_write_isactiver0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ISACTIVER0, val);
}

/*
 * Accessors to read/write GIC Redistributor ISACTIVER0 and ISACTIVERE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_isactiver(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISACTIVER + (reg_num << 2));
}

static inline void gicr_write_isactiver(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ISACTIVER + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor ISENABLER0 register
 */
static inline unsigned int gicr_read_isenabler0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ISENABLER0);
}

static inline void gicr_write_isenabler0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ISENABLER0, val);
}

/*
 * Accessors to read/write GIC Redistributor ISENABLER0 and ISENABLERE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_isenabler(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISENABLER + (reg_num << 2));
}

static inline void gicr_write_isenabler(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ISENABLER + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor ISPENDR0 register
 */
static inline unsigned int gicr_read_ispendr0(uintptr_t base)
{
	return mmio_read_32(base + GICR_ISPENDR0);
}

static inline void gicr_write_ispendr0(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_ISPENDR0, val);
}

/*
 * Accessors to read/write GIC Redistributor ISPENDR0 and ISPENDRE
 * register corresponding to its number
 */
static inline unsigned int gicr_read_ispendr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISPENDR + (reg_num << 2));
}

static inline void gicr_write_ispendr(uintptr_t base, unsigned int reg_num,
						unsigned int val)
{
	mmio_write_32(base + GICR_ISPENDR + (reg_num << 2), val);
}

/*
 * Accessors to read/write GIC Redistributor NSACR register
 */
static inline unsigned int gicr_read_nsacr(uintptr_t base)
{
	return mmio_read_32(base + GICR_NSACR);
}

static inline void gicr_write_nsacr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_NSACR, val);
}

/*
 * Accessors to read/write GIC Redistributor PROPBASER register
 */
static inline uint64_t gicr_read_propbaser(uintptr_t base)
{
	return mmio_read_64(base + GICR_PROPBASER);
}

static inline void gicr_write_propbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GICR_PROPBASER, val);
}

/*
 * Accessors to read/write GIC Redistributor PENDBASER register
 */
static inline uint64_t gicr_read_pendbaser(uintptr_t base)
{
	return mmio_read_64(base + GICR_PENDBASER);
}

static inline void gicr_write_pendbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GICR_PENDBASER, val);
}

/*******************************************************************************
 * GIC ITS functions to read and write entire ITS registers.
 ******************************************************************************/
static inline uint32_t gits_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GITS_CTLR);
}

static inline void gits_write_ctlr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GITS_CTLR, val);
}

static inline uint64_t gits_read_cbaser(uintptr_t base)
{
	return mmio_read_64(base + GITS_CBASER);
}

static inline void gits_write_cbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GITS_CBASER, val);
}

static inline uint64_t gits_read_cwriter(uintptr_t base)
{
	return mmio_read_64(base + GITS_CWRITER);
}

static inline void gits_write_cwriter(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GITS_CWRITER, val);
}

static inline uint64_t gits_read_baser(uintptr_t base,
					unsigned int its_table_id)
{
	assert(its_table_id < 8U);
	return mmio_read_64(base + GITS_BASER + (8U * its_table_id));
}

static inline void gits_write_baser(uintptr_t base, unsigned int its_table_id,
					uint64_t val)
{
	assert(its_table_id < 8U);
	mmio_write_64(base + GITS_BASER + (8U * its_table_id), val);
}

/*
 * Wait for Quiescent bit when GIC ITS is disabled
 */
static inline void gits_wait_for_quiescent_bit(uintptr_t gits_base)
{
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_ENABLED_BIT) == 0U);
	while ((gits_read_ctlr(gits_base) & GITS_CTLR_QUIESCENT_BIT) == 0U) {
	}
}

#endif /* GICV3_PRIVATE_H */

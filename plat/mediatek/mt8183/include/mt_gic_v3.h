/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GIC_V3_H
#define MT_GIC_V3_H

#include <lib/mmio.h>

enum irq_schedule_mode {
    SW_MODE,
    HW_MODE,
};

#define GICR_ISENABLER		0x100
#define GICR_ICENABLER		0x180

#define IRQ_MASK_HEADER		0xF1F1F1F1
#define IRQ_MASK_FOOTER		0xF2F2F2F2

typedef struct mtk_irq_mask {
	uint32_t header;	/* for error checking */
	uint32_t mask0;
	uint32_t mask1;
	uint32_t mask2;
	uint32_t mask3;
	uint32_t mask4;
	uint32_t mask5;
	uint32_t mask6;
	uint32_t mask7;
	uint32_t mask8;
	uint32_t mask9;
	uint32_t mask10;
	uint32_t footer;	/* for error checking */
} mtk_irq_mask_t;

#define GIC_INT_MASK (MCUCFG_BASE + 0x5e8)
#define GIC500_ACTIVE_SEL_SHIFT 3
#define GIC500_ACTIVE_SEL_MASK (0x7 << GIC500_ACTIVE_SEL_SHIFT)
#define GIC500_ACTIVE_CPU_SHIFT 16
#define GIC500_ACTIVE_CPU_MASK (0xff << GIC500_ACTIVE_CPU_SHIFT)
#define SGI_MASK 0xffff

void setup_int_schedule_mode(enum irq_schedule_mode mode, unsigned int active_cpu);
void int_schedule_mode_save(void);
void int_schedule_mode_restore(void);

void gic_rdist_save(void);
void gic_rdist_restore(void);
void gic_rdist_restore_all(void);
void gic_sgi_save_all(void);
void gic_sgi_restore_all(void);
void gic_cpuif_deactivate(unsigned int gicc_base);
void gic_dist_save(void);
void gic_dist_restore(void);
void gic_setup(void);
uint32_t mt_irq_get_pending(uint32_t irq);
void mt_irq_set_pending(uint32_t irq);
int32_t mt_irq_mask_all(mtk_irq_mask_t *mask);
int32_t mt_irq_mask_restore(struct mtk_irq_mask *mask);
void mt_irq_mask_for_sleep(uint32_t irq);
void mt_irq_unmask_for_sleep(int32_t irq);

/* distributor registers & their field definitions, in secure world */
#define GICD_V3_CTLR                        0x0000
#define GICD_V3_TYPER                       0x0004
#define GICD_V3_IIDR                        0x0008
#define GICD_V3_STATUSR                     0x0010
#define GICD_V3_SETSPI_NSR                  0x0040
#define GICD_V3_CLRSPI_NSR                  0x0048
#define GICD_V3_SETSPI_SR                   0x0050
#define GICD_V3_CLRSPI_SR                   0x0058
#define GICD_V3_SEIR                        0x0068
#define GICD_V3_ISENABLER                   0x0100
#define GICD_V3_ICENABLER                   0x0180
#define GICD_V3_ISPENDR                     0x0200
#define GICD_V3_ICPENDR                     0x0280
#define GICD_V3_ISACTIVER                   0x0300
#define GICD_V3_ICACTIVER                   0x0380
#define GICD_V3_IPRIORITYR                  0x0400
#define GICD_V3_ICFGR                       0x0C00
#define GICD_V3_IROUTER                     0x6000
#define GICD_V3_PIDR2                       0xFFE8

#define GICD_V3_CTLR_RWP                    (1U << 31)
#define GICD_V3_CTLR_E1NWF                  (1U << 7)
#define GICD_V3_CTLR_DS                     (1U << 6)
#define GICD_V3_CTLR_ARE_NS                 (1U << 5)
#define GICD_V3_CTLR_ARE_S                  (1U << 4)
#define GICD_V3_CTLR_ENABLE_G1S             (1U << 2)
#define GICD_V3_CTLR_ENABLE_G1NS            (1U << 1)
#define GICD_V3_CTLR_ENABLE_G0              (1U << 0)

#define GICD_V3_TYPER_ID_BITS(typer)        ((((typer) >> 19) & 0x1f) + 1)
#define GICD_V3_TYPER_IRQS(typer)           ((((typer) & 0x1f) + 1) * 32)
#define GICD_V3_TYPER_LPIS                  (1U << 17)
#define GICD_V3_IROUTER_SPI_MODE_ONE        (0U << 31)
#define GICD_V3_IROUTER_SPI_MODE_ANY        (1U << 31)

#define GIC_V3_PIDR2_ARCH_MASK              0xf0
#define GIC_V3_PIDR2_ARCH_GICv3             0x30
#define GIC_V3_PIDR2_ARCH_GICv4             0x40

/*
 * Re-Distributor registers, offsets from RD_base
 */
#define GICR_V3_CTLR                        GICD_V3_CTLR
#define GICR_V3_IIDR                        0x0004
#define GICR_V3_TYPER                       0x0008
#define GICR_V3_STATUSR                     GICD_V3_STATUSR
#define GICR_V3_WAKER                       0x0014
#define GICR_V3_SETLPIR                     0x0040
#define GICR_V3_CLRLPIR                     0x0048
#define GICR_V3_SEIR                        GICD_V3_SEIR
#define GICR_V3_PROPBASER                   0x0070
#define GICR_V3_PENDBASER                   0x0078
#define GICE_V3_IGROUP0                     0x0080
#define GICR_V3_INVLPIR                     0x00A0
#define GICR_V3_INVALLR                     0x00B0
#define GICR_V3_SYNCR                       0x00C0
#define GICR_V3_MOVLPIR                     0x0100
#define GICR_V3_MOVALLR                     0x0110
#define GICE_V3_IGRPMOD0                    0x0d00
#define GICR_V3_PIDR2                       GICD_V3_PIDR2
#define GICR_V3_CTLR_ENABLE_LPIS            (1UL << 0)
#define GICR_V3_TYPER_CPU_NUMBER(r)         (((r) >> 8) & 0xffff)
#define GICR_V3_WAKER_ProcessorSleep        (1U << 1)
#define GICR_V3_WAKER_ChildrenAsleep        (1U << 2)
#define GICR_V3_PROPBASER_NonShareable      (0U << 10)
#define GICR_V3_PROPBASER_InnerShareable    (1U << 10)
#define GICR_V3_PROPBASER_OuterShareable    (2U << 10)
#define GICR_V3_PROPBASER_SHAREABILITY_MASK (3UL << 10)
#define GICR_V3_PROPBASER_nCnB              (0U << 7)
#define GICR_V3_PROPBASER_nC                (1U << 7)
#define GICR_V3_PROPBASER_RaWt              (2U << 7)
#define GICR_V3_PROPBASER_RaWb              (3U << 7)
#define GICR_V3_PROPBASER_WaWt              (4U << 7)
#define GICR_V3_PROPBASER_WaWb              (5U << 7)
#define GICR_V3_PROPBASER_RaWaWt            (6U << 7)
#define GICR_V3_PROPBASER_RaWaWb            (7U << 7)
#define GICR_V3_PROPBASER_IDBITS_MASK       (0x1f)

/*
 * Re-Distributor registers, offsets from SGI_base
 */
#define GICR_V3_ISENABLER0                  GICD_V3_ISENABLER
#define GICR_V3_ICENABLER0                  GICD_V3_ICENABLER
#define GICR_V3_ISPENDR0                    GICD_V3_ISPENDR
#define GICR_V3_ICPENDR0                    GICD_V3_ICPENDR
#define GICR_V3_ISACTIVER0                  GICD_V3_ISACTIVER
#define GICR_V3_ICACTIVER0                  GICD_V3_ICACTIVER
#define GICR_V3_IPRIORITYR0                 GICD_V3_IPRIORITYR
#define GICR_V3_ICFGR0                      GICD_V3_ICFGR
#define GICR_V3_TYPER_PLPIS                 (1U << 0)
#define GICR_V3_TYPER_VLPIS                 (1U << 1)
#define GICR_V3_TYPER_LAST                  (1U << 4)

static inline unsigned int gicd_v3_read_ctlr(unsigned int base)
{
    return mmio_read_32(base + GICD_V3_CTLR);
}

static inline void gicd_v3_write_ctlr(unsigned int base, unsigned int val)
{
    mmio_write_32(base + GICD_V3_CTLR, val);
}

static inline unsigned int gicd_v3_read_pidr2(unsigned int base)
{
    return mmio_read_32(base + GICD_V3_PIDR2);
}

static inline void gicd_v3_set_irouter(unsigned int base, unsigned int id, uint64_t aff)
{
    unsigned int reg = base + GICD_V3_IROUTER + (id*8);

    mmio_write_64(reg, aff);
}

#endif /* MT_GIC_V3_H */

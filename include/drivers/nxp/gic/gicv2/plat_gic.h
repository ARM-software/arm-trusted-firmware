/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_GICV2_H
#define PLAT_GICV2_H

#include <drivers/arm/gicv2.h>

 /* register offsets */
#define GICD_CTLR_OFFSET          0x0
#define GICD_CPENDSGIR3_OFFSET    0xF1C
#define GICD_SPENDSGIR3_OFFSET    0xF2C
#define GICD_SGIR_OFFSET          0xF00
#define GICD_IGROUPR0_OFFSET      0x080
#define GICD_TYPER_OFFSET         0x0004
#define GICD_ISENABLER0_OFFSET    0x0100
#define GICD_ICENABLER0_OFFSET    0x0180
#define GICD_IPRIORITYR3_OFFSET   0x040C
#define GICD_ISENABLERn_OFFSET    0x0100
#define GICD_ISACTIVER0_OFFSET    0x300

#define GICC_CTLR_OFFSET          0x0
#define GICC_PMR_OFFSET           0x0004
#define GICC_IAR_OFFSET           0x000C
#define GICC_DIR_OFFSET           0x1000
#define GICC_EOIR_OFFSET          0x0010

 /* bitfield masks */
#define GICC_CTLR_EN_GRP0           0x1
#define GICC_CTLR_EN_GRP1           0x2
#define GICC_CTLR_EOImodeS_MASK     0x200
#define GICC_CTLR_DIS_BYPASS        0x60
#define GICC_CTLR_CBPR_MASK         0x10
#define GICC_CTLR_FIQ_EN_MASK       0x8
#define GICC_CTLR_ACKCTL_MASK       0x4
#define GICC_PMR_FILTER             0xFF

#define GICD_CTLR_EN_GRP0           0x1
#define GICD_CTLR_EN_GRP1           0x2
#define GICD_IGROUP0_SGI15          0x8000
#define GICD_ISENABLE0_SGI15        0x8000
#define GICD_ICENABLE0_SGI15        0x8000
#define GICD_ISACTIVER0_SGI15       0x8000
#define GICD_CPENDSGIR_CLR_MASK     0xFF000000
#define GICD_IPRIORITY_SGI15_MASK   0xFF000000
#define GICD_SPENDSGIR3_SGI15_MASK  0xFF000000
#define GICD_SPENDSGIR3_SGI15_OFFSET  0x18

#ifndef __ASSEMBLER__

/* GIC common API's */
void plat_ls_gic_driver_init(const uintptr_t nxp_gicd_addr,
			     const uintptr_t nxp_gicc_addr,
			     uint8_t plat_core_count,
			     interrupt_prop_t *ls_interrupt_props,
			     uint8_t ls_interrupt_prop_count,
			     uint32_t *target_mask_array);
void plat_ls_gic_init(void);
void plat_ls_gic_cpuif_enable(void);
void plat_ls_gic_cpuif_disable(void);
void plat_ls_gic_redistif_on(void);
void plat_ls_gic_redistif_off(void);
void plat_gic_pcpu_init(void);
/* GIC utility functions */
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base);
#endif

#endif /* PLAT_GICV2_H */

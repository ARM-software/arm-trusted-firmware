/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MARVELL_PLAT_PRIV_H
#define MARVELL_PLAT_PRIV_H

#include <lib/utils.h>

/*****************************************************************************
 * Function and variable prototypes
 *****************************************************************************
 */
void plat_delay_timer_init(void);

uint64_t mvebu_get_dram_size(uint64_t ap_base_addr);

/*
 * GIC operation, mandatory functions required in Marvell standard platforms
 */
void plat_marvell_gic_driver_init(void);
void plat_marvell_gic_init(void);
void plat_marvell_gic_cpuif_enable(void);
void plat_marvell_gic_cpuif_disable(void);
void plat_marvell_gic_pcpu_init(void);
void plat_marvell_gic_irq_save(void);
void plat_marvell_gic_irq_restore(void);
void plat_marvell_gic_irq_pcpu_save(void);
void plat_marvell_gic_irq_pcpu_restore(void);

#endif /* MARVELL_PLAT_PRIV_H */

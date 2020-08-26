/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PLAT_H
#define QTI_PLAT_H

#include <stdint.h>

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*
 * Utility functions common to QTI platforms
 */
int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr);
int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);

/*
 * Utility functions common to ARM standard platforms
 */
void qti_setup_page_tables(uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit,
			   uintptr_t coh_start, uintptr_t coh_limit);

/*
 * Mandatory functions required in ARM standard platforms
 */
void plat_qti_gic_driver_init(void);
void plat_qti_gic_init(void);
void plat_qti_gic_cpuif_enable(void);
void plat_qti_gic_cpuif_disable(void);
void plat_qti_gic_pcpu_init(void);

/*
 * Optional functions required in ARM standard platforms
 */
unsigned int plat_qti_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_qti_my_cluster_pos(void);

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr);

void qti_pmic_prepare_reset(void);
void qti_pmic_prepare_shutdown(void);

#endif /* QTI_PLAT_H */

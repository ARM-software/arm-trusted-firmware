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
#include <drivers/arm/gicv3.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr);
int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);
void qti_setup_page_tables(
			   uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
			  );
int qti_io_setup(void);
struct image_info *qti_get_image_info(unsigned int image_id);

void plat_qti_gic_driver_init(void);
void plat_qti_gic_init(void);
void plat_qti_gic_cpuif_enable(void);
void plat_qti_gic_cpuif_disable(void);
void plat_qti_gic_pcpu_init(void);
const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props);

unsigned int plat_qti_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_qti_my_cluster_pos(void);

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr);

void qti_pmic_prepare_reset(void);
void qti_pmic_prepare_shutdown(void);

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos);
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states);
void plat_qti_pwr_domain_off(const uint8_t *states);
void plat_qti_pwr_domain_suspend(const uint8_t *states);
void plat_qti_pwr_domain_suspend_finish(const uint8_t *states);
int  plat_qti_pwr_psci_init(uintptr_t warmboot_entry);

void plat_qti_bl31_setup_post(void);
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle);

typedef struct chip_id_info {
	uint16_t jtag_id;
	uint16_t chipinfo_id;
} chip_id_info_t;

#endif /* QTI_PLAT_H */

/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <bl31/interrupt_mgmt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

typedef struct versal_intr_info_type_el3 {
	uint32_t id;
	interrupt_type_handler_t handler;
} versal_intr_info_type_el3_t;

void versal_net_config_setup(void);

const mmap_region_t *plat_versal_net_get_mmap(void);

void plat_versal_net_gic_driver_init(void);
void plat_versal_net_gic_init(void);
void plat_versal_net_gic_cpuif_enable(void);
void plat_versal_net_gic_cpuif_disable(void);
void plat_versal_net_gic_pcpu_init(void);
void plat_versal_net_gic_save(void);
void plat_versal_net_gic_resume(void);
void plat_versal_net_gic_redistif_on(void);
void plat_versal_net_gic_redistif_off(void);

extern uint32_t cpu_clock, platform_id, platform_version;
void board_detection(void);
char *board_name_decode(void);
uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags);
int32_t sip_svc_setup_init(void);
/*
 * Register handler to specific GIC entrance
 * for INTR_TYPE_EL3 type of interrupt
 */
int request_intr_type_el3(uint32_t irq, interrupt_type_handler_t fiq_handler);

#define PM_GET_CHIPID			(24U)
#define IOCTL_OSPI_MUX_SELECT		(21U)

#endif /* PLAT_PRIVATE_H */

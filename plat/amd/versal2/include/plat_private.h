/*
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <bl31/interrupt_mgmt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#define SPP_PSXC_MMI_V2_0	U(6)
#define SPP_PSXC_MMI_V3_0	U(8)

/* MMD */
#define SPP_PSXC_ISP_AIE_V2_0	U(3)
#define SPP_PSXC_MMD_AIE_FRZ_EA	U(4)
#define SPP_PSXC_MMD_AIE_V3_0	U(5)

typedef struct versal_intr_info_type_el3 {
	uint32_t id;
	interrupt_type_handler_t handler;
} versal_intr_info_type_el3_t;

void config_setup(void);
uint32_t get_uart_clk(void);

const mmap_region_t *plat_get_mmap(void);

void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);
void plat_gic_save(void);
void plat_gic_resume(void);
void plat_gic_redistif_on(void);
void plat_gic_redistif_off(void);

extern uint32_t cpu_clock, platform_id, platform_version;
void board_detection(void);
const char *board_name_decode(void);
uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags);
int32_t sip_svc_setup_init(void);
/*
 * Register handler to specific GIC entrance
 * for INTR_TYPE_EL3 type of interrupt
 */
int request_intr_type_el3(uint32_t irq, interrupt_type_handler_t fiq_handler);

#endif /* PLAT_PRIVATE_H */

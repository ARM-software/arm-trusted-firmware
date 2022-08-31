/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <lib/xlat_tables/xlat_tables_v2.h>

void versal_net_config_setup(void);

const mmap_region_t *plat_versal_net_get_mmap(void);

void plat_versal_net_gic_driver_init(void);
void plat_versal_net_gic_init(void);
void plat_versal_net_gic_cpuif_enable(void);
void plat_versal_net_gic_pcpu_init(void);

extern uint32_t cpu_clock, platform_id, platform_version;
void board_detection(void);
char *board_name_decode(void);
uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags);
int32_t sip_svc_setup_init(void);

#define PM_GET_CHIPID			(24U)
#define IOCTL_OSPI_MUX_SELECT		(21U)

#endif /* PLAT_PRIVATE_H */

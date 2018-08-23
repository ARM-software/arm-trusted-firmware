/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_QTI_H__
#define __PLAT_QTI_H__

#include <bl_common.h>
#include <cassert.h>
#include <cpu_data.h>
#include <stdint.h>
#include <xlat_tables_v2.h>

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

#endif /* __PLAT_QTI_H__ */

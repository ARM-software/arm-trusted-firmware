/*
 * Copyright 2024-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_INTERCORE_SVC_H
#define IMX_INTERCORE_SVC_H

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>

#include <imx_interrupt_mgmt.h>
#include <platform_def.h>	/* CACHE_WRITEBACK_GRANULE required */

typedef struct intercore_svc {
	volatile bool enabled;
	volatile bool trigged;
	volatile int result;
	interrupt_type_handler_t handler;
} intercore_svc_t;

typedef struct percpu_svc_inf {
	intercore_svc_t cpu_off_svc;
	intercore_svc_t dvfs_svc;
} __aligned(CACHE_WRITEBACK_GRANULE) percpu_svc_inf_t;

int register_intercore_cpuoff_svc(interrupt_type_handler_t handler);
int trigger_intercore_cpuoff_svc(unsigned int cpu);
int register_intercore_dvfs_svc(interrupt_type_handler_t handler);
int trigger_intercore_dvfs_svc(unsigned int cpu);
void imx_intercore_svc_init(void);

#endif	/* IMX_INTERCORE_SVC_H	*/

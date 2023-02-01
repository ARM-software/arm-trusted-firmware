/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_HW_CONFIG_GETTER_H
#define FCONF_HW_CONFIG_GETTER_H

#include <lib/fconf/fconf.h>
#include <services/rmm_core_manifest.h>

#include <plat/arm/common/arm_def.h>

/* Hardware Config related getter */
#define hw_config__gicv3_config_getter(prop) gicv3_config.prop
#define hw_config__topology_getter(prop) soc_topology.prop
#define hw_config__uart_serial_config_getter(prop) uart_serial_config.prop
#define hw_config__cpu_timer_getter(prop) cpu_timer.prop
#define hw_config__dram_layout_getter(prop) dram_layout.prop

struct gicv3_config_t {
	uint64_t gicd_base;
	uint64_t gicr_base;
};

struct hw_topology_t {
	uint32_t plat_cluster_count;
	uint32_t cluster_cpu_count;
	uint32_t plat_cpu_count;
	uint32_t plat_max_pwr_level;
};

struct uart_serial_config_t {
	uint64_t uart_base;
	uint32_t uart_clk;
};

struct cpu_timer_t {
	uint32_t clock_freq;
};

struct ns_dram_layout {
	uint64_t num_banks;
	struct ns_dram_bank dram_bank[ARM_DRAM_NUM_BANKS];
};

int fconf_populate_gicv3_config(uintptr_t config);
int fconf_populate_topology(uintptr_t config);
int fconf_populate_uart_config(uintptr_t config);
int fconf_populate_cpu_timer(uintptr_t config);
int fconf_populate_dram_layout(uintptr_t config);

extern struct gicv3_config_t gicv3_config;
extern struct hw_topology_t soc_topology;
extern struct uart_serial_config_t uart_serial_config;
extern struct cpu_timer_t cpu_timer;
extern struct ns_dram_layout dram_layout;

#endif /* FCONF_HW_CONFIG_GETTER_H */

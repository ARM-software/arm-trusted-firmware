/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM_PLAT_CONFIG_H
#define SGM_PLAT_CONFIG_H

#include <drivers/arm/ccn.h>
#include <drivers/arm/gicv3.h>

/* The type of interconnect */
typedef enum {
	ARM_CCI = 0,
	ARM_CCN,
	ARM_CMN
} css_inteconn_type_t;

typedef ccn_desc_t inteconn_desc_t;

/* Interconnect configurations */
typedef struct css_inteconn_config {
	css_inteconn_type_t ip_type;
	const inteconn_desc_t *plat_inteconn_desc;
} css_inteconn_config_t;

/* Topology configurations */
typedef struct css_topology {
	const unsigned char *power_tree;
	unsigned int plat_cluster_core_count;
} css_topology_t;

typedef struct css_plat_config {
	const gicv3_driver_data_t *gic_data;
	const css_inteconn_config_t *inteconn;
	const css_topology_t *topology;
} css_plat_config_t;

void plat_config_init(void);
css_plat_config_t *get_plat_config(void);

#endif /* SGM_PLAT_CONFIG_H */

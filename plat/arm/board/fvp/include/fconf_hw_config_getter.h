/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_HW_CONFIG_GETTER_H
#define FCONF_HW_CONFIG_GETTER_H

#include <lib/fconf/fconf.h>

/* Hardware Config related getter */
#define hw_config__gicv3_config_getter(prop) gicv3_config.prop

struct gicv3_config_t {
	void *gicd_base;
	void *gicr_base;
};

int fconf_populate_gicv3_config(uintptr_t config);

extern struct gicv3_config_t gicv3_config;

#endif /* FCONF_HW_CONFIG_GETTER_H */

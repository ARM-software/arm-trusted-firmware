/*
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_DT_H
#define SOCFPGA_DT_H


#include <stdlib.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
/*
 * This macro takes three arguments:
 *   config:	Configuration identifier
 *   name:	property namespace
 *   callback:	populate() function
 */
#define SOCFPGA_REGISTER_POPULATOR(config, name, callback)			\
	__section(".socfpga_populator") __used			\
	static const struct socfpga_populator (name##__populator) = {		\
		.config_type = (#config),					\
		.info = (#name),						\
		.populate = (callback)						\
	}

/*
 * Populator callback
 *
 * This structure are used by the fconf_populate function and should only be
 * defined by the SOCFPGA_REGISTER_POPULATOR macro.
 */
struct socfpga_populator {
	/* Description of the data loaded by the callback */
	const char *config_type;
	const char *info;

	/* Callback used by fconf_populate function with a provided config dtb.
	 * Return 0 on success, err_code < 0 otherwise.
	 */
	int (*populate)(uintptr_t config);
};

/* Hardware Config related getter */
#define hw_config__gicv3_config_getter(prop) plat_gicv3_gic_data.prop

/* Function Definitions */
int socfpga_dt_open_and_check(uintptr_t dt_addr, char *compatible_str);
int socfpga_dt_populate_gicv3_config(uintptr_t dt_addr, gicv3_driver_data_t *plat_driver_data);
int socfpga_dt_populate_dram_layout(uintptr_t dt_addr);

#endif

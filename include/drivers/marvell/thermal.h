/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

/* Driver for thermal unit located in Marvell ARMADA 8K and compatible SoCs */

#ifndef THERMAL_H
#define THERMAL_H

struct tsen_config {
	/* thermal temperature parameters */
	int tsen_offset;
	int tsen_gain;
	int tsen_divisor;
	/* thermal data */
	int tsen_ready;
	void *regs_base;
	/* thermal functionality */
	int (*ptr_tsen_probe)(struct tsen_config *cfg);
	int (*ptr_tsen_read)(struct tsen_config *cfg, int *temp);
};

/* Thermal driver APIs */
int marvell_thermal_init(struct tsen_config *tsen_cfg);
int marvell_thermal_read(struct tsen_config *tsen_cfg, int *temp);
struct tsen_config *marvell_thermal_config_get(void);

#endif /* THERMAL_H */

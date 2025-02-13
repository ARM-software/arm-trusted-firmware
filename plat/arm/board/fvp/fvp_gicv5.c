/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv5.h>
#include <platform_def.h>

/* wire 26 is the timer interrupt. Will be assigned NS by default */
struct gicv5_wire_props irs0_spis[] = {
};

struct gicv5_wire_props iwb0_wires[] = {
};

struct gicv5_irs irss[] = {{
	.el3_config_frame = BASE_IRS_BASE,
	.spis = irs0_spis,
	.num_spis = ARRAY_SIZE(irs0_spis),
}};

struct gicv5_iwb iwbs[] = {{
	.config_frame = BASE_IWB_BASE,
	.wires = iwb0_wires,
	.num_wires = ARRAY_SIZE(iwb0_wires)
}};

const struct gicv5_driver_data plat_gicv5_driver_data = {
	.irss = irss,
	.iwbs = iwbs,
	.num_irss = ARRAY_SIZE(irss),
	.num_iwbs = ARRAY_SIZE(iwbs)
};

void fvp_gic_driver_pre_init(void)
{
}

void fvp_pcpu_init(void)
{
}

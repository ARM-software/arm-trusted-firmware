/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

struct stm32_gic_instance {
	uint32_t cells;
	uint32_t phandle_node;
};

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t stm32mp1_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

/* Fix target_mask_array as secondary core is not able to initialize it */
static unsigned int target_mask_array[PLATFORM_CORE_COUNT] = {1, 2};

static gicv2_driver_data_t platform_gic_data = {
	.interrupt_props = stm32mp1_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(stm32mp1_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

static struct stm32_gic_instance stm32_gic;

void stm32mp1_gic_init(void)
{
	int node;
	void *fdt;
	const fdt32_t *cuint;
	struct dt_node_info dt_gic;

	if (fdt_get_address(&fdt) == 0) {
		panic();
	}

	node = dt_get_node(&dt_gic, -1, "arm,cortex-a7-gic");
	if (node < 0) {
		panic();
	}

	platform_gic_data.gicd_base = dt_gic.base;

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint == NULL) {
		panic();
	}

	platform_gic_data.gicc_base = fdt32_to_cpu(*(cuint + 2));

	cuint = fdt_getprop(fdt, node, "#interrupt-cells", NULL);
	if (cuint == NULL) {
		panic();
	}

	stm32_gic.cells = fdt32_to_cpu(*cuint);

	stm32_gic.phandle_node = fdt_get_phandle(fdt, node);
	if (stm32_gic.phandle_node == 0U) {
		panic();
	}

	gicv2_driver_init(&platform_gic_data);
	gicv2_distif_init();

	stm32mp1_gic_pcpu_init();
}

void stm32mp1_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

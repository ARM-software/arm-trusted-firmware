/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/gicv2.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <lib/utils.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

struct stm32mp_gic_instance {
	uint32_t cells;
	uint32_t phandle_node;
};

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t stm32mp_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

/* Fix target_mask_array as secondary core is not able to initialize it */
static unsigned int target_mask_array[PLATFORM_CORE_COUNT] = {1, 2};

static gicv2_driver_data_t platform_gic_data = {
	.interrupt_props = stm32mp_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(stm32mp_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

static struct stm32mp_gic_instance stm32mp_gic;

void stm32mp_gic_init(void)
{
	int node;
	void *fdt;
	const fdt32_t *cuint;
	uintptr_t addr;
	int err;

	if (fdt_get_address(&fdt) == 0) {
		panic();
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "arm,cortex-a7-gic");
	if (node < 0) {
		panic();
	}

	err = fdt_get_reg_props_by_index(fdt, node, 0, &addr, NULL);
	if (err < 0) {
		panic();
	}
	platform_gic_data.gicd_base = addr;

	err = fdt_get_reg_props_by_index(fdt, node, 1, &addr, NULL);
	if (err < 0) {
		panic();
	}
	platform_gic_data.gicc_base = addr;

	cuint = fdt_getprop(fdt, node, "#interrupt-cells", NULL);
	if (cuint == NULL) {
		panic();
	}

	stm32mp_gic.cells = fdt32_to_cpu(*cuint);

	stm32mp_gic.phandle_node = fdt_get_phandle(fdt, node);
	if (stm32mp_gic.phandle_node == 0U) {
		panic();
	}

	gicv2_driver_init(&platform_gic_data);
	gicv2_distif_init();

	stm32mp_gic_pcpu_init();
}

void stm32mp_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

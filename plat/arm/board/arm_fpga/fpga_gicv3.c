/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic_common.h>
#include <libfdt.h>

#include <platform_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

static const interrupt_prop_t fpga_interrupt_props[] = {
	PLATFORM_G1S_PROPS(INTR_GROUP1S),
	PLATFORM_G0_PROPS(INTR_GROUP0)
};

static uintptr_t fpga_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int fpga_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

static gicv3_driver_data_t fpga_gicv3_driver_data = {
	.interrupt_props = fpga_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(fpga_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = fpga_rdistif_base_addrs,
	.mpidr_to_core_pos = fpga_mpidr_to_core_pos
};

void plat_fpga_gic_init(void)
{
	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	int node, ret;

	node = fdt_node_offset_by_compatible(fdt, 0, "arm,gic-v3");
	if (node < 0) {
		WARN("No \"arm,gic-v3\" compatible node found in DT, no GIC support.\n");
		return;
	}

	/* TODO: Assuming only empty "ranges;" properties up the bus path. */
	ret = fdt_get_reg_props_by_index(fdt, node, 0,
				 &fpga_gicv3_driver_data.gicd_base, NULL);
	if (ret < 0) {
		WARN("Could not read GIC distributor address from DT.\n");
		return;
	}

	ret = fdt_get_reg_props_by_index(fdt, node, 1,
				 &fpga_gicv3_driver_data.gicr_base, NULL);
	if (ret < 0) {
		WARN("Could not read GIC redistributor address from DT.\n");
		return;
	}

	gicv3_driver_init(&fpga_gicv3_driver_data);
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void fpga_pwr_gic_on_finish(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void fpga_pwr_gic_off(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());
}

unsigned int fpga_get_nr_gic_cores(void)
{
	return gicv3_rdistif_get_number_frames(fpga_gicv3_driver_data.gicr_base);
}

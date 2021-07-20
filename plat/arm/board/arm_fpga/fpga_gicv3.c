/*
 * Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/arm_gicv3_common.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <libfdt.h>

#include <platform_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

static const interrupt_prop_t fpga_interrupt_props[] = {
	PLATFORM_G1S_PROPS(INTR_GROUP1S),
	PLATFORM_G0_PROPS(INTR_GROUP0)
};

static uintptr_t fpga_rdistif_base_addrs[PLATFORM_CORE_COUNT];
static int nr_itses;

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
	uintptr_t gicr_base = 0U;
	uint32_t iidr;
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

	iidr = mmio_read_32(fpga_gicv3_driver_data.gicd_base + GICD_IIDR);
	if (((iidr & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_600) ||
	    ((iidr & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_700)) {
		unsigned int frame_id;

		/*
		 * According to the GIC TRMs, if there are any ITSes, they
		 * start four 64K pages after the distributor. After all
		 * the ITSes then follow the redistributors.
		 */
		gicr_base = fpga_gicv3_driver_data.gicd_base + (4U << 16);

		do {
			uint64_t its_typer;

			/* Each GIC component can be identified by its ID. */
			frame_id = gicv3_get_component_partnum(gicr_base);

			if (frame_id == PIDR_COMPONENT_ARM_REDIST) {
				INFO("Found %d ITSes, redistributors start at 0x%llx\n",
				     nr_itses, (unsigned long long)gicr_base);
				break;
			}

			if (frame_id != PIDR_COMPONENT_ARM_ITS) {
				WARN("GICv3: found unexpected frame 0x%x\n",
					frame_id);
				gicr_base = 0U;
				break;
			}

			/*
			 * Found an ITS, now work out if it supports virtual
			 * SGIs (for direct guest injection). If yes, each
			 * ITS occupies four 64K pages, otherwise just two.
			 */
			its_typer = mmio_read_64(gicr_base + GITS_TYPER);
			if ((its_typer & GITS_TYPER_VSGI) != 0U) {
				gicr_base += 4U << 16;
			} else {
				gicr_base += 2U << 16;
			}
			nr_itses++;
		} while (true);
	}

	/*
	 * If this is not a GIC-600 or -700, or the autodetection above failed,
	 * use the base address from the device tree.
	 */
	if (gicr_base == 0U) {
		ret = fdt_get_reg_props_by_index(fdt, node, 1,
					&fpga_gicv3_driver_data.gicr_base,
					NULL);
		if (ret < 0) {
			WARN("Could not read GIC redistributor address from DT.\n");
			return;
		}
	} else {
		fpga_gicv3_driver_data.gicr_base = gicr_base;
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

uintptr_t fpga_get_redist_size(void)
{
	uint64_t typer_val = mmio_read_64(fpga_gicv3_driver_data.gicr_base +
					  GICR_TYPER);

	return gicv3_redist_size(typer_val);
}

uintptr_t fpga_get_redist_base(void)
{
	return fpga_gicv3_driver_data.gicr_base;
}

bool fpga_has_its(void)
{
	return nr_itses > 0;
}

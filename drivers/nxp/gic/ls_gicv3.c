/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <drivers/arm/gicv3.h>
#include <plat_gic.h>
#include <plat/common/platform.h>

/*
 * NXP common helper to initialize the GICv3 only driver.
 */
void plat_ls_gic_driver_init(uintptr_t nxp_gicd_addr,
			     uintptr_t nxp_gicr_addr,
			     uint8_t plat_core_count,
			     interrupt_prop_t *ls_interrupt_props,
			     uint8_t ls_interrupt_prop_count,
			     uintptr_t *target_mask_array,
			     mpidr_hash_fn mpidr_to_core_pos)
{
	static struct gicv3_driver_data ls_gic_data;

	ls_gic_data.gicd_base = nxp_gicd_addr;
	ls_gic_data.gicr_base = nxp_gicr_addr;
	ls_gic_data.interrupt_props = ls_interrupt_props;
	ls_gic_data.interrupt_props_num = ls_interrupt_prop_count;
	ls_gic_data.rdistif_num = plat_core_count;
	ls_gic_data.rdistif_base_addrs = target_mask_array;
	ls_gic_data.mpidr_to_core_pos = mpidr_to_core_pos;

	gicv3_driver_init(&ls_gic_data);
}

void plat_ls_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/*
 * NXP common helper to enable the GICv3 CPU interface
 */
void plat_ls_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/*
 * NXP common helper to disable the GICv3 CPU interface
 */
void plat_ls_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/*
 * NXP common helper to initialize the per cpu distributor interface in GICv3
 */
void plat_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/*
 * Stubs for Redistributor power management. Although GICv3 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 */
void plat_ls_gic_redistif_on(void)
{
}

void plat_ls_gic_redistif_off(void)
{
}

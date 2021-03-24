/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <gicv2.h>
#include <plat_gic.h>


/*
 * NXP common helper to initialize the GICv3 only driver.
 */
void plat_ls_gic_driver_init(uintptr_t nxp_gicd_addr,
			     uintptr_t nxp_gicc_addr,
			     uint8_t plat_core_count,
			     interrupt_prop_t *ls_interrupt_props,
			     uint8_t ls_interrupt_prop_count,
			     uint32_t *target_mask_array)
{
	static struct gicv2_driver_data ls_gic_data;

	ls_gic_data.gicd_base = nxp_gicd_addr;
	ls_gic_data.gicc_base = nxp_gicc_addr;
	ls_gic_data.target_masks = target_mask_array;
	ls_gic_data.target_masks_num = plat_core_count;
	ls_gic_data.interrupt_props = ls_interrupt_props;
	ls_gic_data.interrupt_props_num = ls_interrupt_prop_count;

	gicv2_driver_init(&ls_gic_data);
}

void plat_ls_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void plat_ls_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void plat_ls_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * NXP common helper to initialize GICv2 per cpu
 *****************************************************************************/
void plat_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * Stubs for Redistributor power management. Although GICv2 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 *****************************************************************************/
void plat_ls_gic_redistif_on(void)
{
}

void plat_ls_gic_redistif_off(void)
{
}

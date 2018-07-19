/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <gicv2.h>
#include <plat_marvell.h>
#include <platform.h>
#include <platform_def.h>

/*
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv2 driver is initialised and used.
 */
#pragma weak plat_marvell_gic_driver_init
#pragma weak plat_marvell_gic_init

/*
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 */
static const interrupt_prop_t marvell_interrupt_props[] = {
	PLAT_MARVELL_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
	PLAT_MARVELL_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

/*
 * Ideally `marvell_gic_data` structure definition should be a `const` but it is
 * kept as modifiable for overwriting with different GICD and GICC base when
 * running on FVP with VE memory map.
 */
static gicv2_driver_data_t marvell_gic_data = {
	.gicd_base = PLAT_MARVELL_GICD_BASE,
	.gicc_base = PLAT_MARVELL_GICC_BASE,
	.interrupt_props = marvell_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(marvell_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*
 * ARM common helper to initialize the GICv2 only driver.
 */
void plat_marvell_gic_driver_init(void)
{
	gicv2_driver_init(&marvell_gic_data);
}

void plat_marvell_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

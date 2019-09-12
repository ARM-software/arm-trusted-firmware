/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "aml_private.h"

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl33_image_ep_info;
static image_info_t bl30_image_info;
static image_info_t bl301_image_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(type == NON_SECURE);

	next_image_info = &bl33_image_ep_info;

	/* None of the images can have 0x0 as the entrypoint. */
	if (next_image_info->pc != 0U) {
		return next_image_info;
	} else {
		return NULL;
	}
}

/*******************************************************************************
 * Perform any BL31 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
struct gxl_bl31_param {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
	image_info_t *scp_image_info[];
};

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	struct gxl_bl31_param *from_bl2;

	/* Initialize the console to provide early debug support */
	aml_console_init();

	/* Check that params passed from BL2 are not NULL. */
	from_bl2 = (struct gxl_bl31_param *) arg0;

	/* Check params passed from BL2 are not NULL. */
	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	/*
	 * Copy BL33 entry point information. It is stored in Secure RAM, in
	 * BL2's address space.
	 */
	bl33_image_ep_info = *from_bl2->bl33_ep_info;

	if (bl33_image_ep_info.pc == 0U) {
		ERROR("BL31: BL33 entrypoint not obtained from BL2\n");
		panic();
	}

	bl30_image_info = *from_bl2->scp_image_info[0];
	bl301_image_info = *from_bl2->scp_image_info[1];
}

void bl31_plat_arch_setup(void)
{
	aml_setup_page_tables();

	enable_mmu_el3(0);
}

static inline bool gxl_scp_ready(void)
{
	return AML_AO_RTI_SCP_IS_READY(mmio_read_32(AML_AO_RTI_SCP_STAT));
}

static inline void gxl_scp_boot(void)
{
	aml_scpi_upload_scp_fw(bl30_image_info.image_base,
			       bl30_image_info.image_size, 0);
	aml_scpi_upload_scp_fw(bl301_image_info.image_base,
			       bl301_image_info.image_size, 1);
	while (!gxl_scp_ready())
		;
}

/*******************************************************************************
 * GICv2 driver setup information
 ******************************************************************************/
static const interrupt_prop_t gxl_interrupt_props[] = {
	INTR_PROP_DESC(IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
};

static const gicv2_driver_data_t gxl_gic_data = {
	.gicd_base = AML_GICD_BASE,
	.gicc_base = AML_GICC_BASE,
	.interrupt_props = gxl_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(gxl_interrupt_props),
};

void bl31_platform_setup(void)
{
	aml_mhu_secure_init();

	gicv2_driver_init(&gxl_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	gxl_scp_boot();

	aml_thermal_unknown();
}

/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <drivers/arm/gicv2.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables.h>

#include "ccu/ncore_ccu.h"
#include "socfpga_mailbox.h"
#include "socfpga_private.h"
#include "socfpga_sip_svc.h"

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ?
			  &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

void setup_smmu_secure_context(void)
{
	/*
	 * Program SCR0 register (0xFA000000)
	 * to set SMCFCFG bit[21] to 0x1 which raise stream match conflict fault
	 * to set CLIENTPD bit[0] to 0x0 which enables SMMU for secure context
	 */
	mmio_write_32(0xFA000000, 0x00200000);

	/*
	 * Program SCR1 register (0xFA000004)
	 * to set NSNUMSMRGO bit[14:8] to 0x4 which stream mapping register
	 * for non-secure context and the rest will be secure context
	 * to set NSNUMCBO bit[5:0] to 0x4 which allocate context bank
	 * for non-secure context and the rest will be secure context
	 */
	mmio_write_32(0xFA000004, 0x00000404);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	mmio_write_64(PLAT_SEC_ENTRY, PLAT_SEC_WARM_ENTRY);

	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
		PLAT_BAUDRATE, &console);
	/*
	 * Check params passed from BL31 should not be NULL,
	 */
	void *from_bl2 = (void *) arg0;

	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);

	/*
	 * Copy BL32 (if populated by BL31) and BL33 entry point information.
	 * They are stored in Secure RAM, in BL31's address space.
	 */

	if (params_from_bl2->h.type == PARAM_BL_PARAMS &&
		params_from_bl2->h.version >= VERSION_2) {

		bl_params_node_t *bl_params = params_from_bl2->head;

		while (bl_params) {
			if (bl_params->image_id == BL33_IMAGE_ID)
				bl33_image_ep_info = *bl_params->ep_info;

			bl_params = bl_params->next_params_info;
		}
	} else {
		struct socfpga_bl31_params *arg_from_bl2 =
			(struct socfpga_bl31_params *) from_bl2;

		assert(arg_from_bl2->h.type == PARAM_BL31);
		assert(arg_from_bl2->h.version >= VERSION_1);

		bl32_image_ep_info = *arg_from_bl2->bl32_ep_info;
		bl33_image_ep_info = *arg_from_bl2->bl33_ep_info;
	}
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

static const interrupt_prop_t s10_interrupt_props[] = {
	PLAT_INTEL_SOCFPGA_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
	PLAT_INTEL_SOCFPGA_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static const gicv2_driver_data_t plat_gicv2_gic_data = {
	.gicd_base = PLAT_INTEL_SOCFPGA_GICD_BASE,
	.gicc_base = PLAT_INTEL_SOCFPGA_GICC_BASE,
	.interrupt_props = s10_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(s10_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	socfpga_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&plat_gicv2_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
	setup_smmu_secure_context();

	/* Signal secondary CPUs to jump to BL31 (BL2 = U-boot SPL) */
	mmio_write_64(PLAT_CPU_RELEASE_ADDR,
		(uint64_t)plat_secondary_cpus_bl31_entry);

	mailbox_hps_stage_notify(HPS_EXECUTION_STATE_SSBL);

	ncore_enable_ocram_firewall();
}

const mmap_region_t plat_agilex_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE2_BASE, DEVICE2_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
		MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE3_BASE, DEVICE3_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE4_BASE, DEVICE4_SIZE, MT_DEVICE | MT_RW | MT_NS),
	{0}
};

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE),
#if USE_COHERENT_MEM
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_agilex_mmap);
	enable_mmu_el3(0);
}


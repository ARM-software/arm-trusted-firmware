/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_console.h>

#include <custom_svc.h>
#include <plat_fdt.h>
#include <plat_private.h>
#include <plat_startup.h>
#include <zynqmp_def.h>


static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 */
struct entry_point_info *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	if (type == NON_SECURE) {
		next_image_info = &bl33_image_ep_info;
	} else {
		next_image_info = &bl32_image_ep_info;
	}

	return next_image_info;
}

/*
 * Set the build time defaults. We want to do this when doing a JTAG boot
 * or if we can't find any other config data.
 */
static inline void bl31_set_default_config(void)
{
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);
}

/*
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 */
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint64_t tfa_handoff_addr;

	setup_console();

	/* Initialize the platform config for future decision making */
	zynqmp_config_setup();

	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base ZYNQMP only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */

	/* Populate common information for BL32 and BL33 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	tfa_handoff_addr = mmio_read_32(PMU_GLOBAL_GEN_STORAGE6);

	if (zynqmp_get_bootmode() == ZYNQMP_BOOTMODE_JTAG) {
		bl31_set_default_config();
	} else {
		/* use parameters from XBL */
		enum xbl_handoff ret = xbl_handover(&bl32_image_ep_info,
							  &bl33_image_ep_info,
							  tfa_handoff_addr);
		if (ret != XBL_HANDOFF_SUCCESS) {
			panic();
		}
	}
	if (bl32_image_ep_info.pc != 0) {
		NOTICE("BL31: Secure code at 0x%lx\n", bl32_image_ep_info.pc);
	}
	if (bl33_image_ep_info.pc != 0) {
		NOTICE("BL31: Non secure code at 0x%lx\n", bl33_image_ep_info.pc);
	}

	custom_early_setup();

}

#if ZYNQMP_WDT_RESTART
static zynmp_intr_info_type_el3_t type_el3_interrupt_table[MAX_INTR_EL3];

int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler)
{
	static uint32_t index;
	uint32_t i;

	/* Validate 'handler' and 'id' parameters */
	if (!handler || index >= MAX_INTR_EL3) {
		return -EINVAL;
	}

	/* Check if a handler has already been registered */
	for (i = 0; i < index; i++) {
		if (id == type_el3_interrupt_table[i].id) {
			return -EALREADY;
		}
	}

	type_el3_interrupt_table[index].id = id;
	type_el3_interrupt_table[index].handler = handler;

	index++;

	return 0;
}

static uint64_t rdo_el3_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	uint32_t intr_id;
	uint32_t i;
	interrupt_type_handler_t handler = NULL;

	intr_id = plat_ic_get_pending_interrupt_id();

	for (i = 0; i < MAX_INTR_EL3; i++) {
		if (intr_id == type_el3_interrupt_table[i].id) {
			handler = type_el3_interrupt_table[i].handler;
		}
	}

	if (handler != NULL) {
		return handler(intr_id, flags, handle, cookie);
	}

	return 0;
}
#endif

void bl31_platform_setup(void)
{
	prepare_dtb();

	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
}

void bl31_plat_runtime_setup(void)
{
#if ZYNQMP_WDT_RESTART
	uint64_t flags = 0;
	uint64_t rc;

	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					     rdo_el3_interrupt_handler, flags);
	if (rc) {
		panic();
	}
#endif

	custom_runtime_setup();
}

/*
 * Perform the very early platform specific architectural setup here.
 */
void bl31_plat_arch_setup(void)
{
	plat_arm_interconnect_init();
	plat_arm_interconnect_enter_coherency();

	const mmap_region_t bl_regions[] = {
#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
		MAP_REGION_FLAT(XILINX_OF_BOARD_DTB_ADDR, XILINX_OF_BOARD_DTB_MAX_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
#endif
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
				MT_DEVICE | MT_RW | MT_SECURE),
		{0}
	};

	custom_mmap_add();

	setup_page_tables(bl_regions, plat_get_mmap());
	enable_mmu(0);
}

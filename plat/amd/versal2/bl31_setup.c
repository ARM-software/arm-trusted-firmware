/*
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/dcc.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <plat_arm.h>
#include <plat_console.h>
#include <scmi.h>

#include <def.h>
#include <plat_fdt.h>
#include <plat_private.h>
#include <plat_startup.h>
#include <plat_xfer_list.h>
#include <pm_api_sys.h>
#include <pm_client.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 */
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));

	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	}

	return &bl32_image_ep_info;
}

/*
 * Set the build time defaults,if we can't find any config data.
 */
static inline void bl31_set_default_config(void)
{
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();
#if defined(SPD_opteed)
	/* NS dtb addr passed to optee_os */
	bl32_image_ep_info.args.arg3 = XILINX_OF_BOARD_DTB_ADDR;
#endif
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);
}

/*
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
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
	uint32_t uart_clock;
	int32_t rc;

	board_detection();

	/* FIXME */
	switch (platform_id) {
	case SPP:
		switch (platform_version) {
		case SPP_PSXC_MMI_V2_0:
			cpu_clock = 770000;
			break;
		case SPP_PSXC_MMI_V3_0:
			cpu_clock = 908000;
			break;
		default:
			panic();
		}
		break;
	case SPP_MMD:
		switch (platform_version) {
		case SPP_PSXC_ISP_AIE_V2_0:
		case SPP_PSXC_MMD_AIE_FRZ_EA:
		case SPP_PSXC_MMD_AIE_V3_0:
			cpu_clock = 760000;
			break;
		default:
			panic();
		}
		break;
	case EMU:
	case EMU_MMD:
		cpu_clock = 112203;
		break;
	case QEMU:
		/* Random values now */
		cpu_clock = 3333333;
		break;
	case SILICON:
		cpu_clock = 100000000;
		break;
	default:
		panic();
	}

	uart_clock = get_uart_clk();

	setup_console();

	NOTICE("TF-A running on %s %d.%d\n", board_name_decode(),
	       platform_version / 10U, platform_version % 10U);

	/* Initialize the platform config for future decision making */
	config_setup();

	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */

	/* Populate common information for BL32 and BL33 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	rc = transfer_list_populate_ep_info(&bl32_image_ep_info, &bl33_image_ep_info);
	if (rc == TL_OPS_NON || rc == TL_OPS_CUS) {
		NOTICE("BL31: TL not found, using default config\n");
		bl31_set_default_config();
	}

	long rev_var = cpu_get_rev_var();

	INFO("CPU Revision = 0x%lx\n", rev_var);
	INFO("cpu_clock = %dHz, uart_clock = %dHz\n", cpu_clock, uart_clock);
	NOTICE("BL31: Executing from 0x%x\n", BL31_BASE);
	NOTICE("BL31: Secure code at 0x%lx\n", bl32_image_ep_info.pc);
	NOTICE("BL31: Non secure code at 0x%lx\n", bl33_image_ep_info.pc);

}

static versal_intr_info_type_el3_t type_el3_interrupt_table[MAX_INTR_EL3];

int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler)
{
	static uint32_t index;
	uint32_t i;

	/* Validate 'handler' and 'id' parameters */
	if ((handler == NULL) || (index >= MAX_INTR_EL3)) {
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
	(void)id;
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
		(void)handler(intr_id, flags, handle, cookie);
	}

	return 0;
}

void bl31_platform_setup(void)
{
	prepare_dtb();

	/* Initialize the gic cpu and distributor interfaces */
	plat_gic_driver_init();
	plat_gic_init();

	if (platform_id != EMU) {
		init_scmi_server();
	}
}

void bl31_plat_runtime_setup(void)
{
	uint64_t flags = 0;
	int32_t rc;

	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					     rdo_el3_interrupt_handler, flags);
	if (rc != 0) {
		panic();
	}

	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

/*
 * Perform the very early platform specific architectural setup here.
 */
void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
		MAP_REGION_FLAT(SMT_BUFFER_BASE, 0x1000,
			MT_DEVICE | MT_RW | MT_NON_CACHEABLE | MT_EXECUTE_NEVER | MT_NS),
		{0}
	};

	setup_page_tables(bl_regions, plat_get_mmap());
	enable_mmu(0);
}

/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "agilex5_power_manager.h"
#include "ccu/ncore_ccu.h"
#include "socfpga_mailbox.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"

/* Get non-secure SPSR for BL33. Zephyr and Linux */
uint32_t arm_get_spsr_for_bl33_entry(void);

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

#define SMMU_SDMMC

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

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	mmio_write_64(PLAT_SEC_ENTRY, PLAT_SEC_WARM_ENTRY);

	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
	PLAT_BAUDRATE, &console);

	init_ncore_ccu();
	setup_smmu_stream_id();

	/*
	 * Check params passed from BL31 should not be NULL,
	 */
	void *from_bl2 = (void *) arg0;

#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	void *plat_params_from_bl2 = (void *) arg3;

	assert(plat_params_from_bl2 == NULL);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);

# if ARM_LINUX_KERNEL_AS_BL33
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	bl33_image_ep_info.args.arg0 = (u_register_t)ARM_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
# endif

#else /* RESET_TO_BL31 */
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
			if (bl_params->image_id == BL33_IMAGE_ID) {
				bl33_image_ep_info = *bl_params->ep_info;
			}
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

	bl33_image_ep_info.args.arg0 = (u_register_t)ARM_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
#endif

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();

	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

static const interrupt_prop_t agx5_interrupt_props[] = {
	PLAT_INTEL_SOCFPGA_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_INTEL_SOCFPGA_G0_IRQ_PROPS(INTR_GROUP0)
};

static const gicv3_driver_data_t plat_gicv3_gic_data = {
	.gicd_base = PLAT_INTEL_SOCFPGA_GICD_BASE,
	.gicr_base = PLAT_INTEL_SOCFPGA_GICR_BASE,
	.interrupt_props = agx5_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(agx5_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
};

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	socfpga_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	gicv3_driver_init(&plat_gicv3_gic_data);
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
	mailbox_hps_stage_notify(HPS_EXECUTION_STATE_SSBL);
}

const mmap_region_t plat_agilex_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(PSS_BASE, PSS_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MPFE_BASE, MPFE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE, MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CCU_BASE, CCU_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GIC_BASE, GIC_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	uint32_t boot_core = 0x00;
	uint32_t cpuid = 0x00;

	cpuid = read_mpidr();
	boot_core = (mmio_read_32(AGX5_PWRMGR(MPU_BOOTCONFIG)) & 0xC00);
	NOTICE("BL31: Boot Core = %x\n", boot_core);
	NOTICE("BL31: CPU ID = %x\n", cpuid);

}

/* Get non-secure image entrypoint for BL33. Zephyr and Linux */
uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_NS_IMAGE_OFFSET;
#endif
}

/* Get non-secure SPSR for BL33. Zephyr and Linux */
uint32_t arm_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64((uint64_t)mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/* SMP: Secondary cores BL31 setup reset vector */
void bl31_plat_set_secondary_cpu_entrypoint(unsigned int cpu_id)
{
	unsigned int pch_cpu = 0x00;
	unsigned int pchctlr_old = 0x00;
	unsigned int pchctlr_new = 0x00;
	uint32_t boot_core = 0x00;

	boot_core = (mmio_read_32(AGX5_PWRMGR(MPU_BOOTCONFIG)) & 0xC00);
	/* Update the p-channel based on cpu id */
	pch_cpu = 1 << cpu_id;

	if (boot_core == 0x00) {
		/* Update reset vector to 0x00 */
		mmio_write_64(RSTMGR_CPUxRESETBASELOW_CPU2,
(uint64_t) plat_secondary_cpus_bl31_entry >> 2);
	} else {
		/* Update reset vector to 0x00 */
		mmio_write_64(RSTMGR_CPUxRESETBASELOW_CPU0,
(uint64_t) plat_secondary_cpus_bl31_entry >> 2);
	}

	/* Update reset vector to 0x00 */
	mmio_write_64(RSTMGR_CPUxRESETBASELOW_CPU1, (uint64_t) plat_secondary_cpus_bl31_entry >> 2);
	mmio_write_64(RSTMGR_CPUxRESETBASELOW_CPU3, (uint64_t) plat_secondary_cpus_bl31_entry >> 2);

	/* On all cores - temporary */
	pchctlr_old = mmio_read_32(AGX5_PWRMGR(MPU_PCHCTLR));
	pchctlr_new = pchctlr_old | (pch_cpu<<1);
	mmio_write_32(AGX5_PWRMGR(MPU_PCHCTLR), pchctlr_new);

	/* We will only release the target secondary CPUs */
	/* Bit mask for each CPU BIT0-3 */
	mmio_write_32(RSTMGR_CPUSTRELEASE_CPUx, pch_cpu);
}

void bl31_plat_set_secondary_cpu_off(void)
{
	unsigned int pch_cpu = 0x00;
	unsigned int pch_cpu_off = 0x00;
	unsigned int cpu_id = plat_my_core_pos();

	pch_cpu_off = 1 << cpu_id;

	pch_cpu = mmio_read_32(AGX5_PWRMGR(MPU_PCHCTLR));
	pch_cpu = pch_cpu & ~(pch_cpu_off << 1);

	mmio_write_32(AGX5_PWRMGR(MPU_PCHCTLR), pch_cpu);
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: Enable mmu when needed */
}

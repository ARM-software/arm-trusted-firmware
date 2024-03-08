/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <rpi_shared.h>

/*
 * Fields at the beginning of armstub8.bin.
 * While building the BL31 image, we put the stub magic into the binary.
 * The GPU firmware detects this at boot time, clears that field as a
 * confirmation and puts the kernel and DT address in the following words.
 */
extern uint32_t stub_magic;
extern uint32_t dtb_ptr32;
extern uint32_t kernel_entry32;

static const gicv2_driver_data_t rpi4_gic_data = {
	.gicd_base = RPI4_GIC_GICD_BASE,
	.gicc_base = RPI4_GIC_GICC_BASE,
};

/*
 * To be filled by the code below. At the moment BL32 is not supported.
 * In the future these might be passed down from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type) != 0);

	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images can have 0x0 as the entrypoint. */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	/* Cleared by the GPU if kernel address is valid. */
	if (stub_magic == 0)
		return kernel_entry32;

	WARN("Stub magic failure, using default kernel address 0x80000\n");
	return 0x80000;
#endif
}

uintptr_t rpi4_get_dtb_address(void)
{
#ifdef RPI3_PRELOADED_DTB_BASE
	return RPI3_PRELOADED_DTB_BASE;
#else
	/* Cleared by the GPU if DTB address is valid. */
	if (stub_magic == 0)
		return dtb_ptr32;

	WARN("Stub magic failure, DTB address unknown\n");
	return 0;
#endif
}

static void ldelay(register_t delay)
{
	__asm__ volatile (
		"1:\tcbz %0, 2f\n\t"
		"sub %0, %0, #1\n\t"
		"b 1b\n"
		"2:"
		: "=&r" (delay) : "0" (delay)
	);
}

/*******************************************************************************
 * Perform any BL31 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)

{
	/*
	 * LOCAL_CONTROL:
	 * Bit 9 clear: Increment by 1 (vs. 2).
	 * Bit 8 clear: Timer source is 19.2MHz crystal (vs. APB).
	 */
	mmio_write_32(RPI4_LOCAL_CONTROL_BASE_ADDRESS, 0);

	/* LOCAL_PRESCALER; divide-by (0x80000000 / register_val) == 1 */
	mmio_write_32(RPI4_LOCAL_CONTROL_PRESCALER, 0x80000000);

	/* Early GPU firmware revisions need a little break here. */
	ldelay(100000);

	/* Initialize the console to provide early debug support. */
	rpi3_console_init();

	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = rpi3_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if RPI3_DIRECT_LINUX_BOOT
# if RPI3_BL33_IN_AARCH32
	/*
	 * According to the file ``Documentation/arm/Booting`` of the Linux
	 * kernel tree, Linux expects:
	 * r0 = 0
	 * r1 = machine type number, optional in DT-only platforms (~0 if so)
	 * r2 = Physical address of the device tree blob
	 */
	VERBOSE("rpi: Preparing to boot 32-bit Linux kernel\n");
	bl33_image_ep_info.args.arg0 = 0U;
	bl33_image_ep_info.args.arg1 = ~0U;
	bl33_image_ep_info.args.arg2 = rpi4_get_dtb_address();
# else
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	VERBOSE("rpi: Preparing to boot 64-bit Linux kernel\n");
	bl33_image_ep_info.args.arg0 = rpi4_get_dtb_address();
	bl33_image_ep_info.args.arg1 = 0ULL;
	bl33_image_ep_info.args.arg2 = 0ULL;
	bl33_image_ep_info.args.arg3 = 0ULL;
# endif /* RPI3_BL33_IN_AARCH32 */
#endif /* RPI3_DIRECT_LINUX_BOOT */
}

void bl31_plat_arch_setup(void)
{
	/*
	 * Is the dtb_ptr32 pointer valid? If yes, map the DTB region.
	 * We map the 2MB region the DTB start address lives in, plus
	 * the next 2MB, to have enough room for expansion.
	 */
	if (stub_magic == 0) {
		unsigned long long dtb_region = dtb_ptr32;

		dtb_region &= ~0x1fffff;	/* Align to 2 MB. */
		mmap_add_region(dtb_region, dtb_region, 4U << 20,
				MT_MEMORY | MT_RW | MT_NS);
	}
	/*
	 * Add the first page of memory, which holds the stub magic,
	 * the kernel and the DT address.
	 * This also holds the secondary CPU's entrypoints and mailboxes.
	 */
	mmap_add_region(0, 0, 4096, MT_NON_CACHEABLE | MT_RW | MT_SECURE);

	rpi3_setup_page_tables(BL31_BASE, BL31_END - BL31_BASE,
			       BL_CODE_BASE, BL_CODE_END,
			       BL_RO_DATA_BASE, BL_RO_DATA_END
#if USE_COHERENT_MEM
			       , BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END
#endif
			      );

	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	/* Configure the interrupt controller */
	gicv2_driver_init(&rpi4_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	plat_rpi_bl31_custom_setup();
}

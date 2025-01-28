/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <k3_console.h>
#include <k3_gicv3.h>
#include <ti_sci.h>

#define ADDR_DOWN(_adr) (_adr & XLAT_ADDR_MASK(2U))
#define SIZE_UP(_adr, _sz) (round_up((_adr + _sz), XLAT_BLOCK_SIZE(2U)) - ADDR_DOWN(_adr))

#define K3_MAP_REGION_FLAT(_adr, _sz, _attr) \
	MAP_REGION_FLAT(ADDR_DOWN(_adr), SIZE_UP(_adr, _sz), _attr)

/* Table of regions to map using the MMU */
const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(K3_USART_BASE,       K3_USART_SIZE,       MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GIC_BASE,         K3_GIC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GTC_BASE,         K3_GTC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_RT_BASE,   SEC_PROXY_RT_SIZE,   MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_SCFG_BASE, SEC_PROXY_SCFG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_DATA_BASE, SEC_PROXY_DATA_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

/*
 * Placeholder variables for maintaining information about the next image(s)
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
static uint32_t k3_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup, such as console init and deciding on
 * memory layout.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	k3_console_setup();

#ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
#endif

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	bl33_image_ep_info.pc = PRELOADED_BL33_BASE;
	bl33_image_ep_info.spsr = k3_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#ifdef K3_HW_CONFIG_BASE
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	bl33_image_ep_info.args.arg0 = (u_register_t)K3_HW_CONFIG_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
#endif
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_START,           BL31_SIZE,			          MT_MEMORY  | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE,         BL_CODE_END         - BL_CODE_BASE,         MT_CODE    | MT_RO | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,      BL_RO_DATA_END      - BL_RO_DATA_BASE,      MT_RO_DATA | MT_RO | MT_SECURE),
#if USE_COHERENT_MEM
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE, MT_DEVICE  | MT_RW | MT_SECURE),
#endif
		{ /* sentinel */ }
	};

	setup_page_tables(bl_regions, plat_k3_mmap);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	struct ti_sci_msg_version version;
	int ret;

	k3_gic_driver_init(K3_GIC_BASE);
	k3_gic_init();

	ti_sci_boot_notification();

	ret = ti_sci_get_revision(&version);
	if (ret) {
		ERROR("Unable to communicate with the control firmware (%d)\n", ret);
		return;
	}

	INFO("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	     version.abi_major, version.abi_minor,
	     version.firmware_revision,
	     version.firmware_description);

	/*
	 * Older firmware have a timing issue with DM that crashes few TF-A
	 * lite devices while trying to make calls to DM. Since there is no way
	 * to detect what current DM version we are running - we rely on the
	 * corresponding TIFS versioning to handle this check and ensure that
	 * the platform boots up
	 *
	 * Upgrading to TIFS version 9.1.7 along with the corresponding DM from
	 * ti-linux-firmware will enable this functionality.
	 */
	if (version.firmware_revision > 9 ||
	    (version.firmware_revision == 9 && version.sub_version > 1) ||
	    (version.firmware_revision == 9 && version.sub_version == 1 &&
		 version.patch_version >= 7)
	) {
		if (ti_sci_device_get(PLAT_BOARD_DEVICE_ID)) {
			WARN("Unable to take system power reference\n");
		}
	} else {
		NOTICE("Upgrade Firmwares for Power off functionality\n");
	}
}

void platform_mem_init(void)
{
	/* Do nothing for now... */
}

unsigned int plat_get_syscnt_freq2(void)
{
	uint32_t gtc_freq;
	uint32_t gtc_ctrl;

	/* Lets try and provide basic diagnostics - cost is low */
	gtc_ctrl = mmio_read_32(K3_GTC_BASE + K3_GTC_CNTCR_OFFSET);
	/* Did the bootloader fail to enable timer and OS guys are confused? */
	if ((gtc_ctrl & K3_GTC_CNTCR_EN_MASK) == 0U) {
		ERROR("GTC is disabled! Timekeeping broken. Fix Bootloader\n");
	}
	/*
	 * If debug will not pause time, we will have issues like
	 * drivers timing out while debugging, in cases of OS like Linux,
	 * RCU stall errors, which can be hard to differentiate vs real issues.
	 */
	if ((gtc_ctrl & K3_GTC_CNTCR_HDBG_MASK) == 0U) {
		WARN("GTC: Debug access doesn't stop time. Fix Bootloader\n");
	}

	gtc_freq = mmio_read_32(K3_GTC_BASE + K3_GTC_CNTFID0_OFFSET);
	/* Many older bootloaders may have missed programming FID0 register */
	if (gtc_freq != 0U) {
		return gtc_freq;
	}

	/*
	 * We could have just warned about this, but this can have serious
	 * hard to debug side effects if we are NOT sure what the actual
	 * frequency is. Lets make sure people don't miss this.
	 */
	ERROR("GTC_CNTFID0 is 0! Assuming %d Hz. Fix Bootloader\n",
	      SYS_COUNTER_FREQ_IN_TICKS);

	return SYS_COUNTER_FREQ_IN_TICKS;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image
 * for the security state specified. BL3-3 corresponds to the non-secure
 * image type while BL3-2 corresponds to the secure image type. A NULL
 * pointer is returned if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE) ? &bl33_image_ep_info :
						 &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;

	NOTICE("Requested nonexistent image\n");
	return NULL;
}

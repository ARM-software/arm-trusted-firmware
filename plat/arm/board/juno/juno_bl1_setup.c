/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/css/sds.h>
#include <drivers/arm/sp805.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/arm_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

void juno_reset_to_aarch32_state(void);

static int is_watchdog_reset(void)
{
#if !CSS_USE_SCMI_SDS_DRIVER
	#define RESET_REASON_WDOG_RESET		(0x2)
	const uint32_t *reset_flags_ptr = (const uint32_t *)SSC_GPRETN;

	if ((*reset_flags_ptr & RESET_REASON_WDOG_RESET) != 0)
		return 1;

	return 0;
#else
	int ret;
	uint32_t scp_reset_synd_flags;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SCP SDS initialization failed\n");
		panic();
	}

	ret = sds_struct_read(SDS_RESET_SYNDROME_STRUCT_ID,
					SDS_RESET_SYNDROME_OFFSET,
					&scp_reset_synd_flags,
					SDS_RESET_SYNDROME_SIZE,
					SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Getting reset reason from SDS failed\n");
		panic();
	}

	/* Check if the WATCHDOG_RESET_BIT is set in the reset syndrome */
	if (scp_reset_synd_flags & SDS_RESET_SYNDROME_AP_WD_RESET_BIT)
		return 1;

	return 0;
#endif
}

/*******************************************************************************
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or watchdog reset happened.
 ******************************************************************************/
int plat_arm_bl1_fwu_needed(void)
{
	const int32_t *nv_flags_ptr = (const int32_t *)V2M_SYS_NVFLAGS_ADDR;

	/* Check if TOC is invalid or watchdog reset happened. */
	if ((arm_io_is_toc_valid() != 1) ||
		(((*nv_flags_ptr == -EAUTH) || (*nv_flags_ptr == -ENOENT))
		&& is_watchdog_reset()))
		return 1;

	return 0;
}

/*******************************************************************************
 * On JUNO update the arg2 with address of SCP_BL2U image info.
 ******************************************************************************/
void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{
	if (image_id == BL2U_IMAGE_ID) {
		image_desc_t *image_desc = bl1_plat_get_image_desc(SCP_BL2U_IMAGE_ID);
		ep_info->args.arg2 = (unsigned long)&image_desc->image_info;
	}
}

/*******************************************************************************
 * On Juno clear SYS_NVFLAGS and wait for watchdog reset.
 ******************************************************************************/
__dead2 void bl1_plat_fwu_done(void *client_cookie, void *reserved)
{
	unsigned int *nv_flags_clr = (unsigned int *)
			(V2M_SYSREGS_BASE + V2M_SYS_NVFLAGSCLR);
	unsigned int *nv_flags_ptr = (unsigned int *)
			(V2M_SYSREGS_BASE + V2M_SYS_NVFLAGS);

	/* Clear the NV flags register. */
	*nv_flags_clr = *nv_flags_ptr;

	while (1)
		wfi();
}

#if JUNO_AARCH32_EL3_RUNTIME
void bl1_plat_prepare_exit(entry_point_info_t *ep_info)
{
#if !ARM_DISABLE_TRUSTED_WDOG
	/* Disable watchdog before leaving BL1 */
	sp805_stop(ARM_SP805_TWDG_BASE);
#endif

	juno_reset_to_aarch32_state();
}
#endif /* JUNO_AARCH32_EL3_RUNTIME */

void plat_arm_secure_wdt_start(void)
{
	sp805_start(ARM_SP805_TWDG_BASE, ARM_TWDG_LOAD_VAL);
}

void plat_arm_secure_wdt_stop(void)
{
	sp805_stop(ARM_SP805_TWDG_BASE);
}

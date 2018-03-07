/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform.h>
#include <sp805.h>
#include <tbbr_img_def.h>
#include <v2m_def.h>

void juno_reset_to_aarch32_state(void);

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

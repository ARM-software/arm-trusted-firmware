/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bl_common.h>
#include <errno.h>
#include <platform.h>
#include <plat_arm.h>
#include <tbbr_img_def.h>
#include <v2m_def.h>

#define RESET_REASON_WDOG_RESET		(0x2)

/*******************************************************************************
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or watchdog reset happened.
 ******************************************************************************/
unsigned int bl1_plat_get_next_image_id(void)
{
	unsigned int *reset_flags_ptr = (unsigned int *)SSC_GPRETN;
	unsigned int *nv_flags_ptr = (unsigned int *)
			(V2M_SYSREGS_BASE + V2M_SYS_NVFLAGS);
	/*
	 * Check if TOC is invalid or watchdog reset happened.
	 */
	if ((arm_io_is_toc_valid() != 1) ||
		((*reset_flags_ptr & RESET_REASON_WDOG_RESET) &&
		((*nv_flags_ptr == -EAUTH) || (*nv_flags_ptr == -ENOENT))))
		return NS_BL1U_IMAGE_ID;

	return BL2_IMAGE_ID;
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

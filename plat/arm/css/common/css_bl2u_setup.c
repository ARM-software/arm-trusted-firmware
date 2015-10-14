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
#include <debug.h>
#include <plat_arm.h>
#include "css_scp_bootloader.h"

/* Weak definition may be overridden in specific CSS based platform */
#pragma weak bl2u_plat_handle_scp_bl2u

/* Data structure which holds the SCP_BL2U image info for BL2U */
static image_info_t scp_bl2u_image_info;

/*******************************************************************************
 * BL1 can pass platform dependent information to BL2U in x1.
 * In case of ARM CSS platforms x1 contains SCP_BL2U image info.
 * In case of ARM FVP platforms x1 is not used.
 * In both cases, x0 contains the extents of the memory available to BL2U
 ******************************************************************************/
void bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	if (!plat_info)
		panic();

	arm_bl2u_early_platform_setup(mem_layout, plat_info);

	scp_bl2u_image_info = *(image_info_t *)plat_info;
}

/*******************************************************************************
 * Transfer SCP_BL2U from Trusted RAM using the SCP Download protocol.
 ******************************************************************************/
int bl2u_plat_handle_scp_bl2u(void)
{
	int ret;

	INFO("BL2U: Initiating SCP_BL2U transfer to SCP\n");

	ret = scp_bootloader_transfer((void *)scp_bl2u_image_info.image_base,
		scp_bl2u_image_info.image_size);

	if (ret == 0)
		INFO("BL2U: SCP_BL2U transferred to SCP\n");
	else
		ERROR("BL2U: SCP_BL2U transfer failure\n");

	return ret;
}

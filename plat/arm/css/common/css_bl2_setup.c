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
#include <css_def.h>
#include <debug.h>
#include <mmio.h>
#include <plat_arm.h>
#include <string.h>
#include "css_scp_bootloader.h"

/* Weak definition may be overridden in specific CSS based platform */
#if LOAD_IMAGE_V2
#pragma weak plat_arm_bl2_handle_scp_bl2
#else
#pragma weak bl2_plat_handle_scp_bl2
#endif

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
#if LOAD_IMAGE_V2
int plat_arm_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
#else
int bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
#endif
{
	int ret;

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");

	ret = scp_bootloader_transfer((void *)scp_bl2_image_info->image_base,
		scp_bl2_image_info->image_size);

	if (ret == 0)
		INFO("BL2: SCP_BL2 transferred to SCP\n");
	else
		ERROR("BL2: SCP_BL2 transfer failure\n");

	return ret;
}

#ifdef EL3_PAYLOAD_BASE
/*
 * We need to override some of the platform functions when booting an EL3
 * payload.
 */

static unsigned int scp_boot_config;

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	arm_bl2_early_platform_setup(mem_layout);

	/* Save SCP Boot config before it gets overwritten by SCP_BL2 loading */
	scp_boot_config = mmio_read_32(SCP_BOOT_CFG_ADDR);
	VERBOSE("BL2: Saved SCP Boot config = 0x%x\n", scp_boot_config);
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

	/*
	 * Before releasing the AP cores out of reset, the SCP writes some data
	 * at the beginning of the Trusted SRAM. It is is overwritten before
	 * reaching this function. We need to restore this data, as if the
	 * target had just come out of reset. This implies:
	 *  - zeroing the first 128 bytes of Trusted SRAM;
	 *  - restoring the SCP boot configuration.
	 */
	VERBOSE("BL2: Restoring SCP reset data in Trusted SRAM\n");
	memset((void *) ARM_TRUSTED_SRAM_BASE, 0, 128);
	mmio_write_32(SCP_BOOT_CFG_ADDR, scp_boot_config);
}
#endif /* EL3_PAYLOAD_BASE */

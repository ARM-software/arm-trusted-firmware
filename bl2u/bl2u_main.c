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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <auth_mod.h>
#include <bl_common.h>
#include <bl1.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>

/*******************************************************************************
 * This function is responsible to:
 * Load SCP_BL2U if platform has defined SCP_BL2U_BASE
 * Perform platform setup.
 * Go back to EL3.
 ******************************************************************************/
void bl2u_main(void)
{
	NOTICE("BL2U: %s\n", version_string);
	NOTICE("BL2U: %s\n", build_message);

#if SCP_BL2U_BASE
	int rc;
	/* Load the subsequent bootloader images */
	rc = bl2u_plat_handle_scp_bl2u();
	if (rc) {
		ERROR("Failed to load SCP_BL2U (%i)\n", rc);
		panic();
	}
#endif

	/* Perform platform setup in BL2U after loading SCP_BL2U */
	bl2u_platform_setup();

	/*
	 * Indicate that BL2U is done and resume back to
	 * normal world via an SMC to BL1.
	 * x1 could be passed to Normal world,
	 * so DO NOT pass any secret information.
	 */
	smc(FWU_SMC_SEC_IMAGE_DONE, 0, 0, 0, 0, 0, 0, 0);
	wfi();
}

/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#include <plat_arm.h>
#include "fvp_private.h"

#if LOAD_IMAGE_V2
void bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2)
#else
void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
#endif
{
	arm_bl31_early_platform_setup(from_bl2, plat_params_from_bl2);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * FVP PSCI code will enable coherency for other clusters.
	 */
	fvp_interconnect_enable();
}

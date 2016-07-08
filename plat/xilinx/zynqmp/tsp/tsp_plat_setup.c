/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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
#include <console.h>
#include <debug.h>
#include <platform_tsp.h>
#include <plat_arm.h>
#include "../zynqmp_private.h"

#define BL32_END (unsigned long)(&__BL32_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL32_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL32_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void tsp_early_platform_setup(void)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	console_init(ZYNQMP_UART_BASE, zynqmp_get_uart_clk(),
		     ZYNQMP_UART_BAUDRATE);

	/* Initialize the platform config for future decision making */
	zynqmp_config_setup();
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
	arm_setup_page_tables(BL32_BASE,
			      BL32_END - BL32_BASE,
			      BL_CODE_BASE,
			      BL_CODE_LIMIT,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_LIMIT,
			      BL32_COHERENT_RAM_BASE,
			      BL32_COHERENT_RAM_LIMIT
			      );
	enable_mmu_el1(0);
}

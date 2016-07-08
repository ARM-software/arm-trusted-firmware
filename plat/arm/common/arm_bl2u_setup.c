/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <arm_def.h>
#include <bl_common.h>
#include <console.h>
#include <platform_def.h>
#include <plat_arm.h>
#include <string.h>

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL2U_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2U_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#endif

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl2u_platform_setup
#pragma weak bl2u_early_platform_setup
#pragma weak bl2u_plat_arch_setup

/*
 * Perform ARM standard platform setup for BL2U
 */
void arm_bl2u_platform_setup(void)
{
	/* Initialize the secure environment */
	plat_arm_security_setup();
}

void bl2u_platform_setup(void)
{
	arm_bl2u_platform_setup();
}

void arm_bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_ARM_BOOT_UART_BASE, PLAT_ARM_BOOT_UART_CLK_IN_HZ,
			ARM_CONSOLE_BAUDRATE);
}

/*******************************************************************************
 * BL1 can pass platform dependent information to BL2U in x1.
 * In case of ARM CSS platforms x1 contains SCP_BL2U image info.
 * In case of ARM FVP platforms x1 is not used.
 * In both cases, x0 contains the extents of the memory available to BL2U
 ******************************************************************************/
void bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	arm_bl2u_early_platform_setup(mem_layout, plat_info);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 * The memory that is used by BL2U is only mapped.
 ******************************************************************************/
void arm_bl2u_plat_arch_setup(void)
{
	arm_setup_page_tables(BL2U_BASE,
			      BL31_LIMIT,
			      BL_CODE_BASE,
			      BL_CODE_LIMIT,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_LIMIT
#if USE_COHERENT_MEM
			      ,
			      BL2U_COHERENT_RAM_BASE,
			      BL2U_COHERENT_RAM_LIMIT
#endif
		);
	enable_mmu_el1(0);
}

void bl2u_plat_arch_setup(void)
{
	arm_bl2u_plat_arch_setup();
}

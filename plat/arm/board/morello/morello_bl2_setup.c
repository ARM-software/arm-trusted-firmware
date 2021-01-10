/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>

void bl2_platform_setup(void)
{
#ifdef TARGET_PLATFORM_SOC
	/*
	 * Morello platform supports RDIMMs with ECC capability. To use the ECC
	 * capability, the entire DDR memory space has to be zeroed out before
	 * enabling the ECC bits in DMC-Bing.
	 * Zeroing DDR memory range 0x80000000 - 0xFFFFFFFF during BL2 stage,
	 * as BL33 binary cannot be copied to DDR memory before enabling ECC.
	 * Rest of the DDR memory space is zeroed out during BL31 stage.
	 */
	INFO("Zeroing DDR memory range 0x80000000 - 0xFFFFFFFF\n");
	zero_normalmem((void *)ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
	flush_dcache_range(ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
#endif
	arm_bl2_platform_setup();
}

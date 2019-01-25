/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/tzc_dmc500.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/soc/common/soc_css.h>

#include <sgm_variant.h>

/* Is populated with the DMC-500 controllers base addresses */
static tzc_dmc500_driver_data_t plat_driver_data;

void plat_sgm_dp_security_setup(void)
{
	unsigned int nprot_nsaid;

	/*
	 * At reset the Mali display processors start with NSAIDs set to zero
	 * so the firmware must set them up to the expected values for ARM sgm
	 * platforms.
	 */

	nprot_nsaid = mmio_read_32(MALI_DP_BASE + DP_NPROT_NSAID_OFFSET);
	nprot_nsaid &= ~((0xF << W_NPROT_NSAID_SHIFT) |
			(0xF << LS_NPORT_NSAID_SHIFT));
	nprot_nsaid |= ((TZC_NSAID_DISP1 << W_NPROT_NSAID_SHIFT) |
			(TZC_NSAID_DISP0 << LS_NPORT_NSAID_SHIFT));
	mmio_write_32(MALI_DP_BASE + DP_NPROT_NSAID_OFFSET, nprot_nsaid);
}

void plat_arm_security_setup(void)
{
	unsigned int i;
	unsigned int part_num = GET_PLAT_PART_NUM;

	INFO("part_num: 0x%x\n", part_num);

	/*
	 * Initialise plat_driver_data with platform specific DMC_BASE
	 * addresses
	 */
	switch (part_num) {
	case SGM775_SSC_VER_PART_NUM:
		for (i = 0; i < SGM775_DMC_COUNT; i++)
			plat_driver_data.dmc_base[i] = PLAT_ARM_TZC_BASE
					+ SGM_DMC_SIZE * i;
		plat_driver_data.dmc_count = SGM775_DMC_COUNT;
		break;
	default:
		/* Unexpected platform */
		ERROR("Unexpected platform\n");
		panic();
	}
	/* Initialize the TrustZone Controller in DMC-500 */
	arm_tzc_dmc500_setup(&plat_driver_data, NULL);

	/* Do DP NSAID setup */
	plat_sgm_dp_security_setup();
	/* Do ARM CSS SoC security setup */
	soc_css_security_setup();
}

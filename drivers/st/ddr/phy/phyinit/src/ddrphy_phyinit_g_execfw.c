/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <lib/mmio.h>

#include <platform_def.h>

/*
 * Execute the Training Firmware
 *
 * The training firmware is executed with the procedure:
 *
 * -# Reset the firmware microcontroller by writing the MicroReset register to
 * set the StallToMicro and ResetToMicro fields to 1 (all other fields should be
 * zero). Then rewrite the registers so that only the StallToMicro remains set
 * (all other fields should be zero).
 * -# Begin execution of the training firmware by setting the MicroReset
 * register to 0.
 * -# Wait for the training firmware to complete by following the procedure implemented in
 * ddrphy_phyinit_usercustom_g_waitfwdone() function.
 * -# Halt the microcontroller.
 *
 * \return 0 on success.
 */
int ddrphy_phyinit_g_execfw(void)
{
	int ret;

	/*
	 * 1. Reset the firmware microcontroller by writing the MicroReset CSR to set the
	 * StallToMicro and ResetToMicro fields to 1 (all other fields should be zero).
	 * Then rewrite the CSR so that only the StallToMicro remains set (all other fields should
	 * be zero).
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICROCONTMUXSEL_ADDR))),
		      CSR_STALLTOMICRO_MASK);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICRORESET_ADDR))),
		      CSR_RESETTOMICRO_MASK | CSR_STALLTOMICRO_MASK);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICRORESET_ADDR))),
		      CSR_STALLTOMICRO_MASK);

	/* 2. Begin execution of the training firmware by setting the MicroReset CSR to 0 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICRORESET_ADDR))), 0x0U);

	/*
	 * 3. Wait for the training firmware to complete by following the procedure
	 * implemented in ddrphy_phyinit_usercustom_g_waitfwdone() function.
	 */
	ret = ddrphy_phyinit_usercustom_g_waitfwdone();
	if (ret != 0) {
		return ret;
	}

	/* 4. Halt the microcontroller */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICRORESET_ADDR))),
		      CSR_STALLTOMICRO_MASK);

	return 0;
}

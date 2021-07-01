/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <lib/mmio.h>

#include <platform_def.h>

/*
 * This function loads the training firmware IMEM image into the PHY.
 *
 * This function reads the DDR firmware source memory area to generate a
 * set of apb writes to load IMEM image into the PHY. The exact steps in this
 * function are as follows:
 *
 * -# Ensure DRAM is in reset.
 * -# Load the microcontroller memory with the provided training firmware
 * -# Initialize the firmware mailbox structures to be able to communicate with
 * the firmware.
 *
 * \return void
 */
void ddrphy_phyinit_d_loadimem(void)
{
	uint16_t memresetl;
	uint32_t *ptr32;

	/* Set memresetl to avoid glitch on BP_MemReset_L during training */
	memresetl = CSR_PROTECTMEMRESET_MASK;
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_MEMRESETL_ADDR))), memresetl);

	ptr32 = (uint32_t *)(STM32MP_DDR_FW_BASE + STM32MP_DDR_FW_IMEM_OFFSET);
	ddrphy_phyinit_writeoutmem(ptr32, IMEM_ST_ADDR, IMEM_SIZE);
}

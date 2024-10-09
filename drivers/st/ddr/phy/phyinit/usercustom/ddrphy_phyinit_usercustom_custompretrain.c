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

/* DDRDBG registers */
#define DDRDBG_DDR34_AC_SWIZZLE_ADD3_0		U(0x100)

/*
 * This function is called before training firmware is executed. Any
 * register override in this function might affect the firmware training
 * results.
 *
 * This function is executed before firmware execution loop. Thus this function
 * should be used only for the following:
 *
 *  - Override PHY register values written by
 *  ddrphy_phyinit_c_initphyconfig. An example use case is when this
 *  function does not perform the exact programing desired by the user.
 *  - Write custom PHY registers that need to take effect before training
 *  firmware execution.
 *
 * User shall use mmio_write_16 to write PHY registers in order for the register
 * to be tracked by PhyInit for retention restore.
 *
 * To override settings in the message block, users can assign values to the
 * fields in the message block data structure directly.
 *
 * \ref examples/simple/ddrphy_phyinit_usercustom_custompretrain.c example of this function.
 *
 * @return Void
 */
void ddrphy_phyinit_usercustom_custompretrain(struct stm32mp_ddr_config *config)
{
	uint32_t byte __unused;
	uint32_t i = 0U;
	uint32_t j;
	uintptr_t base;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	base = (uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTSWIZZLEHWTADDRESS0_ADDR)));

	for (i = 0U; i < NB_HWT_SWIZZLE; i++) {
		mmio_write_16(base + (i * sizeof(uint32_t)),
			      (uint16_t)config->uis.swizzle[i]);
	}

	base = (uintptr_t)(stm32_ddrdbg_get_base() + DDRDBG_DDR34_AC_SWIZZLE_ADD3_0);

	for (j = 0U; j < NB_AC_SWIZZLE; j++, i++) {
		mmio_write_32(base + (j * sizeof(uint32_t)), config->uis.swizzle[i]);
	}
#else /* STM32MP_LPDDR4_TYPE */
	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		base = (uintptr_t)(DDRPHYC_BASE + (4U *
						   ((byte << 12) | TDBYTE | CSR_DQ0LNSEL_ADDR)));

		for (j = 0U; j < NB_DQLNSEL_SWIZZLE_PER_BYTE; j++, i++) {
			mmio_write_16(base + (j * sizeof(uint32_t)),
				      (uint16_t)config->uis.swizzle[i]);
		}
	}

	base = (uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_MAPCAA0TODFI_ADDR)));

	for (j = 0U; j < NB_MAPCAATODFI_SWIZZLE; j++, i++) {
		mmio_write_16(base + (j * sizeof(uint32_t)),
			      (uint16_t)config->uis.swizzle[i]);
	}

	base = (uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_MAPCAB0TODFI_ADDR)));

	for (j = 0U; j < NB_MAPCABTODFI_SWIZZLE; j++, i++) {
		mmio_write_16(base + (j * sizeof(uint32_t)),
			      (uint16_t)config->uis.swizzle[i]);
	}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
}

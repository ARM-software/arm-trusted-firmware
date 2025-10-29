/*
 * Copyright (c) 2023-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/st/stm32_rifsc.h>
#include <drivers/st/stm32mp_rifsc_regs.h>
#include <dt-bindings/soc/rif.h>
#include <lib/mmio.h>

#include <platform_def.h>

void stm32_rifsc_ip_configure(int rimu_id, int rifsc_id, uint32_t param)
{
	uint32_t bit;

	assert(rifsc_id < STM32MP25_RIFSC_MAX_ID);

	bit = BIT(rifsc_id % U(32));

	/* Set peripheral accesses to Secure/Privilege only */
	mmio_setbits_32(RIFSC_BASE + _RIFSC_RISC_SECCFGR(rifsc_id), bit);
	mmio_setbits_32(RIFSC_BASE + _RIFSC_RISC_PRIVCFGR(rifsc_id), bit);

	/* Apply specific configuration to RIF master */
	mmio_write_32(RIFSC_BASE + _RIFSC_RIMC_ATTR(rimu_id), param);
}

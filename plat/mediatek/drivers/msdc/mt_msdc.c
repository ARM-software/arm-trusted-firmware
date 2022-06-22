/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_msdc.h>
#include <platform_def.h>

uint64_t msdc_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			     uint64_t arg2, uint64_t arg3)
{
	INFO("[%s] msdc setup call from kernel\n", __func__);
	mmio_setbits_32(MSDC0_BASE + MSDC_CQHCI_CFG, MSDC_CQHCI_CRYPTO_ENABLE);

	return 0L;
}

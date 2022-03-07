// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include "socfpga_fcs.h"
#include "socfpga_mailbox.h"
#include "socfpga_reset_manager.h"
#include "socfpga_sip_svc.h"
#include "socfpga_system_manager.h"

uint32_t intel_ecc_dbe_notification(uint64_t dbe_value)
{
	dbe_value &= WARM_RESET_WFI_FLAG;

	/* Trap CPUs in WFI if warm reset flag is set */
	if (dbe_value > 0) {
		while (1) {
			wfi();
		}
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

bool cold_reset_for_ecc_dbe(void)
{
	uint32_t dbe_int_status;

	dbe_int_status = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_8));

	/* Trigger cold reset only for error in critical memory (DDR/OCRAM) */
	dbe_int_status &= SYSMGR_ECC_DBE_COLD_RST_MASK;

	if (dbe_int_status > 0) {
		return true;
	}

	return false;
}

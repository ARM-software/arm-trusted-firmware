/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>

#include "firewall.h"

const struct fwl_data fwls[] = {
	{OSPI_FWL_ID, OSPI_FWL_REGION},		/* OSPI */
	{ADC_MCASP_FWL_ID, ADC_MCASP_FWL_REGION},/* ADC and MCASP */
	{DDR_FWL_ID, DDR_BG_FWL_REGION},	/* DDR */
};

/*
 * Take ownership and set the configuration for a given firewall ID and region.
 * Function arguments are same as those which will be passed to
 * ti_sci_set_fwl_region()
 */
static void add_fwl_configs(const uint16_t fwl_id, const uint16_t region,
			    const uint32_t n_perm_regs, const uint32_t control,
			    const uint32_t permissions[FWL_MAX_PRIVID_SLOTS],
			    const uint64_t start_address, const uint64_t end_address)
{
	uint8_t owner_index = TFA_HOST_ID;
	uint8_t owner_privid = A53_PRIV_ID;
	uint16_t owner_permission_bits = 0;
	int ret = 0;

	ret = ti_sci_change_fwl_owner(fwl_id, region, owner_index,
				      &owner_privid, &owner_permission_bits);
	if (ret) {
		ERROR("Could not change firewall owner (%d)\n", ret);
		panic();
	}

	ret = ti_sci_set_fwl_region(fwl_id, region, n_perm_regs,
				    control, permissions,
				    start_address, end_address);
	if (ret) {
		ERROR("Could not set firewall region (%d)\n", ret);
		panic();
	}
}

void update_fwl_configs(void)
{
	uint32_t permissions[3] = {FWL_PERM_ALL_RW, FWL_PERM_ALL_RW, FWL_PERM_ALL_RW};

	/* Open up firewalls that were configured by ROM for boot phase */
	for (int i = 0; i < ARRAY_SIZE(fwls); i++) {
		add_fwl_configs(fwls[i].fwl_id, fwls[i].region, FWL_MAX_PRIVID_SLOTS,
				FWL_CTRL_EN_BG, permissions, 0x0, 0xFFFFFFFFF);
	}
}

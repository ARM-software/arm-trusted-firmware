/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>
#include <lib/mmio.h>

/* Vendor header */
#include "apusys_security_ctrl_plat.h"

static void apusys_domain_remap_init(void)
{
	const uint32_t remap_domains[] = {
		D0_REMAP_DOMAIN,  D1_REMAP_DOMAIN,  D2_REMAP_DOMAIN,  D3_REMAP_DOMAIN,
		D4_REMAP_DOMAIN,  D5_REMAP_DOMAIN,  D6_REMAP_DOMAIN,  D7_REMAP_DOMAIN,
		D8_REMAP_DOMAIN,  D9_REMAP_DOMAIN,  D10_REMAP_DOMAIN, D11_REMAP_DOMAIN,
		D12_REMAP_DOMAIN, D13_REMAP_DOMAIN, D14_REMAP_DOMAIN, D15_REMAP_DOMAIN
	};
	uint32_t lower_domain = 0;
	uint32_t higher_domain = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(remap_domains); i++) {
		if (i < REG_DOMAIN_NUM) {
			lower_domain |= (remap_domains[i] << (i * REG_DOMAIN_BITS));
		} else {
			higher_domain |= (remap_domains[i] <<
					  ((i - REG_DOMAIN_NUM) * REG_DOMAIN_BITS));
		}
	}

	mmio_write_32(SOC2APU_SET1_0, lower_domain);
	mmio_write_32(SOC2APU_SET1_1, higher_domain);
	mmio_setbits_32(APU_SEC_CON, DOMAIN_REMAP_SEL);
}

void apusys_security_ctrl_init(void)
{
	apusys_domain_remap_init();
}

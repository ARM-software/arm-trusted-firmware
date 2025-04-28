/*
 * Copyright (c) 2024-2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define ENABLE_SMPU_PROTECT	(1)

#include <common/debug.h>
#include <lib/mmio.h>

#include <apusys_security_ctrl_plat.h>
#include <mtk_bl31_interface.h>

#define APUSYS_SEC_FW_EMI_REGION	(23)

#define bits_clr(x, m, o)	(x & (~(m << o)))
#define bits_set(x, v, m, o)	((bits_clr(x, m, o)) | ((v & m) << o))

static void sec_sideband_init(void)
{
	uint32_t value = mmio_read_32(SEC_CTRL_SIDE_BAND);

	value = bits_set(value, SEC_CTRL_NARE_DOMAIN, SEC_CTRL_DOMAIN_MASK,
			 SEC_CTRL_NARE_DOMAIN_SHF);
	value = bits_set(value, SEC_CTRL_NARE_NS, SEC_CTRL_NS_MASK, SEC_CTRL_NARE_NS_SHF);
	value = bits_set(value, SEC_CTRL_SARE0_DOMAIN, SEC_CTRL_DOMAIN_MASK,
			 SEC_CTRL_SARE0_DOMAIN_SHF);
	value = bits_set(value, SEC_CTRL_SARE0_NS, SEC_CTRL_NS_MASK, SEC_CTRL_SARE0_NS_SHF);
	value = bits_set(value, SEC_CTRL_SARE1_DOMAIN, SEC_CTRL_DOMAIN_MASK,
			 SEC_CTRL_SARE1_DOMAIN_SHF);
	value = bits_set(value, SEC_CTRL_SARE1_NS, SEC_CTRL_NS_MASK, SEC_CTRL_SARE1_NS_SHF);

	mmio_write_32(SEC_CTRL_SIDE_BAND, value);
}

static void domain_remap_init(void)
{
	const uint32_t remap_domains[] = {
		D0_REMAP_DOMAIN,  D1_REMAP_DOMAIN,  D2_REMAP_DOMAIN,  D3_REMAP_DOMAIN,
		D4_REMAP_DOMAIN,  D5_REMAP_DOMAIN,  D6_REMAP_DOMAIN,  D7_REMAP_DOMAIN,
		D8_REMAP_DOMAIN,  D9_REMAP_DOMAIN,  D10_REMAP_DOMAIN, D11_REMAP_DOMAIN,
		D12_REMAP_DOMAIN, D13_REMAP_DOMAIN, D14_REMAP_DOMAIN, D15_REMAP_DOMAIN,
	};
	uint32_t lower_domain = 0;
	uint32_t higher_domain = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(remap_domains); i++) {
		if (i < SEC_CTRL_REG_DOMAIN_NUM)
			lower_domain |= (remap_domains[i] << (i * REG_DOMAIN_BITS));
		else
			higher_domain |= (remap_domains[i] <<
					  ((i - SEC_CTRL_REG_DOMAIN_NUM) * REG_DOMAIN_BITS));
	}

	mmio_write_32(SEC_CTRL_SOC2APU_SET1_0, lower_domain);
	mmio_write_32(SEC_CTRL_SOC2APU_SET1_1, higher_domain);
	mmio_setbits_32(APU_SEC_CON, SEC_CTRL_DOMAIN_REMAP_SEL);
}

void apusys_security_ctrl_init(void)
{
	domain_remap_init();
	sec_sideband_init();
}

int apusys_plat_setup_sec_mem(void)
{
#if ENABLE_SMPU_PROTECT
	return emi_mpu_set_protection(APU_RESERVE_MEMORY >> EMI_MPU_ALIGN_BITS,
				      (APU_RESERVE_MEMORY + APU_RESERVE_SIZE) >>
				      EMI_MPU_ALIGN_BITS,
				      APUSYS_SEC_FW_EMI_REGION);
#else
	INFO("%s: Bypass SMPU protection setup.\n", __func__);
	return 0;
#endif
}

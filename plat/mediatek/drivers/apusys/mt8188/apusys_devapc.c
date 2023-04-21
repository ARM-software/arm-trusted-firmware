/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>
#include <lib/utils_def.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_devapc.h"
#include "apusys_devapc_def.h"
#include <platform_def.h>

#define DUMP_APUSYS_DAPC	(0)

static const struct apc_dom_16 APU_CTRL_DAPC_AO[] = {
	/* ctrl index = 0 */
	SLAVE_VCORE("apu_ao_ctl_o-0"),
	SLAVE_RPC("apu_ao_ctl_o-2"),
	SLAVE_PCU("apu_ao_ctl_o-3"),
	SLAVE_AO_CTRL("apu_ao_ctl_o-4"),
	SLAVE_PLL("apu_ao_ctl_o-5"),
	SLAVE_ACC("apu_ao_ctl_o-6"),
	SLAVE_SEC("apu_ao_ctl_o-7"),
	SLAVE_ARE0("apu_ao_ctl_o-8"),
	SLAVE_ARE1("apu_ao_ctl_o-9"),
	SLAVE_ARE2("apu_ao_ctl_o-10"),

	/* ctrl index = 10 */
	SLAVE_UNKNOWN("apu_ao_ctl_o-11"),
	SLAVE_AO_BCRM("apu_ao_ctl_o-12"),
	SLAVE_AO_DAPC_WRAP("apu_ao_ctl_o-13"),
	SLAVE_AO_DAPC_CON("apu_ao_ctl_o-14"),
	SLAVE_RCX_ACX_BULK("apu_ao_ctl_o-15"),
	SLAVE_UNKNOWN("apu_ao_ctl_o-16"),
	SLAVE_UNKNOWN("apu_ao_ctl_o-17"),
	SLAVE_APU_BULK("apu_ao_ctl_o-18"),
	SLAVE_ACX0_BCRM("apu_ao_ctl_o-20"),
	SLAVE_RPCTOP_LITE_ACX0("apu_ao_ctl_o-21"),

	/* ctrl index = 20 */
	SLAVE_ACX1_BCRM("apu_ao_ctl_o-22"),
	SLAVE_RPCTOP_LITE_ACX1("apu_ao_ctl_o-23"),
	SLAVE_RCX_TO_ACX0_0("apu_rcx2acx0_o-0"),
	SLAVE_RCX_TO_ACX0_1("apu_rcx2acx0_o-1"),
	SLAVE_SAE_TO_ACX0_0("apu_sae2acx0_o-0"),
	SLAVE_SAE_TO_ACX0_1("apu_sae2acx0_o-1"),
	SLAVE_RCX_TO_ACX1_0("apu_rcx2acx1_o-0"),
	SLAVE_RCX_TO_ACX1_1("apu_rcx2acx1_o-1"),
	SLAVE_SAE_TO_ACX1_0("apu_sae2acx1_o-0"),
	SLAVE_SAE_TO_ACX1_1("apu_sae2acx1_o-1"),
};

static enum apusys_apc_err_status set_slave_ao_ctrl_apc(uint32_t slave,
							enum apusys_apc_domain_id domain_id,
							enum apusys_apc_perm_type perm)
{
	uint32_t apc_register_index;
	uint32_t apc_set_index;
	uint32_t base;
	uint32_t clr_bit;
	uint32_t set_bit;

	if ((perm < 0) || (perm >= PERM_NUM)) {
		ERROR(MODULE_TAG "%s: permission type:0x%x is not supported!\n", __func__, perm);
		return APUSYS_APC_ERR_GENERIC;
	}

	if ((slave >= APU_CTRL_DAPC_AO_SLAVE_NUM) ||
	    ((domain_id < 0) || (domain_id >= APU_CTRL_DAPC_AO_DOM_NUM))) {
		ERROR(MODULE_TAG "%s: out of boundary, slave:0x%x, domain_id:0x%x\n",
		      __func__, slave, domain_id);
		return APUSYS_APC_ERR_GENERIC;
	}

	apc_register_index = slave / APU_CTRL_DAPC_AO_SLAVE_NUM_IN_1_DOM;
	apc_set_index = slave % APU_CTRL_DAPC_AO_SLAVE_NUM_IN_1_DOM;

	clr_bit = (DEVAPC_MASK << (apc_set_index * DEVAPC_DOM_SHIFT));
	set_bit = (uint32_t)perm << (apc_set_index * DEVAPC_DOM_SHIFT);

	base = (APU_CTRL_DAPC_AO_BASE + domain_id * DEVAPC_DOM_SIZE +
		apc_register_index * DEVAPC_REG_SIZE);

	mmio_clrsetbits_32(base, clr_bit, set_bit);
	return APUSYS_APC_OK;
}

static void apusys_devapc_init(const char *name, uint32_t base)
{
	mmio_write_32(APUSYS_DAPC_CON(base), APUSYS_DAPC_CON_VIO_MASK);
}

int apusys_devapc_ao_init(void)
{
	enum apusys_apc_err_status ret;

	apusys_devapc_init("APUAPC_CTRL_AO", APU_CTRL_DAPC_AO_BASE);

	ret = SET_APUSYS_DAPC_V1(APU_CTRL_DAPC_AO, set_slave_ao_ctrl_apc);
	if (ret != APUSYS_APC_OK) {
		ERROR(MODULE_TAG "%s: set_apusys_ao_ctrl_dap FAILED!\n", __func__);
		return -1;
	}

#if DUMP_APUSYS_DAPC
	DUMP_APUSYS_DAPC_V1(APU_CTRL_DAPC_AO);
#endif

	return 0;
}

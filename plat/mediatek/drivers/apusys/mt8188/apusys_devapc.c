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

static const struct apc_dom_16 APU_NOC_DAPC_RCX[] = {
	/* ctrl index = 0 */
	SLAVE_MD32_SRAM("slv16-0"),
	SLAVE_MD32_SRAM("slv16-1"),
	SLAVE_MD32_SRAM("slv16-2"),
	SLAVE_MD32_SRAM("slv16-3"),
	SLAVE_MD32_SRAM("slv16-4"),
};

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

static const struct apc_dom_16 APU_CTRL_DAPC_RCX[] = {
	/* ctrl index = 0 */
	SLAVE_MD32_SYSCTRL0("md32_apb_s-0"),
	SLAVE_MD32_SYSCTRL1("md32_apb_s-1"),
	SLAVE_MD32_WDT("md32_apb_s-2"),
	SLAVE_MD32_CACHE("md32_apb_s-3"),
	SLAVE_RPC("apusys_ao-0"),
	SLAVE_PCU("apusys_ao-1"),
	SLAVE_AO_CTRL("apusys_ao-2"),
	SLAVE_PLL("apusys_ao-3"),
	SLAVE_ACC("apusys_ao-4"),
	SLAVE_SEC("apusys_ao-5"),

	/* ctrl index = 10 */
	SLAVE_ARE0("apusys_ao-6"),
	SLAVE_ARE1("apusys_ao-7"),
	SLAVE_ARE2("apusys_ao-8"),
	SLAVE_UNKNOWN("apusys_ao-9"),
	SLAVE_AO_BCRM("apusys_ao-10"),
	SLAVE_AO_DAPC_WRAP("apusys_ao-11"),
	SLAVE_AO_DAPC_CON("apusys_ao-12"),
	SLAVE_VCORE("apusys_ao-13"),
	SLAVE_ACX0_BCRM("apusys_ao-15"),
	SLAVE_ACX1_BCRM("apusys_ao-16"),

	/* ctrl index = 20 */
	SLAVE_NOC_AXI("noc_axi"),
	SLAVE_MD32_DBG("md32_dbg"),
	SLAVE_DBG_CRTL("apb_infra_dbg"),
	SLAVE_IOMMU0_BANK0("apu_n_mmu_r0"),
	SLAVE_IOMMU0_BANK1("apu_n_mmu_r1"),
	SLAVE_IOMMU0_BANK2("apu_n_mmu_r2"),
	SLAVE_IOMMU0_BANK3("apu_n_mmu_r3"),
	SLAVE_IOMMU0_BANK4("apu_n_mmu_r4"),
	SLAVE_IOMMU1_BANK0("apu_s_mmu_r0"),
	SLAVE_IOMMU1_BANK1("apu_s_mmu_r1"),

	/* ctrl index = 30 */
	SLAVE_IOMMU1_BANK2("apu_s_mmu_r2"),
	SLAVE_IOMMU1_BANK3("apu_s_mmu_r3"),
	SLAVE_IOMMU1_BANK4("apu_s_mmu_r4"),
	SLAVE_S0_SSC("apu_s0_ssc_cfg"),
	SLAVE_N0_SSC("apu_n0_ssc_cfg"),
	SLAVE_ACP_SSC("apu_acp_ssc_cfg"),
	SLAVE_S1_SSC("apu_s1_ssc_cfg"),
	SLAVE_N1_SSC("apu_n1_ssc_cfg"),
	SLAVE_CFG("apu_rcx_cfg"),
	SLAVE_SEMA_STIMER("apu_sema_stimer"),

	/* ctrl index = 40 */
	SLAVE_EMI_CFG("apu_emi_cfg"),
	SLAVE_LOG("apu_logtop"),
	SLAVE_CPE_SENSOR("apu_cpe_sensor"),
	SLAVE_CPE_COEF("apu_cpe_coef"),
	SLAVE_CPE_CTRL("apu_cpe_ctrl"),
	SLAVE_UNKNOWN("apu_xpu_rsi"),
	SLAVE_DFD_REG_SOC("apu_dfd"),
	SLAVE_SENSOR_WRAP_ACX0_DLA0("apu_sen_ac0_dla0"),
	SLAVE_SENSOR_WRAP_ACX0_DLA1("apu_sen_ac0_dla1"),
	SLAVE_SENSOR_WRAP_ACX0_VPU0("apu_sen_ac0_vpu"),

	/* ctrl index = 50 */
	SLAVE_SENSOR_WRAP_ACX1_DLA0("apu_sen_ac1_dla0"),
	SLAVE_SENSOR_WRAP_ACX1_DLA1("apu_sen_ac1_dla1"),
	SLAVE_SENSOR_WRAP_ACX1_VPU0("apu_sen_ac1_vpu"),
	SLAVE_REVISER("noc_cfg-0"),
	SLAVE_NOC("noc_cfg-1"),
	SLAVE_BCRM("infra_bcrm"),
	SLAVE_DAPC_WRAP("infra_dapc_wrap"),
	SLAVE_DAPC_CON("infra_dapc_con"),
	SLAVE_NOC_DAPC_WRAP("noc_dapc_wrap"),
	SLAVE_NOC_DAPC_CON("noc_dapc_con"),

	/* ctrl index = 60 */
	SLAVE_NOC_BCRM("noc_bcrm"),
	SLAVE_ACS("apu_rcx_acs"),
	SLAVE_HSE("apu_hse"),
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

static enum apusys_apc_err_status set_slave_noc_dapc_rcx(uint32_t slave,
							 enum apusys_apc_domain_id domain_id,
							 enum apusys_apc_perm_type perm)
{
	uint32_t apc_register_index;
	uint32_t apc_set_index;
	uint32_t base;
	uint32_t clr_bit;
	uint32_t set_bit;

	if ((perm >= PERM_NUM) || (perm < 0)) {
		ERROR(MODULE_TAG "%s: permission type:0x%x is not supported!\n", __func__, perm);
		return APUSYS_APC_ERR_GENERIC;
	}

	if ((slave >= APU_NOC_DAPC_RCX_SLAVE_NUM) ||
	    ((domain_id < 0) || (domain_id >= APU_NOC_DAPC_RCX_DOM_NUM))) {
		ERROR(MODULE_TAG "%s: out of boundary, slave:0x%x, domain_id:0x%x\n",
		      __func__, slave, domain_id);
		return APUSYS_APC_ERR_GENERIC;
	}

	apc_register_index = slave / APU_NOC_DAPC_RCX_SLAVE_NUM_IN_1_DOM;
	apc_set_index = slave % APU_NOC_DAPC_RCX_SLAVE_NUM_IN_1_DOM;

	clr_bit = (DEVAPC_MASK << (apc_set_index * DEVAPC_DOM_SHIFT));
	set_bit = ((uint32_t)perm) << (apc_set_index * DEVAPC_DOM_SHIFT);
	base = (APU_NOC_DAPC_RCX_BASE + domain_id * DEVAPC_DOM_SIZE +
		apc_register_index * DEVAPC_REG_SIZE);

	mmio_clrsetbits_32(base, clr_bit, set_bit);
	return APUSYS_APC_OK;
}

static enum apusys_apc_err_status set_slave_rcx_ctrl_apc(uint32_t slave,
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

	if ((slave >= APU_CTRL_DAPC_RCX_SLAVE_NUM) ||
	    ((domain_id < 0) || (domain_id >= APU_CTRL_DAPC_RCX_DOM_NUM))) {
		ERROR(MODULE_TAG "%s: out of boundary, slave:0x%x, domain_id:0x%x\n",
		      __func__, slave, domain_id);
		return APUSYS_APC_ERR_GENERIC;
	}

	apc_register_index = slave / APU_CTRL_DAPC_RCX_SLAVE_NUM_IN_1_DOM;
	apc_set_index = slave % APU_CTRL_DAPC_RCX_SLAVE_NUM_IN_1_DOM;

	clr_bit = (DEVAPC_MASK << (apc_set_index * DEVAPC_DOM_SHIFT));
	set_bit = (uint32_t)perm << (apc_set_index * DEVAPC_DOM_SHIFT);
	base = (APU_CTRL_DAPC_RCX_BASE + domain_id * DEVAPC_DOM_SIZE +
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

int apusys_devapc_rcx_init(void)
{
	static bool apusys_devapc_rcx_init_called;
	enum apusys_apc_err_status ret;

	if (apusys_devapc_rcx_init_called == true) {
		INFO(MODULE_TAG "%s: init more than once!\n", __func__);
		return -1;
	}
	apusys_devapc_rcx_init_called = true;

	apusys_devapc_init("APUAPC_CTRL_RCX", APU_CTRL_DAPC_RCX_BASE);
	apusys_devapc_init("APUAPC_NOC_RCX", APU_NOC_DAPC_RCX_BASE);

	ret = SET_APUSYS_DAPC_V1(APU_CTRL_DAPC_RCX, set_slave_rcx_ctrl_apc);
	if (ret != APUSYS_APC_OK) {
		ERROR(MODULE_TAG "%s: set_slave_rcx_ctrl_apc FAILED!\n", __func__);
		return -1;
	}

#if DUMP_APUSYS_DAPC
	DUMP_APUSYS_DAPC_V1(APU_CTRL_DAPC_RCX);
#endif

	ret = SET_APUSYS_DAPC_V1(APU_NOC_DAPC_RCX, set_slave_noc_dapc_rcx);
	if (ret != APUSYS_APC_OK) {
		ERROR(MODULE_TAG "%s: set_slave_noc_dapc_rcx FAILED\n", __func__);
		return -1;
	}

#if DUMP_APUSYS_DAPC
	DUMP_APUSYS_DAPC_V1(APU_NOC_DAPC_RCX);
#endif

	return 0;
}

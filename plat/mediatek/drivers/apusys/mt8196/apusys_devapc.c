/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include <apusys_devapc.h>
#include <apusys_devapc_def.h>
#include <mtk_mmap_pool.h>

/* AO CONTROL DEVAPC - apu_rcx_ao_infra_dapc_con */
static const struct apc_dom_16 APUSYS_CTRL_DAPC_AO[] = {
	/* ctrl index = 0 */
	SLAVE_RCX_BULK0("apu_ao_ctl_o-0"),
	SLAVE_MD32_APB("apu_ao_ctl_o-1"),
	SLAVE_ACP_TCU_SSC("apu_ao_ctl_o-2"),
	SLAVE_PTP_THM("apu_ao_ctl_o-3"),
	SLAVE_VCORE("apu_ao_ctl_o-4"),
	SLAVE_IOMMU0_BANK0("apu_ao_ctl_o-5"),
	SLAVE_IOMMU0_BANK1("apu_ao_ctl_o-6"),
	SLAVE_IOMMU0_BANK2("apu_ao_ctl_o-7"),
	SLAVE_IOMMU0_BANK3("apu_ao_ctl_o-8"),
	SLAVE_IOMMU0_BANK4("apu_ao_ctl_o-9"),

	/* ctrl index = 10 */
	SLAVE_IOMMU1_BANK0("apu_ao_ctl_o-10"),
	SLAVE_IOMMU1_BANK1("apu_ao_ctl_o-11"),
	SLAVE_IOMMU1_BANK2("apu_ao_ctl_o-12"),
	SLAVE_IOMMU1_BANK3("apu_ao_ctl_o-13"),
	SLAVE_IOMMU1_BANK4("apu_ao_ctl_o-14"),
	SLAVE_S0_SSC("apu_ao_ctl_o-15"),
	SLAVE_N0_SSC("apu_ao_ctl_o-16"),
	SLAVE_S1_SSC("apu_ao_ctl_o-17"),
	SLAVE_N1_SSC("apu_ao_ctl_o-18"),
	SLAVE_ACP_SSC("apu_ao_ctl_o-19"),

	/* ctrl index = 20 */
	SLAVE_WDEC("apu_ao_ctl_o-20"),
	SLAVE_SMMU_IP_REG("apu_ao_ctl_o-21"),
	SLAVE_SMMU_NSEC("apu_ao_ctl_o-22"),
	SLAVE_SMMU_SEC("apu_ao_ctl_o-23"),
	SLAVE_ARE0("apu_ao_ctl_o-24"),
	SLAVE_ARE1("apu_ao_ctl_o-25"),
	SLAVE_SONC("apu_ao_ctl_o-26"),
	SLAVE_RPC("apu_ao_ctl_o-28"),
	SLAVE_PCU("apu_ao_ctl_o-29"),
	SLAVE_AO_CTRL("apu_ao_ctl_o-30"),

	/* ctrl index = 30 */
	SLAVE_AO_CTRL("apu_ao_ctl_o-31"),
	SLAVE_ACC("apu_ao_ctl_o-32"),
	SLAVE_SEC("apu_ao_ctl_o-33"),
	SLAVE_PLL("apu_ao_ctl_o-34"),
	SLAVE_RPC_MDLA("apu_ao_ctl_o-35"),
	SLAVE_TOP_PMU("apu_ao_ctl_o-36"),
	SLAVE_AO_BCRM("apu_ao_ctl_o-37"),
	SLAVE_AO_DAPC_WRAP("apu_ao_ctl_o-38"),
	SLAVE_AO_DAPC_CON("apu_ao_ctl_o-39"),
	SLAVE_UNDEFINE0("apu_ao_ctl_o-40"),

	/* ctrl index = 40 */
	SLAVE_UNDEFINE1("apu_ao_ctl_o-41"),
	SLAVE_RCX_BULK0("apu_ao_ctl_o-42"),
	SLAVE_UNDEFINE2("apu_ao_ctl_o-43"),
	SLAVE_UNDEFINE3("apu_ao_ctl_o-44"),
	SLAVE_UNDEFINE4("apu_ao_ctl_o-45"),
	SLAVE_UNDEFINE5("apu_ao_ctl_o-46"),
	SLAVE_UNDEFINE6("apu_ao_ctl_o-47"),
	SLAVE_UNDEFINE7("apu_ao_ctl_o-48"),
	SLAVE_DATA_BULK("apu_ao_ctl_o-49"),
	SLAVE_ACX0_BULK("apu_ao_ctl_o-50"),

	/* ctrl index = 50 */
	SLAVE_ACX0_AO("apu_ao_ctl_o-51"),
	SLAVE_ACX1_BULK("apu_ao_ctl_o-52"),
	SLAVE_ACX1_AO("apu_ao_ctl_o-53"),
	SLAVE_NCX_BULK("apu_ao_ctl_o-54"),
	SLAVE_NCX_AO("apu_ao_ctl_o-55"),
	SLAVE_ACX0_BULK("apu_rcx2acx0_o-0"),
	SLAVE_ACX0_AO("apu_rcx2acx0_o-1"),
	SLAVE_ACX0_BULK("apu_sae2acx0_o-0"),
	SLAVE_ACX0_AO("apu_sae2acx0_o-1"),
	SLAVE_ACX1_BULK("apu_rcx2acx1_o-0"),

	/* ctrl index = 60 */
	SLAVE_ACX1_AO("apu_rcx2acx1_o-1"),
	SLAVE_ACX1_BULK("apu_sae2acx1_o-0"),
	SLAVE_ACX1_AO("apu_sae2acx1_o-1"),
	SLAVE_NCX_BULK("apu_rcx2ncx_o-0"),
	SLAVE_NCX_AO("apu_rcx2ncx_o-1"),
	SLAVE_NCX_BULK("apu_sae2ncx_o-0"),
	SLAVE_NCX_AO("apu_sae2ncx_o-1"),
};

static enum apusys_apc_err_status set_slave_ctrl_apc(uint32_t slave,
						     enum apusys_apc_type type,
						     enum apusys_apc_domain_id domain_id,
						     enum apusys_apc_perm_type perm)
{
	uint32_t apc_register_index;
	uint32_t apc_set_index;
	uint32_t base = 0;
	uint32_t clr_bit;
	uint32_t set_bit;
	uint32_t slave_num_in_1_dom;
	uint32_t slave_num, dom_num;
	uint32_t dapc_base;

	if (perm >= PERM_NUM) {
		ERROR("%s: permission type:0x%x is not supported!\n", __func__, perm);
		return APUSYS_APC_ERR_PERMISSION_NOT_SUPPORTED;
	}

	switch (type) {
	case DAPC_AO:
		slave_num_in_1_dom = APUSYS_CTRL_DAPC_AO_SLAVE_NUM_IN_1_DOM;
		slave_num = APUSYS_CTRL_DAPC_AO_SLAVE_NUM;
		dom_num = APUSYS_CTRL_DAPC_AO_DOM_NUM;
		dapc_base = APUSYS_CTRL_DAPC_AO_BASE;
		break;
	default:
		ERROR("%s: unsupported devapc type: %u\n", __func__, type);
		return APUSYS_APC_ERR_GENERIC;
	}

	apc_register_index = slave / slave_num_in_1_dom;
	apc_set_index = slave % slave_num_in_1_dom;

	clr_bit = DEVAPC_MASK << (apc_set_index * DEVAPC_DOM_SHIFT);
	set_bit = (uint32_t)perm << (apc_set_index * DEVAPC_DOM_SHIFT);

	if (slave < slave_num && domain_id < dom_num) {
		base = dapc_base + domain_id * DEVAPC_DOM_SIZE
		       + apc_register_index * DEVAPC_REG_SIZE;
	} else {
		ERROR("%s: out of boundary, devapc type: %d, slave: 0x%x, domain_id: 0x%x\n",
			__func__, type, slave, domain_id);
		return APUSYS_APC_ERR_OUT_OF_BOUNDARY;
	}

	if (!base)
		return APUSYS_APC_ERR_GENERIC;

	mmio_clrsetbits_32(base, clr_bit, set_bit);
	return APUSYS_APC_OK;
}

static enum apusys_apc_err_status set_slave_ao_ctrl_apc(uint32_t slave,
							enum apusys_apc_domain_id domain_id,
							enum apusys_apc_perm_type perm)
{
	return set_slave_ctrl_apc(slave, DAPC_AO, domain_id, perm);
}

static void apusys_devapc_init(uint32_t base)
{
	mmio_write_32(APUSYS_DAPC_CON(base), APUSYS_DAPC_CON_VIO_MASK);
}

int apusys_devapc_ao_init(void)
{
	int32_t ret = APUSYS_APC_OK;

	apusys_devapc_init(APUSYS_CTRL_DAPC_AO_BASE);

	ret = SET_APUSYS_DAPC_V1(APUSYS_CTRL_DAPC_AO, set_slave_ao_ctrl_apc);

	if (ret) {
		ERROR("[APUAPC_AO] %s: set_apusys_ao_ctrl_dapc failed\n", __func__);
		return ret;
	}

#ifdef DUMP_CFG
	DUMP_APUSYS_DAPC_V1(APUSYS_CTRL_DAPC_AO);
#endif

	INFO("[APUAPC_AO] %s done\n", __func__);

	return ret;
}

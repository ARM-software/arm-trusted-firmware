/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm_hwreq.h>
#include <mt_spm_reg.h>
#include <platform_def.h>

static uint32_t spm_hwcg_index2res(uint32_t idx)
{
	uint32_t res;

	if (idx >= HWCG_MAX)
		return 0;

	switch (idx) {
	case HWCG_DDREN:
		res = (MT_SPM_DRAM_S0 | MT_SPM_DRAM_S1 | MT_SPM_EMI);
		break;
	case HWCG_VRF18:
		res = MT_SPM_SYSPLL;
		break;
	case HWCG_INFRA:
		res = MT_SPM_INFRA;
		break;
	case HWCG_PMIC:
		res = MT_SPM_PMIC;
		break;
	case HWCG_F26M:
		res = MT_SPM_26M;
		break;
	case HWCG_VCORE:
		res = MT_SPM_VCORE;
		break;
	default:
		res = 0;
	}
	return res;
}

static uint32_t spm_hwcg_ctrl_get(struct spm_hwcg_info *info,
				  enum spm_hwcg_setting type)
{
	uint32_t reg = 0;

	if (!info)
		return 0;

	switch (type) {
	case HWCG_PWR:
		reg = info->pwr;
		break;
	case HWCG_PWR_MSB:
		reg = info->pwr_msb;
		break;
	default:
		reg = info->module_busy;
		break;
	}
	return reg;
}

static void __spm_hwcg_ctrl(struct spm_hwcg_info *info,
			    enum spm_hwcg_setting type, uint32_t is_set,
			    uint32_t val)
{
	uint32_t reg;

	reg = spm_hwcg_ctrl_get(info, type);

	if (!reg)
		return;

	if (is_set)
		mmio_setbits_32(reg, val);
	else
		mmio_clrbits_32(reg, val);
}

void spm_hwcg_ctrl(uint32_t res, enum spm_hwcg_setting type, uint32_t is_set,
		   uint32_t val)
{
	struct spm_hwcg_info info;

	if (res & (MT_SPM_DRAM_S0 | MT_SPM_DRAM_S1 | MT_SPM_EMI))
		DECLARE_HWCG_REG(DDREN, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_HWCG_REG(VRF18, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_HWCG_REG(INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_HWCG_REG(PMIC, info);
	else if (res & MT_SPM_26M)
		DECLARE_HWCG_REG(F26M, info);
	else if (res & MT_SPM_VCORE)
		DECLARE_HWCG_REG(VCORE, info);
	else
		HWCG_INFO_INIT(info);

	if (info.pwr)
		__spm_hwcg_ctrl(&info, type, is_set, val);
}

void spm_hwcg_ctrl_by_index(uint32_t idx, enum spm_hwcg_setting type,
			    uint32_t is_set, uint32_t val)
{
	uint32_t res = spm_hwcg_index2res(idx);

	if (res)
		spm_hwcg_ctrl(res, type, is_set, val);
}

static uint32_t spm_hwcg_mask_get(uint32_t res, enum spm_hwcg_setting type)
{
	struct spm_hwcg_info info;
	uint32_t raw_val = 0, reg = 0;

	if (res & (MT_SPM_DRAM_S0 | MT_SPM_DRAM_S1 | MT_SPM_EMI))
		DECLARE_HWCG_REG(DDREN, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_HWCG_REG(VRF18, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_HWCG_REG(INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_HWCG_REG(PMIC, info);
	else if (res & MT_SPM_26M)
		DECLARE_HWCG_REG(F26M, info);
	else if (res & MT_SPM_VCORE)
		DECLARE_HWCG_REG(VCORE, info);
	else
		HWCG_INFO_INIT(info);

	if (!info.pwr)
		return 0;

	reg = spm_hwcg_ctrl_get(&info, type);

	if (!reg)
		return 0;

	raw_val = ~mmio_read_32(reg);

	return raw_val;
}

static uint32_t spm_hwcg_get_default(uint32_t res, enum spm_hwcg_setting type)
{
	struct spm_hwcg_info info;

	if (res & (MT_SPM_DRAM_S0 | MT_SPM_DRAM_S1 | MT_SPM_EMI))
		DECLARE_HWCG_DEFAULT(DDREN, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_HWCG_DEFAULT(VRF18, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_HWCG_DEFAULT(INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_HWCG_DEFAULT(PMIC, info);
	else if (res & MT_SPM_26M)
		DECLARE_HWCG_DEFAULT(F26M, info);
	else if (res & MT_SPM_VCORE)
		DECLARE_HWCG_DEFAULT(VCORE, info);
	else
		HWCG_INFO_INIT(info);

	if (!info.pwr)
		return 0;

	return spm_hwcg_ctrl_get(&info, type);
}

#define _APMIXEDSYS(ofs) (APMIXEDSYS + ofs)
#define PLL_UNIV _APMIXEDSYS(0x314)
#define PLL_MM _APMIXEDSYS(0x324)
#define PLL_MSDC _APMIXEDSYS(0x35C)
#define PLL_UFS _APMIXEDSYS(0x36C)
#define PLLEN_ALL _APMIXEDSYS(0x070)
#define PLL_UNIV_MERG BIT(5)
#define PLL_MM_MERG BIT(3)
#define PLL_MSDC_MERG BIT(4)
#define PLL_UFS_MERG BIT(2)

uint32_t spm_hwcg_get_status(uint32_t idx, enum spm_hwcg_setting type)
{
	uint32_t val = 0;
	uint32_t pllen_all = 0;

	switch (type) {
	case HWCG_PWR:
		val = mmio_read_32(PWR_STATUS);
		break;
	case HWCG_PWR_MSB:
		val = mmio_read_32(PWR_STATUS_MSB);
		break;
	default:
		pllen_all = mmio_read_32(PLLEN_ALL);

		if ((mmio_read_32(PLL_UNIV) & 0x1) ||
		    (pllen_all & PLL_UNIV_MERG))
			val |= BIT(HWCG_MODULE_UNIVPLL);
		if ((mmio_read_32(PLL_MSDC) & 0x1) ||
		    (pllen_all & PLL_MSDC_MERG))
			val |= BIT(HWCG_MODULE_MSDCPLL);
		if ((mmio_read_32(PLL_UFS) & 0x1) || (pllen_all & PLL_UFS_MERG))
			val |= BIT(HWCG_MODULE_UFSPLL);
		if ((mmio_read_32(PLL_MM) & 0x1) || (pllen_all & PLL_MM_MERG))
			val |= BIT(HWCG_MODULE_MMPLL);
		break;
	}
	return val;
}

int spm_hwcg_get_setting(uint32_t res, enum spm_hwcg_sta_type sta_type,
			 enum spm_hwcg_setting type, struct spm_hwcg_sta *sta)
{
	int ret = 0;

	if (!sta)
		return -1;

	switch (sta_type) {
	case HWCG_STA_DEFAULT_MASK:
		sta->sta = spm_hwcg_get_default(res, type);
		break;
	case HWCG_STA_MASK:
		sta->sta = spm_hwcg_mask_get(res, type);
		break;
	default:
		ret = -1;
		MT_SPM_HW_CG_STA_INIT(sta);
		break;
	}
	return ret;
}

int spm_hwcg_get_setting_by_index(uint32_t idx, enum spm_hwcg_sta_type sta_type,
				  enum spm_hwcg_setting type,
				  struct spm_hwcg_sta *sta)
{
	uint32_t res = spm_hwcg_index2res(idx);

	return spm_hwcg_get_setting(res, sta_type, type, sta);
}

static void spm_infra_swcg_init(void)
{
	mmio_write_32(INFRA_SW_CG_0_MASK, ~INFRA_SW_CG_MB);
	mmio_write_32(INFRA_SW_CG_1_MASK, ~INFRA_SW_CG_MB);
	mmio_write_32(INFRA_SW_CG_2_MASK, ~INFRA_SW_CG_MB);
	mmio_write_32(INFRA_SW_CG_3_MASK, ~INFRA_SW_CG_MB);
	mmio_write_32(INFRA_SW_CG_4_MASK, ~INFRA_SW_CG_MB);
}

static void spm_hwcg_init(void)
{
	/* HW CG for ddren, apsrc, emi resource req */
	mmio_write_32(REG_PWR_STATUS_DDREN_REQ_MASK,
		      ~SPM_HWCG_DDREN_PWR_MB);
	mmio_write_32(REG_PWR_STATUS_MSB_DDREN_REQ_MASK,
		      ~SPM_HWCG_DDREN_PWR_MSB_MB);
	mmio_write_32(REG_MODULE_BUSY_DDREN_REQ_MASK,
		      ~SPM_HWCG_DDREN_MODULE_BUSY_MB);

	/* HW CG for vrf18 resource req */
	mmio_write_32(REG_PWR_STATUS_VRF18_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VRF18_PWR_MB));
	mmio_write_32(REG_PWR_STATUS_MSB_VRF18_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VRF18_PWR_MSB_MB));
	mmio_write_32(REG_MODULE_BUSY_VRF18_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VRF18_MODULE_BUSY_MB));

	/* HW CG for infra resource req */
	mmio_write_32(REG_PWR_STATUS_INFRA_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_INFRA_PWR_MB));
	mmio_write_32(REG_PWR_STATUS_MSB_INFRA_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_INFRA_PWR_MSB_MB));
	mmio_write_32(REG_MODULE_BUSY_INFRA_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_INFRA_MODULE_BUSY_MB));

	/* HW CG for pmic resource req */
	mmio_write_32(REG_PWR_STATUS_PMIC_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_PMIC_PWR_MB));
	mmio_write_32(REG_PWR_STATUS_MSB_PMIC_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_PMIC_PWR_MSB_MB));
	mmio_write_32(REG_MODULE_BUSY_PMIC_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_PMIC_MODULE_BUSY_MB));

	/* HW CG for f26m resource req */
	mmio_write_32(REG_PWR_STATUS_F26M_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_F26M_PWR_MB));
	mmio_write_32(REG_PWR_STATUS_MSB_F26M_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_F26M_PWR_MSB_MB));
	mmio_write_32(REG_MODULE_BUSY_F26M_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_F26M_MODULE_BUSY_MB));

	/* HW CG for vcore resource req */
	mmio_write_32(REG_PWR_STATUS_VCORE_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VCORE_PWR_MB));
	mmio_write_32(REG_PWR_STATUS_MSB_VCORE_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VCORE_PWR_MSB_MB));
	mmio_write_32(REG_MODULE_BUSY_VCORE_REQ_MASK,
		      (uint32_t)(~SPM_HWCG_VCORE_MODULE_BUSY_MB));
}

#define PERI_CG(ofs) (PERICFG_AO_BASE + 0x10 + (0x4 * ofs))
#define PERI_REQ_DDREN_MB	(BIT(PERI_REQ_EN_DMA)	|\
				BIT(PERI_REQ_EN_UART1)	|\
				BIT(PERI_REQ_EN_UART2)	|\
				BIT(PERI_REQ_EN_UART3)	|\
				BIT(PERI_REQ_EN_PWM)	|\
				BIT(PERI_REQ_EN_SPI0)	|\
				BIT(PERI_REQ_EN_SPI1)	|\
				BIT(PERI_REQ_EN_SPI2)	|\
				BIT(PERI_REQ_EN_SPI3)	|\
				BIT(PERI_REQ_EN_SPI4)	|\
				BIT(PERI_REQ_EN_SPI5)	|\
				BIT(PERI_REQ_EN_I2C)	|\
				BIT(PERI_REQ_EN_MSDC0)	|\
				BIT(PERI_REQ_EN_MSDC1)	|\
				BIT(PERI_REQ_EN_MSDC2)	|\
				BIT(PERI_REQ_EN_SSUSB0)	|\
				BIT(PERI_REQ_EN_SSUSB1)	|\
				BIT(PERI_REQ_EN_SSUSB2)	|\
				BIT(PERI_REQ_EN_SSUSB3)	|\
				BIT(PERI_REQ_EN_SSUSB4)	|\
				BIT(PERI_REQ_EN_PEXTP)	|\
				BIT(PERI_REQ_EN_AFE))

#define PERI_REQ_26M_MB		(BIT(PERI_REQ_EN_DMA)	|\
				BIT(PERI_REQ_EN_UART1)	|\
				BIT(PERI_REQ_EN_UART2)	|\
				BIT(PERI_REQ_EN_UART3)	|\
				BIT(PERI_REQ_EN_PWM)	|\
				BIT(PERI_REQ_EN_SPI0)	|\
				BIT(PERI_REQ_EN_SPI1)	|\
				BIT(PERI_REQ_EN_SPI2)	|\
				BIT(PERI_REQ_EN_SPI3)	|\
				BIT(PERI_REQ_EN_SPI4)	|\
				BIT(PERI_REQ_EN_SPI5)	|\
				BIT(PERI_REQ_EN_I2C)	|\
				BIT(PERI_REQ_EN_MSDC0)	|\
				BIT(PERI_REQ_EN_MSDC1)	|\
				BIT(PERI_REQ_EN_MSDC2)	|\
				BIT(PERI_REQ_EN_SSUSB0)	|\
				BIT(PERI_REQ_EN_SSUSB1)	|\
				BIT(PERI_REQ_EN_SSUSB2)	|\
				BIT(PERI_REQ_EN_SSUSB4)	|\
				BIT(PERI_REQ_EN_PEXTP)	|\
				BIT(PERI_REQ_EN_AFE))

#define PERI_REQ_APSRC_MB (PERI_REQ_DDREN_MB)
#define PERI_REQ_EMI_MB (PERI_REQ_DDREN_MB)
#define PERI_REQ_INFRA_MB (PERI_REQ_DDREN_MB)
#define PERI_REQ_SYSPLL_MB (PERI_REQ_DDREN_MB)
#define PERI_REQ_PMIC_MB (PERI_REQ_DDREN_MB)
#define PERI_REQ_F26M_MB (PERI_REQ_26M_MB)

uint32_t spm_peri_req_get_status(uint32_t idx, enum spm_peri_req_status type)
{
	uint32_t val = 0, reg = 0;
	struct spm_peri_req_info info;

	switch (type) {
	case PERI_RES_REQ_EN:

		switch (idx) {
		case PERI_REQ_DDREN:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_DDREN, info);

			break;
		case PERI_REQ_APSRC:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_APSRC, info);

			break;
		case PERI_REQ_EMI:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_EMI, info);

			break;
		case PERI_REQ_SYSPLL:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_SYSPLL, info);

			break;
		case PERI_REQ_INFRA:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_INFRA, info);

			break;
		case PERI_REQ_PMIC:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_PMIC, info);

			break;
		case PERI_REQ_F26M:
			DECLARE_PERI_REQ_STA_REG(PERI_REQ_F26M, info);

			break;
		default:
			PERI_REQ_STA_INFO_INIT(info);
			break;
		}

		if (!info.req_sta)
			return 0;

		reg = info.req_sta;
		val = (mmio_read_32(reg) & PERI_REQ_EN_MASK);

		break;
	default:
		break;
	}
	return val;
}

int spm_peri_req_name(uint32_t idex, char *name, size_t sz)
{
	int ret = 0;

	if (!name)
		return -1;

	switch (idex) {
	case PERI_REQ_DDREN:
		ret = snprintf(name, sz - 1, "ddren");
		break;
	case PERI_REQ_APSRC:
		ret = snprintf(name, sz - 1, "apsrc");
		break;
	case PERI_REQ_EMI:
		ret = snprintf(name, sz - 1, "emi");
		break;
	case PERI_REQ_SYSPLL:
		ret = snprintf(name, sz - 1, "syspll");
		break;
	case PERI_REQ_INFRA:
		ret = snprintf(name, sz - 1, "infra");
		break;
	case PERI_REQ_PMIC:
		ret = snprintf(name, sz - 1, "pmic");
		break;
	case PERI_REQ_F26M:
		ret = snprintf(name, sz - 1, "26m_vcore");
		break;
	default:
		ret = -1;
		break;
	}

	name[sz - 1] = '\0';

	return ret;
}

uint32_t spm_peri_req_get_status_raw(enum spm_peri_req_status_raw type,
				     uint32_t idx, char *name, size_t sz)
{
	return 0;
}

static uint32_t spm_peri_req_get_default(uint32_t res)
{
	struct spm_peri_req_info info;

	if (res & MT_SPM_DRAM_S1)
		DECLARE_PERI_REQ_DEFAULT(DDREN, info);
	else if (res & MT_SPM_DRAM_S0)
		DECLARE_PERI_REQ_DEFAULT(APSRC, info);
	else if (res & MT_SPM_EMI)
		DECLARE_PERI_REQ_DEFAULT(EMI, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_PERI_REQ_DEFAULT(SYSPLL, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_PERI_REQ_DEFAULT(INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_PERI_REQ_DEFAULT(PMIC, info);
	else if (res & (MT_SPM_26M | MT_SPM_VCORE))
		DECLARE_PERI_REQ_DEFAULT(F26M, info);
	else
		PERI_REQ_EN_INFO_INIT(info);

	return info.req_en;
}

static uint32_t spm_peri_req_mask_get(uint32_t res)
{
	struct spm_peri_req_info info;
	uint32_t raw_val = 0, reg = 0;

	if (res & MT_SPM_DRAM_S1)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_DDREN, info);
	else if (res & MT_SPM_DRAM_S0)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_APSRC, info);
	else if (res & MT_SPM_EMI)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_EMI, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_SYSPLL, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_PMIC, info);
	else if (res & (MT_SPM_26M | MT_SPM_VCORE))
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_F26M, info);
	else
		PERI_REQ_EN_INFO_INIT(info);

	if (!info.req_en)
		return 0;

	reg = info.req_en;

	raw_val = (mmio_read_32(reg) & PERI_REQ_EN_MASK);

	return raw_val;
}

int spm_peri_req_get_setting(uint32_t res, enum spm_peri_req_sta_type sta_type,
			     struct spm_peri_req_sta *sta)
{
	int ret = 0;

	if (!sta)
		return -1;

	switch (sta_type) {
	case PERI_REQ_STA_DEFAULT_MASK:
		sta->sta = spm_peri_req_get_default(res);
		break;
	case PERI_REQ_STA_MASK:
		sta->sta = spm_peri_req_mask_get(res);
		break;
	default:
		ret = -1;
		MT_SPM_HW_CG_STA_INIT(sta);
		break;
	}
	return ret;
}

static uint32_t spm_peri_req_index2res(uint32_t idx)
{
	uint32_t res;

	if (idx >= PERI_REQ_MAX)
		return 0;

	switch (idx) {
	case PERI_REQ_DDREN:
		res = MT_SPM_DRAM_S1;
		break;
	case PERI_REQ_APSRC:
		res = MT_SPM_DRAM_S0;
		break;
	case PERI_REQ_EMI:
		res = MT_SPM_EMI;
		break;
	case PERI_REQ_SYSPLL:
		res = MT_SPM_SYSPLL;
		break;
	case PERI_REQ_INFRA:
		res = MT_SPM_INFRA;
		break;
	case PERI_REQ_PMIC:
		res = (MT_SPM_PMIC);
		break;
	case PERI_REQ_F26M:
		res = (MT_SPM_26M | MT_SPM_VCORE);
		break;
	default:
		res = 0;
	}
	return res;
}

int spm_peri_req_get_setting_by_index(uint32_t idx,
				      enum spm_peri_req_sta_type sta_type,
				      struct spm_peri_req_sta *sta)
{
	uint32_t res = spm_peri_req_index2res(idx);

	return spm_peri_req_get_setting(res, sta_type, sta);
}

static void __spm_peri_req_ctrl(struct spm_peri_req_info *info, uint32_t is_set,
				uint32_t val)
{
	uint32_t reg;

	if (!info)
		return;

	reg = info->req_en;

	if (!reg)
		return;

	if (is_set)
		mmio_setbits_32(reg, val);
	else
		mmio_clrbits_32(reg, val);
}

void spm_peri_req_ctrl(uint32_t res, uint32_t is_set, uint32_t val)
{
	struct spm_peri_req_info info;

	if (res & MT_SPM_DRAM_S1)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_DDREN, info);
	else if (res & MT_SPM_DRAM_S0)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_APSRC, info);
	else if (res & MT_SPM_EMI)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_EMI, info);
	else if (res & MT_SPM_SYSPLL)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_SYSPLL, info);
	else if (res & MT_SPM_INFRA)
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_INFRA, info);
	else if (res & MT_SPM_PMIC)
		DECLARE_PERI_REQ_EN_REG(MT_SPM_PMIC, info);
	else if (res & (MT_SPM_26M | MT_SPM_VCORE))
		DECLARE_PERI_REQ_EN_REG(PERI_REQ_F26M, info);
	else
		PERI_REQ_EN_INFO_INIT(info);

	if (info.req_en)
		__spm_peri_req_ctrl(&info, is_set, val);
}

void spm_peri_req_ctrl_by_index(uint32_t idx, uint32_t is_set, uint32_t val)
{
	uint32_t res = spm_peri_req_index2res(idx);

	if (res)
		spm_peri_req_ctrl(res, is_set, val);
}

static void spm_peri_req_init(void)
{
	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_DDREN), PERI_REQ_DDREN_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_EMI), PERI_REQ_EMI_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_APSRC), PERI_REQ_APSRC_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_INFRA), PERI_REQ_INFRA_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_SYSPLL), PERI_REQ_SYSPLL_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_PMIC), PERI_REQ_PMIC_MB);

	mmio_write_32(REG_PERI_REQ_EN(PERI_REQ_F26M), PERI_REQ_F26M_MB);
}

void spm_hwreq_init(void)
{
	spm_infra_swcg_init();
	spm_hwcg_init();
	spm_peri_req_init();
}

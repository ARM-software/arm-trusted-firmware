/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <lib/mmio.h>

#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <platform_def.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

#define MT_LP_TZ_APMIXEDSYS(ofs) (APMIXEDSYS + ofs)
#define MT_LP_TZ_MFG(ofs) (MFGSYS_BASE + 0xFA0000 + ofs)

/***********************************************************
 * Check pll idle condition
 ***********************************************************/
#define PLL_MFG MT_LP_TZ_MFG(0x008)
#define PLL_MFGSC MT_LP_TZ_MFG(0xC08)

/**********************************************************
 * Check pll status other than idle condition
 ***********************************************************/
#define PLL_APLL1 MT_LP_TZ_APMIXEDSYS(0x404)
#define PLL_APLL2 MT_LP_TZ_APMIXEDSYS(0x418)

static struct plat_pll_cond_info pll_info[] = {
	/* pll_addr, pll_mask, pll_bit_set */
	{ PLL_MFG, 0x1, PLL_BIT_MFG },
	{ PLL_MFGSC, 0x1, PLL_BIT_MFGSC },
	{ PLL_APLL1, 0x1, PLL_BIT_APLL1 },
	{ PLL_APLL2, 0x1, PLL_BIT_APLL2 },
	{ 0, 0, PLL_BIT_MAX },
};

static struct plat_cond_info_t plat_cond_info = {
	.pll_cond_table = pll_info,
};

void plat_spm_cond_init(void)
{
	register_plat_cond_info(&plat_cond_info);
}

/*
 * BIT Operation
 */
#define _BIT_(_bit_) (uint32_t)(1 << (_bit_))
#define _BITS_(_bits_, _val_)                                                                 \
		((((uint32_t) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1)) &\
		((_val_)<<((0) ? _bits_)))
#define _BITMASK_(_bits_)  (((uint32_t) -1 >> (31 - ((1) ? _bits_))) &\
		~((1U << ((0) ? _bits_)) - 1))
#define _GET_BITS_VAL_(_bits_, _val_)  (((_val_) & (_BITMASK_(_bits_))) >> ((0) ? _bits_))

/*
 * PMIC_WRAP
 */
#define VCORE_BASE_UV 40000 /* PMIC MT6365 */
#define VOLT_TO_PMIC_VAL(volt) (((volt) - VCORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT(pmic) (((pmic) * 625) + VCORE_BASE_UV)

#define NR_PMIC_WRAP_CMD (NR_IDX_ALL)
#define MAX_RETRY_COUNT (100)
#define SPM_DATA_SHIFT (16)

/* MT6365 */
#define MT6365_BUCK_VCORE 0x17B2
#define MT6365_BUCK_VCORE_SRAM 0x15B4
#define MT6365_BUCK_VADSP_SRAM 0x204E
#define MT6365_BUCK_VADSP_SRAM_LOCK 0x03A8
#define MT6365_BUCK_O2_CON0 0x178A
#define MT6365_BUCK_O2_CON1 0x178C
#define MT6365_BUCK_O2_CON2 0x0456
#define MT6365_BUCK_VMDDR 0x1488

/* MT6319 */
#define MT6319_BUCK_VDDQ_CON0 0x1442
#define MT6319_BUCK_VDDQ_CON1 0x1441

static struct pmic_wrap_cmd_setting cmd_table[NR_PMIC_WRAP_CMD] = {
	{
		(uint32_t)SPM_PWRAP_CMD0,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(55000)),
	}, /* 0.575 V */
	{
		(uint32_t)SPM_PWRAP_CMD1,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(60000)),
	}, /* 0.600 V */
	{
		(uint32_t)SPM_PWRAP_CMD2,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(65000)),
	}, /* 0.650 V */
	{
		(uint32_t)SPM_PWRAP_CMD3,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(72500)),
	}, /* 0.725 V */
	{
		(uint32_t)SPM_PWRAP_CMD4,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(80000)),
	}, /* 0.8 V */
	{
		(uint32_t)SPM_PWRAP_CMD5,
		MT6365_BUCK_VCORE_SRAM,
		0x38,
	}, /* VCORE_SRAM 0.75 V */
	{
		(uint32_t)SPM_PWRAP_CMD6,
		MT6365_BUCK_VCORE_SRAM,
		0x40,
	}, /* VCORE_SRAM 0.8 V */
	{
		(uint32_t)SPM_PWRAP_CMD7,
		MT6365_BUCK_VADSP_SRAM,
		0x150,
	}, /* VADSP_SRAM 0.75 V */
	{
		(uint32_t)SPM_PWRAP_CMD8,
		MT6365_BUCK_VADSP_SRAM,
		0x200,
	}, /* VADSP_SRAM 0.8 V */
	{
		(uint32_t)SPM_PWRAP_CMD9,
		MT6365_BUCK_O2_CON0,
		0x2,
	}, /* VCORE LP */
	{
		(uint32_t)SPM_PWRAP_CMD10,
		MT6365_BUCK_O2_CON1,
		0x2,
	}, /* VCORE NO_LP */
	{
		(uint32_t)SPM_PWRAP_CMD11,
		MT6365_BUCK_O2_CON2,
		0x1,
	}, /* NORMAL */
	{
		(uint32_t)SPM_PWRAP_CMD12,
		MT6365_BUCK_O2_CON2,
		0x0,
	}, /* SLEEP */
	{
		(uint32_t)SPM_PWRAP_CMD13,
		MT6319_BUCK_VDDQ_CON0,
		0x8,
	}, /* VDDQ OFF */
	{
		(uint32_t)SPM_PWRAP_CMD14,
		MT6319_BUCK_VDDQ_CON1,
		0x8,
	}, /* VDDQ ON */
	{
		(uint32_t)SPM_PWRAP_CMD15,
		MT6365_BUCK_VADSP_SRAM_LOCK,
		0x9ca6,
	}, /* unlock key */
	{
		(uint32_t)SPM_PWRAP_CMD16,
		MT6365_BUCK_VADSP_SRAM_LOCK,
		0x0,
	}, /* unlock key */
	{
		(uint32_t)SPM_PWRAP_CMD17,
		MT6365_BUCK_VMDDR,
		0,
	}, /* VMDDR off CMD */
	{
		(uint32_t)SPM_PWRAP_CMD18,
		MT6365_BUCK_VMDDR,
		1,
	}, /* VMDDR on CMD */
	{
		(uint32_t)SPM_PWRAP_CMD19,
		0,
		0,
	}, /* spare CMD */
	{
		(uint32_t)SPM_PWRAP_CMD20,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(55000)),
	}, /* 0.575 V */
	{
		(uint32_t)SPM_PWRAP_CMD21,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(60000)),
	}, /* 0.600 V */
	{
		(uint32_t)SPM_PWRAP_CMD22,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(65000)),
	}, /* 0.650 V */
	{
		(uint32_t)SPM_PWRAP_CMD23,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(72500)),
	}, /* 0.725 V */
	{
		(uint32_t)SPM_PWRAP_CMD24,
		MT6365_BUCK_VCORE,
		_BITS_(7 : 0, VOLT_TO_PMIC_VAL(80000)),
	}, /* 0. 8 V */
};

static struct pmic_wrap_phase_setting phase_table[NR_PMIC_WRAP_PHASE] = {
	{
		.cmd = cmd_table,
		.nr_idx = NR_IDX_ALL,
	},
};

static struct pmic_wrap_setting pmic_wrap_table = {
	.phase = phase_table,
	.phase_nr_idx = NR_PMIC_WRAP_PHASE,
};

void plat_spm_pmic_wrap_init(void)
{
	mt_spm_pmic_wrap_set_table(&pmic_wrap_table);
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
}

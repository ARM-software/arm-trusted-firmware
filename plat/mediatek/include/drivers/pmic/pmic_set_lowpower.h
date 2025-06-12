/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_SET_LOWPOWER_H
#define PMIC_SET_LOWPOWER_H

#include <stdint.h>

#include <drivers/spmi_api.h>

#include "mt6316_lowpower_reg.h"
#include "mt6319_lowpower_reg.h"
#include "mt6363_lowpower_reg.h"
#include "mt6373_lowpower_reg.h"

#define OP_MODE_MU	0
#define OP_MODE_LP	1

#define HW_OFF		0
#define HW_ON		0
#define HW_LP		1
#define HW_ONLV		(0x10 | 1)
#define NORMAL_OP_CFG	0x10

enum {
	RC0 = 0,
	RC1,
	RC2,
	RC3,
	RC4,
	RC5,
	RC6,
	RC7,
	RC8 = 0,
	RC9,
	RC10,
	RC11,
	RC12,
	RC13,
	HW0 = 0,
	HW1,
	HW2,
	HW3,
	HW4,
	HW5,
	HW6,
	HW7,
	HW8 = 0,
	HW9,
	HW10,
	HW11,
	HW12,
	HW13,
};

#define VOTER_EN_SET	1
#define VOTER_EN_CLR	2

enum {
	VOTER_EN_LO_BIT0 = 0,
	VOTER_EN_LO_BIT1,
	VOTER_EN_LO_BIT2,
	VOTER_EN_LO_BIT3,
	VOTER_EN_LO_BIT4,
	VOTER_EN_LO_BIT5,
	VOTER_EN_LO_BIT6,
	VOTER_EN_LO_BIT7,
	VOTER_EN_HI_BIT0 = 0,
	VOTER_EN_HI_BIT1,
	VOTER_EN_HI_BIT2,
	VOTER_EN_HI_BIT3,
};

enum {
	MT6363_SLAVE = SPMI_SLAVE_4,
	MT6368_SLAVE = SPMI_SLAVE_5,
	MT6369_SLAVE = SPMI_SLAVE_5,
	MT6373_SLAVE = SPMI_SLAVE_5,
	MT6316_S6_SLAVE = SPMI_SLAVE_6,
	MT6316_S7_SLAVE = SPMI_SLAVE_7,
	MT6316_S8_SLAVE = SPMI_SLAVE_8,
	MT6316_S15_SLAVE = SPMI_SLAVE_15,
	MT6319_S6_SLAVE = SPMI_SLAVE_6,
	MT6319_S7_SLAVE = SPMI_SLAVE_7,
	MT6319_S8_SLAVE = SPMI_SLAVE_8,
	MT6319_S15_SLAVE = SPMI_SLAVE_15,
};

extern struct spmi_device *lowpower_sdev[SPMI_MAX_SLAVE_ID];

#define PMIC_BUCK_SET_LP(_chip, _name, _user, _en, _mode, _cfg) \
{ \
	uint8_t val = 0; \
	struct spmi_device *sdev = lowpower_sdev[_chip##_SLAVE]; \
	if (sdev && \
	    !spmi_ext_register_readl(sdev, _chip##_RG_BUCK_##_name##_HW0_OP_CFG_ADDR, &val, 1) && \
	    !(val & NORMAL_OP_CFG)) {\
		if ((_cfg) == HW_ONLV) { \
			pmic_spmi_update_bits(sdev, \
					      _chip##_RG_BUCK_##_name##_ONLV_EN_ADDR, \
					      (1 << _chip##_RG_BUCK_##_name##_ONLV_EN_SHIFT), \
					      (1 << _chip##_RG_BUCK_##_name##_ONLV_EN_SHIFT)); \
		} else if ((_cfg) == HW_LP) { \
			pmic_spmi_update_bits(sdev, \
					      _chip##_RG_BUCK_##_name##_ONLV_EN_ADDR, \
					      (1 << _chip##_RG_BUCK_##_name##_ONLV_EN_SHIFT), \
					      0); \
		} \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_CFG_ADDR, \
				      1 << (_user), \
				      ((_cfg) & 0x1) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_MODE_ADDR, \
				      1 << (_user), \
				      (_mode) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_EN_ADDR, \
				      1 << (_user), \
				      (_en) ? 1 << (_user) : 0); \
	} \
}

#define PMIC_LDO_SET_LP(_chip, _name, _user, _en, _mode, _cfg) \
{ \
	uint8_t val = 0; \
	struct spmi_device *sdev = lowpower_sdev[_chip##_SLAVE]; \
	if (sdev && \
	    !spmi_ext_register_readl(sdev, _chip##_RG_LDO_##_name##_HW0_OP_CFG_ADDR, &val, 1) && \
	    !(val & NORMAL_OP_CFG)) {\
		if ((_cfg) == HW_ONLV) { \
			pmic_spmi_update_bits(sdev, \
					      _chip##_RG_LDO_##_name##_ONLV_EN_ADDR, \
					      (1 << _chip##_RG_LDO_##_name##_ONLV_EN_SHIFT), \
					      (1 << _chip##_RG_LDO_##_name##_ONLV_EN_SHIFT)); \
		} else { \
			pmic_spmi_update_bits(sdev, \
					      _chip##_RG_LDO_##_name##_ONLV_EN_ADDR, \
					      (1 << _chip##_RG_LDO_##_name##_ONLV_EN_SHIFT), \
					      0); \
		} \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_LDO_##_name##_##_user##_OP_CFG_ADDR, \
				      1 << (_user), \
				      ((_cfg) & 0x1) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_LDO_##_name##_##_user##_OP_MODE_ADDR, \
				      1 << (_user), \
				      (_mode) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_LDO_##_name##_##_user##_OP_EN_ADDR, \
				      1 << (_user), \
				      (_en) ? 1 << (_user) : 0); \
	} \
}

#define PMIC_SLVID_BUCK_SET_LP(_chip, _slvid, _name, _user, _en, _mode, _cfg) \
{ \
	struct spmi_device *sdev = lowpower_sdev[_chip##_##_slvid##_SLAVE]; \
	if (sdev) {\
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_CFG_ADDR, \
				      1 << (_user), \
				      (_cfg) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_MODE_ADDR, \
				      1 << (_user), \
				      (_mode) ? 1 << (_user) : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_EN_ADDR, \
				      1 << (_user), \
				      (_en) ? 1 << (_user) : 0); \
	} \
}

#define PMIC_BUCK_VOTER_EN(_chip, _name, _user, _cfg) \
{ \
	struct spmi_device *sdev = lowpower_sdev[_chip##_SLAVE]; \
	if (sdev) {\
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_ADDR + (_cfg), \
				      1 << (_user), \
				      1 << (_user)); \
	} \
}

static inline int pmic_spmi_update_bits(struct spmi_device *sdev, uint16_t reg,
					uint8_t mask, uint8_t val)
{
	uint8_t org = 0;
	int ret = 0;

	ret = spmi_ext_register_readl(sdev, reg, &org, 1);
	if (ret < 0)
		return ret;

	org &= ~mask;
	org |= val & mask;

	ret = spmi_ext_register_writel(sdev, reg, &org, 1);
	return ret;
}

#endif /* PMIC_SET_LOWPOWER_H */

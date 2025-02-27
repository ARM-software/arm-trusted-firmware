/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT6359P_SET_LOWPOWER_H
#define MT6359P_SET_LOWPOWER_H

#include <stdint.h>

#include "mt6359p_lowpower_reg.h"
#include <pmic_wrap_init_common.h>

#define OP_MODE_MU	(0)
#define OP_MODE_LP	(1)

#define HW_OFF		(0)
#define HW_ON		(0)
#define HW_LP		(1)

enum {
	HW0 = 0,
	HW1,
	HW2,
	HW3,
	HW4,
	HW5,
	HW6,
	HW7,
	HW8,
	HW9,
	HW10,
	HW11,
	HW12,
	HW13,
	HW14,
};

#define PMIC_BUCK_SET_LP(_chip, _name, _user, _en, _mode, _cfg) \
{ \
	pmic_wrap_update_bits(_chip##_RG_BUCK_##_name##_##_user##_OP_CFG_ADDR, \
			      1 << _user, \
			      (_cfg & 0x1) ? 1 << _user : 0); \
	pmic_wrap_update_bits(_chip##_RG_BUCK_##_name##_##_user##_OP_MODE_ADDR, \
			      1 << _user, \
			      _mode ? 1 << _user : 0); \
	pmic_wrap_update_bits(_chip##_RG_BUCK_##_name##_##_user##_OP_EN_ADDR, \
			      1 << _user, \
			      _en ? 1 << _user : 0); \
}

#define PMIC_LDO_SET_LP(_chip, _name, _user, _en, _mode, _cfg) \
{ \
	if (_user <= HW2) { \
		pmic_wrap_update_bits(_chip##_RG_LDO_##_name##_OP_MODE_ADDR, \
			1 << (_user + _chip##_RG_LDO_##_name##_OP_MODE_SHIFT), \
			_mode ? 1 << (_user + _chip##_RG_LDO_##_name##_OP_MODE_SHIFT) : 0); \
	} \
	pmic_wrap_update_bits(_chip##_RG_LDO_##_name##_##_user##_OP_CFG_ADDR, \
			      1 << _user, \
			      (_cfg & 0x1) ? 1 << _user : 0); \
	pmic_wrap_update_bits(_chip##_RG_LDO_##_name##_##_user##_OP_EN_ADDR, \
			      1 << _user, \
			      _en ? 1 << _user : 0); \
}

static inline int pmic_wrap_update_bits(uint32_t reg, uint32_t mask, uint32_t val)
{
	uint32_t orig = 0;
	int ret = 0;

	ret = pwrap_read(reg, &orig);
	if (ret < 0)
		return ret;

	orig &= ~mask;
	orig |= val & mask;

	ret = pwrap_write(reg, orig);
	return ret;
}

#endif /* MT6359P_MT6359P_SET_LOWPOWER_H */

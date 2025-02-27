/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>

#include <drivers/pmic/mt6319_lowpower_reg.h>
#include <drivers/pmic/mt6359p_set_lowpower.h>
#include <drivers/pmic/pmic_swap_api.h>
#include <lib/mtk_init/mtk_init.h>
#include <pmic_wrap_init_common.h>

#define PMIC_SLVID_BUCK_SET_LP(_chip, _slvid, _name, _user, _en, _mode, _cfg) \
{ \
	struct spmi_device *sdev = lowpower_sdev[_slvid]; \
	if (sdev) {\
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_CFG_ADDR, \
				      1 << _user, \
				      _cfg ? 1 << _user : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_MODE_ADDR, \
				      1 << _user, \
				      _mode ? 1 << _user : 0); \
		pmic_spmi_update_bits(sdev, \
				      _chip##_RG_BUCK_##_name##_##_user##_OP_EN_ADDR, \
				      1 << _user, \
				      _en ? 1 << _user : 0); \
	} \
}

struct spmi_device *lowpower_sdev[SPMI_MAX_SLAVE_ID];

static const uint8_t lowpower_slvid_arr[] = {
	SPMI_SLAVE_7,
};

static int pmic_spmi_update_bits(struct spmi_device *sdev, uint16_t reg,
				 uint8_t mask, uint8_t val)
{
	uint8_t orig = 0;
	int ret = 0;

	ret = spmi_ext_register_readl(sdev, reg, &orig, 1);
	if (ret < 0)
		return ret;
	orig &= ~mask;
	orig |= val & mask;
	ret = spmi_ext_register_writel(sdev, reg, &orig, 1);
	return ret;
}

static int pmic_lowpower_init(void)
{
	uint8_t i, slvid;

	for (i = 0; i < ARRAY_SIZE(lowpower_slvid_arr); i++) {
		slvid = lowpower_slvid_arr[i];
		lowpower_sdev[slvid] = get_spmi_device(SPMI_MASTER_P_1, slvid);
		if (!lowpower_sdev[slvid])
			return -ENODEV;
	}

	PMIC_SLVID_BUCK_SET_LP(MT6319, SPMI_SLAVE_7, VBUCK3, HW0, true, OP_MODE_LP, HW_LP);

	PMIC_BUCK_SET_LP(MT6359P, VPROC2, HW0, true, OP_MODE_LP, HW_OFF);
	PMIC_BUCK_SET_LP(MT6359P, VPROC2, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VGPU11, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VGPU11, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VS1, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VS1, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VS2, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VS2, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VRF12, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VRF12, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VA12, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VA12, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VA09, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VA09, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VAUX18, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VAUX18, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VXO22, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VXO22, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VUSB, HW0, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VUSB, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6359P, VUFS, HW0, true, OP_MODE_LP, HW_LP);

	return 0;
}

MTK_PLAT_SETUP_0_INIT(pmic_lowpower_init);

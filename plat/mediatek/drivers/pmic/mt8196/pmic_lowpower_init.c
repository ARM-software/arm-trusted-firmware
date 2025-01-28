/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/pmic/pmic_set_lowpower.h>
#include <drivers/pmic/pmic_swap_api.h>
#include <drivers/spmi/spmi_common.h>
#include <lib/mtk_init/mtk_init.h>

#define MASTER_ID		SPMI_MASTER_1

struct spmi_device *lowpower_sdev[SPMI_MAX_SLAVE_ID];

static const uint8_t lowpower_slvid_arr[] = {
	MT6363_SLAVE,
	MT6373_SLAVE,
	MT6316_S6_SLAVE,
	MT6316_S7_SLAVE,
	MT6316_S8_SLAVE,
	MT6316_S15_SLAVE,
};

static int pmic_lowpower_init(void)
{
	uint8_t i, slvid;

	for (i = 0; i < ARRAY_SIZE(lowpower_slvid_arr); i++) {
		slvid = lowpower_slvid_arr[i];
		lowpower_sdev[slvid] = get_spmi_device(MASTER_ID, slvid);
		if (!lowpower_sdev[slvid])
			return -ENODEV;
	}

	/* MT6363 Deep idle, SODI3 */
	/* VREQ config by SCP owner in LK2 */
	PMIC_BUCK_SET_LP(MT6363, VBUCK4, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6363, VBUCK4, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_BUCK_SET_LP(MT6363, VS2, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6363, VS2, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_BUCK_SET_LP(MT6363, VS3, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VSRAM_CPUB, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VSRAM_CPUB, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VSRAM_CPUL, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VSRAM_CPUL, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VSRAM_APU, RC2, true, OP_MODE_MU, HW_OFF);
	PMIC_LDO_SET_LP(MT6363, VSRAM_MODEM, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VSRAM_MODEM, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VA12_1, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VA12_1, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VA12_2, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VA12_2, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VUFS18, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VUFS18, RC9, true, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VUFS12, HW2, true, OP_MODE_LP, HW_LP);

	/* MT6373 Deep idle, SODI3 */
	PMIC_BUCK_SET_LP(MT6373, VBUCK4, HW2, true, OP_MODE_LP, HW_OFF);
	PMIC_BUCK_SET_LP(MT6373, VBUCK5, HW2, true, OP_MODE_LP, HW_OFF);
	PMIC_BUCK_SET_LP(MT6373, VBUCK6, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6373, VUSB, HW2, true, OP_MODE_LP, HW_LP);

	/* MT6316 Deep idle, SODI3 */
	PMIC_SLVID_BUCK_SET_LP(MT6316, S8, VBUCK1, HW2, true, OP_MODE_LP, HW_LP);
	PMIC_SLVID_BUCK_SET_LP(MT6316, S6, VBUCK3, HW2, true, OP_MODE_LP, HW_ONLV);
	return 0;
}

MTK_PLAT_SETUP_0_INIT(pmic_lowpower_init);

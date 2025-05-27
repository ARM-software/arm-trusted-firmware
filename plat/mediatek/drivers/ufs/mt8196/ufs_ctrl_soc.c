/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/pmic/pmic_set_lowpower.h>
#if defined(CONFIG_MTK_MTCMOS)
#include <mtcmos.h>
#endif
#include <mtk_bl31_interface.h>

static void ufs_vsx_lpm(bool lpm, uint64_t ufs_version)
{
	if (lpm) {
		/* MT6363 VS2 voter LOW byte BIT6 vote reduce VS2 voltage */
		PMIC_BUCK_VOTER_EN(MT6363, VS2, VOTER_EN_LO_BIT6, VOTER_EN_CLR);

		/* VS2 buck can enter LPM */
		PMIC_BUCK_SET_LP(MT6363, VS2, HW2, true, OP_MODE_LP, HW_LP);
	} else {
		/* MT6363 VS2 voter LOW byte BIT6 vote raise VS2 voltage */
		PMIC_BUCK_VOTER_EN(MT6363, VS2, VOTER_EN_LO_BIT6, VOTER_EN_SET);

		/* VS2 buck can not enter LPM */
		PMIC_BUCK_SET_LP(MT6363, VS2, HW2, true, OP_MODE_LP, HW_ONLV);
	}
}

void ufs_device_pwr_ctrl_soc(bool vcc_on, uint64_t ufs_version)
{
	if (vcc_on)
		ufs_vsx_lpm(false, ufs_version);
	else
		ufs_vsx_lpm(true, ufs_version);
}

int ufs_spm_mtcmos_power(bool on)
{
#if defined(CONFIG_MTK_MTCMOS)
	return spm_mtcmos_ctrl_ufs0(on ? STA_POWER_ON : STA_POWER_DOWN);
#else
	return 0;
#endif
}

int ufs_phy_spm_mtcmos_power(bool on)
{
#if defined(CONFIG_MTK_MTCMOS)
	return spm_mtcmos_ctrl_ufs0_phy(on ? STA_POWER_ON : STA_POWER_DOWN);
#else
	return 0;
#endif
}

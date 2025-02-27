/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "../mt6359p/registers.h"
#include <drivers/pmic/pmic_shutdown_cfg.h>
#include <drivers/spmi/spmi_common.h>
#include <drivers/spmi_api.h>
#include <lib/mtk_init/mtk_init.h>
#include <pmic_wrap_init_common.h>

#define MT6319_RG_SEQ_OFF		0x2d
#define MT6319_TOP_RST_MISC_CLR		0x128
#define MT6319_TOP_DIG_WPK_H		0x3a9
#define MT6319_TOP_DIG_WPK_H_MASK	0xFF
#define MT6319_TOP_DIG_WPK_H_SHIFT	0
#define MT6319_TOP_DIG_WPK		0x3a8
#define MT6319_TOP_DIG_WPK_MASK		0xFF
#define MT6319_TOP_DIG_WPK_SHIFT	0


int pmic_shutdown_cfg(void)
{
/*
 * In mt8189, the pmic_shutdown_cfg() api does not need to read and write the
 * pmic register to determine the return value and in order not to modify the
 * common code to affect other ICs, the pmic_shutdown_cfg() will directly
 * return 1.
 */
	return 1;
}

static void shutdown_slave_dev(struct spmi_device *dev)
{
	spmi_ext_register_writel_field(dev, MT6319_TOP_DIG_WPK_H, 0x63,
				       MT6319_TOP_DIG_WPK_H_MASK,
				       MT6319_TOP_DIG_WPK_H_SHIFT);
	spmi_ext_register_writel_field(dev, MT6319_TOP_DIG_WPK, 0x15,
				       MT6319_TOP_DIG_WPK_MASK,
				       MT6319_TOP_DIG_WPK_SHIFT);

	/* Disable WDTRSTB_EN */
	spmi_ext_register_writel_field(dev, MT6319_TOP_RST_MISC_CLR, 1, 0x1, 0);
	/* Normal sequence power off when PAD_EN falling */
	spmi_ext_register_writel_field(dev, MT6319_RG_SEQ_OFF, 1, 0x1, 0);

	spmi_ext_register_writel_field(dev, MT6319_TOP_DIG_WPK_H, 0,
				       MT6319_TOP_DIG_WPK_H_MASK,
				       MT6319_TOP_DIG_WPK_H_SHIFT);
	spmi_ext_register_writel_field(dev, MT6319_TOP_DIG_WPK, 0,
				       MT6319_TOP_DIG_WPK_MASK,
				       MT6319_TOP_DIG_WPK_SHIFT);

}

int spmi_shutdown(void)
{
	struct spmi_device *mt6319_sdev;

	mt6319_sdev = get_spmi_device(SPMI_MASTER_P_1, SPMI_SLAVE_7);
	if (!mt6319_sdev)
		return -ENODEV;
	shutdown_slave_dev(mt6319_sdev);

	if (mmio_read_32((uintptr_t)CHIP_ID_REG) == MTK_CPU_ID_MT8189 &&
	    mmio_read_32((uintptr_t)CPU_SEG_ID_REG) == MTK_CPU_SEG_ID_MT8189H) {
		mt6319_sdev = get_spmi_device(SPMI_MASTER_P_1, SPMI_SLAVE_8);
		if (!mt6319_sdev)
			return -ENODEV;
		shutdown_slave_dev(mt6319_sdev);
	}

	/* clear main pmic power hold */
	pwrap_write_field(MT6359P_PPCCTL0, 0, 0x1, 0);

	return 0;
}

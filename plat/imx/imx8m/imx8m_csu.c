/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx8m_csu.h>

void imx_csu_init(const struct imx_csu_cfg *csu_cfg)
{
	const struct imx_csu_cfg *csu = csu_cfg;
	uint32_t val;

	while (csu->type != CSU_INVALID) {
		switch (csu->type) {
		case CSU_CSL:
			val = mmio_read_32(CSLx_REG(csu->idx));
			if (val & CSLx_LOCK(csu->idx)) {
				break;
			}
			mmio_clrsetbits_32(CSLx_REG(csu->idx), CSLx_CFG(0xff, csu->idx),
				CSLx_CFG(csu->csl_level | (csu->lock << 8), csu->idx));
			break;
		case CSU_HP:
			val = mmio_read_32(CSU_HP_REG(csu->idx));
			if (val & CSU_HP_LOCK(csu->idx)) {
				break;
			}
			mmio_clrsetbits_32(CSU_HP_REG(csu->idx), CSU_HP_CFG(0x1, csu->idx),
				CSU_HP_CFG(csu->hp | (csu->lock << 0x1), csu->idx));
			break;
		case CSU_SA:
			val = mmio_read_32(CSU_SA_REG(csu->idx));
			if (val & CSU_SA_LOCK(csu->idx)) {
				break;
			}
			mmio_clrsetbits_32(CSU_SA_REG(csu->idx), CSU_SA_CFG(0x1, csu->idx),
				CSU_SA_CFG(csu->sa | (csu->lock << 0x1), csu->idx));
			break;
		case CSU_HPCONTROL:
			val = mmio_read_32(CSU_HPCONTROL_REG(csu->idx));
			if (val & CSU_HPCONTROL_LOCK(csu->idx)) {
				break;
			}
			mmio_clrsetbits_32(CSU_HPCONTROL_REG(csu->idx), CSU_HPCONTROL_CFG(0x1, csu->idx),
				CSU_HPCONTROL_CFG(csu->hpctrl | (csu->lock << 0x1), csu->idx));
			break;
		default:
			break;
		}

		csu++;
	}
}

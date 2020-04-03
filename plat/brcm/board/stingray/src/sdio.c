/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <sdio.h>
#include <sr_def.h>
#include <sr_utils.h>

const SDIO_CFG sr_sdio0_cfg = {
	.cfg_base = SR_IPROC_SDIO0_CFG_BASE,
	.sid_base = SR_IPROC_SDIO0_SID_BASE,
	.io_ctrl_base = SR_IPROC_SDIO0_IOCTRL_BASE,
	.pad_base = SR_IPROC_SDIO0_PAD_BASE,
};
const SDIO_CFG sr_sdio1_cfg = {
	.cfg_base = SR_IPROC_SDIO1_CFG_BASE,
	.sid_base = SR_IPROC_SDIO1_SID_BASE,
	.io_ctrl_base = SR_IPROC_SDIO1_IOCTRL_BASE,
	.pad_base = SR_IPROC_SDIO1_PAD_BASE,
};

void brcm_stingray_sdio_init(void)
{
	unsigned int val;
	const SDIO_CFG *sdio0_cfg, *sdio1_cfg;

	sdio0_cfg = &sr_sdio0_cfg;
	sdio1_cfg = &sr_sdio1_cfg;

	INFO("set sdio0 caps\n");
	/* SDIO0 CAPS0 */
	val = SDIO0_CAP0_CFG;
	INFO("caps0 0x%x\n", val);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_CAP0, val);

	/* SDIO0 CAPS1 */
	val = SDIO0_CAP1_CFG;
	INFO("caps1 0x%x\n", val);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_CAP1, val);

	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_0,
		      SDIO_PRESETVAL0);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_1,
		      SDIO_PRESETVAL1);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_2,
		      SDIO_PRESETVAL2);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_3,
		      SDIO_PRESETVAL3);
	mmio_write_32(sdio0_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_4,
		      SDIO_PRESETVAL4);

	val = SR_SID_VAL(0x3, 0x0, 0x2) << SDIO_SID_SHIFT;
	mmio_write_32(sdio0_cfg->sid_base + ICFG_SDIO_SID_ARADDR, val);
	mmio_write_32(sdio0_cfg->sid_base + ICFG_SDIO_SID_AWADDR, val);

	val = mmio_read_32(sdio0_cfg->io_ctrl_base);
	val &= ~(0xff << 23); /* Clear ARCACHE and AWCACHE */
	val |= (0xb7 << 23); /* Set ARCACHE and AWCACHE */
	mmio_write_32(sdio0_cfg->io_ctrl_base, val);

	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_CLK,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA0,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA1,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA2,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA3,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA4,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA5,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA6,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_DATA7,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio0_cfg->pad_base + PAD_SDIO_CMD,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);

	INFO("set sdio1 caps\n");

	/* SDIO1 CAPS0 */
	val = SDIO1_CAP0_CFG;
	INFO("caps0 0x%x\n", val);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_CAP0, val);
	/* SDIO1 CAPS1 */
	val = SDIO1_CAP1_CFG;
	INFO("caps1 0x%x\n", val);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_CAP1, val);

	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_0,
		      SDIO_PRESETVAL0);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_1,
		      SDIO_PRESETVAL1);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_2,
		      SDIO_PRESETVAL2);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_3,
		      SDIO_PRESETVAL3);
	mmio_write_32(sdio1_cfg->cfg_base + ICFG_SDIO_STRAPSTATUS_4,
		      SDIO_PRESETVAL4);

	val = SR_SID_VAL(0x3, 0x0, 0x3) << SDIO_SID_SHIFT;
	mmio_write_32(sdio1_cfg->sid_base + ICFG_SDIO_SID_ARADDR, val);
	mmio_write_32(sdio1_cfg->sid_base + ICFG_SDIO_SID_AWADDR, val);

	val = mmio_read_32(sdio1_cfg->io_ctrl_base);
	val &= ~(0xff << 23); /* Clear ARCACHE and AWCACHE */
	val |= (0xb7 << 23); /* Set ARCACHE and AWCACHE */
	mmio_write_32(sdio1_cfg->io_ctrl_base, val);

	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_CLK,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA0,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA1,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA2,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA3,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA4,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA5,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA6,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_DATA7,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);
	mmio_clrsetbits_32(sdio1_cfg->pad_base + PAD_SDIO_CMD,
			   PAD_SDIO_MASK, PAD_SDIO_VALUE);

	INFO("sdio init done\n");
}

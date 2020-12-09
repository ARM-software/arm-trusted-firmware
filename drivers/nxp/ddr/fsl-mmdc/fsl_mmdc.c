/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Generic driver for Freescale MMDC(Multi Mode DDR Controller).
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include "ddr_io.h"
#include <drivers/delay_timer.h>
#include <fsl_mmdc.h>

static void set_wait_for_bits_clear(void *ptr, unsigned int value,
				    unsigned int bits)
{
	int timeout = 1000;

	ddr_out32(ptr, value);

	while ((ddr_in32(ptr) & bits) != 0) {
		udelay(100);
		timeout--;
	}
	if (timeout <= 0) {
		INFO("Error: %llx", (unsigned long long)ptr);
		INFO(" wait for clear timeout.\n");
	}
}

void mmdc_init(const struct fsl_mmdc_info *priv, uintptr_t nxp_ddr_addr)
{
	struct mmdc_regs *mmdc = (struct mmdc_regs *)nxp_ddr_addr;
	unsigned int tmp;

	/* 1. set configuration request */
	ddr_out32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	/* 2. configure the desired timing parameters */
	ddr_out32(&mmdc->mdotc, priv->mdotc);
	ddr_out32(&mmdc->mdcfg0, priv->mdcfg0);
	ddr_out32(&mmdc->mdcfg1, priv->mdcfg1);
	ddr_out32(&mmdc->mdcfg2, priv->mdcfg2);

	/* 3. configure DDR type and other miscellaneous parameters */
	ddr_out32(&mmdc->mdmisc, priv->mdmisc);
	ddr_out32(&mmdc->mpmur0, MMDC_MPMUR0_FRC_MSR);
	ddr_out32(&mmdc->mdrwd, priv->mdrwd);
	ddr_out32(&mmdc->mpodtctrl, priv->mpodtctrl);

	/* 4. configure the required delay while leaving reset */
	ddr_out32(&mmdc->mdor, priv->mdor);

	/* 5. configure DDR physical parameters */
	/* set row/column address width, burst length, data bus width */
	tmp = priv->mdctl & ~(MDCTL_SDE0 | MDCTL_SDE1);
	ddr_out32(&mmdc->mdctl, tmp);
	/* configure address space partition */
	ddr_out32(&mmdc->mdasp, priv->mdasp);

	/* 6. perform a ZQ calibration - not needed here, doing in #8b */

	/* 7. enable MMDC with the desired chip select */
#if (DDRC_NUM_CS == 1)
	ddr_out32(&mmdc->mdctl, tmp | MDCTL_SDE0);
#elif (DDRC_NUM_CS == 2)
	ddr_out32(&mmdc->mdctl, tmp | MDCTL_SDE0 | MDCTL_SDE1);
#else
#error "Unsupported DDRC_NUM_CS"
#endif

	/* 8a. dram init sequence: update MRs for ZQ, ODT, PRE, etc */
	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(8) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_2);

	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(0) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(4) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_1);

	ddr_out32(&mmdc->mdscr, CMD_ADDR_MSB_MR_OP(0x19) |
				CMD_ADDR_LSB_MR_ADDR(0x30) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_0);

	/* 8b. ZQ calibration */
	ddr_out32(&mmdc->mdscr, CMD_ADDR_MSB_MR_OP(0x4) |
				MDSCR_ENABLE_CON_REQ |
				CMD_ZQ_CALIBRATION | CMD_BANK_ADDR_0);

	set_wait_for_bits_clear(&mmdc->mpzqhwctrl, priv->mpzqhwctrl,
				MPZQHWCTRL_ZQ_HW_FORCE);

	/* 9a. calibrations now, wr lvl */
	ddr_out32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(0x84) | MDSCR_WL_EN |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	set_wait_for_bits_clear(&mmdc->mpwlgcr, MPWLGCR_HW_WL_EN,
				MPWLGCR_HW_WL_EN);

	mdelay(1);

	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(4) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	ddr_out32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	mdelay(1);

	/* 9b. read DQS gating calibration */
	ddr_out32(&mmdc->mdscr, CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);

	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);

	ddr_out32(&mmdc->mppdcmpr2, MPPDCMPR2_MPR_COMPARE_EN);

	/* set absolute read delay offset */
	if (priv->mprddlctl != 0) {
		ddr_out32(&mmdc->mprddlctl, priv->mprddlctl);
	} else {
		ddr_out32(&mmdc->mprddlctl, MMDC_MPRDDLCTL_DEFAULT_DELAY);
	}

	set_wait_for_bits_clear(&mmdc->mpdgctrl0,
				AUTO_RD_DQS_GATING_CALIBRATION_EN,
				AUTO_RD_DQS_GATING_CALIBRATION_EN);

	ddr_out32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 9c. read calibration */
	ddr_out32(&mmdc->mdscr, CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);
	ddr_out32(&mmdc->mdscr, CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);
	ddr_out32(&mmdc->mppdcmpr2,  MPPDCMPR2_MPR_COMPARE_EN);
	set_wait_for_bits_clear(&mmdc->mprddlhwctl,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN);

	ddr_out32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 10. configure power-down, self-refresh entry, exit parameters */
	ddr_out32(&mmdc->mdpdc, priv->mdpdc);
	ddr_out32(&mmdc->mapsr, MMDC_MAPSR_PWR_SAV_CTRL_STAT);

	/* 11. ZQ config again? do nothing here */

	/* 12. refresh scheme */
	set_wait_for_bits_clear(&mmdc->mdref, priv->mdref,
				MDREF_START_REFRESH);

	/* 13. disable CON_REQ */
	ddr_out32(&mmdc->mdscr, MDSCR_DISABLE_CFG_REQ);
}

/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include "dcfg.h"
#include <lib/mmio.h>
#ifdef NXP_SFP_ENABLED
#include <sfp.h>
#endif

static soc_info_t soc_info = {0};
static devdisr5_info_t devdisr5_info = {0};
static dcfg_init_info_t *dcfg_init_info;

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(void)
{
	unsigned int *porsr1_addr = NULL;

	if (dcfg_init_info->porsr1 != 0U) {
		return dcfg_init_info->porsr1;
	}

	porsr1_addr = (void *)
			(dcfg_init_info->g_nxp_dcfg_addr + DCFG_PORSR1_OFFSET);
	dcfg_init_info->porsr1 = gur_in32(porsr1_addr);

	return dcfg_init_info->porsr1;
}


const soc_info_t *get_soc_info(void)
{
	uint32_t reg;

	if (soc_info.is_populated == true) {
		return (const soc_info_t *) &soc_info;
	}

	reg = gur_in32(dcfg_init_info->g_nxp_dcfg_addr + DCFG_SVR_OFFSET);

	soc_info.svr_reg.val = reg;

	/* zero means SEC enabled. */
	soc_info.sec_enabled =
		(((reg & SVR_SEC_MASK) >> SVR_SEC_SHIFT) == 0) ? true : false;

	soc_info.is_populated = true;
	return (const soc_info_t *) &soc_info;
}

void dcfg_init(dcfg_init_info_t *dcfg_init_data)
{
	dcfg_init_info = dcfg_init_data;
	read_reg_porsr1();
	get_soc_info();
}

bool is_sec_enabled(void)
{
	return soc_info.sec_enabled;
}

const devdisr5_info_t *get_devdisr5_info(void)
{
	uint32_t reg;

	if (devdisr5_info.is_populated == true)
		return (const devdisr5_info_t *) &devdisr5_info;

	reg = gur_in32(dcfg_init_info->g_nxp_dcfg_addr + DCFG_DEVDISR5_OFFSET);

	devdisr5_info.ddrc1_present = (reg & DISR5_DDRC1_MASK) ? 0 : 1;
#if defined(CONFIG_CHASSIS_3_2)
	devdisr5_info.ddrc2_present = (reg & DISR5_DDRC2_MASK) ? 0 : 1;
#endif
	devdisr5_info.ocram_present = (reg & DISR5_OCRAM_MASK) ? 0 : 1;
	devdisr5_info.is_populated = true;

	return (const devdisr5_info_t *) &devdisr5_info;
}

int get_clocks(struct sysinfo *sys)
{
	unsigned int *rcwsr0 = NULL;
	const unsigned long sysclk = dcfg_init_info->nxp_sysclk_freq;
	const unsigned long ddrclk = dcfg_init_info->nxp_ddrclk_freq;

	rcwsr0 = (void *)(dcfg_init_info->g_nxp_dcfg_addr + RCWSR0_OFFSET);
	sys->freq_platform = sysclk;
	sys->freq_ddr_pll0 = ddrclk;
	sys->freq_ddr_pll1 = ddrclk;

	sys->freq_platform *= (gur_in32(rcwsr0) >>
				RCWSR0_SYS_PLL_RAT_SHIFT) &
				RCWSR0_SYS_PLL_RAT_MASK;

	sys->freq_platform /= dcfg_init_info->nxp_plat_clk_divider;

	sys->freq_ddr_pll0 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM_PLL_RAT_SHIFT) &
				RCWSR0_MEM_PLL_RAT_MASK;
	sys->freq_ddr_pll1 *= (gur_in32(rcwsr0) >>
				RCWSR0_MEM2_PLL_RAT_SHIFT) &
				RCWSR0_MEM2_PLL_RAT_MASK;
	if (sys->freq_platform == 0) {
		return 1;
	} else {
		return 0;
	}
}

#ifdef NXP_SFP_ENABLED
/*******************************************************************************
 * Returns true if secur eboot is enabled on board
 * mode = 0  (development mode - sb_en = 1)
 * mode = 1 (production mode - ITS = 1)
 ******************************************************************************/
bool check_boot_mode_secure(uint32_t *mode)
{
	uint32_t val = 0U;
	uint32_t *rcwsr = NULL;
	*mode = 0U;

	if (sfp_check_its() == 1) {
		/* ITS =1 , Production mode */
		*mode = 1U;
		return true;
	}

	rcwsr = (void *)(dcfg_init_info->g_nxp_dcfg_addr + RCWSR_SB_EN_OFFSET);

	val = (gur_in32(rcwsr) >> RCWSR_SBEN_SHIFT) &
				RCWSR_SBEN_MASK;

	if (val == RCWSR_SBEN_MASK) {
		*mode = 0U;
		return true;
	}

	return false;
}
#endif

void error_handler(int error_code)
{
	 /* Dump error code in SCRATCH4 register */
	INFO("Error in Fuse Provisioning: %x\n", error_code);
	gur_out32((void *)
		  (dcfg_init_info->g_nxp_dcfg_addr + DCFG_SCRATCH4_OFFSET),
		  error_code);
}

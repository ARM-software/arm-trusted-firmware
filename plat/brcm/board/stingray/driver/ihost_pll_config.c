/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <dmu.h>

#define IHOST0_CONFIG_ROOT	0x66000000
#define IHOST1_CONFIG_ROOT	0x66002000
#define IHOST2_CONFIG_ROOT	0x66004000
#define IHOST3_CONFIG_ROOT	0x66006000
#define A72_CRM_PLL_PWR_ON	0x00000070
#define A72_CRM_PLL_PWR_ON__PLL0_RESETB_R	4
#define A72_CRM_PLL_PWR_ON__PLL0_POST_RESETB_R	5
#define A72_CRM_PLL_CHNL_BYPS_EN		0x000000ac
#define A72_CRM_PLL_CHNL_BYPS_EN__PLL_0_CHNL_0_BYPS_EN_R	0
#define A72_CRM_PLL_CHNL_BYPS_EN_DATAMASK	0x0000ec1f
#define A72_CRM_PLL_CMD				0x00000080
#define A72_CRM_PLL_CMD__UPDATE_PLL0_FREQUENCY_VCO_R		0
#define A72_CRM_PLL_CMD__UPDATE_PLL0_FREQUENCY_POST_R		1
#define A72_CRM_PLL_STATUS			0x00000084
#define A72_CRM_PLL_STATUS__PLL0_LOCK_R		9
#define A72_CRM_PLL0_CTRL1			0x00000100
#define A72_CRM_PLL0_CTRL2 			0x00000104
#define A72_CRM_PLL0_CTRL3 			0x00000108
#define A72_CRM_PLL0_CTRL3__PLL0_PDIV_R 12
#define A72_CRM_PLL0_CTRL4 			0x0000010c
#define A72_CRM_PLL0_CTRL4__PLL0_KP_R		0
#define A72_CRM_PLL0_CTRL4__PLL0_KI_R		4
#define A72_CRM_PLL0_CTRL4__PLL0_KA_R		7
#define A72_CRM_PLL0_CTRL4__PLL0_FREFEFF_INFO_R	10

#define PLL_MODE_VCO		0x0
#define PLL_MODE_BYPASS		0x1
#define PLL_RESET_TYPE_PLL	0x1
#define PLL_RESET_TYPE_POST	0x2
#define PLL_VCO			0x1
#define PLL_POSTDIV		0x2
#define ARM_FREQ_3G		PLL_FREQ_FULL
#define ARM_FREQ_1P5G		PLL_FREQ_HALF
#define ARM_FREQ_750M		PLL_FREQ_QRTR

static unsigned int ARMCOE_crm_getBaseAddress(unsigned int cluster_num)
{
	unsigned int ihostx_config_root;

	switch (cluster_num) {
	case 0:
	default:
		ihostx_config_root = IHOST0_CONFIG_ROOT;
		break;
	case 1:
		ihostx_config_root = IHOST1_CONFIG_ROOT;
		break;
	case 2:
		ihostx_config_root = IHOST2_CONFIG_ROOT;
		break;
	case 3:
		ihostx_config_root = IHOST3_CONFIG_ROOT;
		break;
	}

	return ihostx_config_root;
}

static void ARMCOE_crm_pllAssertReset(unsigned int cluster_num,
				      unsigned int reset_type)
{
	unsigned long ihostx_config_root;
	unsigned int pll_rst_ctrl;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);
	pll_rst_ctrl = mmio_read_32(ihostx_config_root + A72_CRM_PLL_PWR_ON);

	// PLL reset
	if (reset_type & PLL_RESET_TYPE_PLL) {
		pll_rst_ctrl &= ~(0x1<<A72_CRM_PLL_PWR_ON__PLL0_RESETB_R);
	}
	// post-div channel reset
	if (reset_type & PLL_RESET_TYPE_POST) {
		pll_rst_ctrl &= ~(0x1<<A72_CRM_PLL_PWR_ON__PLL0_POST_RESETB_R);
	}

	mmio_write_32(ihostx_config_root + A72_CRM_PLL_PWR_ON, pll_rst_ctrl);
}

static void ARMCOE_crm_pllSetMode(unsigned int cluster_num, unsigned int mode)
{
	unsigned long ihostx_config_root;
	unsigned int pll_byp_ctrl;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);
	pll_byp_ctrl = mmio_read_32(ihostx_config_root +
				    A72_CRM_PLL_CHNL_BYPS_EN);

	if (mode == PLL_MODE_VCO) {
		// use PLL DCO output
		pll_byp_ctrl &=
			~BIT(A72_CRM_PLL_CHNL_BYPS_EN__PLL_0_CHNL_0_BYPS_EN_R);
	} else {
		// use PLL bypass sources
		pll_byp_ctrl |=
			BIT(A72_CRM_PLL_CHNL_BYPS_EN__PLL_0_CHNL_0_BYPS_EN_R);
	}

	mmio_write_32(ihostx_config_root + A72_CRM_PLL_CHNL_BYPS_EN,
		      pll_byp_ctrl);
}

static void ARMCOE_crm_pllFreqSet(unsigned int cluster_num,
				  unsigned int ihost_pll_freq_sel,
				  unsigned int pdiv)
{
	unsigned int ndiv_int;
	unsigned int ndiv_frac_low, ndiv_frac_high;
	unsigned long ihostx_config_root;

	ndiv_frac_low = 0x0;
	ndiv_frac_high = 0x0;

	if (ihost_pll_freq_sel == ARM_FREQ_3G) {
		ndiv_int = 0x78;
	} else if (ihost_pll_freq_sel == ARM_FREQ_1P5G) {
		ndiv_int = 0x3c;
	} else if (ihost_pll_freq_sel == ARM_FREQ_750M) {
		ndiv_int = 0x1e;
	} else {
		return;
	}

	ndiv_int &= 0x3FF;                // low 10 bits
	ndiv_frac_low &= 0x3FF;
	ndiv_frac_high &= 0x3FF;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);

	mmio_write_32(ihostx_config_root+A72_CRM_PLL0_CTRL1, ndiv_frac_low);
	mmio_write_32(ihostx_config_root+A72_CRM_PLL0_CTRL2, ndiv_frac_high);
	mmio_write_32(ihostx_config_root+A72_CRM_PLL0_CTRL3,
		      ndiv_int |
		      ((pdiv << A72_CRM_PLL0_CTRL3__PLL0_PDIV_R & 0xF000)));

	mmio_write_32(ihostx_config_root + A72_CRM_PLL0_CTRL4,
			/* From Section 10 of PLL spec */
			(3 << A72_CRM_PLL0_CTRL4__PLL0_KP_R) |
			/* From Section 10 of PLL spec */
			(2 << A72_CRM_PLL0_CTRL4__PLL0_KI_R) |
			/* Normal mode (i.e. not fast-locking) */
			(0 << A72_CRM_PLL0_CTRL4__PLL0_KA_R) |
			/* 50 MHz */
			(50 << A72_CRM_PLL0_CTRL4__PLL0_FREFEFF_INFO_R));
}

static void ARMCOE_crm_pllDeassertReset(unsigned int cluster_num,
					unsigned int reset_type)
{
	unsigned long ihostx_config_root;
	unsigned int pll_rst_ctrl;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);
	pll_rst_ctrl = mmio_read_32(ihostx_config_root + A72_CRM_PLL_PWR_ON);

	// PLL reset
	if (reset_type & PLL_RESET_TYPE_PLL) {
		pll_rst_ctrl |= (0x1 << A72_CRM_PLL_PWR_ON__PLL0_RESETB_R);
	}

	// post-div channel reset
	if (reset_type & PLL_RESET_TYPE_POST) {
		pll_rst_ctrl |= (0x1 << A72_CRM_PLL_PWR_ON__PLL0_POST_RESETB_R);
	}

	mmio_write_32(ihostx_config_root + A72_CRM_PLL_PWR_ON, pll_rst_ctrl);
}

static void ARMCOE_crm_pllUpdate(unsigned int cluster_num, unsigned int type)
{
	unsigned long ihostx_config_root;
	unsigned int pll_cmd;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);
	pll_cmd = mmio_read_32(ihostx_config_root + A72_CRM_PLL_CMD);

	// VCO update
	if (type & PLL_VCO) {
		pll_cmd |= BIT(A72_CRM_PLL_CMD__UPDATE_PLL0_FREQUENCY_VCO_R);
	}
	// post-div channel update
	if (type & PLL_POSTDIV) {
		pll_cmd |= BIT(A72_CRM_PLL_CMD__UPDATE_PLL0_FREQUENCY_POST_R);
	}

	mmio_write_32(ihostx_config_root+A72_CRM_PLL_CMD, pll_cmd);
}

static void insert_delay(unsigned int delay)
{
	volatile unsigned int index;

	for (index = 0; index < delay; index++)
		;
}


/*
 * Returns 1 if PLL locked within certain interval
 */
static unsigned int ARMCOE_crm_pllIsLocked(unsigned int cluster_num)
{
	unsigned long ihostx_config_root;
	unsigned int lock_status;
	unsigned int i;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);

	/* wait a while for pll to lock before returning from this function */
	for (i = 0; i < 1500; i++) {
		insert_delay(256);
		lock_status = mmio_read_32(ihostx_config_root +
					   A72_CRM_PLL_STATUS);
		if (lock_status & BIT(A72_CRM_PLL_STATUS__PLL0_LOCK_R))
			return 1;
	}

	ERROR("PLL of Cluster #%u failed to lock\n", cluster_num);
	return 0;
}

/*
 * ihost PLL Variable Frequency Configuration
 *
 * Frequency Limit {VCO,ARM} (GHz):
 *	0 - no limit,
 *	1 - {3.0,1.5},
 *	2 - {4.0,2.0},
 *	3 - {5.0,2.5}
 */
uint32_t bcm_set_ihost_pll_freq(uint32_t cluster_num, int ihost_pll_freq_sel)
{
	NOTICE("cluster: %u, freq_sel:0x%x\n", cluster_num, ihost_pll_freq_sel);

	//bypass PLL
	ARMCOE_crm_pllSetMode(cluster_num, PLL_MODE_BYPASS);
	//assert reset
	ARMCOE_crm_pllAssertReset(cluster_num,
				  PLL_RESET_TYPE_PLL | PLL_RESET_TYPE_POST);
	//set ndiv_int for different freq
	ARMCOE_crm_pllFreqSet(cluster_num, ihost_pll_freq_sel, 0x1);
	//de-assert reset
	ARMCOE_crm_pllDeassertReset(cluster_num, PLL_RESET_TYPE_PLL);
	ARMCOE_crm_pllUpdate(cluster_num, PLL_VCO);
	//waiting for PLL lock
	ARMCOE_crm_pllIsLocked(cluster_num);
	ARMCOE_crm_pllDeassertReset(cluster_num, PLL_RESET_TYPE_POST);
	//disable bypass PLL
	ARMCOE_crm_pllSetMode(cluster_num, PLL_MODE_VCO);

	return 0;
}

uint32_t bcm_get_ihost_pll_freq(uint32_t cluster_num)
{
	unsigned long ihostx_config_root;
	uint32_t ndiv_int;
	uint32_t ihost_pll_freq_sel;

	ihostx_config_root = ARMCOE_crm_getBaseAddress(cluster_num);
	ndiv_int = mmio_read_32(ihostx_config_root+A72_CRM_PLL0_CTRL3) & 0x3FF;

	if (ndiv_int == 0x78) {
		ihost_pll_freq_sel = ARM_FREQ_3G;
	} else if (ndiv_int == 0x3c) {
		ihost_pll_freq_sel = ARM_FREQ_1P5G;
	} else if (ndiv_int == 0x1e) {
		ihost_pll_freq_sel = ARM_FREQ_750M;
	} else {
		/* return unlimit otherwise*/
		ihost_pll_freq_sel = 0;
	}
	return ihost_pll_freq_sel;
}

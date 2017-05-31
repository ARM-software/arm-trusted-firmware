/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <dw_ufs.h>
#include <mmio.h>
#include <stdint.h>
#include <string.h>
#include <ufs.h>

static int dwufs_phy_init(ufs_params_t *params)
{
	uintptr_t base;
	unsigned int fsm0, fsm1;
	unsigned int data;
	int result;

	assert((params != NULL) && (params->reg_base != 0));

	base = params->reg_base;

	/* Unipro VS_MPHY disable */
	ufshc_dme_set(VS_MPHY_DISABLE_OFFSET, 0, VS_MPHY_DISABLE_MPHYDIS);
	ufshc_dme_set(PA_HS_SERIES_OFFSET, 0, 2);
	/* MPHY CBRATESEL */
	ufshc_dme_set(0x8114, 0, 1);
	/* MPHY CBOVRCTRL2 */
	ufshc_dme_set(0x8121, 0, 0x2d);
	/* MPHY CBOVRCTRL3 */
	ufshc_dme_set(0x8122, 0, 0x1);
	ufshc_dme_set(VS_MPHY_CFG_UPDT_OFFSET, 0, 1);

	/* MPHY RXOVRCTRL4 rx0 */
	ufshc_dme_set(0x800d, 4, 0x58);
	/* MPHY RXOVRCTRL4 rx1 */
	ufshc_dme_set(0x800d, 5, 0x58);
	/* MPHY RXOVRCTRL5 rx0 */
	ufshc_dme_set(0x800e, 4, 0xb);
	/* MPHY RXOVRCTRL5 rx1 */
	ufshc_dme_set(0x800e, 5, 0xb);
	/* MPHY RXSQCONTROL rx0 */
	ufshc_dme_set(0x8009, 4, 0x1);
	/* MPHY RXSQCONTROL rx1 */
	ufshc_dme_set(0x8009, 5, 0x1);
	ufshc_dme_set(VS_MPHY_CFG_UPDT_OFFSET, 0, 1);

	ufshc_dme_set(0x8113, 0, 0x1);
	ufshc_dme_set(VS_MPHY_CFG_UPDT_OFFSET, 0, 1);

	ufshc_dme_set(RX_HS_G3_SYNC_LENGTH_CAP_OFFSET, 4, 0x4a);
	ufshc_dme_set(RX_HS_G3_SYNC_LENGTH_CAP_OFFSET, 5, 0x4a);
	ufshc_dme_set(RX_HS_G2_SYNC_LENGTH_CAP_OFFSET, 4, 0x4a);
	ufshc_dme_set(RX_HS_G2_SYNC_LENGTH_CAP_OFFSET, 5, 0x4a);
	ufshc_dme_set(RX_MIN_ACTIVATETIME_CAP_OFFSET, 4, 0x7);
	ufshc_dme_set(RX_MIN_ACTIVATETIME_CAP_OFFSET, 5, 0x7);
	ufshc_dme_set(TX_HIBERN8TIME_CAP_OFFSET, 0, 0x5);
	ufshc_dme_set(TX_HIBERN8TIME_CAP_OFFSET, 1, 0x5);
	ufshc_dme_set(VS_MPHY_CFG_UPDT_OFFSET, 0, 1);

	result = ufshc_dme_get(VS_MPHY_DISABLE_OFFSET, 0, &data);
	assert((result == 0) && (data == VS_MPHY_DISABLE_MPHYDIS));
	/* enable Unipro VS MPHY */
	ufshc_dme_set(VS_MPHY_DISABLE_OFFSET, 0, 0);

	while (1) {
		result = ufshc_dme_get(TX_FSM_STATE_OFFSET, 0, &fsm0);
		assert(result == 0);
		result = ufshc_dme_get(TX_FSM_STATE_OFFSET, 1, &fsm1);
		assert(result == 0);
		if ((fsm0 == TX_FSM_STATE_HIBERN8) &&
		    (fsm1 == TX_FSM_STATE_HIBERN8))
			break;
	}

	mmio_write_32(base + HCLKDIV, 0xE4);
	mmio_clrbits_32(base + AHIT, 0x3FF);

	ufshc_dme_set(PA_LOCAL_TX_LCC_ENABLE_OFFSET, 0, 0);
	ufshc_dme_set(VS_MK2_EXTN_SUPPORT_OFFSET, 0, 0);

	result = ufshc_dme_get(VS_MK2_EXTN_SUPPORT_OFFSET, 0, &data);
	assert((result == 0) && (data == 0));

	ufshc_dme_set(DL_AFC0_CREDIT_THRESHOLD_OFFSET, 0, 0);
	ufshc_dme_set(DL_TC0_OUT_ACK_THRESHOLD_OFFSET, 0, 0);
	ufshc_dme_set(DL_TC0_TX_FC_THRESHOLD_OFFSET, 0, 9);
	(void)result;
	return 0;
}

static int dwufs_phy_set_pwr_mode(ufs_params_t *params)
{
	int result;
	unsigned int data, tx_lanes, rx_lanes;
	uintptr_t base;

	assert((params != NULL) && (params->reg_base != 0));

	base = params->reg_base;

	result = ufshc_dme_get(PA_TACTIVATE_OFFSET, 0, &data);
	assert(result == 0);
	if (data < 7) {
		result = ufshc_dme_set(PA_TACTIVATE_OFFSET, 0, 7);
		assert(result == 0);
	}
	result = ufshc_dme_get(PA_CONNECTED_TX_DATA_LANES_OFFSET, 0, &tx_lanes);
	assert(result == 0);
	result = ufshc_dme_get(PA_CONNECTED_RX_DATA_LANES_OFFSET, 0, &rx_lanes);
	assert(result == 0);

	result = ufshc_dme_set(PA_TX_SKIP_OFFSET, 0, 0);
	assert(result == 0);
	result = ufshc_dme_set(PA_TX_GEAR_OFFSET, 0, 3);
	assert(result == 0);
	result = ufshc_dme_set(PA_RX_GEAR_OFFSET, 0, 3);
	assert(result == 0);
	result = ufshc_dme_set(PA_HS_SERIES_OFFSET, 0, 2);
	assert(result == 0);
	result = ufshc_dme_set(PA_TX_TERMINATION_OFFSET, 0, 1);
	assert(result == 0);
	result = ufshc_dme_set(PA_RX_TERMINATION_OFFSET, 0, 1);
	assert(result == 0);
	result = ufshc_dme_set(PA_SCRAMBLING_OFFSET, 0, 0);
	assert(result == 0);
	result = ufshc_dme_set(PA_ACTIVE_TX_DATA_LANES_OFFSET, 0, tx_lanes);
	assert(result == 0);
	result = ufshc_dme_set(PA_ACTIVE_RX_DATA_LANES_OFFSET, 0, rx_lanes);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA0_OFFSET, 0, 8191);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA1_OFFSET, 0, 65535);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA2_OFFSET, 0, 32767);
	assert(result == 0);
	result = ufshc_dme_set(DME_FC0_PROTECTION_TIMEOUT_OFFSET, 0, 8191);
	assert(result == 0);
	result = ufshc_dme_set(DME_TC0_REPLAY_TIMEOUT_OFFSET, 0, 65535);
	assert(result == 0);
	result = ufshc_dme_set(DME_AFC0_REQ_TIMEOUT_OFFSET, 0, 32767);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA3_OFFSET, 0, 8191);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA4_OFFSET, 0, 65535);
	assert(result == 0);
	result = ufshc_dme_set(PA_PWR_MODE_USER_DATA5_OFFSET, 0, 32767);
	assert(result == 0);
	result = ufshc_dme_set(DME_FC1_PROTECTION_TIMEOUT_OFFSET, 0, 8191);
	assert(result == 0);
	result = ufshc_dme_set(DME_TC1_REPLAY_TIMEOUT_OFFSET, 0, 65535);
	assert(result == 0);
	result = ufshc_dme_set(DME_AFC1_REQ_TIMEOUT_OFFSET, 0, 32767);
	assert(result == 0);

	result = ufshc_dme_set(PA_PWR_MODE_OFFSET, 0, 0x11);
	assert(result == 0);
	do {
		data = mmio_read_32(base + IS);
	} while ((data & UFS_INT_UPMS) == 0);
	mmio_write_32(base + IS, UFS_INT_UPMS);
	data = mmio_read_32(base + HCS);
	if ((data & HCS_UPMCRS_MASK) == HCS_PWR_LOCAL)
		INFO("ufs: change power mode success\n");
	else
		WARN("ufs: HCS.UPMCRS error, HCS:0x%x\n", data);
	(void)result;
	return 0;
}

const ufs_ops_t dw_ufs_ops = {
	.phy_init		= dwufs_phy_init,
	.phy_set_pwr_mode	= dwufs_phy_set_pwr_mode,
};

int dw_ufs_init(dw_ufs_params_t *params)
{
	ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params));
	ufs_params.reg_base = params->reg_base;
	ufs_params.desc_base = params->desc_base;
	ufs_params.desc_size = params->desc_size;
	ufs_params.flags = params->flags;
	ufs_init(&dw_ufs_ops, &ufs_params);
	return 0;
}

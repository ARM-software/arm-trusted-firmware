/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DW_UFS_H__
#define __DW_UFS_H__

#include <sys/types.h>

/* Bus Throtting */
#define BUSTHRTL				0xC0
/* Outstanding OCP Requests */
#define OOCPR					0xC4
/* Fatal Error Interrupt Enable */
#define FEIE					0xC8
/* C-Port Direct Access Configuration register */
#define CDACFG					0xD0
/* C-Port Direct Access Transmit 1 register */
#define CDATX1					0xD4
/* C-Port Direct Access Transmit 2 register */
#define CDATX2					0xD8
/* C-Port Direct Access Receive 1 register */
#define CDARX1					0xDC
/* C-Port Direct Access Receive 2 register */
#define CDARX2					0xE0
/* C-Port Direct Access Status register */
#define CDASTA					0xE4
/* UPIU Loopback Configuration register */
#define LBMCFG					0xF0
/* UPIU Loopback Status */
#define LBMSTA					0xF4
/* Debug register */
#define DBG					0xF8
/* HClk Divider register */
#define HCLKDIV					0xFC

#define TX_HIBERN8TIME_CAP_OFFSET		0x000F
#define TX_FSM_STATE_OFFSET			0x0041
#define TX_FSM_STATE_LINE_RESET			7
#define TX_FSM_STATE_LINE_CFG			6
#define TX_FSM_STATE_HS_BURST			5
#define TX_FSM_STATE_LS_BURST			4
#define TX_FSM_STATE_STALL			3
#define TX_FSM_STATE_SLEEP			2
#define TX_FSM_STATE_HIBERN8			1
#define TX_FSM_STATE_DISABLE			0

#define RX_MIN_ACTIVATETIME_CAP_OFFSET		0x008F
#define RX_HS_G2_SYNC_LENGTH_CAP_OFFSET		0x0094
#define RX_HS_G3_SYNC_LENGTH_CAP_OFFSET		0x0095

#define PA_AVAIL_TX_DATA_LANES_OFFSET		0x1520
#define PA_TX_SKIP_OFFSET			0x155C
#define PA_TX_SKIP_PERIOD_OFFSET		0x155D
#define PA_LOCAL_TX_LCC_ENABLE_OFFSET		0x155E
#define PA_ACTIVE_TX_DATA_LANES_OFFSET		0x1560
#define PA_CONNECTED_TX_DATA_LANES_OFFSET	0x1561
#define PA_TX_TRAILING_CLOCKS_OFFSET		0x1564
#define PA_TX_GEAR_OFFSET			0x1568
#define PA_TX_TERMINATION_OFFSET		0x1569
#define PA_HS_SERIES_OFFSET			0x156A
#define PA_PWR_MODE_OFFSET			0x1571
#define PA_ACTIVE_RX_DATA_LANES_OFFSET		0x1580
#define PA_CONNECTED_RX_DATA_LANES_OFFSET	0x1581
#define PA_RX_PWR_STATUS_OFFSET			0x1582
#define PA_RX_GEAR_OFFSET			0x1583
#define PA_RX_TERMINATION_OFFSET		0x1584
#define PA_SCRAMBLING_OFFSET			0x1585
#define PA_MAX_RX_PWM_GEAR_OFFSET		0x1586
#define PA_MAX_RX_HS_GEAR_OFFSET		0x1587
#define PA_PACP_REQ_TIMEOUT_OFFSET		0x1590
#define PA_PACP_REQ_EOB_TIMEOUT_OFFSET		0x1591
#define PA_REMOTE_VER_INFO_OFFSET		0x15A0
#define PA_LOGICAL_LANE_MAP_OFFSET		0x15A1
#define PA_TACTIVATE_OFFSET			0x15A8
#define PA_PWR_MODE_USER_DATA0_OFFSET		0x15B0
#define PA_PWR_MODE_USER_DATA1_OFFSET		0x15B1
#define PA_PWR_MODE_USER_DATA2_OFFSET		0x15B2
#define PA_PWR_MODE_USER_DATA3_OFFSET		0x15B3
#define PA_PWR_MODE_USER_DATA4_OFFSET		0x15B4
#define PA_PWR_MODE_USER_DATA5_OFFSET		0x15B5

#define DL_TC0_TX_FC_THRESHOLD_OFFSET		0x2040
#define DL_AFC0_CREDIT_THRESHOLD_OFFSET		0x2044
#define DL_TC0_OUT_ACK_THRESHOLD_OFFSET		0x2045

#define DME_FC0_PROTECTION_TIMEOUT_OFFSET	0xD041
#define DME_TC0_REPLAY_TIMEOUT_OFFSET		0xD042
#define DME_AFC0_REQ_TIMEOUT_OFFSET		0xD043
#define DME_FC1_PROTECTION_TIMEOUT_OFFSET	0xD044
#define DME_TC1_REPLAY_TIMEOUT_OFFSET		0xD045
#define DME_AFC1_REQ_TIMEOUT_OFFSET		0xD046

#define VS_MPHY_CFG_UPDT_OFFSET			0xD085
#define VS_MK2_EXTN_SUPPORT_OFFSET		0xD0AB
#define VS_MPHY_DISABLE_OFFSET			0xD0C1
#define VS_MPHY_DISABLE_MPHYDIS			(1 << 0)

typedef struct dw_ufs_params {
	uintptr_t		reg_base;
	uintptr_t		desc_base;
	size_t			desc_size;
	unsigned long		flags;
} dw_ufs_params_t;

int dw_ufs_init(dw_ufs_params_t *params);

#endif /* __DW_UFS_H__ */

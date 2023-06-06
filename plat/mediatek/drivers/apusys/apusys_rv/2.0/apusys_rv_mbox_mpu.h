/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_MBOX_MPU_H
#define APUSYS_RV_MBOX_MPU_H

#define MPU_EN		(0)
#define MPU_DIS		(1)
#define MBOX0_TX_DOMAIN	(0)
#define MBOX0_TX_NS	(1)
#define MBOX4_RX_DOMAIN	(0)
#define MBOX4_RX_NS	(0)
#define MBOX5_TX_DOMAIN	(3)
#define MBOX5_TX_NS	(0)
#define MBOXN_RX_DOMAIN	(5)
#define MBOXN_RX_NS	(1)
#define MBOXN_TX_DOMAIN	(0)
#define MBOXN_TX_NS	(0)

struct mbox_mpu_setting {
	uint32_t no_mpu;
	uint32_t rx_ns;
	uint32_t rx_domain;
	uint32_t tx_ns;
	uint32_t tx_domain;
};

static const struct mbox_mpu_setting mbox_mpu_setting_tab[] = {
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOX0_TX_NS, MBOX0_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_DIS, MBOX4_RX_NS, MBOX4_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOX5_TX_NS, MBOX5_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
	{ MPU_EN,  MBOXN_RX_NS, MBOXN_RX_DOMAIN, MBOXN_TX_NS, MBOXN_TX_DOMAIN },
};

#define APU_MBOX_NUM ARRAY_SIZE(mbox_mpu_setting_tab)

#endif /* APUSYS_RV_MBOX_MPU_H */

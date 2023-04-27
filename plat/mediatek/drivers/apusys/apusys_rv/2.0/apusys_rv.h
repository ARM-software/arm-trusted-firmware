/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_H
#define APUSYS_RV_H

#include <platform_def.h>

/* APU MBOX */
#define MBOX_FUNC_CFG		(0xb0)
#define MBOX_DOMAIN_CFG		(0xe0)
#define MBOX_CTRL_LOCK		BIT(0)
#define MBOX_NO_MPU_SHIFT	(16)
#define MBOX_RX_NS_SHIFT	(16)
#define MBOX_RX_DOMAIN_SHIFT	(17)
#define MBOX_TX_NS_SHIFT	(24)
#define MBOX_TX_DOMAIN_SHIFT	(25)
#define MBOX_SIZE		(0x100)
#define MBOX_NUM		(8)

#define APU_MBOX(i)		(((i) < MBOX_NUM) ? (APU_MBOX0 + MBOX_SIZE * (i)) : \
						  (APU_MBOX1 + MBOX_SIZE * ((i) - MBOX_NUM)))
#define APU_MBOX_FUNC_CFG(i)	(APU_MBOX(i) + MBOX_FUNC_CFG)
#define APU_MBOX_DOMAIN_CFG(i)	(APU_MBOX(i) + MBOX_DOMAIN_CFG)

void apusys_rv_mbox_mpu_init(void);

#endif /* APUSYS_RV_H */

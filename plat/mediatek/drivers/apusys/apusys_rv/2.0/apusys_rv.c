/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>
#include <lib/mmio.h>

/* Vendor header */
#include "apusys_rv.h"
#include "apusys_rv_mbox_mpu.h"

void apusys_rv_mbox_mpu_init(void)
{
	int i;

	for (i = 0; i < APU_MBOX_NUM; i++) {
		mmio_write_32(APU_MBOX_FUNC_CFG(i),
			      (MBOX_CTRL_LOCK |
			       (mbox_mpu_setting_tab[i].no_mpu << MBOX_NO_MPU_SHIFT)));
		mmio_write_32(APU_MBOX_DOMAIN_CFG(i),
			      (MBOX_CTRL_LOCK |
			       (mbox_mpu_setting_tab[i].rx_ns << MBOX_RX_NS_SHIFT) |
			       (mbox_mpu_setting_tab[i].rx_domain << MBOX_RX_DOMAIN_SHIFT) |
			       (mbox_mpu_setting_tab[i].tx_ns << MBOX_TX_NS_SHIFT) |
			       (mbox_mpu_setting_tab[i].tx_domain << MBOX_TX_DOMAIN_SHIFT)));
	}
}

/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_PWR_CTL_H
#define APUSYS_RV_PWR_CTL_H

#include <platform_def.h>

#include "apusys_rv.h"

#define SUPPORT_APU_CLEAR_MBOX_DUMMY	(1)

enum apu_hw_sem_sys_id {
	APU_HW_SEM_SYS_APU   = 0UL,	/* mbox0 */
	APU_HW_SEM_SYS_GZ    = 1UL,	/* mbox1 */
	APU_HW_SEM_SYS_SCP   = 3UL,	/* mbox3 */
	APU_HW_SEM_SYS_APMCU = 11UL,	/* mbox11 */
};

int apusys_rv_pwr_ctrl(enum APU_PWR_OP op);
int rv_iommu_hw_sem_unlock(void);
int rv_iommu_hw_sem_trylock(void);
int apu_hw_sema_ctl(uint32_t sem_addr, uint8_t usr_bit, uint8_t ctl, uint32_t timeout,
		    uint8_t bypass);

#define HW_SEM_TIMEOUT	(300) /* 300 us */
#define HW_SEM_NO_WAIT	(0)   /* no wait */

/* APU MBOX */
#define MBOX_WKUP_CFG		(0x80)
#define MBOX_WKUP_MASK		(0x84)
#define MBOX_FUNC_CFG		(0xb0)
#define MBOX_DOMAIN_CFG		(0xe0)

#define MBOX_CTRL_LOCK		BIT(0)
#define MBOX_NO_MPU_SHIFT	(16)
#define MBOX_RC_SHIFT		(24)

#define MBOX_RX_NS_SHIFT	(16)
#define MBOX_RX_DOMAIN_SHIFT	(17)
#define MBOX_TX_NS_SHIFT	(24)
#define MBOX_TX_DOMAIN_SHIFT	(25)

#define APU_REG_AO_GLUE_CONFG	(APU_AO_CTRL + 0x20)

#define ENABLE_INFRA_WA

enum apu_infra_bit_id {
	APU_INFRA_SYS_APMCU = 1UL,
	APU_INFRA_SYS_GZ    = 2UL,
	APU_INFRA_SYS_SCP   = 3UL,
};

#define APU_MBOX(i)		(APU_MBOX0 + 0x10000 * i)

#define APU_MBOX_FUNC_CFG(i)	(APU_MBOX(i) + MBOX_FUNC_CFG)
#define APU_MBOX_DOMAIN_CFG(i)	(APU_MBOX(i) + MBOX_DOMAIN_CFG)
#define APU_MBOX_WKUP_CFG(i)	(APU_MBOX(i) + MBOX_WKUP_CFG)

enum apu_hw_sem_op {
	HW_SEM_PUT = 0,
	HW_SEM_GET = 1,
};

#define HW_SEM_PUT_BIT_SHIFT	(16)

/* bypass mbox register Dump for secure master */
#define APU_MBOX_DBG_EN		(0x190f2380)

/* apu_mbox register definition for mbox addr change*/
#define APU_MBOX_SEMA0_CTRL	(0x090)
#define APU_MBOX_SEMA0_RST	(0x094)
#define APU_MBOX_SEMA0_STA	(0x098)
#define APU_MBOX_SEMA1_CTRL	(0x0A0)
#define APU_MBOX_SEMA1_RST	(0x0A4)
#define APU_MBOX_SEMA1_STA	(0x0A8)
#define APU_MBOX_DUMMY		(0x040)
#define APU_MBOX_OFFSET(i)	(0x10000 * i)

/* apu infra workaround */
#define APU_INFRA_DISABLE	(APU_INFRA_BASE + 0xC18)
#define APU_INFRA_ENABLE	(APU_INFRA_BASE + 0xC14)
#define APU_INFRA_STATUS	(APU_INFRA_BASE + 0xC10)
#define APU_INFRA_STATUS_MASK	(0x1fffe)
#define APU_INFRA_HW_SEM	(APUSYS_CE_BASE + 0xE00)
#define APU_RPC_STATUS		(0x190f0044)

#define APU_INFRA_BIT_OFF	(16)
#define APU_RPC_STATUS_BIT	BIT(0)

#endif /* APUSYS_RV_PWR_CTL_H */

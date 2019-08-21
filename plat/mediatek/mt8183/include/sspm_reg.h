/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SSPM_REG_H__
#define __SSPM_REG_H__

#include "platform_def.h"

#define SSPM_CFGREG_RSV_RW_REG0        (SSPM_CFGREG_BASE + 0x0100)
#define SSPM_CFGREG_ACAO_INT_SET       (SSPM_CFGREG_BASE + 0x00D8)
#define SSPM_CFGREG_ACAO_INT_CLR       (SSPM_CFGREG_BASE + 0x00DC)
#define SSPM_CFGREG_ACAO_WAKEUP_EN     (SSPM_CFGREG_BASE + 0x0204)

#define STANDBYWFI_EN(n)               (1 << (n +  8))
#define GIC_IRQOUT_EN(n)               (1 << (n +  0))

#define NF_MCDI_MBOX                            19
#define MCDI_MBOX_CLUSTER_0_CAN_POWER_OFF       0
#define MCDI_MBOX_CLUSTER_1_CAN_POWER_OFF       1
#define MCDI_MBOX_BUCK_POWER_OFF_MASK           2
#define MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE     3
#define MCDI_MBOX_CLUSTER_1_ATF_ACTION_DONE     4
#define MCDI_MBOX_BOOTADDR                      5
#define MCDI_MBOX_PAUSE_ACTION                  6
#define MCDI_MBOX_AVAIL_CPU_MASK                7
#define MCDI_MBOX_CPU_CLUSTER_PWR_STAT          8
#define MCDI_MBOX_ACTION_STAT                   9
#define MCDI_MBOX_CLUSTER_0_CNT                 10
#define MCDI_MBOX_CLUSTER_1_CNT                 11
#define MCDI_MBOX_CPU_ISOLATION_MASK            12
#define MCDI_MBOX_PAUSE_ACK                     13
#define MCDI_MBOX_PENDING_ON_EVENT              14
#define MCDI_MBOX_PROF_CMD                      15
#define MCDI_MBOX_DRCC_CALI_DONE                16
#define MCDI_MBOX_HP_CMD                        17
#define MCDI_MBOX_HP_ACK                        18

#endif /* __SSPM_REG_H__ */

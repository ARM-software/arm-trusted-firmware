/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_CPC_H
#define MT_CPU_PM_CPC_H

#include <lib/mmio.h>

#include <mcucfg.h>
#include <platform_def.h>

#define NEED_CPUSYS_PROT_WORKAROUND	(1)

/* system sram registers */
#define CPUIDLE_SRAM_REG(r)	(CPU_IDLE_SRAM_BASE + (r))

/* db dump */
#define CPC_TRACE_SIZE		(0x20)
#define CPC_TRACE_ID_NUM	(10)
#define CPC_TRACE_SRAM(id)	(CPUIDLE_SRAM_REG(0x10) + (id) * CPC_TRACE_SIZE)

/* buckup off count */
#define CPC_CLUSTER_CNT_BACKUP	CPUIDLE_SRAM_REG(0x1f0)
#define CPC_MCUSYS_CNT		CPUIDLE_SRAM_REG(0x1f4)

/* CPC_MCUSYS_CPC_FLOW_CTRL_CFG (0xA814): debug setting */
#define CPC_PWR_ON_SEQ_DIS	BIT(1)
#define CPC_PWR_ON_PRIORITY	BIT(2)
#define CPC_AUTO_OFF_EN		BIT(5)
#define CPC_DORMANT_WAIT_EN	BIT(14)
#define CPC_CTRL_EN		BIT(16)
#define CPC_OFF_PRE_EN		BIT(29)

/* CPC_MCUSYS_LAST_CORE_REQ (0xA818) : last core protection */
#define CPUSYS_PROT_SET		BIT(0)
#define MCUSYS_PROT_SET		BIT(8)
#define CPUSYS_PROT_CLR		BIT(8)
#define MCUSYS_PROT_CLR		BIT(9)

#define CPC_PROT_RESP_MASK	(0x3)
#define CPUSYS_RESP_OFS		(16)
#define MCUSYS_RESP_OFS		(30)

#define RETRY_CNT_MAX		(1000)

#define PROT_RETRY		(0)
#define PROT_SUCCESS		(1)
#define PROT_GIVEUP		(2)

/* CPC_MCUSYS_CPC_DBG_SETTING (0xAB00): debug setting */
#define CPC_PROF_EN		BIT(0)
#define CPC_DBG_EN		BIT(1)
#define CPC_FREEZE		BIT(2)
#define CPC_CALC_EN		BIT(3)

enum mcusys_cpc_lastcore_prot_status {
	CPC_SUCCESS = 0,
	CPC_ERR_FAIL,
	CPC_ERR_TIMEOUT,
	NF_CPC_ERR,
};

enum mcusys_cpc_smc_events {
	CPC_SMC_EVENT_DUMP_TRACE_DATA,
	CPC_SMC_EVENT_GIC_DPG_SET,
	CPC_SMC_EVENT_CPC_CONFIG,
	CPC_SMC_EVENT_READ_CONFIG,
	NF_CPC_SMC_EVENT,
};

enum mcusys_cpc_smc_config {
	CPC_SMC_CONFIG_PROF,
	CPC_SMC_CONFIG_AUTO_OFF,
	CPC_SMC_CONFIG_AUTO_OFF_THRES,
	CPC_SMC_CONFIG_CNT_CLR,
	CPC_SMC_CONFIG_TIME_SYNC,
	NF_CPC_SMC_CONFIG,
};

#define US_TO_TICKS(us)		((us) * 13)
#define TICKS_TO_US(tick)	((tick) / 13)

int mtk_cpu_pm_cluster_prot_aquire(void);
void mtk_cpu_pm_cluster_prot_release(void);

void mtk_cpc_mcusys_off_reflect(void);
int mtk_cpc_mcusys_off_prepare(void);

void mtk_cpc_core_on_hint_set(int cpu);
void mtk_cpc_core_on_hint_clr(int cpu);
void mtk_cpc_time_sync(void);

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2);
uint64_t mtk_cpc_trace_dump(uint64_t act, uint64_t arg1, uint64_t arg2);
void mtk_cpc_init(void);

#endif /* MT_CPU_PM_CPC_H */

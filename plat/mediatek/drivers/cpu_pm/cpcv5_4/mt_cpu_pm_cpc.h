/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_CPC_H
#define MT_CPU_PM_CPC_H

#include <lib/mmio.h>
#include <platform_def.h>
#include <mcucfg.h>
#include <mcupm_cfg.h>

#define NEED_CPUSYS_PROT_WORKAROUND	1

/* system sram registers */
#define CPUIDLE_SRAM_REG(r)	(0x11B000 + (r))

/* db dump */
#define CPC_TRACE_SIZE		0x20
#define CPC_TRACE_ID_NUM	13
#define CPC_TRACE_SRAM(id)	(CPUIDLE_SRAM_REG(0x10) + (id) * CPC_TRACE_SIZE)

/* backup off count */
#define SYSRAM_RECENT_CPU_CNT(i)	CPUIDLE_SRAM_REG(4 * (i) + 0x1B0)
#define SYSRAM_RECENT_CLUSTER_CNT	CPUIDLE_SRAM_REG(0x1D0)
#define SYSRAM_RECENT_MCUSYS_CNT	CPUIDLE_SRAM_REG(0x1D4)
#define SYSRAM_RECENT_CNT_TS_L		CPUIDLE_SRAM_REG(0x1D8)
#define SYSRAM_RECENT_CNT_TS_H		CPUIDLE_SRAM_REG(0x1DC)
#define SYSRAM_CPUSYS_CNT		CPUIDLE_SRAM_REG(0x1E8)
#define SYSRAM_MCUSYS_CNT		CPUIDLE_SRAM_REG(0x1EC)
#define SYSRAM_CLUSTER_CNT_BACKUP	CPUIDLE_SRAM_REG(0x1F0)
#define MCUPM_TCM_MCUSYS_COUNTER \
	(CPU_EB_TCM_CNT_BASE + CPU_EB_MCUSYS_CNT_OFST)

/* CPC_MCUSYS_CPC_FLOW_CTRL_CFG(0x114): debug setting */
#define CPC_PWR_ON_SEQ_DIS	BIT(1)
#define CPC_PWR_ON_PRIORITY	BIT(2)
#define CPC_AUTO_OFF_EN		BIT(5)
#define CPC_DORMANT_WAIT_EN	BIT(14)
#define CPC_CTRL_EN		BIT(16)
#define CPC_OFF_PRE_EN		BIT(29)

/* CPC_MCUSYS_LAST_CORE_REQ(0x118) : last core protection */
#define CPUSYS_PROT_SET		BIT(0)
#define MCUSYS_PROT_SET		BIT(8)
/* CPC_PWR_ON_MASK(0x128) : last core protection */
#define CPUSYS_PROT_CLR		BIT(8)
#define MCUSYS_PROT_CLR		BIT(9)

#define CPC_PROT_RESP_MASK	(0x3)
/* CPC_CPUSYS_LAST_CORE_RESP(0x11C) : last core protection */
#define CPUSYS_RESP_OFS		(16)
/* CPC_MCUSYS_LAST_CORE_RESP(0x124) : last core protection */
#define MCUSYS_RESP_OFS		(30)

#define RETRY_CNT_MAX		(1000)

#define PROT_RETRY		(0)
#define PROT_SUCCESS		(1)
#define PROT_GIVEUP		(2)

/* CPC_MCUSYS_CPC_DBG_SETTING(0x200): debug setting */
#define CPC_PROF_EN		BIT(0)
#define CPC_DBG_EN		BIT(1)
#define CPC_FREEZE		BIT(2)
#define CPC_CALC_EN		BIT(3)

enum mcusys_cpc_lastcore_prot_status {
	CPC_SUCCESS = 0,
	CPC_ERR_FAIL,
	CPC_ERR_TIMEOUT,
	NF_CPC_ERR
};

enum mcusys_cpc_smc_events {
	CPC_SMC_EVENT_DUMP_TRACE_DATA,
	CPC_SMC_EVENT_GIC_DPG_SET,
	CPC_SMC_EVENT_CPC_CONFIG,
	CPC_SMC_EVENT_READ_CONFIG,
	CPC_SMC_EVENT_PROF_LATENCY,
	NF_CPC_SMC_EVENT
};

enum mcusys_cpc_smc_config {
	CPC_SMC_CONFIG_PROF,
	CPC_SMC_CONFIG_CNT_CLR,
	CPC_SMC_CONFIG_TIME_SYNC,

	NF_CPC_SMC_CONFIG,
};

enum dev_type {
	DEV_TYPE_CPU_0 = 0,
	DEV_TYPE_CPUSYS = PLATFORM_CORE_COUNT,
	DEV_TYPE_MCUSYS,
	DEV_TYPE_NUM
};

enum {
	CPC_PROF_ENABLE,
	CPC_PROF_ENABLED,
	CPC_PROF_DEV_NUM,
	CPC_PROF_DEV_NAME,
	CPC_PROF_OFF_CNT,
	CPC_PROF_OFF_AVG,
	CPC_PROF_OFF_MAX,
	CPC_PROF_OFF_MIN,
	CPC_PROF_ON_CNT,
	CPC_PROF_ON_AVG,
	CPC_PROF_ON_MAX,
	CPC_PROF_ON_MIN,

	CPC_PROF_NUM
};

#define MTK_CPC_AUTO_DORMANT_THR_US	(8000)
#define US_TO_TICKS(us)			((us) * 26)
#define TICKS_TO_US(tick)		((tick) / 26)

int mtk_cpu_pm_cluster_prot_aquire(int cluster);
void mtk_cpu_pm_cluster_prot_release(int cluster);

void mtk_cpc_mcusys_off_reflect(void);
int mtk_cpc_mcusys_off_prepare(void);

void mtk_cpc_core_on_hint_set(int cpu);
void mtk_cpc_core_on_hint_clr(int cpu);
void mtk_cpc_time_sync(void);

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2);
uint64_t mtk_cpc_trace_dump(uint64_t act, uint64_t arg1, uint64_t arg2);
void mtk_cpu_pm_counter_enable(bool enable);
bool mtk_cpu_pm_counter_enabled(void);
void mtk_cpu_pm_counter_update(unsigned int cpu);
void mtk_cpc_prof_enable(bool enable);
bool mtk_cpc_prof_is_enabled(void);
void mtk_cpu_pm_save_cpc_latency(enum dev_type dev_type);
void mtk_cpc_init(void);

#endif /* MT_CPU_PM_CPC_H */

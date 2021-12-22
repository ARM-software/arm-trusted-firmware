/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_CPC_H
#define MT_CPU_PM_CPC_H

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <mcucfg.h>
#include <platform_def.h>

#define NEED_CPUSYS_PROT_WORKAROUND	1

/* system sram registers */
#define CPUIDLE_SRAM_REG(r)	(0x11B000 + (r))

/* db dump */
#define CPC_TRACE_SIZE		U(0x20)
#define CPC_TRACE_ID_NUM	U(10)
#define CPC_TRACE_SRAM(id)	(CPUIDLE_SRAM_REG(0x10) + (id) * CPC_TRACE_SIZE)

/* buckup off count */
#define CPC_CLUSTER_CNT_BACKUP	CPUIDLE_SRAM_REG(0x1F0)
#define CPC_MCUSYS_CNT		CPUIDLE_SRAM_REG(0x1F4)

/* CPC_MCUSYS_CPC_FLOW_CTRL_CFG(0xA814): debug setting */
#define CPC_PWR_ON_SEQ_DIS	BIT(1)
#define CPC_PWR_ON_PRIORITY	BIT(2)
#define CPC_AUTO_OFF_EN		BIT(5)
#define CPC_DORMANT_WAIT_EN	BIT(14)
#define CPC_CTRL_EN		BIT(16)
#define CPC_OFF_PRE_EN		BIT(29)

/* CPC_MCUSYS_LAST_CORE_REQ(0xA818) : last core protection */
#define CPUSYS_PROT_SET		BIT(0)
#define MCUSYS_PROT_SET		BIT(8)
#define CPUSYS_PROT_CLR		BIT(8)
#define MCUSYS_PROT_CLR		BIT(9)

#define CPC_PROT_RESP_MASK	U(0x3)
#define CPUSYS_RESP_OFS		U(16)
#define MCUSYS_RESP_OFS		U(30)

#define cpusys_resp(r)		(((r) >> CPUSYS_RESP_OFS) & CPC_PROT_RESP_MASK)
#define mcusys_resp(r)		(((r) >> MCUSYS_RESP_OFS) & CPC_PROT_RESP_MASK)

#define RETRY_CNT_MAX		U(1000)

#define PROT_RETRY		U(0)
#define PROT_SUCCESS		U(1)
#define PROT_GIVEUP		U(2)

/* CPC_MCUSYS_CPC_DBG_SETTING(0xAB00): debug setting */
#define CPC_PROF_EN		BIT(0)
#define CPC_DBG_EN		BIT(1)
#define CPC_FREEZE		BIT(2)
#define CPC_CALC_EN		BIT(3)

enum {
	CPC_SUCCESS = 0U,
	CPC_ERR_FAIL = 1U,
	CPC_ERR_TIMEOUT = 2U,
	NF_CPC_ERR = 3U,
};

enum {
	CPC_SMC_EVENT_DUMP_TRACE_DATA = 0U,
	CPC_SMC_EVENT_GIC_DPG_SET = 1U,
	CPC_SMC_EVENT_CPC_CONFIG = 2U,
	CPC_SMC_EVENT_READ_CONFIG = 3U,
	NF_CPC_SMC_EVENT = 4U,
};

enum {
	CPC_SMC_CONFIG_PROF = 0U,
	CPC_SMC_CONFIG_AUTO_OFF = 1U,
	CPC_SMC_CONFIG_AUTO_OFF_THRES = 2U,
	CPC_SMC_CONFIG_CNT_CLR = 3U,
	CPC_SMC_CONFIG_TIME_SYNC = 4U,
	NF_CPC_SMC_CONFIG = 5U,
};

#define us_to_ticks(us)		((us) * 13)
#define ticks_to_us(tick)	((tick) / 13)

int mtk_cpu_pm_cluster_prot_aquire(unsigned int cluster);
void mtk_cpu_pm_cluster_prot_release(unsigned int cluster);

void mtk_cpc_mcusys_off_reflect(void);
int mtk_cpc_mcusys_off_prepare(void);

void mtk_cpc_core_on_hint_set(unsigned int cpu);
void mtk_cpc_core_on_hint_clr(unsigned int cpu);
void mtk_cpc_time_sync(void);

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2);
void mtk_cpc_init(void);

#endif /* MT_CPU_PM_CPC_H */

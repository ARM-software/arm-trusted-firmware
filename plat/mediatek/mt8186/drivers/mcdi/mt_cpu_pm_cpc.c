/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <drivers/delay_timer.h>

#include <mt_cpu_pm_cpc.h>
#include <mt_timer.h>

struct mtk_cpc_dev {
	int auto_off;
	unsigned int auto_thres_tick;
};

static struct mtk_cpc_dev cpc;

static int mtk_cpc_last_core_prot(uint32_t prot_req,
				uint32_t resp_reg, uint32_t resp_ofs)
{
	uint32_t sta, retry;

	retry = 0U;

	while (retry++ < RETRY_CNT_MAX) {

		mmio_write_32(CPC_MCUSYS_LAST_CORE_REQ, prot_req);

		udelay(1U);

		sta = (mmio_read_32(resp_reg) >> resp_ofs) & CPC_PROT_RESP_MASK;

		if (sta == PROT_SUCCESS) {
			return CPC_SUCCESS;
		} else if (sta == PROT_GIVEUP) {
			return CPC_ERR_FAIL;
		}
	}

	return CPC_ERR_TIMEOUT;
}

int mtk_cpu_pm_mcusys_prot_aquire(void)
{
	return mtk_cpc_last_core_prot(
			MCUSYS_PROT_SET,
			CPC_MCUSYS_LAST_CORE_RESP,
			MCUSYS_RESP_OFS);
}

void mtk_cpu_pm_mcusys_prot_release(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, MCUSYS_PROT_CLR);
}

int mtk_cpu_pm_cluster_prot_aquire(unsigned int cluster)
{
	return mtk_cpc_last_core_prot(
			CPUSYS_PROT_SET,
			CPC_MCUSYS_MP_LAST_CORE_RESP,
			CPUSYS_RESP_OFS);
}

void mtk_cpu_pm_cluster_prot_release(unsigned int cluster)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, CPUSYS_PROT_CLR);
}

static void mtk_cpc_cluster_cnt_backup(void)
{
	uint32_t backup_cnt;
	uint32_t curr_cnt;
	uint32_t cnt_mask = GENMASK(14, 0);
	uint32_t clr_mask = GENMASK(1, 0);

	/* Single Cluster */
	backup_cnt = mmio_read_32(CPC_CLUSTER_CNT_BACKUP);
	curr_cnt = mmio_read_32(CPC_MCUSYS_CLUSTER_COUNTER);

	/* Get off count if dormant count is 0 */
	if ((curr_cnt & cnt_mask) == 0U) {
		curr_cnt = (curr_cnt >> 16) & cnt_mask;
	} else {
		curr_cnt = curr_cnt & cnt_mask;
	}

	mmio_write_32(CPC_CLUSTER_CNT_BACKUP, backup_cnt + curr_cnt);
	mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, clr_mask);
}

static inline void mtk_cpc_mcusys_off_en(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_CTRL, 1U);
}

static inline void mtk_cpc_mcusys_off_dis(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_CTRL, 0U);
}

void mtk_cpc_mcusys_off_reflect(void)
{
	mtk_cpc_mcusys_off_dis();
	mtk_cpu_pm_mcusys_prot_release();
}

int mtk_cpc_mcusys_off_prepare(void)
{
	if (mtk_cpu_pm_mcusys_prot_aquire() != CPC_SUCCESS) {
		return CPC_ERR_FAIL;
	}

	mtk_cpc_cluster_cnt_backup();
	mtk_cpc_mcusys_off_en();

	return CPC_SUCCESS;
}

void mtk_cpc_core_on_hint_set(unsigned int cpu)
{
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_SET, BIT(cpu));
}

void mtk_cpc_core_on_hint_clr(unsigned int cpu)
{
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));
}

static void mtk_cpc_dump_timestamp(void)
{
	uint32_t id;

	for (id = 0U; id < CPC_TRACE_ID_NUM; id++) {
		mmio_write_32(CPC_MCUSYS_TRACE_SEL, id);

		memcpy((void *)(uintptr_t)CPC_TRACE_SRAM(id),
				(const void *)(uintptr_t)CPC_MCUSYS_TRACE_DATA,
				CPC_TRACE_SIZE);
	}
}

void mtk_cpc_time_sync(void)
{
	uint64_t kt;
	uint32_t systime_l, systime_h;

	kt = sched_clock();
	systime_l = mmio_read_32(CNTSYS_L_REG);
	systime_h = mmio_read_32(CNTSYS_H_REG);

	/* sync kernel timer to cpc */
	mmio_write_32(CPC_MCUSYS_CPC_KERNEL_TIME_L_BASE, (uint32_t)kt);
	mmio_write_32(CPC_MCUSYS_CPC_KERNEL_TIME_H_BASE, (uint32_t)(kt >> 32));
	/* sync system timer to cpc */
	mmio_write_32(CPC_MCUSYS_CPC_SYSTEM_TIME_L_BASE, systime_l);
	mmio_write_32(CPC_MCUSYS_CPC_SYSTEM_TIME_H_BASE, systime_h);
}

static void mtk_cpc_config(uint32_t cfg, uint32_t data)
{
	uint32_t val;
	uint32_t reg = 0U;

	switch (cfg) {
	case CPC_SMC_CONFIG_PROF:
		reg = CPC_MCUSYS_CPC_DBG_SETTING;
		val = mmio_read_32(reg);
		val = (data != 0U) ? (val | CPC_PROF_EN) : (val & ~CPC_PROF_EN);
		break;
	case CPC_SMC_CONFIG_AUTO_OFF:
		reg = CPC_MCUSYS_CPC_FLOW_CTRL_CFG;
		val = mmio_read_32(reg);
		if (data != 0U) {
			val |= CPC_AUTO_OFF_EN;
			cpc.auto_off = 1;
		} else {
			val &= ~CPC_AUTO_OFF_EN;
			cpc.auto_off = 0;
		}
		break;
	case CPC_SMC_CONFIG_AUTO_OFF_THRES:
		reg = CPC_MCUSYS_CPC_OFF_THRES;
		cpc.auto_thres_tick = us_to_ticks(data);
		val = cpc.auto_thres_tick;
		break;
	case CPC_SMC_CONFIG_CNT_CLR:
		reg = CPC_MCUSYS_CLUSTER_COUNTER_CLR;
		val = GENMASK(1, 0);	/* clr_mask */
		break;
	case CPC_SMC_CONFIG_TIME_SYNC:
		mtk_cpc_time_sync();
		break;
	default:
		break;
	}

	if (reg != 0U) {
		mmio_write_32(reg, val);
	}
}

static uint32_t mtk_cpc_read_config(uint32_t cfg)
{
	uint32_t res = 0U;

	switch (cfg) {
	case CPC_SMC_CONFIG_PROF:
		res = (mmio_read_32(CPC_MCUSYS_CPC_DBG_SETTING) & CPC_PROF_EN) ?
			1U : 0U;
		break;
	case CPC_SMC_CONFIG_AUTO_OFF:
		res = cpc.auto_off;
		break;
	case CPC_SMC_CONFIG_AUTO_OFF_THRES:
		res = ticks_to_us(cpc.auto_thres_tick);
		break;
	case CPC_SMC_CONFIG_CNT_CLR:
		break;
	default:
		break;
	}

	return res;
}

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2)
{
	uint64_t res = 0ULL;

	switch (act) {
	case CPC_SMC_EVENT_DUMP_TRACE_DATA:
		mtk_cpc_dump_timestamp();
		break;
	case CPC_SMC_EVENT_GIC_DPG_SET:
		/* isolated_status = x2; */
		break;
	case CPC_SMC_EVENT_CPC_CONFIG:
		mtk_cpc_config((uint32_t)arg1, (uint32_t)arg2);
		break;
	case CPC_SMC_EVENT_READ_CONFIG:
		res = mtk_cpc_read_config((uint32_t)arg1);
		break;
	default:
		break;
	}

	return res;
}

void mtk_cpc_init(void)
{
	mmio_write_32(CPC_MCUSYS_CPC_DBG_SETTING,
			mmio_read_32(CPC_MCUSYS_CPC_DBG_SETTING)
			| CPC_DBG_EN
			| CPC_CALC_EN);

	cpc.auto_off = 1;
	cpc.auto_thres_tick = us_to_ticks(8000);

	mmio_write_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
			mmio_read_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG)
			| CPC_OFF_PRE_EN
			| (cpc.auto_off ? CPC_AUTO_OFF_EN : 0U));

	mmio_write_32(CPC_MCUSYS_CPC_OFF_THRES, cpc.auto_thres_tick);
}

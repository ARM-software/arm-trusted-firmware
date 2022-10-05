/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <drivers/delay_timer.h>

#include "mt_cpu_pm.h"
#include "mt_cpu_pm_cpc.h"
#include "mt_smp.h"
#include <mt_timer.h>

struct mtk_cpc_dev {
	int auto_off;
	unsigned int auto_thres_tick;
};

static struct mtk_cpc_dev cpc;

static int mtk_cpc_last_core_prot(int prot_req, int resp_reg, int resp_ofs)
{
	unsigned int staus;
	unsigned int retry = 0;

	while (retry < RETRY_CNT_MAX) {
		retry++;

		mmio_write_32(CPC_MCUSYS_LAST_CORE_REQ, prot_req);

		udelay(1);

		staus = (mmio_read_32(resp_reg) >> resp_ofs) & CPC_PROT_RESP_MASK;

		if (staus == PROT_SUCCESS) {
			return CPC_SUCCESS;
		} else if (staus == PROT_GIVEUP) {
			return CPC_ERR_FAIL;
		}
	}

	return CPC_ERR_TIMEOUT;
}

static int mtk_cpu_pm_mcusys_prot_aquire(void)
{
	return mtk_cpc_last_core_prot(MCUSYS_PROT_SET, CPC_MCUSYS_LAST_CORE_RESP, MCUSYS_RESP_OFS);
}

static void mtk_cpu_pm_mcusys_prot_release(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, MCUSYS_PROT_CLR);
}

int mtk_cpu_pm_cluster_prot_aquire(void)
{
	return mtk_cpc_last_core_prot(CPUSYS_PROT_SET, CPC_MCUSYS_MP_LAST_CORE_RESP,
				      CPUSYS_RESP_OFS);
}

void mtk_cpu_pm_cluster_prot_release(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, CPUSYS_PROT_CLR);
}

static void mtk_cpc_cluster_cnt_backup(void)
{
	/* single cluster */
	uint32_t backup_cnt = mmio_read_32(CPC_CLUSTER_CNT_BACKUP);
	uint32_t curr_cnt = mmio_read_32(CPC_MCUSYS_CLUSTER_COUNTER);

	if ((curr_cnt & 0x7fff) == 0) {
		curr_cnt = (curr_cnt >> 16) & 0x7fff;
	} else {
		curr_cnt = curr_cnt & 0x7fff;
	}

	mmio_write_32(CPC_CLUSTER_CNT_BACKUP, backup_cnt + curr_cnt);
	mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, 0x3);
}

static inline void mtk_cpc_mcusys_off_enable(bool enable)
{
	mmio_write_32(CPC_MCUSYS_PWR_CTRL, enable ? 1 : 0);
}

void mtk_cpc_mcusys_off_reflect(void)
{
	mtk_cpc_mcusys_off_enable(false);
	mtk_cpu_pm_mcusys_prot_release();
}

int mtk_cpc_mcusys_off_prepare(void)
{
	if (mtk_cpu_pm_mcusys_prot_aquire() != CPC_SUCCESS) {
		return CPC_ERR_FAIL;
	}

	mtk_cpc_cluster_cnt_backup();
	mtk_cpc_mcusys_off_enable(true);

	return CPC_SUCCESS;
}

void mtk_cpc_core_on_hint_set(int cpu)
{
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_SET, BIT(cpu));
}

void mtk_cpc_core_on_hint_clr(int cpu)
{
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));
}

static void mtk_cpc_dump_timestamp(void)
{
	unsigned int id;

	for (id = 0; id < CPC_TRACE_ID_NUM; id++) {
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

static void mtk_cpc_config(unsigned int cfg, unsigned int data)
{
	switch (cfg) {
	case CPC_SMC_CONFIG_PROF:
		if (data) {
			mmio_setbits_32(CPC_MCUSYS_CPC_DBG_SETTING, CPC_PROF_EN);
		} else {
			mmio_clrbits_32(CPC_MCUSYS_CPC_DBG_SETTING, CPC_PROF_EN);
		}
		break;
	case CPC_SMC_CONFIG_AUTO_OFF:
		if (data) {
			mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_AUTO_OFF_EN);
			cpc.auto_off = 1;
		} else {
			mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_AUTO_OFF_EN);
			cpc.auto_off = 0;
		}
		break;
	case CPC_SMC_CONFIG_AUTO_OFF_THRES:
		cpc.auto_thres_tick = US_TO_TICKS(data);
		mmio_write_32(CPC_MCUSYS_CPC_OFF_THRES, cpc.auto_thres_tick);
		break;
	case CPC_SMC_CONFIG_CNT_CLR:
		mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, 0x3);
		break;
	case CPC_SMC_CONFIG_TIME_SYNC:
		mtk_cpc_time_sync();
		break;
	default:
		break;
	}
}

static unsigned int mtk_cpc_read_config(unsigned int cfg)
{
	unsigned int res = 0;

	switch (cfg) {
	case CPC_SMC_CONFIG_PROF:
		res = (mmio_read_32(CPC_MCUSYS_CPC_DBG_SETTING) & CPC_PROF_EN) ? 1 : 0;
		break;
	case CPC_SMC_CONFIG_AUTO_OFF:
		res = cpc.auto_off;
		break;
	case CPC_SMC_CONFIG_AUTO_OFF_THRES:
		res = TICKS_TO_US(cpc.auto_thres_tick);
		break;
	case CPC_SMC_CONFIG_CNT_CLR:
	default:
		break;
	}

	return res;
}

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2)
{
	uint64_t res = 0;

	switch (act) {
	case CPC_SMC_EVENT_CPC_CONFIG:
		mtk_cpc_config((unsigned int)arg1, (unsigned int)arg2);
		break;
	case CPC_SMC_EVENT_READ_CONFIG:
		res = mtk_cpc_read_config((unsigned int)arg1);
		break;
	case CPC_SMC_EVENT_GIC_DPG_SET:
		/* isolated_status = x2; */
	default:
		break;
	}

	return res;
}

uint64_t mtk_cpc_trace_dump(uint64_t act, uint64_t arg1, uint64_t arg2)
{
	switch (act) {
	case CPC_SMC_EVENT_DUMP_TRACE_DATA:
		mtk_cpc_dump_timestamp();
		break;
	default:
		break;
	}

	return 0;
}

void mtk_cpc_init(void)
{
#if CONFIG_MTK_SMP_EN
	mt_smp_init();
#endif
	mmio_setbits_32(CPC_MCUSYS_CPC_DBG_SETTING, (CPC_DBG_EN | CPC_CALC_EN));

	cpc.auto_off = 1;
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, (CPC_OFF_PRE_EN |
						      ((cpc.auto_off > 0) ? CPC_AUTO_OFF_EN : 0)));

	mtk_cpc_config(CPC_SMC_CONFIG_AUTO_OFF_THRES, 8000);

	/* enable CPC */
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, SSPM_CORE_PWR_ON_EN);
}

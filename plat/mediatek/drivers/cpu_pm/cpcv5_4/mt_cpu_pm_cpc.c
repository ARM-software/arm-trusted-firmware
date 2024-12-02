/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <drivers/delay_timer.h>
#include <lib/spinlock.h>

#include <lib/pm/mtk_pm.h>
#include <mcucfg.h>
#include "mt_cpu_pm.h"
#include "mt_cpu_pm_cpc.h"
#include "mt_smp.h"
#include <mt_timer.h>

#define CHECK_GIC_SGI_PENDING		(0)
#define MTK_SYS_TIMER_SYNC_SUPPORT	(1)
#define MCUSYS_CLUSTER_DORMANT_MASK	0xFFFF

struct mtk_cpc_lat_data {
	unsigned int on_sum;
	unsigned int on_min;
	unsigned int on_max;
	unsigned int off_sum;
	unsigned int off_min;
	unsigned int off_max;
	unsigned int on_cnt;
	unsigned int off_cnt;
};

struct mtk_cpc_device {
	union {
		struct mtk_cpc_lat_data p[DEV_TYPE_NUM];
		struct {
			struct mtk_cpc_lat_data cpu[PLATFORM_CORE_COUNT];
			struct mtk_cpc_lat_data cluster;
			struct mtk_cpc_lat_data mcusys;
		};
	};
};

static struct mtk_cpc_device cpc_dev;

static bool cpu_pm_counter_enabled;
static bool cpu_cpc_prof_enabled;

static void mtk_cpc_auto_dormant_en(unsigned int en)
{
	struct mtk_plat_dev_config *cfg = NULL;

	if (en)
		mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_AUTO_OFF_EN);
	else
		mmio_clrbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_AUTO_OFF_EN);

	mt_plat_cpu_pm_dev_config(&cfg);

	if (cfg) {
		cfg->auto_off = !!en;
		mt_plat_cpu_pm_dev_update(cfg);
	}
}

static void mtk_cpc_auto_dormant_tick(unsigned int us)
{
	struct mtk_plat_dev_config *cfg = NULL;

	mmio_write_32(CPC_MCUSYS_CPC_OFF_THRES, US_TO_TICKS(us));

	mt_plat_cpu_pm_dev_config(&cfg);

	if (cfg) {
		cfg->auto_thres_us = us;
		mt_plat_cpu_pm_dev_update(cfg);
	}
}

static void mtk_cpu_pm_mcusys_prot_release(void)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, MCUSYS_PROT_CLR);
}

static int mtk_cpc_last_core_prot(int prot_req, int resp_reg, int resp_ofs)
{
	unsigned int sta, retry;

	retry = 0;

	while (retry < RETRY_CNT_MAX) {
		mmio_write_32(CPC_MCUSYS_LAST_CORE_REQ, prot_req);
		udelay(1);
		sta = (mmio_read_32(resp_reg) >> resp_ofs) & CPC_PROT_RESP_MASK;

		if (sta == PROT_GIVEUP)
			return CPC_ERR_FAIL;

		if (sta == PROT_SUCCESS) {
			if (mmio_read_32(CPC_WAKEUP_REQ) ==
			    CPC_WAKEUP_STAT_NONE)
				return CPC_SUCCESS;

			mtk_cpu_pm_mcusys_prot_release();
		}

		retry++;
	}

	return CPC_ERR_TIMEOUT;
}

static int mtk_cpu_pm_mcusys_prot_aquire(void)
{
	return mtk_cpc_last_core_prot(MCUSYS_PROT_SET,
				      CPC_MCUSYS_LAST_CORE_RESP,
				      MCUSYS_RESP_OFS);
}

int mtk_cpu_pm_cluster_prot_aquire(int cluster)
{
	return mtk_cpc_last_core_prot(CPUSYS_PROT_SET,
				      CPC_MCUSYS_MP_LAST_CORE_RESP,
				      CPUSYS_RESP_OFS);
}

void mtk_cpu_pm_cluster_prot_release(int cluster)
{
	mmio_write_32(CPC_MCUSYS_PWR_ON_MASK, CPUSYS_PROT_CLR);
}

static bool is_cpu_pm_counter_enabled(void)
{
	return cpu_pm_counter_enabled;
}

static void mtk_cpc_cluster_cnt_backup(void)
{
	int backup_cnt;
	int curr_cnt;

	if (is_cpu_pm_counter_enabled() == false)
		return;

	/* Single Cluster */
	backup_cnt = mmio_read_32(SYSRAM_CLUSTER_CNT_BACKUP);
	curr_cnt = mmio_read_32(CPC_MCUSYS_CLUSTER_COUNTER);

	/* Get off count if dormant count is 0 */
	if ((curr_cnt & MCUSYS_CLUSTER_DORMANT_MASK) == 0)
		curr_cnt = (curr_cnt >> 16) & MCUSYS_CLUSTER_DORMANT_MASK;
	else
		curr_cnt = curr_cnt & MCUSYS_CLUSTER_DORMANT_MASK;

	mmio_write_32(SYSRAM_CLUSTER_CNT_BACKUP, backup_cnt + curr_cnt);
	mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, 0x3);
}

static inline void mtk_cpc_mcusys_off_en(void)
{
	mmio_setbits_32(CPC_MCUSYS_PWR_CTRL, CPC_MCUSYS_OFF_EN);
}

static inline void mtk_cpc_mcusys_off_dis(void)
{
	mmio_clrbits_32(CPC_MCUSYS_PWR_CTRL, CPC_MCUSYS_OFF_EN);
}

void mtk_cpc_mcusys_off_reflect(void)
{
	mtk_cpc_mcusys_off_dis();
	mtk_cpu_pm_mcusys_prot_release();
}

int mtk_cpc_mcusys_off_prepare(void)
{
	if (mtk_cpu_pm_mcusys_prot_aquire() != CPC_SUCCESS)
		return CPC_ERR_FAIL;

#if CHECK_GIC_SGI_PENDING
	if (!!(gicr_get_sgi_pending())) {
		mtk_cpu_pm_mcusys_prot_release();
		return CPC_ERR_FAIL;
	}
#endif /* CHECK_GIC_SGI_PENDING */
	mtk_cpc_cluster_cnt_backup();
	mtk_cpc_mcusys_off_en();

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
#if MTK_SYS_TIMER_SYNC_SUPPORT
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
#endif /* MTK_SYS_TIMER_SYNC_SUPPORT */
}

static void mtk_cpc_time_freeze(bool is_freeze)
{
#if MTK_SYS_TIMER_SYNC_SUPPORT
	mtk_cpc_time_sync();
	if (is_freeze)
		mmio_setbits_32(CPC_MCUSYS_CPC_DBG_SETTING, CPC_FREEZE);
	else
		mmio_clrbits_32(CPC_MCUSYS_CPC_DBG_SETTING, CPC_FREEZE);
#endif /* MTK_SYS_TIMER_SYNC_SUPPORT */
}

static void *mtk_cpc_el3_timesync_handler(const void *arg)
{
	if (arg) {
		unsigned int *is_time_sync = (unsigned int *)arg;

		if (*is_time_sync)
			mtk_cpc_time_freeze(false);
		else
			mtk_cpc_time_freeze(true);
	}
	return (void *)arg;
}
MT_CPUPM_SUBCRIBE_EL3_UPTIME_SYNC_WITH_KERNEL(mtk_cpc_el3_timesync_handler);

static void mtk_cpc_config(unsigned int cfg, unsigned int data)
{
	unsigned int reg = 0;

	switch (cfg) {
	case CPC_SMC_CONFIG_PROF:
		reg = CPC_MCUSYS_CPC_DBG_SETTING;
		if (data)
			mmio_setbits_32(reg, CPC_PROF_EN);
		else
			mmio_clrbits_32(reg, CPC_PROF_EN);
		break;
	case CPC_SMC_CONFIG_CNT_CLR:
		reg = CPC_MCUSYS_CLUSTER_COUNTER_CLR;
		mmio_write_32(reg, 0x3);
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
		res = mmio_read_32(CPC_MCUSYS_CPC_DBG_SETTING) & CPC_PROF_EN
			? 1 : 0;
		break;
	default:
		break;
	}

	return res;
}

#define PROF_DEV_NAME_LEN	8
uint64_t mtk_cpc_prof_dev_name(unsigned int dev_id)
{
	uint64_t ret = 0, tran = 0;
	unsigned int i = 0;
	static const char *prof_dev_name[DEV_TYPE_NUM] = {
		"CPU0",
		"CPU1",
		"CPU2",
		"CPU3",
		"CPU4",
		"CPU5",
		"CPU6",
		"CPU7",
		"CPUSYS",
		"MCUSYS"
	};

	while ((prof_dev_name[dev_id][i] != '\0') && (i < PROF_DEV_NAME_LEN)) {
		tran = (uint64_t)(prof_dev_name[dev_id][i] & 0xFF);
		ret |= (tran  << (i << 3));
		i++;
	}

	return ret;
}

static void mtk_cpc_prof_clr(void)
{
	int i;

	for (i = 0; i < DEV_TYPE_NUM; i++)
		memset((char *)&cpc_dev.p[i], 0,
			sizeof(struct mtk_cpc_lat_data));
}

void mtk_cpc_prof_enable(bool enable)
{
	unsigned int reg = 0;

	reg = CPC_MCUSYS_CPC_DBG_SETTING;
	if (enable)
		mmio_setbits_32(reg, CPC_PROF_EN);
	else
		mmio_clrbits_32(reg, CPC_PROF_EN);

	if ((cpu_cpc_prof_enabled == false) && (enable == true))
		mtk_cpc_prof_clr();
	cpu_cpc_prof_enabled = enable;
}

bool mtk_cpc_prof_is_enabled(void)
{
	return cpu_cpc_prof_enabled;
}

uint64_t mtk_cpc_prof_dev_num(void)
{
	return DEV_TYPE_NUM;
}

#define cpc_tick_to_us(val) ((val) / 13)
uint64_t mtk_cpc_prof_read(unsigned int prof_act, unsigned int dev_type)
{
	uint64_t ret = 0;
	struct mtk_cpc_lat_data *lat_data;

	if (dev_type >= DEV_TYPE_NUM)
		return CPC_ERR_FAIL;

	lat_data = &cpc_dev.p[dev_type];

	switch (prof_act) {
	case CPC_PROF_OFF_CNT:
		ret = lat_data->off_cnt;
		break;
	case CPC_PROF_OFF_AVG:
		ret = cpc_tick_to_us(lat_data->off_sum / lat_data->off_cnt);
		break;
	case CPC_PROF_OFF_MAX:
		ret = cpc_tick_to_us(lat_data->off_max);
		break;
	case CPC_PROF_OFF_MIN:
		ret = cpc_tick_to_us(lat_data->off_min);
		break;
	case CPC_PROF_ON_CNT:
		ret = lat_data->on_cnt;
		break;
	case CPC_PROF_ON_AVG:
		ret = cpc_tick_to_us(lat_data->on_sum / lat_data->on_cnt);
		break;
	case CPC_PROF_ON_MAX:
		ret = cpc_tick_to_us(lat_data->on_max);
		break;
	case CPC_PROF_ON_MIN:
		ret = cpc_tick_to_us(lat_data->on_min);
		break;
	default:
		break;
	}

	return ret;
}

uint64_t mtk_cpc_prof_latency(unsigned int prof_act, unsigned int arg)
{
	uint64_t res = 0;

	switch (prof_act) {
	case CPC_PROF_ENABLE:
		mtk_cpc_prof_enable((bool)arg);
		break;
	case CPC_PROF_ENABLED:
		res = (uint64_t)mtk_cpc_prof_is_enabled();
		break;
	case CPC_PROF_DEV_NUM:
		res = mtk_cpc_prof_dev_num();
		break;
	case CPC_PROF_DEV_NAME:
		res = mtk_cpc_prof_dev_name(arg);
		break;
	case CPC_PROF_OFF_CNT:
	case CPC_PROF_OFF_AVG:
	case CPC_PROF_OFF_MAX:
	case CPC_PROF_OFF_MIN:
	case CPC_PROF_ON_CNT:
	case CPC_PROF_ON_AVG:
	case CPC_PROF_ON_MAX:
	case CPC_PROF_ON_MIN:
		res = (uint64_t)mtk_cpc_prof_read(prof_act, arg);
		break;

	default:
		break;
	}

	return res;
}

uint64_t mtk_cpc_handler(uint64_t act, uint64_t arg1, uint64_t arg2)
{
	uint64_t res = 0;

	switch (act) {
	case CPC_SMC_EVENT_GIC_DPG_SET:
		/* isolated_status = x2; */
		break;
	case CPC_SMC_EVENT_CPC_CONFIG:
		mtk_cpc_config((unsigned int)arg1, (unsigned int)arg2);
		break;
	case CPC_SMC_EVENT_READ_CONFIG:
		res = mtk_cpc_read_config((unsigned int)arg1);
		break;
	case CPC_SMC_EVENT_PROF_LATENCY:
		res = mtk_cpc_prof_latency((unsigned int)arg1,
					   (unsigned int)arg2);
		break;
	default:
		break;
	}

	return res;
}

uint64_t mtk_cpc_trace_dump(uint64_t act, uint64_t arg1, uint64_t arg2)
{
	uint64_t res = 0;

	switch (act) {
	case CPC_SMC_EVENT_DUMP_TRACE_DATA:
		mtk_cpc_dump_timestamp();
		break;
	default:
		break;
	}

	return res;
}

void mtk_cpu_pm_counter_clear(void)
{
	unsigned int cpu = 0;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		mmio_write_32(SYSRAM_RECENT_CPU_CNT(cpu), 0);

	mmio_write_32(SYSRAM_RECENT_CLUSTER_CNT, 0);
	mmio_write_32(SYSRAM_RECENT_MCUSYS_CNT, 0);
	mmio_write_32(SYSRAM_CPUSYS_CNT, 0);
	mmio_write_32(SYSRAM_MCUSYS_CNT, 0);
	mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, 0x3);
	mmio_write_32(SYSRAM_CLUSTER_CNT_BACKUP, 0x0);
	mmio_write_32(SYSRAM_RECENT_CNT_TS_H, 0x0);
	mmio_write_32(SYSRAM_RECENT_CNT_TS_L, 0x0);
}

void mtk_cpu_pm_counter_enable(bool enable)
{
	cpu_pm_counter_enabled = enable;
	if (cpu_pm_counter_enabled == false)
		mtk_cpu_pm_counter_clear();
}

bool mtk_cpu_pm_counter_enabled(void)
{
	return cpu_pm_counter_enabled;
}

#define sec_to_us(v)	((v) * 1000 * 1000ULL)
#define DUMP_INTERVAL	sec_to_us(5)
void mtk_cpu_pm_counter_update(unsigned int cpu)
{
#ifdef CONFIG_MTK_CPU_SUSPEND_EN
	unsigned int cnt = 0, curr_mcusys_cnt = 0, mcusys_cnt = 0;
	static unsigned int prev_mcusys_cnt = 0,
			    cpu_cnt[PLATFORM_CORE_COUNT] = {0};
	uint64_t curr_us = 0;
	static uint64_t last_dump_us;
	static bool reset;

	if (is_cpu_pm_counter_enabled() == false) {
		reset = true;
		return;
	}

	if (reset == true) {
		last_dump_us = sched_clock() / 1000;
		prev_mcusys_cnt = mmio_read_32(MCUPM_TCM_MCUSYS_COUNTER);
		mtk_cpu_pm_counter_clear();
		cpu_cnt[cpu] = 0;
		reset = false;
	}

	cpu_cnt[cpu]++;

	curr_us = sched_clock() / 1000;
	if (curr_us - last_dump_us > DUMP_INTERVAL) {
		last_dump_us = curr_us;

		/* CPU off count */
		for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
			mmio_write_32(SYSRAM_RECENT_CPU_CNT(cpu),
				      cpu_cnt[cpu]);
			cpu_cnt[cpu] = 0;
		}

		/* Cluster off count */
		curr_mcusys_cnt = mmio_read_32(MCUPM_TCM_MCUSYS_COUNTER);
		if (curr_mcusys_cnt >= prev_mcusys_cnt)
			mcusys_cnt = curr_mcusys_cnt - prev_mcusys_cnt;
		else
			mcusys_cnt = curr_mcusys_cnt;
		prev_mcusys_cnt = mmio_read_32(MCUPM_TCM_MCUSYS_COUNTER);

		cnt = mmio_read_32(CPC_MCUSYS_CLUSTER_COUNTER);
		/**
		 * bit[0:15] : memory retention
		 * bit[16:31] : memory off
		 */
		if ((cnt & MCUSYS_CLUSTER_DORMANT_MASK) == 0)
			cnt = ((cnt >> 16) & MCUSYS_CLUSTER_DORMANT_MASK);
		else
			cnt = cnt & MCUSYS_CLUSTER_DORMANT_MASK;
		cnt += mmio_read_32(SYSRAM_CLUSTER_CNT_BACKUP);
		cnt += mcusys_cnt;

		mmio_write_32(SYSRAM_RECENT_CLUSTER_CNT, cnt);
		mmio_write_32(SYSRAM_CPUSYS_CNT,
			      cnt + mmio_read_32(SYSRAM_CPUSYS_CNT));
		mmio_write_32(CPC_MCUSYS_CLUSTER_COUNTER_CLR, 0x3);
		mmio_write_32(SYSRAM_CLUSTER_CNT_BACKUP, 0x0);

		/* MCUSYS off count */
		mmio_write_32(SYSRAM_RECENT_MCUSYS_CNT,
			      mcusys_cnt);

		mmio_write_32(SYSRAM_MCUSYS_CNT,
			      mmio_read_32(SYSRAM_MCUSYS_CNT) + mcusys_cnt);

		mmio_write_32(SYSRAM_RECENT_CNT_TS_H,
			      (unsigned int)((last_dump_us >> 32) & 0xFFFFFFFF));

		mmio_write_32(SYSRAM_RECENT_CNT_TS_L,
			      (unsigned int)(last_dump_us & 0xFFFFFFFF));
	}
#endif /* CONFIG_MTK_CPU_SUSPEND_EN */
}

#define __mtk_cpc_record_lat(sum, min, max, lat)\
	do {					\
		if (lat > max)			\
			max = lat;		\
		if ((lat < min) || (min == 0))	\
			min = lat;		\
		(sum) += (lat);			\
	} while (0)

#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_cpu_pm_cpc_lock);
#define plat_cpu_pm_cpc_lock_init() bakery_lock_init(&mt_cpu_pm_cpc_lock)
#define plat_cpu_pm_cpc_lock() bakery_lock_get(&mt_cpu_pm_cpc_lock)
#define plat_cpu_pm_cpc_unlock() bakery_lock_release(&mt_cpu_pm_cpc_lock)
#else
spinlock_t mt_cpu_pm_cpc_lock;
#define plat_cpu_pm_cpc_lock_init()
#define plat_cpu_pm_cpc_lock() spin_lock(&mt_cpu_pm_cpc_lock)
#define plat_cpu_pm_cpc_unlock() spin_unlock(&mt_cpu_pm_cpc_lock)
#endif /* MT_CPU_PM_USING_BAKERY_LOCK */

static void mtk_cpc_record_lat(struct mtk_cpc_lat_data *lat,
			       unsigned int on_ticks, unsigned int off_ticks)
{
	if ((on_ticks == 0) || (off_ticks == 0))
		return;

	__mtk_cpc_record_lat(lat->on_sum, lat->on_min, lat->on_max, on_ticks);
	lat->on_cnt++;
	__mtk_cpc_record_lat(lat->off_sum, lat->off_min,
			     lat->off_max, off_ticks);
	lat->off_cnt++;
}

#define CPC_CPU_LATENCY_MASK	0xFFFF
void mtk_cpu_pm_save_cpc_latency(enum dev_type dev_type)
{
	unsigned int lat = 0, lat_on = 0, lat_off = 0;
	struct mtk_cpc_lat_data *lat_data = NULL;

	if (mtk_cpc_prof_is_enabled() == false)
		return;

	plat_cpu_pm_cpc_lock();

	if (dev_type < DEV_TYPE_CPUSYS) {
		lat = mmio_read_32(CPC_CPU_ON_LATENCY(dev_type));
		lat_on = lat & CPC_CPU_LATENCY_MASK;
		lat = mmio_read_32(CPC_CPU_OFF_LATENCY(dev_type));
		lat_off = lat & CPC_CPU_LATENCY_MASK;
		lat_data = &cpc_dev.cpu[dev_type];
	} else if (dev_type == DEV_TYPE_CPUSYS) {
		lat_on = mmio_read_32(CPC_CLUSTER_ON_LATENCY);
		lat_on = lat_on & CPC_CPU_LATENCY_MASK;
		lat_off = mmio_read_32(CPC_CLUSTER_OFF_LATENCY);
		lat_off = lat_off & CPC_CPU_LATENCY_MASK;
		lat_data = &cpc_dev.cluster;
	} else if (dev_type == DEV_TYPE_MCUSYS) {
		lat = mmio_read_32(CPC_MCUSYS_ON_LATENCY);
		lat_on = lat & CPC_CPU_LATENCY_MASK;
		lat = mmio_read_32(CPC_MCUSYS_OFF_LATENCY);
		lat_off = lat & CPC_CPU_LATENCY_MASK;
		lat_data = &cpc_dev.mcusys;
	}

	if (lat_data)
		mtk_cpc_record_lat(lat_data, lat_on, lat_off);

	plat_cpu_pm_cpc_unlock();
}

#define RVBARADDR_ONKEEPON_SEL			(MCUCFG_BASE + 0x388)

void mtk_cpc_init(void)
{
	struct mtk_plat_dev_config cfg = {
#ifndef CPU_PM_ACP_FSM
		.auto_off = 1,
#else
		.auto_off = 0,
#endif /* CPU_PM_ACP_FSM */
		.auto_thres_us = MTK_CPC_AUTO_DORMANT_THR_US,
	};

	if (mmio_read_32(RVBARADDR_ONKEEPON_SEL) == 0x1) {
		ERROR("ONKEEPON_SEL=%x, CPC_FLOW_CTRL_CFG=%x\n",
		      mmio_read_32(RVBARADDR_ONKEEPON_SEL),
		      mmio_read_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG));
		mmio_write_32(RVBARADDR_ONKEEPON_SEL, 0x1);
	}

#if CONFIG_MTK_SMP_EN
	mt_smp_init();
#endif /* CONFIG_MTK_SMP_EN */

#if CONFIG_MTK_CPU_SUSPEND_EN
	mtk_cpu_pm_counter_clear();
#endif /* CONFIG_MTK_CPU_SUSPEND_EN */

	mtk_cpc_auto_dormant_en(cfg.auto_off);
	mtk_cpc_auto_dormant_tick(cfg.auto_thres_us);

	mmio_setbits_32(CPC_MCUSYS_CPC_DBG_SETTING,
			CPC_DBG_EN | CPC_CALC_EN);

	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
			CPC_OFF_PRE_EN);

	/* enable CPC */
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);

	plat_cpu_pm_cpc_lock_init();
}

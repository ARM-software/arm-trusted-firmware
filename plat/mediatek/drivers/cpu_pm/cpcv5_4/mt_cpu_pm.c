/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/delay_timer.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rm.h>
#include "mt_cpu_pm.h"
#include "mt_cpu_pm_cpc.h"
#include "mt_cpu_pm_mbox.h"
#include "mt_cpu_pm_smc.h"
#include "mt_lp_irqremain.h"
#include "mt_ppu.h"
#include "mt_smp.h"
#include <mtk_mmap_pool.h>
#include <pwr_topology.h>

/*
 * The locker must use the bakery locker when cache turn off.
 * Using spin_lock will has better performance.
 */
#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_cpu_pm_lock);
#define plat_cpu_pm_lock_init() bakery_lock_init(&mt_cpu_pm_lock)
#define plat_cpu_pm_lock() bakery_lock_get(&mt_cpu_pm_lock)
#define plat_cpu_pm_unlock() bakery_lock_release(&mt_cpu_pm_lock)
#else
spinlock_t mt_cpu_pm_lock;
#define plat_cpu_pm_lock_init()
#define plat_cpu_pm_lock() spin_lock(&mt_cpu_pm_lock)
#define plat_cpu_pm_unlock() spin_unlock(&mt_cpu_pm_lock)
#endif /* MT_CPU_PM_USING_BAKERY_LOCK */

#define cpu_pm_unlikely(x) __builtin_expect(!!(x), 0)

enum mt_pwr_node {
	MT_PWR_SYSTEM_MCUSYS = 0,
	MT_PWR_SYSTEM_VCORE,
	MT_PWR_MAX
};

#define CPU_PM_DEPD_MASK		0x0000000F
#define CPU_PM_DEPD_INIT		BIT(0)
#define CPU_PM_DEPD_READY		BIT(1)
#define CPU_PM_PLAT_READY		BIT(2)

#define CPU_PM_AFFLV_CLUSTER_ABORT	BIT(0)
#define CPU_PM_AFFLV_MCUSYS_ABORT	BIT(4)

enum cpupm_pwr_req_def {
	CPUPM_PWR_REQ_CLUSTER,
	CPUPM_PWR_REQ_MCUSYS,
	CPUPM_PWR_REQ_MAX
};

#ifdef CPU_PM_TINYSYS_SUPPORT
#define CPU_PM_LP_READY		(CPU_PM_DEPD_INIT | \
				 CPU_PM_DEPD_READY | \
				 CPU_PM_PLAT_READY)
#else
#define CPU_PM_LP_READY		(CPU_PM_PLAT_READY)
#endif /* CPU_PM_TINYSYS_SUPPORT */

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
	!CPU_PM_DOMAIN_CORE_ONLY
static int mt_pwr_nodes[MT_PWR_MAX];
static int plat_mt_lp_cpu_rc;
static struct mt_cpu_pm_record cpu_pm_record;
static uint64_t suspend_abort_reason;
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN &&
	* !CPU_PM_DOMAIN_CORE_ONLY
	*/

static struct mtk_plat_dev_config plat_dev;

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
#define CPUPM_ARCH_TIME_MS(_ms)	((_ms) * 1000 * SYS_COUNTER_FREQ_IN_MHZ)
#define CPUPM_BOOTUP_TIME_THR	CPUPM_ARCH_TIME_MS(CPUPM_READY_MS)

static unsigned int cpu_pm_status;

#ifdef CPU_PM_PWR_REQ
unsigned int cpupm_pwr_reqs[CPUPM_PWR_REQ_MAX];
#endif /* CPU_PM_PWR_REQ */

#ifdef CPU_PM_SUSPEND_NOTIFY
#define IS_CPU_SUPEND_SAVE(__cid) (cpu_stage[__cid].cpu_status & \
			   (PER_CPU_STATUS_S2IDLE | PER_CPU_STATUS_HOTPLUG))

/* make sure all available cores have passed by s2idle flow in kernel */
#define IS_PLAT_ALL_ONLINE_CORES_S2IDLE(__st) ({ \
	int _res = 0; \
	if (cpu_pm_unlikely(cpu_stage[__st->info.cpuid].cpu_status \
			& PER_CPU_STATUS_S2IDLE)) { \
		unsigned int i;\
		for (i = 0, _res = 1; i < PLATFORM_CORE_COUNT; ++i) \
			if (!IS_CPU_SUPEND_SAVE(i)) { \
				_res = 0; \
				break; \
			} \
	} _res; })

#else
#define IS_PLAT_ALL_ONLINE_CORES_S2IDLE(__st) \
	IS_PLAT_SUSPEND_ID(__st->pwr.state_id)

#endif /* CPU_PM_SUSPEND_NOTIFY */

#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */

#ifdef CPU_PM_SUSPEND_NOTIFY
static struct per_cpu_stage cpu_stage[PLATFORM_CORE_COUNT];
#endif /* CPU_PM_SUSPEND_NOTIFY */

#if CONFIG_MTK_CPU_SUSPEND_EN || CONFIG_MTK_SMP_EN
#if CONFIG_MTK_PM_SUPPORT
static void cpupm_cluster_resume_common(void)
{
	struct cluster_pwr_ctrl cls_pwr_ctrl;

	PER_CLUSTER_PWR_CTRL(cls_pwr_ctrl, 0);

#ifndef CPU_PM_ACP_FSM
	mt_smp_ppu_pwr_set(&cls_pwr_ctrl.pwr, PPU_PWPR_DYNAMIC_MODE,
		(plat_dev.auto_off) ? PPU_PWPR_MEM_RET : PPU_PWPR_OFF);
#endif /* CPU_PM_ACP_FSM */
}

#ifdef CONFIG_MTK_CPU_ILDO

#define read_cpupwrctlr()		read_cpupwrctlr_el3()
#define write_cpupwrctlr(_v)		write_cpupwrctlr_el3(_v)
#define mt_cpu_retention_enable(_ret_delay) \
	write_cpupwrctlr((read_cpupwrctlr() & \
			 (~(CPUPWRCTLR_EL3_WFI_RET_MASK << \
			 CPUPWRCTLR_EL3_WFI_RET_SHIFT))) | \
			 ((_ret_delay & CPUPWRCTLR_EL3_WFI_RET_MASK) << \
			 CPUPWRCTLR_EL3_WFI_RET_SHIFT))
#define mt_cpu_retention_disable() \
	write_cpupwrctlr(read_cpupwrctlr() & \
			 (~(CPUPWRCTLR_EL3_WFI_RET_MASK << \
			 CPUPWRCTLR_EL3_WFI_RET_SHIFT)))

static unsigned int cpu_retention_enable[PLATFORM_CORE_COUNT];

static void cpupm_cpu_retention_init(void)
{
	unsigned int i;

	for (i = 0; i < PLATFORM_CORE_COUNT; i++)
		cpu_retention_enable[i] = 0;
	INFO("[CPU_PM]: CPU_RET_MASK: 0x%x\n", CPU_PM_CPU_RET_MASK);
}

static void cpupm_cpu_retention_set(unsigned int ret_delay)
{
	mt_cpu_retention_enable(ret_delay);
}

static unsigned int cpupm_cpu_ildo_state_valid(unsigned int cpu)
{
	unsigned int timeout = 0, ret_sta_reg;

	if (!cpu_retention_enable[cpu])
		return CPU_PM_RET_SET_FAIL;

	CPU_PM_ASSERT(cpu < PLATFORM_CORE_COUNT);
	while (timeout < CPU_RET_TIMEOUT) {
		if (mmio_read_32(CPU_EB_RET_STA_REG) & BIT(cpu)) {
			cpupm_cpu_retention_set(cpu_retention_enable[cpu]);
			return CPU_PM_RET_SET_SUCCESS;
		}
		udelay(1);
		timeout++;
	}

	ERROR("[CPU_RET] wait brisket init timeout, sta:%x\n", ret_sta_reg);
	return CPU_PM_RET_SET_FAIL;
}

unsigned int cpupu_get_cpu_retention_control(void)
{
	unsigned int i, ret = 0;

	for (i = 0; i < PLATFORM_CORE_COUNT; i++)
		ret |= cpu_retention_enable[i];
	return ret;
}

unsigned int cpupm_cpu_retention_control(unsigned int enable)
{
	unsigned int ret = CPU_PM_RET_SET_FAIL;
	unsigned int cpu = plat_my_core_pos();

	if ((cpu_pm_status == CPU_PM_LP_READY) &&
	    (CPU_PM_CPU_RET_MASK & BIT(cpu))) {
		enable &= 0x7;
		cpu_retention_enable[cpu] = (enable & 0x7);
		if (enable) {
			ret = cpupm_cpu_ildo_state_valid(cpu);
		} else {
			mt_cpu_retention_disable();
			ret = CPU_PM_RET_SET_SUCCESS;
		}
	}
	return ret;
}
#else
#define cpupm_cpu_ildo_state_valid(cpu)
#endif /* CONFIG_MTK_CPU_ILDO */

static void cpupm_cpu_resume_common(const struct mtk_cpupm_pwrstate *state)
{
	CPU_PM_ASSERT(state);
	mtk_cpc_core_on_hint_clr(state->info.cpuid);
	cpupm_cpu_ildo_state_valid(state->info.cpuid);
}
#endif /* CONFIG_MTK_PM_SUPPORT */
#endif /* CONFIG_MTK_CPU_SUSPEND_EN || CONFIG_MTK_SMP_EN */

#define RVBARADDR_ONKEEPON_SEL		(MCUCFG_BASE + 0x388)

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN
static int cpupm_cpu_smp_afflv(unsigned int cur_afflv,
			       const struct mtk_cpupm_pwrstate *state,
			       const struct pwr_toplogy *topology)
{
#ifdef CPU_PM_TINYSYS_SUPPORT
	if (topology)
		mtk_set_mcupm_group_hint(topology->group);
#endif /* CPU_PM_TINYSYS_SUPPORT */
	return 0;
}

static int cpupm_cpu_pwr_on_prepare(unsigned int cpu, uintptr_t entry)
{
	struct cpu_pwr_ctrl pwr_ctrl = {};
	int ret = MTK_CPUPM_E_OK;

	if (mmio_read_32(RVBARADDR_ONKEEPON_SEL) == 0x1) {
		ERROR("ONKEEPON_SEL=%x, CPC_FLOW_CTRL_CFG=%x\n",
		      mmio_read_32(RVBARADDR_ONKEEPON_SEL),
		      mmio_read_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG));
		mmio_write_32(RVBARADDR_ONKEEPON_SEL, 0x1);
	}

	PER_CPU_PWR_CTRL(pwr_ctrl, cpu);
	mt_smp_core_bootup_address_set(0, cpu, &pwr_ctrl, entry);
	mt_smp_core_init_arch(0, cpu, 1, &pwr_ctrl);
	ret = mt_smp_power_core_on(cpu, &pwr_ctrl);

	return ret;
}

void cpupm_cpu_resume_smp(const struct mtk_cpupm_pwrstate *state)
{
	CPU_PM_ASSERT(state);

	cpupm_cpu_resume_common(state);
#ifdef CPU_PM_SUSPEND_NOTIFY
	cpu_stage[state->info.cpuid].cpu_status &= ~PER_CPU_STATUS_HOTPLUG;
#endif /* CPU_PM_SUSPEND_NOTIFY */
	pwr_domain_coordination(PWR_DOMAIN_SMP_ON,
				0,
				state,
				cpupm_cpu_smp_afflv);
}

void cpupm_cpu_suspend_smp(const struct mtk_cpupm_pwrstate *state)
{
	struct cpu_pwr_ctrl pwr_ctrl = {};

	CPU_PM_ASSERT(state);

	PER_CPU_PWR_CTRL(pwr_ctrl, state->info.cpuid);
	mt_smp_power_core_off(state->info.cpuid, &pwr_ctrl);

#ifdef CPU_PM_SUSPEND_NOTIFY
	cpu_stage[state->info.cpuid].cpu_status |= PER_CPU_STATUS_HOTPLUG;
#endif /* CPU_PM_SUSPEND_NOTIFY */
	pwr_domain_coordination(PWR_DOMAIN_SMP_OFF,
				0,
				state,
				cpupm_cpu_smp_afflv);
}

static void cpupm_smp_init(unsigned int cpu, uintptr_t sec_entrypoint)
{
	unsigned int reg;
	struct mtk_cpupm_pwrstate state = {
		.info = {
			.cpuid = cpu,
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = 0,
			.state_id = 0,
		},
	};

	struct cluster_pwr_ctrl cls_pwr_ctrl;

	reg = mmio_read_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG);
	if (reg & CPC_MCUSYS_CPC_RESET_PWR_ON_EN) {
		INFO("[%s:%d][CPU_PM] reset pwr on is enabled and clear it!\n",
		     __func__, __LINE__);
		mmio_clrbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
				CPC_MCUSYS_CPC_RESET_PWR_ON_EN);
	}

	PER_CLUSTER_PWR_CTRL(cls_pwr_ctrl, 0);

	mt_smp_ppu_op_set(&cls_pwr_ctrl.pwr,
			  PPU_PWPR_OP_DYNAMIC_MODE,
			  PPU_PWPR_OP_ONE_SLICE_SF_ONLY);
	cpupm_cluster_resume_common();
	cpupm_cpu_pwr_on_prepare(cpu, sec_entrypoint);
	cpupm_cpu_resume_smp(&state);
}
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN */

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN

#if !CPU_PM_DOMAIN_CORE_ONLY
static unsigned int plat_prev_stateid;
static int mcusys_prepare_suspend(unsigned int cur_afflv,
				  const struct mtk_cpupm_pwrstate *state,
				  const struct pwr_toplogy *topology)
{
	unsigned int stateid;

	if (!state)
		return MTK_CPUPM_E_FAIL;

	stateid = state->pwr.state_id;

#ifdef CPU_PM_TINYSYS_SUPPORT
	if (topology)
		mtk_set_mcupm_group_hint(topology->group);
#endif /* CPU_PM_TINYSYS_SUPPORT */

	if (!IS_PLAT_MCUSYSOFF_AFFLV(cur_afflv))
		return MTK_CPUPM_E_OK;

#ifdef CPU_PM_PWR_REQ
	if (cpupm_pwr_reqs[CPUPM_PWR_REQ_CLUSTER] ||
	    cpupm_pwr_reqs[CPUPM_PWR_REQ_MCUSYS]) {
		suspend_abort_reason = MTK_PM_SUSPEND_ABORT_PWR_REQ;
		goto mt_pwr_mcusysoff_break;
	}
#endif /* CPU_PM_PWR_REQ */

	if (mtk_cpc_mcusys_off_prepare() != CPC_SUCCESS) {
		suspend_abort_reason = MTK_PM_SUSPEND_ABORT_LAST_CORE;
		goto mt_pwr_mcusysoff_break;
	}

	if (IS_PLAT_ALL_ONLINE_CORES_S2IDLE(state))
		stateid = MT_PLAT_PWR_STATE_SUSPEND;
	else if (mt_pwr_nodes[MT_PWR_SYSTEM_MCUSYS] != 0)
		stateid = MT_PLAT_PWR_STATE_MCUSYS;
	else if (mt_pwr_nodes[MT_PWR_SYSTEM_VCORE] != 0)
		stateid = MT_PLAT_PWR_STATE_SYSTEM_VCORE;
	else
		stateid = MT_PLAT_PWR_STATE_MCUSYS;

	plat_prev_stateid = stateid;
	plat_mt_lp_cpu_rc =
		mt_lp_rm_find_constraint(0, state->info.cpuid, stateid, NULL);

	if (plat_mt_lp_cpu_rc < 0) {
		suspend_abort_reason = MTK_PM_SUSPEND_ABORT_RC_INVALID;
		goto mt_pwr_mcusysoff_reflect;
	}

#ifdef CPU_PM_TINYSYS_SUPPORT
	mtk_set_cpu_pm_preffered_cpu(state->info.cpuid);
#endif /* CPU_PM_TINYSYS_SUPPORT */
	suspend_abort_reason = MTK_PM_SUSPEND_OK;
	return MTK_CPUPM_E_OK;

mt_pwr_mcusysoff_reflect:
	mtk_cpc_mcusys_off_reflect();
mt_pwr_mcusysoff_break:
	plat_mt_lp_cpu_rc = -1;

	if (IS_PLAT_ALL_ONLINE_CORES_S2IDLE(state)) {
		/* set SPM pending if s2idle fail to turn mcusys off */
		if (suspend_abort_reason == MTK_PM_SUSPEND_ABORT_PWR_REQ)
			NOTICE("[LPM] PWR_REQ is held\n");
		else if (suspend_abort_reason == MTK_PM_SUSPEND_ABORT_LAST_CORE)
			NOTICE("[LPM] suspend last core prot fail\n");
		else if (suspend_abort_reason ==
			 MTK_PM_SUSPEND_ABORT_RC_INVALID)
			NOTICE("[LPM] no available RC\n");
	}

	return MTK_CPUPM_E_FAIL;
}

#define RECORD_NAME_LEN		(16)
#define RECORD_NAME_LEN_SMC	(8)
void mtk_cpu_pm_mcusys_record(const struct mtk_cpupm_pwrstate *state)
{
	unsigned int i = 0, j = 0;
	unsigned int stateid = state->pwr.state_id;
	char name[RECORD_NAME_LEN];
	int ret = 0;
	uint64_t tran = 0;

	memset(name, 0, sizeof(name));
	switch (stateid) {
	case MT_PLAT_PWR_STATE_MCUSYS:
	case MT_PLAT_PWR_STATE_MCUSYS_BUCK:
		ret = snprintf(name, RECORD_NAME_LEN-1, "mcusys_off");
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_MEM:
		ret = snprintf(name, RECORD_NAME_LEN-1, "system_mem");
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_PLL:
		ret = snprintf(name, RECORD_NAME_LEN-1, "system_pll");
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_BUS:
		ret = snprintf(name, RECORD_NAME_LEN-1, "system_bus");
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_VCORE:
		ret = snprintf(name, RECORD_NAME_LEN-1, "system_vcore");
		break;
	case MT_PLAT_PWR_STATE_SUSPEND:
		ret = snprintf(name, RECORD_NAME_LEN-1, "suspend");
		break;
	default:
		ret = snprintf(name, RECORD_NAME_LEN-1, "Unknown_State");
		break;
	}

	if (ret < 0) {
		INFO("[%s]snprintf error%d\n", __func__, ret);
		return;
	}

	memset(cpu_pm_record.name, 0, sizeof(cpu_pm_record.name));
	while ((i < RECORD_NAME_LEN) && (*(name + i) != '\0')) {
		if (i == RECORD_NAME_LEN_SMC)
			++j;
		tran = (uint64_t)(*(name + i) & 0xFF);
		cpu_pm_record.name[j] |= (tran <<
			((i - (RECORD_NAME_LEN_SMC * j)) << 3));
		if (name[i] == '\0')
			break;
		i++;
	}

	cpu_pm_record.cnt++;
}

uint64_t mtk_mcusys_off_record_cnt_get(void)
{
	return cpu_pm_record.cnt;
}

uint64_t mtk_mcusys_off_record_name_get(void)
{
	static unsigned int idx;
	uint64_t ret = 0;

	ret = cpu_pm_record.name[idx];
	idx = !idx;

	return ret;
}

static int mcusys_prepare_resume(unsigned int cur_afflv,
				 const struct mtk_cpupm_pwrstate *state,
				 const struct pwr_toplogy *topology)
{
	uint32_t cpu = plat_my_core_pos();

	if (!state)
		return MTK_CPUPM_E_FAIL;

#ifdef CPU_PM_TINYSYS_SUPPORT
	if (topology)
		mtk_set_mcupm_group_hint(topology->group);
#endif /* CPU_PM_TINYSYS_SUPPORT */

	if (!IS_PLAT_MCUSYSOFF_AFFLV(cur_afflv))
		return MTK_CPUPM_E_OK;

#ifdef CPU_PM_PWR_REQ
	if (cpupm_pwr_reqs[CPUPM_PWR_REQ_CLUSTER] ||
	    cpupm_pwr_reqs[CPUPM_PWR_REQ_MCUSYS])
		return MTK_CPUPM_E_FAIL;
#endif /* CPU_PM_PWR_REQ */

	if (plat_mt_lp_cpu_rc < 0)
		return MTK_CPUPM_E_FAIL;

	mt_lp_rm_reset_constraint(plat_mt_lp_cpu_rc,
				  state->info.cpuid,
				  plat_prev_stateid);
	mtk_cpc_mcusys_off_reflect();

	mtk_cpu_pm_mcusys_record(state);

	/* Clear EXT_INT_WAKEUP_REQ of the first-on CPU */
	mmio_write_32(SPM_EXT_INT_WAKEUP_REQ_CLR, BIT(cpu));
	if (mmio_read_32(SPM_EXT_INT_WAKEUP_REQ)) {
		NOTICE("EXT_INT_WAKEUP_REQ(%u) is not cleared. CPU: %lu\n",
		       mmio_read_32(SPM_EXT_INT_WAKEUP_REQ),
		       BIT(cpu));
		CPU_PM_ASSERT(0);
	}

	return MTK_CPUPM_E_OK;
}

uint64_t mtk_suspend_abort_reason_get(void)
{
	return suspend_abort_reason;
}
#endif /* CPU_PM_DOMAIN_CORE_ONLY */

static unsigned int cpupm_do_pstate_off(const mtk_pstate_type psci_state,
					const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = MT_CPUPM_PWR_DOMAIN_CORE;

#ifdef CPU_PM_DOMAIN_CORE_ONLY
	pstate &= ~(MT_CPUPM_PWR_DOMAIN_CLUSTER |
		    MT_CPUPM_PWR_DOMAIN_PERCORE_DSU |
		    MT_CPUPM_PWR_DOMAIN_MCUSYS);
#else
	if (!state || (state->pwr.afflv > PLAT_MAX_PWR_LVL)) {
		CPU_PM_ASSERT(state);
		CPU_PM_ASSERT(state->pwr.afflv <= PLAT_MAX_PWR_LVL);
	}

	/*
	 * If all core afflv is higher than PLAT_MAX_RET_STATE
	 * and state's id is MT_PLAT_PWR_STATE_MCUSYS
	 */
	switch (state->pwr.state_id) {
	case MT_PLAT_PWR_STATE_MCUSYS_BUCK:
		mt_pwr_nodes[MT_PWR_SYSTEM_MCUSYS]++;
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_VCORE:
		mt_pwr_nodes[MT_PWR_SYSTEM_VCORE]++;
		break;
	default:
		break;
	}
	pstate |= pwr_domain_coordination(PWR_DOMAIN_OFF,
					  psci_state,
					  state,
					  mcusys_prepare_suspend);

#endif /* CPU_PM_DOMAIN_CORE_ONLY */

	return pstate;
}

static unsigned int cpupm_do_pstate_on(const mtk_pstate_type psci_state,
				       const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = MT_CPUPM_PWR_DOMAIN_CORE;

#ifdef CPU_PM_DOMAIN_CORE_ONLY
	pstate &= ~(MT_CPUPM_PWR_DOMAIN_CLUSTER |
		    MT_CPUPM_PWR_DOMAIN_PERCORE_DSU |
		    MT_CPUPM_PWR_DOMAIN_MCUSYS);
#else
	CPU_PM_ASSERT(state);

	if (state->pwr.afflv > PLAT_MAX_PWR_LVL)
		CPU_PM_ASSERT(0);

	switch (state->pwr.state_id) {
	case MT_PLAT_PWR_STATE_MCUSYS_BUCK:
		mt_pwr_nodes[MT_PWR_SYSTEM_MCUSYS]--;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SYSTEM_MCUSYS] >= 0);
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_VCORE:
		mt_pwr_nodes[MT_PWR_SYSTEM_VCORE]--;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SYSTEM_VCORE] >= 0);
		break;
	default:
		break;
	}

	pstate |= pwr_domain_coordination(PWR_DOMAIN_ON,
					  psci_state,
					  state,
					  mcusys_prepare_resume);
#endif /* CPU_PM_DOMAIN_CORE_ONLY */
	return pstate;
}

static void cpupm_cpu_resume(const struct mtk_cpupm_pwrstate *state)
{
	cpupm_cpu_resume_common(state);
}

static void cpupm_cluster_resume(const struct mtk_cpupm_pwrstate *state)
{
	cpupm_cluster_resume_common();
	mtk_cpu_pm_save_cpc_latency(DEV_TYPE_CPUSYS);
}

#if CPU_PM_PWR_REQ || CPU_PM_ACP_FSM
static void cpupm_cluster_suspend(const struct mtk_cpupm_pwrstate *state)
{
	struct cluster_pwr_ctrl cls_pwr_ctrl;

	PER_CLUSTER_PWR_CTRL(cls_pwr_ctrl, 0);

#ifdef CPU_PM_PWR_REQ
	if (cpupm_pwr_reqs[CPUPM_PWR_REQ_CLUSTER]) {
		mt_smp_ppu_pwr_dynamic_set(&cls_pwr_ctrl.pwr,
					   PPU_PWPR_ON);
		return;
	}
#endif /* CPU_PM_PWR_REQ */
#ifdef CPU_PM_ACP_FSM
	unsigned int val, pwsr, timeout_cnt = 0;

	do {
		val = mmio_read_32(MCUSYS_ACP_UTB_FSM);
		DO_ACP_FSM_WAIT_TIMEOUT(timeout_cnt);
	} while ((val & ACP_PWR_CTRL_OP_STATUS) != ACP_PWR_CTRL_OP_ST_IDLE);

	mt_smp_ppu_set(&cls_pwr_ctrl.pwr,
		       PPU_PWPR_OP_DYNAMIC_MODE,
		       DSU_PPU_PWPR_OP_MODE_DEF,
		       PPU_PWPR_DYNAMIC_MODE,
		       (plat_dev.auto_off) ?
		       PPU_PWPR_MEM_RET :
		       PPU_PWPR_OFF);

	timeout_cnt = 0;
	do {
		pwsr = mmio_read_32(cls_pwr_ctrl.pwr.ppu_pwsr);
		DO_ACP_FSM_WAIT_TIMEOUT(timeout_cnt);
	} while ((pwsr & PPU_PWSR_OP_STATUS) == PPU_OP_ST_SF_ONLY);
#endif /* CPU_PM_ACP_FSM */
}
#endif /* CPU_PM_PWR_REQ || CPU_PM_ACP_FSM */

static void cpupm_mcusys_resume(const struct mtk_cpupm_pwrstate *state)
{
#ifdef CPU_PM_IRQ_REMAIN_ENABLE
	mt_lp_irqremain_release();
#endif /* CPU_PM_IRQ_REMAIN_ENABLE */
	mtk_cpu_pm_save_cpc_latency(DEV_TYPE_MCUSYS);
}

static void cpupm_mcusys_suspend(const struct mtk_cpupm_pwrstate *state)
{
#if !CPU_PM_DOMAIN_CORE_ONLY
	struct cluster_pwr_ctrl cls_pwr_ctrl;

	assert(state);

	if (plat_mt_lp_cpu_rc < 0)
		return;

	mt_lp_rm_do_constraint(plat_mt_lp_cpu_rc,
			       state->info.cpuid,
			       plat_prev_stateid);

#ifdef CPU_PM_IRQ_REMAIN_ENABLE
	mt_lp_irqremain_aquire();
#endif /* CPU_PM_IRQ_REMAIN_ENABLE */
	if (plat_dev.auto_off) {
		/*
		 * The DSU ppu setting is DYN_MEM_RET when auto dormant enable.
		 * Need to set PPU to DYN_OFF when mcusys off.
		 *
		 */
		PER_CLUSTER_PWR_CTRL(cls_pwr_ctrl, 0);
		mt_smp_ppu_pwr_set(&cls_pwr_ctrl.pwr,
				   PPU_PWPR_DYNAMIC_MODE,
				   PPU_PWPR_OFF);
	}
#endif /* CPU_PM_DOMAIN_CORE_ONLY */
}

static unsigned int cpupm_get_pstate(enum mt_cpupm_pwr_domain domain,
				     const mtk_pstate_type psci_state,
				     const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = 0;

	if (!state)
		return 0;

	if (state->info.mode == MTK_CPU_PM_SMP)
		pstate = MT_CPUPM_PWR_DOMAIN_CORE;
	else {
		if (domain == CPUPM_PWR_OFF)
			pstate = cpupm_do_pstate_off(psci_state, state);
		else if (domain == CPUPM_PWR_ON)
			pstate = cpupm_do_pstate_on(psci_state, state);
		else {
			INFO("[%s:%d][CPU_PM] unknown pwr domain :%d\n",
			     __func__, __LINE__, domain);
			assert(0);
		}
	}
	return pstate;
}

#define CPUPM_READY_MS (40000)
static int cpupm_pwr_state_valid(unsigned int afflv, unsigned int state)
{
	if (cpu_pm_status == CPU_PM_LP_READY)
		return MTK_CPUPM_E_OK;

	if (cpu_pm_status != CPU_PM_LP_READY) {
#ifdef CPU_PM_TINYSYS_SUPPORT
		int status = 0;

		if (!(cpu_pm_status & CPU_PM_DEPD_INIT)) {
			status = mtk_lp_depd_condition(
				CPUPM_MBOX_WAIT_DEV_INIT);
			if (status == 0) {
				plat_cpu_pm_lock();
				cpu_pm_status |= CPU_PM_DEPD_INIT;
				plat_cpu_pm_unlock();
			}
		} else if (!(cpu_pm_status & CPU_PM_DEPD_READY)) {
			status = mtk_lp_depd_condition(
				CPUPM_MBOX_WAIT_TASK_READY);
			if (status == 0) {
				plat_cpu_pm_lock();
				cpu_pm_status |= CPU_PM_DEPD_READY;
				plat_cpu_pm_unlock();
			}
		} else {
#endif /* CPU_PM_TINYSYS_SUPPORT */
			uint64_t arch_time = read_cntpct_el0();

			if (arch_time > (uint64_t)CPUPM_BOOTUP_TIME_THR) {
				plat_cpu_pm_lock();
				cpu_pm_status |= CPU_PM_PLAT_READY;
				plat_cpu_pm_unlock();
			}
#ifdef CPU_PM_TINYSYS_SUPPORT
		}
#endif /* CPU_PM_TINYSYS_SUPPORT */
		return MTK_CPUPM_E_FAIL;
	}
	return MTK_CPUPM_E_OK;
}
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */

#define CPUPM_PWR_STAT_REQ_UID_MAGIC	(0xbacd1103)

#define IS_VALID_CPUPM_PWR_STAT_REQ(mg) \
	((mg & CPUPM_PWR_STAT_REQ_UID_MAGIC) == CPUPM_PWR_STAT_REQ_UID_MAGIC)

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
static int cpupm_invoke(unsigned int func_id, void *priv)
{
	int ret = MTK_CPUPM_E_OK;
#ifdef CPU_PM_SUSPEND_NOTIFY
	int i, reverse = 0;
	struct cpupm_invoke_data *save_status = (struct cpupm_invoke_data *) priv;
	unsigned int cpu_status;
#endif
	struct cpupm_pwr_req *req = (struct cpupm_pwr_req *)priv;
	unsigned int pwr_req = req->req;

	switch (func_id) {
#ifdef CPU_PM_SUSPEND_NOTIFY
	case CPUPM_INVOKE_WAKED_CPU:
		if (priv) {
			for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
				cpu_status = cpu_stage[i].cpu_status;
				if (IS_CPUPM_SAVE_PWR_STATUS(cpu_status))
					reverse |= BIT(i);
			}
			save_status->val.v_u32 = ~reverse;
		} else
			ret = MTK_CPUPM_E_ERR;
		break;
#endif /* CPU_PM_SUSPEND_NOTIFY */

#ifdef CPU_PM_PWR_REQ
	case CPUPM_INVOKE_PWR_REQ_ACTIVE:
		if (priv) {
			if (req->stat.uid == CPUPM_PWR_REQ_UID_MAGIC)
				req->stat.uid = CPUPM_PWR_STAT_REQ_UID_MAGIC;
			else
				ret = MTK_CPUPM_E_ERR;
		} else
			ret = MTK_CPUPM_E_ERR;
		break;
	case CPUPM_INVOKE_PWR_REQ_ACQUIRE:
	case CPUPM_INVOKE_PWR_REQ_RELASE:
		if (priv && (IS_VALID_CPUPM_PWR_STAT_REQ(req->stat.uid))) {
			plat_cpu_pm_lock();
			if (func_id == CPUPM_INVOKE_PWR_REQ_ACQUIRE) {
				if (pwr_req & MT_CPUPM_PWR_DOMAIN_CLUSTER)
					pwr_req |=
					MT_CPUPM_PWR_DOMAIN_MCUSYS_BY_CLUSTER;

				pwr_req = pwr_req & ~req->stat.sta_req;

				if (pwr_req & MT_CPUPM_PWR_DOMAIN_CLUSTER)
					cpupm_pwr_reqs[CPUPM_PWR_REQ_CLUSTER]++;

				if ((pwr_req & MT_CPUPM_MCUSYS_REQ) &&
				    !(req->stat.sta_req & MT_CPUPM_MCUSYS_REQ))
					cpupm_pwr_reqs[CPUPM_PWR_REQ_MCUSYS]++;

				req->stat.sta_req |= pwr_req;
			} else {
				if (pwr_req & MT_CPUPM_PWR_DOMAIN_CLUSTER)
					pwr_req |=
					MT_CPUPM_PWR_DOMAIN_MCUSYS_BY_CLUSTER;

				pwr_req = pwr_req & req->stat.sta_req;
				req->stat.sta_req &= ~pwr_req;

				if (pwr_req & MT_CPUPM_PWR_DOMAIN_CLUSTER) {
					if (cpupm_pwr_reqs[
					   CPUPM_PWR_REQ_CLUSTER] > 0)
						cpupm_pwr_reqs[
						CPUPM_PWR_REQ_CLUSTER]--;
				}

				if ((pwr_req & MT_CPUPM_MCUSYS_REQ) &&
				    !(req->stat.sta_req &
				    MT_CPUPM_MCUSYS_REQ)) {
					if (cpupm_pwr_reqs[
					    CPUPM_PWR_REQ_MCUSYS] > 0)
						cpupm_pwr_reqs[
						CPUPM_PWR_REQ_MCUSYS]--;
				}
			}

			plat_cpu_pm_unlock();
		} else
			ret = MTK_CPUPM_E_ERR;
		break;
#endif /* CPU_PM_PWR_REQ */
	default:
		ret = MTK_CPUPM_E_ERR;
		break;
	}
	return ret;
}
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */

void mt_plat_cpu_pm_dev_update(struct mtk_plat_dev_config *config)
{
	if (!config)
		return;
	plat_dev.auto_off = config->auto_off;
	plat_dev.auto_thres_us = config->auto_thres_us;
}

int mt_plat_cpu_pm_dev_config(struct mtk_plat_dev_config **config)
{
	if (!config)
		return MTK_CPUPM_E_FAIL;
	*config = &plat_dev;
	return 0;
}

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN
static struct mtk_cpu_smp_ops cpcv5_0_cpu_smp = {
	.init = cpupm_smp_init,
	.cpu_pwr_on_prepare = cpupm_cpu_pwr_on_prepare,
	.cpu_on = cpupm_cpu_resume_smp,
	.cpu_off = cpupm_cpu_suspend_smp,
};
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN */

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
static struct mtk_cpu_pm_ops cpcv5_0_mcdi = {
	.get_pstate = cpupm_get_pstate,
	.pwr_state_valid = cpupm_pwr_state_valid,
	.cpu_resume = cpupm_cpu_resume,
#if CPU_PM_PWR_REQ || CPU_PM_ACP_FSM
	.cluster_suspend = cpupm_cluster_suspend,
#endif /* CPU_PM_PWR_REQ || CPU_PM_ACP_FSM */
	.cluster_resume = cpupm_cluster_resume,
	.mcusys_suspend = cpupm_mcusys_suspend,
	.mcusys_resume = cpupm_mcusys_resume,
	.invoke = cpupm_invoke,
};
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */

/* Init cpu_state.cpu_status as Hotplugged for non-boot CPUs. */
static void mtk_cpu_status_init(void)
{
#ifdef CPU_PM_SUSPEND_NOTIFY
	for (int i = 1 ; i < PLATFORM_CORE_COUNT; i++)
		cpu_stage[i].cpu_status |= PER_CPU_STATUS_HOTPLUG;
#endif /* CPU_PM_SUSPEND_NOTIFY */
}

/*
 * Depend on mtk pm methodology, the psci op init must
 * be invoked after cpu pm to avoid initialization fail.
 */
int mt_plat_cpu_pm_init(void)
{
	plat_cpu_pm_lock_init();
	pwr_topology_init();
	mtk_cpc_init();
	mtk_cpu_status_init();
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
	register_cpu_pm_ops(CPU_PM_FN(), &cpcv5_0_mcdi);
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN
	register_cpu_smp_ops(CPU_PM_FN(), &cpcv5_0_cpu_smp);
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_SMP_EN */
#ifdef CPU_PM_IRQ_REMAIN_ENABLE
	mt_lp_irqremain_init();
#endif /* CPU_PM_IRQ_REMAIN_ENABLE */
	cpupm_smc_init();
#ifdef CONFIG_MTK_CPU_ILDO
	cpupm_cpu_retention_init();
#endif /* CONFIG_MTK_CPU_ILDO */
	INFO("[%s:%d] - MCDI finished\n", __func__, __LINE__);
	return 0;
}
MTK_ARCH_INIT(mt_plat_cpu_pm_init);

static const mmap_region_t cpu_pm_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(MT_UTILITYBUS_BASE,
			MT_UTILITYBUS_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
#ifdef CPU_PM_TINYSYS_SUPPORT
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
	MAP_REGION_FLAT(CPU_EB_TCM_BASE,
			CPU_EB_TCM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
#ifdef CPU_EB_TCM_CNT_BASE
	MAP_REGION_FLAT(CPU_EB_TCM_CNT_BASE,
			CPU_EB_TCM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif /* CPU_EB_TCM_CNT_BASE */
#endif /* CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN */
#endif /* CPU_PM_TINYSYS_SUPPORT */
	{0},
};
DECLARE_MTK_MMAP_REGIONS(cpu_pm_mmap);

static void *cpupm_core_pwr_handler(const void *arg, unsigned int act)
{
	struct mt_cpupm_event_data *nb =
	(struct mt_cpupm_event_data *)arg;

	if (!arg || (nb->cpuid >= PLATFORM_CORE_COUNT))
		return (void *)arg;

	if (act & MT_CPUPM_PWR_ON) {
#ifdef CPU_PM_SUSPEND_NOTIFY
		cpu_stage[nb->cpuid].cpu_status &= ~PER_CPU_STATUS_PDN;
#endif /* CPU_PM_SUSPEND_NOTIFY */
		mtk_cpu_pm_counter_update(nb->cpuid);
		mtk_cpu_pm_save_cpc_latency(nb->cpuid);
	} else {
#ifdef CPU_PM_SUSPEND_NOTIFY
		cpu_stage[nb->cpuid].cpu_status |= PER_CPU_STATUS_PDN;
#endif /* CPU_PM_SUSPEND_NOTIFY */
	}

	return (void *)arg;
}

void *cpupm_core_pwr_off_handler(const void *arg)
{
	return cpupm_core_pwr_handler(arg, MT_CPUPM_PWR_OFF);
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(cpupm_core_pwr_off_handler);

void *cpupm_core_pwr_on_handler(const void *arg)
{
	return cpupm_core_pwr_handler(arg, MT_CPUPM_PWR_ON);
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(cpupm_core_pwr_on_handler);

#ifdef CPU_PM_SUSPEND_NOTIFY
int cpupm_set_suspend_state(unsigned int act, unsigned int cpuid)
{
	if (cpuid >= PLATFORM_CORE_COUNT)
		return MTK_CPUPM_E_ERR;

	if (act & MT_LPM_SMC_ACT_SET)
		cpu_stage[cpuid].cpu_status |= PER_CPU_STATUS_S2IDLE;
	else
		cpu_stage[cpuid].cpu_status &= ~PER_CPU_STATUS_S2IDLE;

	return 0;
}
#endif /* CPU_PM_SUSPEND_NOTIFY */

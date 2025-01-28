/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_H
#define MT_CPU_PM_H

#include <assert.h>

#include <platform_def.h>

#include <lib/pm/mtk_pm.h>

#if !HW_ASSISTED_COHERENCY
#define MT_CPU_PM_USING_BAKERY_LOCK
#endif /* !HW_ASSISTED_COHERENCY */

/*
 * Enable bit of CPU_PM callbacks
 */
static inline unsigned int CPU_PM_FN(void)
{
	return (MTK_CPUPM_FN_CPUPM_GET_PWR_STATE |
		MTK_CPUPM_FN_PWR_STATE_VALID |
		MTK_CPUPM_FN_PWR_ON_CORE_PREPARE |
		MTK_CPUPM_FN_RESUME_CORE |
#ifdef CPU_PM_PWR_REQ
		MTK_CPUPM_FN_SUSPEND_CLUSTER |
#endif /* CPU_PM_PWR_REQ */
		MTK_CPUPM_FN_RESUME_CLUSTER |
		MTK_CPUPM_FN_SUSPEND_MCUSYS |
		MTK_CPUPM_FN_RESUME_MCUSYS |
		MTK_CPUPM_FN_SMP_INIT |
		MTK_CPUPM_FN_SMP_CORE_ON |
		MTK_CPUPM_FN_SMP_CORE_OFF);
}

#define CPU_PM_ASSERT(_cond) ({ \
	if (!(_cond)) { \
		INFO("[%s:%d] - %s\n", __func__, __LINE__, #_cond); \
		panic(); \
	} })

/* related registers */
#define SPM_POWERON_CONFIG_EN			(SPM_BASE + 0x000)
#define SPM_CPU_PWR_STATUS			(SPM_BASE + 0x174)

/* bit-fields of SPM_POWERON_CONFIG_EN */
#define PROJECT_CODE				(0xB16U << 16)
#define BCLK_CG_EN				BIT(0)

#define CPC_PWR_MASK_MCUSYS_MP0			(0xC001)

#define PER_CLUSTER_PWR_DATA(_p, _cl) ({ \
	_p.pwr.ppu_pwpr = CLUSTER_PPU_PWPR_##_cl; \
	_p.pwr.ppu_pwsr = CLUSTER_PPU_PWSR_##_cl; \
	_p.pwr.ppu_dcdr0 = CLUSTER_PPU_DCDR0_##_cl; \
	_p.pwr.ppu_dcdr1 = CLUSTER_PPU_DCDR1_##_cl; \
	})

#define PER_CLUSTER_PWR_CTRL(_val, _cl) ({ \
	switch (_cl) { \
	case 0: \
		PER_CLUSTER_PWR_DATA(_val, 0); \
		break; \
	default: \
		assert(0); \
		break; \
	} })

#define PER_CPU_PWR_DATA(_p, _cl, _c) ({ \
	_p.rvbaraddr_l = CORE_RVBRADDR_##_cl##_##_c##_L; \
	_p.rvbaraddr_h = CORE_RVBRADDR_##_cl##_##_c##_H; \
	_p.pwr.ppu_pwpr = CORE_PPU_PWPR_##_cl##_##_c; \
	_p.pwr.ppu_pwsr = CORE_PPU_PWSR_##_cl##_##_c; \
	_p.pwr.ppu_dcdr0 = CORE_PPU_DCDR0_##_cl##_##_c; \
	_p.pwr.ppu_dcdr1 = CORE_PPU_DCDR1_##_cl##_##_c; })

#define PER_CPU_PWR_CTRL(_val, _cpu) ({ \
	switch (_cpu) { \
	case 0: \
		PER_CPU_PWR_DATA(_val, 0, 0); \
		break; \
	case 1: \
		PER_CPU_PWR_DATA(_val, 0, 1); \
		break; \
	case 2: \
		PER_CPU_PWR_DATA(_val, 0, 2); \
		break; \
	case 3: \
		PER_CPU_PWR_DATA(_val, 0, 3); \
		break; \
	case 4: \
		PER_CPU_PWR_DATA(_val, 0, 4); \
		break; \
	case 5: \
		PER_CPU_PWR_DATA(_val, 0, 5); \
		break; \
	case 6: \
		PER_CPU_PWR_DATA(_val, 0, 6); \
		break; \
	case 7: \
		PER_CPU_PWR_DATA(_val, 0, 7); \
		break; \
	default: \
		assert(0); \
		break; \
	} })

/*
 * Definition about bootup address for each core
 * CORE_RVBRADDR_clusterid_cpuid
 */
#define CORE_RVBRADDR_0_0_L		(MCUCFG_BASE + 0x00)
#define CORE_RVBRADDR_0_1_L		(MCUCFG_BASE + 0x08)
#define CORE_RVBRADDR_0_2_L		(MCUCFG_BASE + 0x10)
#define CORE_RVBRADDR_0_3_L		(MCUCFG_BASE + 0x18)
#define CORE_RVBRADDR_0_4_L		(MCUCFG_BASE + 0x20)
#define CORE_RVBRADDR_0_5_L		(MCUCFG_BASE + 0x28)
#define CORE_RVBRADDR_0_6_L		(MCUCFG_BASE + 0x30)
#define CORE_RVBRADDR_0_7_L		(MCUCFG_BASE + 0x38)

#define CORE_RVBRADDR_0_0_H		(MCUCFG_BASE + 0x04)
#define CORE_RVBRADDR_0_1_H		(MCUCFG_BASE + 0x0C)
#define CORE_RVBRADDR_0_2_H		(MCUCFG_BASE + 0x14)
#define CORE_RVBRADDR_0_3_H		(MCUCFG_BASE + 0x1C)
#define CORE_RVBRADDR_0_4_H		(MCUCFG_BASE + 0x24)
#define CORE_RVBRADDR_0_5_H		(MCUCFG_BASE + 0x2C)
#define CORE_RVBRADDR_0_6_H		(MCUCFG_BASE + 0x34)
#define CORE_RVBRADDR_0_7_H		(MCUCFG_BASE + 0x3C)

/*
 * Definition about PPU PWPR for each core
 * PPU_PWPR_clusterid_cpuid
 */
#define CORE_PPU_PWPR_0_0		(MT_UTILITYBUS_BASE + 0x080000)
#define CORE_PPU_PWPR_0_1		(MT_UTILITYBUS_BASE + 0x180000)
#define CORE_PPU_PWPR_0_2		(MT_UTILITYBUS_BASE + 0x280000)
#define CORE_PPU_PWPR_0_3		(MT_UTILITYBUS_BASE + 0x380000)
#define CORE_PPU_PWPR_0_4		(MT_UTILITYBUS_BASE + 0x480000)
#define CORE_PPU_PWPR_0_5		(MT_UTILITYBUS_BASE + 0x580000)
#define CORE_PPU_PWPR_0_6		(MT_UTILITYBUS_BASE + 0x680000)
#define CORE_PPU_PWPR_0_7		(MT_UTILITYBUS_BASE + 0x780000)

/*
 * Definition about PPU PWSR for each core
 * PPU_PWSR_clusterid_cpuid
 */
#define CORE_PPU_PWSR_0_0		(MT_UTILITYBUS_BASE + 0x080008)
#define CORE_PPU_PWSR_0_1		(MT_UTILITYBUS_BASE + 0x180008)
#define CORE_PPU_PWSR_0_2		(MT_UTILITYBUS_BASE + 0x280008)
#define CORE_PPU_PWSR_0_3		(MT_UTILITYBUS_BASE + 0x380008)
#define CORE_PPU_PWSR_0_4		(MT_UTILITYBUS_BASE + 0x480008)
#define CORE_PPU_PWSR_0_5		(MT_UTILITYBUS_BASE + 0x580008)
#define CORE_PPU_PWSR_0_6		(MT_UTILITYBUS_BASE + 0x680008)
#define CORE_PPU_PWSR_0_7		(MT_UTILITYBUS_BASE + 0x780008)

/*
 * Definition about device delay control 0
 * PPU_DCDR0_clusterid_cpuid
 */
#define CORE_PPU_DCDR0_0_0		(MT_UTILITYBUS_BASE + 0x080170)
#define CORE_PPU_DCDR0_0_1		(MT_UTILITYBUS_BASE + 0x180170)
#define CORE_PPU_DCDR0_0_2		(MT_UTILITYBUS_BASE + 0x280170)
#define CORE_PPU_DCDR0_0_3		(MT_UTILITYBUS_BASE + 0x380170)
#define CORE_PPU_DCDR0_0_4		(MT_UTILITYBUS_BASE + 0x480170)
#define CORE_PPU_DCDR0_0_5		(MT_UTILITYBUS_BASE + 0x580170)
#define CORE_PPU_DCDR0_0_6		(MT_UTILITYBUS_BASE + 0x680170)
#define CORE_PPU_DCDR0_0_7		(MT_UTILITYBUS_BASE + 0x780170)

/*
 * Definition about device delay control 1
 * PPU_DCDR0_clusterid_cpuid
 */
#define CORE_PPU_DCDR1_0_0		(MT_UTILITYBUS_BASE + 0x080174)
#define CORE_PPU_DCDR1_0_1		(MT_UTILITYBUS_BASE + 0x180174)
#define CORE_PPU_DCDR1_0_2		(MT_UTILITYBUS_BASE + 0x280174)
#define CORE_PPU_DCDR1_0_3		(MT_UTILITYBUS_BASE + 0x380174)
#define CORE_PPU_DCDR1_0_4		(MT_UTILITYBUS_BASE + 0x480174)
#define CORE_PPU_DCDR1_0_5		(MT_UTILITYBUS_BASE + 0x580174)
#define CORE_PPU_DCDR1_0_6		(MT_UTILITYBUS_BASE + 0x680174)
#define CORE_PPU_DCDR1_0_7		(MT_UTILITYBUS_BASE + 0x780174)

/*
 * Definition about PPU PWPR for cluster
 * PPU_PWPR_clusterid
 */
#define CLUSTER_PPU_PWPR_0		(MT_UTILITYBUS_BASE + 0x030000)
#define CLUSTER_PPU_PWSR_0		(MT_UTILITYBUS_BASE + 0x030008)
#define CLUSTER_PPU_DCDR0_0		(MT_UTILITYBUS_BASE + 0x030170)
#define CLUSTER_PPU_DCDR1_0		(MT_UTILITYBUS_BASE + 0x030174)

struct ppu_pwr_ctrl {
	unsigned int ppu_pwpr;
	unsigned int ppu_pwsr;
	unsigned int ppu_dcdr0;
	unsigned int ppu_dcdr1;
};

struct cpu_pwr_ctrl {
	unsigned int rvbaraddr_l;
	unsigned int rvbaraddr_h;
#ifndef CPU_PM_CORE_ARCH64_ONLY
	unsigned int arch_addr;
#endif /* CPU_PM_CORE_ARCH64_ONLY */
	struct ppu_pwr_ctrl pwr;
	unsigned int pwr_ctrl;
};

struct cluster_pwr_ctrl {
	struct ppu_pwr_ctrl pwr;
};

#define MT_CPUPM_PWR_ON			BIT(0)
#define MT_CPUPM_PWR_OFF		BIT(1)

#ifdef CPU_PM_SUSPEND_NOTIFY
#define PER_CPU_STATUS_S2IDLE		BIT(0)
#define PER_CPU_STATUS_PDN		BIT(1)
#define PER_CPU_STATUS_HOTPLUG		BIT(2)
#define PER_CPU_STATUS_S2IDLE_PDN \
	(PER_CPU_STATUS_S2IDLE | PER_CPU_STATUS_PDN)

#define CPUPM_PWR_STATUS(_state, _tar)		((_state & _tar) == _tar)
#define IS_CPUPM_SAVE_PWR_STATUS(_state)	( \
	CPUPM_PWR_STATUS(_state, PER_CPU_STATUS_S2IDLE_PDN) || \
	(_state & PER_CPU_STATUS_HOTPLUG))

#ifdef CONFIG_MTK_CPU_ILDO
#define CPU_PM_CPU_RET_IS_ENABLED	CPU_PM_CPU_RET_MASK

enum {
	CPU_PM_RET_SET_SUCCESS = 0,
	CPU_PM_RET_SET_FAIL
};

#define CPU_EB_RET_STA_REG	(CPU_EB_TCM_BASE + CPU_EB_RET_STA_OFFSET)
#define CPU_RET_TIMEOUT		100
#endif /* CONFIG_MTK_CPU_ILDO */

struct per_cpu_stage {
	unsigned int cpu_status;
};
#endif /* CPU_PM_SUSPEND_NOTIFY */

#define MCUSYS_STATUS_PDN		BIT(0)
#define MCUSYS_STATUS_CPUSYS_PROTECT	BIT(8)
#define MCUSYS_STATUS_MCUSYS_PROTECT	BIT(9)

#ifdef CPU_PM_ACP_FSM
#define ACP_FSM_TIMEOUT_MAX		(500)
#define ACP_FSM_AWARE_TIME		(100)
#define DO_ACP_FSM_WAIT_TIMEOUT(k_cnt) ({ \
	if (k_cnt >= ACP_FSM_TIMEOUT_MAX) { \
		INFO("[%s:%d] - ACP FSM TIMEOUT %u us (> %u)\n", \
		     __func__, __LINE__, k_cnt, ACP_FSM_TIMEOUT_MAX); \
		panic(); \
	} else if (k_cnt == ACP_FSM_AWARE_TIME) { \
		INFO("[%s:%d] - ACP FSM latency exceed %u us\n", \
		     __func__, __LINE__, ACP_FSM_AWARE_TIME); \
	} \
	k_cnt++; udelay(1); })
#endif /* CPU_PM_ACP_FSM */

/* cpu_pm function ID */
enum mt_cpu_pm_user_id {
	MCUSYS_STATUS = 0,
	CPC_COMMAND,
};

/* cpu_pm lp function ID */
enum mt_cpu_pm_lp_smc_id {
	LP_CPC_COMMAND = 0,
	IRQS_REMAIN_ALLOC,
	IRQS_REMAIN_CTRL,
	IRQS_REMAIN_IRQ,
	IRQS_REMAIN_WAKEUP_CAT,
	IRQS_REMAIN_WAKEUP_SRC,
	SUSPEND_SRC,
	CPU_PM_COUNTER_CTRL,
	CPU_PM_RECORD_CTRL,
	SUSPEND_ABORT_REASON,
	CPU_PM_RET_CTRL
};

enum mt_suspend_abort_reason {
	MTK_PM_SUSPEND_OK = 0,
	MTK_PM_SUSPEND_ABORT_PWR_REQ,
	MTK_PM_SUSPEND_ABORT_LAST_CORE,
	MTK_PM_SUSPEND_ABORT_RC_INVALID,
};

struct mtk_plat_dev_config {
	int auto_off;
	unsigned int auto_thres_us;
};

struct mt_cpu_pm_record {
	unsigned int cnt;
	uint64_t name[2];
};

unsigned int cpupm_cpu_retention_control(unsigned int enable);
unsigned int cpupu_get_cpu_retention_control(void);
void mt_plat_cpu_pm_dev_update(struct mtk_plat_dev_config *config);
int mt_plat_cpu_pm_dev_config(struct mtk_plat_dev_config **config);
int cpupm_set_suspend_state(unsigned int act, unsigned int cpuid);
uint64_t mtk_mcusys_off_record_cnt_get(void);
uint64_t mtk_mcusys_off_record_name_get(void);
uint64_t mtk_suspend_abort_reason_get(void);

#endif /* MT_CPU_PM_H */

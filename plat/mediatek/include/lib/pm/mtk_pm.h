/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PM_H
#define MTK_PM_H
#include <lib/psci/psci.h>

#if MTK_PUBEVENT_ENABLE
#include <vendor_pubsub_events.h>
#endif

#define MTK_CPUPM_E_OK				0
#define MTK_CPUPM_E_UNKNOWN			-1
#define MTK_CPUPM_E_ERR				-2
#define MTK_CPUPM_E_FAIL			-3
#define MTK_CPUPM_E_NOT_SUPPORT			-4

#define MTK_CPUPM_FN_PWR_LOCK_AQUIRE		BIT(0)
#define MTK_CPUPM_FN_INIT			BIT(1)
#define MTK_CPUPM_FN_PWR_STATE_VALID		BIT(2)
#define MTK_CPUPM_FN_PWR_ON_CORE_PREPARE	BIT(3)
#define MTK_CPUPM_FN_SUSPEND_CORE		BIT(4)
#define MTK_CPUPM_FN_RESUME_CORE		BIT(5)
#define MTK_CPUPM_FN_SUSPEND_CLUSTER		BIT(6)
#define MTK_CPUPM_FN_RESUME_CLUSTER		BIT(7)
#define MTK_CPUPM_FN_SUSPEND_MCUSYS		BIT(8)
#define MTK_CPUPM_FN_RESUME_MCUSYS		BIT(9)
#define MTK_CPUPM_FN_CPUPM_GET_PWR_STATE	BIT(10)
#define MTK_CPUPM_FN_SMP_INIT			BIT(11)
#define MTK_CPUPM_FN_SMP_CORE_ON		BIT(12)
#define MTK_CPUPM_FN_SMP_CORE_OFF		BIT(13)
#define MTK_CPUPM_FN_PWR_DOMAIN_POWER_DOWN_WFI	BIT(14)

enum mtk_cpupm_pstate {
	MTK_CPUPM_CORE_ON,
	MTK_CPUPM_CORE_OFF,
	MTK_CPUPM_CORE_SUSPEND,
	MTK_CPUPM_CORE_RESUME,
	MTK_CPUPM_CLUSTER_SUSPEND,
	MTK_CPUPM_CLUSTER_RESUME,
	MTK_CPUPM_MCUSYS_SUSPEND,
	MTK_CPUPM_MCUSYS_RESUME,
};

enum mtk_cpu_pm_mode {
	MTK_CPU_PM_CPUIDLE,
	MTK_CPU_PM_SMP,
};

#define MT_IRQ_REMAIN_MAX       32
#define MT_IRQ_REMAIN_CAT_LOG   BIT(31)

struct mt_irqremain {
	unsigned int count;
	unsigned int irqs[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc_cat[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc[MT_IRQ_REMAIN_MAX];
};

typedef void (*plat_init_func)(unsigned int, uintptr_t);
struct plat_pm_smp_ctrl {
	plat_init_func init;
	int (*pwr_domain_on)(u_register_t mpidr);
	void (*pwr_domain_off)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
};

struct plat_pm_pwr_ctrl {
	void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish_late)(
				const psci_power_state_t *target_state);
	void (*pwr_domain_suspend_finish)(
				const psci_power_state_t *target_state);
	int (*validate_power_state)(unsigned int power_state,
				    psci_power_state_t *req_state);
	void (*get_sys_suspend_power_state)(
				    psci_power_state_t *req_state);
	void (*pwr_domain_pwr_down_wfi)(
				const psci_power_state_t *req_state);
};

struct plat_pm_reset_ctrl {
	__dead2 void (*system_off)();
	__dead2 void (*system_reset)();
	int (*system_reset2)(int is_vendor,
			     int reset_type,
			     u_register_t cookie);
};

struct mtk_cpu_pm_info {
	unsigned int cpuid;
	unsigned int mode;
};

struct mtk_cpu_pm_state {
	unsigned int afflv;
	unsigned int state_id;
	const psci_power_state_t *raw;
};

struct mtk_cpupm_pwrstate {
	struct mtk_cpu_pm_info info;
	struct mtk_cpu_pm_state pwr;
};

struct mtk_cpu_smp_ops {
	void (*init)(unsigned int cpu, uintptr_t sec_entrypoint);
	int (*cpu_pwr_on_prepare)(unsigned int cpu, uintptr_t entry);
	void (*cpu_on)(const struct mtk_cpupm_pwrstate *state);
	void (*cpu_off)(const struct mtk_cpupm_pwrstate *state);
	int (*invoke)(unsigned int funcID, void *priv);
};

#define CPUPM_PWR_REQ_UID_MAGIC		0x1103BAAD

#ifdef CPU_PM_PWR_REQ_DEBUG
#define DECLARE_CPUPM_PWR_REQ(var_name)\
	static struct cpupm_pwr_req var_name = {\
		.stat.name = #var_name,\
		.stat.uid = CPUPM_PWR_REQ_UID_MAGIC,\
		.stat.sta_req = 0,\
	}
#else
#define DECLARE_CPUPM_PWR_REQ(name)\
	static struct cpupm_pwr_req name = {\
		.stat.uid = CPUPM_PWR_REQ_UID_MAGIC,\
		.stat.sta_req = 0,\
	}
#endif

#define CPUPM_PWR_REQ_ACTIVE(_cpupm_req) ({\
	int in_ret;\
	in_ret = plat_pm_invoke_func(MTK_CPU_PM_CPUIDLE,\
			    CPUPM_INVOKE_PWR_REQ_ACTIVE,\
			    &_cpupm_req);\
	in_ret; })

#define CPUPM_PWR_REQ_ACQUIRE(_cpupm_req, _pm_req) ({\
	int in_ret;\
	_cpupm_req.req = _pm_req;\
	in_ret = plat_pm_invoke_func(MTK_CPU_PM_CPUIDLE,\
			    CPUPM_INVOKE_PWR_REQ_ACQUIRE,\
			    &_cpupm_req);\
	in_ret; })

#define CPUPM_PWR_REQ_RELEASE(_cpupm_req, _pm_req) ({\
	int in_ret;\
	_cpupm_req.req = _pm_req;\
	in_ret = plat_pm_invoke_func(MTK_CPU_PM_CPUIDLE,\
			    CPUPM_INVOKE_PWR_REQ_RELASE,\
			    &_cpupm_req);\
	in_ret; })

struct cpupm_pwr_stat_req {
	unsigned int sta_req;
	unsigned int uid;
#ifdef CPU_PM_PWR_REQ_DEBUG
	const char *name;
#endif
};

struct cpupm_pwr_req {
	unsigned int req;
	struct cpupm_pwr_stat_req stat;
};

struct cpupm_invoke_data {
	union {
		unsigned int v_u32;
		struct cpupm_pwr_req *req;
	} val;
};

enum cpupm_invoke_func_id {
	/* Get regular active cpumask */
	CPUPM_INVOKE_WAKED_CPU = 0,
	CPUPM_INVOKE_PWR_REQ_ACTIVE,
	CPUPM_INVOKE_PWR_REQ_ACQUIRE,
	CPUPM_INVOKE_PWR_REQ_RELASE,
};

#define MT_CPUPM_MCUSYS_REQ	(MT_CPUPM_PWR_DOMAIN_MCUSYS | \
				 MT_CPUPM_PWR_DOMAIN_MCUSYS_BY_CLUSTER)
#define MT_CPUPM_PWR_DOMAIN_CORE		BIT(0)
#define MT_CPUPM_PWR_DOMAIN_PERCORE_DSU		BIT(1)
#define MT_CPUPM_PWR_DOMAIN_PERCORE_DSU_MEM	BIT(2)
#define MT_CPUPM_PWR_DOMAIN_CLUSTER		BIT(3)
#define MT_CPUPM_PWR_DOMAIN_MCUSYS		BIT(4)
#define MT_CPUPM_PWR_DOMAIN_SUSPEND		BIT(5)
#define MT_CPUPM_PWR_DOMAIN_MCUSYS_BY_CLUSTER	BIT(6)

enum mt_cpupm_pwr_domain {
	CPUPM_PWR_ON,
	CPUPM_PWR_OFF,
};

#define mtk_pstate_type	unsigned int

struct mtk_cpu_pm_ops {
	void (*init)(unsigned int cpu, uintptr_t sec_entrypoint);

	unsigned int (*get_pstate)(enum mt_cpupm_pwr_domain domain,
				   const mtk_pstate_type psci_state,
				   const struct mtk_cpupm_pwrstate *state);

	int (*pwr_state_valid)(unsigned int afflv, unsigned int state);

	void (*cpu_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*cpu_resume)(const struct mtk_cpupm_pwrstate *state);

	void (*cluster_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*cluster_resume)(const struct mtk_cpupm_pwrstate *state);

	void (*mcusys_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*mcusys_resume)(const struct mtk_cpupm_pwrstate *state);
	int (*pwr_domain_pwr_down_wfi)(unsigned int cpu);

	int (*invoke)(unsigned int funcID, void *priv);
};

int register_cpu_pm_ops(unsigned int fn_flags, struct mtk_cpu_pm_ops *ops);
int register_cpu_smp_ops(unsigned int fn_flags, struct mtk_cpu_smp_ops *ops);

struct mt_cpupm_event_data {
	unsigned int cpuid;
	unsigned int pwr_domain;
};

/* Extension event for platform driver */
#if MTK_PUBEVENT_ENABLE
/* [PUB_EVENT] Core power on */
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_pwr_on, _fn)

/* [PUB_EVENT] Core power off */
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_pwr_off, _fn)

/* [PUB_EVENT] Cluster power on */
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_on, _fn)

/* [PUB_EVENT] Cluster power off */
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_off, _fn)

/* [PUB_EVENT] Mcusys power on */
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_on, _fn)

/* [PUB_EVENT] Mcusys power off */
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_off, _fn)

/* [PUB_EVENT] el3 time sync */
#define MT_CPUPM_SUBCRIBE_EL3_UPTIME_SYNC_WITH_KERNEL(_fn) \
	SUBSCRIBE_TO_EVENT(el3_uptime_sync_with_kernel, _fn)
#else
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(_fn)
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_OFF(_fn)
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_OFF(_fn)
#define MT_CPUPM_SUBCRIBE_EL3_UPTIME_SYNC_WITH_KERNEL(_fn)
#endif

/*
 * Definition c-state power domain.
 * bit 0:	Cluster
 * bit 1:	CPU buck
 * bit 2:	Mcusys
 * bit 3:	Memory
 * bit 4:	System pll
 * bit 5:	System bus
 * bit 6:	SoC 26m/DCXO
 * bit 7:	Vcore buck
 * bit 8~14:	Reserved
 * bit 15:	Suspend
 */
#define MT_PLAT_PWR_STATE_CLUSTER			0x0001
#define MT_PLAT_PWR_STATE_MCUSYS			0x0005
#define MT_PLAT_PWR_STATE_MCUSYS_BUCK			0x0007
#define MT_PLAT_PWR_STATE_SYSTEM_MEM			0x000F
#define MT_PLAT_PWR_STATE_SYSTEM_PLL			0x001F
#define MT_PLAT_PWR_STATE_SYSTEM_BUS			0x007F
#define MT_PLAT_PWR_STATE_SYSTEM_VCORE			0x00FF
#define MT_PLAT_PWR_STATE_SUSPEND			0x80FF

#define IS_MT_PLAT_PWR_STATE(_state, _tar)		\
	(((_state) & _tar) == _tar)
#define IS_MT_PLAT_PWR_STATE_MCUSYS(state)		\
	IS_MT_PLAT_PWR_STATE(state, MT_PLAT_PWR_STATE_MCUSYS)
#define IS_MT_PLAT_PWR_STATE_SYSTEM(state)		((state) & 0x7ff8)

#ifdef PLAT_AFFLV_SYSTEM
#define PLAT_MT_SYSTEM_SUSPEND		PLAT_AFFLV_SYSTEM
#else
#define PLAT_MT_SYSTEM_SUSPEND		PLAT_MAX_OFF_STATE
#endif

#ifdef PLAT_AFFLV_CLUSTER
#define PLAT_MT_CPU_SUSPEND_CLUSTER	PLAT_AFFLV_CLUSTER
#else
#define PLAT_MT_CPU_SUSPEND_CLUSTER	PLAT_MAX_RET_STATE
#endif

#ifdef PLAT_AFFLV_MCUSYS
#define PLAT_MT_CPU_SUSPEND_MCUSYS	PLAT_AFFLV_MCUSYS
#else
#define PLAT_MT_CPU_SUSPEND_MCUSYS	PLAT_MAX_RET_STATE
#endif

#define IS_PLAT_SYSTEM_SUSPEND(aff)	((aff) == PLAT_MT_SYSTEM_SUSPEND)
#define IS_PLAT_SYSTEM_RETENTION(aff)	((aff) >= PLAT_MAX_RET_STATE)

#define IS_PLAT_SUSPEND_ID(stateid) \
	((stateid) == MT_PLAT_PWR_STATE_SUSPEND)

#define IS_PLAT_MCUSYSOFF_AFFLV(_afflv) \
	((_afflv) >= PLAT_MT_CPU_SUSPEND_MCUSYS)

int plat_pm_ops_setup_pwr(struct plat_pm_pwr_ctrl *ops);

int plat_pm_ops_setup_reset(struct plat_pm_reset_ctrl *ops);

int plat_pm_ops_setup_smp(struct plat_pm_smp_ctrl *ops);

uintptr_t plat_pm_get_warm_entry(void);

int plat_pm_invoke_func(enum mtk_cpu_pm_mode mode, unsigned int id, void *priv);

#endif

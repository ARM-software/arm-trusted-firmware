/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <arch.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include <constraints/mt_spm_rc_api.h>
#include <constraints/mt_spm_rc_internal.h>
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rm.h>
#include <lpm_v2/mt_lp_rqm.h>
#include <lpm_v2/mt_lpm_smc.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_common.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_dispatcher.h>
#include <mt_spm_hwreq.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_suspend.h>
#include <mtk_mmap_pool.h>
#include <sleep_def.h>

#ifdef MT_SPM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock_init() \
	bakery_lock_init(&spm_lock)
#else
spinlock_t spm_lock;
#define plat_spm_lock_init()
#endif

static uint32_t spm_irq_num;

void spm_set_sysclk_settle(void)
{
	uint32_t settle;

	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);
	settle = mmio_read_32(SPM_CLK_SETTLE);

	INFO("md_settle = %u, settle = %u\n", SPM_SYSCLK_SETTLE, settle);
}

void spm_set_irq_num(uint32_t num)
{
	spm_irq_num = num;
}

void spm_irq0_handler(uint64_t x1, uint64_t x2)
{
	if (x2 == 0) {
		mmio_setbits_32(SPM_IRQ_MASK, ISRM_ALL_EXC_TWAM);
		mmio_write_32(SPM_IRQ_STA, x1);
		mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT0);
	}
}

static int spm_ap_mdsrc_ack(void)
{
	int ack, md_state = 0;

	/* Check ap_mdsrc_ack = 1'b1, for md internal resource on ack */
	ack = !!(mmio_read_32(AP_MDSRC_REQ) & AP_MDSMSRC_ACK_LSB);

	if (!ack) {
		/* Check md_apsrc_req = 1'b0, for md state 0:sleep, 1:wakeup */
		md_state = !!(mmio_read_32(SPM_REQ_STA_10)
			      & MD_APSRC_REQ_LSB);

		ERROR("[SPM] error: md_sleep = %d\n", md_state);
		ERROR("%s can not get AP_MDSRC_ACK\n", __func__);
		return -1;
	}
	return 0;
}

static void spm_ap_mdsrc_req(int set)
{
	spm_lock_get();

	if (set)
		mmio_setbits_32(AP_MDSRC_REQ, AP_MDSMSRC_REQ_LSB);
	else
		mmio_clrbits_32(AP_MDSRC_REQ, AP_MDSMSRC_REQ_LSB);

	spm_lock_release();
}

static int spm_is_md_sleep(void *priv)
{
	int md_state = 0;
	int *sleep = (int *)priv;

	if (!priv)
		return -1;

	/* Check md_apsrc_req = 1'b0, for md state 0:sleep, 1:wakeup */
	md_state = !!(mmio_read_32(SPM_REQ_STA_10)
		      & MD_APSRC_REQ_LSB);

	if (md_state == 0)
		*sleep = 1;
	else
		*sleep = 0;

	return 0;
}

static void spm_ap_gpueb_pll_control(int set)
{
	spm_lock_get();

	if (set)
		mmio_setbits_32(SPM2GPUPM_CON, SC_MFG_PLL_EN_LSB);
	else
		mmio_clrbits_32(SPM2GPUPM_CON, SC_MFG_PLL_EN_LSB);

	spm_lock_release();
}

static uint32_t spm_ap_gpueb_get_pwr_status(void)
{
	uint32_t ret;

	ret = mmio_read_32(XPU_PWR_STATUS);

	return ret;
}

static uint32_t spm_ap_gpueb_get_mfg0_pwr_con(void)
{
	uint32_t ret;

	ret = mmio_read_32(MFG0_PWR_CON);

	return ret;
}

#ifndef MTK_PLAT_SPM_UNSUPPORT
struct mt_lp_res_req rq_xo_fpm = {
	.res_id = MT_LP_RQ_XO_FPM,
	.res_rq = MT_SPM_XO_FPM,
	.res_usage = 0,
};

struct mt_lp_res_req rq_26m = {
	.res_id = MT_LP_RQ_26M,
	.res_rq = MT_SPM_26M,
	.res_usage = 0,
};

struct mt_lp_res_req rq_infra = {
	.res_id = MT_LP_RQ_INFRA,
	.res_rq = MT_SPM_INFRA,
	.res_usage = 0,
};

struct mt_lp_res_req rq_syspll = {
	.res_id = MT_LP_RQ_SYSPLL,
	.res_rq = MT_SPM_SYSPLL,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s0 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S0,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s1 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S1,
	.res_usage = 0,
};

struct mt_lp_res_req rq_vcore = {
	.res_id = MT_LP_RQ_VCORE,
	.res_rq = MT_SPM_VCORE,
	.res_usage = 0,
};

struct mt_lp_res_req rq_emi = {
	.res_id = MT_LP_RQ_EMI,
	.res_rq = MT_SPM_EMI,
	.res_usage = 0,
};

struct mt_lp_res_req rq_pmic = {
	.res_id = MT_LP_RQ_PMIC,
	.res_rq = MT_SPM_PMIC,
	.res_usage = 0,
};

struct mt_lp_res_req *spm_resources[] = {
	&rq_xo_fpm,
	&rq_26m,
	&rq_infra,
	&rq_syspll,
	&rq_dram_s0,
	&rq_dram_s1,
	&rq_vcore,
	&rq_emi,
	&rq_pmic,
	NULL,
};

struct mt_resource_req_manager plat_mt8196_rq = {
	.res = spm_resources,
};

struct mt_resource_constraint plat_constraint_vcore = {
	.is_valid = spm_is_valid_rc_vcore,
	.update = spm_update_rc_vcore,
	.allow = spm_allow_rc_vcore,
	.run = spm_run_rc_vcore,
	.reset = spm_reset_rc_vcore,
	.get_status = spm_get_status_rc_vcore,
};

struct mt_resource_constraint plat_constraint_bus26m = {
	.is_valid = spm_is_valid_rc_bus26m,
	.update = spm_update_rc_bus26m,
	.allow = spm_allow_rc_bus26m,
	.run = spm_run_rc_bus26m,
	.reset = spm_reset_rc_bus26m,
	.get_status = spm_get_status_rc_bus26m,
};

struct mt_resource_constraint plat_constraint_syspll = {
	.is_valid = spm_is_valid_rc_syspll,
	.update = spm_update_rc_syspll,
	.allow = spm_allow_rc_syspll,
	.run = spm_run_rc_syspll,
	.reset = spm_reset_rc_syspll,
	.get_status = spm_get_status_rc_syspll,
};

struct mt_resource_constraint *plat_constraints[] = {
	&plat_constraint_vcore,
	&plat_constraint_bus26m,
	&plat_constraint_syspll,
	NULL,
};
#endif

int mt_spm_hwctrl(uint32_t type, int set, void *priv)
{
	int ret = 0;

	if (type == PLAT_AP_MDSRC_REQ) {
		spm_ap_mdsrc_req(set);
	} else if (type == PLAT_AP_MDSRC_ACK) {
		ret = spm_ap_mdsrc_ack();
	} else if (type == PLAT_AP_IS_MD_SLEEP) {
		ret = spm_is_md_sleep(priv);
	} else if (type == PLAT_AP_MDSRC_SETTLE) {
		if (!priv)
			return -1;
		*(int *)priv = AP_MDSRC_REQ_MD_26M_SETTLE;
	} else if (type == PLAT_AP_GPUEB_PLL_CONTROL) {
		spm_ap_gpueb_pll_control(set);
	} else if (type == PLAT_AP_GPUEB_PWR_STATUS) {
		if (!priv)
			return -1;
		*(uint32_t *)priv = spm_ap_gpueb_get_pwr_status();
	} else if (type == PLAT_AP_GPUEB_MFG0_PWR_CON) {
		if (!priv)
			return -1;
		*(uint32_t *)priv = spm_ap_gpueb_get_mfg0_pwr_con();
	} else if (type == PLAT_AP_SPM_RESOURCE_REQUEST_UPDATE) {
		struct spm_lp_scen *spmlp = NULL;

#ifdef MT_SPM_COMMON_SODI_SUPPORT
		mt_spm_common_sodi_get_spm_lp(&spmlp);
#else
#if defined(MT_SPM_FEATURE_SUPPORT)
		mt_spm_idle_generic_get_spm_lp(&spmlp);
#endif
#endif
		if (!spmlp)
			return -1;
		__spm_set_power_control(spmlp->pwrctrl, *(uint32_t *)priv);
		ret = __spm_wait_spm_request_ack(*(uint32_t *)priv,
						 SPM_ACK_TIMEOUT_US);
	} else if (type == PLAT_AP_SPM_WDT_TRIGGER) {
		mmio_write_32(PCM_WDT_VAL, 0x1);
		mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY |
				REG_PCM_WDT_EN_LSB | REG_PCM_WDT_WAKE_LSB);
	} else {
		/* Not supported type */
		return -1;
	}

	return ret;
}

#ifndef MTK_PLAT_SPM_UNSUPPORT
struct mt_resource_manager plat_mt8196_rm = {
	.update = NULL,
	.hwctrl = mt_spm_hwctrl,
	.consts = plat_constraints,
};
#else
struct mt_resource_manager plat_mt8196_rm = {
	.hwctrl = mt_spm_hwctrl,
};
#endif

/* Determine for spm sw resource user */
static struct mt_lp_resource_user spm_res_user;

struct mt_lp_resource_user *get_spm_res_user(void)
{
	return &spm_res_user;
}

#ifdef MT_SPM_COMMON_SODI_SUPPORT
int mt_spm_common_sodi_get_spm_pcm_flag(uint32_t  *lp, uint32_t idx)
{
	struct spm_lp_scen *spmlp;
	struct pwr_ctrl *pwrctrl;

	mt_spm_common_sodi_get_spm_lp(&spmlp);

	pwrctrl = spmlp->pwrctrl;

	if (!lp || idx > 1)
		return -1;

	switch (idx) {
	case 0:
		*lp = pwrctrl->pcm_flags;
		break;
	case 1:
		*lp = pwrctrl->pcm_flags;
		break;
	default:
		return -1;
	}
	return 0;
}

void mt_spm_common_sodi_en(bool en)
{
	struct spm_lp_scen *spmlp;
	struct pwr_ctrl *pwrctrl;

	mt_spm_common_sodi_get_spm_lp(&spmlp);
	pwrctrl = spmlp->pwrctrl;
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	__spm_sync_vcore_dvfs_pcm_flags(&pwrctrl->pcm_flags,
					&__spm_vcorefs.pwrctrl->pcm_flags);
#endif
	if (en)
		pwrctrl->pcm_flags |= SPM_FLAG_ENABLE_COMMON_SODI5;
	else
		pwrctrl->pcm_flags &= (~SPM_FLAG_ENABLE_COMMON_SODI5);

	/* Set PCM flags */
	__spm_set_pcm_flags(pwrctrl);

	__spm_send_cpu_wakeup_event();
}

int mt_spm_common_sodi_get_spm_lp(struct spm_lp_scen **lp)
{
	if (!lp)
		return -1;

	*lp = &__spm_common_sodi;
	return 0;
}

void mt_spm_set_common_sodi_pwrctr(void)
{
	struct resource_req_status common_sodi_spm_resource_req = {
		.id = MT_LP_RQ_ID_ALL_USAGE,
		.val = 0,
	};
	struct spm_lp_scen *spmlp;

	mt_lp_rq_get_status(PLAT_RQ_REQ_USAGE,
			&common_sodi_spm_resource_req);
	mt_spm_common_sodi_get_spm_lp(&spmlp);

	__spm_set_power_control(spmlp->pwrctrl,
		common_sodi_spm_resource_req.val);
}

void mt_spm_set_common_sodi_pcm_flags(void)
{
	struct spm_lp_scen *spmlp;
	struct pwr_ctrl *pwrctrl;

	mt_spm_common_sodi_get_spm_lp(&spmlp);
	pwrctrl = spmlp->pwrctrl;
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	/* Set PCM flags */
	__spm_sync_vcore_dvfs_pcm_flags(&pwrctrl->pcm_flags,
					&__spm_vcorefs.pwrctrl->pcm_flags);
#endif
	__spm_set_pcm_flags(pwrctrl);

	__spm_send_cpu_wakeup_event();

}
#endif

int spm_boot_init(void)
{
	plat_spm_lock_init();

#if defined(MT_SPM_FEATURE_SUPPORT)
	plat_spm_pmic_wrap_init();
#endif
	mt_lp_rm_register(&plat_mt8196_rm);

#ifndef MTK_PLAT_SPM_UNSUPPORT
	mt_lp_resource_request_manager_register(&plat_mt8196_rq);
	mt_lp_resource_user_register("SPM", &spm_res_user);
	mt_spm_dispatcher_init();
#endif
#if defined(MT_SPM_FEATURE_SUPPORT)
	spm_hwreq_init();
#endif
	spm_irq_num = 0xFFFFFFFF;

	INFO("[%s], PC = 0x%x\n", __func__, mmio_read_32(MD32PCM_PC));
	return 0;
}
MTK_PLAT_SETUP_1_INIT(spm_boot_init);

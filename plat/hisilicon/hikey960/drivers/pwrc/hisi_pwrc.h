/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HISI_PWRC_H__
#define __HISI_PWRC_H__

#include <hi3660.h>
#include <hi3660_crg.h>

#define PCTRL_BASE					(PCTRL_REG_BASE)
#define CRG_BASE					(CRG_REG_BASE)

#define SOC_CRGPERIPH_A53_PDCEN_ADDR(base)		((base) + (0x260))
#define SOC_CRGPERIPH_MAIA_PDCEN_ADDR(base)		((base) + (0x300))

#define SOC_PCTRL_RESOURCE0_LOCK_ADDR(base)		((base) + (0x400))
#define SOC_PCTRL_RESOURCE0_UNLOCK_ADDR(base)		((base) + (0x404))
#define SOC_PCTRL_RESOURCE0_LOCK_ST_ADDR(base)		((base) + (0x408))
#define SOC_PCTRL_RESOURCE1_LOCK_ADDR(base)		((base) + (0x40C))
#define SOC_PCTRL_RESOURCE1_UNLOCK_ADDR(base)		((base) + (0x410))
#define SOC_PCTRL_RESOURCE1_LOCK_ST_ADDR(base)		((base) + (0x414))
#define SOC_PCTRL_RESOURCE2_LOCK_ADDR(base)		((base) + (0x418))

#define SOC_SCTRL_SCBAKDATA3_ADDR(base)			((base) + (0x418))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base)			((base) + (0x42C))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base)			((base) + (0x430))

#define SOC_ACPU_SCTRL_BASE_ADDR			(0xFFF0A000)

void hisi_cpuidle_lock(unsigned int cluster, unsigned int core);
void hisi_cpuidle_unlock(unsigned int cluster, unsigned int core);
void hisi_set_cpuidle_flag(unsigned int cluster, unsigned int core);
void hisi_clear_cpuidle_flag(unsigned int cluster, unsigned int core);
void hisi_set_cpu_boot_flag(unsigned int cluster, unsigned int core);
void hisi_clear_cpu_boot_flag(unsigned int cluster, unsigned int core);
int cluster_is_powered_on(unsigned int cluster);
void hisi_enter_core_idle(unsigned int cluster, unsigned int core);
void hisi_enter_cluster_idle(unsigned int cluster, unsigned int core);
int hisi_test_ap_suspend_flag(unsigned int cluster);
void hisi_enter_ap_suspend(unsigned int cluster, unsigned int core);


/* pdc api */
void hisi_pdc_mask_cluster_wakeirq(unsigned int cluster);
int hisi_test_pwrdn_allcores(unsigned int cluster, unsigned int core);
void hisi_disable_pdc(unsigned int cluster);
void hisi_enable_pdc(unsigned int cluster);
void hisi_powerup_core(unsigned int cluster, unsigned int core);
void hisi_powerdn_core(unsigned int cluster, unsigned int core);
void hisi_powerup_cluster(unsigned int cluster, unsigned int core);
void hisi_powerdn_cluster(unsigned int cluster, unsigned int core);
unsigned int hisi_test_cpu_down(unsigned int cluster, unsigned int core);

#endif /* __HISI_PWRC_H__ */

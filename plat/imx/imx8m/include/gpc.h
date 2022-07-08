/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_GPC_H
#define IMX8M_GPC_H

#include <gpc_reg.h>

/* helper macro */
#define A53_LPM_MASK	U(0xF)
#define A53_LPM_WAIT	U(0x5)
#define A53_LPM_STOP	U(0xA)
#define LPM_MODE(local_state)		((local_state) == PLAT_WAIT_RET_STATE ? A53_LPM_WAIT : A53_LPM_STOP)

#define DSM_MODE_MASK	BIT(31)
#define CORE_WKUP_FROM_GIC		(IRQ_SRC_C0 | IRQ_SRC_C1 | IRQ_SRC_C2 | IRQ_SRC_C3)
#define A53_CORE_WUP_SRC(core_id)	(1 << ((core_id) < 2 ? 28 + (core_id) : 22 + (core_id) - 2))
#define COREx_PGC_PCR(core_id)		(0x800 + (core_id) * 0x40)
#define COREx_WFI_PDN(core_id)		(1 << ((core_id) < 2 ? (core_id) * 2 : ((core_id) - 2) * 2 + 16))
#define COREx_IRQ_WUP(core_id)		((core_id) < 2 ? (1 << ((core_id) * 2 + 8)) : (1 << ((core_id) * 2 + 20)))
#define COREx_LPM_PUP(core_id)		((core_id) < 2 ? (1 << ((core_id) * 2 + 9)) : (1 << ((core_id) * 2 + 21)))
#define SLTx_CFG(n)			((SLT0_CFG + ((n) * 4)))
#define SLT_COREx_PUP(core_id)		(0x2 << ((core_id) * 2))

#define IMR_MASK_ALL	0xffffffff

#define IMX_PD_DOMAIN(name, on)				\
	{						\
		.pwr_req = name##_PWR_REQ,		\
		.pgc_offset = name##_PGC,		\
		.need_sync = false,			\
		.always_on = (on),			\
	}

#define IMX_MIX_DOMAIN(name, on)			\
	{						\
		.pwr_req = name##_PWR_REQ,		\
		.pgc_offset = name##_PGC,		\
		.adb400_sync = name##_ADB400_SYNC,	\
		.adb400_ack = name##_ADB400_ACK,	\
		.need_sync = true,			\
		.always_on = (on),			\
	}

struct imx_pwr_domain {
	uint32_t pwr_req;
	uint32_t adb400_sync;
	uint32_t adb400_ack;
	uint32_t pgc_offset;
	bool need_sync;
	bool always_on;
};

DECLARE_BAKERY_LOCK(gpc_lock);

/* function declare */
void imx_gpc_init(void);
void imx_set_cpu_secure_entry(unsigned int core_index, uintptr_t sec_entrypoint);
void imx_set_cpu_pwr_off(unsigned int core_index);
void imx_set_cpu_pwr_on(unsigned int core_index);
void imx_set_cpu_lpm(unsigned int core_index, bool pdn);
void imx_set_cluster_standby(bool retention);
void imx_set_cluster_powerdown(unsigned int last_core, uint8_t power_state);
void imx_noc_slot_config(bool pdn);
void imx_set_sys_wakeup(unsigned int last_core, bool pdn);
void imx_set_sys_lpm(unsigned last_core, bool retention);
void imx_set_rbc_count(void);
void imx_clear_rbc_count(void);
void imx_anamix_override(bool enter);
void imx_gpc_pm_domain_enable(uint32_t domain_id, bool on);

#endif /*IMX8M_GPC_H */

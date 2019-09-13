/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DCM_H
#define PLAT_DCM_H

#define MP2_SYNC_DCM		(MCUCFG_BASE + 0x2274)
#define MP2_SYNC_DCM_MASK	(0x1 << 0)
#define MP2_SYNC_DCM_ON		(0x1 << 0)
#define MP2_SYNC_DCM_OFF	(0x0 << 0)

extern uint64_t plat_dcm_mcsi_a_addr;
extern uint32_t plat_dcm_mcsi_a_val;
extern int plat_dcm_initiated;

extern void plat_dcm_mcsi_a_backup(void);
extern void plat_dcm_mcsi_a_restore(void);
extern void plat_dcm_rgu_enable(void);
extern void plat_dcm_restore_cluster_on(unsigned long mpidr);
extern void plat_dcm_msg_handler(uint64_t x1);
extern unsigned long plat_dcm_get_enabled_cnt(uint64_t type);
extern void plat_dcm_init(void);

#define ALL_DCM_TYPE  (ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE \
			| STALL_DCM_TYPE | BIG_CORE_DCM_TYPE \
			| GIC_SYNC_DCM_TYPE | RGU_DCM_TYPE \
			| INFRA_DCM_TYPE \
			| DDRPHY_DCM_TYPE | EMI_DCM_TYPE | DRAMC_DCM_TYPE \
			| MCSI_DCM_TYPE)

enum {
	ARMCORE_DCM_TYPE	= (1U << 0),
	MCUSYS_DCM_TYPE		= (1U << 1),
	INFRA_DCM_TYPE		= (1U << 2),
	PERI_DCM_TYPE		= (1U << 3),
	EMI_DCM_TYPE		= (1U << 4),
	DRAMC_DCM_TYPE		= (1U << 5),
	DDRPHY_DCM_TYPE		= (1U << 6),
	STALL_DCM_TYPE		= (1U << 7),
	BIG_CORE_DCM_TYPE	= (1U << 8),
	GIC_SYNC_DCM_TYPE	= (1U << 9),
	LAST_CORE_DCM_TYPE	= (1U << 10),
	RGU_DCM_TYPE		= (1U << 11),
	TOPCKG_DCM_TYPE		= (1U << 12),
	LPDMA_DCM_TYPE		= (1U << 13),
	MCSI_DCM_TYPE		= (1U << 14),
	NR_DCM_TYPE = 15,
};

#endif /* PLAT_DCM_H */
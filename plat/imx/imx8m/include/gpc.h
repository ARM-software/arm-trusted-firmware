/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_GPC_H
#define IMX8M_GPC_H

#define LPCR_A53_BSC			0x0
#define LPCR_A53_BSC2			0x108
#define LPCR_A53_AD			0x4
#define LPCR_M4				0x8
#define SLPCR				0x14
#define MST_CPU_MAPPING			0x18
#define MLPCR				0x20
#define PGC_ACK_SEL_A53			0x24
#define IMR1_CORE0_A53			0x30
#define IMR1_CORE1_A53			0x40
#define IMR1_CORE2_A53			0x1C0
#define IMR1_CORE3_A53			0x1D0
#define IMR1_CORE0_M4			0x50
#define SLT0_CFG			0xB0
#define GPC_PU_PWRHSK			0x1FC
#define PGC_CPU_0_1_MAPPING		0xEC
#define CPU_PGC_UP_TRG			0xF0
#define PU_PGC_UP_TRG			0xF8
#define CPU_PGC_DN_TRG			0xFC
#define PU_PGC_DN_TRG			0x104
#define A53_CORE0_PGC			0x800
#define A53_PLAT_PGC			0x900
#define PLAT_PGC_PCR			0x900
#define PGC_SCU_TIMING			0x910

#define MASK_DSM_TRIGGER_A53		BIT(31)
#define IRQ_SRC_A53_WUP			BIT(30)
#define IRQ_SRC_A53_WUP_SHIFT		30
#define IRQ_SRC_C1			BIT(29)
#define IRQ_SRC_C0			BIT(28)
#define IRQ_SRC_C3			BIT(23)
#define IRQ_SRC_C2			BIT(22)
#define CPU_CLOCK_ON_LPM		BIT(14)
#define A53_CLK_ON_LPM			BIT(14)
#define MASTER0_LPM_HSK			BIT(6)

#define L2PGE				BIT(31)
#define EN_L2_WFI_PDN			BIT(5)
#define EN_PLAT_PDN			BIT(4)

#define SLPCR_EN_DSM			BIT(31)
#define SLPCR_RBC_EN			BIT(30)
#define SLPCR_A53_FASTWUP_STOP_MODE	BIT(17)
#define SLPCR_A53_FASTWUP_WAIT_MODE	BIT(16)
#define SLPCR_VSTBY			BIT(2)
#define SLPCR_SBYOS			BIT(1)
#define SLPCR_BYPASS_PMIC_READY		BIT(0)
#define SLPCR_RBC_COUNT_SHIFT		24
#define SLPCR_STBY_COUNT_SHFT		3

#define A53_DUMMY_PDN_ACK		BIT(15)
#define A53_DUMMY_PUP_ACK		BIT(31)
#define A53_PLAT_PDN_ACK		BIT(2)
#define A53_PLAT_PUP_ACK		BIT(18)

#define PLAT_PUP_SLT_CTRL		BIT(9)
#define PLAT_PDN_SLT_CTRL		BIT(8)

#define SLT_PLAT_PDN			BIT(8)
#define SLT_PLAT_PUP			BIT(9)

#define MASTER1_MAPPING			BIT(1)
#define MASTER2_MAPPING			BIT(2)

/* helper macro */
#define A53_LPM_MASK	U(0xF)
#define A53_LPM_WAIT	U(0x5)
#define A53_LPM_STOP	U(0xA)
#define LPM_MODE(local_state)		((local_state) == PLAT_WAIT_RET_STATE ? A53_LPM_WAIT : A53_LPM_STOP)

#define DSM_MODE_MASK	BIT(31)

#define A53_CORE_WUP_SRC(core_id)	(1 << ((core_id) < 2 ? 28 + (core_id) : 22 + (core_id) - 2))
#define COREx_PGC_PCR(core_id)		(0x800 + (core_id) * 0x40)
#define COREx_WFI_PDN(core_id)		(1 << ((core_id) < 2 ? (core_id) * 2 : ((core_id) - 2) * 2 + 16))
#define COREx_IRQ_WUP(core_id)		((core_id) < 2 ? (1 << ((core_id) * 2 + 8)) : (1 << ((core_id) * 2 + 20)))
#define COREx_LPM_PUP(core_id)		((core_id) < 2 ? (1 << ((core_id) * 2 + 9)) : (1 << ((core_id) * 2 + 21)))
#define SLTx_CFG(n)			((SLT0_CFG + ((n) * 4)))
#define SLT_COREx_PUP(core_id)		(0x2 << ((core_id) * 2))

#define IRQ_IMR_NUM	4
#define IMR_MASK_ALL	0xffffffff

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

#endif /*IMX8M_GPC_H */

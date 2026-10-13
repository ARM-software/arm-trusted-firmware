/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWR_CTRL_H
#define PWR_CTRL_H

#include <stdbool.h>

#include <lib/mmio.h>

#include <platform_def.h>

/*******************************************************************************
 * GPC definitions & declarations
 ******************************************************************************/
/* GPC GLOBAL */
#define GPC_GLOBAL_BASE		U(GPC_BASE + 0x4000)
#define GPC_AUTHEN_CTRL		U(0x4)
#define GPC_DOMAIN		U(0x10)
#define GPC_MASTER		U(0x1c)
#define GPC_SYS_SLEEP		U(0x40)
#define PMIC_CTRL		U(0x100)
#define PMIC_PRE_DLY_CTRL	U(0x104)
#define PMIC_STBY_ACK_CTRL	U(0x108)
#define GPC_ROSC_CTRL		U(0x200)
#define GPC_AON_MEM_CTRL	U(0x204)
#define GPC_EFUSE_CTRL		U(0x208)

#define FORCE_CPUx_DISABLE(x)	(1 << (16 + (x)))
#define PMIC_STBY_EN		BIT(0)
#define ROSC_OFF_EN		BIT(0)

/* PMIC_STBY_ACK_CTRL bit field definitions */
#define PMIC_STBY_OFF_DELAY_SHIFT	16U
#define PMIC_STBY_OFF_DELAY_MASK	(0xFFFU << PMIC_STBY_OFF_DELAY_SHIFT)
#define PMIC_STBY_OFF_DELAY(x)		(((x) & 0xFFFU) << PMIC_STBY_OFF_DELAY_SHIFT)
/* Default delay value for 1.5ms */
#define PMIC_STBY_OFF_DELAY_1_5MS	0x31U

/* GPC CPU_CTRL */
#define CM_SLICE(x)		(GPC_BASE + 0x800 * (x))
#define CM_AUTHEN_CTRL		U(0x4)
#define CM_MISC			U(0xc)
#define CM_MODE_CTRL		U(0x10)
#define CM_IRQ_WAKEUP_MASK0	U(0x100)
#define CM_SYS_SLEEP_CTRL	U(0x380)
#define IMR_NUM			U(8)

/* CM_MISC */
#define SLEEP_HOLD_EN		BIT(1)
#define IRQ_MUX			BIT(5)
#define SW_WAKEUP		BIT(6)

/* CM_SYS_SLEEP_CTRL */
#define SS_WAIT			BIT(0)
#define SS_STOP			BIT(1)
#define SS_SUSPEND		BIT(2)

#define CM_MODE_RUN		U(0x0)
#define CM_MODE_WAIT		U(0x1)
#define CM_MODE_STOP		U(0x2)
#define CM_MODE_SUSPEND		U(0x3)

#define LPM_SETTING(d, m)	((m) << (((d) % 8) * 4))

enum gpc_cmc_slice {
	CPU_M33,
	CPU_A55C0,
	CPU_A55C1,
	CPU_A55_PLAT,
};

/* set gpc domain assignment */
static inline void gpc_assign_domains(unsigned int domains)
{
	mmio_write_32(GPC_GLOBAL_BASE + GPC_DOMAIN, domains);
}

/* force a cpu into sleep status */
static inline void gpc_force_cpu_suspend(unsigned int cpu)
{
	mmio_setbits_32(GPC_GLOBAL_BASE + GPC_SYS_SLEEP, FORCE_CPUx_DISABLE(cpu));
}

static inline void gpc_pmic_stby_en(bool en)
{
	mmio_write_32(GPC_GLOBAL_BASE + PMIC_CTRL, en ? 1 : 0);
}

static inline void gpc_rosc_off(bool off)
{
	mmio_write_32(GPC_GLOBAL_BASE + GPC_ROSC_CTRL, off ? 1 : 0);
}

static inline void gpc_set_cpu_mode(unsigned int cpu, unsigned int mode)
{
	mmio_write_32(CM_SLICE(cpu) + CM_MODE_CTRL, mode);
}

static inline void gpc_select_wakeup_gic(unsigned int cpu)
{
	mmio_setbits_32(CM_SLICE(cpu) + CM_MISC, IRQ_MUX);
}

static inline void gpc_select_wakeup_raw_irq(unsigned int cpu)
{
	mmio_clrbits_32(CM_SLICE(cpu) + CM_MISC, IRQ_MUX);
}

static inline void gpc_assert_sw_wakeup(unsigned int cpu)
{
	mmio_setbits_32(CM_SLICE(cpu) + CM_MISC, SW_WAKEUP);
}

static inline void gpc_deassert_sw_wakeup(unsigned int cpu)
{
	mmio_clrbits_32(CM_SLICE(cpu) + CM_MISC, SW_WAKEUP);
}

static inline void gpc_clear_cpu_sleep_hold(unsigned int cpu)
{
	mmio_clrbits_32(CM_SLICE(cpu) + CM_MISC, SLEEP_HOLD_EN);
}

static inline void gpc_set_irq_mask(unsigned int cpu, unsigned int idx, uint32_t mask)
{
	mmio_write_32(CM_SLICE(cpu) + idx * 0x4 + CM_IRQ_WAKEUP_MASK0, mask);
}

/*******************************************************************************
 * SRC definitions & declarations
 ******************************************************************************/
#define SRC_SLICE(x)		(SRC_BASE + 0x400 * (x))
#define SRC_AUTHEN_CTRL		U(0x4)
#define SRC_LPM_SETTING0	U(0x10)
#define SRC_LPM_SETTING1	U(0x14)
#define SRC_LPM_SETTING2	U(0x18)
#define SRC_SLICE_SW_CTRL	U(0x20)
#define PSW_ACK_CTRL_0		U(0x80)

#define SRC_MEM_CTRL		U(0x4)
#define MEM_LP_EN		BIT(2)
#define MEM_LP_RETN		BIT(1)

#define CNT_MODE(m)		(((m) & 0x3) << 28)

enum mix_mem_mode {
	MEM_OFF,
	MEM_RETN,
};

enum src_mix_mem_slice {
	SRC_GLOBAL,

	/* MIX slice */
	SRC_SENTINEL,
	SRC_AON,
	SRC_WKUP,
	SRC_DDR,
	SRC_DPHY,
	SRC_ML,
	SRC_NIC,
	SRC_HSIO,
	SRC_MEDIA,
	SRC_M33P,
	SRC_A55C0,
	SRC_A55C1,
	SRC_A55P,

	/* MEM slice */
	SRC_AON_MEM,
	SRC_WKUP_MEM,
	SRC_DDR_MEM,
	SRC_DPHY_MEM,
	SRC_ML_MEM,
	SRC_NIC_MEM,
	SRC_NIC_OCRAM,
	SRC_HSIO_MEM,
	SRC_MEDIA_MEM,
	SRC_A55P0_MEM,
	SRC_A55P1_MEM,
	SRC_A55_SCU_MEM,
	SRC_A55_L3_MEM,
};

static inline void src_authen_config(unsigned int mix, unsigned int wlist,
				unsigned int lpm_en)
{
	mmio_write_32(SRC_SLICE(mix) + SRC_AUTHEN_CTRL, (wlist << 16) | (lpm_en << 2));
}

static inline void src_mix_set_lpm(unsigned int mix, unsigned int did, unsigned int lpm_mode)
{
	mmio_clrsetbits_32(SRC_SLICE(mix) + SRC_LPM_SETTING1 + (did / 8) * 0x4,
			   LPM_SETTING(did, 0x7), LPM_SETTING(did, lpm_mode));
}

static inline void src_mem_lpm_en(unsigned int mix, bool retn)
{
	mmio_setbits_32(SRC_SLICE(mix) + SRC_MEM_CTRL, MEM_LP_EN | (retn ? MEM_LP_RETN : 0));
}

static inline void src_mem_lpm_dis(unsigned int mix)
{
	mmio_clrbits_32(SRC_SLICE(mix) + SRC_MEM_CTRL, MEM_LP_EN | MEM_LP_RETN);
}

static inline void src_ack_cnt_mode(unsigned int mix, uint8_t mode)
{
	mmio_clrsetbits_32(SRC_SLICE(mix) + PSW_ACK_CTRL_0, CNT_MODE(0x3), CNT_MODE(mode));
}

/*******************************************************************************
 * CCM definitions & declarations
 ******************************************************************************/
#define LPCG(x) 	(CCM_BASE + 0x8000 + (x) * 0x40)

/*******************************************************************************
 * BLK_CTRL_S definitions & declarations
 ******************************************************************************/
#define HW_LP_HANDHSK		U(0x110)
#define HW_LP_HANDHSK2		U(0x114)
#define CA55_CPUWAIT		U(0x118)
#define CA55_RVBADDR0_L		U(0x11c)
#define CA55_RVBADDR0_H		U(0x120)

/*******************************************************************************
 * ANA definitions & declarations
 ******************************************************************************/
#define DRAM_PLL_BASE			U(0x44481300)

#define PLL_CTRL			U(0x0)
#define PLL_CTRL_SET			U(0x4)
#define PLL_CTRL_CLR			U(0x8)
#define PLL_CTRL_TOG			U(0xC)

#define PLL_SPREAD_SPECTRUM		U(0x30)
#define PLL_SPREAD_SPECTRUM_SET		U(0x34)
#define PLL_SPREAD_SPECTRUM_CLR		U(0x38)
#define PLL_SPREAD_SPECTRUM_TOG		U(0x3C)

#define PLL_NUMERATOR			U(0x40)
#define PLL_DENOMINATOR			U(0x50)

#define PLL_DIV				U(0x60)
#define PLL_STATUS			U(0xF0)

/*******************************************************************************
 * Other definitions & declarations
 ******************************************************************************/
void nicmix_qos_init(void);
void wakeupmix_qos_init(void);
void pwr_sys_init(void);

#endif /* PWR_CTRL_H */

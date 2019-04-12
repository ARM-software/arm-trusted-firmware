/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "iic_dvfs.h"
#include "rcar_def.h"
#include "rcar_private.h"
#include "micro_delay.h"
#include "pwrc.h"

/*
 * Someday there will be a generic power controller api. At the moment each
 * platform has its own pwrc so just exporting functions should be acceptable.
 */
RCAR_INSTANTIATE_LOCK

#define	WUP_IRQ_SHIFT				(0U)
#define	WUP_FIQ_SHIFT				(8U)
#define	WUP_CSD_SHIFT				(16U)
#define	BIT_SOFTRESET				(1U<<15)
#define	BIT_CA53_SCU				(1U<<21)
#define	BIT_CA57_SCU				(1U<<12)
#define	REQ_RESUME				(1U<<1)
#define	REQ_OFF					(1U<<0)
#define	STATUS_PWRUP				(1U<<4)
#define	STATUS_PWRDOWN				(1U<<0)
#define	STATE_CA57_CPU				(27U)
#define	STATE_CA53_CPU				(22U)
#define	MODE_L2_DOWN				(0x00000002U)
#define	CPU_PWR_OFF				(0x00000003U)
#define	RCAR_PSTR_MASK				(0x00000003U)
#define	ST_ALL_STANDBY				(0x00003333U)
/* Suspend to ram	*/
#define	DBSC4_REG_BASE				(0xE6790000U)
#define	DBSC4_REG_DBSYSCNT0			(DBSC4_REG_BASE + 0x0100U)
#define	DBSC4_REG_DBACEN			(DBSC4_REG_BASE + 0x0200U)
#define	DBSC4_REG_DBCMD				(DBSC4_REG_BASE + 0x0208U)
#define	DBSC4_REG_DBRFEN			(DBSC4_REG_BASE + 0x0204U)
#define	DBSC4_REG_DBWAIT			(DBSC4_REG_BASE + 0x0210U)
#define	DBSC4_REG_DBCALCNF			(DBSC4_REG_BASE + 0x0424U)
#define	DBSC4_REG_DBDFIPMSTRCNF			(DBSC4_REG_BASE + 0x0520U)
#define	DBSC4_REG_DBPDLK0			(DBSC4_REG_BASE + 0x0620U)
#define	DBSC4_REG_DBPDRGA0			(DBSC4_REG_BASE + 0x0624U)
#define	DBSC4_REG_DBPDRGD0			(DBSC4_REG_BASE + 0x0628U)
#define	DBSC4_REG_DBCAM0CTRL0			(DBSC4_REG_BASE + 0x0940U)
#define	DBSC4_REG_DBCAM0STAT0			(DBSC4_REG_BASE + 0x0980U)
#define	DBSC4_REG_DBCAM1STAT0			(DBSC4_REG_BASE + 0x0990U)
#define	DBSC4_REG_DBCAM2STAT0			(DBSC4_REG_BASE + 0x09A0U)
#define	DBSC4_REG_DBCAM3STAT0			(DBSC4_REG_BASE + 0x09B0U)
#define	DBSC4_BIT_DBACEN_ACCEN			((uint32_t)(1U << 0))
#define	DBSC4_BIT_DBRFEN_ARFEN			((uint32_t)(1U << 0))
#define	DBSC4_BIT_DBCAMxSTAT0			(0x00000001U)
#define	DBSC4_BIT_DBDFIPMSTRCNF_PMSTREN		(0x00000001U)
#define	DBSC4_SET_DBCMD_OPC_PRE			(0x04000000U)
#define	DBSC4_SET_DBCMD_OPC_SR			(0x0A000000U)
#define	DBSC4_SET_DBCMD_OPC_PD			(0x08000000U)
#define	DBSC4_SET_DBCMD_OPC_MRW			(0x0E000000U)
#define	DBSC4_SET_DBCMD_CH_ALL			(0x00800000U)
#define	DBSC4_SET_DBCMD_RANK_ALL		(0x00040000U)
#define	DBSC4_SET_DBCMD_ARG_ALL			(0x00000010U)
#define	DBSC4_SET_DBCMD_ARG_ENTER		(0x00000000U)
#define	DBSC4_SET_DBCMD_ARG_MRW_ODTC		(0x00000B00U)
#define	DBSC4_SET_DBSYSCNT0_WRITE_ENABLE	(0x00001234U)
#define	DBSC4_SET_DBSYSCNT0_WRITE_DISABLE	(0x00000000U)
#define	DBSC4_SET_DBPDLK0_PHY_ACCESS		(0x0000A55AU)
#define	DBSC4_SET_DBPDRGA0_ACIOCR0		(0x0000001AU)
#define	DBSC4_SET_DBPDRGD0_ACIOCR0		(0x33C03C11U)
#define	DBSC4_SET_DBPDRGA0_DXCCR		(0x00000020U)
#define	DBSC4_SET_DBPDRGD0_DXCCR		(0x00181006U)
#define	DBSC4_SET_DBPDRGA0_PGCR1		(0x00000003U)
#define	DBSC4_SET_DBPDRGD0_PGCR1		(0x0380C600U)
#define	DBSC4_SET_DBPDRGA0_ACIOCR1		(0x0000001BU)
#define	DBSC4_SET_DBPDRGD0_ACIOCR1		(0xAAAAAAAAU)
#define	DBSC4_SET_DBPDRGA0_ACIOCR3		(0x0000001DU)
#define	DBSC4_SET_DBPDRGD0_ACIOCR3		(0xAAAAAAAAU)
#define	DBSC4_SET_DBPDRGA0_ACIOCR5		(0x0000001FU)
#define	DBSC4_SET_DBPDRGD0_ACIOCR5		(0x000000AAU)
#define	DBSC4_SET_DBPDRGA0_DX0GCR2		(0x000000A2U)
#define	DBSC4_SET_DBPDRGD0_DX0GCR2		(0xAAAA0000U)
#define	DBSC4_SET_DBPDRGA0_DX1GCR2		(0x000000C2U)
#define	DBSC4_SET_DBPDRGD0_DX1GCR2		(0xAAAA0000U)
#define	DBSC4_SET_DBPDRGA0_DX2GCR2		(0x000000E2U)
#define	DBSC4_SET_DBPDRGD0_DX2GCR2		(0xAAAA0000U)
#define	DBSC4_SET_DBPDRGA0_DX3GCR2		(0x00000102U)
#define	DBSC4_SET_DBPDRGD0_DX3GCR2		(0xAAAA0000U)
#define	DBSC4_SET_DBPDRGA0_ZQCR			(0x00000090U)
#define	DBSC4_SET_DBPDRGD0_ZQCR_MD19_0		(0x04058904U)
#define	DBSC4_SET_DBPDRGD0_ZQCR_MD19_1		(0x04058A04U)
#define	DBSC4_SET_DBPDRGA0_DX0GCR0		(0x000000A0U)
#define	DBSC4_SET_DBPDRGD0_DX0GCR0		(0x7C0002E5U)
#define	DBSC4_SET_DBPDRGA0_DX1GCR0		(0x000000C0U)
#define	DBSC4_SET_DBPDRGD0_DX1GCR0		(0x7C0002E5U)
#define	DBSC4_SET_DBPDRGA0_DX2GCR0		(0x000000E0U)
#define	DBSC4_SET_DBPDRGD0_DX2GCR0		(0x7C0002E5U)
#define	DBSC4_SET_DBPDRGA0_DX3GCR0		(0x00000100U)
#define	DBSC4_SET_DBPDRGD0_DX3GCR0		(0x7C0002E5U)
#define	DBSC4_SET_DBPDRGA0_DX0GCR1		(0x000000A1U)
#define	DBSC4_SET_DBPDRGD0_DX0GCR1		(0x55550000U)
#define	DBSC4_SET_DBPDRGA0_DX1GCR1		(0x000000C1U)
#define	DBSC4_SET_DBPDRGD0_DX1GCR1		(0x55550000U)
#define	DBSC4_SET_DBPDRGA0_DX2GCR1		(0x000000E1U)
#define	DBSC4_SET_DBPDRGD0_DX2GCR1		(0x55550000U)
#define	DBSC4_SET_DBPDRGA0_DX3GCR1		(0x00000101U)
#define	DBSC4_SET_DBPDRGD0_DX3GCR1		(0x55550000U)
#define	DBSC4_SET_DBPDRGA0_DX0GCR3		(0x000000A3U)
#define	DBSC4_SET_DBPDRGD0_DX0GCR3		(0x00008484U)
#define	DBSC4_SET_DBPDRGA0_DX1GCR3		(0x000000C3U)
#define	DBSC4_SET_DBPDRGD0_DX1GCR3		(0x00008484U)
#define	DBSC4_SET_DBPDRGA0_DX2GCR3		(0x000000E3U)
#define	DBSC4_SET_DBPDRGD0_DX2GCR3		(0x00008484U)
#define	DBSC4_SET_DBPDRGA0_DX3GCR3		(0x00000103U)
#define	DBSC4_SET_DBPDRGD0_DX3GCR3		(0x00008484U)
#define	RST_BASE				(0xE6160000U)
#define	RST_MODEMR				(RST_BASE + 0x0060U)
#define	RST_MODEMR_BIT0				(0x00000001U)

#define RCAR_CNTCR_OFF				(0x00U)
#define RCAR_CNTCVL_OFF				(0x08U)
#define RCAR_CNTCVU_OFF				(0x0CU)
#define RCAR_CNTFID_OFF				(0x20U)

#define RCAR_CNTCR_EN				((uint32_t)1U << 0U)
#define RCAR_CNTCR_FCREQ(x)			((uint32_t)(x) << 8U)

#if PMIC_ROHM_BD9571
#define	BIT_BKUP_CTRL_OUT			((uint8_t)(1U << 4))
#define	PMIC_BKUP_MODE_CNT			(0x20U)
#define	PMIC_QLLM_CNT				(0x27U)
#define	PMIC_RETRY_MAX				(100U)
#endif
#define	SCTLR_EL3_M_BIT				((uint32_t)1U << 0)
#define	RCAR_CA53CPU_NUM_MAX			(4U)
#define	RCAR_CA57CPU_NUM_MAX			(4U)
#define IS_A53A57(c) 	((c) == RCAR_CLUSTER_A53A57)
#define IS_CA57(c) 	((c) == RCAR_CLUSTER_CA57)
#define IS_CA53(c) 	((c) == RCAR_CLUSTER_CA53)

#ifndef __ASSEMBLY__
IMPORT_SYM(unsigned long, __system_ram_start__, SYSTEM_RAM_START);
IMPORT_SYM(unsigned long, __system_ram_end__, SYSTEM_RAM_END);
IMPORT_SYM(unsigned long, __SRAM_COPY_START__, SRAM_COPY_START);
#endif

uint32_t rcar_pwrc_status(uint64_t mpidr)
{
	uint32_t ret = 0;
	uint64_t cm, cpu;
	uint32_t reg;
	uint32_t c;

	rcar_lock_get();

	c = rcar_pwrc_get_cluster();
	cm = mpidr & MPIDR_CLUSTER_MASK;

	if (!IS_A53A57(c) && cm != 0) {
		ret = RCAR_INVALID;
		goto done;
	}

	reg = mmio_read_32(RCAR_PRR);
	cpu = mpidr & MPIDR_CPU_MASK;

	if (IS_CA53(c))
		if (reg & (1 << (STATE_CA53_CPU + cpu)))
			ret = RCAR_INVALID;
	if (IS_CA57(c))
		if (reg & (1 << (STATE_CA57_CPU + cpu)))
			ret = RCAR_INVALID;
done:
	rcar_lock_release();

	return ret;
}

static void scu_power_up(uint64_t mpidr)
{
	uintptr_t reg_pwrsr, reg_cpumcr, reg_pwron, reg_pwrer;
	uint32_t c, sysc_reg_bit;

	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	reg_cpumcr = IS_CA57(c) ? RCAR_CA57CPUCMCR : RCAR_CA53CPUCMCR;
	sysc_reg_bit = IS_CA57(c) ? BIT_CA57_SCU : BIT_CA53_SCU;
	reg_pwron = IS_CA57(c) ? RCAR_PWRONCR5 : RCAR_PWRONCR3;
	reg_pwrer = IS_CA57(c) ? RCAR_PWRER5 : RCAR_PWRER3;
	reg_pwrsr = IS_CA57(c) ? RCAR_PWRSR5 : RCAR_PWRSR3;

	if ((mmio_read_32(reg_pwrsr) & STATUS_PWRDOWN) == 0)
		return;

	if (mmio_read_32(reg_cpumcr) != 0)
		mmio_write_32(reg_cpumcr, 0);

	mmio_setbits_32(RCAR_SYSCIER, sysc_reg_bit);
	mmio_setbits_32(RCAR_SYSCIMR, sysc_reg_bit);

	do {
		while ((mmio_read_32(RCAR_SYSCSR) & REQ_RESUME) == 0)
			;
		mmio_write_32(reg_pwron, 1);
	} while (mmio_read_32(reg_pwrer) & 1);

	while ((mmio_read_32(RCAR_SYSCISR) & sysc_reg_bit) == 0)
		;
	mmio_write_32(RCAR_SYSCISR, sysc_reg_bit);
	while ((mmio_read_32(reg_pwrsr) & STATUS_PWRUP) == 0)
		;
}

void rcar_pwrc_cpuon(uint64_t mpidr)
{
	uint32_t res_data, on_data;
	uintptr_t res_reg, on_reg;
	uint32_t limit, c;
	uint64_t cpu;

	rcar_lock_get();

	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	res_reg = IS_CA53(c) ? RCAR_CA53RESCNT : RCAR_CA57RESCNT;
	on_reg = IS_CA53(c) ? RCAR_CA53WUPCR : RCAR_CA57WUPCR;
	limit = IS_CA53(c) ? 0x5A5A0000 : 0xA5A50000;

	res_data = mmio_read_32(res_reg) | limit;
	scu_power_up(mpidr);
	cpu = mpidr & MPIDR_CPU_MASK;
	on_data = 1 << cpu;
	mmio_write_32(RCAR_CPGWPR, ~on_data);
	mmio_write_32(on_reg, on_data);
	mmio_write_32(res_reg, res_data & (~(1 << (3 - cpu))));

	rcar_lock_release();
}

void rcar_pwrc_cpuoff(uint64_t mpidr)
{
	uint32_t c;
	uintptr_t reg;
	uint64_t cpu;

	rcar_lock_get();

	cpu = mpidr & MPIDR_CPU_MASK;
	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	reg = IS_CA53(c) ? RCAR_CA53CPU0CR : RCAR_CA57CPU0CR;

	if (read_mpidr_el1() != mpidr)
		panic();

	mmio_write_32(RCAR_CPGWPR, ~CPU_PWR_OFF);
	mmio_write_32(reg + cpu * 0x0010, CPU_PWR_OFF);

	rcar_lock_release();
}

void rcar_pwrc_enable_interrupt_wakeup(uint64_t mpidr)
{
	uint32_t c, shift_irq, shift_fiq;
	uintptr_t reg;
	uint64_t cpu;

	rcar_lock_get();

	cpu = mpidr & MPIDR_CPU_MASK;
	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	reg = IS_CA53(c) ? RCAR_WUPMSKCA53 : RCAR_WUPMSKCA57;

	shift_irq = WUP_IRQ_SHIFT + cpu;
	shift_fiq = WUP_FIQ_SHIFT + cpu;

	mmio_write_32(reg, ~((uint32_t) 1 << shift_irq) &
		      ~((uint32_t) 1 << shift_fiq));
	rcar_lock_release();
}

void rcar_pwrc_disable_interrupt_wakeup(uint64_t mpidr)
{
	uint32_t c, shift_irq, shift_fiq;
	uintptr_t reg;
	uint64_t cpu;

	rcar_lock_get();

	cpu = mpidr & MPIDR_CPU_MASK;
	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	reg = IS_CA53(c) ? RCAR_WUPMSKCA53 : RCAR_WUPMSKCA57;

	shift_irq = WUP_IRQ_SHIFT + cpu;
	shift_fiq = WUP_FIQ_SHIFT + cpu;

	mmio_write_32(reg, ((uint32_t) 1 << shift_irq) |
		      ((uint32_t) 1 << shift_fiq));
	rcar_lock_release();
}

void rcar_pwrc_clusteroff(uint64_t mpidr)
{
	uint32_t c, product, cut, reg;
	uintptr_t dst;

	rcar_lock_get();

	reg = mmio_read_32(RCAR_PRR);
	product = reg & RCAR_PRODUCT_MASK;
	cut = reg & RCAR_CUT_MASK;

	c = rcar_pwrc_get_mpidr_cluster(mpidr);
	dst = IS_CA53(c) ? RCAR_CA53CPUCMCR : RCAR_CA57CPUCMCR;

	if (RCAR_PRODUCT_M3 == product && cut < RCAR_CUT_VER30)
		goto done;

	if (RCAR_PRODUCT_H3 == product && cut <= RCAR_CUT_VER20)
		goto done;

	/* all of the CPUs in the cluster is in the CoreStandby mode */
	mmio_write_32(dst, MODE_L2_DOWN);
done:
	rcar_lock_release();
}

static uint64_t rcar_pwrc_saved_cntpct_el0;
static uint32_t rcar_pwrc_saved_cntfid;

#if RCAR_SYSTEM_SUSPEND
static void rcar_pwrc_save_timer_state(void)
{
	rcar_pwrc_saved_cntpct_el0 = read_cntpct_el0();

	rcar_pwrc_saved_cntfid =
		mmio_read_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTFID_OFF));
}
#endif

void rcar_pwrc_restore_timer_state(void)
{
	/* Stop timer before restoring counter value */
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCR_OFF), 0U);

	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCVL_OFF),
		(uint32_t)(rcar_pwrc_saved_cntpct_el0 & 0xFFFFFFFFU));
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCVU_OFF),
		(uint32_t)(rcar_pwrc_saved_cntpct_el0 >> 32U));

	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTFID_OFF),
		rcar_pwrc_saved_cntfid);

	/* Start generic timer back */
	write_cntfrq_el0((u_register_t)plat_get_syscnt_freq2());

	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCR_OFF),
		(RCAR_CNTCR_FCREQ(0U) | RCAR_CNTCR_EN));
}

#if !PMIC_ROHM_BD9571
void rcar_pwrc_system_reset(void)
{
	mmio_write_32(RCAR_SRESCR, 0x5AA50000U | BIT_SOFTRESET);
}
#endif /* PMIC_ROHM_BD9571 */

#define	RST_CA53_CPU0_BARH		(0xE6160080U)
#define	RST_CA53_CPU0_BARL		(0xE6160084U)
#define	RST_CA57_CPU0_BARH		(0xE61600C0U)
#define	RST_CA57_CPU0_BARL		(0xE61600C4U)

void rcar_pwrc_setup(void)
{
	uintptr_t rst_barh;
	uintptr_t rst_barl;
	uint32_t i, j;
	uint64_t reset = (uint64_t) (&plat_secondary_reset) & 0xFFFFFFFF;

	const uint32_t cluster[PLATFORM_CLUSTER_COUNT] = {
		RCAR_CLUSTER_CA53,
		RCAR_CLUSTER_CA57
	};
	const uintptr_t reg_barh[PLATFORM_CLUSTER_COUNT] = {
		RST_CA53_CPU0_BARH,
		RST_CA57_CPU0_BARH
	};
	const uintptr_t reg_barl[PLATFORM_CLUSTER_COUNT] = {
		RST_CA53_CPU0_BARL,
		RST_CA57_CPU0_BARL
	};

	for (i = 0; i < PLATFORM_CLUSTER_COUNT; i++) {
		rst_barh = reg_barh[i];
		rst_barl = reg_barl[i];
		for (j = 0; j < rcar_pwrc_get_cpu_num(cluster[i]); j++) {
			mmio_write_32(rst_barh, 0);
			mmio_write_32(rst_barl, (uint32_t) reset);
			rst_barh += 0x10;
			rst_barl += 0x10;
		}
	}

	rcar_lock_init();
}

#if RCAR_SYSTEM_SUSPEND
#define DBCAM_FLUSH(__bit)		\
do {					\
	;				\
} while (!(mmio_read_32(DBSC4_REG_DBCAM##__bit##STAT0) & DBSC4_BIT_DBCAMxSTAT0))


static void __attribute__ ((section(".system_ram")))
	rcar_pwrc_set_self_refresh(void)
{
	uint32_t reg = mmio_read_32(RCAR_PRR);
	uint32_t cut, product;

	product = reg & RCAR_PRODUCT_MASK;
	cut = reg & RCAR_CUT_MASK;

	if (product == RCAR_PRODUCT_M3 && cut < RCAR_CUT_VER30)
		goto self_refresh;

	if (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)
		goto self_refresh;

	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_ENABLE);

self_refresh:

	/* DFI_PHYMSTR_ACK setting */
	mmio_write_32(DBSC4_REG_DBDFIPMSTRCNF,
			mmio_read_32(DBSC4_REG_DBDFIPMSTRCNF) &
			(~DBSC4_BIT_DBDFIPMSTRCNF_PMSTREN));

	/* Set the Self-Refresh mode */
	mmio_write_32(DBSC4_REG_DBACEN, 0);

	if (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)
		rcar_micro_delay(100);
	else if (product == RCAR_PRODUCT_H3) {
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
		DBCAM_FLUSH(0);
		DBCAM_FLUSH(1);
		DBCAM_FLUSH(2);
		DBCAM_FLUSH(3);
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
	} else if (product == RCAR_PRODUCT_M3) {
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
		DBCAM_FLUSH(0);
		DBCAM_FLUSH(1);
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
	} else {
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
		DBCAM_FLUSH(0);
		mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
	}

	/* Set the SDRAM calibration configuration register */
	mmio_write_32(DBSC4_REG_DBCALCNF, 0);

	reg = DBSC4_SET_DBCMD_OPC_PRE | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ALL;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	/* Self-Refresh entry command   */
	reg = DBSC4_SET_DBCMD_OPC_SR | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	/* Mode Register Write command. (ODT disabled)  */
	reg = DBSC4_SET_DBCMD_OPC_MRW | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_MRW_ODTC;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	/* Power Down entry command     */
	reg = DBSC4_SET_DBCMD_OPC_PD | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	/* Set the auto-refresh enable register */
	mmio_write_32(DBSC4_REG_DBRFEN, 0U);
	rcar_micro_delay(1U);

	if (product == RCAR_PRODUCT_M3 && cut < RCAR_CUT_VER30)
		return;

	if (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)
		return;

	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_DISABLE);
}

static void __attribute__ ((section(".system_ram")))
    rcar_pwrc_set_self_refresh_e3(void)
{
	uint32_t ddr_md;
	uint32_t reg;

	ddr_md = (mmio_read_32(RST_MODEMR) >> 19) & RST_MODEMR_BIT0;

	/* Write enable */
	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_ENABLE);
	mmio_write_32(DBSC4_REG_DBACEN, 0);
	DBCAM_FLUSH(0);

	reg = DBSC4_SET_DBCMD_OPC_PRE | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ALL;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	reg = DBSC4_SET_DBCMD_OPC_SR | DBSC4_SET_DBCMD_CH_ALL |
	    DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
	mmio_write_32(DBSC4_REG_DBCMD, reg);
	while (mmio_read_32(DBSC4_REG_DBWAIT))
		;

	/* Set the auto-refresh enable register */
	/* Set the ARFEN bit to 0 in the DBRFEN */
	mmio_write_32(DBSC4_REG_DBRFEN, 0);

	mmio_write_32(DBSC4_REG_DBPDLK0, DBSC4_SET_DBPDLK0_PHY_ACCESS);

	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_ACIOCR0);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_ACIOCR0);

	/* DDR_DXCCR */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DXCCR);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DXCCR);

	/* DDR_PGCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_PGCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_PGCR1);

	/* DDR_ACIOCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_ACIOCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_ACIOCR1);

	/* DDR_ACIOCR3 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_ACIOCR3);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_ACIOCR3);

	/* DDR_ACIOCR5 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_ACIOCR5);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_ACIOCR5);

	/* DDR_DX0GCR2 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX0GCR2);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX0GCR2);

	/* DDR_DX1GCR2 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX1GCR2);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX1GCR2);

	/* DDR_DX2GCR2 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX2GCR2);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX2GCR2);

	/* DDR_DX3GCR2 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX3GCR2);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX3GCR2);

	/* DDR_ZQCR */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_ZQCR);

	mmio_write_32(DBSC4_REG_DBPDRGD0, ddr_md == 0 ?
		      DBSC4_SET_DBPDRGD0_ZQCR_MD19_0 :
		      DBSC4_SET_DBPDRGD0_ZQCR_MD19_1);

	/* DDR_DX0GCR0 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX0GCR0);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX0GCR0);

	/* DDR_DX1GCR0 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX1GCR0);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX1GCR0);

	/* DDR_DX2GCR0 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX2GCR0);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX2GCR0);

	/* DDR_DX3GCR0 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX3GCR0);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX3GCR0);

	/* DDR_DX0GCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX0GCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX0GCR1);

	/* DDR_DX1GCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX1GCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX1GCR1);

	/* DDR_DX2GCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX2GCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX2GCR1);

	/* DDR_DX3GCR1 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX3GCR1);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX3GCR1);

	/* DDR_DX0GCR3 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX0GCR3);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX0GCR3);

	/* DDR_DX1GCR3 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX1GCR3);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX1GCR3);

	/* DDR_DX2GCR3 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX2GCR3);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX2GCR3);

	/* DDR_DX3GCR3 */
	mmio_write_32(DBSC4_REG_DBPDRGA0, DBSC4_SET_DBPDRGA0_DX3GCR3);
	mmio_write_32(DBSC4_REG_DBPDRGD0, DBSC4_SET_DBPDRGD0_DX3GCR3);

	/* Write disable */
	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_DISABLE);
}

void __attribute__ ((section(".system_ram"))) __attribute__ ((noinline))
    rcar_pwrc_go_suspend_to_ram(void)
{
#if PMIC_ROHM_BD9571
	int32_t rc = -1, qllm = -1;
	uint8_t mode;
	uint32_t i;
#endif
	uint32_t reg, product;

	reg = mmio_read_32(RCAR_PRR);
	product = reg & RCAR_PRODUCT_MASK;

	if (product != RCAR_PRODUCT_E3)
		rcar_pwrc_set_self_refresh();
	else
		rcar_pwrc_set_self_refresh_e3();

#if PMIC_ROHM_BD9571
	/* Set QLLM Cnt Disable */
	for (i = 0; (i < PMIC_RETRY_MAX) && (qllm != 0); i++)
		qllm = rcar_iic_dvfs_send(PMIC, PMIC_QLLM_CNT, 0);

	/* Set trigger of power down to PMIV */
	for (i = 0; (i < PMIC_RETRY_MAX) && (rc != 0) && (qllm == 0); i++) {
		rc = rcar_iic_dvfs_receive(PMIC, PMIC_BKUP_MODE_CNT, &mode);
		if (rc == 0) {
			mode |= BIT_BKUP_CTRL_OUT;
			rc = rcar_iic_dvfs_send(PMIC, PMIC_BKUP_MODE_CNT, mode);
		}
	}
#endif
	wfi();

	while (1)
		;
}

void rcar_pwrc_set_suspend_to_ram(void)
{
	uintptr_t jump = (uintptr_t) &rcar_pwrc_go_suspend_to_ram;
	uintptr_t stack = (uintptr_t) (DEVICE_SRAM_STACK_BASE +
				       DEVICE_SRAM_STACK_SIZE);
	uint32_t sctlr;

	rcar_pwrc_save_timer_state();

	/* disable MMU */
	sctlr = (uint32_t) read_sctlr_el3();
	sctlr &= (uint32_t) ~SCTLR_EL3_M_BIT;
	write_sctlr_el3((uint64_t) sctlr);

	rcar_pwrc_switch_stack(jump, stack, NULL);
}

void rcar_pwrc_init_suspend_to_ram(void)
{
#if PMIC_ROHM_BD9571
	uint8_t mode;

	if (rcar_iic_dvfs_receive(PMIC, PMIC_BKUP_MODE_CNT, &mode))
		panic();

	mode &= (uint8_t) (~BIT_BKUP_CTRL_OUT);
	if (rcar_iic_dvfs_send(PMIC, PMIC_BKUP_MODE_CNT, mode))
		panic();
#endif
}

void rcar_pwrc_suspend_to_ram(void)
{
#if RCAR_SYSTEM_RESET_KEEPON_DDR
	int32_t error;

	error = rcar_iic_dvfs_send(PMIC, REG_KEEP10, 0);
	if (error) {
		ERROR("Failed send KEEP10 init ret=%d \n", error);
		return;
	}
#endif
	rcar_pwrc_set_suspend_to_ram();
}
#endif

void rcar_pwrc_code_copy_to_system_ram(void)
{
	int ret __attribute__ ((unused));	/* in assert */
	uint32_t attr;
	struct device_sram_t {
		uintptr_t base;
		size_t len;
	} sram = {
		.base = (uintptr_t) DEVICE_SRAM_BASE,
		.len = DEVICE_SRAM_SIZE,
	};
	struct ddr_code_t {
		void *base;
		size_t len;
	} code = {
		.base = (void *) SRAM_COPY_START,
		.len = SYSTEM_RAM_END - SYSTEM_RAM_START,
	};

	attr = MT_MEMORY | MT_RW | MT_SECURE | MT_EXECUTE_NEVER;
	ret = xlat_change_mem_attributes(sram.base, sram.len, attr);
	assert(ret == 0);

	memcpy((void *)sram.base, code.base, code.len);
	flush_dcache_range((uint64_t) sram.base, code.len);

	/* Invalidate instruction cache */
	plat_invalidate_icache();
	dsb();
	isb();

	attr = MT_MEMORY | MT_RO | MT_SECURE | MT_EXECUTE;
	ret = xlat_change_mem_attributes(sram.base, sram.len, attr);
	assert(ret == 0);
}

uint32_t rcar_pwrc_get_cluster(void)
{
	uint32_t reg;

	reg = mmio_read_32(RCAR_PRR);

	if (reg & (1 << (STATE_CA53_CPU + RCAR_CA53CPU_NUM_MAX)))
		return RCAR_CLUSTER_CA57;

	if (reg & (1 << (STATE_CA57_CPU + RCAR_CA57CPU_NUM_MAX)))
		return RCAR_CLUSTER_CA53;

	return RCAR_CLUSTER_A53A57;
}

uint32_t rcar_pwrc_get_mpidr_cluster(uint64_t mpidr)
{
	uint32_t c = rcar_pwrc_get_cluster();

	if (IS_A53A57(c)) {
		if (mpidr & MPIDR_CLUSTER_MASK)
			return RCAR_CLUSTER_CA53;

		return RCAR_CLUSTER_CA57;
	}

	return c;
}

#if RCAR_LSI == RCAR_D3
uint32_t rcar_pwrc_get_cpu_num(uint32_t c)
{
	return 1;
}
#else
uint32_t rcar_pwrc_get_cpu_num(uint32_t c)
{
	uint32_t reg = mmio_read_32(RCAR_PRR);
	uint32_t count = 0, i;

	if (IS_A53A57(c) || IS_CA53(c)) {
		if (reg & (1 << (STATE_CA53_CPU + RCAR_CA53CPU_NUM_MAX)))
			goto count_ca57;

		for (i = 0; i < RCAR_CA53CPU_NUM_MAX; i++) {
			if (reg & (1 << (STATE_CA53_CPU + i)))
				continue;
			count++;
		}
	}

count_ca57:
	if (IS_A53A57(c) || IS_CA57(c)) {
		if (reg & (1 << (STATE_CA57_CPU + RCAR_CA57CPU_NUM_MAX)))
			goto done;

		for (i = 0; i < RCAR_CA57CPU_NUM_MAX; i++) {
			if (reg & (1 << (STATE_CA57_CPU + i)))
				continue;
			count++;
		}
	}

done:
	return count;
}
#endif

int32_t rcar_pwrc_cpu_on_check(uint64_t mpidr)
{
	uint64_t i;
	uint64_t j;
	uint64_t cpu_count;
	uintptr_t reg_PSTR;
	uint32_t status;
	uint64_t my_cpu;
	int32_t rtn;
	uint32_t my_cluster_type;

	const uint32_t cluster_type[PLATFORM_CLUSTER_COUNT] = {
			RCAR_CLUSTER_CA53,
			RCAR_CLUSTER_CA57
	};
	const uintptr_t registerPSTR[PLATFORM_CLUSTER_COUNT] = {
			RCAR_CA53PSTR,
			RCAR_CA57PSTR
	};

	my_cluster_type = rcar_pwrc_get_cluster();

	rtn = 0;
	my_cpu = mpidr & ((uint64_t)(MPIDR_CPU_MASK));
	for (i = 0U; i < ((uint64_t)(PLATFORM_CLUSTER_COUNT)); i++) {
		cpu_count = rcar_pwrc_get_cpu_num(cluster_type[i]);
		reg_PSTR = registerPSTR[i];
		for (j = 0U; j < cpu_count; j++) {
			if ((my_cluster_type != cluster_type[i]) || (my_cpu != j)) {
				status = mmio_read_32(reg_PSTR) >> (j * 4U);
				if ((status & 0x00000003U) == 0U) {
					rtn--;
				}
			}
		}
	}
	return (rtn);

}

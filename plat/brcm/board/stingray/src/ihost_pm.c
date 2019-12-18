/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <dmu.h>
#include <ihost_pm.h>
#include <platform_def.h>

#define CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST1			2
#define CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST2			1
#define CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST3			0
#define CDRU_MISC_RESET_CONTROL__CDRU_IH1_RESET				9
#define CDRU_MISC_RESET_CONTROL__CDRU_IH2_RESET				8
#define CDRU_MISC_RESET_CONTROL__CDRU_IH3_RESET				7
#define A72_CRM_SOFTRESETN_0						0x480
#define A72_CRM_SOFTRESETN_1						0x484
#define A72_CRM_DOMAIN_4_CONTROL					0x810
#define A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_DFT			3
#define A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_MEM			6
#define A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_I_O			0
#define A72_CRM_SUBSYSTEM_MEMORY_CONTROL_3				0xB4C
#define MEMORY_PDA_HI_SHIFT						0x0
#define A72_CRM_PLL_PWR_ON						0x70
#define A72_CRM_PLL_PWR_ON__PLL0_ISO_PLLOUT				4
#define A72_CRM_PLL_PWR_ON__PLL0_PWRON_LDO				1
#define A72_CRM_PLL_PWR_ON__PLL0_PWRON_PLL				0
#define A72_CRM_SUBSYSTEM_MEMORY_CONTROL_2				0xB48
#define A72_CRM_PLL_INTERRUPT_STATUS					0x8c
#define A72_CRM_PLL_INTERRUPT_STATUS__PLL0_LOCK_LOST_STATUS		8
#define A72_CRM_PLL_INTERRUPT_STATUS__PLL0_LOCK_STATUS			9
#define A72_CRM_INTERRUPT_ENABLE					0x4
#define A72_CRM_INTERRUPT_ENABLE__PLL0_INT_ENABLE			4
#define A72_CRM_PLL_INTERRUPT_ENABLE					0x88
#define A72_CRM_PLL_INTERRUPT_ENABLE__PLL0_LOCK_STATUS_INT_ENB		9
#define A72_CRM_PLL_INTERRUPT_ENABLE__PLL0_LOCK_LOST_STATUS_INT_ENB	8
#define A72_CRM_PLL0_CFG0_CTRL						0x120
#define A72_CRM_PLL0_CFG1_CTRL						0x124
#define A72_CRM_PLL0_CFG2_CTRL						0x128
#define A72_CRM_PLL0_CFG3_CTRL						0x12C
#define A72_CRM_CORE_CONFIG_DBGCTRL__DBGROMADDRV			0
#define A72_CRM_CORE_CONFIG_DBGCTRL					0xD50
#define A72_CRM_CORE_CONFIG_DBGROM_LO					0xD54
#define A72_CRM_CORE_CONFIG_DBGROM_HI					0xD58
#define A72_CRM_SUBSYSTEM_CONFIG_1__DBGL1RSTDISABLE			2
#define A72_CRM_SOFTRESETN_0__CRYSTAL26_SOFTRESETN			0
#define A72_CRM_SOFTRESETN_0__CRM_PLL0_SOFTRESETN			1
#define A72_CRM_AXI_CLK_DESC						0x304
#define A72_CRM_ACP_CLK_DESC						0x308
#define A72_CRM_ATB_CLK_DESC						0x30C
#define A72_CRM_PCLKDBG_DESC						0x310
#define A72_CRM_CLOCK_MODE_CONTROL					0x40
#define A72_CRM_CLOCK_MODE_CONTROL__CLK_CHANGE_TRIGGER			0
#define A72_CRM_CLOCK_CONTROL_0						0x200
#define A72_CRM_CLOCK_CONTROL_0__ARM_HW_SW_ENABLE_SEL			0
#define A72_CRM_CLOCK_CONTROL_0__AXI_HW_SW_ENABLE_SEL			2
#define A72_CRM_CLOCK_CONTROL_0__ACP_HW_SW_ENABLE_SEL			4
#define A72_CRM_CLOCK_CONTROL_0__ATB_HW_SW_ENABLE_SEL			6
#define A72_CRM_CLOCK_CONTROL_0__PCLKDBG_HW_SW_ENA_SEL			8
#define A72_CRM_CLOCK_CONTROL_1						0x204
#define A72_CRM_CLOCK_CONTROL_1__TMON_HW_SW_ENABLE_SEL			6
#define A72_CRM_CLOCK_CONTROL_1__APB_HW_SW_ENABLE_SEL			8
#define A72_CRM_SOFTRESETN_0__CRYSTAL26_SOFTRESETN			0
#define A72_CRM_SOFTRESETN_0__CRM_PLL0_SOFTRESETN			1
#define A72_CRM_SOFTRESETN_0__AXI_SOFTRESETN				9
#define A72_CRM_SOFTRESETN_0__ACP_SOFTRESETN				10
#define A72_CRM_SOFTRESETN_0__ATB_SOFTRESETN				11
#define A72_CRM_SOFTRESETN_0__PCLKDBG_SOFTRESETN			12
#define A72_CRM_SOFTRESETN_0__TMON_SOFTRESETN				15
#define A72_CRM_SOFTRESETN_0__L2_SOFTRESETN				3
#define A72_CRM_SOFTRESETN_1__APB_SOFTRESETN				8

/* core related regs */
#define A72_CRM_DOMAIN_0_CONTROL					0x800
#define A72_CRM_DOMAIN_0_CONTROL__DOMAIN_0_ISO_MEM			0x6
#define A72_CRM_DOMAIN_0_CONTROL__DOMAIN_0_ISO_I_O			0x0
#define A72_CRM_DOMAIN_1_CONTROL					0x804
#define A72_CRM_DOMAIN_1_CONTROL__DOMAIN_1_ISO_MEM			0x6
#define A72_CRM_DOMAIN_1_CONTROL__DOMAIN_1_ISO_I_O			0x0
#define A72_CRM_CORE_CONFIG_RVBA0_LO					0xD10
#define A72_CRM_CORE_CONFIG_RVBA0_MID					0xD14
#define A72_CRM_CORE_CONFIG_RVBA0_HI					0xD18
#define A72_CRM_CORE_CONFIG_RVBA1_LO					0xD20
#define A72_CRM_CORE_CONFIG_RVBA1_MID					0xD24
#define A72_CRM_CORE_CONFIG_RVBA1_HI					0xD28
#define A72_CRM_SUBSYSTEM_CONFIG_0					0xC80
#define A72_CRM_SUBSYSTEM_CONFIG_0__DBGPWRDUP_CFG_SHIFT			4
#define A72_CRM_SOFTRESETN_0__COREPOR0_SOFTRESETN			4
#define A72_CRM_SOFTRESETN_0__COREPOR1_SOFTRESETN			5
#define A72_CRM_SOFTRESETN_1__CORE0_SOFTRESETN				0
#define A72_CRM_SOFTRESETN_1__DEBUG0_SOFTRESETN				4
#define A72_CRM_SOFTRESETN_1__CORE1_SOFTRESETN				1
#define A72_CRM_SOFTRESETN_1__DEBUG1_SOFTRESETN				5

#define SPROC_MEMORY_BISR 0

static int cluster_power_status[PLAT_BRCM_CLUSTER_COUNT] = {CLUSTER_POWER_ON,
							   CLUSTER_POWER_OFF,
							   CLUSTER_POWER_OFF,
							   CLUSTER_POWER_OFF};

void ihost_power_on_cluster(u_register_t mpidr)
{
	uint32_t rst, d2xs;
	uint32_t cluster_id;
	uint32_t ihost_base;
#if SPROC_MEMORY_BISR
	uint32_t bisr, cnt;
#endif
	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t cluster0_freq_sel;

	if (cluster_power_status[cluster_id] == CLUSTER_POWER_ON)
		return;

	cluster_power_status[cluster_id] = CLUSTER_POWER_ON;
	INFO("enabling Cluster #%u\n", cluster_id);

	switch (cluster_id) {
	case 1:
		rst = (1 << CDRU_MISC_RESET_CONTROL__CDRU_IH1_RESET);
		d2xs = (1 << CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST1);
#if SPROC_MEMORY_BISR
		bisr = CRMU_BISR_PDG_MASK__CRMU_BISR_IHOST1;
#endif
		break;
	case 2:
		rst = (1 << CDRU_MISC_RESET_CONTROL__CDRU_IH2_RESET);
		d2xs = (1 << CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST2);
#if SPROC_MEMORY_BISR
		bisr = CRMU_BISR_PDG_MASK__CRMU_BISR_IHOST2;
#endif
		break;
	case 3:
		rst = (1 << CDRU_MISC_RESET_CONTROL__CDRU_IH3_RESET);
		d2xs = (1 << CDRU_CCN_REGISTER_CONTROL_1__D2XS_PD_IHOST3);
#if SPROC_MEMORY_BISR
		bisr = CRMU_BISR_PDG_MASK__CRMU_BISR_IHOST3;
#endif
		break;
	default:
		ERROR("Invalid cluster :%u\n", cluster_id);
		return;
	}

	/* Releasing ihost resets */
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL, rst);

	/* calculate cluster/ihost base address */
	ihost_base = IHOST0_BASE + cluster_id * IHOST_ADDR_SPACE;

	/* Remove Cluster IO isolation */
	mmio_clrsetbits_32(ihost_base + A72_CRM_DOMAIN_4_CONTROL,
		       (1 << A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_I_O),
		       (1 << A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_DFT) |
		       (1 << A72_CRM_DOMAIN_4_CONTROL__DOMAIN_4_ISO_MEM));

	/*
	 * Since BISR sequence requires that all cores of cluster should
	 * have removed I/O isolation hence doing same here.
	 */
	/* Remove core0 memory IO isolations */
	mmio_clrsetbits_32(ihost_base + A72_CRM_DOMAIN_0_CONTROL,
			  (1 << A72_CRM_DOMAIN_0_CONTROL__DOMAIN_0_ISO_I_O),
			  (1 << A72_CRM_DOMAIN_0_CONTROL__DOMAIN_0_ISO_MEM));

	/* Remove core1 memory IO isolations */
	mmio_clrsetbits_32(ihost_base + A72_CRM_DOMAIN_1_CONTROL,
			  (1 << A72_CRM_DOMAIN_1_CONTROL__DOMAIN_1_ISO_I_O),
			  (1 << A72_CRM_DOMAIN_1_CONTROL__DOMAIN_1_ISO_MEM));

#if SPROC_MEMORY_BISR
	mmio_setbits_32(CRMU_BISR_PDG_MASK, (1 << bisr));

	if (!(mmio_read_32(CDRU_CHIP_STRAP_DATA_LSW) &
		       (1 << CDRU_CHIP_STRAP_DATA_LSW__BISR_BYPASS_MODE))) {
		/* BISR completion would take max 2 usec */
		cnt = 0;
		while (cnt < 2) {
			udelay(1);
			if (mmio_read_32(CRMU_CHIP_OTPC_STATUS) &
			(1 << CRMU_CHIP_OTPC_STATUS__OTP_BISR_LOAD_DONE))
				break;
			cnt++;
		}
	}

	/* if BISR is not completed, need to be checked with ASIC team */
	if (((mmio_read_32(CRMU_CHIP_OTPC_STATUS)) &
	   (1 << CRMU_CHIP_OTPC_STATUS__OTP_BISR_LOAD_DONE)) == 0) {
		WARN("BISR did not completed and need to be addressed\n");
	}
#endif

	/* PLL Power up. supply is already on. Turn on PLL LDO/PWR */
	mmio_write_32(ihost_base + A72_CRM_PLL_PWR_ON,
		     (1 << A72_CRM_PLL_PWR_ON__PLL0_ISO_PLLOUT) |
		     (1 << A72_CRM_PLL_PWR_ON__PLL0_PWRON_LDO) |
		     (1 << A72_CRM_PLL_PWR_ON__PLL0_PWRON_PLL));

	/* 1us in spec; Doubling it to be safe*/
	udelay(2);

	/* Remove PLL output ISO */
	mmio_write_32(ihost_base + A72_CRM_PLL_PWR_ON,
		     (1 << A72_CRM_PLL_PWR_ON__PLL0_PWRON_LDO) |
		     (1 << A72_CRM_PLL_PWR_ON__PLL0_PWRON_PLL));

	/*
	 * PLL0 Configuration Control Register
	 * these 4 registers drive the i_pll_ctrl[63:0] input of pll
	 * (16b per register).
	 * the values are derived from the spec (sections 8 and 10).
	 */

	mmio_write_32(ihost_base + A72_CRM_PLL0_CFG0_CTRL, 0x00000000);
	mmio_write_32(ihost_base + A72_CRM_PLL0_CFG1_CTRL, 0x00008400);
	mmio_write_32(ihost_base + A72_CRM_PLL0_CFG2_CTRL, 0x00000001);
	mmio_write_32(ihost_base + A72_CRM_PLL0_CFG3_CTRL, 0x00000000);

	/* Read the freq_sel from cluster 0, which is up already */
	cluster0_freq_sel = bcm_get_ihost_pll_freq(0);
	bcm_set_ihost_pll_freq(cluster_id, cluster0_freq_sel);

	udelay(1);

	/* Release clock source reset */
	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_0,
		       (1 << A72_CRM_SOFTRESETN_0__CRYSTAL26_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__CRM_PLL0_SOFTRESETN));

	udelay(1);

	/*
	 * Integer division for clks (divider value = n+1).
	 * These are the divisor of ARM PLL clock frequecy.
	 */
	mmio_write_32(ihost_base + A72_CRM_AXI_CLK_DESC, 0x00000001);
	mmio_write_32(ihost_base + A72_CRM_ACP_CLK_DESC, 0x00000001);
	mmio_write_32(ihost_base + A72_CRM_ATB_CLK_DESC, 0x00000004);
	mmio_write_32(ihost_base + A72_CRM_PCLKDBG_DESC, 0x0000000b);

	/*
	 * clock change trigger - must set to take effect after clock
	 * source change
	 */
	mmio_setbits_32(ihost_base + A72_CRM_CLOCK_MODE_CONTROL,
		       (1 << A72_CRM_CLOCK_MODE_CONTROL__CLK_CHANGE_TRIGGER));

	/* turn on functional clocks */
	mmio_setbits_32(ihost_base + A72_CRM_CLOCK_CONTROL_0,
		       (3 << A72_CRM_CLOCK_CONTROL_0__ARM_HW_SW_ENABLE_SEL) |
		       (3 << A72_CRM_CLOCK_CONTROL_0__AXI_HW_SW_ENABLE_SEL) |
		       (3 << A72_CRM_CLOCK_CONTROL_0__ACP_HW_SW_ENABLE_SEL) |
		       (3 << A72_CRM_CLOCK_CONTROL_0__ATB_HW_SW_ENABLE_SEL) |
		       (3 << A72_CRM_CLOCK_CONTROL_0__PCLKDBG_HW_SW_ENA_SEL));

	mmio_setbits_32(ihost_base + A72_CRM_CLOCK_CONTROL_1,
		       (3 << A72_CRM_CLOCK_CONTROL_1__TMON_HW_SW_ENABLE_SEL) |
		       (3 << A72_CRM_CLOCK_CONTROL_1__APB_HW_SW_ENABLE_SEL));

	/* Program D2XS Power Down Registers */
	mmio_setbits_32(CDRU_CCN_REGISTER_CONTROL_1, d2xs);

	/* Program Core Config Debug ROM Address Registers */
	/* mark valid for Debug ROM base address */
	mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_DBGCTRL,
		     (1 << A72_CRM_CORE_CONFIG_DBGCTRL__DBGROMADDRV));

	/* Program Lo and HI address of coresight DBG rom address */
	mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_DBGROM_LO,
		     (CORESIGHT_BASE_ADDR >> 12) & 0xffff);
	mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_DBGROM_HI,
		     (CORESIGHT_BASE_ADDR >> 28) & 0xffff);

	/*
	 * Release soft resets of different components.
	 * Order: Bus clocks --> PERIPH --> L2 --> cores
	 */

	/* Bus clocks soft resets */
	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_0,
		       (1 << A72_CRM_SOFTRESETN_0__CRYSTAL26_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__CRM_PLL0_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__AXI_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__ACP_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__ATB_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_0__PCLKDBG_SOFTRESETN));

	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_1,
		       (1 << A72_CRM_SOFTRESETN_1__APB_SOFTRESETN));

	/* Periph component softreset */
	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_0,
		       (1 << A72_CRM_SOFTRESETN_0__TMON_SOFTRESETN));

	/* L2 softreset */
	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_0,
		       (1 << A72_CRM_SOFTRESETN_0__L2_SOFTRESETN));

	/* Enable and program Satellite timer */
	ihost_enable_satellite_timer(cluster_id);
}

void ihost_power_on_secondary_core(u_register_t mpidr, uint64_t rvbar)
{
	uint32_t ihost_base;
	uint32_t coreid = MPIDR_AFFLVL0_VAL(mpidr);
	uint32_t cluster_id = MPIDR_AFFLVL1_VAL(mpidr);

	ihost_base = IHOST0_BASE + cluster_id * IHOST_ADDR_SPACE;
	INFO("programming core #%u\n", coreid);

	if (coreid) {
		/* program the entry point for core1 */
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA1_LO,
			      rvbar & 0xFFFF);
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA1_MID,
			     (rvbar >> 16) & 0xFFFF);
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA1_HI,
			     (rvbar >> 32) & 0xFFFF);
	} else {
		/* program the entry point for core */
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA0_LO,
			      rvbar & 0xFFFF);
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA0_MID,
			     (rvbar >> 16) & 0xFFFF);
		mmio_write_32(ihost_base + A72_CRM_CORE_CONFIG_RVBA0_HI,
			     (rvbar >> 32) & 0xFFFF);
	}

	/* Tell debug logic which processor is up */
	mmio_setbits_32(ihost_base + A72_CRM_SUBSYSTEM_CONFIG_0,
		       (coreid ?
		       (2 << A72_CRM_SUBSYSTEM_CONFIG_0__DBGPWRDUP_CFG_SHIFT) :
		       (1 << A72_CRM_SUBSYSTEM_CONFIG_0__DBGPWRDUP_CFG_SHIFT)));

	/* releasing soft resets for IHOST core */
	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_0,
		       (coreid ?
		       (1 << A72_CRM_SOFTRESETN_0__COREPOR1_SOFTRESETN) :
		       (1 << A72_CRM_SOFTRESETN_0__COREPOR0_SOFTRESETN)));

	mmio_setbits_32(ihost_base + A72_CRM_SOFTRESETN_1,
		       (coreid ?
		       ((1 << A72_CRM_SOFTRESETN_1__CORE1_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_1__DEBUG1_SOFTRESETN)) :
		       ((1 << A72_CRM_SOFTRESETN_1__CORE0_SOFTRESETN) |
		       (1 << A72_CRM_SOFTRESETN_1__DEBUG0_SOFTRESETN))));
}

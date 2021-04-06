/*
 * Copyright (C) 2018-2020 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#ifdef USE_CCI
#include <drivers/arm/cci.h>
#endif
#include <lib/psci/psci.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <a3700_pm.h>
#include <arch_helpers.h>
#include <armada_common.h>
#include <dram_win.h>
#include <io_addr_dec.h>
#include <mvebu.h>
#include <mvebu_def.h>
#include <marvell_plat_priv.h>
#include <plat_marvell.h>

/* Warm reset register */
#define MVEBU_WARM_RESET_REG		(MVEBU_NB_REGS_BASE + 0x840)
#define MVEBU_WARM_RESET_MAGIC		0x1D1E

/* North Bridge GPIO1 SEL register */
#define MVEBU_NB_GPIO1_SEL_REG		(MVEBU_NB_REGS_BASE + 0x830)
 #define MVEBU_NB_GPIO1_UART1_SEL	BIT(19)
 #define MVEBU_NB_GPIO1_GPIO_25_26_EN	BIT(17)
 #define MVEBU_NB_GPIO1_GPIO_19_EN	BIT(14)
 #define MVEBU_NB_GPIO1_GPIO_18_EN	BIT(13)

/* CPU 1 reset register */
#define MVEBU_CPU_1_RESET_VECTOR	(MVEBU_REGS_BASE + 0x14044)
#define MVEBU_CPU_1_RESET_REG		(MVEBU_REGS_BASE + 0xD00C)
#define MVEBU_CPU_1_RESET_BIT		31

/* IRQ register */
#define MVEBU_NB_IRQ_STATUS_1_REG		(MVEBU_NB_SB_IRQ_REG_BASE)
#define MVEBU_NB_IRQ_STATUS_2_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0x10)
#define MVEBU_NB_IRQ_MASK_2_REG			(MVEBU_NB_SB_IRQ_REG_BASE + \
						0x18)
#define MVEBU_SB_IRQ_STATUS_1_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0x40)
#define MVEBU_SB_IRQ_STATUS_2_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0x50)
#define MVEBU_NB_GPIO_IRQ_MASK_1_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0xC8)
#define MVEBU_NB_GPIO_IRQ_MASK_2_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0xD8)
#define MVEBU_SB_GPIO_IRQ_MASK_REG		(MVEBU_NB_SB_IRQ_REG_BASE + \
						0xE8)
#define MVEBU_NB_GPIO_IRQ_EN_LOW_REG		(MVEBU_NB_GPIO_IRQ_REG_BASE)
#define MVEBU_NB_GPIO_IRQ_EN_HIGH_REG		(MVEBU_NB_GPIO_IRQ_REG_BASE + \
						0x04)
#define MVEBU_NB_GPIO_IRQ_STATUS_LOW_REG	(MVEBU_NB_GPIO_IRQ_REG_BASE + \
						0x10)
#define MVEBU_NB_GPIO_IRQ_STATUS_HIGH_REG	(MVEBU_NB_GPIO_IRQ_REG_BASE + \
						0x14)
#define MVEBU_NB_GPIO_IRQ_WK_LOW_REG		(MVEBU_NB_GPIO_IRQ_REG_BASE + \
						0x18)
#define MVEBU_NB_GPIO_IRQ_WK_HIGH_REG		(MVEBU_NB_GPIO_IRQ_REG_BASE + \
						0x1C)
#define MVEBU_SB_GPIO_IRQ_EN_REG		(MVEBU_SB_GPIO_IRQ_REG_BASE)
#define MVEBU_SB_GPIO_IRQ_STATUS_REG		(MVEBU_SB_GPIO_IRQ_REG_BASE + \
						0x10)
#define MVEBU_SB_GPIO_IRQ_WK_REG		(MVEBU_SB_GPIO_IRQ_REG_BASE + \
						0x18)

/* PMU registers */
#define MVEBU_PM_NB_PWR_CTRL_REG	(MVEBU_PMSU_REG_BASE)
 #define MVEBU_PM_PWR_DN_CNT_SEL	BIT(28)
 #define MVEBU_PM_SB_PWR_DWN		BIT(4)
 #define MVEBU_PM_INTERFACE_IDLE	BIT(0)
#define MVEBU_PM_NB_CPU_PWR_CTRL_REG	(MVEBU_PMSU_REG_BASE + 0x4)
 #define MVEBU_PM_L2_FLUSH_EN		BIT(22)
#define MVEBU_PM_NB_PWR_OPTION_REG	(MVEBU_PMSU_REG_BASE + 0x8)
 #define MVEBU_PM_DDR_SR_EN		BIT(29)
 #define MVEBU_PM_DDR_CLK_DIS_EN	BIT(28)
 #define MVEBU_PM_WARM_RESET_EN		BIT(27)
 #define MVEBU_PM_DDRPHY_PWRDWN_EN	BIT(23)
 #define MVEBU_PM_DDRPHY_PAD_PWRDWN_EN	BIT(22)
 #define MVEBU_PM_OSC_OFF_EN		BIT(21)
 #define MVEBU_PM_TBG_OFF_EN		BIT(20)
 #define MVEBU_PM_CPU_VDDV_OFF_EN	BIT(19)
 #define MVEBU_PM_AVS_DISABLE_MODE	BIT(14)
 #define MVEBU_PM_AVS_VDD2_MODE		BIT(13)
 #define MVEBU_PM_AVS_HOLD_MODE		BIT(12)
 #define MVEBU_PM_L2_SRAM_LKG_PD_EN	BIT(8)
 #define MVEBU_PM_EIP_SRAM_LKG_PD_EN	BIT(7)
 #define MVEBU_PM_DDRMC_SRAM_LKG_PD_EN	BIT(6)
 #define MVEBU_PM_MCI_SRAM_LKG_PD_EN	BIT(5)
 #define MVEBU_PM_MMC_SRAM_LKG_PD_EN	BIT(4)
 #define MVEBU_PM_SATA_SRAM_LKG_PD_EN	BIT(3)
 #define MVEBU_PM_DMA_SRAM_LKG_PD_EN	BIT(2)
 #define MVEBU_PM_SEC_SRAM_LKG_PD_EN	BIT(1)
 #define MVEBU_PM_CPU_SRAM_LKG_PD_EN	BIT(0)
 #define MVEBU_PM_NB_SRAM_LKG_PD_EN	(MVEBU_PM_L2_SRAM_LKG_PD_EN |\
	MVEBU_PM_EIP_SRAM_LKG_PD_EN | MVEBU_PM_DDRMC_SRAM_LKG_PD_EN |\
	MVEBU_PM_MCI_SRAM_LKG_PD_EN | MVEBU_PM_MMC_SRAM_LKG_PD_EN |\
	MVEBU_PM_SATA_SRAM_LKG_PD_EN | MVEBU_PM_DMA_SRAM_LKG_PD_EN |\
	MVEBU_PM_SEC_SRAM_LKG_PD_EN | MVEBU_PM_CPU_SRAM_LKG_PD_EN)
#define MVEBU_PM_NB_PWR_DEBUG_REG	(MVEBU_PMSU_REG_BASE + 0xC)
 #define MVEBU_PM_NB_FORCE_CLK_ON	BIT(30)
 #define MVEBU_PM_IGNORE_CM3_SLEEP	BIT(21)
 #define MVEBU_PM_IGNORE_CM3_DEEP	BIT(20)
#define MVEBU_PM_NB_WAKE_UP_EN_REG	(MVEBU_PMSU_REG_BASE + 0x2C)
 #define MVEBU_PM_SB_WKP_NB_EN		BIT(31)
 #define MVEBU_PM_NB_GPIO_WKP_EN	BIT(27)
 #define MVEBU_PM_SOC_TIMER_WKP_EN	BIT(26)
 #define MVEBU_PM_UART_WKP_EN		BIT(25)
 #define MVEBU_PM_UART2_WKP_EN		BIT(19)
 #define MVEBU_PM_CPU_TIMER_WKP_EN	BIT(17)
 #define MVEBU_PM_NB_WKP_EN		BIT(16)
 #define MVEBU_PM_CORE1_FIQ_IRQ_WKP_EN	BIT(13)
 #define MVEBU_PM_CORE0_FIQ_IRQ_WKP_EN	BIT(12)
#define MVEBU_PM_CPU_0_PWR_CTRL_REG	(MVEBU_PMSU_REG_BASE + 0x34)
#define MVEBU_PM_CPU_1_PWR_CTRL_REG	(MVEBU_PMSU_REG_BASE + 0x38)
 #define MVEBU_PM_CORE_SOC_PD		BIT(2)
 #define MVEBU_PM_CORE_PROC_PD		BIT(1)
 #define MVEBU_PM_CORE_PD		BIT(0)
#define MVEBU_PM_CORE_1_RETURN_ADDR_REG	(MVEBU_PMSU_REG_BASE + 0x44)
#define MVEBU_PM_CPU_VDD_OFF_INFO_1_REG	(MVEBU_PMSU_REG_BASE + 0x48)
#define MVEBU_PM_CPU_VDD_OFF_INFO_2_REG	(MVEBU_PMSU_REG_BASE + 0x4C)
 #define MVEBU_PM_LOW_POWER_STATE	BIT(0)
#define MVEBU_PM_CPU_WAKE_UP_CONF_REG	(MVEBU_PMSU_REG_BASE + 0x54)
 #define MVEBU_PM_CORE1_WAKEUP		BIT(13)
 #define MVEBU_PM_CORE0_WAKEUP		BIT(12)
#define MVEBU_PM_WAIT_DDR_RDY_VALUE	(0x15)
#define MVEBU_PM_SB_CPU_PWR_CTRL_REG	(MVEBU_SB_WAKEUP_REG_BASE)
  #define MVEBU_PM_SB_PM_START		BIT(0)
#define MVEBU_PM_SB_PWR_OPTION_REG	(MVEBU_SB_WAKEUP_REG_BASE + 0x4)
  #define MVEBU_PM_SDIO_PHY_PDWN_EN	BIT(17)
  #define MVEBU_PM_SB_VDDV_OFF_EN	BIT(16)
  #define MVEBU_PM_EBM_SRAM_LKG_PD_EN		BIT(11)
  #define MVEBU_PM_PCIE_SRAM_LKG_PD_EN		BIT(10)
  #define MVEBU_PM_GBE1_TX_SRAM_LKG_PD_EN	BIT(9)
  #define MVEBU_PM_GBE1_RX_SRAM_LKG_PD_EN	BIT(8)
  #define MVEBU_PM_GBE1_MIB_SRAM_LKG_PD_EN	BIT(7)
  #define MVEBU_PM_GBE0_TX_SRAM_LKG_PD_EN	BIT(6)
  #define MVEBU_PM_GBE0_RX_SRAM_LKG_PD_EN	BIT(5)
  #define MVEBU_PM_GBE0_MIB_SRAM_LKG_PD_EN	BIT(4)
  #define MVEBU_PM_SDIO_SRAM_LKG_PD_EN		BIT(3)
  #define MVEBU_PM_USB2_SRAM_LKG_PD_EN		BIT(2)
  #define MVEBU_PM_USB3_H_SRAM_LKG_PD_EN	BIT(1)
  #define MVEBU_PM_SB_SRAM_LKG_PD_EN	(MVEBU_PM_EBM_SRAM_LKG_PD_EN |\
	MVEBU_PM_PCIE_SRAM_LKG_PD_EN | MVEBU_PM_GBE1_TX_SRAM_LKG_PD_EN |\
	MVEBU_PM_GBE1_RX_SRAM_LKG_PD_EN | MVEBU_PM_GBE1_MIB_SRAM_LKG_PD_EN |\
	MVEBU_PM_GBE0_TX_SRAM_LKG_PD_EN | MVEBU_PM_GBE0_RX_SRAM_LKG_PD_EN |\
	MVEBU_PM_GBE0_MIB_SRAM_LKG_PD_EN | MVEBU_PM_SDIO_SRAM_LKG_PD_EN |\
	MVEBU_PM_USB2_SRAM_LKG_PD_EN | MVEBU_PM_USB3_H_SRAM_LKG_PD_EN)
#define MVEBU_PM_SB_WK_EN_REG		(MVEBU_SB_WAKEUP_REG_BASE + 0x10)
  #define MVEBU_PM_SB_GPIO_WKP_EN	BIT(24)
  #define MVEBU_PM_SB_WKP_EN		BIT(20)

/* DRAM registers */
#define MVEBU_DRAM_STATS_CH0_REG	(MVEBU_DRAM_REG_BASE + 0x4)
 #define MVEBU_DRAM_WCP_EMPTY		BIT(19)
#define MVEBU_DRAM_CMD_0_REG		(MVEBU_DRAM_REG_BASE + 0x20)
 #define MVEBU_DRAM_CH0_CMD0		BIT(28)
 #define MVEBU_DRAM_CS_CMD0		BIT(24)
 #define MVEBU_DRAM_WCB_DRAIN_REQ	BIT(1)
#define MVEBU_DRAM_PWR_CTRL_REG		(MVEBU_DRAM_REG_BASE + 0x54)
 #define MVEBU_DRAM_PHY_CLK_GATING_EN	BIT(1)
 #define MVEBU_DRAM_PHY_AUTO_AC_OFF_EN	BIT(0)

/* AVS registers */
#define MVEBU_AVS_CTRL_2_REG		(MVEBU_AVS_REG_BASE + 0x8)
 #define MVEBU_LOW_VDD_MODE_EN		BIT(6)

/* Clock registers */
#define MVEBU_NB_CLOCK_SEL_REG		(MVEBU_NB_REGS_BASE + 0x10)
 #define MVEBU_A53_CPU_CLK_SEL		BIT(15)

/* North Bridge Step-Down Registers */
#define MVEBU_NB_STEP_DOWN_INT_EN_REG	MVEBU_NB_STEP_DOWN_REG_BASE
 #define MVEBU_NB_GPIO_INT_WAKE_WCPU_CLK	BIT(8)

#define MVEBU_NB_GPIO_18	18
#define MVEBU_NB_GPIO_19	19
#define MVEBU_NB_GPIO_25	25
#define MVEBU_NB_GPIO_26	26

typedef int (*wake_up_src_func)(union pm_wake_up_src_data *);

struct wake_up_src_func_map {
	enum pm_wake_up_src_type type;
	wake_up_src_func func;
};

void marvell_psci_arch_init(int die_index)
{
}

static void a3700_pm_ack_irq(void)
{
	uint32_t reg;

	reg = mmio_read_32(MVEBU_NB_IRQ_STATUS_1_REG);
	if (reg)
		mmio_write_32(MVEBU_NB_IRQ_STATUS_1_REG, reg);

	reg = mmio_read_32(MVEBU_NB_IRQ_STATUS_2_REG);
	if (reg)
		mmio_write_32(MVEBU_NB_IRQ_STATUS_2_REG, reg);

	reg = mmio_read_32(MVEBU_SB_IRQ_STATUS_1_REG);
	if (reg)
		mmio_write_32(MVEBU_SB_IRQ_STATUS_1_REG, reg);

	reg = mmio_read_32(MVEBU_SB_IRQ_STATUS_2_REG);
	if (reg)
		mmio_write_32(MVEBU_SB_IRQ_STATUS_2_REG, reg);

	reg = mmio_read_32(MVEBU_NB_GPIO_IRQ_STATUS_LOW_REG);
	if (reg)
		mmio_write_32(MVEBU_NB_GPIO_IRQ_STATUS_LOW_REG, reg);

	reg = mmio_read_32(MVEBU_NB_GPIO_IRQ_STATUS_HIGH_REG);
	if (reg)
		mmio_write_32(MVEBU_NB_GPIO_IRQ_STATUS_HIGH_REG, reg);

	reg = mmio_read_32(MVEBU_SB_GPIO_IRQ_STATUS_REG);
	if (reg)
		mmio_write_32(MVEBU_SB_GPIO_IRQ_STATUS_REG, reg);
}

/*****************************************************************************
 * A3700 handler called to check the validity of the power state
 * parameter.
 *****************************************************************************
 */
int a3700_validate_power_state(unsigned int power_state,
			       psci_power_state_t *req_state)
{
	ERROR("%s needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * A3700 handler called when a CPU is about to enter standby.
 *****************************************************************************
 */
void a3700_cpu_standby(plat_local_state_t cpu_state)
{
	ERROR("%s needs to be implemented\n", __func__);
	panic();
}

/*****************************************************************************
 * A3700 handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 *****************************************************************************
 */
int a3700_pwr_domain_on(u_register_t mpidr)
{
	/* Set barrier */
	dsbsy();

	/* Set the cpu start address to BL1 entry point */
	mmio_write_32(MVEBU_CPU_1_RESET_VECTOR,
		      PLAT_MARVELL_CPU_ENTRY_ADDR >> 2);

	/* Get the cpu out of reset */
	mmio_clrbits_32(MVEBU_CPU_1_RESET_REG, BIT(MVEBU_CPU_1_RESET_BIT));
	mmio_setbits_32(MVEBU_CPU_1_RESET_REG, BIT(MVEBU_CPU_1_RESET_BIT));

	return 0;
}

/*****************************************************************************
 * A3700 handler called to validate the entry point.
 *****************************************************************************
 */
int a3700_validate_ns_entrypoint(uintptr_t entrypoint)
{
	return PSCI_E_SUCCESS;
}

/*****************************************************************************
 * A3700 handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
void a3700_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	plat_marvell_gic_cpuif_disable();

	/* Core can not be powered down with pending IRQ,
	 * acknowledge all the pending IRQ
	 */
	a3700_pm_ack_irq();
}

static void a3700_set_gen_pwr_off_option(void)
{
	/* Enable L2 flush -> processor state-machine option */
	mmio_setbits_32(MVEBU_PM_NB_CPU_PWR_CTRL_REG, MVEBU_PM_L2_FLUSH_EN);

	/*
	 * North bridge cannot be VDD off (always ON).
	 * The NB state machine support low power mode by its state machine.
	 * This bit MUST be set for north bridge power down, e.g.,
	 * OSC input cutoff(NOT TEST), SRAM power down, PMIC, etc.
	 * It is not related to CPU VDD OFF!!
	 */
	mmio_clrbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_CPU_VDDV_OFF_EN);

	/*
	 * MUST: Switch CPU/AXI clock to OSC
	 * NB state machine clock is always connected to OSC (slow clock).
	 * But Core0/1/processor state machine's clock are connected to AXI
	 *  clock. Now, AXI clock takes the TBG as clock source.
	 * If using AXI clock, Core0/1/processor state machine may much faster
	 * than NB state machine. It will cause problem in this case if cores
	 * are released before north bridge gets ready.
	 */
	mmio_clrbits_32(MVEBU_NB_CLOCK_SEL_REG, MVEBU_A53_CPU_CLK_SEL);

	/*
	 * These register bits will trigger north bridge
	 * power-down state machine regardless CM3 status.
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_DEBUG_REG, MVEBU_PM_IGNORE_CM3_SLEEP);
	mmio_setbits_32(MVEBU_PM_NB_PWR_DEBUG_REG, MVEBU_PM_IGNORE_CM3_DEEP);

	/*
	 * SRAM => controlled by north bridge state machine.
	 * Core VDD OFF is not related to CPU SRAM power down.
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_NB_SRAM_LKG_PD_EN);

	/*
	 * Idle AXI interface in order to get L2_WFI
	 * L2 WFI is only asserted after CORE-0 and CORE-1 WFI asserted.
	 * (only both core-0/1in WFI, L2 WFI will be issued by CORE.)
	 * Once L2 WFI asserted, this bit is used for signalling assertion
	 * to AXI IO masters.
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_CTRL_REG, MVEBU_PM_INTERFACE_IDLE);

	/* Enable core0 and core1 VDD_OFF */
	mmio_setbits_32(MVEBU_PM_CPU_0_PWR_CTRL_REG, MVEBU_PM_CORE_PD);
	mmio_setbits_32(MVEBU_PM_CPU_1_PWR_CTRL_REG, MVEBU_PM_CORE_PD);

	/* Enable North bridge power down -
	 * Both Cores MUST enable this bit to power down north bridge!
	 */
	mmio_setbits_32(MVEBU_PM_CPU_0_PWR_CTRL_REG, MVEBU_PM_CORE_SOC_PD);
	mmio_setbits_32(MVEBU_PM_CPU_1_PWR_CTRL_REG, MVEBU_PM_CORE_SOC_PD);

	/* CA53 (processor domain) power down */
	mmio_setbits_32(MVEBU_PM_CPU_0_PWR_CTRL_REG, MVEBU_PM_CORE_PROC_PD);
	mmio_setbits_32(MVEBU_PM_CPU_1_PWR_CTRL_REG, MVEBU_PM_CORE_PROC_PD);
}

static void a3700_en_ddr_self_refresh(void)
{
	/*
	 * Both count is 16 bits and configurable. By default, osc stb cnt
	 * is 0xFFF for lower 12 bits.
	 * Thus, powerdown count is smaller than osc count.
	 * This count is used for exiting DDR SR mode on wakeup event.
	 * The powerdown count also has impact on the following
	 * state changes: idle -> count-down -> ... (power-down, vdd off, etc)
	 * Here, make stable counter shorter
	 * Use power down count value instead of osc_stb_cnt to speed up
	 * DDR self refresh exit
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_CTRL_REG, MVEBU_PM_PWR_DN_CNT_SEL);

	/*
	 * Enable DDR SR mode => controlled by north bridge state machine
	 * Therefore, we must powerdown north bridge to trigger the DDR SR
	 * mode switching.
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_DDR_SR_EN);
	/* Disable DDR clock, otherwise DDR will not enter into SR mode. */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_DDR_CLK_DIS_EN);
	/* Power down DDR PHY (PAD) */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_DDRPHY_PWRDWN_EN);
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG,
			MVEBU_PM_DDRPHY_PAD_PWRDWN_EN);

	/* Set wait time for DDR ready in ROM code */
	mmio_write_32(MVEBU_PM_CPU_VDD_OFF_INFO_1_REG,
		      MVEBU_PM_WAIT_DDR_RDY_VALUE);

	/* DDR flush write buffer - mandatory */
	mmio_write_32(MVEBU_DRAM_CMD_0_REG, MVEBU_DRAM_CH0_CMD0 |
		      MVEBU_DRAM_CS_CMD0 | MVEBU_DRAM_WCB_DRAIN_REQ);
	while ((mmio_read_32(MVEBU_DRAM_STATS_CH0_REG) &
			     MVEBU_DRAM_WCP_EMPTY) != MVEBU_DRAM_WCP_EMPTY)
		;

	/* Trigger PHY reset after ddr out of self refresh =>
	 * supply reset pulse for DDR phy after wake up
	 */
	mmio_setbits_32(MVEBU_DRAM_PWR_CTRL_REG, MVEBU_DRAM_PHY_CLK_GATING_EN |
						 MVEBU_DRAM_PHY_AUTO_AC_OFF_EN);
}

static void a3700_pwr_dn_avs(void)
{
	/*
	 * AVS power down - controlled by north bridge statemachine
	 * Enable AVS power down by clear the AVS disable bit.
	 */
	mmio_clrbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_AVS_DISABLE_MODE);
	/*
	 * Should set BIT[12:13] to powerdown AVS.
	 * 1. Enable AVS VDD2 mode
	 * 2. After power down AVS, we must hold AVS output voltage.
	 * 3. We can choose the lower VDD for AVS power down.
	 */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_AVS_VDD2_MODE);
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_AVS_HOLD_MODE);

	/* Enable low VDD mode, AVS will set CPU to lowest core VDD 747mV */
	mmio_setbits_32(MVEBU_AVS_CTRL_2_REG, MVEBU_LOW_VDD_MODE_EN);
}

static void a3700_pwr_dn_tbg(void)
{
	/* Power down TBG */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_TBG_OFF_EN);
}

static void a3700_pwr_dn_sb(void)
{
	/* Enable south bridge power down option */
	mmio_setbits_32(MVEBU_PM_NB_PWR_CTRL_REG, MVEBU_PM_SB_PWR_DWN);

	/* Enable SDIO_PHY_PWRDWN */
	mmio_setbits_32(MVEBU_PM_SB_PWR_OPTION_REG, MVEBU_PM_SDIO_PHY_PDWN_EN);

	/* Enable SRAM LRM on SB */
	mmio_setbits_32(MVEBU_PM_SB_PWR_OPTION_REG, MVEBU_PM_SB_SRAM_LKG_PD_EN);

	/* Enable SB Power Off */
	mmio_setbits_32(MVEBU_PM_SB_PWR_OPTION_REG, MVEBU_PM_SB_VDDV_OFF_EN);

	/* Kick off South Bridge Power Off */
	mmio_setbits_32(MVEBU_PM_SB_CPU_PWR_CTRL_REG, MVEBU_PM_SB_PM_START);
}

static void a3700_set_pwr_off_option(void)
{
	/* Set general power off option */
	a3700_set_gen_pwr_off_option();

	/* Enable DDR self refresh in low power mode */
	a3700_en_ddr_self_refresh();

	/* Power down AVS */
	a3700_pwr_dn_avs();

	/* Power down TBG */
	a3700_pwr_dn_tbg();

	/* Power down south bridge, pay attention south bridge setting
	 * should be done before
	 */
	a3700_pwr_dn_sb();
}

static void a3700_set_wake_up_option(void)
{
	/*
	 * Enable the wakeup event for NB SOC => north-bridge
	 * state-machine enablement on wake-up event
	 */
	mmio_setbits_32(MVEBU_PM_NB_WAKE_UP_EN_REG, MVEBU_PM_NB_WKP_EN);

	 /* Enable both core0 and core1 wakeup on demand */
	mmio_setbits_32(MVEBU_PM_CPU_WAKE_UP_CONF_REG,
			MVEBU_PM_CORE1_WAKEUP | MVEBU_PM_CORE0_WAKEUP);

	/* Enable warm reset in low power mode */
	mmio_setbits_32(MVEBU_PM_NB_PWR_OPTION_REG, MVEBU_PM_WARM_RESET_EN);
}

static void a3700_pm_en_nb_gpio(uint32_t gpio)
{
	/* For GPIO1 interrupt -- North bridge only */
	if (gpio >= 32) {
		/* GPIO int mask */
		mmio_clrbits_32(MVEBU_NB_GPIO_IRQ_MASK_2_REG, BIT(gpio - 32));

		/* NB_CPU_WAKE-up ENABLE GPIO int */
		mmio_setbits_32(MVEBU_NB_GPIO_IRQ_EN_HIGH_REG, BIT(gpio - 32));
	} else {
		/* GPIO int mask */
		mmio_clrbits_32(MVEBU_NB_GPIO_IRQ_MASK_1_REG, BIT(gpio));

		/* NB_CPU_WAKE-up ENABLE GPIO int */
		mmio_setbits_32(MVEBU_NB_GPIO_IRQ_EN_LOW_REG, BIT(gpio));
	}

	mmio_setbits_32(MVEBU_NB_STEP_DOWN_INT_EN_REG,
			MVEBU_NB_GPIO_INT_WAKE_WCPU_CLK);

	/* Enable using GPIO as wakeup event
	 * (actually not only for north bridge)
	 */
	mmio_setbits_32(MVEBU_PM_NB_WAKE_UP_EN_REG, MVEBU_PM_NB_GPIO_WKP_EN |
		MVEBU_PM_NB_WKP_EN | MVEBU_PM_CORE1_FIQ_IRQ_WKP_EN |
		MVEBU_PM_CORE0_FIQ_IRQ_WKP_EN);
}

static void a3700_pm_en_sb_gpio(uint32_t gpio)
{
	/* Enable using GPIO as wakeup event */
	mmio_setbits_32(MVEBU_PM_NB_WAKE_UP_EN_REG, MVEBU_PM_SB_WKP_NB_EN |
		MVEBU_PM_NB_WKP_EN | MVEBU_PM_CORE1_FIQ_IRQ_WKP_EN |
		MVEBU_PM_CORE0_FIQ_IRQ_WKP_EN);

	/* SB GPIO Wake UP | South Bridge Wake Up Enable */
	mmio_setbits_32(MVEBU_PM_SB_WK_EN_REG, MVEBU_PM_SB_GPIO_WKP_EN |
			MVEBU_PM_SB_GPIO_WKP_EN);

	/* GPIO int mask */
	mmio_clrbits_32(MVEBU_SB_GPIO_IRQ_MASK_REG, BIT(gpio));

	/* NB_CPU_WAKE-up ENABLE GPIO int */
	mmio_setbits_32(MVEBU_SB_GPIO_IRQ_EN_REG, BIT(gpio));
}

int a3700_pm_src_gpio(union pm_wake_up_src_data *src_data)
{
	if (src_data->gpio_data.bank_num == 0)
		/* North Bridge GPIO */
		a3700_pm_en_nb_gpio(src_data->gpio_data.gpio_num);
	else
		a3700_pm_en_sb_gpio(src_data->gpio_data.gpio_num);
	return 0;
}

int a3700_pm_src_uart1(union pm_wake_up_src_data *src_data)
{
	/* Clear Uart1 select */
	mmio_clrbits_32(MVEBU_NB_GPIO1_SEL_REG, MVEBU_NB_GPIO1_UART1_SEL);
	/* set pin 19 gpio usage*/
	mmio_setbits_32(MVEBU_NB_GPIO1_SEL_REG, MVEBU_NB_GPIO1_GPIO_19_EN);
	/* Enable gpio wake-up*/
	a3700_pm_en_nb_gpio(MVEBU_NB_GPIO_19);
	/* set pin 18 gpio usage*/
	mmio_setbits_32(MVEBU_NB_GPIO1_SEL_REG, MVEBU_NB_GPIO1_GPIO_18_EN);
	/* Enable gpio wake-up*/
	a3700_pm_en_nb_gpio(MVEBU_NB_GPIO_18);

	return 0;
}

int a3700_pm_src_uart0(union pm_wake_up_src_data *src_data)
{
	/* set pin 25/26 gpio usage*/
	mmio_setbits_32(MVEBU_NB_GPIO1_SEL_REG, MVEBU_NB_GPIO1_GPIO_25_26_EN);
	/* Enable gpio wake-up*/
	a3700_pm_en_nb_gpio(MVEBU_NB_GPIO_25);
	/* Enable gpio wake-up*/
	a3700_pm_en_nb_gpio(MVEBU_NB_GPIO_26);

	return 0;
}

struct wake_up_src_func_map src_func_table[WAKE_UP_SRC_MAX] = {
	{WAKE_UP_SRC_GPIO, a3700_pm_src_gpio},
	{WAKE_UP_SRC_UART1, a3700_pm_src_uart1},
	{WAKE_UP_SRC_UART0, a3700_pm_src_uart0},
	/* FOLLOWING SRC NOT SUPPORTED YET */
	{WAKE_UP_SRC_TIMER, NULL}
};

static wake_up_src_func a3700_get_wake_up_src_func(
						  enum pm_wake_up_src_type type)
{
	uint32_t loop;

	for (loop = 0; loop < WAKE_UP_SRC_MAX; loop++) {
		if (src_func_table[loop].type == type)
			return src_func_table[loop].func;
	}
	return NULL;
}

#pragma weak mv_wake_up_src_config_get
struct pm_wake_up_src_config *mv_wake_up_src_config_get(void)
{
	static struct pm_wake_up_src_config wake_up_src_cfg = {};
	return &wake_up_src_cfg;
}

static void a3700_set_wake_up_source(void)
{
	struct pm_wake_up_src_config *wake_up_src;
	uint32_t loop;
	wake_up_src_func src_func = NULL;

	wake_up_src = mv_wake_up_src_config_get();
	for (loop = 0; loop < wake_up_src->wake_up_src_num; loop++) {
		src_func = a3700_get_wake_up_src_func(
			   wake_up_src->wake_up_src[loop].wake_up_src_type);
		if (src_func)
			src_func(
				&(wake_up_src->wake_up_src[loop].wake_up_data));
	}
}

static void a3700_pm_save_lp_flag(void)
{
	/* Save the flag for enter the low power mode */
	mmio_setbits_32(MVEBU_PM_CPU_VDD_OFF_INFO_2_REG,
			MVEBU_PM_LOW_POWER_STATE);
}

static void a3700_pm_clear_lp_flag(void)
{
	/* Clear the flag for enter the low power mode */
	mmio_clrbits_32(MVEBU_PM_CPU_VDD_OFF_INFO_2_REG,
			MVEBU_PM_LOW_POWER_STATE);
}

static uint32_t a3700_pm_get_lp_flag(void)
{
	/* Get the flag for enter the low power mode */
	return mmio_read_32(MVEBU_PM_CPU_VDD_OFF_INFO_2_REG) &
			    MVEBU_PM_LOW_POWER_STATE;
}

/*****************************************************************************
 * A3700 handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
void a3700_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	plat_marvell_gic_cpuif_disable();

	/* Save IRQ states */
	plat_marvell_gic_irq_save();

	/* Set wake up options */
	a3700_set_wake_up_option();

	/* Set wake up sources */
	a3700_set_wake_up_source();

	/* SoC can not be powered down with pending IRQ,
	 * acknowledge all the pending IRQ
	 */
	a3700_pm_ack_irq();

	/* Set power off options */
	a3700_set_pwr_off_option();

	/* Save the flag for enter the low power mode */
	a3700_pm_save_lp_flag();

	isb();
}

/*****************************************************************************
 * A3700 handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 *****************************************************************************
 */
void a3700_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* arch specific configuration */
	marvell_psci_arch_init(0);

	/* Per-CPU interrupt initialization */
	plat_marvell_gic_pcpu_init();
	plat_marvell_gic_cpuif_enable();

	/* Restore the per-cpu IRQ state */
	if (a3700_pm_get_lp_flag())
		plat_marvell_gic_irq_pcpu_restore();
}

/*****************************************************************************
 * A3700 handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 *****************************************************************************
 */
void a3700_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	struct dec_win_config *io_dec_map;
	uint32_t dec_win_num;
	struct dram_win_map dram_wins_map;

	/* arch specific configuration */
	marvell_psci_arch_init(0);

	/* Interrupt initialization */
	plat_marvell_gic_init();

	/* Restore IRQ states */
	plat_marvell_gic_irq_restore();

	/*
	 * Initialize CCI for this cluster after resume from suspend state.
	 * No need for locks as no other CPU is active.
	 */
	plat_marvell_interconnect_init();
	/*
	 * Enable CCI coherency for the primary CPU's cluster.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_marvell_interconnect_enter_coherency();

	/* CPU address decoder windows initialization. */
	cpu_wins_init();

	/* fetch CPU-DRAM window mapping information by reading
	 * CPU-DRAM decode windows (only the enabled ones)
	 */
	dram_win_map_build(&dram_wins_map);

	/* Get IO address decoder windows */
	if (marvell_get_io_dec_win_conf(&io_dec_map, &dec_win_num)) {
		printf("No IO address decoder windows configurations found!\n");
		return;
	}

	/* IO address decoder init */
	if (init_io_addr_dec(&dram_wins_map, io_dec_map, dec_win_num)) {
		printf("IO address decoder windows initialization failed!\n");
		return;
	}

	/* Clear low power mode flag */
	a3700_pm_clear_lp_flag();
}

/*****************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
 *****************************************************************************
 */
void a3700_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* lower affinities use PLAT_MAX_OFF_STATE */
	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

/*****************************************************************************
 * A3700 handlers to shutdown/reboot the system
 *****************************************************************************
 */
static void __dead2 a3700_system_off(void)
{
	ERROR("%s needs to be implemented\n", __func__);
	panic();
}

#pragma weak cm3_system_reset
void cm3_system_reset(void)
{
}

/*****************************************************************************
 * A3700 handlers to reset the system
 *****************************************************************************
 */
static void __dead2 a3700_system_reset(void)
{
	/* Clean the mailbox magic number to let it as act like cold boot */
	mmio_write_32(PLAT_MARVELL_MAILBOX_BASE, 0x0);

	dsbsy();

	/* Flush data cache if the mail box shared RAM is cached */
#if PLAT_MARVELL_SHARED_RAM_CACHED
	flush_dcache_range((uintptr_t)PLAT_MARVELL_MAILBOX_BASE,
			   2 * sizeof(uint64_t));
#endif

	/* Use Cortex-M3 secure coprocessor for system reset */
	cm3_system_reset();

	/* Trigger the warm reset */
	mmio_write_32(MVEBU_WARM_RESET_REG, MVEBU_WARM_RESET_MAGIC);

	/* Shouldn't get to this point */
	panic();
}

/*****************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 *****************************************************************************
 */
const plat_psci_ops_t plat_arm_psci_pm_ops = {
	.cpu_standby = a3700_cpu_standby,
	.pwr_domain_on = a3700_pwr_domain_on,
	.pwr_domain_off = a3700_pwr_domain_off,
	.pwr_domain_suspend = a3700_pwr_domain_suspend,
	.pwr_domain_on_finish = a3700_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = a3700_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = a3700_get_sys_suspend_power_state,
	.system_off = a3700_system_off,
	.system_reset = a3700_system_reset,
	.validate_power_state = a3700_validate_power_state,
	.validate_ns_entrypoint = a3700_validate_ns_entrypoint
};

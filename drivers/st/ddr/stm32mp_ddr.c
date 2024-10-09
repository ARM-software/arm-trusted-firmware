/*
 * Copyright (C) 2022-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_ddr.h>
#include <drivers/st/stm32mp_ddrctrl_regs.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define INVALID_OFFSET	0xFFU

static bool axi_port_reenable_request;
static bool host_interface_reenable_request;

static uintptr_t get_base_addr(const struct stm32mp_ddr_priv *priv, enum stm32mp_ddr_base_type base)
{
	if (base == DDRPHY_BASE) {
		return (uintptr_t)priv->phy;
	} else {
		return (uintptr_t)priv->ctl;
	}
}

void stm32mp_ddr_set_reg(const struct stm32mp_ddr_priv *priv, enum stm32mp_ddr_reg_type type,
			 const void *param, const struct stm32mp_ddr_reg_info *ddr_registers)
{
	unsigned int i;
	unsigned int value;
	enum stm32mp_ddr_base_type base = ddr_registers[type].base;
	uintptr_t base_addr = get_base_addr(priv, base);
	const struct stm32mp_ddr_reg_desc *desc = ddr_registers[type].desc;

	VERBOSE("init %s\n", ddr_registers[type].name);
	for (i = 0; i < ddr_registers[type].size; i++) {
		uintptr_t ptr = base_addr + desc[i].offset;

		if (desc[i].par_offset == INVALID_OFFSET) {
			ERROR("invalid parameter offset for %s - index %u",
			      ddr_registers[type].name, i);
			panic();
		} else {
#if !STM32MP13 && !STM32MP15
			if (desc[i].qd) {
				stm32mp_ddr_start_sw_done(priv->ctl);
			}
#endif
			value = *((uint32_t *)((uintptr_t)param +
					       desc[i].par_offset));
			mmio_write_32(ptr, value);
#if !STM32MP13 && !STM32MP15
			if (desc[i].qd) {
				stm32mp_ddr_wait_sw_done_ack(priv->ctl);
			}
#endif
		}
	}
}

/* Start quasi dynamic register update */
void stm32mp_ddr_start_sw_done(struct stm32mp_ddrctl *ctl)
{
	mmio_clrbits_32((uintptr_t)&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);
	VERBOSE("[0x%lx] swctl = 0x%x\n",
		(uintptr_t)&ctl->swctl,  mmio_read_32((uintptr_t)&ctl->swctl));
}

/* Wait quasi dynamic register update */
void stm32mp_ddr_wait_sw_done_ack(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t swstat;

	mmio_setbits_32((uintptr_t)&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);
	VERBOSE("[0x%lx] swctl = 0x%x\n",
		(uintptr_t)&ctl->swctl, mmio_read_32((uintptr_t)&ctl->swctl));

	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		swstat = mmio_read_32((uintptr_t)&ctl->swstat);
		VERBOSE("[0x%lx] swstat = 0x%x ",
			(uintptr_t)&ctl->swstat, swstat);
		if (timeout_elapsed(timeout)) {
			panic();
		}
	} while ((swstat & DDRCTRL_SWSTAT_SW_DONE_ACK) == 0U);

	VERBOSE("[0x%lx] swstat = 0x%x\n",
		(uintptr_t)&ctl->swstat, swstat);
}

void stm32mp_ddr_enable_axi_port(struct stm32mp_ddrctl *ctl)
{
	/* Enable uMCTL2 AXI port 0 */
	mmio_setbits_32((uintptr_t)&ctl->pctrl_0, DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_0 = 0x%x\n", (uintptr_t)&ctl->pctrl_0,
		mmio_read_32((uintptr_t)&ctl->pctrl_0));

#if STM32MP_DDR_DUAL_AXI_PORT
	/* Enable uMCTL2 AXI port 1 */
	mmio_setbits_32((uintptr_t)&ctl->pctrl_1, DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_1 = 0x%x\n", (uintptr_t)&ctl->pctrl_1,
		mmio_read_32((uintptr_t)&ctl->pctrl_1));
#endif
}

int stm32mp_ddr_disable_axi_port(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t pstat;

	/* Disable uMCTL2 AXI port 0 */
	mmio_clrbits_32((uintptr_t)&ctl->pctrl_0, DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_0 = 0x%x\n", (uintptr_t)&ctl->pctrl_0,
		mmio_read_32((uintptr_t)&ctl->pctrl_0));

#if STM32MP_DDR_DUAL_AXI_PORT
	/* Disable uMCTL2 AXI port 1 */
	mmio_clrbits_32((uintptr_t)&ctl->pctrl_1, DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_1 = 0x%x\n", (uintptr_t)&ctl->pctrl_1,
		mmio_read_32((uintptr_t)&ctl->pctrl_1));
#endif

	/*
	 * Waits until all AXI ports are idle
	 * Poll PSTAT.rd_port_busy_n = 0
	 * Poll PSTAT.wr_port_busy_n = 0
	 */
	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		pstat = mmio_read_32((uintptr_t)&ctl->pstat);
		VERBOSE("[0x%lx] pstat = 0x%x ",
			(uintptr_t)&ctl->pstat, pstat);
		if (timeout_elapsed(timeout)) {
			return -1;
		}
	} while (pstat != 0U);

	return 0;
}

static bool ddr_is_axi_port_enabled(struct stm32mp_ddrctl *ctl)
{
	return (mmio_read_32((uintptr_t)&ctl->pctrl_0) & DDRCTRL_PCTRL_N_PORT_EN) != 0U;
}

void stm32mp_ddr_enable_host_interface(struct stm32mp_ddrctl *ctl)
{
	mmio_clrbits_32((uintptr_t)&ctl->dbg1, DDRCTRL_DBG1_DIS_HIF);
	VERBOSE("[0x%lx] dbg1 = 0x%x\n",
		(uintptr_t)&ctl->dbg1,
		mmio_read_32((uintptr_t)&ctl->dbg1));
}

void stm32mp_ddr_disable_host_interface(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t dbgcam;
	int count = 0;

	mmio_setbits_32((uintptr_t)&ctl->dbg1, DDRCTRL_DBG1_DIS_HIF);
	VERBOSE("[0x%lx] dbg1 = 0x%x\n",
		(uintptr_t)&ctl->dbg1,
		mmio_read_32((uintptr_t)&ctl->dbg1));

	/*
	 * Waits until all queues and pipelines are empty
	 * Poll DBGCAM.dbg_wr_q_empty = 1
	 * Poll DBGCAM.dbg_rd_q_empty = 1
	 * Poll DBGCAM.dbg_wr_data_pipeline_empty = 1
	 * Poll DBGCAM.dbg_rd_data_pipeline_empty = 1
	 *
	 * data_pipeline fields must be polled twice to ensure
	 * value propoagation, so count is added to loop condition.
	 */
	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		dbgcam = mmio_read_32((uintptr_t)&ctl->dbgcam);
		VERBOSE("[0x%lx] dbgcam = 0x%x ",
			(uintptr_t)&ctl->dbgcam, dbgcam);
		if (timeout_elapsed(timeout)) {
			panic();
		}
		count++;
	} while (((dbgcam & DDRCTRL_DBG_Q_AND_DATA_PIPELINE_EMPTY) !=
		  DDRCTRL_DBG_Q_AND_DATA_PIPELINE_EMPTY) || (count < 2));
}

static bool ddr_is_host_interface_enabled(struct stm32mp_ddrctl *ctl)
{
	return (mmio_read_32((uintptr_t)&ctl->dbg1) & DDRCTRL_DBG1_DIS_HIF) == 0U;
}

int stm32mp_ddr_sw_selfref_entry(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t stat;
	uint32_t operating_mode;
	uint32_t selref_type;

	mmio_setbits_32((uintptr_t)&ctl->pwrctl, DDRCTRL_PWRCTL_SELFREF_SW);
	VERBOSE("[0x%lx] pwrctl = 0x%x\n",
		(uintptr_t)&ctl->pwrctl,
		mmio_read_32((uintptr_t)&ctl->pwrctl));

	/*
	 * Wait operating mode change in self-refresh mode
	 * with STAT.operating_mode[1:0]==11.
	 * Ensure transition to self-refresh was due to software
	 * by checking also that STAT.selfref_type[1:0]=2.
	 */
	timeout = timeout_init_us(DDR_TIMEOUT_500US);
	while (!timeout_elapsed(timeout)) {
		stat = mmio_read_32((uintptr_t)&ctl->stat);
		operating_mode = stat & DDRCTRL_STAT_OPERATING_MODE_MASK;
		selref_type = stat & DDRCTRL_STAT_SELFREF_TYPE_MASK;

		if ((operating_mode == DDRCTRL_STAT_OPERATING_MODE_SR) &&
		    (selref_type == DDRCTRL_STAT_SELFREF_TYPE_SR)) {
			return 0;
		}
	}

	return -1;
}

void stm32mp_ddr_sw_selfref_exit(struct stm32mp_ddrctl *ctl)
{
	mmio_clrbits_32((uintptr_t)&ctl->pwrctl, DDRCTRL_PWRCTL_SELFREF_SW);
	VERBOSE("[0x%lx] pwrctl = 0x%x\n",
		(uintptr_t)&ctl->pwrctl,
		mmio_read_32((uintptr_t)&ctl->pwrctl));
}

void stm32mp_ddr_set_qd3_update_conditions(struct stm32mp_ddrctl *ctl)
{
	if (ddr_is_axi_port_enabled(ctl)) {
		if (stm32mp_ddr_disable_axi_port(ctl) != 0) {
			panic();
		}
		axi_port_reenable_request = true;
	}

	if (ddr_is_host_interface_enabled(ctl)) {
		stm32mp_ddr_disable_host_interface(ctl);
		host_interface_reenable_request = true;
	}

	stm32mp_ddr_start_sw_done(ctl);
}

void stm32mp_ddr_unset_qd3_update_conditions(struct stm32mp_ddrctl *ctl)
{
	stm32mp_ddr_wait_sw_done_ack(ctl);

	if (host_interface_reenable_request) {
		stm32mp_ddr_enable_host_interface(ctl);
		host_interface_reenable_request = false;
	}

	if (axi_port_reenable_request) {
		stm32mp_ddr_enable_axi_port(ctl);
		axi_port_reenable_request = false;
	}
}

void stm32mp_ddr_wait_refresh_update_done_ack(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t rfshctl3;
	uint32_t refresh_update_level = DDRCTRL_RFSHCTL3_REFRESH_UPDATE_LEVEL;

	/* Toggle rfshctl3.refresh_update_level */
	rfshctl3 = mmio_read_32((uintptr_t)&ctl->rfshctl3);
	if ((rfshctl3 & refresh_update_level) == refresh_update_level) {
		mmio_setbits_32((uintptr_t)&ctl->rfshctl3, refresh_update_level);
	} else {
		mmio_clrbits_32((uintptr_t)&ctl->rfshctl3, refresh_update_level);
		refresh_update_level = 0U;
	}

	VERBOSE("[0x%lx] rfshctl3 = 0x%x\n",
		(uintptr_t)&ctl->rfshctl3, mmio_read_32((uintptr_t)&ctl->rfshctl3));

	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		rfshctl3 = mmio_read_32((uintptr_t)&ctl->rfshctl3);
		VERBOSE("[0x%lx] rfshctl3 = 0x%x ", (uintptr_t)&ctl->rfshctl3, rfshctl3);
		if (timeout_elapsed(timeout)) {
			panic();
		}
	} while ((rfshctl3 & DDRCTRL_RFSHCTL3_REFRESH_UPDATE_LEVEL) != refresh_update_level);

	VERBOSE("[0x%lx] rfshctl3 = 0x%x\n", (uintptr_t)&ctl->rfshctl3, rfshctl3);
}

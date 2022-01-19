/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_ddr.h>
#include <drivers/st/stm32mp_ddrctrl_regs.h>
#include <drivers/st/stm32mp_pmic.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define INVALID_OFFSET	0xFFU

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
			ERROR("invalid parameter offset for %s", desc[i].name);
			panic();
		} else {
			value = *((uint32_t *)((uintptr_t)param +
					       desc[i].par_offset));
			mmio_write_32(ptr, value);
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

	timeout = timeout_init_us(TIMEOUT_US_1S);
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

int stm32mp_board_ddr_power_init(enum ddr_type ddr_type)
{
	if (dt_pmic_status() > 0) {
		return pmic_ddr_power_init(ddr_type);
	}

	return 0;
}

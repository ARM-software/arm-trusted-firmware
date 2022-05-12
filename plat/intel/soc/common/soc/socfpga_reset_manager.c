/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "socfpga_f2sdram_manager.h"
#include "socfpga_mailbox.h"
#include "socfpga_reset_manager.h"
#include "socfpga_system_manager.h"


void deassert_peripheral_reset(void)
{
	mmio_clrbits_32(SOCFPGA_RSTMGR(PER1MODRST),
			RSTMGR_FIELD(PER1, WATCHDOG0) |
			RSTMGR_FIELD(PER1, WATCHDOG1) |
			RSTMGR_FIELD(PER1, WATCHDOG2) |
			RSTMGR_FIELD(PER1, WATCHDOG3) |
			RSTMGR_FIELD(PER1, L4SYSTIMER0) |
			RSTMGR_FIELD(PER1, L4SYSTIMER1) |
			RSTMGR_FIELD(PER1, SPTIMER0) |
			RSTMGR_FIELD(PER1, SPTIMER1) |
			RSTMGR_FIELD(PER1, I2C0) |
			RSTMGR_FIELD(PER1, I2C1) |
			RSTMGR_FIELD(PER1, I2C2) |
			RSTMGR_FIELD(PER1, I2C3) |
			RSTMGR_FIELD(PER1, I2C4) |
			RSTMGR_FIELD(PER1, UART0) |
			RSTMGR_FIELD(PER1, UART1) |
			RSTMGR_FIELD(PER1, GPIO0) |
			RSTMGR_FIELD(PER1, GPIO1));

	mmio_clrbits_32(SOCFPGA_RSTMGR(PER0MODRST),
			RSTMGR_FIELD(PER0, EMAC0OCP) |
			RSTMGR_FIELD(PER0, EMAC1OCP) |
			RSTMGR_FIELD(PER0, EMAC2OCP) |
			RSTMGR_FIELD(PER0, USB0OCP) |
			RSTMGR_FIELD(PER0, USB1OCP) |
			RSTMGR_FIELD(PER0, NANDOCP) |
			RSTMGR_FIELD(PER0, SDMMCOCP) |
			RSTMGR_FIELD(PER0, DMAOCP));

	mmio_clrbits_32(SOCFPGA_RSTMGR(PER0MODRST),
			RSTMGR_FIELD(PER0, EMAC0) |
			RSTMGR_FIELD(PER0, EMAC1) |
			RSTMGR_FIELD(PER0, EMAC2) |
			RSTMGR_FIELD(PER0, USB0) |
			RSTMGR_FIELD(PER0, USB1) |
			RSTMGR_FIELD(PER0, NAND) |
			RSTMGR_FIELD(PER0, SDMMC) |
			RSTMGR_FIELD(PER0, DMA) |
			RSTMGR_FIELD(PER0, SPIM0) |
			RSTMGR_FIELD(PER0, SPIM1) |
			RSTMGR_FIELD(PER0, SPIS0) |
			RSTMGR_FIELD(PER0, SPIS1) |
			RSTMGR_FIELD(PER0, EMACPTP) |
			RSTMGR_FIELD(PER0, DMAIF0) |
			RSTMGR_FIELD(PER0, DMAIF1) |
			RSTMGR_FIELD(PER0, DMAIF2) |
			RSTMGR_FIELD(PER0, DMAIF3) |
			RSTMGR_FIELD(PER0, DMAIF4) |
			RSTMGR_FIELD(PER0, DMAIF5) |
			RSTMGR_FIELD(PER0, DMAIF6) |
			RSTMGR_FIELD(PER0, DMAIF7));

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX
	mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_FIELD(BRG, MPFE));
#endif
}

void config_hps_hs_before_warm_reset(void)
{
	uint32_t or_mask = 0;

	or_mask |= RSTMGR_HDSKEN_SDRSELFREFEN;
	or_mask |= RSTMGR_HDSKEN_FPGAHSEN;
	or_mask |= RSTMGR_HDSKEN_ETRSTALLEN;
	or_mask |= RSTMGR_HDSKEN_L2FLUSHEN;
	or_mask |= RSTMGR_HDSKEN_L3NOC_DBG;
	or_mask |= RSTMGR_HDSKEN_DEBUG_L3NOC;

	mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN), or_mask);
}

static int poll_idle_status(uint32_t addr, uint32_t mask, uint32_t match)
{
	int time_out = 300;

	while (time_out--) {
		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
		udelay(1000);
	}
	return -ETIMEDOUT;
}

static void socfpga_s2f_bridge_mask(uint32_t mask,
				uint32_t *brg_mask,
				uint32_t *noc_mask)
{
	*brg_mask = 0;
	*noc_mask = 0;

	if ((mask & SOC2FPGA_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, SOC2FPGA);
		*noc_mask |= IDLE_DATA_SOC2FPGA;
	}

	if ((mask & LWHPS2FPGA_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, LWHPS2FPGA);
		*noc_mask |= IDLE_DATA_LWSOC2FPGA;
	}
}

static void socfpga_f2s_bridge_mask(uint32_t mask,
				uint32_t *brg_mask,
				uint32_t *f2s_idlereq,
				uint32_t *f2s_force_drain,
				uint32_t *f2s_en,
				uint32_t *f2s_idleack,
				uint32_t *f2s_respempty)
{
	*brg_mask = 0;
	*f2s_idlereq = 0;
	*f2s_force_drain = 0;
	*f2s_en = 0;
	*f2s_idleack = 0;
	*f2s_respempty = 0;

#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
	if ((mask & FPGA2SOC_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, FPGA2SOC);
	}
	if ((mask & F2SDRAM0_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM0);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINTSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINTSTATUS_F2SDRAM0_RESPEMPTY;
	}
	if ((mask & F2SDRAM1_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM1);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM1_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM1_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM1_ENABLE;
		*f2s_idleack |= FLAGINTSTATUS_F2SDRAM1_IDLEACK;
		*f2s_respempty |= FLAGINTSTATUS_F2SDRAM1_RESPEMPTY;
	}
	if ((mask & F2SDRAM2_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM2);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM2_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM2_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM2_ENABLE;
		*f2s_idleack |= FLAGINTSTATUS_F2SDRAM2_IDLEACK;
		*f2s_respempty |= FLAGINTSTATUS_F2SDRAM2_RESPEMPTY;
	}
#else
	if ((mask & FPGA2SOC_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, FPGA2SOC);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINTSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINTSTATUS_F2SDRAM0_RESPEMPTY;
	}
#endif
}

int socfpga_bridges_enable(uint32_t mask)
{
	int ret = 0;
	uint32_t brg_mask = 0;
	uint32_t noc_mask = 0;
	uint32_t f2s_idlereq = 0;
	uint32_t f2s_force_drain = 0;
	uint32_t f2s_en = 0;
	uint32_t f2s_idleack = 0;
	uint32_t f2s_respempty = 0;

	/* Enable s2f bridge */
	socfpga_s2f_bridge_mask(mask, &brg_mask, &noc_mask);
	if (brg_mask != 0U) {
		/* Clear idle request */
		mmio_setbits_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_CLR),
				noc_mask);

		/* De-assert all bridges */
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		/* Wait until idle ack becomes 0 */
		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
						noc_mask, 0);
		if (ret < 0) {
			ERROR("S2F bridge enable: "
					"Timeout waiting for idle ack\n");
		}
	}

	/* Enable f2s bridge */
	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
						&f2s_force_drain, &f2s_en,
						&f2s_idleack, &f2s_respempty);
	if (brg_mask != 0U) {
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		mmio_clrbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_idlereq);

		ret = poll_idle_status(SOCFPGA_F2SDRAMMGR(
			SIDEBANDMGR_FLAGINSTATUS0), f2s_idleack, 0);
		if (ret < 0) {
			ERROR("F2S bridge enable: "
					"Timeout waiting for idle ack");
		}

		mmio_clrbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_force_drain);
		udelay(5);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_en);
		udelay(5);
	}

	return ret;
}

int socfpga_bridges_disable(uint32_t mask)
{
	int ret = 0;
	int timeout = 300;
	uint32_t brg_mask = 0;
	uint32_t noc_mask = 0;
	uint32_t f2s_idlereq = 0;
	uint32_t f2s_force_drain = 0;
	uint32_t f2s_en = 0;
	uint32_t f2s_idleack = 0;
	uint32_t f2s_respempty = 0;

	/* Disable s2f bridge */
	socfpga_s2f_bridge_mask(mask, &brg_mask, &noc_mask);
	if (brg_mask != 0U) {
		mmio_setbits_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_SET),
				noc_mask);

		mmio_write_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 1);

		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
						noc_mask, noc_mask);
		if (ret < 0) {
			ERROR("S2F Bridge disable: "
					"Timeout waiting for idle ack\n");
		}

		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLESTATUS),
						noc_mask, noc_mask);
		if (ret < 0) {
			ERROR("S2F Bridge disable: "
					"Timeout waiting for idle status\n");
		}

		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		mmio_write_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 0);
	}

	/* Disable f2s bridge */
	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
						&f2s_force_drain, &f2s_en,
						&f2s_idleack, &f2s_respempty);
	if (brg_mask != 0U) {
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN),
				RSTMGR_HDSKEN_FPGAHSEN);

		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKREQ_FPGAHSREQ);

		poll_idle_status(SOCFPGA_RSTMGR(HDSKACK),
				RSTMGR_HDSKACK_FPGAHSACK_MASK,
				RSTMGR_HDSKACK_FPGAHSACK_MASK);

		mmio_clrbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
				f2s_en);
		udelay(5);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
				f2s_force_drain);
		udelay(5);

		do {
			/* Read response queue status to ensure it is empty */
			uint32_t idle_status;

			idle_status = mmio_read_32(SOCFPGA_F2SDRAMMGR(
				SIDEBANDMGR_FLAGINSTATUS0));
			if ((idle_status & f2s_respempty) != 0U) {
				idle_status = mmio_read_32(SOCFPGA_F2SDRAMMGR(
					SIDEBANDMGR_FLAGINSTATUS0));
				if ((idle_status & f2s_respempty) != 0U) {
					break;
				}
			}
			udelay(1000);
		} while (timeout-- > 0);

#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
		/* Software must never write a 0x1 to FPGA2SOC_MASK bit */
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask & ~RSTMGR_FIELD(BRG, FPGA2SOC));
#else
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask);
#endif
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKEQ_FPGAHSREQ);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_idlereq);
	}

	return ret;
}

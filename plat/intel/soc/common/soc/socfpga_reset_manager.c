/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "socfpga_f2sdram_manager.h"
#include "socfpga_mailbox.h"
#include "socfpga_plat_def.h"
#include "socfpga_reset_manager.h"
#include "socfpga_system_manager.h"

void deassert_peripheral_reset(void)
{
	mmio_clrbits_32(SOCFPGA_RSTMGR(PER1MODRST),
			RSTMGR_FIELD(PER1, WATCHDOG0) |
			RSTMGR_FIELD(PER1, WATCHDOG1) |
			RSTMGR_FIELD(PER1, WATCHDOG2) |
			RSTMGR_FIELD(PER1, WATCHDOG3) |
			RSTMGR_FIELD(PER1, WATCHDOG4) |
			RSTMGR_FIELD(PER1, L4SYSTIMER0) |
			RSTMGR_FIELD(PER1, L4SYSTIMER1) |
			RSTMGR_FIELD(PER1, SPTIMER0) |
			RSTMGR_FIELD(PER1, SPTIMER1) |
			RSTMGR_FIELD(PER1, I2C0) |
			RSTMGR_FIELD(PER1, I2C1) |
			RSTMGR_FIELD(PER1, I2C2) |
			RSTMGR_FIELD(PER1, I2C3) |
			RSTMGR_FIELD(PER1, I2C4) |
			RSTMGR_FIELD(PER1, I3C0) |
			RSTMGR_FIELD(PER1, I3C1) |
			RSTMGR_FIELD(PER1, UART0) |
			RSTMGR_FIELD(PER1, UART1) |
			RSTMGR_FIELD(PER1, GPIO0) |
			RSTMGR_FIELD(PER1, GPIO1));

	mmio_clrbits_32(SOCFPGA_RSTMGR(PER0MODRST),
			RSTMGR_FIELD(PER0, SOFTPHY) |
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

	or_mask |= RSTMGR_HDSKEN_EMIF_FLUSH;
	or_mask |= RSTMGR_HDSKEN_FPGAHSEN;
	or_mask |= RSTMGR_HDSKEN_ETRSTALLEN;
	or_mask |= RSTMGR_HDSKEN_LWS2F_FLUSH;
	or_mask |= RSTMGR_HDSKEN_L3NOC_DBG;
	or_mask |= RSTMGR_HDSKEN_DEBUG_L3NOC;

	mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN), or_mask);
}

static int poll_idle_status(uint32_t addr, uint32_t mask, uint32_t match, uint32_t delay_ms)
{
	int time_out = delay_ms;

	while (time_out-- > 0) {

		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
		udelay(1000);
	}
	return -ETIMEDOUT;
}

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
static int poll_idle_status_by_counter(uint32_t addr, uint32_t mask,
					uint32_t match, uint32_t delay_ms)
{
	int time_out = delay_ms;

	while (time_out-- > 0) {

		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}

		/* ToDo: Shall use udelay for product release */
		for (int i = 0; i < 2000; i++) {
			/* dummy delay */
		}
	}
	return -ETIMEDOUT;
}
#endif

static int poll_idle_status_by_clkcycles(uint32_t addr, uint32_t mask,
					 uint32_t match, uint32_t delay_clk_cycles)
{
	int time_out = delay_clk_cycles;

	while (time_out-- > 0) {

		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
		udelay(1);
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
				    uint32_t *f2s_respempty,
				    uint32_t *f2s_cmdidle)
{
	*brg_mask = 0;
	*f2s_idlereq = 0;
	*f2s_force_drain = 0;
	*f2s_en = 0;
	*f2s_idleack = 0;
	*f2s_respempty = 0;
	*f2s_cmdidle = 0;

#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
	if ((mask & FPGA2SOC_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, FPGA2SOC);
	}
	if ((mask & F2SDRAM0_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM0);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM0_RESPEMPTY;
		*f2s_cmdidle |= FLAGINSTATUS_F2SDRAM0_CMDIDLE;
	}
	if ((mask & F2SDRAM1_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM1);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM1_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM1_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM1_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM1_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM1_RESPEMPTY;
		*f2s_cmdidle |= FLAGINSTATUS_F2SDRAM1_CMDIDLE;
	}
	if ((mask & F2SDRAM2_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM2);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM2_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM2_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM2_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM2_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM2_RESPEMPTY;
		*f2s_cmdidle |= FLAGINSTATUS_F2SDRAM2_CMDIDLE;
	}
#elif PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	if (mask & FPGA2SOC_MASK) {
		*brg_mask |= RSTMGR_FIELD(BRG, FPGA2SOC);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM0_RESPEMPTY;
	}
	if (mask & F2SDRAM0_MASK) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM0);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM0_RESPEMPTY;
	}
	if (mask & F2SDRAM1_MASK) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM1);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM1_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM1_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM1_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM1_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM1_RESPEMPTY;
	}
	if (mask & F2SDRAM2_MASK) {
		*brg_mask |= RSTMGR_FIELD(BRG, F2SSDRAM2);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM2_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM2_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM2_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM2_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM2_RESPEMPTY;
	}
#else
	if ((mask & FPGA2SOC_MASK) != 0U) {
		*brg_mask |= RSTMGR_FIELD(BRG, FPGA2SOC);
		*f2s_idlereq |= FLAGOUTSETCLR_F2SDRAM0_IDLEREQ;
		*f2s_force_drain |= FLAGOUTSETCLR_F2SDRAM0_FORCE_DRAIN;
		*f2s_en |= FLAGOUTSETCLR_F2SDRAM0_ENABLE;
		*f2s_idleack |= FLAGINSTATUS_F2SDRAM0_IDLEACK;
		*f2s_respempty |= FLAGINSTATUS_F2SDRAM0_RESPEMPTY;
		*f2s_cmdidle |= FLAGINSTATUS_F2SDRAM0_CMDIDLE;
	}
#endif
}

int socfpga_bridges_reset(uint32_t mask)
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
	uint32_t f2s_cmdidle = 0;

	/* Reset s2f bridge */
	socfpga_s2f_bridge_mask(mask, &brg_mask, &noc_mask);
	if (brg_mask) {
		if (mask & SOC2FPGA_MASK) {
			/* Request handshake with SOC2FPGA bridge to clear traffic */
			mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
					RSTMGR_HDSKREQ_S2F_FLUSH);

			/* Wait for bridge to idle status */
			ret = poll_idle_status(SOCFPGA_RSTMGR(HDSKACK),
					RSTMGR_HDSKACK_S2F_FLUSH,
					RSTMGR_HDSKACK_S2F_FLUSH, 300);
		}

		if (mask & LWHPS2FPGA_MASK) {
			/* Request handshake with LWSOC2FPGA bridge to clear traffic */
			mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
					RSTMGR_HDSKREQ_LWS2F_FLUSH);

			/* Wait for bridge to idle status */
			ret = poll_idle_status(SOCFPGA_RSTMGR(HDSKACK),
					RSTMGR_HDSKACK_LWS2F_FLUSH,
					RSTMGR_HDSKACK_LWS2F_FLUSH, 300);
		}

		if (ret < 0) {
			ERROR("S2F Bridge reset: Timeout waiting for idle ack\n");
			assert(false);
		}

		/* Assert reset to bridge */
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask);

		/* Clear idle requests to bridge */
		if (mask & SOC2FPGA_MASK) {
			mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
					RSTMGR_HDSKREQ_S2F_FLUSH);
		}

		if (mask & LWHPS2FPGA_MASK) {
			mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
					RSTMGR_HDSKREQ_LWS2F_FLUSH);
		}

		/* When FPGA reconfig is complete */
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);
	}

	/* Reset f2s bridge */
	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
					&f2s_force_drain, &f2s_en,
					&f2s_idleack, &f2s_respempty,
					&f2s_cmdidle);

	if (brg_mask) {
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN),
				RSTMGR_HDSKEN_FPGAHSEN);

		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKREQ_FPGAHSREQ);

		ret = poll_idle_status(SOCFPGA_RSTMGR(HDSKACK),
				RSTMGR_HDSKACK_FPGAHSREQ,
				RSTMGR_HDSKACK_FPGAHSREQ, 300);

		if (ret < 0) {
			ERROR("F2S Bridge disable: Timeout waiting for idle req\n");
			assert(false);
		}

		/* Disable f2s bridge */
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
			if (idle_status & f2s_respempty) {
				idle_status = mmio_read_32(SOCFPGA_F2SDRAMMGR(
					SIDEBANDMGR_FLAGINSTATUS0));
				if (idle_status & f2s_respempty) {
					break;
				}
			}
			udelay(1000);
		} while (timeout-- > 0);

		/* Assert reset to f2s bridge */
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask);

		/* Clear idle request to FPGA */
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Clear idle request to MPFE */
		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_idlereq);

		/* When FPGA reconfig is complete */
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		/* Enable f2s bridge */
		mmio_clrbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_idlereq);

		ret = poll_idle_status(SOCFPGA_F2SDRAMMGR(
			SIDEBANDMGR_FLAGINSTATUS0), f2s_idleack, 0, 300);
		if (ret < 0) {
			ERROR("F2S bridge enable: Timeout waiting for idle ack");
			assert(false);
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
	uint32_t f2s_cmdidle = 0;
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	uint32_t delay = 0;
#endif

	/* Enable s2f bridge */
	socfpga_s2f_bridge_mask(mask, &brg_mask, &noc_mask);
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	/* Enable SOC2FPGA bridge */
	if (brg_mask & RSTMGR_BRGMODRSTMASK_SOC2FPGA) {
		/* Write Reset Manager hdskreq[soc2fpga_flush_req] = 1 */
		NOTICE("Set S2F hdskreq ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_SOC2FPGAREQ);

		/* Read Reset Manager hdskack[soc2fpga] = 1 */
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_SOC2FPGAACK, RSTMGR_HDSKACK_SOC2FPGAACK,
			300);

		if (ret < 0) {
			ERROR("S2F bridge enable: Timeout hdskack\n");
		}

		/* Write Reset Manager hdskreq[soc2fpga_flush_req] = 0 */
		NOTICE("Clear S2F hdskreq ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_SOC2FPGAREQ);

		/* Write Reset Manager brgmodrst[soc2fpga] = 1 */
		NOTICE("Assert S2F ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_SOC2FPGA);

		/* ToDo: Shall use udelay for product release */
		for (delay = 0; delay < 1000; delay++) {
			/* dummy delay */
		}

		/* Write Reset Manager brgmodrst[soc2fpga] = 0 */
		NOTICE("Deassert S2F ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_SOC2FPGA);
	}

	/* Enable LWSOC2FPGA bridge */
	if (brg_mask & RSTMGR_BRGMODRSTMASK_LWHPS2FPGA) {
		/* Write Reset Manager hdskreq[lwsoc2fpga_flush_req] = 1 */
		NOTICE("Set LWS2F hdskreq ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_LWSOC2FPGAREQ);

		/* Read Reset Manager hdskack[lwsoc2fpga] = 1 */
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_LWSOC2FPGAACK, RSTMGR_HDSKACK_LWSOC2FPGAACK,
			300);

		if (ret < 0) {
			ERROR("LWS2F bridge enable: Timeout hdskack\n");
		}

		/* Write Reset Manager hdskreq[lwsoc2fpga_flush_req] = 0 */
		NOTICE("Clear LWS2F hdskreq ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_LWSOC2FPGAREQ);

		/* Write Reset Manager brgmodrst[lwsoc2fpga] = 1 */
		NOTICE("Assert LWS2F ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_LWHPS2FPGA);

		/* ToDo: Shall use udelay for product release */
		for (delay = 0; delay < 1000; delay++) {
			/* dummy delay */
		}

		/* Write Reset Manager brgmodrst[lwsoc2fpga] = 0 */
		NOTICE("Deassert LWS2F ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_LWHPS2FPGA);
	}
#else
	if (brg_mask != 0U) {
		/* Clear idle request */
		mmio_setbits_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_CLR),
				noc_mask);

		/* De-assert all bridges */
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		/* Wait until idle ack becomes 0 */
		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
				       noc_mask, 0, 300);
		if (ret < 0) {
			ERROR("S2F bridge enable: Timeout idle ack\n");
		}
	}
#endif

	/* Enable f2s bridge */
	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
				&f2s_force_drain, &f2s_en,
				&f2s_idleack, &f2s_respempty, &f2s_cmdidle);
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	/* Enable FPGA2SOC bridge */
	if (brg_mask & RSTMGR_BRGMODRSTMASK_FPGA2SOC) {
		/* Write Reset Manager hdsken[fpgahsen] = 1 */
		NOTICE("Set FPGA hdsken(fpgahsen) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN), RSTMGR_HDSKEN_FPGAHSEN);

		/* Write Reset Manager hdskreq[fpgahsreq] = 1 */
		NOTICE("Set FPGA hdskreq(fpgahsreq) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ), RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Read Reset Manager hdskack[fpgahsack] = 1 */
		NOTICE("Get FPGA hdskack(fpgahsack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGAHSACK, RSTMGR_HDSKACK_FPGAHSACK,
			300);

		if (ret < 0) {
			ERROR("FPGA bridge fpga handshake fpgahsreq: Timeout\n");
		}

		/* Write Reset Manager hdskreq[f2s_flush_req] = 1 */
		NOTICE("Set F2S hdskreq(f2s_flush_req) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_FPGA2SOCREQ);

		/* Read Reset Manager hdskack[f2s_flush_ack] = 1 */
		NOTICE("Get F2S hdskack(f2s_flush_ack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGA2SOCACK, RSTMGR_HDSKACK_FPGA2SOCACK,
			300);

		if (ret < 0) {
			ERROR("F2S bridge fpga handshake f2sdram_flush_req: Timeout\n");
		}

		/* Write Reset Manager hdskreq[fpgahsreq] = 1 */
		NOTICE("Clear FPGA hdskreq(fpgahsreq) ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ), RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Write Reset Manager hdskreq[f2s_flush_req] = 1 */
		NOTICE("Clear F2S hdskreq(f2s_flush_req) ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_FPGA2SOCREQ);

		/* Read Reset Manager hdskack[f2s_flush_ack] = 0 */
		NOTICE("Get F2SDRAM hdskack(f2s_flush_ack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGA2SOCACK, RSTMGR_HDSKACK_FPGA2SOCACK_DASRT,
			300);

		if (ret < 0) {
			ERROR("F2S bridge fpga handshake f2s_flush_ack: Timeout\n");
		}

		/* Read Reset Manager hdskack[fpgahsack] = 0 */
		NOTICE("Get FPGA hdskack(fpgahsack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGAHSACK, RSTMGR_HDSKACK_FPGAHSACK_DASRT,
			300);

		if (ret < 0) {
			ERROR("F2S bridge fpga handshake fpgahsack: Timeout\n");
		}

		/* Write Reset Manager brgmodrst[fpga2soc] = 1 */
		NOTICE("Assert F2S ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST), RSTMGR_BRGMODRST_FPGA2SOC);

		/* ToDo: Shall use udelay for product release */
		for (delay = 0; delay < 1000; delay++) {
			/* dummy delay */
		}

		/* Write Reset Manager brgmodrst[fpga2soc] = 0 */
		NOTICE("Deassert F2S ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), RSTMGR_BRGMODRST_FPGA2SOC);

		/* Write System Manager f2s bridge control register[f2soc_enable] = 1 */
		NOTICE("Deassert F2S f2soc_enable ...\n");
		mmio_setbits_32(SOCFPGA_SYSMGR(F2S_BRIDGE_CTRL),
			SYSMGR_F2S_BRIDGE_CTRL_EN);
	}

	/* Enable FPGA2SDRAM bridge */
	if (brg_mask & RSTMGR_BRGMODRSTMASK_F2SDRAM0) {
		/* Write Reset Manager hdsken[fpgahsen] = 1 */
		NOTICE("Set F2SDRAM hdsken(fpgahsen) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN), RSTMGR_HDSKEN_FPGAHSEN);

		/* Write Reset Manager hdskreq[fpgahsreq] = 1 */
		NOTICE("Set F2SDRAM hdskreq(fpgahsreq) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ), RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Read Reset Manager hdskack[fpgahsack] = 1 */
		NOTICE("Get F2SDRAM hdskack(fpgahsack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGAHSACK, RSTMGR_HDSKACK_FPGAHSACK,
			300);

		if (ret < 0) {
			ERROR("F2SDRAM bridge fpga handshake fpgahsreq: Timeout\n");
		}

		/* Write Reset Manager hdskreq[f2sdram_flush_req] = 1 */
		NOTICE("Set F2SDRAM hdskreq(f2sdram_flush_req) ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
			RSTMGR_HDSKREQ_F2SDRAM0REQ);

		/* Read Reset Manager hdskack[f2sdram_flush_ack] = 1 */
		NOTICE("Get F2SDRAM hdskack(f2sdram_flush_ack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_F2SDRAM0ACK, RSTMGR_HDSKACK_F2SDRAM0ACK,
			300);

		if (ret < 0) {
			ERROR("F2SDRAM bridge fpga handshake f2sdram_flush_req: Timeout\n");
		}

		/* Write Reset Manager hdskreq[fpgahsreq] = 1 */
		NOTICE("Clear F2SDRAM hdskreq(fpgahsreq) ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ), RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Write Reset Manager hdskreq[f2sdram_flush_req] = 1 */
		NOTICE("Clear F2SDRAM hdskreq(f2sdram_flush_req) ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ), RSTMGR_HDSKREQ_F2SDRAM0REQ);

		/* Read Reset Manager hdskack[f2sdram_flush_ack] = 0 */
		NOTICE("Get F2SDRAM hdskack(f2sdram_flush_ack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_F2SDRAM0ACK, RSTMGR_HDSKACK_F2SDRAM0ACK_DASRT,
			300);

		if (ret < 0) {
			ERROR("F2SDRAM bridge fpga handshake f2sdram_flush_ack: Timeout\n");
		}

		/* Read Reset Manager hdskack[fpgahsack] = 0 */
		NOTICE("Get F2SDRAM hdskack(fpgahsack) ...\n");
		ret = poll_idle_status_by_counter(SOCFPGA_RSTMGR(HDSKACK),
			RSTMGR_HDSKACK_FPGAHSACK, RSTMGR_HDSKACK_FPGAHSACK_DASRT,
			300);

		if (ret < 0) {
			ERROR("F2SDRAM bridge fpga handshake fpgahsack: Timeout\n");
		}

		/* Write Reset Manager brgmodrst[fpga2sdram] = 1 */
		NOTICE("Assert F2SDRAM ...\n");
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_F2SSDRAM0);

		/* ToDo: Shall use udelay for product release */
		for (delay = 0; delay < 1000; delay++) {
			/* dummy delay */
		}

		/* Write Reset Manager brgmodrst[fpga2sdram] = 0 */
		NOTICE("Deassert F2SDRAM ...\n");
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST),
			RSTMGR_BRGMODRST_F2SSDRAM0);

		/*
		 * Clear fpga2sdram_manager_main_SidebandManager_FlagOutClr0
		 * f2s_ready_latency_enable
		 */
		NOTICE("Clear F2SDRAM f2s_ready_latency_enable ...\n");
		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
			FLAGOUTCLR0_F2SDRAM0_ENABLE);
	}
#else
	if (brg_mask != 0U) {
		mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_idlereq);

		ret = poll_idle_status(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGINSTATUS0),
				       f2s_idleack, 0, 300);

		if (ret < 0) {
			ERROR("F2S bridge enable: Timeout idle ack");
		}

		/* Clear the force drain */
		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_force_drain);
		udelay(5);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
				f2s_en);
		udelay(5);
	}
#endif
	return ret;
}

int socfpga_bridge_nongraceful_disable(uint32_t mask)
{
	int ret = 0;
	int timeout = 1000;
	uint32_t brg_mask = 0;
	uint32_t f2s_idlereq = 0;
	uint32_t f2s_force_drain = 0;
	uint32_t f2s_en = 0;
	uint32_t f2s_idleack = 0;
	uint32_t f2s_respempty = 0;
	uint32_t f2s_cmdidle = 0;

	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
				&f2s_force_drain, &f2s_en,
				&f2s_idleack, &f2s_respempty, &f2s_cmdidle);

	mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_idlereq);

	/* Time out Error - Bus is still active */
	/* Performing a non-graceful shutdown with Force drain */
	mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTSET0),
			f2s_force_drain);

	ret = -ETIMEDOUT;
	do {
		/* Read response queue status to ensure it is empty */
		uint32_t idle_status;

		idle_status = mmio_read_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGINSTATUS0));
		if ((idle_status & f2s_respempty) != 0U) {
			idle_status = mmio_read_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGINSTATUS0));
			if ((idle_status & f2s_respempty) != 0U) {
				/* No time-out we are good! */
				ret = 0;
				break;
			}
		}

		asm("nop");

	} while (timeout-- > 0);

	return ret;
}

int socfpga_bridges_disable(uint32_t mask)
{
	int ret = 0;
	uint32_t brg_mask = 0;
	uint32_t noc_mask = 0;
	uint32_t f2s_idlereq = 0;
	uint32_t f2s_force_drain = 0;
	uint32_t f2s_en = 0;
	uint32_t f2s_idleack = 0;
	uint32_t f2s_respempty = 0;
	uint32_t f2s_cmdidle = 0;

	/* Disable s2f bridge */
	socfpga_s2f_bridge_mask(mask, &brg_mask, &noc_mask);
	if (brg_mask != 0U) {
		mmio_setbits_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_SET),
				noc_mask);

		mmio_write_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 1);

		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
				       noc_mask, noc_mask, 300);
		if (ret < 0) {
			ERROR("S2F Bridge disable: Timeout idle ack\n");
		}

		ret = poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLESTATUS),
				       noc_mask, noc_mask, 300);
		if (ret < 0) {
			ERROR("S2F Bridge disable: Timeout idle status\n");
		}

		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST), brg_mask);

		mmio_write_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 0);
	}

	/* Disable f2s bridge */
	socfpga_f2s_bridge_mask(mask, &brg_mask, &f2s_idlereq,
				&f2s_force_drain, &f2s_en,
				&f2s_idleack, &f2s_respempty, &f2s_cmdidle);
	if (brg_mask != 0U) {

		if (mmio_read_32(SOCFPGA_RSTMGR(BRGMODRST)) & brg_mask) {
			/* Bridge cannot be reset twice */
			return 0;
		}

		/* Starts the fence and drain traffic from F2SDRAM to MPFE */
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN),
				RSTMGR_HDSKEN_FPGAHSEN);
		udelay(5);
		/* Ignoring FPGA ACK as it will time-out */
		mmio_setbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKREQ_FPGAHSREQ);

		ret = poll_idle_status_by_clkcycles(SOCFPGA_RSTMGR(HDSKACK),
						    RSTMGR_HDSKACK_FPGAHSREQ,
						    RSTMGR_HDSKACK_FPGAHSREQ, 1000);

		/* DISABLE F2S Bridge */
		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_en);
		udelay(5);

		ret = socfpga_bridge_nongraceful_disable(mask);

		/* Bridge reset */
#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
		/* Software must never write a 0x1 to FPGA2SOC_MASK bit */
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask & ~RSTMGR_FIELD(BRG, FPGA2SOC));
#else
		mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
				brg_mask);
#endif
		/* Re-enable traffic to SDRAM*/
		mmio_clrbits_32(SOCFPGA_RSTMGR(HDSKREQ),
				RSTMGR_HDSKREQ_FPGAHSREQ);

		mmio_setbits_32(SOCFPGA_F2SDRAMMGR(SIDEBANDMGR_FLAGOUTCLR0),
				f2s_idlereq);
	}

	return ret;
}

/* CPUxRESETBASELOW */
int socfpga_cpu_reset_base(unsigned int cpu_id)
{
	int ret = 0;
	uint32_t entrypoint = 0;

	ret = socfpga_cpurstrelease(cpu_id);

	if (ret < 0) {
		return RSTMGR_RET_ERROR;
	}

	if (ret == RSTMGR_RET_OK) {

		switch (cpu_id) {
		case 0:
			entrypoint = mmio_read_32(SOCFPGA_RSTMGR_CPUBASELOW_0);
			entrypoint |= mmio_read_32(SOCFPGA_RSTMGR_CPUBASEHIGH_0) << 24;
		break;

		case 1:
			entrypoint = mmio_read_32(SOCFPGA_RSTMGR_CPUBASELOW_1);
			entrypoint |= mmio_read_32(SOCFPGA_RSTMGR_CPUBASEHIGH_1) << 24;
		break;

		case 2:
			entrypoint = mmio_read_32(SOCFPGA_RSTMGR_CPUBASELOW_2);
			entrypoint |= mmio_read_32(SOCFPGA_RSTMGR_CPUBASEHIGH_2) << 24;
		break;

		case 3:
			entrypoint = mmio_read_32(SOCFPGA_RSTMGR_CPUBASELOW_3);
			entrypoint |= mmio_read_32(SOCFPGA_RSTMGR_CPUBASEHIGH_3) << 24;
		break;

		default:
		break;
		}

		mmio_write_64(PLAT_SEC_ENTRY, entrypoint);
	}

	return RSTMGR_RET_OK;
}

/* CPURSTRELEASE */
int socfpga_cpurstrelease(unsigned int cpu_id)
{
	unsigned int timeout = 0;

	do {
		/* Read response queue status to ensure it is empty */
		uint32_t cpurstrelease_status;

		cpurstrelease_status = mmio_read_32(SOCFPGA_RSTMGR(CPURSTRELEASE));

		if ((cpurstrelease_status & RSTMGR_CPUSTRELEASE_CPUx) == cpu_id) {
			return RSTMGR_RET_OK;
		}
		udelay(1000);
	} while (timeout-- > 0);

	return RSTMGR_RET_ERROR;
}

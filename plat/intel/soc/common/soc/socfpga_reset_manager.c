/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <errno.h>
#include <lib/mmio.h>

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
	int time_out = 1000;

	while (time_out--) {
		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
	}
	return -ETIMEDOUT;
}

int socfpga_bridges_enable(void)
{
	/* Clear idle request */
	mmio_setbits_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_CLR), ~0);

	/* De-assert all bridges */
	mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), ~0);

	/* Wait until idle ack becomes 0 */
	return poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
				IDLE_DATA_MASK, 0);
}

int socfpga_bridges_disable(void)
{
	/* Set idle request */
	mmio_write_32(SOCFPGA_SYSMGR(NOC_IDLEREQ_SET), ~0);

	/* Enable NOC timeout */
	mmio_setbits_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 1);

	/* Wait until each idle ack bit toggle to 1 */
	if (poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLEACK),
				IDLE_DATA_MASK, IDLE_DATA_MASK))
		return -ETIMEDOUT;

	/* Wait until each idle status bit toggle to 1 */
	if (poll_idle_status(SOCFPGA_SYSMGR(NOC_IDLESTATUS),
				IDLE_DATA_MASK, IDLE_DATA_MASK))
		return -ETIMEDOUT;

	/* Assert all bridges */
#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
	mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
		~(RSTMGR_FIELD(BRG, DDRSCH) | RSTMGR_FIELD(BRG, FPGA2SOC)));
#elif PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX
	mmio_setbits_32(SOCFPGA_RSTMGR(BRGMODRST),
		~(RSTMGR_FIELD(BRG, MPFE) | RSTMGR_FIELD(BRG, FPGA2SOC)));
#endif

	/* Disable NOC timeout */
	mmio_clrbits_32(SOCFPGA_SYSMGR(NOC_TIMEOUT), 1);

	return 0;
}

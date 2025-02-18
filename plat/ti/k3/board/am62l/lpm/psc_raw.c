/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <k3_lpm_timeout.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <psc_raw.h>

#define PSC_PID                 0x000U
#define PSC_GBLCTL              0x010U
#define PSC_GBLSTAT             0x014U
#define PSC_INTEVAL             0x018U
#define PSC_IPWKCNT             0x01cU
#define PSC_MERRPR0             0x040U
#define PSC_MERRPR1             0x044U
#define PSC_MERRCR0             0x050U
#define PSC_MERRCR1             0x054U
#define PSC_PERRPR              0x060U
#define PSC_PERRCR              0x068U
#define PSC_EPCPR               0x070U
#define PSC_EPCRR               0x078U
#define PSC_RAILSTAT            0x100U
#define PSC_RAILCTL             0x104U
#define PSC_RAILSET             0x108U
#define PSC_PTCMD               0x120U
#define PSC_PTSTAT              0x128U
#define PSC_PDSTAT(domain)      (0x200U + (4U * (domain)))
#define PSC_PDCTL(domain)       (0x300U + (4U * (domain)))
#define PSC_PDCFG(domain)       (0x400U + (4U * (domain)))
#define PSC_MDCFG(id)           (0x600U + (4U * (id)))
#define PSC_MDSTAT(id)          (0x800U + (4U * (id)))
#define PSC_MDCTL(id)           (0xa00U + (4U * (id)))

#define MDSTAT_STATE_MASK               0x3fU
#define MDSTAT_BUSY_MASK                0x30U
#define MDSTAT_STATE_SWRSTDISABLE       0x00U
#define MDSTAT_STATE_SYNCRST            0x01U
#define MDSTAT_STATE_DISABLE            0x02U
#define MDSTAT_STATE_ENABLE             0x03U
#define MDSTAT_STATE_AUTO_SLEEP         0x04U
#define MDSTAT_STATE_AUTO_WAKE          0x05U
#define MDSTAT_STATE_DISABLE_CLK_ON     0x21U
#define MDSTAT_STATE_DISABLE_IN_PROG    0x22U
#define MDSTAT_STATE_RETRY_DISABLE      0x23U
#define MDSTAT_STATE_ENABLE_IN_PROG     0x24U
#define MDSTAT_STATE_SLEEP_IN_PROG      0x25U
#define MDSTAT_STATE_CLK_ON1            0x26U
#define MDSTAT_STATE_CLK_OFF1           0x27U
#define MDSTAT_STATE_CLK_ON2            0x28U
#define MDSTAT_STATE_CLK_OFF2           0x29U
#define MDSTAT_STATE_CLK_ON3            0x2aU
#define MDSTAT_STATE_CLK_OFF3           0x2bU

#define MDSTAT_EMUIHB			BIT(17) /* EMU alters module state */
#define MDSTAT_EMURST			BIT(16) /* EMU alters reset to module */
#define MDSTAT_MCKOUT			BIT(12) /* Module clock output status */
#define MDSTAT_MRSTDONE			BIT(11) /* Module reset done */
#define MDSTAT_MRST			BIT(10) /* Module reset deasserted */
#define MDSTAT_LRSTDONE			BIT(9)  /* Local reset done */
#define MDSTAT_LRST			BIT(8)  /* Local reset deasserted */

#define MDCTL_STATE_MASK		0x3fU
#define MDCTL_FORCE			BIT(31)
#define MDCTL_RESET_ISO			BIT(12)         /* Enable reset isolation */
#define MDCTL_EMUIHBIE			BIT(10)         /* EMU alters module IE */
#define MDCTL_EMURSTIE			BIT(9)          /* EMU alter reset IE */
#define MDCTL_LRST			BIT(8)          /* Assert local reset when 0 */

#define PDSTAT_EMUIHB                   BIT(11) /* EMU alters domain state */
#define PDSTAT_PWRBAD                   BIT(10) /* Power bad error */
#define PDSTAT_PORDONE                  BIT(9)  /* Power on reset done */
#define PDSTAT_POR                      BIT(8)  /* Power on reset deasserted */
#define PDSTAT_STATE_MASK               0x1fU
#define PDSTAT_STATE_OFF                0x00U
#define PDSTAT_STATE_ON                 0x01U
#define PDSTAT_STATE_ON_ARB             0x10U
#define PDSTAT_STATE_SWITCH_ON          0x11U
#define PDSTAT_STATE_ON_RAIL_CNT        0x12U
#define PDSTAT_STATE_WAIT_ALL_SYNC_RST  0x13U
#define PDSTAT_STATE_STRETCH_POR        0x14U
#define PDSTAT_STATE_WAIT_POR_DONE      0x15U
#define PDSTAT_STATE_COUNT16            0x16U
#define PDSTAT_STATE_WAIT_ALL_SWRST_DIS 0x17U
#define PDSTAT_STATE_OFF_ARB            0x18U
#define PDSTAT_STATE_SWITCH_OFF         0x19U
#define PDSTAT_STATE_OFF_RAIL_CNT       0x1aU

#define PDCTL_STATE_MASK		BIT(0)
#define PDCTL_EPCGOOD			BIT(8)  /* External power control on */
#define PDCTL_EMUIHBIE			BIT(9)  /* EMU alters domain state IE */
#define PDCTL_FORCE			BIT(31)
/* TODO: Recheck the timeout value */
#define PSC_TRANSITION_TIMEOUT  100000

#define mmio_read_32 mmio_read_32
#define mmio_write_32 mmio_write_32

__wkupsramfunc int32_t psc_raw_pd_wait(uint64_t psc_base, uint8_t pd)
{
	int32_t ret = 0;
	int32_t i = PSC_TRANSITION_TIMEOUT;

	while (((mmio_read_32(psc_base + PSC_PTSTAT) & BIT(pd)) != 0U) && (i != 0)) {
		k3_lpm_delay_1us();
		i--;
	}

	if (i == 0) {
		ret = -ETIMEDOUT;
	}
	return ret;
}

__wkupsramfunc void psc_raw_pd_initiate(uint64_t psc_base, uint8_t pd)
{
	mmio_write_32(psc_base + PSC_PTCMD, BIT(pd));
}

__wkupsramfunc void psc_raw_pd_set_state(uint64_t psc_base, uint8_t pd, uint32_t state, bool force)
{
	uint32_t pdctl = mmio_read_32(psc_base + PSC_PDCTL(pd));

	pdctl &= ~PDCTL_STATE_MASK;
	pdctl |= state;

	if (force) {
		pdctl |= PDCTL_FORCE;
	} else {
		pdctl &= ~PDCTL_FORCE;
	}

	mmio_write_32(psc_base + PSC_PDCTL(pd), pdctl);
}

__wkupsramfunc void psc_raw_lpsc_set_state(uint64_t psc_base, uint8_t lpsc, uint32_t state, bool force)
{
	uint32_t mdctl = mmio_read_32(psc_base + PSC_MDCTL(lpsc));

	mdctl &= ~MDCTL_STATE_MASK;
	mdctl |= state;

	if (force) {
		mdctl |= MDCTL_FORCE;
	} else {
		mdctl &= ~MDCTL_FORCE;
	}

	mmio_write_32(psc_base + PSC_MDCTL(lpsc), mdctl);
}

/*
 * Copyright 2022-2026 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/runtime_svc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <imx_sip_svc.h>
#include <plat_imx8.h>
#include <platform_def.h>

#define M33_CFG_OFF		0x60U
#define M33_INITSVTOR		0x5CU
#define M33_CPU_WAIT_MASK	BIT(2)

#define SRC_GLOBAL_RBASE		(SRC_BASE + 0x0000U)
#define SRC_M33P_RBASE			(SRC_BASE + 0x2800U)
#define REG_SRC_GLOBAL_CM_QUIESCE	(SRC_GLOBAL_RBASE + 0x104U)
#define REG_SRC_GLOBAL_SRMASK		(SRC_GLOBAL_RBASE + 0x18U)
#define REG_SRC_GLOBAL_SRMR1		(SRC_GLOBAL_RBASE + 0x20U)
#define REG_SRC_GLOBAL_SRMR5		(SRC_GLOBAL_RBASE + 0x30U)
#define REG_SRC_GLOBAL_GPR9		(SRC_GLOBAL_RBASE + 0x9CU)

#define REG_SRC_M33P_FUNC_STAT		(SRC_M33P_RBASE + 0xB4U)

#define SRC_MIX_SLICE_FUNC_STAT_RST_STAT	BIT(2)

#define M33_DEFAULT_ENTRY		0xFFE0000U

static void disable_wdog(uintptr_t wdog_base)
{
	uint32_t val_cs = mmio_read_32(wdog_base + WDOG_CS);

	/* Check if watchdog is already disabled */
	if ((val_cs & WDOG_CS_EN) == 0U) {
		return;
	}

	/* Refresh the counter (default is 32-bit command) */
	mmio_write_32(wdog_base + WDOG_CNT, WDOG_REFRESH);

	/* Unlock the watchdog if not already unlocked */
	if ((val_cs & WDOG_CS_ULK) == 0U) {
		mmio_write_32(wdog_base + WDOG_CNT, WDOG_UNLOCK);
		while ((mmio_read_32(wdog_base + WDOG_CS) & WDOG_CS_ULK) == 0U) {
			/* Wait for unlock */
		}
	}

	/* Set WIN to 0 */
	mmio_write_32(wdog_base + WDOG_WIN, 0U);

	/* Set timeout to default */
	mmio_write_32(wdog_base + WDOG_TOVAL, WDOG_TIMEOUT_DEFAULT);

	/* Disable watchdog and set update */
	mmio_write_32(wdog_base + WDOG_CS, WDOG_CONFIG_VALUE);

	/* Wait for reconfiguration to complete */
	while ((mmio_read_32(wdog_base + WDOG_CS) & WDOG_CS_RCS) == 0U) {
		/* Wait for RCS bit */
	}
}

static void wdog1_trigger_reset(void)
{
	disable_wdog(WDOG1_BASE);

	/* Unlock watchdog (CMD32EN=1 by default, use 32-bit unlock sequence) */
	mmio_write_32(WDOG1_BASE + WDOG_CNT, WDOG_UNLOCK);

	/* Wait for watchdog unlock */
	while ((mmio_read_32(WDOG1_BASE + WDOG_CS) & WDOG_CS_ULK) == 0U) {
		/* Wait for unlock */
	}

	/* Set short timeout */
	mmio_write_32(WDOG1_BASE + WDOG_TOVAL, WDOG_TIMEOUT_SHORT);
	mmio_write_32(WDOG1_BASE + WDOG_WIN, 0U);

	/* Enable watchdog with LPO clock */
	mmio_write_32(WDOG1_BASE + WDOG_CS,
		      (WDOG_ENABLE_CONFIG | WDOG_CS_CMD32EN | WDOG_CS_EN |
		       WDOG_CS_UPDATE | (WDOG_LPO_CLK << 8) | WDOG_CS_FLG));

	/* Wait for watchdog reconfiguration */
	while ((mmio_read_32(WDOG1_BASE + WDOG_CS) & WDOG_CS_RCS) == 0U) {
		/* Wait for RCS bit */
	}
}

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	uint32_t val;

	switch (x1) {
	case IMX_SIP_SRC_M4_START:
		/* Set M33 initial vector table address */
		mmio_write_32(BLK_CTRL_S_BASE + M33_INITSVTOR, 0U);

		/* Configure GPR9 for M33 firmware loading */
		if (x2 != 0U) {
			mmio_write_32(REG_SRC_GLOBAL_GPR9, x2);
		} else {
			mmio_write_32(REG_SRC_GLOBAL_GPR9, M33_DEFAULT_ENTRY);
		}

		/* Release M33 from wait state */
		mmio_clrbits_32(BLK_CTRL_S_BASE + M33_CFG_OFF,
				M33_CPU_WAIT_MASK);
		break;

	case IMX_SIP_SRC_M4_STARTED:
		/* Check if M33 is in wait state */
		val = mmio_read_32(BLK_CTRL_S_BASE + M33_CFG_OFF) &
		      M33_CPU_WAIT_MASK;
		return (val != 0U) ? 0U : 1U;

	case IMX_SIP_SRC_M4_STOP:
		/* Configure reset manager registers */
		mmio_setbits_32(REG_SRC_GLOBAL_SRMR1, BIT(8) | BIT(24));
		mmio_setbits_32(REG_SRC_GLOBAL_SRMR5, BIT(8));
		mmio_clrbits_32(REG_SRC_GLOBAL_SRMASK, BIT(1) | BIT(0));

		/* Put M33 into wait state */
		mmio_setbits_32(BLK_CTRL_S_BASE + M33_CFG_OFF,
				M33_CPU_WAIT_MASK);

		/* Clear CM quiesce */
		mmio_clrbits_32(REG_SRC_GLOBAL_CM_QUIESCE, BIT(0));

		/* Trigger watchdog reset */
		wdog1_trigger_reset();

		/* Wait for quiesce acknowledgment */
		udelay(5);

		val = mmio_read_32(REG_SRC_GLOBAL_CM_QUIESCE);
		while ((val & BIT(1)) != 0U) {
			val = mmio_read_32(REG_SRC_GLOBAL_CM_QUIESCE);
		}

		/* Wait for M33 reset status */
		val = mmio_read_32(REG_SRC_M33P_FUNC_STAT);
		while ((val & SRC_MIX_SLICE_FUNC_STAT_RST_STAT) == 0U) {
			val = mmio_read_32(REG_SRC_M33P_FUNC_STAT);
		}

		SMC_SET_GP(handle, CTX_GPREG_X1, 0U);
		SMC_SET_GP(handle, CTX_GPREG_X2, 0U);
		break;

	default:
		return SMC_UNK;
	}

	return 0;
}

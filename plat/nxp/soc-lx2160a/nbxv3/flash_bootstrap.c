/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * nbxv3 flash bootstrap orchestrator. Pure policy: detect that
 * the Service Processor fell back to its boot-ROM hard-coded
 * RCW, run the JSON-manifest-driven NOR provisioning, then signal
 * the host (OpenOCD) via SYS_EXIT semihosting that we are done.
 *
 * The detection rule is: read DCFG_RCWSR0.MEM_PLL_RAT, a value
 * below the DDR4-1600 DLL-lock minimum (16) means the live RCW
 * cannot train DDR and must therefore be a ROM fallback. On a
 * provisioned board (production RCW with MEM_PLL_RAT=24) this
 * function returns silently and BL2 proceeds to DDR init.
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <flash_vendor_gigadevice.h>
#include <fspi_api.h>
#include <lib/mmio.h>
#include <lib/semihosting.h>
#include <xspi_nor_diag.h>

#include "flash_bootstrap.h"
#include "manifest_parse.h"
#include "manifest_runner.h"
#include "nbxv3_runtime_mode.h"
#include <soc.h>

/* Hard-coded RCW signature: MEM_PLL_RAT below DDR4 DLL-lock min. */
#define NBXV3_DDR4_MIN_RAT		16U

/*
 * 100 MHz SYSCLK from the on-board DPLL. Used to compute the
 * FSPI root clock from the live RCW SYS_PLL_RAT.
 */
#define NBXV3_SYSCLK_HZ			100000000U

/*
 * Default SCK ceiling when the manifest does not set spi_mhz.
 *
 * Per GD55LB02GF datasheet:
 *   - read commands  SCK <= 166 MHz
 *   - program/erase  SCK <=  50 MHz
 * The bootstrap path does sector erase + program + verify, so the
 * 50 MHz ceiling rules. We pick 150 MHz as the requested default
 * because fspi_set_serclk_max_hz() picks the smallest SERCLKDIV
 * that keeps SCK at or below the request, and the RCW chain caps
 * the root clock well below 150 MHz on this board:
 *
 *   fspi_root_hz = SYSCLK * SYS_PLL_RAT / 2 / 2
 *
 *   - hardcoded RCW (SP fallback): SYS_PLL_RAT=12 -> platform=600 MHz
 *                                                 -> fspi_root=300 MHz
 *     SERCLKDIV picks 4 -> SCK = 300/(4+1) =  60 MHz (>50; tolerated
 *     by GD55 for short-burst program windows on this board, no
 *     bit errors observed in the verify step).
 *   - production RCW: SYS_PLL_RAT=14 -> platform=700 MHz
 *                                    -> fspi_root=350 MHz
 *     SERCLKDIV picks 4 -> SCK = 350/(4+1) =  70 MHz.
 *
 * The manifest may override via spi_mhz (e.g. preset to 30 MHz on
 * a marginal harness). 150 MHz is just the safe ceiling that lets
 * the divisor logic land at the right value autonomously.
 */
#define NBXV3_FSPI_DEFAULT_MAX_SCK_HZ	(150ULL * 1000000ULL)

/* Manifest file name. Resolved by the host's semihost basedir. */
#define NBXV3_MANIFEST_PATH		"flash_manifest.json"

static uint32_t nbxv3_mem_pll_rat(void)
{
	uint32_t rcwsr0 = mmio_read_32(NXP_DCFG_ADDR + RCWSR0_OFFSET);

	return (rcwsr0 >> RCWSR0_MEM_PLL_RAT_SHIFT) & RCWSR0_MEM_PLL_RAT_MASK;
}

static uint64_t nbxv3_fspi_root_hz(void)
{
	uint32_t rcwsr0 = mmio_read_32(NXP_DCFG_ADDR + RCWSR0_OFFSET);
	uint32_t rat = (rcwsr0 >> RCWSR0_SYS_PLL_RAT_SHIFT) &
		       RCWSR0_SYS_PLL_RAT_MASK;
	uint64_t platform_hz = (uint64_t)NBXV3_SYSCLK_HZ * (uint64_t)rat / 2ULL;

	return platform_hz / 2ULL;
}

/*
 * GD55 SR3 layout decode. Vendor-specific (Table 6 of the
 * GD55LB02GF datasheet); kept here rather than in the generic
 * NOR diag because SR3's bit layout is not standard.
 */
static void nbxv3_print_sr3(const char *label, uint8_t sr3)
{
	NOTICE("nbxv3 bootstrap: SR3 %s = 0x%02x [ADP=%u ADS=%u DC1=%u DC0=%u]; %s-byte mode\n",
	       label, sr3,
	       (sr3 >> 4) & 1U, (sr3 >> 3) & 1U,
	       (sr3 >> 1) & 1U, (sr3 >> 0) & 1U,
	       ((sr3 >> 3) & 1U) ? "4" : "3");
}

/*
 * GD55 pre-flight: Global VL Unlock + VL/NL snapshot. The
 * GD55LB02GF NVCR<02>.bit2 powers up at 1, so every block boots
 * with VL=0xFF (LOCKED) and silently drops SE/PP. Issue 0x98
 * before the first erase.
 */
static void nbxv3_gd_preflight(void)
{
	uint8_t sr_before = 0U;
	uint8_t sr_after = 0U;
	uint8_t vl0 = 0xFFU;
	uint8_t nl0 = 0xFFU;

	(void)fspi_read_sr1(&sr_before);
	xspi_print_sr1("before GBL_UL", sr_before);

	if (gd_global_vl_unlock() != 0) {
		ERROR("nbxv3 bootstrap: gd_global_vl_unlock failed\n");
		panic();
	}
	NOTICE("nbxv3 bootstrap: Global VL Unlock (0x98) completed\n");

	(void)fspi_read_sr1(&sr_after);
	xspi_print_sr1("after  GBL_UL", sr_after);

	(void)gd_read_vl_register(0U, &vl0);
	(void)gd_read_nl_register(0U, &nl0);
	NOTICE("nbxv3 bootstrap: VL[block0] = 0x%02x (%s), NL[block0] = 0x%02x (%s)\n",
	       vl0, (vl0 == 0x00U) ? "unlocked" :
		    (vl0 == 0xFFU) ? "LOCKED"   : "invalid",
	       nl0, (nl0 == 0x00U) ? "unlocked" :
		    (nl0 == 0xFFU) ? "LOCKED"   : "invalid");
}

/*
 * Hand back to the host via ARM semihosting SYS_EXIT.
 */
static __dead2 void nbxv3_finish_via_semihost_exit(long total)
{
	NOTICE("\n");
	NOTICE("============================================================\n");
	NOTICE("nbxv3 bootstrap: DONE %ld payload bytes programmed across\n",
	       total);
	NOTICE("                    the manifest regions.\n");
	NOTICE("  Signalling the host via SYS_EXIT; the host shall power-cycle\n");
	NOTICE("  the board so the SP re-samples RCW from the freshly flashed\n");
	NOTICE("  NOR on the next boot.\n");
	NOTICE("============================================================\n");

	/* Drain the NOTICE buffer onto UART before the host halts us. */
	mdelay(200U);

	semihosting_exit(ADP_Stopped_ApplicationExit, 0U);

	/* SYS_EXIT does not return */
	for (;;) {
		wfi();
	}
}

void __dead2 nbxv3_flash_bootstrap(void)
{
	const struct nbxv3_manifest *m;
	uint64_t root_hz;
	uint64_t max_sck_hz;
	long total;

	NOTICE("nbxv3 bootstrap: hard-coded RCW in effect (MEM_PLL_RAT=%u, < %u);\n",
	       nbxv3_mem_pll_rat(), NBXV3_DDR4_MIN_RAT);
	NOTICE("  Service Processor fell back to its boot-ROM RCW; provisioning\n");
	NOTICE("  the on-board NOR over ARM semihosting.\n");

	NOTICE("nbxv3 bootstrap: initialising FlexSPI NOR controller @ 0x%x\n",
	       NXP_FLEXSPI_ADDR);
	if (fspi_init(NXP_FLEXSPI_ADDR, NXP_FLEXSPI_FLASH_ADDR) != 0) {
		ERROR("nbxv3 bootstrap: fspi_init failed\n");
		panic();
	}

	/*
	 * Load the manifest before clamping SCK so spi_mhz from the
	 * manifest can override the default. The manifest itself is
	 * read over semihosting through the direct semihosting
	 * library, the io_storage path comes online later for the
	 * payload reads.
	 */
	if (nbxv3_manifest_load(NBXV3_MANIFEST_PATH) != 0) {
		ERROR("nbxv3 bootstrap: failed to load %s\n",
		      NBXV3_MANIFEST_PATH);
		panic();
	}
	m = nbxv3_manifest_get();

	root_hz = nbxv3_fspi_root_hz();
	max_sck_hz = (m->spi_mhz != 0U)
		? ((uint64_t)m->spi_mhz * 1000000ULL)
		: NBXV3_FSPI_DEFAULT_MAX_SCK_HZ;
	fspi_print_speed(root_hz);
	fspi_set_serclk_max_hz(root_hz, max_sck_hz);
	fspi_print_speed(root_hz);

	NOTICE("nbxv3 bootstrap: issuing EN4B (0xB7) for 4-byte address mode\n");
	(void)fspi_enter_4byte_mode();
	{
		uint8_t sr3 = 0U;

		(void)fspi_read_sr3(&sr3);
		nbxv3_print_sr3("after EN4B", sr3);
	}

	nbxv3_gd_preflight();
	(void)xspi_probe_rdid();
	xspi_probe_sfdp();
	(void)xspi_probe_status();

	total = nbxv3_manifest_run(m);
	if (total < 0) {
		ERROR("nbxv3 bootstrap: manifest run failed (%ld)\n", total);
		panic();
	}

	nbxv3_finish_via_semihost_exit(total);
	/* no return */
}

/*
 * Bootrom RCW is in effect but the OpenOCD attach-magic was not seen
 * in DCFG_SCRATCHRW2. Either the operator power-cycled into recovery
 * without arming the host first, or the host is booting up but hasn't
 * reached `nbxv3_set_mode_flash_bootstrap` yet. Poll scratch every
 * 200 ms for 5 seconds: if the magic appears, dispatch into the flash
 * bootstrap, otherwise issue SW_RST_REQ_INIT (which on this board
 * feeds back to PORESET_B) so the SP boot loop retries from scratch.
 *
 * The retry loop avoids to power-cycle the CPU board PDU again, you can
 * attach OpenOCD a few seconds late and the next boot cycle picks up the magic.
 */
#define NBXV3_NO_HOST_TIMEOUT_MS	5000U
#define NBXV3_NO_HOST_POLL_MS		200U
#define NBXV3_DCFG_SCRATCHRW2_OFFSET	0x204U

void __dead2 nbxv3_wait_or_por_retry(void)
{
	uint32_t magic;
	unsigned int waited_ms = 0U;

	NOTICE("\n");
	NOTICE("nbxv3 bootstrap: bootrom RCW active but no OpenOCD magic seen.\n");
	NOTICE("  attach OpenOCD with `nbxv3_set_mode_flash_bootstrap` within %u s,\n",
	       NBXV3_NO_HOST_TIMEOUT_MS / 1000U);
	NOTICE("  or BL2 will trigger SW_RST_REQ_INIT and the SP will retry boot.\n");

	while (waited_ms < NBXV3_NO_HOST_TIMEOUT_MS) {
		mdelay(NBXV3_NO_HOST_POLL_MS);
		waited_ms += NBXV3_NO_HOST_POLL_MS;

		magic = mmio_read_32(NXP_DCFG_ADDR +
				     NBXV3_DCFG_SCRATCHRW2_OFFSET);
		if (magic == NBXV3_MAGIC_FLASH) {
			mmio_write_32(NXP_DCFG_ADDR +
				      NBXV3_DCFG_SCRATCHRW2_OFFSET, 0U);
			NOTICE("nbxv3 bootstrap: host attached at t=%u ms; "
			       "switching to FLASH_BOOTSTRAP\n", waited_ms);
			nbxv3_flash_bootstrap();
			/* no return */
		}
	}

	NOTICE("nbxv3 bootstrap: no host after %u ms; issuing SW_RST_REQ_INIT.\n",
	       NBXV3_NO_HOST_TIMEOUT_MS);
	mdelay(50U);
	mmio_write_32(NXP_RST_ADDR + RSTCNTL_OFFSET, SW_RST_REQ_INIT);
	mdelay(1000U);

	/* If the reset somehow did not fire, park here. */
	NOTICE("nbxv3 bootstrap: SW_RST_REQ_INIT did not propagate; halting.\n");
	for (;;) {
		wfi();
	}
}

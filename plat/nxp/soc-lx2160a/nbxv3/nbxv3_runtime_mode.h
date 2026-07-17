/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Runtime classification of the nbxv3 BL2 boot scenario.
 *
 * One BL2 source tree, two link layouts (XIP NOR vs OCRAM bootstrap).
 * Whichever binary is running, this classifier picks the mode at
 * boot by reading two registers:
 *
 *   1. RCWSR0.MEM_PLL_RAT: below the DDR4 DLL-lock minimum (16)
 *      means the SP fell back to its boot-ROM hard-coded RCW (no
 *      DDR4 RCW would set such a low ratio). Production RCWs always
 *      land >= 16.
 *
 *   2. DCFG_SCRATCHRW2: a software-only register the boot ROM
 *      and PBI never touch. OpenOCD writes a magic word here right
 *      before cpu.0 release; BL2 reads it once, classifies, then
 *      zeroes it (so a soft re-entry classifies cleanly).
 *
 * Mode table:
 *
 *   MEM_PLL_RAT       SCRATCHRW2     mode
 *   ---------------   ------------   ----------------------
 *   < 16 (bootrom)    MAGIC_FLASH    FLASH_BOOTSTRAP
 *   < 16 (bootrom)    other          FLASH_NO_HOST (5 s wait + POR)
 *   >= 16 (prod RCW)  any            PROD (normal autonomous boot)
 */

#ifndef NBXV3_RUNTIME_MODE_H
#define NBXV3_RUNTIME_MODE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	NBXV3_MODE_PROD,            /* NOR boot: normal autonomous flow */
	NBXV3_MODE_FLASH_BOOTSTRAP, /* bootrom RCW + OpenOCD attached     */
	NBXV3_MODE_FLASH_NO_HOST,   /* bootrom RCW, no OpenOCD -- POR retry */
} nbxv3_mode_t;

/*
 * Magic word written by OpenOCD into DCFG_SCRATCHRW2 before cpu.0
 * release. Drives mode = FLASH_BOOTSTRAP (only meaningful when the
 * SP loaded a bootrom RCW. OpenOCD will then have load_image'd the
 * OCRAM bootstrap BL2 which runs nbxv3_flash_bootstrap()).
 */
#define NBXV3_MAGIC_FLASH		0xB007F1A5U /* "BOOT-FLAS" */

/*
 * AArch64 ARM Semihosting clean-exit reason. Used at the end of
 * nbxv3_flash_bootstrap() so OpenOCD's wait_halt returns. cpu.0
 * stays halted (NOR is freshly flashed, the host then power-cycles
 * to drop into prod boot).
 */
#define ADP_Stopped_ApplicationExit	0x20026U

/*
 * Classify the current BL2 invocation. Reads MEM_PLL_RAT + SCRATCHRW2,
 * consumes any recognised magic, and returns the mode. SCRATCHRW2 is
 * always zeroed before return.
 */
nbxv3_mode_t nbxv3_runtime_mode(void);

/* Symbolic getter for the mode; useful for diagnostics. */
const char *nbxv3_mode_name(nbxv3_mode_t mode);

#endif /* NBXV3_RUNTIME_MODE_H */

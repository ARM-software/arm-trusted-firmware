/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 */

#include <common/debug.h>
#include <dcfg.h>
#include <lib/mmio.h>

#include "nbxv3_runtime_mode.h"
#include <soc.h>

/*
 * DDR4 DLL-lock minimum (LX2160A RM "DDR Controller" chapter): below
 * this MEM_PLL_RAT, no DDR4-1600 timing is reachable. Production RCWs
 * always set MEM_PLL_RAT >= 16; the boot-ROM hard-coded RCWs use a
 * lower ratio (specifically 12 on this revision), so MEM_PLL_RAT < 16
 * is a reliable "the SP fell back to bootrom" signal.
 */
#define NBXV3_DDR4_MIN_RAT		16U

/*
 * DCFG_SCRATCHRW2 lives at DCFG offset 0x204 on every Layerscape
 * SoC we care about. The boot ROM and PBI never write to it; software
 * (OpenOCD or our own BL2) is the only writer.
 */
#define NBXV3_DCFG_SCRATCHRW2_OFFSET	0x204U

static uint32_t nbxv3_read_mem_pll_rat(void)
{
	uint32_t rcwsr0 = mmio_read_32(NXP_DCFG_ADDR + RCWSR0_OFFSET);

	return (rcwsr0 >> RCWSR0_MEM_PLL_RAT_SHIFT) & RCWSR0_MEM_PLL_RAT_MASK;
}

static uint32_t nbxv3_read_scratchrw2(void)
{
	return mmio_read_32(NXP_DCFG_ADDR + NBXV3_DCFG_SCRATCHRW2_OFFSET);
}

static void nbxv3_clear_scratchrw2(void)
{
	mmio_write_32(NXP_DCFG_ADDR + NBXV3_DCFG_SCRATCHRW2_OFFSET, 0U);
}

nbxv3_mode_t nbxv3_runtime_mode(void)
{
	uint32_t mem_pll_rat = nbxv3_read_mem_pll_rat();
	uint32_t magic = nbxv3_read_scratchrw2();
	bool bootrom_rcw = (mem_pll_rat < NBXV3_DDR4_MIN_RAT);

	/* Always consume the scratch slot, even on unrecognised values. */
	nbxv3_clear_scratchrw2();

	if (bootrom_rcw) {
		return (magic == NBXV3_MAGIC_FLASH)
			? NBXV3_MODE_FLASH_BOOTSTRAP
			: NBXV3_MODE_FLASH_NO_HOST;
	}

	return NBXV3_MODE_PROD;
}

const char *nbxv3_mode_name(nbxv3_mode_t mode)
{
	switch (mode) {
	case NBXV3_MODE_PROD:            return "PROD";
	case NBXV3_MODE_FLASH_BOOTSTRAP: return "FLASH_BOOTSTRAP";
	case NBXV3_MODE_FLASH_NO_HOST:   return "FLASH_NO_HOST";
	default:                         return "?";
	}
}

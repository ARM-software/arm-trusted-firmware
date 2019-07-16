/*
 * Copyright (c) 2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

#include <lib/mmio.h>

#define CPG_BASE		0xE6150000
#define CPG_MSTPSR3		0x0048
#define MSTP318			(1 << 18)
#define MSTP319			(1 << 19)
#define PMSR			0x5c
#define PMSR_L1FAEG		(1U << 31)
#define PMSR_PMEL1RX		(1 << 23)
#define PMCTLR			0x60
#define PMSR_L1IATN		(1U << 31)

static int rcar_pcie_fixup(unsigned int controller)
{
	uint32_t rcar_pcie_base[] = { 0xfe011000, 0xee811000 };
	uint32_t addr = rcar_pcie_base[controller];
	uint32_t cpg, pmsr;
	int ret = 0;

	/* Test if PCIECx is enabled */
	cpg = mmio_read_32(CPG_BASE + CPG_MSTPSR3);
	if (cpg & (MSTP318 << !controller))
		return ret;

	pmsr = mmio_read_32(addr + PMSR);

	if ((pmsr & PMSR_PMEL1RX) && ((pmsr & 0x70000) != 0x30000)) {
		/* Fix applicable */
		mmio_write_32(addr + PMCTLR, PMSR_L1IATN);
		while (!(mmio_read_32(addr + PMSR) & PMSR_L1FAEG))
			;
		mmio_write_32(addr + PMSR, PMSR_L1FAEG | PMSR_PMEL1RX);
		ret = 1;
	}

	return ret;
}

/* RAS functions common to AArch64 ARM platforms */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
	unsigned int fixed = 0;

	fixed |= rcar_pcie_fixup(0);
	fixed |= rcar_pcie_fixup(1);

	if (fixed)
		return;

	ERROR("Unhandled External Abort received on 0x%lx at EL3!\n",
			read_mpidr_el1());
	ERROR(" exception reason=%u syndrome=0x%llx\n", ea_reason, syndrome);

	panic();
}

#include <drivers/renesas/rcar/console/console.h>

static console_rcar_t rcar_boot_console;
static console_rcar_t rcar_runtime_console;

void rcar_console_boot_init(void)
{
	int ret;

	ret = console_rcar_register(0, 0, 0, &rcar_boot_console);
	if (!ret)
		panic();

	console_set_scope(&rcar_boot_console.console, CONSOLE_FLAG_BOOT);
}

void rcar_console_boot_end(void)
{
}

void rcar_console_runtime_init(void)
{
	int ret;

	ret = console_rcar_register(1, 0, 0, &rcar_runtime_console);
	if (!ret)
		panic();

	console_set_scope(&rcar_boot_console.console, CONSOLE_FLAG_RUNTIME);
}

void rcar_console_runtime_end(void)
{
}

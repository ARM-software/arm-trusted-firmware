/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>

static void brcm_stingray_pnor_pinmux_init(void)
{
	unsigned int i;

	INFO(" - pnor pinmux init start.\n");

	/* Set PNOR_ADV_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2dc),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_BAA_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2e0),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_BLS_0_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2e4),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_BLS_1_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2e8),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_CRE_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2ec),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_CS_2_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2f0),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_CS_1_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2f4),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_CS_0_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2f8),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_WE_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x2fc),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_OE_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x300),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_INTR_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x304),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set PNOR_DAT_x_MODE_SEL_CONTROL.fsel = 0x2 */
	for (i = 0; i < 0x40; i += 0x4) {
		mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x308 + i),
				   MODE_SEL_CONTROL_FSEL_MASK,
				   MODE_SEL_CONTROL_FSEL_MODE2);
	}

	/* Set NAND_CE1_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x348),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_CE0_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x34c),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_WE_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x350),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_WP_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x354),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_RE_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x358),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_RDY_BSY_N_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x35c),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_IOx_0_MODE_SEL_CONTROL.fsel = 0x2 */
	for (i = 0; i < 0x40; i += 0x4) {
		mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x360 + i),
				   MODE_SEL_CONTROL_FSEL_MASK,
				   MODE_SEL_CONTROL_FSEL_MODE2);
	}

	/* Set NAND_ALE_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x3a0),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	/* Set NAND_CLE_MODE_SEL_CONTROL.fsel = 0x2 */
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x3a4),
			   MODE_SEL_CONTROL_FSEL_MASK,
			   MODE_SEL_CONTROL_FSEL_MODE2);

	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x40), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x44), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x48), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x4c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x50), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x54), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x58), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x5c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x60), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x64), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x68), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x6c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x70), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x74), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x78), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x7c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x80), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x84), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x88), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x8c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x90), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x94), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x98), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0x9c), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0xa0), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0xa4), (7 << 1), 0x8);
	mmio_clrsetbits_32((uintptr_t)(HSLS_IOPAD_BASE + 0xa8), (7 << 1), 0x8);

	INFO(" - pnor pinmux init done.\n");
}

#if BL2_TEST_EXT_SRAM
#define SRAM_CHECKS_GRANUL	0x100000
#define SRAM_CHECKS_CNT		8
static unsigned int sram_checks[SRAM_CHECKS_CNT] = {
	/* offset, magic */
	0xd00dfeed,
	0xfadebabe,
	0xc001d00d,
	0xa5a5b5b5,
	0x5a5a5b5b,
	0xc5c5d5d5,
	0x5c5c5d5d,
	0xe5e5f5f5,
};
#endif

static void brcm_stingray_pnor_sram_init(void)
{
	unsigned int val, tmp;
#if BL2_TEST_EXT_SRAM
	unsigned int off, i;
#endif
	INFO(" - pnor sram init start.\n");

	/* Enable PNOR Clock */
	INFO(" -- enable pnor clock\n");
	mmio_write_32((uintptr_t)(PNOR_IDM_IO_CONTROL_DIRECT), 0x1);
	udelay(500);

	/* Reset PNOR */
	INFO(" -- reset pnor\n");
	mmio_setbits_32((uintptr_t)(PNOR_IDM_IO_RESET_CONTROL), 0x1);
	udelay(500);
	mmio_clrbits_32((uintptr_t)(PNOR_IDM_IO_RESET_CONTROL), 0x1);
	udelay(500);

	/* Configure slave address to chip-select mapping */
	INFO(" -- configure pnor slave address to chip-select mapping\n");
	/* 0x74000000-0x75ffffff => CS0 (32MB) */
	val = (0xfe << PNOR_ICFG_CS_x_MASK0_SHIFT);
	val |= (0x74);
	mmio_write_32((uintptr_t)(PNOR_ICFG_CS_0), val);
	/* 0x76000000-0x77ffffff => CS1 (32MB) */
	val = (0xfe << PNOR_ICFG_CS_x_MASK0_SHIFT);
	val |= (0x76);
	mmio_write_32((uintptr_t)(PNOR_ICFG_CS_1), val);
	/* 0xffffffff-0xffffffff => CS2 (0MB) */
	val = (0x00 << PNOR_ICFG_CS_x_MASK0_SHIFT);
	val |= (0xff);
	mmio_write_32((uintptr_t)(PNOR_ICFG_CS_2), val);

	/* Print PNOR ID */
	tmp = 0x0;
	val = mmio_read_32((uintptr_t)(PNOR_REG_PERIPH_ID0));
	tmp |= (val & PNOR_REG_PERIPH_IDx_MASK);
	val = mmio_read_32((uintptr_t)(PNOR_REG_PERIPH_ID1));
	tmp |= ((val & PNOR_REG_PERIPH_IDx_MASK) << 8);
	val = mmio_read_32((uintptr_t)(PNOR_REG_PERIPH_ID2));
	tmp |= ((val & PNOR_REG_PERIPH_IDx_MASK) << 16);
	val = mmio_read_32((uintptr_t)(PNOR_REG_PERIPH_ID3));
	tmp |= ((val & PNOR_REG_PERIPH_IDx_MASK) << 24);
	INFO(" -- pnor primecell_id = 0x%x\n", tmp);

	/* PNOR set_cycles */
#ifdef EMULATION_SETUP
	val = 0x00129A44;
#else
	val = 0x00125954; /* 0x00002DEF; */
#endif
	mmio_write_32((uintptr_t)(PNOR_REG_SET_CYCLES), val);
	INFO(" -- pnor set_cycles = 0x%x\n", val);

	/* PNOR set_opmode */
	val = 0x0;
#ifdef EMULATION_SETUP
	/* TODO: Final values to be provided by DV folks */
	val &= ~(0x7 << 7); /* set_wr_bl */
	val &= ~(0x7 << 3);  /* set_rd_bl */
	val &= ~(0x3);
	val |= (0x1); /* set_mw */
#else
	/* TODO: Final values to be provided by DV folks */
	val &= ~(0x7 << 7); /* set_wr_bl */
	val &= ~(0x7 << 3);  /* set_rd_bl */
	val &= ~(0x3);
	val |= (0x1); /* set_mw */
#endif
	mmio_write_32((uintptr_t)(PNOR_REG_SET_OPMODE), val);
	INFO(" -- pnor set_opmode = 0x%x\n", val);

#ifndef EMULATION_SETUP
	/* Actual SRAM chip will require self-refresh */
	val = 0x1;
	mmio_write_32((uintptr_t)(PNOR_REG_REFRESH_0), val);
	INFO(" -- pnor refresh_0 = 0x%x\n", val);
#endif

#if BL2_TEST_EXT_SRAM
	/* Check PNOR SRAM access */
	for (off = 0; off < NOR_SIZE; off += SRAM_CHECKS_GRANUL) {
		i = (off / SRAM_CHECKS_GRANUL) % SRAM_CHECKS_CNT;
		val = sram_checks[i];
		INFO(" -- pnor sram write addr=0x%lx value=0x%lx\n",
		     (unsigned long)(NOR_BASE_ADDR + off),
		     (unsigned long)val);
		mmio_write_32((uintptr_t)(NOR_BASE_ADDR + off), val);
	}
	tmp = 0;
	for (off = 0; off < NOR_SIZE; off += SRAM_CHECKS_GRANUL) {
		i = (off / SRAM_CHECKS_GRANUL) % SRAM_CHECKS_CNT;
		val = mmio_read_32((uintptr_t)(NOR_BASE_ADDR + off));
		INFO(" -- pnor sram read addr=0x%lx value=0x%lx\n",
		     (unsigned long)(NOR_BASE_ADDR + off),
		     (unsigned long)val);
		if (val == sram_checks[i])
			tmp++;
	}
	INFO(" -- pnor sram checks pass=%d total=%d\n",
	     tmp, (NOR_SIZE / SRAM_CHECKS_GRANUL));

	if (tmp != (NOR_SIZE / SRAM_CHECKS_GRANUL)) {
		INFO(" - pnor sram init failed.\n");
		while (1)
			;
	} else {
		INFO(" - pnor sram init done.\n");
	}
#endif
}

void ext_sram_init(void)
{
	INFO("%s start.\n", __func__);

	brcm_stingray_pnor_pinmux_init();

	brcm_stingray_pnor_sram_init();

	INFO("%s done.\n", __func__);
}

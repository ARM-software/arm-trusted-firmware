/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module - platform definitions.
 */

#ifndef PLAT_DEF_H
#define PLAT_DEF_H

#include <arch.h>
#include <cortex_a72.h>
/*
 * tbbr_img_def.h: image ID definitions for Trusted Board Boot.
 * Required even without TBBR because the DDR PHY firmware images
 * (trained by the Synopsys DDR PHY Gen2) use these IDs for loading.
 */
#include <tbbr_img_def.h>

#include <policy.h>
#include <soc.h>

/*
 * Arm Generic Timer base address (RM Section 5, Interrupt Assignments).
 * The system timer control frame lives in the CCSR space. BL31 uses
 * it to configure the non-secure timer for Linux.
 * Only needed in the BL31 image (runtime firmware).
 */
#if defined(IMAGE_BL31)
#define LS_SYS_TIMCTL_BASE		0x2890000
#define PLAT_LS_NSTIMER_FRAME_ID	0
#define LS_CONFIG_CNTACR		1
#endif

/*
 * External oscillator frequencies.
 *
 * NXP_SYSCLK_FREQ: the DIFF_SYSCLK input to the SoC, from the DPLL.
 *   Must match RCW SYSCLK_FREQ (600 = 100 MHz) and the on-board
 *   DPLL/OCXO output. Used by TF-A to compute PLL ratios, timer
 *   frequencies, and UART dividers.
 *
 * NXP_DDRCLK_FREQ: the DDR reference clock input. On the Nodebox v3,
 *   the DDR PLL reference comes from DIFF_SYSCLK (DDR_REFCLK_SEL in
 *   the RCW selects the source; both are 100 MHz on this board).
 *   The actual DDR data rate (2400 MT/s) is set by MEM_PLL_RAT=24
 *   in the RCW, not here. This value is the reference input to the
 *   DDR PLL, not the data rate.
 */
#define NXP_SYSCLK_FREQ		100000000	/* 100 MHz */
#define NXP_DDRCLK_FREQ		100000000	/* 100 MHz */

/*
 * UART console configuration.
 *
 * The board's primary serial console is wired directly to the
 * LX2160A's UART1 peripheral through an on-board RS-232 level
 * shifter. Pin assignments on the SoC (LX2160A FCBGA-1517):
 *
 *   UART1_SOUT (ball B6) -> RS-232 TX on the board's console header
 *   UART1_SIN  (ball B5) -> RS-232 RX on the board's console header
 *
 * The adjacent UART1_RTS_B / UART1_CTS_B pins are not used for
 * hardware flow control on this board; the RCW muxes them
 * (UART1_RTSCTS_PMUX=2) to expose UART3.TX / UART3.RX, which is
 * a secondary UART for non-console use. Do NOT try to enable
 * CTS/RTS on this console.
 *
 * LX2160A UART register bases (see RM "System peripherals
 * memory map"):
 *   0x021C0000  UART1  <- console on nbxv3
 *   0x021D0000  UART2
 *   0x021E0000  UART3  (pinned out via UART1_RTS_B/CTS_B)
 *   0x021F0000  UART4
 *
 * 115200 baud 8N1. The UART reference clock is derived from the
 * platform clock:
 *   uart_clk = platform_clk / NXP_UART_CLK_DIVIDER
 *            = (SYSCLK * SYS_PLL_RAT / 2) / 4
 *            = (100 MHz * 14 / 2) / 4
 *            = 175 MHz
 *
 * The baud divisor (IBRD=0x5E, FBRD=0x3C) is programmed by the PBI
 * during Service-Processor boot that is set per the RCW settings.
 */
#define NXP_CONSOLE_ADDR	NXP_UART_ADDR		/* chip UART1 */
#define NXP_CONSOLE_BAUDRATE	115200

/*
 * Per-image stack sizes (in OCRAM during BL2, in DRAM during BL31).
 *
 * BL2 needs more stack when Trusted Board Boot (TBBR) is enabled
 * because certificate parsing and signature verification are
 * stack-intensive. 0x2000 (8 KB) with TBBR, 0x1000 (4 KB) without.
 *
 * BL31 is the EL3 runtime firmware; 4 KB is sufficient for the
 * PSCI (Power State Coordination Interface) call paths.
 */
#if defined(IMAGE_BL2)
#if defined(TRUSTED_BOARD_BOOT)
#define PLATFORM_STACK_SIZE	0x2000		/* 8 KB with secure boot */
#else
#define PLATFORM_STACK_SIZE	0x1000		/* 4 KB without */
#endif
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE	0x1000		/* 4 KB for EL3 runtime */
#endif

/*
 * BL2 memory layout: one source tree, two linker layouts.
 *
 *   XIP BL2 (NBXV3_BL2_BOOTSTRAP=0, BL2_IN_XIP_MEM=1, default):
 *     .text/.rodata execute from the FlexSPI NOR AHB window at
 *     0x20009000; only .data/.bss/.stack live in OCRAM. The PBI
 *     in RCW configures FlexSPI for AHB and tells
 *     the SP boot ROM to release cpu.0 with PC=0x20009000. No
 *     Block Copy is performed.
 *
 *   OCRAM BL2 (NBXV3_BL2_BOOTSTRAP=1, NOT XIP):
 *     Loaded via OpenOCD into OCRAM at BL2_BASE while cpu.0 is held off,
 *     then resumed.
 *     The SP never loaded a BL2 itself (it fell back to bootrom RCW which
 *     holds cpu.0 in reset). .text/.rodata/.data/.bss/.stack all
 *     live in OCRAM.
 *
 * Both options embed the same runtime-mode classifier
 * (nbxv3_runtime_mode.c) and dispatch identically, only the
 * linker layout differs.
 *
 * OCRAM layout (LX2160A: 256 KB @ 0x18000000):
 *   0x18000000 - 0x18009FFF : (40KB) ROM code reserved (NXP_ROM_RSVD=0xA000)
 *   0x1800A000 - 0x1800CFFF : (12KB) CSF header region (unused w/o secure boot)
 *   0x1800D000 - 0x1803FFFF : (204KB) BL2 RW (data/bss/stack) on the
 *                             XIP build, or the entire BL2 on the
 *                             OCRAM build (BL2_BASE = BL2_RW_BASE)
 *   0x18040000              : OCRAM end
 */

/*
 * soc.def pushes BL2_BASE via command-line -D. It evaluates to
 * OCRAM + ROM_RSVD + CSF_HDR = 0x1800D000, which equals BL2_RW_BASE
 * in the XIP case so both builds land at the same OCRAM address.
 */

#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE)

#if !NBXV3_BL2_BOOTSTRAP
/*
 * XIP build: add BL2_RO_* / BL2_RW_* for bl2_el3.ld.S's XIP branch.
 * .text/.rodata in NOR AHB window, .data/.bss/.stack in OCRAM.
 */
#define BL2_RO_BASE		UL(0x20009000)
#define BL2_RO_LIMIT		UL(0x20100000)	/* up to (but not incl.) FIP */
#define BL2_RW_BASE		UL(0x1800D000)
#define BL2_RW_LIMIT		BL2_LIMIT
#endif

/*
 * Unified runtime bootstrap / production BL2.
 *
 * A single BL2 build serves both factory-fresh boards (blank NOR,
 * Service Processor fell back to one of the ROM hard-coded RCWs)
 * and provisioned boards (production RCW in NOR). The decision can be
 * made at runtime by reading
 * DCFG_RCWSR0.MEM_PLL_RAT (LX2160A RM "Device Configuration"
 * chapter; the RCWSR* registers are populated by the Service
 * Processor after RCW load and reflect whatever the SP actually
 * applied, which is the only way for software to tell "production
 * RCW" from "ROM fallback RCW"):
 *
 *   MEM_PLL_RAT < 16
 *   below DDR4-1600 DLL-lock minimum; DDR4 training cannot succeed. BL2
 *   should open a staged PBL ("rcw_bootstrap.bin") over ARM semihosting,
 *   erases NOR sector 0..N, programs it, verifies, and triggers a
 *   software reset which on this board becomes a full POR.
 *
 *   MEM_PLL_RAT >= 16
 *   production RCW is live (ex: MEM_PLL_RAT=24 if the board = DDR4-2400).
 *   The bootstrap returns silently and BL2 proceeds normally into DDR
 *   init + FIP load + BL31/BL33 handoff in DRAM.
 *
 * There is no compile-time "skip DDR" knob. The bootstrap path
 * halts after the flash write and reset, so the DDR init code is
 * only ever reached when the PLLs are correctly programmed.
 */

/*
 * IO framework limits.
 *
 * MAX_IO_DEVICES: number of IO backends. On Nodebox v3 the active
 *   back-ends are:
 *     - flexspi_nor
 *       for production flash boot and for the runtime flash_bootstrap
 *       rewrite path
 *     - io_semihosting
 *       for JTAG-served FIP images
 *     - io_fip
 *       FIP unpacker layered on top
 *     - io_memmap
 *       direct XSPI AHB window reads
 *   => 4 total.
 * MAX_IO_BLOCK_DEVICES: SD / eMMC back-ends. Nodebox v3 boots only
 * from FlexSPI NOR, so no block device is in use.
 * MAX_IO_HANDLES: concurrent open file handles (BL2 opens the FIP
 * container plus one cert / payload inside it -> peak 4).
 */
#define MAX_IO_DEVICES		4
#define MAX_IO_BLOCK_DEVICES	0
#define MAX_IO_HANDLES		4

/*
 * DDR PHY Gen2 firmware image buffer.
 *
 * The Synopsys DDR PHY Gen2 requires firmware images (1D and 2D
 * training sequences) to be loaded into OCRAM before DDR init.
 * This buffer starts right after the CSF header area in OCRAM.
 * The DDR PHY firmware is temporary: once DDR training completes,
 * this OCRAM region is available for reuse by BL2.
 */
#define PHY_GEN2_FW_IMAGE_BUFFER	(NXP_OCRAM_ADDR + CSF_HDR_SZ)

/*
 * FIP (Firmware Image Package) configuration.
 *
 * The FIP contains BL31 (EL3 runtime), BL33 (U-Boot/UEFI), and
 * optionally BL32 (secure payload/OP-TEE). TF-A's IO framework
 * parses the FIP to extract each component.
 *
 * MAX_FIP_DEVICES: 2 normally (main FIP + DDR FIP), 3 if fuse
 * provisioning is enabled (adds a fuse FIP).
 */
#ifdef POLICY_FUSE_PROVISION
#define MAX_FIP_DEVICES		3
#endif

#ifndef MAX_FIP_DEVICES
#define MAX_FIP_DEVICES		2
#endif

/*
 * GICv3 interrupt assignments (RM Section 5, Interrupt Assignments).
 *
 * BL32_IRQ_SEC_PHY_TIMER (SPI 29): Secure physical timer interrupt,
 * used by BL32 (OP-TEE or other secure payload) for its tick.
 *
 * BL31_WDOG_SEC (SPI 89): Secure watchdog interrupt. TF-A's EL3
 * runtime handles this to detect firmware hangs.
 *
 * BL31_NS_WDOG_WS1 (SPI 108): Non-secure watchdog window-start
 * interrupt. Routed to EL3 so TF-A can log/handle NS watchdog
 * events before the second (reset) stage fires.
 *
 * NXP_IRQ_SEC_SGI_7 (SGI 15): Secure Software Generated Interrupt.
 * Used for inter-core signaling within the EL3 runtime (PSCI
 * power management, CPU hotplug coordination).
 *
 * Group 0 interrupts: handled at EL3 (highest priority, FIQ).
 * Group 1 Secure interrupts: handled by the secure payload (BL32).
 */
#define BL32_IRQ_SEC_PHY_TIMER	29

#define BL31_WDOG_SEC		89

#define BL31_NS_WDOG_WS1	108

#define PLAT_LS_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BL32_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define NXP_IRQ_SEC_SGI_7		15

#define PLAT_LS_G0_IRQ_PROPS(grp)	\
	INTR_PROP_DESC(BL31_WDOG_SEC, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(BL31_NS_WDOG_WS1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(NXP_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
#endif /* PLAT_DEF_H */

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module - platform security and performance policy.
 *
 * These defines control TF-A's security configuration and CCN-508
 * interconnect tuning. They are consumed by the common NXP platform
 * code during BL2 and BL31 initialization.
 *
 * About CCN-508.
 *
 * CCN-508 is ARM's "Cache Coherent Network, 508 variant", the on-die
 * system interconnect IP licensed by NXP for the LX2160A (and other
 * Layerscape Chassis-3 SoCs: LS2080, LS2088, LX2162). It implements
 * the AMBA 5 CHI (Coherent Hub Interface) protocol and provides
 * cache coherency across all CPU clusters via a snoop directory,
 * plus a single point of coherence and serialisation for shared
 * memory.
 *
 * The "508" in the name is the size class: up to 32 cores in 8
 * quad-clusters. LX2160A populates 8 dual-core A72 clusters (16
 * cores) into a CCN-508 sized for 32, so the upper half of the
 * cluster slots is unused.
 *
 * CCN models the chip as a graph of nodes. The node types this
 * file's policies care about:
 *
 *   RN-F  Request Node, Fully coherent
 *       a coherent CPU port (the 8 A72 clusters).
 *
 *   RN-I  Request Node, I/O coherent
 *       a DMA-capable I/O master (WRIOP, qDMA, USB, PCIe, SEC).
 *
 *   HN-F  Home Node, Fully coherent
 *       a memory-controller home (the two DDR controllers).
 *
 *   HN-I  Home Node, I/O
 *       a peripheral / config space home (DCFG and friends).
 *
 *   SN-F  Slave Node, Fully coherent
 *       a downstream coherent target (DRAM behind an HN-F).
 *
 *   SN-I  Slave Node, I/O
 *       a downstream I/O target (peripheral block behind an HN-I).
 *
 * Each node has a stable numeric node ID in the floorplan; that's
 * how POLICY_PERF_WRIOP below selects which RN-I port to tune.
 *
 * Authoritative reference: ARM IHI 0469 "CoreLink CCN-508 Cache
 * Coherent Network Technical Reference Manual" (publicly available
 * from ARM Developer).
 */

#ifndef	POLICY_H
#define	POLICY_H

/*
 * ECC memory policy.
 *
 * 0 = platform does NOT handle ECC errors in TF-A (ECC init skipped).
 * 1 = TF-A initializes ECC scrubbing on OCRAM and secure DRAM.
 *
 * The Nodebox DDR SODIMMs support ECC (DDR_ECC_EN=yes in platform.mk),
 * but the ECC init here refers to the OCRAM and secure SRAM paths
 * that TF-A itself uses. Set to 0 because the ROM code and DDR
 * driver handle their own ECC initialization. Change to 1 only if
 * you observe ECC errors in OCRAM during early boot.
 */
#define	POLICY_USING_ECC	0x0

/*
 * SMMU page size policy.
 */
#define	POLICY_SMMU_PAGESZ_64K	0x1

/*
 * Per-RN-I ENABLE_WUO selector on the CCN-508.
 *
 * What the NXP code does: soc_interconnect_config() in
 * plat/nxp/soc-lx2160a/soc.c always sets ENABLE_WUO ("Enable Write
 * Unique Optimization") in some RN-I's SA_AUX_CTRL_REG. ENABLE_WUO
 * lets the CCN collapse a write-unique into a write-clean +
 * invalidate when the line is not shared, which improves DDR write
 * throughput on streaming DMA producers. The macro selects the
 * target node ID:
 *
 * 0 = write WUO to RN-I 17 (the #else branch).
 * 1 = write WUO to RN-I 7  (the #if  branch).
 * 2 = write WUO to RN-I 23 (the #if  branch).
 *
 * Any other value triggers a runtime panic from BL2's
 * soc_interconnect_config(), so a typo at the make line surfaces at
 * first power-on rather than at link time. There is no value that
 * disables WUO entirely; even the default (0) still issues a write.
 *
 * What is unclear from the NXP code: the macro and its three
 * values were introduced by upstream NXP commit 87056d319 in
 * December 2020 ("nxp: adding support of soc lx2160a"). Neither that
 * commit message nor the comment block in
 * plat/nxp/soc-lx2160a/lx2160ardb/policy.h documents what the three
 * RN-I node IDs are or why an integrator might pick one over
 * another. The upstream comment, quoted verbatim, reads:
 *
 *     POLICY_PERF_WRIOP = 0 : No Performance enhancement for WRIOP RN-I
 *     POLICY_PERF_WRIOP = 1 : No Performance enhancement for WRIOP RN-I = 7
 *     POLICY_PERF_WRIOP = 2 : No Performance enhancement for WRIOP RN-I = 23
 *
 * which is broken logic (the negation is duplicated) and gives no
 * rationale. Compounding the ambiguity, the RN-I 17 reference in the
 * default branch does not match the only other RN-I 17 mention in
 * the same source file: the comment at soc.c near the PCIe HN-I
 * setup states "PCIe is Connected to RN-I 17 which is connected to
 * HN-I 13", which makes RN-I 17 a PCIe master rather than a WRIOP
 * one. Either WRIOP and PCIe share RN-I 17 on this SoC, or the
 * macro is named for the wrong I/O master. The TF-A tree does not
 * resolve the question.
 *
 * A second oddity in the consumer code: the value-1 / value-2
 * branch reads NODE_TYPE_RNI but writes NODE_TYPE_HNI at the same
 * offset; the value-0 branch reads and writes NODE_TYPE_RNI
 * consistently. Whether that is intentional or a typo is also
 * undocumented in upstream.
 *
 * Recommended setting. Keep the value at 0 to match upstream NXP's
 * default for every LX2160A reference board (lx2160ardb,
 * lx2160aqds, lx2162aqds). Pick a non-zero value only after
 * verifying with NXP what RN-I 7, 17, and 23 each correspond to on
 * this exact SoC variant and floorplan, and confirming that the
 * NODE_TYPE_HNI write is intentional.
 *
 * TODO TBC
 */
#define	POLICY_PERF_WRIOP	0

/*
 * Debug clock + interface policy for LPM20 low-power transitions.
 *
 * Read at three sites in lx2160a.S (BL31 only), all in the LPM20
 * entry / pre-WFI / resume path. The macro gates three distinct
 * actions. cbz / cbnz means any non-zero value behaves like 1.
 *
 * 0 = production. All three quiescing actions fire:
 *     - the debug clock bit stays in the DEVDISR5 mask, so it gets
 *       gated when the SoC enters LPM20 (max power saving);
 *     - OSDLR_EL1.DLK is asserted before WFI, forcing the debug
 *       interface quiescent in cache-only mode;
 *     - OSDLR_EL1.DLK is cleared on resume so the interface comes
 *       back in a known-good state.
 *
 * 1 = debug-only. All three actions skipped:
 *     - debug clock keeps running through LPM20 (extra power draw,
 *       SoC never reaches its deepest sleep state);
 *     - OSDLR_EL1.DLK is left as-is on entry and on resume, so a
 *       debugger that has set DLK pre-LPM20 sees that state persist
 *       across the transition. Required only when actively debugging
 *       TF-A's LPM20 path with a JTAG probe + CoreSight trace, where
 *       the debug clock must keep ticking to capture trace data.
 */
#define	POLICY_DEBUG_ENABLE	0


#endif /* POLICY_H */

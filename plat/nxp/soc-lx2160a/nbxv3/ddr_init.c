/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module - DDR initialization.
 *
 * The Nodebox uses SODIMMs, so DDR timings are read from the SPD
 * EEPROMs over I2C at boot time. No hardcoded timing tables needed.
 * Do NOT define CONFIG_STATIC_DDR or CONFIG_DDR_NODIMM like some
 * other NXP LX2160 boards.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <dimm.h>
#include <drivers/delay_timer.h>
#include <lib/utils.h>

#include <ddr.h>
#include <load_img.h>
#include "plat_common.h"
#include <platform_def.h>

/*
 * Board-specific DDR signal-integrity tuning.
 *
 * The DDR driver reads timing (tCK, tAA, tRCD, etc.) from the SODIMM
 * SPD EEPROM, but it cannot know the board's PCB trace impedance,
 * stub lengths, or slot geometry. This function provides the
 * board-level electrical parameters that depend on physical layout.
 *
 * Starting values are taken from the lx2160aqds single-slot path.
 * TODO: fine-tune vref_dimm, vref_phy, and odt by running DDR eye diagrams
 * on the Nodebox v3 hardware at temperature extremes.
 *
 * DDR controller registers: LX2160ARM Chapter 15 (DDR Memory Controller),
 * base addresses 0x0108_0000 (DDR1) and 0x0109_0000 (DDR2).
 *
 * DDR4 mode registers (MR0-MR6): defined by JEDEC JESD79-4A, referenced
 * in LX2160ARM Section 15.3. Programmed via DDR_SDRAM_MD_CNTL (offset
 * 0x120, Section 15.5.1.11).
 *
 * DDR PHY registers: Synopsys DDR PHY Gen2, PHY1 at 0x0140_0000 and
 * PHY2 at 0x0160_0000. The PHY Vref, ODT impedance and TX impedance
 * are Synopsys-internal parameters, not documented in the LX2160A RM.
 */
int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;

	/* DRAM-side (DIMM) settings */

	/*
	 * DRAM input Vref (voltage reference), written to DDR4 MR6
	 * (JESD79-4A Section 4.10.6).
	 * 7-bit field: 0x0d = range 0, step 13 = 60% + 13*0.65% = 68.45%
	 * of VDDQ. Sets the logic-0 / logic-1 threshold on the DRAM's
	 * DQ receivers. Board-specific: depends on trace SI between SoC
	 * and SODIMM slot.
	 * Programmed via DDR_SDRAM_MD_CNTL (RM 15.5.1.11, offset 0x120)
	 * with MD_SEL targeting MR6.
	 *
	 * This starting value is the trained mode captured by the
	 * Synopsys PHY Gen2 1D/2D training firmware across 3 cold
	 * boots on this CPU module + DRAM combination. Starting close
	 * to the convergence point shortens training time.
	 */
	popts->vref_dimm = U(0x0d);

	/*
	 * Don't override RTT_NOM / RTT_WR. Let the driver compute them
	 * from SPD and topology automatically (safe for single-DIMM).
	 * RTT values are written to DDR4 MR1 (RTT_NOM) and MR2 (RTT_WR)
	 * per JESD79-4A Sections 4.10.2 and 4.10.3.
	 */
	popts->rtt_override = 0;

	/*
	 * RTT_PARK = 240 ohms, written to DDR4 MR5 bits [8:6]
	 * (JESD79-4A Section 4.10.5, encoded as 0x4 = RZQ/1).
	 * On-die termination on the DRAM when the rank is idle ("parked").
	 * 240 ohms (weak) is correct for single-DIMM: no second slot
	 * creates reflections. Use 120 or 80 for dual-DIMM topologies.
	 */
	popts->rtt_park = U(240);

	/*
	 * Burst and general controller settings
	 * (RM Section 15.5.1.8, DDR_SDRAM_CFG register, offset 0x110)
	 */

	/*
	 * Disable on-the-fly burst chop (BC4). Every transaction uses
	 * full BL8 (8 beats x 8 bytes = 64 bytes = one A72 cache line).
	 * Controls DDR_SDRAM_CFG[BE_8] (bit 18) and OBC_CFG in
	 * DDR_SDRAM_CFG_2. Simpler and slightly more efficient for
	 * streaming workloads (100GE packet processing).
	 */
	popts->otf_burst_chop_en = 0;
	popts->burst_length = U(DDR_BL8);

	/*
	 * Enable manual override of bus turnaround timings (trwt, twrt,
	 * trrt, twwt below). These depend on board-level SI, not the
	 * DIMM spec, so the driver can't compute them from SPD.
	 */
	popts->trwt_override = U(1);

	/*
	 * Auto-precharge mode via DDR_SDRAM_INTERVAL[BSTOPRE] (RM
	 * Section 15.5.1.13, offset 0x124, bits [13:0]).
	 * 0 = global auto-precharge: the controller issues auto-precharge
	 * with every read/write instead of retaining open pages.
	 * See also DDR_SDRAM_CFG[AP_EN] (bit 23) which enables
	 * per-chip-select auto-precharge.
	 */
	popts->bstopre = U(0);

	/*
	 * Address hashing: XOR-hash address bits before bank/row decode
	 * to distribute accesses evenly across banks (RM Section 15.7,
	 * DDR interleaving configurations). Reduces bank conflicts when
	 * multiple DMA engines (DPAA2, WRIOP, CCN-508) hit DDR
	 * simultaneously.
	 */
	popts->addr_hash = 1;

	/*
	 * Bus turnaround timing (board-specific)
	 *
	 * TIMING_CFG_0 register (RM Section 15.5.1.5, offset 0x104):
	 *   RWT  bits [31:30] (+ EXT_RWT in TIMING_CFG_4 for 4-bit total)
	 *   WRT  bits [29:28] (+ EXT_WRT in TIMING_CFG_4 for 3-bit total)
	 *   RRT  bits [27:26] (+ EXT_RRT in TIMING_CFG_4 for 3-bit total)
	 *   WWT  bits [25:24] (+ EXT_WWT in TIMING_CFG_4 for 3-bit total)
	 *
	 * These are extra clock cycles added to the minimum turnaround
	 * time computed from CAS latency and burst length. They
	 * compensate for bus settling time when switching direction or
	 * switching between ranks.
	 *
	 * Nodebox v3 has 1 SODIMM per controller (DDRC_NUM_DIMM=1),
	 * so cs_on_dimm[1] is always 0 -> single-slot path. 0x3 is
	 * conservative for short stubs with no second-slot reflections.
	 * Dual-DIMM boards (lx2160aqds two-slot) use 0x7 or 0xF.
	 */

	/* Read-to-write: gap after last read beat before driving write */
	popts->trwt = U(0x3);
	/* Write-to-read: gap after last write beat before sampling read */
	popts->twrt = U(0x3);
	/* Read-to-read (different ranks on a dual-rank SODIMM) */
	popts->trrt = U(0x3);
	/* Write-to-write (different ranks) */
	popts->twwt = U(0x3);

	/*
	 * PHY-side (SoC) signal integrity
	 *
	 * These values configure the Synopsys DDR PHY Gen2 inside the
	 * LX2160A (PHY1 at 0x0140_0000, PHY2 at 0x0160_0000). They are
	 * NOT documented in the LX2160A RM (Chapter 15 covers the DDR
	 * controller only). TODO: refer to the Synopsys DDR PHY Gen2 user
	 * guide for register-level detail. The TF-A driver passes these
	 * via input.vref, input.adv.odtimpedance, input.adv.tx_impedance
	 * to the PHY firmware.
	 */

	/*
	 * DDR PHY Vref (SoC-side receiver reference). Threshold voltage
	 * inside the LX2160A's DDR PHY when reading data from the DRAM.
	 * 7-bit DAC code; 0x43 (67 decimal) was the trained mode across
	 * 3 cold boots on the nbxv3 reference system. Starting close to
	 * the convergence point shortens PHY training.
	 */
	popts->vref_phy = U(0x43);

	/*
	 * PHY ODT (on-die termination) impedance = 48 ohms. Termination
	 * resistance inside the LX2160A's DDR PHY during reads. Should
	 * match the board's characteristic impedance. 48 ohms is typical
	 * for single-SODIMM topologies. (Driver default if 0: 60 ohms.)
	 */
	popts->odt = U(48);

	/*
	 * PHY TX drive impedance = 28 ohms. Output driver strength when
	 * transmitting data (writes) and address/command. Lower value =
	 * stronger drive, needed to push clean signals through PCB
	 * traces + SODIMM connector. 28 ohms is the standard across all
	 * NXP LX2160A reference boards. (Driver default if 0: 28 ohms.)
	 */
	popts->phy_tx_impedance = U(28);

	return 0;
}

/*
 * Per-socket SPD probe + summary print.
 *
 * Runs before dram_init() for an explicit * "TOP"/"BOTTOM" mapping for
 * every cold boot.
 *
 *   Controller 0 / addr 0x50  <=>  TOP    (J900, DDR1 nets)
 *   Controller 1 / addr 0x51  <=>  BOTTOM (J600, DDR2 nets)
 *
 * Minimal SPD decode (capacity + ranks + mpart)
 */
struct nbxv3_sodimm_slot {
	const char	*label;
	const char	*refdes;
	unsigned char	 i2c_addr;
};

static const struct nbxv3_sodimm_slot nbxv3_sodimm_slots[] = {
	{ .label = "TOP   ", .refdes = "J900", .i2c_addr = 0x50 },
	{ .label = "BOTTOM", .refdes = "J600", .i2c_addr = 0x51 },
};

/*
 * DDR4 SPD EEPROM total size, in bytes.
 *
 * Per JEDEC Standard No. 21-C, Annex K / SPD for DDR4 SDRAM, the SPD
 * EEPROM is organised as TWO 256-byte pages. The page is selected via
 * the write-only I2C "Set Page Address" commands SPA0=0x36 (page 0)
 * and SPA1=0x37 (page 1); see drivers/nxp/ddr/nxp-ddr/dimm.h
 * (SPD_SPA0_ADDRESS/SPD_SPA1_ADDRESS) and read_spd() which does
 * exactly that: 256 bytes from page 0 followed by 256 bytes from
 * page 1, for 512 bytes total.
 *
 * struct ddr4_spd (same header) describes the layout of these 512
 * bytes; byte 329..348 is the Mfg Module Part Number that we dump,
 * well inside page 1.
 */
#define NBXV3_DDR4_SPD_PAGE_BYTES	256U
#define NBXV3_DDR4_SPD_NUM_PAGES	2U
#define NBXV3_DDR4_SPD_BYTES	\
	(NBXV3_DDR4_SPD_PAGE_BYTES * NBXV3_DDR4_SPD_NUM_PAGES)

/*
 * DDR4 SPD timebases (JEDEC JESD400-5 / SPD annex):
 *   MTB (Medium Time Base) = 125 ps   byte-18 (tCK_min), byte-24 (tAA_min) units
 *   FTB (Fine Time Base)   =   1 ps   byte-125 / 123 units (signed)
 * Byte 17 "timebases" is always 0x00 on DDR4, so these are fixed.
 */
#define NBXV3_SPD_DDR4_MTB_PS	125
#define NBXV3_SPD_DDR4_FTB_PS	1

/* SPD byte-3 module_type enum, per JEDEC JESD21-C Annex K. */
static const char *nbxv3_spd_decode_module_type(uint8_t v)
{
	switch (v) {
	case 0x01: return "RDIMM";
	case 0x02: return "UDIMM";
	case 0x03: return "SO-DIMM (generic, catch-all)";
	case 0x04: return "LRDIMM";
	case 0x05: return "Mini-RDIMM";
	case 0x06: return "Mini-UDIMM";
	case 0x08: return "72b-SO-RDIMM (ECC SO-RDIMM)";
	case 0x09: return "72b-SO-UDIMM (ECC SO-UDIMM)";
	case 0x0a: return "72b-SO-DIMM (ECC SO-DIMM)";
	case 0x0b: return "16b-SO-DIMM";
	case 0x0c: return "32b-SO-DIMM";
	default:   return "(unknown enum)";
	}
}

static const char *nbxv3_spd_byte_name(unsigned int idx)
{
	switch (idx) {
	case 0:  return "SPD bytes used + total";
	case 1:  return "SPD encoding revision";
	case 2:  return "DRAM device type (key)";
	case 3:  return "Module type";
	case 4:  return "SDRAM density / banks";
	case 5:  return "SDRAM addressing (rows/cols)";
	case 6:  return "Package type";
	case 7:  return "Optional features";
	case 8:  return "Thermal / refresh options";
	case 11: return "Module nominal voltage";
	case 12: return "Module organization";
	case 13: return "Bus width / ECC";
	case 14: return "Module thermal sensor";
	case 17: return "Time bases (MTB / FTB)";
	case 18: return "tCK_min";
	case 24: return "tAA_min";
	default: return NULL;
	}
}

static const char *nbxv3_spd_byte_meaning(unsigned int idx, uint8_t v)
{
	if (idx == 3)
		return nbxv3_spd_decode_module_type(v);
	return NULL;
}

static void nbxv3_print_diff(unsigned int byte_idx,
			     uint8_t top_v, uint8_t bot_v)
{
	const char *name = nbxv3_spd_byte_name(byte_idx);
	const char *top_m = nbxv3_spd_byte_meaning(byte_idx, top_v);
	const char *bot_m = nbxv3_spd_byte_meaning(byte_idx, bot_v);

	NOTICE("    Byte %u: %s\n", byte_idx,
	       name ? name : "(no decoder)");
	NOTICE("      TOP    (J900) 0x%02x%s%s\n", top_v,
	       top_m ? "  " : "", top_m ? top_m : "");
	NOTICE("      BOTTOM (J600) 0x%02x%s%s\n", bot_v,
	       bot_m ? "  " : "", bot_m ? bot_m : "");
}

/* On SPD-checksum mismatch, dump each differing CRC-covered byte. */
static void nbxv3_diff_spds_on_mismatch(
	const uint8_t spds[][NBXV3_DDR4_SPD_BYTES],
	const bool valid[],
	unsigned int n_slots)
{
	if (n_slots < 2 || !valid[0] || !valid[1])
		return;

	uint32_t cs[2];
	unsigned int s;

	for (s = 0; s < 2U; s++) {
		cs[s] = ((uint32_t)spds[s][127] << 24)
		      | ((uint32_t)spds[s][126] << 16)
		      | ((uint32_t)spds[s][255] <<  8)
		      | ((uint32_t)spds[s][254]);
	}
	if (cs[0] == cs[1])
		return;

	NOTICE("nbxv3 ddr: SPD checksum MISMATCH -- driver may reject the pair\n");
	NOTICE("    TOP    cs=0x%08x  base@126,127=%02x %02x  mod@254,255=%02x %02x\n",
	       cs[0], spds[0][126], spds[0][127],
	       spds[0][254], spds[0][255]);
	NOTICE("    BOTTOM cs=0x%08x  base@126,127=%02x %02x  mod@254,255=%02x %02x\n",
	       cs[1], spds[1][126], spds[1][127],
	       spds[1][254], spds[1][255]);

	NOTICE("    diffs in CRC-covered ranges:\n");
	static const struct {
		unsigned int lo, hi;
	} ranges[] = {
		{ .lo = 0U,   .hi = 125U }, /* base data section (excl. CRC @126-127)  */
		{ .lo = 128U, .hi = 253U }, /* mod-specific section (excl. CRC @254-255) */
	};
	unsigned int diffs = 0U;
	unsigned int r, b;

	for (r = 0U; r < ARRAY_SIZE(ranges); r++) {
		for (b = ranges[r].lo; b <= ranges[r].hi; b++) {
			if (spds[0][b] != spds[1][b]) {
				nbxv3_print_diff(b, spds[0][b], spds[1][b]);
				diffs++;
				if (diffs >= 16U) {
					NOTICE("    ... (truncated at 16 -- DIMMs likely from different vendors/lots)\n");
					return;
				}
			}
		}
	}

	if (diffs == 0U)
		NOTICE("    no covered-range diffs -- CRC bytes themselves disagree (rare; SPD corruption?)\n");
}

static void nbxv3_probe_sodimm_sockets(void)
{
	static uint8_t spds[ARRAY_SIZE(nbxv3_sodimm_slots)][NBXV3_DDR4_SPD_BYTES];
	bool valid[ARRAY_SIZE(nbxv3_sodimm_slots)] = { false };
	unsigned int i;

	NOTICE("nbxv3 ddr: SODIMM socket map\n");
	for (i = 0; i < ARRAY_SIZE(nbxv3_sodimm_slots); i++) {
		const struct nbxv3_sodimm_slot *s = &nbxv3_sodimm_slots[i];
		uint8_t *spd = spds[i];
		struct ddr4_spd *p = (struct ddr4_spd *)spd;
		unsigned long long rank_bytes;
		unsigned int n_ranks, nbit_cap, nbit_bus, nbit_dev;
		unsigned int mtps, cl;
		int tck_ps, taa_ps;
		bool ecc;
		char mpart[21];
		int ret;

		zeromem(spd, NBXV3_DDR4_SPD_BYTES);
		ret = read_spd(s->i2c_addr, spd, NBXV3_DDR4_SPD_BYTES);
		if (ret != 0) {
			WARN("  %s (%s @ 0x%02x): SPD not responding (i2c ret=%d)\n",
			     s->label, s->refdes, s->i2c_addr, ret);
			continue;
		}
		valid[i] = true;

		/* mpart @ bytes 329-348, guaranteed-terminated copy. */
		memcpy(mpart, p->mpart, sizeof(p->mpart));
		mpart[sizeof(p->mpart)] = '\0';

		/* Minimal DDR4 capacity decode (matches compute_ranksize()). */
		nbit_cap = (p->density_banks & 0xf) <= 7 ?
			   (p->density_banks & 0xf) + 28 : 0;
		nbit_bus = (p->bus_width    & 0x7) <  4 ?
			   (p->bus_width    & 0x7) +  3 : 0;
		nbit_dev = (p->organization & 0x7) <  4 ?
			   (p->organization & 0x7) +  2 : 0;
		n_ranks  = ((p->organization >> 3) & 0x7) + 1;
		rank_bytes = (nbit_cap && nbit_bus && nbit_dev) ?
			1ULL << (nbit_cap - 3 + nbit_bus - nbit_dev) : 0;

		/*
		 * Speed bin: tCK_min in picoseconds = MTB*byte18 + FTB*byte125.
		 * DDR4 data rate (MT/s) = 2 * (1/tCK_s) / 1e6 = 2e6 / tCK_ps.
		 * Example: tck_ps=833 -> 2400 MT/s -> DDR4-2400.
		 */
		tck_ps = (int)p->tck_min * NBXV3_SPD_DDR4_MTB_PS +
			 (int)(int8_t)p->fine_tck_min * NBXV3_SPD_DDR4_FTB_PS;
		mtps = (tck_ps > 0) ? (2000000U / (unsigned int)tck_ps) : 0U;

		/*
		 * Minimum CAS latency the DIMM declares at its tCK_min bin:
		 *   CL = ceil(tAA_min / tCK_min)
		 */
		taa_ps = (int)p->taa_min * NBXV3_SPD_DDR4_MTB_PS +
			 (int)(int8_t)p->fine_taa_min * NBXV3_SPD_DDR4_FTB_PS;
		cl = (tck_ps > 0 && taa_ps > 0) ?
		     (unsigned int)((taa_ps + tck_ps - 1) / tck_ps) : 0U;

		/*
		 * bus_width byte 13 bits [4:3] = "Bus Width Extension":
		 *   0b00 = 0 extra bits (non-ECC)
		 *   0b01 = 8 extra bits (ECC)
		 */
		ecc = (((p->bus_width >> 3) & 0x3) == 1U);

		NOTICE("  %s (%s @ 0x%02x): %lu MiB, %u ranks, DDR4-%u, CL%u, ECC %s, mpart='%s'\n",
		       s->label, s->refdes, s->i2c_addr,
		       (unsigned long)((rank_bytes * n_ranks) >> 20),
		       n_ranks, mtps, cl, ecc ? "on" : "off", mpart);
	}

	nbxv3_diff_spds_on_mismatch(spds, valid, ARRAY_SIZE(nbxv3_sodimm_slots));
}

#ifdef NXP_WARM_BOOT
long long init_ddr(uint32_t wrm_bt_flg)
#else
long long init_ddr(void)
#endif
{
	/*
	 * SPD EEPROM I2C addresses per DDR controller, mapped to the
	 * physical SODIMM sockets:
	 *
	 *   spd_addr[0] = 0x50 -> DDR ctrlr 0 (base 0x0108_0000, net DDR1)
	 *                      -> J900 (TOP SODIMM), SA0=GND, SA1=GND
	 *   spd_addr[1] = 0x51 -> DDR ctrlr 1 (base 0x0109_0000, net DDR2)
	 *                      -> J600 (BOTTOM SODIMM), SA0=+2V5, SA1=GND
	 *
	 * Both sockets share the SoC's IIC1 bus (NXP_I2C_ADDR, already
	 * initialised in soc_early_init()). The two DIMMs are distinguished
	 * only by their 7-bit SPD slave address. SA0 is strapped high at
	 * J600 pin 256 via +2V5 on the PCB, low at J900 pin 256 via GND.
	 * See nbxv3_probe_sodimm_sockets() below for the per-socket probe
	 * that prints TOP / BOTTOM labels before the NXP DDR core's own
	 * "Controller N / addr 0x5N" prints.
	 *
	 * Do NOT pad this array with 0x0 placeholders. The NXP DDR core
	 * (drivers/nxp/ddr/nxp-ddr/ddr.c) walks spd_addr sequentially
	 * and would read 0x0 for controller 1 slot 0, then bail out
	 * with "First SPD addr wrong". The padded 4-entry form is only
	 * correct when DDRC_NUM_DIMM=2 (two SODIMMs per controller).
	 * Nodebox v3 has DDRC_NUM_DIMM=1: one entry per controller,
	 * total 2 entries.
	 */
	int spd_addr[] = { 0x50, 0x51 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys) == 1) {
		ERROR("System clocks are not set.\n");
		panic();
	}
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	nbxv3_probe_sodimm_sockets();

	zeromem(&info, sizeof(info));

	/* Set two DDRC. Unused DDRC will be removed automatically. */
	info.num_ctlrs = NUM_OF_DDRC;
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	info.phy[1] = (void *)NXP_DDR_PHY2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	info.img_loadr = load_img;
	info.phy_gen2_fw_img_buf = PHY_GEN2_FW_IMAGE_BUFFER;
	if (info.clk == 0) {
		info.clk = get_ddr_freq(&sys, 1);
	}
	info.dimm_on_ctlr = DDRC_NUM_DIMM;

	info.warm_boot_flag = DDR_WRM_BOOT_NT_SUPPORTED;
#ifdef NXP_WARM_BOOT
	info.warm_boot_flag = DDR_COLD_BOOT;
	if (wrm_bt_flg != 0U) {
		info.warm_boot_flag = DDR_WARM_BOOT;
	} else {
		info.warm_boot_flag = DDR_COLD_BOOT;
	}
#endif

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
		    );

	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	return dram_size;
}

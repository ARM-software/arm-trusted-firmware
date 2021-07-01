/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MNPMUSRAMMSGBLOCK_DDR3_H
#define MNPMUSRAMMSGBLOCK_DDR3_H

/*
 * DDR3U_1D training firmware message block structure
 *
 * Please refer to the Training Firmware App Note for futher information about
 * the usage for Message Block.
 */
struct pmu_smb_ddr_1d {
	uint8_t reserved00;		/*
					 * Byte offset 0x00, CSR Addr 0x54000, Direction=In
					 * reserved00[0:4] RFU, must be zero
					 *
					 * reserved00[5] = Train vrefDAC0 During Read Deskew
					 *   0x1 = Read Deskew will begin by enabling and roughly
					 *   training the phy's per-lane reference voltages.
					 *   Training the vrefDACs CSRs will increase the maximum 1D
					 *   training time by around half a millisecond, but will
					 *   improve 1D training accuracy on systems with
					 *   significant voltage-offsets between lane read eyes.
					 *   0x0 = Read Deskew will assume the messageblock's
					 *   phyVref setting will work for all lanes.
					 *
					 * reserved00[6] = Enable High Effort WrDQ1D
					 *   0x1 = WrDQ1D will conditionally retry training at
					 *   several extra RxClkDly Timings. This will increase the
					 *   maximum 1D training time by up to 4 extra iterations of
					 *   WrDQ1D. This is only required in systems that suffer
					 *   from very large, asymmetric eye-collapse when receiving
					 *   PRBS patterns.
					 *   0x0 = WrDQ1D assume rxClkDly values found by SI
					 *   Friendly RdDqs1D will work for receiving PRBS patterns
					 *
					 * reserved00[7] = Optimize for the special hard macros in
					 * TSMC28.
					 *   0x1 = set if the phy being trained was manufactured in
					 *   any TSMC28 process node.
					 *   0x0 = otherwise, when not training a TSMC28 phy, leave
					 *   this field as 0.
					 */
	uint8_t msgmisc;		/*
					 * Byte offset 0x01, CSR Addr 0x54000, Direction=In
					 * Contains various global options for training.
					 *
					 * Bit fields:
					 *
					 * msgmisc[0] = MTESTEnable
					 *   0x1 = Pulse primary digital test output bump at the end
					 *   of each major training stage. This enables observation
					 *   of training stage completion by observing the digital
					 *   test output.
					 *   0x0 = Do not pulse primary digital test output bump
					 *
					 * msgmisc[1] = SimulationOnlyReset
					 *   0x1 = Verilog only simulation option to shorten
					 *   duration of DRAM reset pulse length to 1ns.
					 *   Must never be set to 1 in silicon.
					 *   0x0 = Use reset pulse length specified by JEDEC
					 *   standard.
					 *
					 * msgmisc[2] = SimulationOnlyTraining
					 *   0x1 = Verilog only simulation option to shorten the
					 *   duration of the training steps by performing fewer
					 *   iterations.
					 *   Must never be set to 1 in silicon.
					 *   0x0 = Use standard training duration.
					 *
					 * msgmisc[3] = RFU, must be zero
					 *
					 * msgmisc[4] = Suppress streaming messages, including
					 * assertions, regardless of hdtctrl setting.
					 * Stage Completion messages, as well as training completion
					 * and error messages are still sent depending on hdtctrl
					 * setting.
					 *
					 * msgmisc[5] = PerByteMaxRdLat
					 *   0x1 = Each DBYTE will return dfi_rddata_valid at the
					 *   lowest possible latency. This may result in unaligned
					 *   data between bytes to be returned to the DFI.
					 *   0x0 = Every DBYTE will return dfi_rddata_valid
					 *   simultaneously. This will ensure that data bytes will
					 *   return aligned accesses to the DFI.
					 *
					 * msgmisc[6] = PartialRank (DDR3 UDIMM and DDR4 UDIMM only,
					 * otherwise RFU, must be zero)
					 *   0x1 = Support rank populated with a subset of byte, but
					 *   where even-odd pair of rank support all the byte
					 *   0x0 = All rank populated with all the byte (tyical
					 *   configuration)
					 *
					 * msgmisc[7] RFU, must be zero
					 *
					 * Notes:
					 *
					 * - SimulationOnlyReset and SimulationOnlyTraining can be
					 *   used to speed up simulation run times, and must never
					 *   be used in real silicon. Some VIPs may have checks on
					 *   DRAM reset parameters that may need to be disabled when
					 *   using SimulationOnlyReset.
					 */
	uint16_t pmurevision;		/*
					 * Byte offset 0x02, CSR Addr 0x54001, Direction=Out
					 * PMU firmware revision ID
					 * After training is run, this address will contain the
					 * revision ID of the firmware.
					 * Please reference this revision ID when filing support
					 * cases.
					 */
	uint8_t pstate;			/*
					 * Byte offset 0x04, CSR Addr 0x54002, Direction=In
					 * Must be set to the target pstate to be trained
					 *   0x0 = pstate 0
					 *   0x1 = pstate 1
					 *   0x2 = pstate 2
					 *   0x3 = pstate 3
					 *   All other encodings are reserved
					 */
	uint8_t pllbypassen;		/*
					 * Byte offset 0x05, CSR Addr 0x54002, Direction=In
					 * Set according to whether target pstate uses PHY PLL
					 * bypass
					 *   0x0 = PHY PLL is enabled for target pstate
					 *   0x1 = PHY PLL is bypassed for target pstate
					 */
	uint16_t dramfreq;		/*
					 * Byte offset 0x06, CSR Addr 0x54003, Direction=In
					 * DDR data rate for the target pstate in units of MT/s.
					 * For example enter 0x0640 for DDR1600.
					 */
	uint8_t dfifreqratio;		/*
					 * Byte offset 0x08, CSR Addr 0x54004, Direction=In
					 * Frequency ratio betwen DfiCtlClk and SDRAM memclk.
					 *   0x1 = 1:1
					 *   0x2 = 1:2
					 *   0x4 = 1:4
					 */
	uint8_t bpznresval;		/*
					 * Byte offset 0x09, CSR Addr 0x54004, Direction=In
					 * Overwrite the value of precision resistor connected to
					 * Phy BP_ZN
					 *   0x00 = Do not program. Use current CSR value.
					 *   0xf0 = 240 Ohm
					 *   0x78 = 120 Ohm
					 *   0x28 = 40 Ohm
					 *   All other values are reserved.
					 * It is recommended to set this to 0x00.
					 */
	uint8_t phyodtimpedance;	/*
					 * Byte offset 0x0a, CSR Addr 0x54005, Direction=In
					 * Must be programmed to the termination impedance in ohms
					 * used by PHY during reads.
					 *
					 *   0x0 = Firmware skips programming (must be manually
					 *   programmed by user prior to training start)
					 *
					 * See PHY databook for legal termination impedance values.
					 *
					 * For digital simulation, any legal value can be used. For
					 * silicon, the users must determine the correct value
					 * through SI simulation or other methods.
					 */
	uint8_t phydrvimpedance;	/*
					 * Byte offset 0x0b, CSR Addr 0x54005, Direction=In
					 * Must be programmed to the driver impedance in ohms used
					 * by PHY during writes for all DBYTE drivers
					 * (DQ/DM/DBI/DQS).
					 *
					 *   0x0 = Firmware skips programming (must be manually
					 *   programmed by user prior to training start)
					 *
					 * See PHY databook for legal R_on driver impedance values.
					 *
					 * For digital simulation, any value can be used that is not
					 * Hi-Z. For silicon, the users must determine the correct
					 * value through SI simulation or other methods.
					 */
	uint8_t phyvref;		/*
					 * Byte offset 0x0c, CSR Addr 0x54006, Direction=In
					 * Must be programmed with the Vref level to be used by the
					 * PHY during reads
					 *
					 * The units of this field are a percentage of VDDQ
					 * according to the following equation:
					 *
					 * Receiver Vref = VDDQ*phyvref[6:0]/128
					 *
					 * For example to set Vref at 0.75*VDDQ, set this field to
					 * 0x60.
					 *
					 * For digital simulation, any legal value can be used. For
					 * silicon, the users must calculate the analytical Vref by
					 * using the impedances, terminations, and series resistance
					 * present in the system.
					 */
	uint8_t dramtype;		/*
					 * Byte offset 0x0d, CSR Addr 0x54006, Direction=In
					 * Module Type:
					 *   0x01 = DDR3 unbuffered
					 *   0x02 = Reserved
					 *   0x03 = Reserved
					 *   0x04 = Reserved
					 *   0x05 = Reserved
					 */
	uint8_t disableddbyte;		/*
					 * Byte offset 0x0e, CSR Addr 0x54007, Direction=In
					 * Bitmap to indicate which Dbyte are not connected (for
					 * DByte 0 to 7):
					 * Set disableddbyte[i] to 1 only to specify that DByte is
					 * not need to be trained (DByte 8 can be disabled via
					 * enableddqs setting)
					 */
	uint8_t enableddqs;		/*
					 * Byte offset 0x0f, CSR Addr 0x54007, Direction=In
					 * Total number of DQ bits enabled in PHY
					 */
	uint8_t cspresent;		/*
					 * Byte offset 0x10, CSR Addr 0x54008, Direction=In
					 * Indicates presence of DRAM at each chip select for PHY.
					 * Each bit corresponds to a logical CS.
					 *
					 * If the bit is set to 1, the CS is connected to DRAM.
					 * If the bit is set to 0, the CS is not connected to DRAM.
					 *
					 * cspresent[0] = CS0 is populated with DRAM
					 * cspresent[1] = CS1 is populated with DRAM
					 * cspresent[2] = CS2 is populated with DRAM
					 * cspresent[3] = CS3 is populated with DRAM
					 * cspresent[7:4] = Reserved (must be programmed to 0)
					 */
	uint8_t cspresentd0;		/*
					 * Byte offset 0x11, CSR Addr 0x54008, Direction=In
					 * The CS signals from field cspresent that are routed to
					 * DIMM connector 0
					 */
	uint8_t cspresentd1;		/*
					 * Byte offset 0x12, CSR Addr 0x54009, Direction=In
					 * The CS signals from field cspresent that are routed to
					 * DIMM connector 1
					 */
	uint8_t addrmirror;		/*
					 * Byte offset 0x13, CSR Addr 0x54009, Direction=In
					 * Corresponds to CS[3:0]
					 *   1 = Address Mirror.
					 *   0 = No Address Mirror.
					 */
	uint8_t cstestfail;		/*
					 * Byte offset 0x14, CSR Addr 0x5400a, Direction=Out
					 * This field will be set if training fails on any rank.
					 *   0x0 = No failures
					 *   non-zero = one or more ranks failed training
					 */
	uint8_t phycfg;			/*
					 * Byte offset 0x15, CSR Addr 0x5400a, Direction=In
					 * Additional mode bits.
					 *
					 * Bit fields:
					 * [0] SlowAccessMode:
					 *   1 = 2T Address Timing.
					 *   0 = 1T Address Timing.
					 * [7-1] RFU, must be zero
					 *
					 * WARNING: In case of DDR4 Geardown Mode (mr3[A3] == 1),
					 * phycfg[0] must be 0.
					 */
	uint16_t sequencectrl;		/*
					 * Byte offset 0x16, CSR Addr 0x5400b, Direction=In
					 * Controls the training steps to be run. Each bit
					 * corresponds to a training step.
					 *
					 * If the bit is set to 1, the training step will run.
					 * If the bit is set to 0, the training step will be
					 * skipped.
					 *
					 * Training step to bit mapping:
					 * sequencectrl[0] = Run DevInit - Device/phy
					 *		     initialization. Should always be set.
					 * sequencectrl[1] = Run WrLvl - Write leveling
					 * sequencectrl[2] = Run RxEn - Read gate training
					 * sequencectrl[3] = Run RdDQS1D - 1d read dqs training
					 * sequencectrl[4] = Run WrDQ1D - 1d write dq training
					 * sequencectrl[5] = RFU, must be zero
					 * sequencectrl[6] = RFU, must be zero
					 * sequencectrl[7] = RFU, must be zero
					 * sequencectrl[8] = Run RdDeskew - Per lane read dq deskew
					 *		     training
					 * sequencectrl[9] = Run MxRdLat - Max read latency training
					 * sequencectrl[10] = RFU, must be zero
					 * sequencectrl[11] = RFU, must be zero
					 * sequencectrl[12] = RFU, must be zero
					 * sequencectrl[13] = RFU, must be zero
					 * sequencectrl[15-14] = RFU, must be zero
					 */
	uint8_t hdtctrl;		/*
					 * Byte offset 0x18, CSR Addr 0x5400c, Direction=In
					 * To control the total number of debug messages, a
					 * verbosity subfield (hdtctrl, Hardware Debug Trace
					 * Control) exists in the message block. Every message has a
					 * verbosity level associated with it, and as the hdtctrl
					 * value is increased, less important s messages stop being
					 * sent through the mailboxes. The meanings of several major
					 * hdtctrl thresholds are explained below:
					 *
					 *   0x04 = Maximal debug messages (e.g., Eye contours)
					 *   0x05 = Detailed debug messages (e.g. Eye delays)
					 *   0x0A = Coarse debug messages (e.g. rank information)
					 *   0xC8 = Stage completion
					 *   0xC9 = Assertion messages
					 *   0xFF = Firmware completion messages only
					 */
	uint8_t reserved19;		/* Byte offset 0x19, CSR Addr 0x5400c, Direction=N/A */
	uint8_t reserved1a;		/* Byte offset 0x1a, CSR Addr 0x5400d, Direction=N/A */
	uint8_t share2dvrefresult;	/*
					 * Byte offset 0x1b, CSR Addr 0x5400d, Direction=In
					 * Bitmap that designates the phy's vref source for every
					 * pstate
					 * If share2dvrefresult[x] = 0, then after 2D training,
					 * pstate x will continue using the phyVref provided in
					 * pstate x's 1D messageblock.
					 * If share2dvrefresult[x] = 1, then after 2D training,
					 * pstate x will use the per-lane VrefDAC0/1 CSRs trained by
					 * 2d training.
					 */
	uint8_t reserved1c;		/* Byte offset 0x1c, CSR Addr 0x5400e, Direction=N/A */
	uint8_t reserved1d;		/* Byte offset 0x1d, CSR Addr 0x5400e, Direction=N/A */
	uint8_t reserved1e;		/*
					 * Byte offset 0x1e, CSR Addr 0x5400f, Direction=In
					 * Input for constraining the range of vref(DQ) values
					 * training will collect data for, usually reducing training
					 * time. However, too large of a voltage range may cause
					 * longer 2D training times while too small of a voltage
					 * range may truncate passing regions. When in doubt, leave
					 * this field set to 0.
					 * Used by 2D training in: Rd2D, Wr2D
					 *
					 * reserved1E[0-3]: Rd2D Voltage Range
					 *   0 = Training will search all phy vref(DQ) settings
					 *   1 = limit to +/-2 %VDDQ from phyVref
					 *   2 = limit to +/-4 %VDDQ from phyVref
					 *     . . .
					 *   15 = limit to +/-30% VDDQ from phyVref
					 *
					 * reserved1E[4-7]: Wr2D Voltage Range
					 *   0 = Training will search all dram vref(DQ) settings
					 *   1 = limit to +/-2 %VDDQ from mr6
					 *   2 = limit to +/-4 %VDDQ from mr6
					 *     . . .
					 *   15 = limit to +/-30% VDDQ from mr6
					 */
	uint8_t reserved1f;		/*
					 * Byte offset 0x1f, CSR Addr 0x5400f, Direction=In
					 * Extended training option:
					 *
					 * reserved1F[1:0]: Configured RxClkDly offset try during
					 * WrDq1D high-effort (i.e., when reserved00[6] is set)
					 *   0: -8, +8, -16, +16
					 *   1: -4, +4, -8, +8, -12, +12, -16, +16
					 *   2: -2, +2, -4, +4, -6, +6, -8, +8
					 *   3: -2, +2, -4, +4, -6, +6, -8, +8, -10, +10, -12, +12,
					 *      -14, +14, -16, +16
					 *
					 * reserved1F[2]: When set, execute again WrDq1D after
					 * RdDqs1D PRBS
					 *
					 * reserved1F[3]: When set redo RdDeskew with PRBS after
					 * (first) WrDqs1D
					 *
					 * reserved1F[7:4]: This field is reserved and must be
					 * programmed to 0x00.
					 */
	uint8_t reserved20;		/*
					 * Byte offset 0x20, CSR Addr 0x54010, Direction=In
					 * This field is reserved and must be programmed to 0x00,
					 * excepted for Reserved:
					 * Reserved MREP assume raising edge is found when
					 * reserved20[3:0]+3 consecutive 1 are received during MREP
					 * fine delay swept; reserved20[6:0] thus permits to
					 * increase tolerance for noisy system. And if reserved20[7]
					 * is set, MREP training is failing if no raising edge is
					 * found (otherwise the raising edge is assume close to
					 * delay 0).
					 */
	uint8_t reserved21;		/*
					 * Byte offset 0x21, CSR Addr 0x54010, Direction=In
					 * This field is reserved and must be programmed to 0x00,
					 * excepted for Reserved:
					 * Reserved DWL assume raising edge is found when
					 * reserved21[3:0]+3 consecutive 1 are received during DWL
					 * fine delay swept; reserved21[6:0] thus permits to
					 * increase tolerance for noisy system. And if reserved21[7]
					 * is set, DWL training is failing if no raising edge is
					 * found (otherwise the raising edge is assume close to
					 * delay 0).
					 */
	uint16_t phyconfigoverride;	/*
					 * Byte offset 0x22, CSR Addr 0x54011, Direction=In
					 * Override PhyConfig csr.
					 *   0x0: Use hardware csr value for PhyConfing
					 *   (recommended)
					 *   Other values: Use value for PhyConfig instead of
					 *   Hardware value.
					 */
	uint8_t dfimrlmargin;		/*
					 * Byte offset 0x24, CSR Addr 0x54012, Direction=In
					 * Margin added to smallest passing trained DFI Max Read
					 * Latency value, in units of DFI clocks. Recommended to be
					 * >= 1.
					 */
	int8_t cdd_rr_3_2;		/*
					 * Byte offset 0x25, CSR Addr 0x54012, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 3 to cs 2.
					 */
	int8_t cdd_rr_3_1;		/*
					 * Byte offset 0x26, CSR Addr 0x54013, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 3 to cs 1.
					 */
	int8_t cdd_rr_3_0;		/*
					 * Byte offset 0x27, CSR Addr 0x54013, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 3 to cs 0.
					 */
	int8_t cdd_rr_2_3;		/*
					 * Byte offset 0x28, CSR Addr 0x54014, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 2 to cs 3.
					 */
	int8_t cdd_rr_2_1;		/*
					 * Byte offset 0x29, CSR Addr 0x54014, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 2 to cs 1.
					 */
	int8_t cdd_rr_2_0;		/*
					 * Byte offset 0x2a, CSR Addr 0x54015, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 2 to cs 0.
					 */
	int8_t cdd_rr_1_3;		/*
					 * Byte offset 0x2b, CSR Addr 0x54015, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 1 to cs 3.
					 */
	int8_t cdd_rr_1_2;		/*
					 * Byte offset 0x2c, CSR Addr 0x54016, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 1 to cs 2.
					 */
	int8_t cdd_rr_1_0;		/*
					 * Byte offset 0x2d, CSR Addr 0x54016, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 1 to cs 0.
					 */
	int8_t cdd_rr_0_3;		/*
					 * Byte offset 0x2e, CSR Addr 0x54017, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 0 to cs 3.
					 */
	int8_t cdd_rr_0_2;		/*
					 * Byte offset 0x2f, CSR Addr 0x54017, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 0 to cs 2.
					 */
	int8_t cdd_rr_0_1;		/*
					 * Byte offset 0x30, CSR Addr 0x54018, Direction=Out
					 * This is a signed integer value.
					 * Read to read critical delay difference from cs 0 to cs 1.
					 */
	int8_t cdd_ww_3_2;		/*
					 * Byte offset 0x31, CSR Addr 0x54018, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 3 to cs
					 * 2.
					 */
	int8_t cdd_ww_3_1;		/*
					 * Byte offset 0x32, CSR Addr 0x54019, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 3 to cs
					 * 1.
					 */
	int8_t cdd_ww_3_0;		/*
					 * Byte offset 0x33, CSR Addr 0x54019, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 3 to cs
					 * 0.
					 */
	int8_t cdd_ww_2_3;		/*
					 * Byte offset 0x34, CSR Addr 0x5401a, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 2 to cs
					 * 3.
					 */
	int8_t cdd_ww_2_1;		/*
					 * Byte offset 0x35, CSR Addr 0x5401a, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 2 to cs
					 * 1.
					 */
	int8_t cdd_ww_2_0;		/*
					 * Byte offset 0x36, CSR Addr 0x5401b, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 2 to cs
					 * 0.
					 */
	int8_t cdd_ww_1_3;		/*
					 * Byte offset 0x37, CSR Addr 0x5401b, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 1 to cs
					 * 3.
					 */
	int8_t cdd_ww_1_2;		/*
					 * Byte offset 0x38, CSR Addr 0x5401c, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 1 to cs
					 * 2.
					 */
	int8_t cdd_ww_1_0;		/*
					 * Byte offset 0x39, CSR Addr 0x5401c, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 1 to cs
					 * 0.
					 */
	int8_t cdd_ww_0_3;		/*
					 * Byte offset 0x3a, CSR Addr 0x5401d, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 0 to cs
					 * 3.
					 */
	int8_t cdd_ww_0_2;		/*
					 * Byte offset 0x3b, CSR Addr 0x5401d, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 0 to cs
					 * 2.
					 */
	int8_t cdd_ww_0_1;		/*
					 * Byte offset 0x3c, CSR Addr 0x5401e, Direction=Out
					 * This is a signed integer value.
					 * Write to write critical delay difference from cs 0 to cs
					 * 1.
					 */
	int8_t cdd_rw_3_3;		/*
					 * Byte offset 0x3d, CSR Addr 0x5401e, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 3 to
					 * cs 3.
					 */
	int8_t cdd_rw_3_2;		/*
					 * Byte offset 0x3e, CSR Addr 0x5401f, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 3 to
					 * cs 2.
					 */
	int8_t cdd_rw_3_1;		/*
					 * Byte offset 0x3f, CSR Addr 0x5401f, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 3 to
					 * cs 1.
					 */
	int8_t cdd_rw_3_0;		/*
					 * Byte offset 0x40, CSR Addr 0x54020, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 3 to
					 * cs 0.
					 */
	int8_t cdd_rw_2_3;		/*
					 * Byte offset 0x41, CSR Addr 0x54020, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 2 to
					 * cs 3.
					 */
	int8_t cdd_rw_2_2;		/*
					 * Byte offset 0x42, CSR Addr 0x54021, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 2 to
					 * cs 2.
					 */
	int8_t cdd_rw_2_1;		/*
					 * Byte offset 0x43, CSR Addr 0x54021, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 2 to
					 * cs 1.
					 */
	int8_t cdd_rw_2_0;		/*
					 * Byte offset 0x44, CSR Addr 0x54022, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 2 to
					 * cs 0.
					 */
	int8_t cdd_rw_1_3;		/*
					 * Byte offset 0x45, CSR Addr 0x54022, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 1 to
					 * cs 3.
					 */
	int8_t cdd_rw_1_2;		/*
					 * Byte offset 0x46, CSR Addr 0x54023, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 1 to
					 * cs 2.
					 */
	int8_t cdd_rw_1_1;		/*
					 * Byte offset 0x47, CSR Addr 0x54023, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 1 to
					 * cs 1.
					 */
	int8_t cdd_rw_1_0;		/*
					 * Byte offset 0x48, CSR Addr 0x54024, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 1 to
					 * cs 0.
					 */
	int8_t cdd_rw_0_3;		/*
					 * Byte offset 0x49, CSR Addr 0x54024, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 0 to
					 * cs 3.
					 */
	int8_t cdd_rw_0_2;		/*
					 * Byte offset 0x4a, CSR Addr 0x54025, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 0 to
					 * cs 2.
					 */
	int8_t cdd_rw_0_1;		/*
					 * Byte offset 0x4b, CSR Addr 0x54025, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 0 to
					 * cs 1.
					 */
	int8_t cdd_rw_0_0;		/*
					 * Byte offset 0x4c, CSR Addr 0x54026, Direction=Out
					 * This is a signed integer value.
					 * Read to write critical delay difference from cs 0 to
					 * cs 0.
					 */
	int8_t cdd_wr_3_3;		/*
					 * Byte offset 0x4d, CSR Addr 0x54026, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 3 to
					 * cs 3.
					 */
	int8_t cdd_wr_3_2;		/*
					 * Byte offset 0x4e, CSR Addr 0x54027, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 3 to
					 * cs 2.
					 */
	int8_t cdd_wr_3_1;		/*
					 * Byte offset 0x4f, CSR Addr 0x54027, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 3 to
					 * cs 1.
					 */
	int8_t cdd_wr_3_0;		/*
					 * Byte offset 0x50, CSR Addr 0x54028, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 3 to
					 * cs 0.
					 */
	int8_t cdd_wr_2_3;		/*
					 * Byte offset 0x51, CSR Addr 0x54028, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 2 to
					 * cs 3.
					 */
	int8_t cdd_wr_2_2;		/*
					 * Byte offset 0x52, CSR Addr 0x54029, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 2 to
					 * cs 2.
					 */
	int8_t cdd_wr_2_1;		/*
					 * Byte offset 0x53, CSR Addr 0x54029, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 2 to
					 * cs 1.
					 */
	int8_t cdd_wr_2_0;		/*
					 * Byte offset 0x54, CSR Addr 0x5402a, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 2 to
					 * cs 0.
					 */
	int8_t cdd_wr_1_3;		/*
					 * Byte offset 0x55, CSR Addr 0x5402a, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 1 to
					 * cs 3.
					 */
	int8_t cdd_wr_1_2;		/*
					 * Byte offset 0x56, CSR Addr 0x5402b, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 1 to
					 * cs 2.
					 */
	int8_t cdd_wr_1_1;		/*
					 * Byte offset 0x57, CSR Addr 0x5402b, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 1 to
					 * cs 1.
					 */
	int8_t cdd_wr_1_0;		/*
					 * Byte offset 0x58, CSR Addr 0x5402c, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 1 to
					 * cs 0.
					 */
	int8_t cdd_wr_0_3;		/*
					 * Byte offset 0x59, CSR Addr 0x5402c, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 0 to
					 * cs 3.
					 */
	int8_t cdd_wr_0_2;		/*
					 * Byte offset 0x5a, CSR Addr 0x5402d, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 0 to
					 * cs 2.
					 */
	int8_t cdd_wr_0_1;		/*
					 * Byte offset 0x5b, CSR Addr 0x5402d, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 0 to
					 * cs 1.
					 */
	int8_t cdd_wr_0_0;		/*
					 * Byte offset 0x5c, CSR Addr 0x5402e, Direction=Out
					 * This is a signed integer value.
					 * Write to read critical delay difference from cs 0 to
					 * cs 0.
					 */
	uint8_t reserved5d;		/*
					 * Byte offset 0x5d, CSR Addr 0x5402e, Direction=In
					 * This field is reserved and must be programmed to 0x00,
					 * excepted for DDR4:
					 * By default, if this parameter is 0, the offset applied at
					 * the end of DDR4 RxEn training resulting in the trained
					 * RxEnDly is 3/8 of the RX preamble width; if reserved5D is
					 * non zero, this offset is used instead (in fine step).
					 */
	uint16_t mr0;			/*
					 * Byte offset 0x5e, CSR Addr 0x5402f, Direction=In
					 * Value of DDR mode register mr0 for all ranks for current
					 * pstate.
					 */
	uint16_t mr1;			/*
					 * Byte offset 0x60, CSR Addr 0x54030, Direction=In
					 * Value of DDR mode register mr1 for all ranks for current
					 * pstate.
					 */
	uint16_t mr2;			/*
					 * Byte offset 0x62, CSR Addr 0x54031, Direction=In
					 * Value of DDR mode register mr2 for all ranks for current
					 * pstate.
					 */
	uint8_t reserved64;		/*
					 * Byte offset 0x64, CSR Addr 0x54032, Direction=In
					 * Reserved64[0] = protect memory reset
					 *   0x0 = dfi_reset_n cannot control CP_MEMRESET_L to
					 *	   devices after training. (Default value)
					 *   0x1 = dfi_reset_n can control CP_MEMRESET_L to
					 *	   devices after training.
					 *
					 * Reserved64[7:1] RFU, must be zero
					 */
	uint8_t reserved65;		/*
					 * Byte offset 0x65, CSR Addr 0x54032, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved66;		/*
					 * Byte offset 0x66, CSR Addr 0x54033, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved67;		/*
					 * Byte offset 0x67, CSR Addr 0x54033, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved68;		/*
					 * Byte offset 0x68, CSR Addr 0x54034, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved69;		/*
					 * Byte offset 0x69, CSR Addr 0x54034, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6a;		/*
					 * Byte offset 0x6a, CSR Addr 0x54035, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6b;		/*
					 * Byte offset 0x6b, CSR Addr 0x54035, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6c;		/*
					 * Byte offset 0x6c, CSR Addr 0x54036, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6d;		/*
					 * Byte offset 0x6d, CSR Addr 0x54036, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6e;		/*
					 * Byte offset 0x6e, CSR Addr 0x54037, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved6f;		/*
					 * Byte offset 0x6f, CSR Addr 0x54037, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved70;		/*
					 * Byte offset 0x70, CSR Addr 0x54038, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved71;		/*
					 * Byte offset 0x71, CSR Addr 0x54038, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved72;		/*
					 * Byte offset 0x72, CSR Addr 0x54039, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved73;		/*
					 * Byte offset 0x73, CSR Addr 0x54039, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t acsmodtctrl0;		/*
					 * Byte offset 0x74, CSR Addr 0x5403a, Direction=In
					 * Odt pattern for accesses targeting rank 0. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl1;		/*
					 * Byte offset 0x75, CSR Addr 0x5403a, Direction=In
					 * Odt pattern for accesses targeting rank 1. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl2;		/*
					 * Byte offset 0x76, CSR Addr 0x5403b, Direction=In
					 * Odt pattern for accesses targeting rank 2. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl3;		/*
					 * Byte offset 0x77, CSR Addr 0x5403b, Direction=In
					 * Odt pattern for accesses targeting rank 3. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl4;		/*
					 * Byte offset 0x78, CSR Addr 0x5403c, Direction=In
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t acsmodtctrl5;		/*
					 * Byte offset 0x79, CSR Addr 0x5403c, Direction=In
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t acsmodtctrl6;		/*
					 * Byte offset 0x7a, CSR Addr 0x5403d, Direction=In
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t acsmodtctrl7;		/*
					 * Byte offset 0x7b, CSR Addr 0x5403d, Direction=In
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved7c;		/*
					 * Byte offset 0x7c, CSR Addr 0x5403e, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved7d;		/*
					 * Byte offset 0x7d, CSR Addr 0x5403e, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved7e;		/*
					 * Byte offset 0x7e, CSR Addr 0x5403f, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved7f;		/*
					 * Byte offset 0x7f, CSR Addr 0x5403f, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved80;		/*
					 * Byte offset 0x80, CSR Addr 0x54040, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved81;		/*
					 * Byte offset 0x81, CSR Addr 0x54040, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved82;		/*
					 * Byte offset 0x82, CSR Addr 0x54041, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved83;		/*
					 * Byte offset 0x83, CSR Addr 0x54041, Direction=N/A
					 * This field is reserved and must be programmed to 0x00.
					 */
	uint8_t reserved84;		/* Byte offset 0x84, CSR Addr 0x54042, Direction=N/A */
	uint8_t reserved85;		/* Byte offset 0x85, CSR Addr 0x54042, Direction=N/A */
	uint8_t reserved86;		/* Byte offset 0x86, CSR Addr 0x54043, Direction=N/A */
	uint8_t reserved87;		/* Byte offset 0x87, CSR Addr 0x54043, Direction=N/A */
	uint8_t reserved88;		/* Byte offset 0x88, CSR Addr 0x54044, Direction=N/A */
	uint8_t reserved89;		/* Byte offset 0x89, CSR Addr 0x54044, Direction=N/A */
	uint8_t reserved8a;		/* Byte offset 0x8a, CSR Addr 0x54045, Direction=N/A */
	uint8_t reserved8b;		/* Byte offset 0x8b, CSR Addr 0x54045, Direction=N/A */
	uint8_t reserved8c;		/* Byte offset 0x8c, CSR Addr 0x54046, Direction=N/A */
	uint8_t reserved8d;		/* Byte offset 0x8d, CSR Addr 0x54046, Direction=N/A */
	uint8_t reserved8e;		/* Byte offset 0x8e, CSR Addr 0x54047, Direction=N/A */
	uint8_t reserved8f;		/* Byte offset 0x8f, CSR Addr 0x54047, Direction=N/A */
	uint8_t reserved90;		/* Byte offset 0x90, CSR Addr 0x54048, Direction=N/A */
	uint8_t reserved91;		/* Byte offset 0x91, CSR Addr 0x54048, Direction=N/A */
	uint8_t reserved92;		/* Byte offset 0x92, CSR Addr 0x54049, Direction=N/A */
	uint8_t reserved93;		/* Byte offset 0x93, CSR Addr 0x54049, Direction=N/A */
	uint8_t reserved94;		/* Byte offset 0x94, CSR Addr 0x5404a, Direction=N/A */
	uint8_t reserved95;		/* Byte offset 0x95, CSR Addr 0x5404a, Direction=N/A */
	uint8_t reserved96;		/* Byte offset 0x96, CSR Addr 0x5404b, Direction=N/A */
	uint8_t reserved97;		/* Byte offset 0x97, CSR Addr 0x5404b, Direction=N/A */
	uint8_t reserved98;		/* Byte offset 0x98, CSR Addr 0x5404c, Direction=N/A */
	uint8_t reserved99;		/* Byte offset 0x99, CSR Addr 0x5404c, Direction=N/A */
	uint8_t reserved9a;		/* Byte offset 0x9a, CSR Addr 0x5404d, Direction=N/A */
	uint8_t reserved9b;		/* Byte offset 0x9b, CSR Addr 0x5404d, Direction=N/A */
	uint8_t reserved9c;		/* Byte offset 0x9c, CSR Addr 0x5404e, Direction=N/A */
	uint8_t reserved9d;		/* Byte offset 0x9d, CSR Addr 0x5404e, Direction=N/A */
	uint8_t reserved9e;		/* Byte offset 0x9e, CSR Addr 0x5404f, Direction=N/A */
	uint8_t reserved9f;		/* Byte offset 0x9f, CSR Addr 0x5404f, Direction=N/A */
	uint8_t reserveda0;		/* Byte offset 0xa0, CSR Addr 0x54050, Direction=N/A */
	uint8_t reserveda1;		/* Byte offset 0xa1, CSR Addr 0x54050, Direction=N/A */
	uint8_t reserveda2;		/* Byte offset 0xa2, CSR Addr 0x54051, Direction=N/A */
	uint8_t reserveda3;		/* Byte offset 0xa3, CSR Addr 0x54051, Direction=N/A */
} __packed __aligned(2);

#endif /* MNPMUSRAMMSGBLOCK_DDR3_H */

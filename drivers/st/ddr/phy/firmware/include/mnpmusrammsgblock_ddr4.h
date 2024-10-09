/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MNPMUSRAMMSGBLOCK_DDR4_H
#define MNPMUSRAMMSGBLOCK_DDR4_H

/* DDR4U_1D training firmware message block structure
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
					 *   0x1 = Program user characterized Vref DQ values per
					 *   DDR4 DRAM device. The message block vrefdqr*nib* fields
					 *   must be populated with the desired per device Vref DQs
					 *   when using this option. Note: this option is not
					 *   applicable in 2D training because these values are
					 *   explicitly trained in 2D.
					 *   0x0 = Program Vref DQ for all DDR4 devices with the
					 *   single value provided in mr6 message block field
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
					 * 0x0 = Firmware skips programming (must be manually
					 * programmed by user prior to training start)
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
					 *   0x01 = Reserved
					 *   0x02 = DDR4 unbuffered
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
					 * sequencectrl[10] = Run Reserved
					 * sequencectrl[11] = Run Reserved
					 * sequencectrl[12] = Run Reserved
					 * sequencectrl[13] = Run Reserved
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
					 * reserved1F[3]: When set redo RdDeskew with PRBS after
					 * (first) WrDqs1D
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
	uint16_t mr3;			/*
					 * Byte offset 0x64, CSR Addr 0x54032, Direction=In
					 * Value of DDR mode register mr3 for all ranks for current
					 * pstate.
					 */
	uint16_t mr4;			/*
					 * Byte offset 0x66, CSR Addr 0x54033, Direction=In
					 * Value of DDR mode register mr4 for all ranks for current
					 * pstate.
					 */
	uint16_t mr5;			/*
					 * Byte offset 0x68, CSR Addr 0x54034, Direction=In
					 * Value of DDR mode register mr5 for all ranks for current
					 * pstate.
					 */
	uint16_t mr6;			/*
					 * Byte offset 0x6a, CSR Addr 0x54035, Direction=In
					 * Value of DDR mode register mr6 for all ranks for current
					 * pstate. Note: The initial VrefDq value and range must be
					 * set in A6:A0.
					 */
	uint8_t x16present;		/*
					 * Byte offset 0x6c, CSR Addr 0x54036, Direction=In
					 * X16 device map. Corresponds to CS[3:0].
					 * x16present[0] = CS0 is populated with X16 devices
					 * x16present[1] = CS1 is populated with X16 devices
					 * x16present[2] = CS2 is populated with X16 devices
					 * x16present[3] = CS3 is populated with X16 devices
					 * x16present[7:4] = Reserved (must be programmed to 0)
					 *
					 * Ranks may not contain mixed device types.
					 */
	uint8_t cssetupgddec;		/*
					 * Byte offset 0x6d, CSR Addr 0x54036, Direction=In
					 * controls timing of chip select signals when DDR4
					 * gear-down mode is active
					 *   0 - Leave delay of chip select timing group signal
					 *   the same both before and after gear-down sync occurs
					 *   1 - Add 1UI of delay to chip select timing group
					 *   signals when geardown-mode is active. This allows CS
					 *   signals to have equal setup and hold time in gear-down
					 *   mode
					 */
	uint16_t rtt_nom_wr_park0;	/*
					 * Byte offset 0x6e, CSR Addr 0x54037, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 0
					 * DRAM:
					 * rtt_nom_wr_park0[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park0[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 0
					 * rtt_nom_wr_park0[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 0
					 * rtt_nom_wr_park0[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 0
					 */
	uint16_t rtt_nom_wr_park1;	/*
					 * Byte offset 0x70, CSR Addr 0x54038, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 1
					 * DRAM:
					 * rtt_nom_wr_park1[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park1[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 1
					 * rtt_nom_wr_park1[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 1
					 * rtt_nom_wr_park1[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 1
					 */
	uint16_t rtt_nom_wr_park2;	/*
					 * Byte offset 0x72, CSR Addr 0x54039, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 2
					 * DRAM:
					 * rtt_nom_wr_park2[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park2[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 2
					 * rtt_nom_wr_park2[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 2
					 * rtt_nom_wr_park2[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 2
					 */
	uint16_t rtt_nom_wr_park3;	/*
					 * Byte offset 0x74, CSR Addr 0x5403a, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 3
					 * DRAM:
					 * rtt_nom_wr_park3[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park3[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 3
					 * rtt_nom_wr_park3[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 3
					 * rtt_nom_wr_park3[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 3
					 */
	uint16_t rtt_nom_wr_park4;	/*
					 * Byte offset 0x76, CSR Addr 0x5403b, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 4
					 * DRAM:
					 * rtt_nom_wr_park4[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park4[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 4
					 * rtt_nom_wr_park4[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 4
					 * rtt_nom_wr_park4[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 4
					 */
	uint16_t rtt_nom_wr_park5;	/*
					 * Byte offset 0x78, CSR Addr 0x5403c, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 5
					 * DRAM:
					 * rtt_nom_wr_park5[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park5[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 5
					 * rtt_nom_wr_park5[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 5
					 * rtt_nom_wr_park5[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 5
					 */
	uint16_t rtt_nom_wr_park6;	/*
					 * Byte offset 0x7a, CSR Addr 0x5403d, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 6
					 * DRAM:
					 * rtt_nom_wr_park6[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park6[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 6
					 * rtt_nom_wr_park6[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 6
					 * rtt_nom_wr_park6[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 6
					 */
	uint16_t rtt_nom_wr_park7;	/*
					 * Byte offset 0x7c, CSR Addr 0x5403e, Direction=In
					 * Optional RTT_NOM, RTT_WR and RTT_PARK values for rank 7
					 * DRAM:
					 * rtt_nom_wr_park7[0] = 1: Option is enable (otherwise,
					 * remaining bit fields are don't care)
					 * rtt_nom_wr_park7[5:3]: Optional RTT_NOM value to be used
					 * in mr1[10:8] for rank 7
					 * rtt_nom_wr_park7[11:9]: Optional RTT_WR value to be used
					 * in mr2[11:9] for rank 7
					 * rtt_nom_wr_park7[8:6]: Optional RTT_PARK value to be used
					 * in mr5[8:6] for rank 7
					 */
	uint8_t acsmodtctrl0;		/*
					 * Byte offset 0x7e, CSR Addr 0x5403f, Direction=In
					 * Odt pattern for accesses targeting rank 0. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl1;		/*
					 * Byte offset 0x7f, CSR Addr 0x5403f, Direction=In
					 * Odt pattern for accesses targeting rank 1. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl2;		/*
					 * Byte offset 0x80, CSR Addr 0x54040, Direction=In
					 * Odt pattern for accesses targeting rank 2. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl3;		/*
					 * Byte offset 0x81, CSR Addr 0x54040, Direction=In
					 * Odt pattern for accesses targeting rank 3. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl4;		/*
					 * Byte offset 0x82, CSR Addr 0x54041, Direction=In
					 * Odt pattern for accesses targeting rank 4. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl5;		/*
					 * Byte offset 0x83, CSR Addr 0x54041, Direction=In
					 * Odt pattern for accesses targeting rank 5. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl6;		/*
					 * Byte offset 0x84, CSR Addr 0x54042, Direction=In
					 * Odt pattern for accesses targeting rank 6. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t acsmodtctrl7;		/*
					 * Byte offset 0x85, CSR Addr 0x54042, Direction=In
					 * Odt pattern for accesses targeting rank 7. [3:0] is used
					 * for write ODT [7:4] is used for read ODT
					 */
	uint8_t vrefdqr0nib0;		/*
					 * Byte offset 0x86, CSR Addr 0x54043, Direction=InOut
					 * VrefDq for rank 0 nibble 0. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr0nib1;		/*
					 * Byte offset 0x87, CSR Addr 0x54043, Direction=InOut
					 * VrefDq for rank 0 nibble 1. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib0 for x8 or x16 devices.
					 */
	uint8_t vrefdqr0nib2;		/*
					 * Byte offset 0x88, CSR Addr 0x54044, Direction=InOut
					 * VrefDq for rank 0 nibble 2. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib0 for x16 devices.
					 */
	uint8_t vrefdqr0nib3;		/*
					 * Byte offset 0x89, CSR Addr 0x54044, Direction=InOut
					 * VrefDq for rank 0 nibble 3. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib0 for x16 devices, or
					 * vrefdqr0nib2 for x8 devices.
					 */
	uint8_t vrefdqr0nib4;		/*
					 * Byte offset 0x8a, CSR Addr 0x54045, Direction=InOut
					 * VrefDq for rank 0 nibble 4. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr0nib5;		/*
					 * Byte offset 0x8b, CSR Addr 0x54045, Direction=InOut
					 * VrefDq for rank 0 nibble 5. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib4 for x8 or x16 devices.
					 */
	uint8_t vrefdqr0nib6;		/*
					 * Byte offset 0x8c, CSR Addr 0x54046, Direction=InOut
					 * VrefDq for rank 0 nibble 6. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib4 for x16 devices.
					 */
	uint8_t vrefdqr0nib7;		/*
					 * Byte offset 0x8d, CSR Addr 0x54046, Direction=InOut
					 * VrefDq for rank 0 nibble 7. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib4 for x16 devices,
					 * or vrefdqr0nib6 for x8 devices.
					 */
	uint8_t vrefdqr0nib8;		/*
					 * Byte offset 0x8e, CSR Addr 0x54047, Direction=InOut
					 * VrefDq for rank 0 nibble 8. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr0nib9;		/*
					 * Byte offset 0x8f, CSR Addr 0x54047, Direction=InOut
					 * VrefDq for rank 0 nibble 9. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib8 for x8 or x16 devices.
					 */
	uint8_t vrefdqr0nib10;		/*
					 * Byte offset 0x90, CSR Addr 0x54048, Direction=InOut
					 * VrefDq for rank 0 nibble 10. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib8 for x16 devices.
					 */
	uint8_t vrefdqr0nib11;		/*
					 * Byte offset 0x91, CSR Addr 0x54048, Direction=InOut
					 * VrefDq for rank 0 nibble 11. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib8 for x16 devices,
					 * or vrefdqr0nib10 for x8 devices.
					 */
	uint8_t vrefdqr0nib12;		/*
					 * Byte offset 0x92, CSR Addr 0x54049, Direction=InOut
					 * VrefDq for rank 0 nibble 12. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr0nib13;		/*
					 * Byte offset 0x93, CSR Addr 0x54049, Direction=InOut
					 * VrefDq for rank 0 nibble 13. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib12 for x8 or x16 devices.
					 */
	uint8_t vrefdqr0nib14;		/*
					 * Byte offset 0x94, CSR Addr 0x5404a, Direction=InOut
					 * VrefDq for rank 0 nibble 14. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib12 for x16 devices.
					 */
	uint8_t vrefdqr0nib15;		/*
					 * Byte offset 0x95, CSR Addr 0x5404a, Direction=InOut
					 * VrefDq for rank 0 nibble 15. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib12 for x16 devices,
					 * or vrefdqr0nib14 for x8 devices.
					 */
	uint8_t vrefdqr0nib16;		/*
					 * Byte offset 0x96, CSR Addr 0x5404b, Direction=InOut
					 * VrefDq for rank 0 nibble 16. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr0nib17;		/*
					 * Byte offset 0x97, CSR Addr 0x5404b, Direction=InOut
					 * VrefDq for rank 0 nibble 17. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib16 for x8 or x16 devices.
					 */
	uint8_t vrefdqr0nib18;		/*
					 * Byte offset 0x98, CSR Addr 0x5404c, Direction=InOut
					 * VrefDq for rank 0 nibble 18. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib16 for x16 devices.
					 */
	uint8_t vrefdqr0nib19;		/*
					 * Byte offset 0x99, CSR Addr 0x5404c, Direction=InOut
					 * VrefDq for rank 0 nibble 19. Specifies mr6[6:0].
					 * Identical to vrefdqr0nib16 for x16 devices,
					 * or vrefdqr0nib18 for x8 devices.
					 */
	uint8_t vrefdqr1nib0;		/*
					 * Byte offset 0x9a, CSR Addr 0x5404d, Direction=InOut
					 * VrefDq for rank 1 nibble 0. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr1nib1;		/*
					 * Byte offset 0x9b, CSR Addr 0x5404d, Direction=InOut
					 * VrefDq for rank 1 nibble 1. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib0 for x8 or x16 devices.
					 */
	uint8_t vrefdqr1nib2;		/*
					 * Byte offset 0x9c, CSR Addr 0x5404e, Direction=InOut
					 * VrefDq for rank 1 nibble 2. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib0 for x16 devices.
					 */
	uint8_t vrefdqr1nib3;		/*
					 * Byte offset 0x9d, CSR Addr 0x5404e, Direction=InOut
					 * VrefDq for rank 1 nibble 3. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib0 for x16 devices,
					 * or vrefdqr1nib2 for x8 devices.
					 */
	uint8_t vrefdqr1nib4;		/*
					 * Byte offset 0x9e, CSR Addr 0x5404f, Direction=InOut
					 * VrefDq for rank 1 nibble 4. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr1nib5;		/*
					 * Byte offset 0x9f, CSR Addr 0x5404f, Direction=InOut
					 * VrefDq for rank 1 nibble 5. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib4 for x8 or x16 devices.
					 */
	uint8_t vrefdqr1nib6;		/*
					 * Byte offset 0xa0, CSR Addr 0x54050, Direction=InOut
					 * VrefDq for rank 1 nibble 6. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib4 for x16 devices.
					 */
	uint8_t vrefdqr1nib7;		/*
					 * Byte offset 0xa1, CSR Addr 0x54050, Direction=InOut
					 * VrefDq for rank 1 nibble 7. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib4 for x16 devices,
					 * or vrefdqr1nib6 for x8 devices.
					 */
	uint8_t vrefdqr1nib8;		/*
					 * Byte offset 0xa2, CSR Addr 0x54051, Direction=InOut
					 * VrefDq for rank 1 nibble 8. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr1nib9;		/*
					 * Byte offset 0xa3, CSR Addr 0x54051, Direction=InOut
					 * VrefDq for rank 1 nibble 9. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib8 for x8 or x16 devices.
					 */
	uint8_t vrefdqr1nib10;		/*
					 * Byte offset 0xa4, CSR Addr 0x54052, Direction=InOut
					 * VrefDq for rank 1 nibble 10. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib8 for x16 devices.
					 */
	uint8_t vrefdqr1nib11;		/*
					 * Byte offset 0xa5, CSR Addr 0x54052, Direction=InOut
					 * VrefDq for rank 1 nibble 11. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib8 for x16 devices,
					 * or vrefdqr1nib10 for x8 devices.
					 */
	uint8_t vrefdqr1nib12;		/*
					 * Byte offset 0xa6, CSR Addr 0x54053, Direction=InOut
					 * VrefDq for rank 1 nibble 12. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr1nib13;		/*
					 * Byte offset 0xa7, CSR Addr 0x54053, Direction=InOut
					 * VrefDq for rank 1 nibble 13. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib12 for x8 or x16 devices.
					 */
	uint8_t vrefdqr1nib14;		/*
					 * Byte offset 0xa8, CSR Addr 0x54054, Direction=InOut
					 * VrefDq for rank 1 nibble 14. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib12 for x16 devices.
					 */
	uint8_t vrefdqr1nib15;		/*
					 * Byte offset 0xa9, CSR Addr 0x54054, Direction=InOut
					 * VrefDq for rank 1 nibble 15. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib12 for x16 devices,
					 * or vrefdqr1nib14 for x8 devices.
					 */
	uint8_t vrefdqr1nib16;		/*
					 * Byte offset 0xaa, CSR Addr 0x54055, Direction=InOut
					 * VrefDq for rank 1 nibble 16. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr1nib17;		/*
					 * Byte offset 0xab, CSR Addr 0x54055, Direction=InOut
					 * VrefDq for rank 1 nibble 17. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib16 for x8 or x16 devices.
					 */
	uint8_t vrefdqr1nib18;		/*
					 * Byte offset 0xac, CSR Addr 0x54056, Direction=InOut
					 * VrefDq for rank 1 nibble 18. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib16 for x16 devices.
					 */
	uint8_t vrefdqr1nib19;		/*
					 * Byte offset 0xad, CSR Addr 0x54056, Direction=InOut
					 * VrefDq for rank 1 nibble 19. Specifies mr6[6:0].
					 * Identical to vrefdqr1nib16 for x16 devices,
					 * or vrefdqr1nib18 for x8 devices.
					 */
	uint8_t vrefdqr2nib0;		/*
					 * Byte offset 0xae, CSR Addr 0x54057, Direction=InOut
					 * VrefDq for rank 2 nibble 0. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr2nib1;		/*
					 * Byte offset 0xaf, CSR Addr 0x54057, Direction=InOut
					 * VrefDq for rank 2 nibble 1. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib0 for x8 or x16 devices.
					 */
	uint8_t vrefdqr2nib2;		/*
					 * Byte offset 0xb0, CSR Addr 0x54058, Direction=InOut
					 * VrefDq for rank 2 nibble 2. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib0 for x16 devices.
					 */
	uint8_t vrefdqr2nib3;		/*
					 * Byte offset 0xb1, CSR Addr 0x54058, Direction=InOut
					 * VrefDq for rank 2 nibble 3. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib0 for x16 devices,
					 * or vrefdqr2nib2 for x8 devices.
					 */
	uint8_t vrefdqr2nib4;		/*
					 * Byte offset 0xb2, CSR Addr 0x54059, Direction=InOut
					 * VrefDq for rank 2 nibble 4. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr2nib5;		/*
					 * Byte offset 0xb3, CSR Addr 0x54059, Direction=InOut
					 * VrefDq for rank 2 nibble 5. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib4 for x8 or x16 devices.
					 */
	uint8_t vrefdqr2nib6;		/*
					 * Byte offset 0xb4, CSR Addr 0x5405a, Direction=InOut
					 * VrefDq for rank 2 nibble 6. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib4 for x16 devices.
					 */
	uint8_t vrefdqr2nib7;		/*
					 * Byte offset 0xb5, CSR Addr 0x5405a, Direction=InOut
					 * VrefDq for rank 2 nibble 7. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib4 for x16 devices,
					 * or vrefdqr2nib6 for x8 devices.
					 */
	uint8_t vrefdqr2nib8;		/*
					 * Byte offset 0xb6, CSR Addr 0x5405b, Direction=InOut
					 * VrefDq for rank 2 nibble 8. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr2nib9;		/*
					 * Byte offset 0xb7, CSR Addr 0x5405b, Direction=InOut
					 * VrefDq for rank 2 nibble 9. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib8 for x8 or x16 devices.
					 */
	uint8_t vrefdqr2nib10;		/*
					 * Byte offset 0xb8, CSR Addr 0x5405c, Direction=InOut
					 * VrefDq for rank 2 nibble 10. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib8 for x16 devices.
					 */
	uint8_t vrefdqr2nib11;		/*
					 * Byte offset 0xb9, CSR Addr 0x5405c, Direction=InOut
					 * VrefDq for rank 2 nibble 11. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib8 for x16 devices,
					 * or vrefdqr2nib10 for x8 devices.
					 */
	uint8_t vrefdqr2nib12;		/*
					 * Byte offset 0xba, CSR Addr 0x5405d, Direction=InOut
					 * VrefDq for rank 2 nibble 12. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr2nib13;		/*
					 * Byte offset 0xbb, CSR Addr 0x5405d, Direction=InOut
					 * VrefDq for rank 2 nibble 13. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib12 for x8 or x16 devices.
					 */
	uint8_t vrefdqr2nib14;		/*
					 * Byte offset 0xbc, CSR Addr 0x5405e, Direction=InOut
					 * VrefDq for rank 2 nibble 14. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib12 for x16 devices.
					 */
	uint8_t vrefdqr2nib15;		/*
					 * Byte offset 0xbd, CSR Addr 0x5405e, Direction=InOut
					 * VrefDq for rank 2 nibble 15. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib12 for x16 devices,
					 * or vrefdqr2nib14 for x8 devices.
					 */
	uint8_t vrefdqr2nib16;		/*
					 * Byte offset 0xbe, CSR Addr 0x5405f, Direction=InOut
					 * VrefDq for rank 2 nibble 16. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr2nib17;		/*
					 * Byte offset 0xbf, CSR Addr 0x5405f, Direction=InOut
					 * VrefDq for rank 2 nibble 17. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib16 for x8 or x16 devices.
					 */
	uint8_t vrefdqr2nib18;		/*
					 * Byte offset 0xc0, CSR Addr 0x54060, Direction=InOut
					 * VrefDq for rank 2 nibble 18. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib16 for x16 devices.
					 */
	uint8_t vrefdqr2nib19;		/*
					 * Byte offset 0xc1, CSR Addr 0x54060, Direction=InOut
					 * VrefDq for rank 2 nibble 19. Specifies mr6[6:0].
					 * Identical to vrefdqr2nib16 for x16 devices,
					 * or vrefdqr2nib18 for x8 devices.
					 */
	uint8_t vrefdqr3nib0;		/*
					 * Byte offset 0xc2, CSR Addr 0x54061, Direction=InOut
					 * VrefDq for rank 3 nibble 0. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr3nib1;		/*
					 * Byte offset 0xc3, CSR Addr 0x54061, Direction=InOut
					 * VrefDq for rank 3 nibble 1. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib0 for x8 or x16 devices.
					 */
	uint8_t vrefdqr3nib2;		/*
					 * Byte offset 0xc4, CSR Addr 0x54062, Direction=InOut
					 * VrefDq for rank 3 nibble 2. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib0 for x16 devices.
					 */
	uint8_t vrefdqr3nib3;		/*
					 * Byte offset 0xc5, CSR Addr 0x54062, Direction=InOut
					 * VrefDq for rank 3 nibble 3. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib0 for x16 devices,
					 * or vrefdqr3nib2 for x8 devices.
					 */
	uint8_t vrefdqr3nib4;		/*
					 * Byte offset 0xc6, CSR Addr 0x54063, Direction=InOut
					 * VrefDq for rank 3 nibble 4. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr3nib5;		/*
					 * Byte offset 0xc7, CSR Addr 0x54063, Direction=InOut
					 * VrefDq for rank 3 nibble 5. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib4 for x8 or x16 devices.
					 */
	uint8_t vrefdqr3nib6;		/*
					 * Byte offset 0xc8, CSR Addr 0x54064, Direction=InOut
					 * VrefDq for rank 3 nibble 6. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib4 for x16 devices.
					 */
	uint8_t vrefdqr3nib7;		/*
					 * Byte offset 0xc9, CSR Addr 0x54064, Direction=InOut
					 * VrefDq for rank 3 nibble 7. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib4 for x16 devices,
					 * or vrefdqr3nib6 for x8 devices.
					 */
	uint8_t vrefdqr3nib8;		/*
					 * Byte offset 0xca, CSR Addr 0x54065, Direction=InOut
					 * VrefDq for rank 3 nibble 8. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr3nib9;		/*
					 * Byte offset 0xcb, CSR Addr 0x54065, Direction=InOut
					 * VrefDq for rank 3 nibble 9. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib8 for x8 or x16 devices.
					 */
	uint8_t vrefdqr3nib10;		/*
					 * Byte offset 0xcc, CSR Addr 0x54066, Direction=InOut
					 * VrefDq for rank 3 nibble 10. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib8 for x16 devices.
					 */
	uint8_t vrefdqr3nib11;		/*
					 * Byte offset 0xcd, CSR Addr 0x54066, Direction=InOut
					 * VrefDq for rank 3 nibble 11. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib8 for x16 devices,
					 * or vrefdqr3nib10 for x8 devices.
					 */
	uint8_t vrefdqr3nib12;		/*
					 * Byte offset 0xce, CSR Addr 0x54067, Direction=InOut
					 * VrefDq for rank 3 nibble 12. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr3nib13;		/*
					 * Byte offset 0xcf, CSR Addr 0x54067, Direction=InOut
					 * VrefDq for rank 3 nibble 13. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib12 for x8 or x16 devices.
					 */
	uint8_t vrefdqr3nib14;		/*
					 * Byte offset 0xd0, CSR Addr 0x54068, Direction=InOut
					 * VrefDq for rank 3 nibble 14. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib12 for x16 devices.
					 */
	uint8_t vrefdqr3nib15;		/*
					 * Byte offset 0xd1, CSR Addr 0x54068, Direction=InOut
					 * VrefDq for rank 3 nibble 15. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib12 for x16 devices,
					 * or vrefdqr3nib14 for x8 devices.
					 */
	uint8_t vrefdqr3nib16;		/*
					 * Byte offset 0xd2, CSR Addr 0x54069, Direction=InOut
					 * VrefDq for rank 3 nibble 16. Specifies mr6[6:0]
					 */
	uint8_t vrefdqr3nib17;		/*
					 * Byte offset 0xd3, CSR Addr 0x54069, Direction=InOut
					 * VrefDq for rank 3 nibble 17. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib16 for x8 or x16 devices.
					 */
	uint8_t vrefdqr3nib18;		/*
					 * Byte offset 0xd4, CSR Addr 0x5406a, Direction=InOut
					 * VrefDq for rank 3 nibble 18. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib16 for x16 devices.
					 */
	uint8_t vrefdqr3nib19;		/*
					 * Byte offset 0xd5, CSR Addr 0x5406a, Direction=InOut
					 * VrefDq for rank 3 nibble 19. Specifies mr6[6:0].
					 * Identical to vrefdqr3nib16 for x16 devices,
					 * or vrefdqr3nib18 for x8 devices.
					 */
	uint8_t reservedd6;		/* Byte offset 0xd6, CSR Addr 0x5406b, Direction=N/A */
	uint8_t reservedd7;		/* Byte offset 0xd7, CSR Addr 0x5406b, Direction=N/A */
	uint8_t reservedd8;		/* Byte offset 0xd8, CSR Addr 0x5406c, Direction=N/A */
	uint8_t reservedd9;		/* Byte offset 0xd9, CSR Addr 0x5406c, Direction=N/A */
	uint8_t reservedda;		/* Byte offset 0xda, CSR Addr 0x5406d, Direction=N/A */
	uint8_t reserveddb;		/* Byte offset 0xdb, CSR Addr 0x5406d, Direction=N/A */
	uint8_t reserveddc;		/* Byte offset 0xdc, CSR Addr 0x5406e, Direction=N/A */
	uint8_t reserveddd;		/* Byte offset 0xdd, CSR Addr 0x5406e, Direction=N/A */
	uint8_t reservedde;		/* Byte offset 0xde, CSR Addr 0x5406f, Direction=N/A */
	uint8_t reserveddf;		/* Byte offset 0xdf, CSR Addr 0x5406f, Direction=N/A */
	uint8_t reservede0;		/* Byte offset 0xe0, CSR Addr 0x54070, Direction=N/A */
	uint8_t reservede1;		/* Byte offset 0xe1, CSR Addr 0x54070, Direction=N/A */
	uint8_t reservede2;		/* Byte offset 0xe2, CSR Addr 0x54071, Direction=N/A */
	uint8_t reservede3;		/* Byte offset 0xe3, CSR Addr 0x54071, Direction=N/A */
	uint8_t reservede4;		/* Byte offset 0xe4, CSR Addr 0x54072, Direction=N/A */
	uint8_t reservede5;		/* Byte offset 0xe5, CSR Addr 0x54072, Direction=N/A */
	uint8_t reservede6;		/* Byte offset 0xe6, CSR Addr 0x54073, Direction=N/A */
	uint8_t reservede7;		/* Byte offset 0xe7, CSR Addr 0x54073, Direction=N/A */
	uint8_t reservede8;		/* Byte offset 0xe8, CSR Addr 0x54074, Direction=N/A */
	uint8_t reservede9;		/* Byte offset 0xe9, CSR Addr 0x54074, Direction=N/A */
	uint8_t reservedea;		/* Byte offset 0xea, CSR Addr 0x54075, Direction=N/A */
	uint8_t reservedeb;		/* Byte offset 0xeb, CSR Addr 0x54075, Direction=N/A */
	uint8_t reservedec;		/* Byte offset 0xec, CSR Addr 0x54076, Direction=N/A */
	uint8_t reserveded;		/* Byte offset 0xed, CSR Addr 0x54076, Direction=N/A */
	uint8_t reservedee;		/* Byte offset 0xee, CSR Addr 0x54077, Direction=N/A */
	uint8_t reservedef;		/* Byte offset 0xef, CSR Addr 0x54077, Direction=N/A */
	uint8_t reservedf0;		/* Byte offset 0xf0, CSR Addr 0x54078, Direction=N/A */
	uint8_t reservedf1;		/* Byte offset 0xf1, CSR Addr 0x54078, Direction=N/A */
	uint8_t reservedf2;		/* Byte offset 0xf2, CSR Addr 0x54079, Direction=N/A */
	uint8_t reservedf3;		/* Byte offset 0xf3, CSR Addr 0x54079, Direction=N/A */
	uint8_t reservedf4;		/* Byte offset 0xf4, CSR Addr 0x5407a, Direction=N/A */
	uint8_t reservedf5;		/* Byte offset 0xf5, CSR Addr 0x5407a, Direction=N/A */
	uint8_t reservedf6;		/* Byte offset 0xf6, CSR Addr 0x5407b, Direction=N/A */
	uint8_t reservedf7;		/* Byte offset 0xf7, CSR Addr 0x5407b, Direction=N/A */
	uint8_t reservedf8;		/* Byte offset 0xf8, CSR Addr 0x5407c, Direction=N/A */
	uint8_t reservedf9;		/* Byte offset 0xf9, CSR Addr 0x5407c, Direction=N/A */
	uint8_t reservedfa;		/* Byte offset 0xfa, CSR Addr 0x5407d, Direction=N/A */
	uint8_t reservedfb;		/* Byte offset 0xfb, CSR Addr 0x5407d, Direction=N/A */
	uint8_t reservedfc;		/* Byte offset 0xfc, CSR Addr 0x5407e, Direction=N/A */
	uint8_t reservedfd;		/* Byte offset 0xfd, CSR Addr 0x5407e, Direction=N/A */
	uint8_t reservedfe;		/* Byte offset 0xfe, CSR Addr 0x5407f, Direction=N/A */
	uint8_t reservedff;		/* Byte offset 0xff, CSR Addr 0x5407f, Direction=N/A */
	uint8_t reserved100;		/* Byte offset 0x100, CSR Addr 0x54080, Direction=N/A */
	uint8_t reserved101;		/* Byte offset 0x101, CSR Addr 0x54080, Direction=N/A */
	uint8_t reserved102;		/* Byte offset 0x102, CSR Addr 0x54081, Direction=N/A */
	uint8_t reserved103;		/* Byte offset 0x103, CSR Addr 0x54081, Direction=N/A */
	uint8_t reserved104;		/* Byte offset 0x104, CSR Addr 0x54082, Direction=N/A */
	uint8_t reserved105;		/* Byte offset 0x105, CSR Addr 0x54082, Direction=N/A */
	uint8_t reserved106;		/* Byte offset 0x106, CSR Addr 0x54083, Direction=N/A */
	uint8_t reserved107;		/* Byte offset 0x107, CSR Addr 0x54083, Direction=N/A */
	uint8_t reserved108;		/* Byte offset 0x108, CSR Addr 0x54084, Direction=N/A */
	uint8_t reserved109;		/* Byte offset 0x109, CSR Addr 0x54084, Direction=N/A */
	uint8_t reserved10a;		/* Byte offset 0x10a, CSR Addr 0x54085, Direction=N/A */
	uint8_t reserved10b;		/* Byte offset 0x10b, CSR Addr 0x54085, Direction=N/A */
	uint8_t reserved10c;		/* Byte offset 0x10c, CSR Addr 0x54086, Direction=N/A */
	uint8_t reserved10d;		/* Byte offset 0x10d, CSR Addr 0x54086, Direction=N/A */
	uint8_t reserved10e;		/* Byte offset 0x10e, CSR Addr 0x54087, Direction=N/A */
	uint8_t reserved10f;		/* Byte offset 0x10f, CSR Addr 0x54087, Direction=N/A */
	uint8_t reserved110;		/* Byte offset 0x110, CSR Addr 0x54088, Direction=N/A */
	uint8_t reserved111;		/* Byte offset 0x111, CSR Addr 0x54088, Direction=N/A */
	uint8_t reserved112;		/* Byte offset 0x112, CSR Addr 0x54089, Direction=N/A */
	uint8_t reserved113;		/* Byte offset 0x113, CSR Addr 0x54089, Direction=N/A */
	uint8_t reserved114;		/* Byte offset 0x114, CSR Addr 0x5408a, Direction=N/A */
	uint8_t reserved115;		/* Byte offset 0x115, CSR Addr 0x5408a, Direction=N/A */
	uint8_t reserved116;		/* Byte offset 0x116, CSR Addr 0x5408b, Direction=N/A */
	uint8_t reserved117;		/* Byte offset 0x117, CSR Addr 0x5408b, Direction=N/A */
	uint8_t reserved118;		/* Byte offset 0x118, CSR Addr 0x5408c, Direction=N/A */
	uint8_t reserved119;		/* Byte offset 0x119, CSR Addr 0x5408c, Direction=N/A */
	uint8_t reserved11a;		/* Byte offset 0x11a, CSR Addr 0x5408d, Direction=N/A */
	uint8_t reserved11b;		/* Byte offset 0x11b, CSR Addr 0x5408d, Direction=N/A */
	uint8_t reserved11c;		/* Byte offset 0x11c, CSR Addr 0x5408e, Direction=N/A */
	uint8_t reserved11d;		/* Byte offset 0x11d, CSR Addr 0x5408e, Direction=N/A */
	uint8_t reserved11e;		/* Byte offset 0x11e, CSR Addr 0x5408f, Direction=N/A */
	uint8_t reserved11f;		/* Byte offset 0x11f, CSR Addr 0x5408f, Direction=N/A */
	uint8_t reserved120;		/* Byte offset 0x120, CSR Addr 0x54090, Direction=N/A */
	uint8_t reserved121;		/* Byte offset 0x121, CSR Addr 0x54090, Direction=N/A */
	uint8_t reserved122;		/* Byte offset 0x122, CSR Addr 0x54091, Direction=N/A */
	uint8_t reserved123;		/* Byte offset 0x123, CSR Addr 0x54091, Direction=N/A */
	uint8_t reserved124;		/* Byte offset 0x124, CSR Addr 0x54092, Direction=N/A */
	uint8_t reserved125;		/* Byte offset 0x125, CSR Addr 0x54092, Direction=N/A */
	uint8_t reserved126;		/* Byte offset 0x126, CSR Addr 0x54093, Direction=N/A */
	uint8_t reserved127;		/* Byte offset 0x127, CSR Addr 0x54093, Direction=N/A */
	uint8_t reserved128;		/* Byte offset 0x128, CSR Addr 0x54094, Direction=N/A */
	uint8_t reserved129;		/* Byte offset 0x129, CSR Addr 0x54094, Direction=N/A */
	uint8_t reserved12a;		/* Byte offset 0x12a, CSR Addr 0x54095, Direction=N/A */
	uint8_t reserved12b;		/* Byte offset 0x12b, CSR Addr 0x54095, Direction=N/A */
	uint8_t reserved12c;		/* Byte offset 0x12c, CSR Addr 0x54096, Direction=N/A */
	uint8_t reserved12d;		/* Byte offset 0x12d, CSR Addr 0x54096, Direction=N/A */
	uint8_t reserved12e;		/* Byte offset 0x12e, CSR Addr 0x54097, Direction=N/A */
	uint8_t reserved12f;		/* Byte offset 0x12f, CSR Addr 0x54097, Direction=N/A */
	uint8_t reserved130;		/* Byte offset 0x130, CSR Addr 0x54098, Direction=N/A */
	uint8_t reserved131;		/* Byte offset 0x131, CSR Addr 0x54098, Direction=N/A */
	uint8_t reserved132;		/* Byte offset 0x132, CSR Addr 0x54099, Direction=N/A */
	uint8_t reserved133;		/* Byte offset 0x133, CSR Addr 0x54099, Direction=N/A */
	uint8_t reserved134;		/* Byte offset 0x134, CSR Addr 0x5409a, Direction=N/A */
	uint8_t reserved135;		/* Byte offset 0x135, CSR Addr 0x5409a, Direction=N/A */
	uint8_t reserved136;		/* Byte offset 0x136, CSR Addr 0x5409b, Direction=N/A */
	uint8_t reserved137;		/* Byte offset 0x137, CSR Addr 0x5409b, Direction=N/A */
	uint8_t reserved138;		/* Byte offset 0x138, CSR Addr 0x5409c, Direction=N/A */
	uint8_t reserved139;		/* Byte offset 0x139, CSR Addr 0x5409c, Direction=N/A */
	uint8_t reserved13a;		/* Byte offset 0x13a, CSR Addr 0x5409d, Direction=N/A */
	uint8_t reserved13b;		/* Byte offset 0x13b, CSR Addr 0x5409d, Direction=N/A */
	uint8_t reserved13c;		/* Byte offset 0x13c, CSR Addr 0x5409e, Direction=N/A */
	uint8_t reserved13d;		/* Byte offset 0x13d, CSR Addr 0x5409e, Direction=N/A */
	uint8_t reserved13e;		/* Byte offset 0x13e, CSR Addr 0x5409f, Direction=N/A */
	uint8_t reserved13f;		/* Byte offset 0x13f, CSR Addr 0x5409f, Direction=N/A */
	uint8_t reserved140;		/* Byte offset 0x140, CSR Addr 0x540a0, Direction=N/A */
	uint8_t reserved141;		/* Byte offset 0x141, CSR Addr 0x540a0, Direction=N/A */
	uint8_t reserved142;		/* Byte offset 0x142, CSR Addr 0x540a1, Direction=N/A */
	uint8_t reserved143;		/* Byte offset 0x143, CSR Addr 0x540a1, Direction=N/A */
	uint8_t reserved144;		/* Byte offset 0x144, CSR Addr 0x540a2, Direction=N/A */
	uint8_t reserved145;		/* Byte offset 0x145, CSR Addr 0x540a2, Direction=N/A */
	uint8_t reserved146;		/* Byte offset 0x146, CSR Addr 0x540a3, Direction=N/A */
	uint8_t reserved147;		/* Byte offset 0x147, CSR Addr 0x540a3, Direction=N/A */
	uint8_t reserved148;		/* Byte offset 0x148, CSR Addr 0x540a4, Direction=N/A */
	uint8_t reserved149;		/* Byte offset 0x149, CSR Addr 0x540a4, Direction=N/A */
	uint8_t reserved14a;		/* Byte offset 0x14a, CSR Addr 0x540a5, Direction=N/A */
	uint8_t reserved14b;		/* Byte offset 0x14b, CSR Addr 0x540a5, Direction=N/A */
	uint8_t reserved14c;		/* Byte offset 0x14c, CSR Addr 0x540a6, Direction=N/A */
	uint8_t reserved14d;		/* Byte offset 0x14d, CSR Addr 0x540a6, Direction=N/A */
	uint8_t reserved14e;		/* Byte offset 0x14e, CSR Addr 0x540a7, Direction=N/A */
	uint8_t reserved14f;		/* Byte offset 0x14f, CSR Addr 0x540a7, Direction=N/A */
	uint8_t reserved150;		/* Byte offset 0x150, CSR Addr 0x540a8, Direction=N/A */
	uint8_t reserved151;		/* Byte offset 0x151, CSR Addr 0x540a8, Direction=N/A */
	uint8_t reserved152;		/* Byte offset 0x152, CSR Addr 0x540a9, Direction=N/A */
	uint8_t reserved153;		/* Byte offset 0x153, CSR Addr 0x540a9, Direction=N/A */
	uint8_t reserved154;		/* Byte offset 0x154, CSR Addr 0x540aa, Direction=N/A */
	uint8_t reserved155;		/* Byte offset 0x155, CSR Addr 0x540aa, Direction=N/A */
	uint8_t reserved156;		/* Byte offset 0x156, CSR Addr 0x540ab, Direction=N/A */
	uint8_t reserved157;		/* Byte offset 0x157, CSR Addr 0x540ab, Direction=N/A */
	uint8_t reserved158;		/* Byte offset 0x158, CSR Addr 0x540ac, Direction=N/A */
	uint8_t reserved159;		/* Byte offset 0x159, CSR Addr 0x540ac, Direction=N/A */
	uint8_t reserved15a;		/* Byte offset 0x15a, CSR Addr 0x540ad, Direction=N/A */
	uint8_t reserved15b;		/* Byte offset 0x15b, CSR Addr 0x540ad, Direction=N/A */
	uint8_t reserved15c;		/* Byte offset 0x15c, CSR Addr 0x540ae, Direction=N/A */
	uint8_t reserved15d;		/* Byte offset 0x15d, CSR Addr 0x540ae, Direction=N/A */
	uint8_t reserved15e;		/* Byte offset 0x15e, CSR Addr 0x540af, Direction=N/A */
	uint8_t reserved15f;		/* Byte offset 0x15f, CSR Addr 0x540af, Direction=N/A */
	uint8_t reserved160;		/* Byte offset 0x160, CSR Addr 0x540b0, Direction=N/A */
	uint8_t reserved161;		/* Byte offset 0x161, CSR Addr 0x540b0, Direction=N/A */
	uint8_t reserved162;		/* Byte offset 0x162, CSR Addr 0x540b1, Direction=N/A */
	uint8_t reserved163;		/* Byte offset 0x163, CSR Addr 0x540b1, Direction=N/A */
	uint8_t reserved164;		/* Byte offset 0x164, CSR Addr 0x540b2, Direction=N/A */
	uint8_t reserved165;		/* Byte offset 0x165, CSR Addr 0x540b2, Direction=N/A */
	uint8_t reserved166;		/* Byte offset 0x166, CSR Addr 0x540b3, Direction=N/A */
	uint8_t reserved167;		/* Byte offset 0x167, CSR Addr 0x540b3, Direction=N/A */
	uint8_t reserved168;		/* Byte offset 0x168, CSR Addr 0x540b4, Direction=N/A */
	uint8_t reserved169;		/* Byte offset 0x169, CSR Addr 0x540b4, Direction=N/A */
	uint8_t reserved16a;		/* Byte offset 0x16a, CSR Addr 0x540b5, Direction=N/A */
	uint8_t reserved16b;		/* Byte offset 0x16b, CSR Addr 0x540b5, Direction=N/A */
	uint8_t reserved16c;		/* Byte offset 0x16c, CSR Addr 0x540b6, Direction=N/A */
	uint8_t reserved16d;		/* Byte offset 0x16d, CSR Addr 0x540b6, Direction=N/A */
	uint8_t reserved16e;		/* Byte offset 0x16e, CSR Addr 0x540b7, Direction=N/A */
	uint8_t reserved16f;		/* Byte offset 0x16f, CSR Addr 0x540b7, Direction=N/A */
	uint8_t reserved170;		/* Byte offset 0x170, CSR Addr 0x540b8, Direction=N/A */
	uint8_t reserved171;		/* Byte offset 0x171, CSR Addr 0x540b8, Direction=N/A */
	uint8_t reserved172;		/* Byte offset 0x172, CSR Addr 0x540b9, Direction=N/A */
	uint8_t reserved173;		/* Byte offset 0x173, CSR Addr 0x540b9, Direction=N/A */
	uint8_t reserved174;		/* Byte offset 0x174, CSR Addr 0x540ba, Direction=N/A */
	uint8_t reserved175;		/* Byte offset 0x175, CSR Addr 0x540ba, Direction=N/A */
	uint8_t reserved176;		/* Byte offset 0x176, CSR Addr 0x540bb, Direction=N/A */
	uint8_t reserved177;		/* Byte offset 0x177, CSR Addr 0x540bb, Direction=N/A */
	uint8_t reserved178;		/* Byte offset 0x178, CSR Addr 0x540bc, Direction=N/A */
	uint8_t reserved179;		/* Byte offset 0x179, CSR Addr 0x540bc, Direction=N/A */
	uint8_t reserved17a;		/* Byte offset 0x17a, CSR Addr 0x540bd, Direction=N/A */
	uint8_t reserved17b;		/* Byte offset 0x17b, CSR Addr 0x540bd, Direction=N/A */
	uint8_t reserved17c;		/* Byte offset 0x17c, CSR Addr 0x540be, Direction=N/A */
	uint8_t reserved17d;		/* Byte offset 0x17d, CSR Addr 0x540be, Direction=N/A */
	uint8_t reserved17e;		/* Byte offset 0x17e, CSR Addr 0x540bf, Direction=N/A */
	uint8_t reserved17f;		/* Byte offset 0x17f, CSR Addr 0x540bf, Direction=N/A */
	uint8_t reserved180;		/* Byte offset 0x180, CSR Addr 0x540c0, Direction=N/A */
	uint8_t reserved181;		/* Byte offset 0x181, CSR Addr 0x540c0, Direction=N/A */
	uint8_t reserved182;		/* Byte offset 0x182, CSR Addr 0x540c1, Direction=N/A */
	uint8_t reserved183;		/* Byte offset 0x183, CSR Addr 0x540c1, Direction=N/A */
	uint8_t reserved184;		/* Byte offset 0x184, CSR Addr 0x540c2, Direction=N/A */
	uint8_t reserved185;		/* Byte offset 0x185, CSR Addr 0x540c2, Direction=N/A */
	uint8_t reserved186;		/* Byte offset 0x186, CSR Addr 0x540c3, Direction=N/A */
	uint8_t reserved187;		/* Byte offset 0x187, CSR Addr 0x540c3, Direction=N/A */
	uint8_t reserved188;		/* Byte offset 0x188, CSR Addr 0x540c4, Direction=N/A */
	uint8_t reserved189;		/* Byte offset 0x189, CSR Addr 0x540c4, Direction=N/A */
	uint8_t reserved18a;		/* Byte offset 0x18a, CSR Addr 0x540c5, Direction=N/A */
	uint8_t reserved18b;		/* Byte offset 0x18b, CSR Addr 0x540c5, Direction=N/A */
	uint8_t reserved18c;		/* Byte offset 0x18c, CSR Addr 0x540c6, Direction=N/A */
	uint8_t reserved18d;		/* Byte offset 0x18d, CSR Addr 0x540c6, Direction=N/A */
	uint8_t reserved18e;		/* Byte offset 0x18e, CSR Addr 0x540c7, Direction=N/A */
	uint8_t reserved18f;		/* Byte offset 0x18f, CSR Addr 0x540c7, Direction=N/A */
	uint8_t reserved190;		/* Byte offset 0x190, CSR Addr 0x540c8, Direction=N/A */
	uint8_t reserved191;		/* Byte offset 0x191, CSR Addr 0x540c8, Direction=N/A */
	uint8_t reserved192;		/* Byte offset 0x192, CSR Addr 0x540c9, Direction=N/A */
	uint8_t reserved193;		/* Byte offset 0x193, CSR Addr 0x540c9, Direction=N/A */
	uint8_t reserved194;		/* Byte offset 0x194, CSR Addr 0x540ca, Direction=N/A */
	uint8_t reserved195;		/* Byte offset 0x195, CSR Addr 0x540ca, Direction=N/A */
	uint8_t reserved196;		/* Byte offset 0x196, CSR Addr 0x540cb, Direction=N/A */
	uint8_t reserved197;		/* Byte offset 0x197, CSR Addr 0x540cb, Direction=N/A */
	uint8_t reserved198;		/* Byte offset 0x198, CSR Addr 0x540cc, Direction=N/A */
	uint8_t reserved199;		/* Byte offset 0x199, CSR Addr 0x540cc, Direction=N/A */
	uint8_t reserved19a;		/* Byte offset 0x19a, CSR Addr 0x540cd, Direction=N/A */
	uint8_t reserved19b;		/* Byte offset 0x19b, CSR Addr 0x540cd, Direction=N/A */
	uint8_t reserved19c;		/* Byte offset 0x19c, CSR Addr 0x540ce, Direction=N/A */
	uint8_t reserved19d;		/* Byte offset 0x19d, CSR Addr 0x540ce, Direction=N/A */
	uint8_t reserved19e;		/* Byte offset 0x19e, CSR Addr 0x540cf, Direction=N/A */
	uint8_t reserved19f;		/* Byte offset 0x19f, CSR Addr 0x540cf, Direction=N/A */
	uint8_t reserved1a0;		/* Byte offset 0x1a0, CSR Addr 0x540d0, Direction=N/A */
	uint8_t reserved1a1;		/* Byte offset 0x1a1, CSR Addr 0x540d0, Direction=N/A */
	uint8_t reserved1a2;		/* Byte offset 0x1a2, CSR Addr 0x540d1, Direction=N/A */
	uint8_t reserved1a3;		/* Byte offset 0x1a3, CSR Addr 0x540d1, Direction=N/A */
	uint8_t reserved1a4;		/* Byte offset 0x1a4, CSR Addr 0x540d2, Direction=N/A */
	uint8_t reserved1a5;		/* Byte offset 0x1a5, CSR Addr 0x540d2, Direction=N/A */
	uint8_t reserved1a6;		/* Byte offset 0x1a6, CSR Addr 0x540d3, Direction=N/A */
	uint8_t reserved1a7;		/* Byte offset 0x1a7, CSR Addr 0x540d3, Direction=N/A */
	uint8_t reserved1a8;		/* Byte offset 0x1a8, CSR Addr 0x540d4, Direction=N/A */
	uint8_t reserved1a9;		/* Byte offset 0x1a9, CSR Addr 0x540d4, Direction=N/A */
	uint8_t reserved1aa;		/* Byte offset 0x1aa, CSR Addr 0x540d5, Direction=N/A */
	uint8_t reserved1ab;		/* Byte offset 0x1ab, CSR Addr 0x540d5, Direction=N/A */
	uint8_t reserved1ac;		/* Byte offset 0x1ac, CSR Addr 0x540d6, Direction=N/A */
	uint8_t reserved1ad;		/* Byte offset 0x1ad, CSR Addr 0x540d6, Direction=N/A */
	uint8_t reserved1ae;		/* Byte offset 0x1ae, CSR Addr 0x540d7, Direction=N/A */
	uint8_t reserved1af;		/* Byte offset 0x1af, CSR Addr 0x540d7, Direction=N/A */
	uint8_t reserved1b0;		/* Byte offset 0x1b0, CSR Addr 0x540d8, Direction=N/A */
	uint8_t reserved1b1;		/* Byte offset 0x1b1, CSR Addr 0x540d8, Direction=N/A */
	uint8_t reserved1b2;		/* Byte offset 0x1b2, CSR Addr 0x540d9, Direction=N/A */
	uint8_t reserved1b3;		/* Byte offset 0x1b3, CSR Addr 0x540d9, Direction=N/A */
	uint8_t reserved1b4;		/* Byte offset 0x1b4, CSR Addr 0x540da, Direction=N/A */
	uint8_t reserved1b5;		/* Byte offset 0x1b5, CSR Addr 0x540da, Direction=N/A */
	uint8_t reserved1b6;		/* Byte offset 0x1b6, CSR Addr 0x540db, Direction=N/A */
	uint8_t reserved1b7;		/* Byte offset 0x1b7, CSR Addr 0x540db, Direction=N/A */
	uint8_t reserved1b8;		/* Byte offset 0x1b8, CSR Addr 0x540dc, Direction=N/A */
	uint8_t reserved1b9;		/* Byte offset 0x1b9, CSR Addr 0x540dc, Direction=N/A */
	uint8_t reserved1ba;		/* Byte offset 0x1ba, CSR Addr 0x540dd, Direction=N/A */
	uint8_t reserved1bb;		/* Byte offset 0x1bb, CSR Addr 0x540dd, Direction=N/A */
	uint8_t reserved1bc;		/* Byte offset 0x1bc, CSR Addr 0x540de, Direction=N/A */
	uint8_t reserved1bd;		/* Byte offset 0x1bd, CSR Addr 0x540de, Direction=N/A */
	uint8_t reserved1be;		/* Byte offset 0x1be, CSR Addr 0x540df, Direction=N/A */
	uint8_t reserved1bf;		/* Byte offset 0x1bf, CSR Addr 0x540df, Direction=N/A */
	uint8_t reserved1c0;		/* Byte offset 0x1c0, CSR Addr 0x540e0, Direction=N/A */
	uint8_t reserved1c1;		/* Byte offset 0x1c1, CSR Addr 0x540e0, Direction=N/A */
	uint8_t reserved1c2;		/* Byte offset 0x1c2, CSR Addr 0x540e1, Direction=N/A */
	uint8_t reserved1c3;		/* Byte offset 0x1c3, CSR Addr 0x540e1, Direction=N/A */
	uint8_t reserved1c4;		/* Byte offset 0x1c4, CSR Addr 0x540e2, Direction=N/A */
	uint8_t reserved1c5;		/* Byte offset 0x1c5, CSR Addr 0x540e2, Direction=N/A */
	uint8_t reserved1c6;		/* Byte offset 0x1c6, CSR Addr 0x540e3, Direction=N/A */
	uint8_t reserved1c7;		/* Byte offset 0x1c7, CSR Addr 0x540e3, Direction=N/A */
	uint8_t reserved1c8;		/* Byte offset 0x1c8, CSR Addr 0x540e4, Direction=N/A */
	uint8_t reserved1c9;		/* Byte offset 0x1c9, CSR Addr 0x540e4, Direction=N/A */
	uint8_t reserved1ca;		/* Byte offset 0x1ca, CSR Addr 0x540e5, Direction=N/A */
	uint8_t reserved1cb;		/* Byte offset 0x1cb, CSR Addr 0x540e5, Direction=N/A */
	uint8_t reserved1cc;		/* Byte offset 0x1cc, CSR Addr 0x540e6, Direction=N/A */
	uint8_t reserved1cd;		/* Byte offset 0x1cd, CSR Addr 0x540e6, Direction=N/A */
	uint8_t reserved1ce;		/* Byte offset 0x1ce, CSR Addr 0x540e7, Direction=N/A */
	uint8_t reserved1cf;		/* Byte offset 0x1cf, CSR Addr 0x540e7, Direction=N/A */
	uint8_t reserved1d0;		/* Byte offset 0x1d0, CSR Addr 0x540e8, Direction=N/A */
	uint8_t reserved1d1;		/* Byte offset 0x1d1, CSR Addr 0x540e8, Direction=N/A */
	uint8_t reserved1d2;		/* Byte offset 0x1d2, CSR Addr 0x540e9, Direction=N/A */
	uint8_t reserved1d3;		/* Byte offset 0x1d3, CSR Addr 0x540e9, Direction=N/A */
	uint8_t reserved1d4;		/* Byte offset 0x1d4, CSR Addr 0x540ea, Direction=N/A */
	uint8_t reserved1d5;		/* Byte offset 0x1d5, CSR Addr 0x540ea, Direction=N/A */
	uint8_t reserved1d6;		/* Byte offset 0x1d6, CSR Addr 0x540eb, Direction=N/A */
	uint8_t reserved1d7;		/* Byte offset 0x1d7, CSR Addr 0x540eb, Direction=N/A */
	uint8_t reserved1d8;		/* Byte offset 0x1d8, CSR Addr 0x540ec, Direction=N/A */
	uint8_t reserved1d9;		/* Byte offset 0x1d9, CSR Addr 0x540ec, Direction=N/A */
	uint8_t reserved1da;		/* Byte offset 0x1da, CSR Addr 0x540ed, Direction=N/A */
	uint8_t reserved1db;		/* Byte offset 0x1db, CSR Addr 0x540ed, Direction=N/A */
	uint8_t reserved1dc;		/* Byte offset 0x1dc, CSR Addr 0x540ee, Direction=N/A */
	uint8_t reserved1dd;		/* Byte offset 0x1dd, CSR Addr 0x540ee, Direction=N/A */
	uint8_t reserved1de;		/* Byte offset 0x1de, CSR Addr 0x540ef, Direction=N/A */
	uint8_t reserved1df;		/* Byte offset 0x1df, CSR Addr 0x540ef, Direction=N/A */
	uint8_t reserved1e0;		/* Byte offset 0x1e0, CSR Addr 0x540f0, Direction=N/A */
	uint8_t reserved1e1;		/* Byte offset 0x1e1, CSR Addr 0x540f0, Direction=N/A */
	uint8_t reserved1e2;		/* Byte offset 0x1e2, CSR Addr 0x540f1, Direction=N/A */
	uint8_t reserved1e3;		/* Byte offset 0x1e3, CSR Addr 0x540f1, Direction=N/A */
	uint8_t reserved1e4;		/* Byte offset 0x1e4, CSR Addr 0x540f2, Direction=N/A */
	uint8_t reserved1e5;		/* Byte offset 0x1e5, CSR Addr 0x540f2, Direction=N/A */
	uint8_t reserved1e6;		/* Byte offset 0x1e6, CSR Addr 0x540f3, Direction=N/A */
	uint8_t reserved1e7;		/* Byte offset 0x1e7, CSR Addr 0x540f3, Direction=N/A */
	uint8_t reserved1e8;		/* Byte offset 0x1e8, CSR Addr 0x540f4, Direction=N/A */
	uint8_t reserved1e9;		/* Byte offset 0x1e9, CSR Addr 0x540f4, Direction=N/A */
	uint8_t reserved1ea;		/* Byte offset 0x1ea, CSR Addr 0x540f5, Direction=N/A */
	uint8_t reserved1eb;		/* Byte offset 0x1eb, CSR Addr 0x540f5, Direction=N/A */
	uint8_t reserved1ec;		/* Byte offset 0x1ec, CSR Addr 0x540f6, Direction=N/A */
	uint8_t reserved1ed;		/* Byte offset 0x1ed, CSR Addr 0x540f6, Direction=N/A */
	uint8_t reserved1ee;		/* Byte offset 0x1ee, CSR Addr 0x540f7, Direction=N/A */
	uint8_t reserved1ef;		/* Byte offset 0x1ef, CSR Addr 0x540f7, Direction=N/A */
	uint8_t reserved1f0;		/* Byte offset 0x1f0, CSR Addr 0x540f8, Direction=N/A */
	uint8_t reserved1f1;		/* Byte offset 0x1f1, CSR Addr 0x540f8, Direction=N/A */
	uint8_t reserved1f2;		/* Byte offset 0x1f2, CSR Addr 0x540f9, Direction=N/A */
	uint8_t reserved1f3;		/* Byte offset 0x1f3, CSR Addr 0x540f9, Direction=N/A */
	uint8_t reserved1f4;		/* Byte offset 0x1f4, CSR Addr 0x540fa, Direction=N/A */
	uint8_t reserved1f5;		/* Byte offset 0x1f5, CSR Addr 0x540fa, Direction=N/A */
	uint8_t reserved1f6;		/* Byte offset 0x1f6, CSR Addr 0x540fb, Direction=N/A */
	uint8_t reserved1f7;		/* Byte offset 0x1f7, CSR Addr 0x540fb, Direction=N/A */
	uint8_t reserved1f8;		/* Byte offset 0x1f8, CSR Addr 0x540fc, Direction=N/A */
	uint8_t reserved1f9;		/* Byte offset 0x1f9, CSR Addr 0x540fc, Direction=N/A */
	uint8_t reserved1fa;		/* Byte offset 0x1fa, CSR Addr 0x540fd, Direction=N/A */
	uint8_t reserved1fb;		/* Byte offset 0x1fb, CSR Addr 0x540fd, Direction=N/A */
	uint8_t reserved1fc;		/* Byte offset 0x1fc, CSR Addr 0x540fe, Direction=N/A */
	uint8_t reserved1fd;		/* Byte offset 0x1fd, CSR Addr 0x540fe, Direction=N/A */
	uint8_t reserved1fe;		/* Byte offset 0x1fe, CSR Addr 0x540ff, Direction=N/A */
	uint8_t reserved1ff;		/* Byte offset 0x1ff, CSR Addr 0x540ff, Direction=N/A */
	uint8_t reserved200;		/* Byte offset 0x200, CSR Addr 0x54100, Direction=N/A */
	uint8_t reserved201;		/* Byte offset 0x201, CSR Addr 0x54100, Direction=N/A */
	uint8_t reserved202;		/* Byte offset 0x202, CSR Addr 0x54101, Direction=N/A */
	uint8_t reserved203;		/* Byte offset 0x203, CSR Addr 0x54101, Direction=N/A */
	uint8_t reserved204;		/* Byte offset 0x204, CSR Addr 0x54102, Direction=N/A */
	uint8_t reserved205;		/* Byte offset 0x205, CSR Addr 0x54102, Direction=N/A */
	uint8_t reserved206;		/* Byte offset 0x206, CSR Addr 0x54103, Direction=N/A */
	uint8_t reserved207;		/* Byte offset 0x207, CSR Addr 0x54103, Direction=N/A */
	uint8_t reserved208;		/* Byte offset 0x208, CSR Addr 0x54104, Direction=N/A */
	uint8_t reserved209;		/* Byte offset 0x209, CSR Addr 0x54104, Direction=N/A */
	uint8_t reserved20a;		/* Byte offset 0x20a, CSR Addr 0x54105, Direction=N/A */
	uint8_t reserved20b;		/* Byte offset 0x20b, CSR Addr 0x54105, Direction=N/A */
	uint8_t reserved20c;		/* Byte offset 0x20c, CSR Addr 0x54106, Direction=N/A */
	uint8_t reserved20d;		/* Byte offset 0x20d, CSR Addr 0x54106, Direction=N/A */
	uint8_t reserved20e;		/* Byte offset 0x20e, CSR Addr 0x54107, Direction=N/A */
	uint8_t reserved20f;		/* Byte offset 0x20f, CSR Addr 0x54107, Direction=N/A */
	uint8_t reserved210;		/* Byte offset 0x210, CSR Addr 0x54108, Direction=N/A */
	uint8_t reserved211;		/* Byte offset 0x211, CSR Addr 0x54108, Direction=N/A */
	uint8_t reserved212;		/* Byte offset 0x212, CSR Addr 0x54109, Direction=N/A */
	uint8_t reserved213;		/* Byte offset 0x213, CSR Addr 0x54109, Direction=N/A */
	uint8_t reserved214;		/* Byte offset 0x214, CSR Addr 0x5410a, Direction=N/A */
	uint8_t reserved215;		/* Byte offset 0x215, CSR Addr 0x5410a, Direction=N/A */
	uint8_t reserved216;		/* Byte offset 0x216, CSR Addr 0x5410b, Direction=N/A */
	uint8_t reserved217;		/* Byte offset 0x217, CSR Addr 0x5410b, Direction=N/A */
	uint8_t reserved218;		/* Byte offset 0x218, CSR Addr 0x5410c, Direction=N/A */
	uint8_t reserved219;		/* Byte offset 0x219, CSR Addr 0x5410c, Direction=N/A */
	uint8_t reserved21a;		/* Byte offset 0x21a, CSR Addr 0x5410d, Direction=N/A */
	uint8_t reserved21b;		/* Byte offset 0x21b, CSR Addr 0x5410d, Direction=N/A */
	uint8_t reserved21c;		/* Byte offset 0x21c, CSR Addr 0x5410e, Direction=N/A */
	uint8_t reserved21d;		/* Byte offset 0x21d, CSR Addr 0x5410e, Direction=N/A */
	uint8_t reserved21e;		/* Byte offset 0x21e, CSR Addr 0x5410f, Direction=N/A */
	uint8_t reserved21f;		/* Byte offset 0x21f, CSR Addr 0x5410f, Direction=N/A */
	uint8_t reserved220;		/* Byte offset 0x220, CSR Addr 0x54110, Direction=N/A */
	uint8_t reserved221;		/* Byte offset 0x221, CSR Addr 0x54110, Direction=N/A */
	uint8_t reserved222;		/* Byte offset 0x222, CSR Addr 0x54111, Direction=N/A */
	uint8_t reserved223;		/* Byte offset 0x223, CSR Addr 0x54111, Direction=N/A */
	uint8_t reserved224;		/* Byte offset 0x224, CSR Addr 0x54112, Direction=N/A */
	uint8_t reserved225;		/* Byte offset 0x225, CSR Addr 0x54112, Direction=N/A */
	uint8_t reserved226;		/* Byte offset 0x226, CSR Addr 0x54113, Direction=N/A */
	uint8_t reserved227;		/* Byte offset 0x227, CSR Addr 0x54113, Direction=N/A */
	uint8_t reserved228;		/* Byte offset 0x228, CSR Addr 0x54114, Direction=N/A */
	uint8_t reserved229;		/* Byte offset 0x229, CSR Addr 0x54114, Direction=N/A */
	uint8_t reserved22a;		/* Byte offset 0x22a, CSR Addr 0x54115, Direction=N/A */
	uint8_t reserved22b;		/* Byte offset 0x22b, CSR Addr 0x54115, Direction=N/A */
	uint8_t reserved22c;		/* Byte offset 0x22c, CSR Addr 0x54116, Direction=N/A */
	uint8_t reserved22d;		/* Byte offset 0x22d, CSR Addr 0x54116, Direction=N/A */
	uint8_t reserved22e;		/* Byte offset 0x22e, CSR Addr 0x54117, Direction=N/A */
	uint8_t reserved22f;		/* Byte offset 0x22f, CSR Addr 0x54117, Direction=N/A */
	uint8_t reserved230;		/* Byte offset 0x230, CSR Addr 0x54118, Direction=N/A */
	uint8_t reserved231;		/* Byte offset 0x231, CSR Addr 0x54118, Direction=N/A */
	uint8_t reserved232;		/* Byte offset 0x232, CSR Addr 0x54119, Direction=N/A */
	uint8_t reserved233;		/* Byte offset 0x233, CSR Addr 0x54119, Direction=N/A */
	uint8_t reserved234;		/* Byte offset 0x234, CSR Addr 0x5411a, Direction=N/A */
	uint8_t reserved235;		/* Byte offset 0x235, CSR Addr 0x5411a, Direction=N/A */
	uint8_t reserved236;		/* Byte offset 0x236, CSR Addr 0x5411b, Direction=N/A */
	uint8_t reserved237;		/* Byte offset 0x237, CSR Addr 0x5411b, Direction=N/A */
	uint8_t reserved238;		/* Byte offset 0x238, CSR Addr 0x5411c, Direction=N/A */
	uint8_t reserved239;		/* Byte offset 0x239, CSR Addr 0x5411c, Direction=N/A */
	uint8_t reserved23a;		/* Byte offset 0x23a, CSR Addr 0x5411d, Direction=N/A */
	uint8_t reserved23b;		/* Byte offset 0x23b, CSR Addr 0x5411d, Direction=N/A */
	uint8_t reserved23c;		/* Byte offset 0x23c, CSR Addr 0x5411e, Direction=N/A */
	uint8_t reserved23d;		/* Byte offset 0x23d, CSR Addr 0x5411e, Direction=N/A */
	uint8_t reserved23e;		/* Byte offset 0x23e, CSR Addr 0x5411f, Direction=N/A */
	uint8_t reserved23f;		/* Byte offset 0x23f, CSR Addr 0x5411f, Direction=N/A */
	uint8_t reserved240;		/* Byte offset 0x240, CSR Addr 0x54120, Direction=N/A */
	uint8_t reserved241;		/* Byte offset 0x241, CSR Addr 0x54120, Direction=N/A */
	uint8_t reserved242;		/* Byte offset 0x242, CSR Addr 0x54121, Direction=N/A */
	uint8_t reserved243;		/* Byte offset 0x243, CSR Addr 0x54121, Direction=N/A */
	uint8_t reserved244;		/* Byte offset 0x244, CSR Addr 0x54122, Direction=N/A */
	uint8_t reserved245;		/* Byte offset 0x245, CSR Addr 0x54122, Direction=N/A */
	uint8_t reserved246;		/* Byte offset 0x246, CSR Addr 0x54123, Direction=N/A */
	uint8_t reserved247;		/* Byte offset 0x247, CSR Addr 0x54123, Direction=N/A */
	uint8_t reserved248;		/* Byte offset 0x248, CSR Addr 0x54124, Direction=N/A */
	uint8_t reserved249;		/* Byte offset 0x249, CSR Addr 0x54124, Direction=N/A */
	uint8_t reserved24a;		/* Byte offset 0x24a, CSR Addr 0x54125, Direction=N/A */
	uint8_t reserved24b;		/* Byte offset 0x24b, CSR Addr 0x54125, Direction=N/A */
	uint8_t reserved24c;		/* Byte offset 0x24c, CSR Addr 0x54126, Direction=N/A */
	uint8_t reserved24d;		/* Byte offset 0x24d, CSR Addr 0x54126, Direction=N/A */
	uint8_t reserved24e;		/* Byte offset 0x24e, CSR Addr 0x54127, Direction=N/A */
	uint8_t reserved24f;		/* Byte offset 0x24f, CSR Addr 0x54127, Direction=N/A */
	uint8_t reserved250;		/* Byte offset 0x250, CSR Addr 0x54128, Direction=N/A */
	uint8_t reserved251;		/* Byte offset 0x251, CSR Addr 0x54128, Direction=N/A */
	uint8_t reserved252;		/* Byte offset 0x252, CSR Addr 0x54129, Direction=N/A */
	uint8_t reserved253;		/* Byte offset 0x253, CSR Addr 0x54129, Direction=N/A */
	uint8_t reserved254;		/* Byte offset 0x254, CSR Addr 0x5412a, Direction=N/A */
	uint8_t reserved255;		/* Byte offset 0x255, CSR Addr 0x5412a, Direction=N/A */
	uint8_t reserved256;		/* Byte offset 0x256, CSR Addr 0x5412b, Direction=N/A */
	uint8_t reserved257;		/* Byte offset 0x257, CSR Addr 0x5412b, Direction=N/A */
	uint8_t reserved258;		/* Byte offset 0x258, CSR Addr 0x5412c, Direction=N/A */
	uint8_t reserved259;		/* Byte offset 0x259, CSR Addr 0x5412c, Direction=N/A */
	uint8_t reserved25a;		/* Byte offset 0x25a, CSR Addr 0x5412d, Direction=N/A */
	uint8_t reserved25b;		/* Byte offset 0x25b, CSR Addr 0x5412d, Direction=N/A */
	uint8_t reserved25c;		/* Byte offset 0x25c, CSR Addr 0x5412e, Direction=N/A */
	uint8_t reserved25d;		/* Byte offset 0x25d, CSR Addr 0x5412e, Direction=N/A */
	uint8_t reserved25e;		/* Byte offset 0x25e, CSR Addr 0x5412f, Direction=N/A */
	uint8_t reserved25f;		/* Byte offset 0x25f, CSR Addr 0x5412f, Direction=N/A */
	uint8_t reserved260;		/* Byte offset 0x260, CSR Addr 0x54130, Direction=N/A */
	uint8_t reserved261;		/* Byte offset 0x261, CSR Addr 0x54130, Direction=N/A */
	uint8_t reserved262;		/* Byte offset 0x262, CSR Addr 0x54131, Direction=N/A */
	uint8_t reserved263;		/* Byte offset 0x263, CSR Addr 0x54131, Direction=N/A */
	uint8_t reserved264;		/* Byte offset 0x264, CSR Addr 0x54132, Direction=N/A */
	uint8_t reserved265;		/* Byte offset 0x265, CSR Addr 0x54132, Direction=N/A */
	uint8_t reserved266;		/* Byte offset 0x266, CSR Addr 0x54133, Direction=N/A */
	uint8_t reserved267;		/* Byte offset 0x267, CSR Addr 0x54133, Direction=N/A */
	uint8_t reserved268;		/* Byte offset 0x268, CSR Addr 0x54134, Direction=N/A */
	uint8_t reserved269;		/* Byte offset 0x269, CSR Addr 0x54134, Direction=N/A */
	uint8_t reserved26a;		/* Byte offset 0x26a, CSR Addr 0x54135, Direction=N/A */
	uint8_t reserved26b;		/* Byte offset 0x26b, CSR Addr 0x54135, Direction=N/A */
	uint8_t reserved26c;		/* Byte offset 0x26c, CSR Addr 0x54136, Direction=N/A */
	uint8_t reserved26d;		/* Byte offset 0x26d, CSR Addr 0x54136, Direction=N/A */
	uint8_t reserved26e;		/* Byte offset 0x26e, CSR Addr 0x54137, Direction=N/A */
	uint8_t reserved26f;		/* Byte offset 0x26f, CSR Addr 0x54137, Direction=N/A */
	uint8_t reserved270;		/* Byte offset 0x270, CSR Addr 0x54138, Direction=N/A */
	uint8_t reserved271;		/* Byte offset 0x271, CSR Addr 0x54138, Direction=N/A */
	uint8_t reserved272;		/* Byte offset 0x272, CSR Addr 0x54139, Direction=N/A */
	uint8_t reserved273;		/* Byte offset 0x273, CSR Addr 0x54139, Direction=N/A */
	uint8_t reserved274;		/* Byte offset 0x274, CSR Addr 0x5413a, Direction=N/A */
	uint8_t reserved275;		/* Byte offset 0x275, CSR Addr 0x5413a, Direction=N/A */
	uint8_t reserved276;		/* Byte offset 0x276, CSR Addr 0x5413b, Direction=N/A */
	uint8_t reserved277;		/* Byte offset 0x277, CSR Addr 0x5413b, Direction=N/A */
	uint8_t reserved278;		/* Byte offset 0x278, CSR Addr 0x5413c, Direction=N/A */
	uint8_t reserved279;		/* Byte offset 0x279, CSR Addr 0x5413c, Direction=N/A */
	uint8_t reserved27a;		/* Byte offset 0x27a, CSR Addr 0x5413d, Direction=N/A */
	uint8_t reserved27b;		/* Byte offset 0x27b, CSR Addr 0x5413d, Direction=N/A */
	uint8_t reserved27c;		/* Byte offset 0x27c, CSR Addr 0x5413e, Direction=N/A */
	uint8_t reserved27d;		/* Byte offset 0x27d, CSR Addr 0x5413e, Direction=N/A */
	uint8_t reserved27e;		/* Byte offset 0x27e, CSR Addr 0x5413f, Direction=N/A */
	uint8_t reserved27f;		/* Byte offset 0x27f, CSR Addr 0x5413f, Direction=N/A */
	uint8_t reserved280;		/* Byte offset 0x280, CSR Addr 0x54140, Direction=N/A */
	uint8_t reserved281;		/* Byte offset 0x281, CSR Addr 0x54140, Direction=N/A */
	uint8_t reserved282;		/* Byte offset 0x282, CSR Addr 0x54141, Direction=N/A */
	uint8_t reserved283;		/* Byte offset 0x283, CSR Addr 0x54141, Direction=N/A */
	uint8_t reserved284;		/* Byte offset 0x284, CSR Addr 0x54142, Direction=N/A */
	uint8_t reserved285;		/* Byte offset 0x285, CSR Addr 0x54142, Direction=N/A */
	uint8_t reserved286;		/* Byte offset 0x286, CSR Addr 0x54143, Direction=N/A */
	uint8_t reserved287;		/* Byte offset 0x287, CSR Addr 0x54143, Direction=N/A */
	uint8_t reserved288;		/* Byte offset 0x288, CSR Addr 0x54144, Direction=N/A */
	uint8_t reserved289;		/* Byte offset 0x289, CSR Addr 0x54144, Direction=N/A */
	uint8_t reserved28a;		/* Byte offset 0x28a, CSR Addr 0x54145, Direction=N/A */
	uint8_t reserved28b;		/* Byte offset 0x28b, CSR Addr 0x54145, Direction=N/A */
	uint8_t reserved28c;		/* Byte offset 0x28c, CSR Addr 0x54146, Direction=N/A */
	uint8_t reserved28d;		/* Byte offset 0x28d, CSR Addr 0x54146, Direction=N/A */
	uint8_t reserved28e;		/* Byte offset 0x28e, CSR Addr 0x54147, Direction=N/A */
	uint8_t reserved28f;		/* Byte offset 0x28f, CSR Addr 0x54147, Direction=N/A */
	uint8_t reserved290;		/* Byte offset 0x290, CSR Addr 0x54148, Direction=N/A */
	uint8_t reserved291;		/* Byte offset 0x291, CSR Addr 0x54148, Direction=N/A */
	uint8_t reserved292;		/* Byte offset 0x292, CSR Addr 0x54149, Direction=N/A */
	uint8_t reserved293;		/* Byte offset 0x293, CSR Addr 0x54149, Direction=N/A */
	uint8_t reserved294;		/* Byte offset 0x294, CSR Addr 0x5414a, Direction=N/A */
	uint8_t reserved295;		/* Byte offset 0x295, CSR Addr 0x5414a, Direction=N/A */
	uint8_t reserved296;		/* Byte offset 0x296, CSR Addr 0x5414b, Direction=N/A */
	uint8_t reserved297;		/* Byte offset 0x297, CSR Addr 0x5414b, Direction=N/A */
	uint8_t reserved298;		/* Byte offset 0x298, CSR Addr 0x5414c, Direction=N/A */
	uint8_t reserved299;		/* Byte offset 0x299, CSR Addr 0x5414c, Direction=N/A */
	uint8_t reserved29a;		/* Byte offset 0x29a, CSR Addr 0x5414d, Direction=N/A */
	uint8_t reserved29b;		/* Byte offset 0x29b, CSR Addr 0x5414d, Direction=N/A */
	uint8_t reserved29c;		/* Byte offset 0x29c, CSR Addr 0x5414e, Direction=N/A */
	uint8_t reserved29d;		/* Byte offset 0x29d, CSR Addr 0x5414e, Direction=N/A */
	uint8_t reserved29e;		/* Byte offset 0x29e, CSR Addr 0x5414f, Direction=N/A */
	uint8_t reserved29f;		/* Byte offset 0x29f, CSR Addr 0x5414f, Direction=N/A */
	uint8_t reserved2a0;		/* Byte offset 0x2a0, CSR Addr 0x54150, Direction=N/A */
	uint8_t reserved2a1;		/* Byte offset 0x2a1, CSR Addr 0x54150, Direction=N/A */
	uint8_t reserved2a2;		/* Byte offset 0x2a2, CSR Addr 0x54151, Direction=N/A */
	uint8_t reserved2a3;		/* Byte offset 0x2a3, CSR Addr 0x54151, Direction=N/A */
	uint8_t reserved2a4;		/* Byte offset 0x2a4, CSR Addr 0x54152, Direction=N/A */
	uint8_t reserved2a5;		/* Byte offset 0x2a5, CSR Addr 0x54152, Direction=N/A */
	uint8_t reserved2a6;		/* Byte offset 0x2a6, CSR Addr 0x54153, Direction=N/A */
	uint8_t reserved2a7;		/* Byte offset 0x2a7, CSR Addr 0x54153, Direction=N/A */
	uint8_t reserved2a8;		/* Byte offset 0x2a8, CSR Addr 0x54154, Direction=N/A */
	uint8_t reserved2a9;		/* Byte offset 0x2a9, CSR Addr 0x54154, Direction=N/A */
	uint8_t reserved2aa;		/* Byte offset 0x2aa, CSR Addr 0x54155, Direction=N/A */
	uint8_t reserved2ab;		/* Byte offset 0x2ab, CSR Addr 0x54155, Direction=N/A */
	uint8_t reserved2ac;		/* Byte offset 0x2ac, CSR Addr 0x54156, Direction=N/A */
	uint8_t reserved2ad;		/* Byte offset 0x2ad, CSR Addr 0x54156, Direction=N/A */
	uint8_t reserved2ae;		/* Byte offset 0x2ae, CSR Addr 0x54157, Direction=N/A */
	uint8_t reserved2af;		/* Byte offset 0x2af, CSR Addr 0x54157, Direction=N/A */
	uint8_t reserved2b0;		/* Byte offset 0x2b0, CSR Addr 0x54158, Direction=N/A */
	uint8_t reserved2b1;		/* Byte offset 0x2b1, CSR Addr 0x54158, Direction=N/A */
	uint8_t reserved2b2;		/* Byte offset 0x2b2, CSR Addr 0x54159, Direction=N/A */
	uint8_t reserved2b3;		/* Byte offset 0x2b3, CSR Addr 0x54159, Direction=N/A */
	uint8_t reserved2b4;		/* Byte offset 0x2b4, CSR Addr 0x5415a, Direction=N/A */
	uint8_t reserved2b5;		/* Byte offset 0x2b5, CSR Addr 0x5415a, Direction=N/A */
	uint8_t reserved2b6;		/* Byte offset 0x2b6, CSR Addr 0x5415b, Direction=N/A */
	uint8_t reserved2b7;		/* Byte offset 0x2b7, CSR Addr 0x5415b, Direction=N/A */
	uint8_t reserved2b8;		/* Byte offset 0x2b8, CSR Addr 0x5415c, Direction=N/A */
	uint8_t reserved2b9;		/* Byte offset 0x2b9, CSR Addr 0x5415c, Direction=N/A */
	uint8_t reserved2ba;		/* Byte offset 0x2ba, CSR Addr 0x5415d, Direction=N/A */
	uint8_t reserved2bb;		/* Byte offset 0x2bb, CSR Addr 0x5415d, Direction=N/A */
	uint8_t reserved2bc;		/* Byte offset 0x2bc, CSR Addr 0x5415e, Direction=N/A */
	uint8_t reserved2bd;		/* Byte offset 0x2bd, CSR Addr 0x5415e, Direction=N/A */
	uint8_t reserved2be;		/* Byte offset 0x2be, CSR Addr 0x5415f, Direction=N/A */
	uint8_t reserved2bf;		/* Byte offset 0x2bf, CSR Addr 0x5415f, Direction=N/A */
	uint8_t reserved2c0;		/* Byte offset 0x2c0, CSR Addr 0x54160, Direction=N/A */
	uint8_t reserved2c1;		/* Byte offset 0x2c1, CSR Addr 0x54160, Direction=N/A */
	uint8_t reserved2c2;		/* Byte offset 0x2c2, CSR Addr 0x54161, Direction=N/A */
	uint8_t reserved2c3;		/* Byte offset 0x2c3, CSR Addr 0x54161, Direction=N/A */
	uint8_t reserved2c4;		/* Byte offset 0x2c4, CSR Addr 0x54162, Direction=N/A */
	uint8_t reserved2c5;		/* Byte offset 0x2c5, CSR Addr 0x54162, Direction=N/A */
	uint8_t reserved2c6;		/* Byte offset 0x2c6, CSR Addr 0x54163, Direction=N/A */
	uint8_t reserved2c7;		/* Byte offset 0x2c7, CSR Addr 0x54163, Direction=N/A */
	uint8_t reserved2c8;		/* Byte offset 0x2c8, CSR Addr 0x54164, Direction=N/A */
	uint8_t reserved2c9;		/* Byte offset 0x2c9, CSR Addr 0x54164, Direction=N/A */
	uint8_t reserved2ca;		/* Byte offset 0x2ca, CSR Addr 0x54165, Direction=N/A */
	uint8_t reserved2cb;		/* Byte offset 0x2cb, CSR Addr 0x54165, Direction=N/A */
	uint8_t reserved2cc;		/* Byte offset 0x2cc, CSR Addr 0x54166, Direction=N/A */
	uint8_t reserved2cd;		/* Byte offset 0x2cd, CSR Addr 0x54166, Direction=N/A */
	uint8_t reserved2ce;		/* Byte offset 0x2ce, CSR Addr 0x54167, Direction=N/A */
	uint8_t reserved2cf;		/* Byte offset 0x2cf, CSR Addr 0x54167, Direction=N/A */
	uint8_t reserved2d0;		/* Byte offset 0x2d0, CSR Addr 0x54168, Direction=N/A */
	uint8_t reserved2d1;		/* Byte offset 0x2d1, CSR Addr 0x54168, Direction=N/A */
	uint8_t reserved2d2;		/* Byte offset 0x2d2, CSR Addr 0x54169, Direction=N/A */
	uint8_t reserved2d3;		/* Byte offset 0x2d3, CSR Addr 0x54169, Direction=N/A */
	uint8_t reserved2d4;		/* Byte offset 0x2d4, CSR Addr 0x5416a, Direction=N/A */
	uint8_t reserved2d5;		/* Byte offset 0x2d5, CSR Addr 0x5416a, Direction=N/A */
	uint8_t reserved2d6;		/* Byte offset 0x2d6, CSR Addr 0x5416b, Direction=N/A */
	uint8_t reserved2d7;		/* Byte offset 0x2d7, CSR Addr 0x5416b, Direction=N/A */
	uint8_t reserved2d8;		/* Byte offset 0x2d8, CSR Addr 0x5416c, Direction=N/A */
	uint8_t reserved2d9;		/* Byte offset 0x2d9, CSR Addr 0x5416c, Direction=N/A */
	uint8_t reserved2da;		/* Byte offset 0x2da, CSR Addr 0x5416d, Direction=N/A */
	uint8_t reserved2db;		/* Byte offset 0x2db, CSR Addr 0x5416d, Direction=N/A */
	uint8_t reserved2dc;		/* Byte offset 0x2dc, CSR Addr 0x5416e, Direction=N/A */
	uint8_t reserved2dd;		/* Byte offset 0x2dd, CSR Addr 0x5416e, Direction=N/A */
	uint8_t reserved2de;		/* Byte offset 0x2de, CSR Addr 0x5416f, Direction=N/A */
	uint8_t reserved2df;		/* Byte offset 0x2df, CSR Addr 0x5416f, Direction=N/A */
	uint8_t reserved2e0;		/* Byte offset 0x2e0, CSR Addr 0x54170, Direction=N/A */
	uint8_t reserved2e1;		/* Byte offset 0x2e1, CSR Addr 0x54170, Direction=N/A */
	uint8_t reserved2e2;		/* Byte offset 0x2e2, CSR Addr 0x54171, Direction=N/A */
	uint8_t reserved2e3;		/* Byte offset 0x2e3, CSR Addr 0x54171, Direction=N/A */
	uint8_t reserved2e4;		/* Byte offset 0x2e4, CSR Addr 0x54172, Direction=N/A */
	uint8_t reserved2e5;		/* Byte offset 0x2e5, CSR Addr 0x54172, Direction=N/A */
	uint8_t reserved2e6;		/* Byte offset 0x2e6, CSR Addr 0x54173, Direction=N/A */
	uint8_t reserved2e7;		/* Byte offset 0x2e7, CSR Addr 0x54173, Direction=N/A */
	uint8_t reserved2e8;		/* Byte offset 0x2e8, CSR Addr 0x54174, Direction=N/A */
	uint8_t reserved2e9;		/* Byte offset 0x2e9, CSR Addr 0x54174, Direction=N/A */
	uint8_t reserved2ea;		/* Byte offset 0x2ea, CSR Addr 0x54175, Direction=N/A */
	uint8_t reserved2eb;		/* Byte offset 0x2eb, CSR Addr 0x54175, Direction=N/A */
	uint8_t reserved2ec;		/* Byte offset 0x2ec, CSR Addr 0x54176, Direction=N/A */
	uint8_t reserved2ed;		/* Byte offset 0x2ed, CSR Addr 0x54176, Direction=N/A */
	uint8_t reserved2ee;		/* Byte offset 0x2ee, CSR Addr 0x54177, Direction=N/A */
	uint8_t reserved2ef;		/* Byte offset 0x2ef, CSR Addr 0x54177, Direction=N/A */
	uint8_t reserved2f0;		/* Byte offset 0x2f0, CSR Addr 0x54178, Direction=N/A */
	uint8_t reserved2f1;		/* Byte offset 0x2f1, CSR Addr 0x54178, Direction=N/A */
	uint8_t reserved2f2;		/* Byte offset 0x2f2, CSR Addr 0x54179, Direction=N/A */
	uint8_t reserved2f3;		/* Byte offset 0x2f3, CSR Addr 0x54179, Direction=N/A */
	uint8_t reserved2f4;		/* Byte offset 0x2f4, CSR Addr 0x5417a, Direction=N/A */
	uint8_t reserved2f5;		/* Byte offset 0x2f5, CSR Addr 0x5417a, Direction=N/A */
	uint8_t reserved2f6;		/* Byte offset 0x2f6, CSR Addr 0x5417b, Direction=N/A */
	uint8_t reserved2f7;		/* Byte offset 0x2f7, CSR Addr 0x5417b, Direction=N/A */
	uint8_t reserved2f8;		/* Byte offset 0x2f8, CSR Addr 0x5417c, Direction=N/A */
	uint8_t reserved2f9;		/* Byte offset 0x2f9, CSR Addr 0x5417c, Direction=N/A */
	uint8_t reserved2fa;		/* Byte offset 0x2fa, CSR Addr 0x5417d, Direction=N/A */
	uint8_t reserved2fb;		/* Byte offset 0x2fb, CSR Addr 0x5417d, Direction=N/A */
	uint8_t reserved2fc;		/* Byte offset 0x2fc, CSR Addr 0x5417e, Direction=N/A */
	uint8_t reserved2fd;		/* Byte offset 0x2fd, CSR Addr 0x5417e, Direction=N/A */
	uint8_t reserved2fe;		/* Byte offset 0x2fe, CSR Addr 0x5417f, Direction=N/A */
	uint8_t reserved2ff;		/* Byte offset 0x2ff, CSR Addr 0x5417f, Direction=N/A */
	uint8_t reserved300;		/* Byte offset 0x300, CSR Addr 0x54180, Direction=N/A */
	uint8_t reserved301;		/* Byte offset 0x301, CSR Addr 0x54180, Direction=N/A */
	uint8_t reserved302;		/* Byte offset 0x302, CSR Addr 0x54181, Direction=N/A */
	uint8_t reserved303;		/* Byte offset 0x303, CSR Addr 0x54181, Direction=N/A */
	uint8_t reserved304;		/* Byte offset 0x304, CSR Addr 0x54182, Direction=N/A */
	uint8_t reserved305;		/* Byte offset 0x305, CSR Addr 0x54182, Direction=N/A */
	uint8_t reserved306;		/* Byte offset 0x306, CSR Addr 0x54183, Direction=N/A */
	uint8_t reserved307;		/* Byte offset 0x307, CSR Addr 0x54183, Direction=N/A */
	uint8_t reserved308;		/* Byte offset 0x308, CSR Addr 0x54184, Direction=N/A */
	uint8_t reserved309;		/* Byte offset 0x309, CSR Addr 0x54184, Direction=N/A */
	uint8_t reserved30a;		/* Byte offset 0x30a, CSR Addr 0x54185, Direction=N/A */
	uint8_t reserved30b;		/* Byte offset 0x30b, CSR Addr 0x54185, Direction=N/A */
	uint8_t reserved30c;		/* Byte offset 0x30c, CSR Addr 0x54186, Direction=N/A */
	uint8_t reserved30d;		/* Byte offset 0x30d, CSR Addr 0x54186, Direction=N/A */
	uint8_t reserved30e;		/* Byte offset 0x30e, CSR Addr 0x54187, Direction=N/A */
	uint8_t reserved30f;		/* Byte offset 0x30f, CSR Addr 0x54187, Direction=N/A */
	uint8_t reserved310;		/* Byte offset 0x310, CSR Addr 0x54188, Direction=N/A */
	uint8_t reserved311;		/* Byte offset 0x311, CSR Addr 0x54188, Direction=N/A */
	uint8_t reserved312;		/* Byte offset 0x312, CSR Addr 0x54189, Direction=N/A */
	uint8_t reserved313;		/* Byte offset 0x313, CSR Addr 0x54189, Direction=N/A */
	uint8_t reserved314;		/* Byte offset 0x314, CSR Addr 0x5418a, Direction=N/A */
	uint8_t reserved315;		/* Byte offset 0x315, CSR Addr 0x5418a, Direction=N/A */
	uint8_t reserved316;		/* Byte offset 0x316, CSR Addr 0x5418b, Direction=N/A */
	uint8_t reserved317;		/* Byte offset 0x317, CSR Addr 0x5418b, Direction=N/A */
	uint8_t reserved318;		/* Byte offset 0x318, CSR Addr 0x5418c, Direction=N/A */
	uint8_t reserved319;		/* Byte offset 0x319, CSR Addr 0x5418c, Direction=N/A */
	uint8_t reserved31a;		/* Byte offset 0x31a, CSR Addr 0x5418d, Direction=N/A */
	uint8_t reserved31b;		/* Byte offset 0x31b, CSR Addr 0x5418d, Direction=N/A */
	uint8_t reserved31c;		/* Byte offset 0x31c, CSR Addr 0x5418e, Direction=N/A */
	uint8_t reserved31d;		/* Byte offset 0x31d, CSR Addr 0x5418e, Direction=N/A */
	uint8_t reserved31e;		/* Byte offset 0x31e, CSR Addr 0x5418f, Direction=N/A */
	uint8_t reserved31f;		/* Byte offset 0x31f, CSR Addr 0x5418f, Direction=N/A */
	uint8_t reserved320;		/* Byte offset 0x320, CSR Addr 0x54190, Direction=N/A */
	uint8_t reserved321;		/* Byte offset 0x321, CSR Addr 0x54190, Direction=N/A */
	uint8_t reserved322;		/* Byte offset 0x322, CSR Addr 0x54191, Direction=N/A */
	uint8_t reserved323;		/* Byte offset 0x323, CSR Addr 0x54191, Direction=N/A */
	uint8_t reserved324;		/* Byte offset 0x324, CSR Addr 0x54192, Direction=N/A */
	uint8_t reserved325;		/* Byte offset 0x325, CSR Addr 0x54192, Direction=N/A */
	uint8_t reserved326;		/* Byte offset 0x326, CSR Addr 0x54193, Direction=N/A */
	uint8_t reserved327;		/* Byte offset 0x327, CSR Addr 0x54193, Direction=N/A */
	uint8_t reserved328;		/* Byte offset 0x328, CSR Addr 0x54194, Direction=N/A */
	uint8_t reserved329;		/* Byte offset 0x329, CSR Addr 0x54194, Direction=N/A */
	uint8_t reserved32a;		/* Byte offset 0x32a, CSR Addr 0x54195, Direction=N/A */
	uint8_t reserved32b;		/* Byte offset 0x32b, CSR Addr 0x54195, Direction=N/A */
	uint8_t reserved32c;		/* Byte offset 0x32c, CSR Addr 0x54196, Direction=N/A */
	uint8_t reserved32d;		/* Byte offset 0x32d, CSR Addr 0x54196, Direction=N/A */
	uint8_t reserved32e;		/* Byte offset 0x32e, CSR Addr 0x54197, Direction=N/A */
	uint8_t reserved32f;		/* Byte offset 0x32f, CSR Addr 0x54197, Direction=N/A */
	uint8_t reserved330;		/* Byte offset 0x330, CSR Addr 0x54198, Direction=N/A */
	uint8_t reserved331;		/* Byte offset 0x331, CSR Addr 0x54198, Direction=N/A */
	uint8_t reserved332;		/* Byte offset 0x332, CSR Addr 0x54199, Direction=N/A */
	uint8_t reserved333;		/* Byte offset 0x333, CSR Addr 0x54199, Direction=N/A */
	uint8_t reserved334;		/* Byte offset 0x334, CSR Addr 0x5419a, Direction=N/A */
	uint8_t reserved335;		/* Byte offset 0x335, CSR Addr 0x5419a, Direction=N/A */
	uint8_t reserved336;		/* Byte offset 0x336, CSR Addr 0x5419b, Direction=N/A */
	uint8_t reserved337;		/* Byte offset 0x337, CSR Addr 0x5419b, Direction=N/A */
	uint8_t reserved338;		/* Byte offset 0x338, CSR Addr 0x5419c, Direction=N/A */
	uint8_t reserved339;		/* Byte offset 0x339, CSR Addr 0x5419c, Direction=N/A */
	uint8_t reserved33a;		/* Byte offset 0x33a, CSR Addr 0x5419d, Direction=N/A */
	uint8_t reserved33b;		/* Byte offset 0x33b, CSR Addr 0x5419d, Direction=N/A */
	uint8_t reserved33c;		/* Byte offset 0x33c, CSR Addr 0x5419e, Direction=N/A */
	uint8_t reserved33d;		/* Byte offset 0x33d, CSR Addr 0x5419e, Direction=N/A */
	uint8_t reserved33e;		/* Byte offset 0x33e, CSR Addr 0x5419f, Direction=N/A */
	uint8_t reserved33f;		/* Byte offset 0x33f, CSR Addr 0x5419f, Direction=N/A */
	uint8_t reserved340;		/* Byte offset 0x340, CSR Addr 0x541a0, Direction=N/A */
	uint8_t reserved341;		/* Byte offset 0x341, CSR Addr 0x541a0, Direction=N/A */
	uint8_t reserved342;		/* Byte offset 0x342, CSR Addr 0x541a1, Direction=N/A */
	uint8_t reserved343;		/* Byte offset 0x343, CSR Addr 0x541a1, Direction=N/A */
	uint8_t reserved344;		/* Byte offset 0x344, CSR Addr 0x541a2, Direction=N/A */
	uint8_t reserved345;		/* Byte offset 0x345, CSR Addr 0x541a2, Direction=N/A */
	uint8_t reserved346;		/* Byte offset 0x346, CSR Addr 0x541a3, Direction=N/A */
	uint8_t reserved347;		/* Byte offset 0x347, CSR Addr 0x541a3, Direction=N/A */
	uint8_t reserved348;		/* Byte offset 0x348, CSR Addr 0x541a4, Direction=N/A */
	uint8_t reserved349;		/* Byte offset 0x349, CSR Addr 0x541a4, Direction=N/A */
	uint8_t reserved34a;		/* Byte offset 0x34a, CSR Addr 0x541a5, Direction=N/A */
	uint8_t reserved34b;		/* Byte offset 0x34b, CSR Addr 0x541a5, Direction=N/A */
	uint8_t reserved34c;		/* Byte offset 0x34c, CSR Addr 0x541a6, Direction=N/A */
	uint8_t reserved34d;		/* Byte offset 0x34d, CSR Addr 0x541a6, Direction=N/A */
	uint8_t reserved34e;		/* Byte offset 0x34e, CSR Addr 0x541a7, Direction=N/A */
	uint8_t reserved34f;		/* Byte offset 0x34f, CSR Addr 0x541a7, Direction=N/A */
	uint8_t reserved350;		/* Byte offset 0x350, CSR Addr 0x541a8, Direction=N/A */
	uint8_t reserved351;		/* Byte offset 0x351, CSR Addr 0x541a8, Direction=N/A */
	uint8_t reserved352;		/* Byte offset 0x352, CSR Addr 0x541a9, Direction=N/A */
	uint8_t reserved353;		/* Byte offset 0x353, CSR Addr 0x541a9, Direction=N/A */
	uint8_t reserved354;		/* Byte offset 0x354, CSR Addr 0x541aa, Direction=N/A */
	uint8_t reserved355;		/* Byte offset 0x355, CSR Addr 0x541aa, Direction=N/A */
	uint8_t reserved356;		/* Byte offset 0x356, CSR Addr 0x541ab, Direction=N/A */
	uint8_t reserved357;		/* Byte offset 0x357, CSR Addr 0x541ab, Direction=N/A */
	uint8_t reserved358;		/* Byte offset 0x358, CSR Addr 0x541ac, Direction=N/A */
	uint8_t reserved359;		/* Byte offset 0x359, CSR Addr 0x541ac, Direction=N/A */
	uint8_t reserved35a;		/* Byte offset 0x35a, CSR Addr 0x541ad, Direction=N/A */
	uint8_t reserved35b;		/* Byte offset 0x35b, CSR Addr 0x541ad, Direction=N/A */
	uint8_t reserved35c;		/* Byte offset 0x35c, CSR Addr 0x541ae, Direction=N/A */
	uint8_t reserved35d;		/* Byte offset 0x35d, CSR Addr 0x541ae, Direction=N/A */
	uint8_t reserved35e;		/* Byte offset 0x35e, CSR Addr 0x541af, Direction=N/A */
	uint8_t reserved35f;		/* Byte offset 0x35f, CSR Addr 0x541af, Direction=N/A */
	uint8_t reserved360;		/* Byte offset 0x360, CSR Addr 0x541b0, Direction=N/A */
	uint8_t reserved361;		/* Byte offset 0x361, CSR Addr 0x541b0, Direction=N/A */
	uint8_t reserved362;		/* Byte offset 0x362, CSR Addr 0x541b1, Direction=N/A */
	uint8_t reserved363;		/* Byte offset 0x363, CSR Addr 0x541b1, Direction=N/A */
	uint8_t reserved364;		/* Byte offset 0x364, CSR Addr 0x541b2, Direction=N/A */
	uint8_t reserved365;		/* Byte offset 0x365, CSR Addr 0x541b2, Direction=N/A */
	uint8_t reserved366;		/* Byte offset 0x366, CSR Addr 0x541b3, Direction=N/A */
	uint8_t reserved367;		/* Byte offset 0x367, CSR Addr 0x541b3, Direction=N/A */
	uint8_t reserved368;		/* Byte offset 0x368, CSR Addr 0x541b4, Direction=N/A */
	uint8_t reserved369;		/* Byte offset 0x369, CSR Addr 0x541b4, Direction=N/A */
	uint8_t reserved36a;		/* Byte offset 0x36a, CSR Addr 0x541b5, Direction=N/A */
	uint8_t reserved36b;		/* Byte offset 0x36b, CSR Addr 0x541b5, Direction=N/A */
	uint8_t reserved36c;		/* Byte offset 0x36c, CSR Addr 0x541b6, Direction=N/A */
	uint8_t reserved36d;		/* Byte offset 0x36d, CSR Addr 0x541b6, Direction=N/A */
	uint8_t reserved36e;		/* Byte offset 0x36e, CSR Addr 0x541b7, Direction=N/A */
	uint8_t reserved36f;		/* Byte offset 0x36f, CSR Addr 0x541b7, Direction=N/A */
	uint8_t reserved370;		/* Byte offset 0x370, CSR Addr 0x541b8, Direction=N/A */
	uint8_t reserved371;		/* Byte offset 0x371, CSR Addr 0x541b8, Direction=N/A */
	uint8_t reserved372;		/* Byte offset 0x372, CSR Addr 0x541b9, Direction=N/A */
	uint8_t reserved373;		/* Byte offset 0x373, CSR Addr 0x541b9, Direction=N/A */
	uint8_t reserved374;		/* Byte offset 0x374, CSR Addr 0x541ba, Direction=N/A */
	uint8_t reserved375;		/* Byte offset 0x375, CSR Addr 0x541ba, Direction=N/A */
	uint8_t reserved376;		/* Byte offset 0x376, CSR Addr 0x541bb, Direction=N/A */
	uint8_t reserved377;		/* Byte offset 0x377, CSR Addr 0x541bb, Direction=N/A */
	uint8_t reserved378;		/* Byte offset 0x378, CSR Addr 0x541bc, Direction=N/A */
	uint8_t reserved379;		/* Byte offset 0x379, CSR Addr 0x541bc, Direction=N/A */
	uint8_t reserved37a;		/* Byte offset 0x37a, CSR Addr 0x541bd, Direction=N/A */
	uint8_t reserved37b;		/* Byte offset 0x37b, CSR Addr 0x541bd, Direction=N/A */
	uint8_t reserved37c;		/* Byte offset 0x37c, CSR Addr 0x541be, Direction=N/A */
	uint8_t reserved37d;		/* Byte offset 0x37d, CSR Addr 0x541be, Direction=N/A */
	uint8_t reserved37e;		/* Byte offset 0x37e, CSR Addr 0x541bf, Direction=N/A */
	uint8_t reserved37f;		/* Byte offset 0x37f, CSR Addr 0x541bf, Direction=N/A */
	uint8_t reserved380;		/* Byte offset 0x380, CSR Addr 0x541c0, Direction=N/A */
	uint8_t reserved381;		/* Byte offset 0x381, CSR Addr 0x541c0, Direction=N/A */
	uint8_t reserved382;		/* Byte offset 0x382, CSR Addr 0x541c1, Direction=N/A */
	uint8_t reserved383;		/* Byte offset 0x383, CSR Addr 0x541c1, Direction=N/A */
	uint8_t reserved384;		/* Byte offset 0x384, CSR Addr 0x541c2, Direction=N/A */
	uint8_t reserved385;		/* Byte offset 0x385, CSR Addr 0x541c2, Direction=N/A */
	uint8_t reserved386;		/* Byte offset 0x386, CSR Addr 0x541c3, Direction=N/A */
	uint8_t reserved387;		/* Byte offset 0x387, CSR Addr 0x541c3, Direction=N/A */
	uint8_t reserved388;		/* Byte offset 0x388, CSR Addr 0x541c4, Direction=N/A */
	uint8_t reserved389;		/* Byte offset 0x389, CSR Addr 0x541c4, Direction=N/A */
	uint8_t reserved38a;		/* Byte offset 0x38a, CSR Addr 0x541c5, Direction=N/A */
	uint8_t reserved38b;		/* Byte offset 0x38b, CSR Addr 0x541c5, Direction=N/A */
	uint8_t reserved38c;		/* Byte offset 0x38c, CSR Addr 0x541c6, Direction=N/A */
	uint8_t reserved38d;		/* Byte offset 0x38d, CSR Addr 0x541c6, Direction=N/A */
	uint8_t reserved38e;		/* Byte offset 0x38e, CSR Addr 0x541c7, Direction=N/A */
	uint8_t reserved38f;		/* Byte offset 0x38f, CSR Addr 0x541c7, Direction=N/A */
	uint8_t reserved390;		/* Byte offset 0x390, CSR Addr 0x541c8, Direction=N/A */
	uint8_t reserved391;		/* Byte offset 0x391, CSR Addr 0x541c8, Direction=N/A */
	uint8_t reserved392;		/* Byte offset 0x392, CSR Addr 0x541c9, Direction=N/A */
	uint8_t reserved393;		/* Byte offset 0x393, CSR Addr 0x541c9, Direction=N/A */
	uint8_t reserved394;		/* Byte offset 0x394, CSR Addr 0x541ca, Direction=N/A */
	uint8_t reserved395;		/* Byte offset 0x395, CSR Addr 0x541ca, Direction=N/A */
	uint8_t reserved396;		/* Byte offset 0x396, CSR Addr 0x541cb, Direction=N/A */
	uint8_t reserved397;		/* Byte offset 0x397, CSR Addr 0x541cb, Direction=N/A */
	uint8_t reserved398;		/* Byte offset 0x398, CSR Addr 0x541cc, Direction=N/A */
	uint8_t reserved399;		/* Byte offset 0x399, CSR Addr 0x541cc, Direction=N/A */
	uint8_t reserved39a;		/* Byte offset 0x39a, CSR Addr 0x541cd, Direction=N/A */
	uint8_t reserved39b;		/* Byte offset 0x39b, CSR Addr 0x541cd, Direction=N/A */
	uint8_t reserved39c;		/* Byte offset 0x39c, CSR Addr 0x541ce, Direction=N/A */
	uint8_t reserved39d;		/* Byte offset 0x39d, CSR Addr 0x541ce, Direction=N/A */
	uint8_t reserved39e;		/* Byte offset 0x39e, CSR Addr 0x541cf, Direction=N/A */
	uint8_t reserved39f;		/* Byte offset 0x39f, CSR Addr 0x541cf, Direction=N/A */
	uint8_t reserved3a0;		/* Byte offset 0x3a0, CSR Addr 0x541d0, Direction=N/A */
	uint8_t reserved3a1;		/* Byte offset 0x3a1, CSR Addr 0x541d0, Direction=N/A */
	uint8_t reserved3a2;		/* Byte offset 0x3a2, CSR Addr 0x541d1, Direction=N/A */
	uint8_t reserved3a3;		/* Byte offset 0x3a3, CSR Addr 0x541d1, Direction=N/A */
	uint8_t reserved3a4;		/* Byte offset 0x3a4, CSR Addr 0x541d2, Direction=N/A */
	uint8_t reserved3a5;		/* Byte offset 0x3a5, CSR Addr 0x541d2, Direction=N/A */
	uint8_t reserved3a6;		/* Byte offset 0x3a6, CSR Addr 0x541d3, Direction=N/A */
	uint8_t reserved3a7;		/* Byte offset 0x3a7, CSR Addr 0x541d3, Direction=N/A */
	uint8_t reserved3a8;		/* Byte offset 0x3a8, CSR Addr 0x541d4, Direction=N/A */
	uint8_t reserved3a9;		/* Byte offset 0x3a9, CSR Addr 0x541d4, Direction=N/A */
	uint8_t reserved3aa;		/* Byte offset 0x3aa, CSR Addr 0x541d5, Direction=N/A */
	uint8_t reserved3ab;		/* Byte offset 0x3ab, CSR Addr 0x541d5, Direction=N/A */
	uint8_t reserved3ac;		/* Byte offset 0x3ac, CSR Addr 0x541d6, Direction=N/A */
	uint8_t reserved3ad;		/* Byte offset 0x3ad, CSR Addr 0x541d6, Direction=N/A */
	uint8_t reserved3ae;		/* Byte offset 0x3ae, CSR Addr 0x541d7, Direction=N/A */
	uint8_t reserved3af;		/* Byte offset 0x3af, CSR Addr 0x541d7, Direction=N/A */
	uint8_t reserved3b0;		/* Byte offset 0x3b0, CSR Addr 0x541d8, Direction=N/A */
	uint8_t reserved3b1;		/* Byte offset 0x3b1, CSR Addr 0x541d8, Direction=N/A */
	uint8_t reserved3b2;		/* Byte offset 0x3b2, CSR Addr 0x541d9, Direction=N/A */
	uint8_t reserved3b3;		/* Byte offset 0x3b3, CSR Addr 0x541d9, Direction=N/A */
	uint8_t reserved3b4;		/* Byte offset 0x3b4, CSR Addr 0x541da, Direction=N/A */
	uint8_t reserved3b5;		/* Byte offset 0x3b5, CSR Addr 0x541da, Direction=N/A */
	uint8_t reserved3b6;		/* Byte offset 0x3b6, CSR Addr 0x541db, Direction=N/A */
	uint8_t reserved3b7;		/* Byte offset 0x3b7, CSR Addr 0x541db, Direction=N/A */
	uint8_t reserved3b8;		/* Byte offset 0x3b8, CSR Addr 0x541dc, Direction=N/A */
	uint8_t reserved3b9;		/* Byte offset 0x3b9, CSR Addr 0x541dc, Direction=N/A */
	uint8_t reserved3ba;		/* Byte offset 0x3ba, CSR Addr 0x541dd, Direction=N/A */
	uint8_t reserved3bb;		/* Byte offset 0x3bb, CSR Addr 0x541dd, Direction=N/A */
	uint8_t reserved3bc;		/* Byte offset 0x3bc, CSR Addr 0x541de, Direction=N/A */
	uint8_t reserved3bd;		/* Byte offset 0x3bd, CSR Addr 0x541de, Direction=N/A */
	uint8_t reserved3be;		/* Byte offset 0x3be, CSR Addr 0x541df, Direction=N/A */
	uint8_t reserved3bf;		/* Byte offset 0x3bf, CSR Addr 0x541df, Direction=N/A */
	uint8_t reserved3c0;		/* Byte offset 0x3c0, CSR Addr 0x541e0, Direction=N/A */
	uint8_t reserved3c1;		/* Byte offset 0x3c1, CSR Addr 0x541e0, Direction=N/A */
	uint8_t reserved3c2;		/* Byte offset 0x3c2, CSR Addr 0x541e1, Direction=N/A */
	uint8_t reserved3c3;		/* Byte offset 0x3c3, CSR Addr 0x541e1, Direction=N/A */
	uint8_t reserved3c4;		/* Byte offset 0x3c4, CSR Addr 0x541e2, Direction=N/A */
	uint8_t reserved3c5;		/* Byte offset 0x3c5, CSR Addr 0x541e2, Direction=N/A */
	uint8_t reserved3c6;		/* Byte offset 0x3c6, CSR Addr 0x541e3, Direction=N/A */
	uint8_t reserved3c7;		/* Byte offset 0x3c7, CSR Addr 0x541e3, Direction=N/A */
	uint8_t reserved3c8;		/* Byte offset 0x3c8, CSR Addr 0x541e4, Direction=N/A */
	uint8_t reserved3c9;		/* Byte offset 0x3c9, CSR Addr 0x541e4, Direction=N/A */
	uint8_t reserved3ca;		/* Byte offset 0x3ca, CSR Addr 0x541e5, Direction=N/A */
	uint8_t reserved3cb;		/* Byte offset 0x3cb, CSR Addr 0x541e5, Direction=N/A */
	uint8_t reserved3cc;		/* Byte offset 0x3cc, CSR Addr 0x541e6, Direction=N/A */
	uint8_t reserved3cd;		/* Byte offset 0x3cd, CSR Addr 0x541e6, Direction=N/A */
	uint8_t reserved3ce;		/* Byte offset 0x3ce, CSR Addr 0x541e7, Direction=N/A */
	uint8_t reserved3cf;		/* Byte offset 0x3cf, CSR Addr 0x541e7, Direction=N/A */
	uint8_t reserved3d0;		/* Byte offset 0x3d0, CSR Addr 0x541e8, Direction=N/A */
	uint8_t reserved3d1;		/* Byte offset 0x3d1, CSR Addr 0x541e8, Direction=N/A */
	uint8_t reserved3d2;		/* Byte offset 0x3d2, CSR Addr 0x541e9, Direction=N/A */
	uint8_t reserved3d3;		/* Byte offset 0x3d3, CSR Addr 0x541e9, Direction=N/A */
	uint8_t reserved3d4;		/* Byte offset 0x3d4, CSR Addr 0x541ea, Direction=N/A */
	uint8_t reserved3d5;		/* Byte offset 0x3d5, CSR Addr 0x541ea, Direction=N/A */
	uint8_t reserved3d6;		/* Byte offset 0x3d6, CSR Addr 0x541eb, Direction=N/A */
	uint8_t reserved3d7;		/* Byte offset 0x3d7, CSR Addr 0x541eb, Direction=N/A */
	uint8_t reserved3d8;		/* Byte offset 0x3d8, CSR Addr 0x541ec, Direction=N/A */
	uint8_t reserved3d9;		/* Byte offset 0x3d9, CSR Addr 0x541ec, Direction=N/A */
	uint8_t reserved3da;		/* Byte offset 0x3da, CSR Addr 0x541ed, Direction=N/A */
	uint8_t reserved3db;		/* Byte offset 0x3db, CSR Addr 0x541ed, Direction=N/A */
	uint8_t reserved3dc;		/* Byte offset 0x3dc, CSR Addr 0x541ee, Direction=N/A */
	uint8_t reserved3dd;		/* Byte offset 0x3dd, CSR Addr 0x541ee, Direction=N/A */
	uint8_t reserved3de;		/* Byte offset 0x3de, CSR Addr 0x541ef, Direction=N/A */
	uint8_t reserved3df;		/* Byte offset 0x3df, CSR Addr 0x541ef, Direction=N/A */
	uint8_t reserved3e0;		/* Byte offset 0x3e0, CSR Addr 0x541f0, Direction=N/A */
	uint8_t reserved3e1;		/* Byte offset 0x3e1, CSR Addr 0x541f0, Direction=N/A */
	uint8_t reserved3e2;		/* Byte offset 0x3e2, CSR Addr 0x541f1, Direction=N/A */
	uint8_t reserved3e3;		/* Byte offset 0x3e3, CSR Addr 0x541f1, Direction=N/A */
	uint8_t reserved3e4;		/* Byte offset 0x3e4, CSR Addr 0x541f2, Direction=N/A */
	uint8_t reserved3e5;		/* Byte offset 0x3e5, CSR Addr 0x541f2, Direction=N/A */
	uint8_t reserved3e6;		/* Byte offset 0x3e6, CSR Addr 0x541f3, Direction=N/A */
	uint8_t reserved3e7;		/* Byte offset 0x3e7, CSR Addr 0x541f3, Direction=N/A */
	uint8_t reserved3e8;		/* Byte offset 0x3e8, CSR Addr 0x541f4, Direction=N/A */
	uint8_t reserved3e9;		/* Byte offset 0x3e9, CSR Addr 0x541f4, Direction=N/A */
	uint8_t reserved3ea;		/* Byte offset 0x3ea, CSR Addr 0x541f5, Direction=N/A */
	uint8_t reserved3eb;		/* Byte offset 0x3eb, CSR Addr 0x541f5, Direction=N/A */
	uint8_t reserved3ec;		/* Byte offset 0x3ec, CSR Addr 0x541f6, Direction=N/A */
	uint8_t reserved3ed;		/* Byte offset 0x3ed, CSR Addr 0x541f6, Direction=N/A */
	uint8_t reserved3ee;		/* Byte offset 0x3ee, CSR Addr 0x541f7, Direction=N/A */
	uint8_t reserved3ef;		/* Byte offset 0x3ef, CSR Addr 0x541f7, Direction=N/A */
	uint8_t reserved3f0;		/* Byte offset 0x3f0, CSR Addr 0x541f8, Direction=N/A */
	uint8_t reserved3f1;		/* Byte offset 0x3f1, CSR Addr 0x541f8, Direction=N/A */
	uint8_t reserved3f2;		/* Byte offset 0x3f2, CSR Addr 0x541f9, Direction=N/A */
	uint8_t reserved3f3;		/* Byte offset 0x3f3, CSR Addr 0x541f9, Direction=N/A */
	uint8_t reserved3f4;		/* Byte offset 0x3f4, CSR Addr 0x541fa, Direction=N/A */
	uint8_t reserved3f5;		/* Byte offset 0x3f5, CSR Addr 0x541fa, Direction=N/A */
	uint16_t alt_cas_l;		/*
					 * Byte offset 0x3f6, CSR Addr 0x541fb, Direction=in
					 * This field must be populated if RdDBI is enabled
					 * (applicable when mr5[A12] == 1).
					 * RdDBI is dynamically disabled in certain training steps,
					 * and so the [RdDBI disabled] CAS Latency must be provided
					 * in this field.
					 * The required encoding is as follows:
					 * alt_cas_l[0] == 0: use value in mr0
					 * alt_cas_l[0] == 1: use value in alt_cas_l, i.e.,
					 *   mr0{A[12],A[6],A[5],A[4],A[2]} = alt_cas_l[12,6,5,4,2]
					 * Other bits are ignored
					 */
	uint8_t alt_wcas_l;		/*
					 * Byte offset 0x3f8, CSR Addr 0x541fc, Direction=In
					 * This field must be populated if 2tCK write preambles are
					 * enabled (applicable when mr4[A12] == 1).
					 * 2tCK write prambles are dynamically disabled in certain
					 * training steps, and so the [1tCK write preamble] WCAS
					 * Latency must be provided in this field.
					 * The required encoding is as follows:
					 * alt_wcas_l[0] == 0: use value in mr2
					 * alt_wcas_l[0] == 1: use value in alt_wcas_l, i.e.,
					 *   mr2{A[5],A[4],A[3]} = alt_wcas_l[5,4,3]
					 * Other bits are ignored
					 */
	uint8_t d4misc;			/*
					 * Byte offset 0x3f9, CSR Addr 0x541fc, Direction=In
					 * Contains various options for training DDR4 Devices.
					 *
					 * Bit fields:
					 *
					 * d4misc[7:5,2,1] RFU, must be zero
					 *
					 * d4misc[0] = protect memory reset
					 *   0x1 = dfi_reset_n cannot control BP_MEMRESERT_L to
					 *         devices after training.
					 *   0x0 = dfi_resert_n can control BP_MEMRESERT_L to
					 *         devices after training
					 *
					 * d4misc[3]: reserved
					 *
					 * d4misc[4]: DRAM reset mode
					 *   0x1 = Do not reset DRAM during devinit
					 *   0x0 = Reset DRAM during devinit
					 */
} __packed __aligned(2);

#endif /* MNPMUSRAMMSGBLOCK_DDR4_H */

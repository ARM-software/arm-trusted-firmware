/*
 * Copyright 2021 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef DEBUG
struct phy_msg {
	uint32_t index;
	const char *msg;
};

static const struct phy_msg messages_1d[] = {
	{0x00000001,
	 "PMU1:prbsGenCtl:%x\n"
	},
	{0x00010000,
	 "PMU1: loading 2D acsm sequence\n"
	},
	{0x00020000,
	 "PMU1: loading 1D acsm sequence\n"
	},
	{0x00030002,
	 "PMU3: %d memclocks @ %d to get half of 300ns\n"
	},
	{0x00040000,
	 "PMU: Error: User requested MPR read pattern for read DQS training in DDR3 Mode\n"
	},
	{0x00050000,
	 "PMU3: Running 1D search for left eye edge\n"
	},
	{0x00060001,
	 "PMU1: In Phase Left Edge Search cs %d\n"
	},
	{0x00070001,
	 "PMU1: Out of Phase Left Edge Search cs %d\n"
	},
	{0x00080000,
	 "PMU3: Running 1D search for right eye edge\n"
	},
	{0x00090001,
	 "PMU1: In Phase Right Edge Search cs %d\n"
	},
	{0x000a0001,
	 "PMU1: Out of Phase Right Edge Search cs %d\n"
	},
	{0x000b0001,
	 "PMU1: mxRdLat training pstate %d\n"
	},
	{0x000c0001,
	 "PMU1: mxRdLat search for cs %d\n"
	},
	{0x000d0001,
	 "PMU0: MaxRdLat non consistent DtsmLoThldXingInd 0x%03x\n"
	},
	{0x000e0003,
	 "PMU4: CS %d Dbyte %d worked with DFIMRL = %d DFICLKs\n"
	},
	{0x000f0004,
	 "PMU3: MaxRdLat Read Lane err mask for csn %d, DFIMRL %2d DFIClks, dbyte %d = 0x%03x\n"
	},
	{0x00100003,
	 "PMU3: MaxRdLat Read Lane err mask for csn %d DFIMRL %2d, All dbytes = 0x%03x\n"
	},
	{0x00110001,
	 "PMU: Error: CS%d failed to find a DFIMRL setting that worked for all bytes during MaxRdLat training\n"
	},
	{0x00120002,
	 "PMU3: Smallest passing DFIMRL for all dbytes in CS%d = %d DFIClks\n"
	},
	{0x00130000,
	 "PMU: Error: No passing DFIMRL value found for any chip select during MaxRdLat training\n"
	},
	{0x00140003,
	 "PMU: Error: Dbyte %d lane %d txDqDly passing region is too small (width = %d)\n"
	},
	{0x00150006,
	 "PMU10: Adjusting rxclkdly db %d nib %d from %d+%d=%d->%d\n"
	},
	{0x00160000,
	 "PMU4: TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x00170005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00180002,
	 "PMU2: TXDQ delayLeft[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00190004,
	 "PMU2: TXDQ delayLeft[%2d] = %3d oopScaled = %3d selectOop %d\n"
	},
	{0x001a0002,
	 "PMU2: TXDQ delayRight[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x001b0004,
	 "PMU2: TXDQ delayRight[%2d] = %3d oopScaled = %3d selectOop %d\n"
	},
	{0x001c0003,
	 "PMU: Error: Dbyte %d lane %d txDqDly passing region is too small (width = %d)\n"
	},
	{0x001d0000,
	 "PMU4: TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x001e0002,
	 "PMU4: DB %d Lane %d: (DISCONNECTED)\n"
	},
	{0x001f0005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00200002,
	 "PMU3: Running 1D search csn %d for DM Right/NotLeft(%d) eye edge\n"
	},
	{0x00210002,
	 "PMU3: WrDq DM byte%2d with Errcnt %d\n"
	},
	{0x00220002,
	 "PMU3: WrDq DM byte%2d avgDly 0x%04x\n"
	},
	{0x00230002,
	 "PMU1: WrDq DM byte%2d with Errcnt %d\n"
	},
	{0x00240001,
	 "PMU: Error: Dbyte %d txDqDly DM training did not start inside the eye\n"
	},
	{0x00250000,
	 "PMU4: DM TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x00260002,
	 "PMU4: DB %d Lane %d: (DISCONNECTED)\n"
	},
	{0x00270005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00280003,
	 "PMU: Error: Dbyte %d lane %d txDqDly DM passing region is too small (width = %d)\n"
	},
	{0x00290004,
	 "PMU3: Errcnt for MRD/MWD search nib %2d delay = (%d, 0x%02x) = %d\n"
	},
	{0x002a0000,
	 "PMU3: Precharge all open banks\n"
	},
	{0x002b0002,
	 "PMU: Error: Dbyte %d nibble %d found mutliple working coarse delay setting for MRD/MWD\n"
	},
	{0x002c0000,
	 "PMU4: MRD Passing Regions (coarseVal, fineLeft fineRight -> fineCenter)\n"
	},
	{0x002d0000,
	 "PMU4: MWD Passing Regions (coarseVal, fineLeft fineRight -> fineCenter)\n"
	},
	{0x002e0004,
	 "PMU10: Warning: DB %d nibble %d has multiple working coarse delays, %d and %d, choosing the smaller delay\n"
	},
	{0x002f0003,
	 "PMU: Error: Dbyte %d nibble %d MRD/MWD passing region is too small (width = %d)\n"
	},
	{0x00300006,
	 "PMU4: DB %d nibble %d: %3d, %3d %3d -> %3d\n"
	},
	{0x00310002,
	 "PMU1: Start MRD/nMWD %d for csn %d\n"
	},
	{0x00320002,
	 "PMU2: RXDQS delayLeft[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00330006,
	 "PMU2: RXDQS delayLeft[%2d] = %3d delayOop[%2d] = %3d OopScaled %4d, selectOop %d\n"
	},
	{0x00340002,
	 "PMU2: RXDQS delayRight[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00350006,
	 "PMU2: RXDQS delayRight[%2d] = %3d delayOop[%2d] = %4d OopScaled %4d, selectOop %d\n"
	},
	{0x00360000,
	 "PMU4: RxClkDly Passing Regions (EyeLeft EyeRight -> EyeCenter)\n"
	},
	{0x00370002,
	 "PMU4: DB %d nibble %d: (DISCONNECTED)\n"
	},
	{0x00380005,
	 "PMU4: DB %d nibble %d: %3d %3d -> %3d\n"
	},
	{0x00390003,
	 "PMU: Error: Dbyte %d nibble %d rxClkDly passing region is too small (width = %d)\n"
	},
	{0x003a0002,
	 "PMU0: goodbar = %d for RDWR_BLEN %d\n"
	},
	{0x003b0001,
	 "PMU3: RxClkDly = %d\n"
	},
	{0x003c0005,
	 "PMU0: db %d l %d absLane %d -> bottom %d top %d\n"
	},
	{0x003d0009,
	 "PMU3: BYTE %d - %3d %3d %3d %3d %3d %3d %3d %3d\n"
	},
	{0x003e0002,
	 "PMU: Error: dbyte %d lane %d's per-lane vrefDAC's had no passing region\n"
	},
	{0x003f0004,
	 "PMU0: db%d l%d - %d %d\n"
	},
	{0x00400002,
	 "PMU0: goodbar = %d for RDWR_BLEN %d\n"
	},
	{0x00410004,
	 "PMU3: db%d l%d saw %d issues at rxClkDly %d\n"
	},
	{0x00420003,
	 "PMU3: db%d l%d first saw a pass->fail edge at rxClkDly %d\n"
	},
	{0x00430002,
	 "PMU3: lane %d PBD = %d\n"
	},
	{0x00440003,
	 "PMU3: db%d l%d first saw a DBI pass->fail edge at rxClkDly %d\n"
	},
	{0x00450003,
	 "PMU2: db%d l%d already passed rxPBD = %d\n"
	},
	{0x00460003,
	 "PMU0: db%d l%d, PBD = %d\n"
	},
	{0x00470002,
	 "PMU: Error: dbyte %d lane %d failed read deskew\n"
	},
	{0x00480003,
	 "PMU0: db%d l%d, inc PBD = %d\n"
	},
	{0x00490003,
	 "PMU1: Running lane deskew on pstate %d csn %d rdDBIEn %d\n"
	},
	{0x004a0000,
	 "PMU: Error: Read deskew training has been requested, but csrMajorModeDbyte[2] is set\n"
	},
	{0x004b0002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x004c0002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x004d0001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D3U Type\n"
	},
	{0x004e0001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D3R Type\n"
	},
	{0x004f0001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4U Type\n"
	},
	{0x00500001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4R Type\n"
	},
	{0x00510001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4LR Type\n"
	},
	{0x00520000,
	 "PMU: Error: Both 2t timing mode and ddr4 geardown mode specified in the messageblock's PhyCfg and MR3 fields. Only one can be enabled\n"
	},
	{0x00530003,
	 "PMU10: PHY TOTALS - NUM_DBYTES %d NUM_NIBBLES %d NUM_ANIBS %d\n"
	},
	{0x00540006,
	 "PMU10: CSA=0x%02x, CSB=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, MMISC=%d DRAMFreq=%dMT DramType=LPDDR3\n"
	},
	{0x00550006,
	 "PMU10: CSA=0x%02x, CSB=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, MMISC=%d DRAMFreq=%dMT DramType=LPDDR4\n"
	},
	{0x00560008,
	 "PMU10: CS=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, 2T=%d, MMISC=%d AddrMirror=%d DRAMFreq=%dMT DramType=%d\n"
	},
	{0x00570004,
	 "PMU10: Pstate%d MR0=0x%04x MR1=0x%04x MR2=0x%04x\n"
	},
	{0x00580008,
	 "PMU10: Pstate%d MRS MR0=0x%04x MR1=0x%04x MR2=0x%04x MR3=0x%04x MR4=0x%04x MR5=0x%04x MR6=0x%04x\n"
	},
	{0x00590005,
	 "PMU10: Pstate%d MRS MR1_A0=0x%04x MR2_A0=0x%04x MR3_A0=0x%04x MR11_A0=0x%04x\n"
	},
	{0x005a0000,
	 "PMU10: UseBroadcastMR set. All ranks and channels use MRXX_A0 for MR settings.\n"
	},
	{0x005b0005,
	 "PMU10: Pstate%d MRS MR01_A0=0x%02x MR02_A0=0x%02x MR03_A0=0x%02x MR11_A0=0x%02x\n"
	},
	{0x005c0005,
	 "PMU10: Pstate%d MRS MR12_A0=0x%02x MR13_A0=0x%02x MR14_A0=0x%02x MR22_A0=0x%02x\n"
	},
	{0x005d0005,
	 "PMU10: Pstate%d MRS MR01_A1=0x%02x MR02_A1=0x%02x MR03_A1=0x%02x MR11_A1=0x%02x\n"
	},
	{0x005e0005,
	 "PMU10: Pstate%d MRS MR12_A1=0x%02x MR13_A1=0x%02x MR14_A1=0x%02x MR22_A1=0x%02x\n"
	},
	{0x005f0005,
	 "PMU10: Pstate%d MRS MR01_B0=0x%02x MR02_B0=0x%02x MR03_B0=0x%02x MR11_B0=0x%02x\n"
	},
	{0x00600005,
	 "PMU10: Pstate%d MRS MR12_B0=0x%02x MR13_B0=0x%02x MR14_B0=0x%02x MR22_B0=0x%02x\n"
	},
	{0x00610005,
	 "PMU10: Pstate%d MRS MR01_B1=0x%02x MR02_B1=0x%02x MR03_B1=0x%02x MR11_B1=0x%02x\n"
	},
	{0x00620005,
	 "PMU10: Pstate%d MRS MR12_B1=0x%02x MR13_B1=0x%02x MR14_B1=0x%02x MR22_B1=0x%02x\n"
	},
	{0x00630002,
	 "PMU1: AcsmOdtCtrl%02d 0x%02x\n"
	},
	{0x00640002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00650002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00660000,
	 "PMU1: HwtCAMode set\n"
	},
	{0x00670001,
	 "PMU3: DDR4 infinite preamble enter/exit mode %d\n"
	},
	{0x00680002,
	 "PMU1: In rxenb_train() csn=%d pstate=%d\n"
	},
	{0x00690000,
	 "PMU3: Finding DQS falling edge\n"
	},
	{0x006a0000,
	 "PMU3: Searching for DDR3/LPDDR3/LPDDR4 read preamble\n"
	},
	{0x006b0009,
	 "PMU3: dtsm fails Even Nibbles : %2x %2x %2x %2x %2x %2x %2x %2x %2x\n"
	},
	{0x006c0009,
	 "PMU3: dtsm fails Odd  Nibbles : %2x %2x %2x %2x %2x %2x %2x %2x %2x\n"
	},
	{0x006d0002,
	 "PMU3: Preamble search pass=%d anyfail=%d\n"
	},
	{0x006e0000,
	 "PMU: Error: RxEn training preamble not found\n"
	},
	{0x006f0000,
	 "PMU3: Found DQS pre-amble\n"
	},
	{0x00700001,
	 "PMU: Error: Dbyte %d couldn't find the rising edge of DQS during RxEn Training\n"
	},
	{0x00710000,
	 "PMU3: RxEn aligning to first rising edge of burst\n"
	},
	{0x00720001,
	 "PMU3: Decreasing RxEn delay by %d fine step to allow full capture of reads\n"
	},
	{0x00730001,
	 "PMU3: MREP Delay = %d\n"
	},
	{0x00740003,
	 "PMU3: Errcnt for MREP nib %2d delay = %2d is %d\n"
	},
	{0x00750002,
	 "PMU3: MREP nibble %d sampled a 1 at data buffer delay %d\n"
	},
	{0x00760002,
	 "PMU3: MREP nibble %d saw a 0 to 1 transition at data buffer delay %d\n"
	},
	{0x00770000,
	 "PMU2:  MREP did not find a 0 to 1 transition for all nibbles. Failing nibbles assumed to have rising edge close to fine delay 63\n"
	},
	{0x00780002,
	 "PMU2:  Rising edge found in alias window, setting rxDly for nibble %d = %d\n"
	},
	{0x00790002,
	 "PMU: Error: Failed MREP for nib %d with %d one\n"
	},
	{0x007a0003,
	 "PMU2:  Rising edge not found in alias window with %d one, leaving rxDly for nibble %d = %d\n"
	},
	{0x007b0002,
	 "PMU3: Training DIMM %d CSn %d\n"
	},
	{0x007c0001,
	 "PMU3: exitCAtrain_lp3 cs 0x%x\n"
	},
	{0x007d0001,
	 "PMU3: enterCAtrain_lp3 cs 0x%x\n"
	},
	{0x007e0001,
	 "PMU3: CAtrain_switchmsb_lp3 cs 0x%x\n"
	},
	{0x007f0001,
	 "PMU3: CATrain_rdwr_lp3 looking for pattern %x\n"
	},
	{0x00800000,
	 "PMU3: exitCAtrain_lp4\n"
	},
	{0x00810001,
	 "PMU3: DEBUG enterCAtrain_lp4 1: cs 0x%x\n"
	},
	{0x00820001,
	 "PMU3: DEBUG enterCAtrain_lp4 3: Put dbyte %d in async mode\n"
	},
	{0x00830000,
	 "PMU3: DEBUG enterCAtrain_lp4 5: Send MR13 to turn on CA training\n"
	},
	{0x00840003,
	 "PMU3: DEBUG enterCAtrain_lp4 7: idx = %d vref = %x mr12 = %x\n"
	},
	{0x00850001,
	 "PMU3: CATrain_rdwr_lp4 looking for pattern %x\n"
	},
	{0x00860004,
	 "PMU3: Phase %d CAreadbackA db:%d %x xo:%x\n"
	},
	{0x00870005,
	 "PMU3: DEBUG lp4SetCatrVref 1: cs=%d chan=%d mr12=%x vref=%d.%d%%\n"
	},
	{0x00880003,
	 "PMU3: DEBUG lp4SetCatrVref 3: mr12 = %x send vref= %x to db=%d\n"
	},
	{0x00890000,
	 "PMU10:Optimizing vref\n"
	},
	{0x008a0004,
	 "PMU4:mr12:%2x cs:%d chan %d r:%4x\n"
	},
	{0x008b0005,
	 "PMU3: i:%2d bstr:%2d bsto:%2d st:%d r:%d\n"
	},
	{0x008c0002,
	 "Failed to find sufficient CA Vref Passing Region for CS %d ch. %d\n"
	},
	{0x008d0005,
	 "PMU3:Found %d.%d%% MR12:%x for cs:%d chan %d\n"
	},
	{0x008e0002,
	 "PMU3:Calculated %d for AtxImpedence from acx %d.\n"
	},
	{0x008f0000,
	 "PMU3:CA Odt impedence ==0.  Use default vref.\n"
	},
	{0x00900003,
	 "PMU3:Calculated %d.%d%% for Vref MR12=0x%x.\n"
	},
	{0x00910000,
	 "PMU3: CAtrain_lp\n"
	},
	{0x00920000,
	 "PMU3: CAtrain Begins.\n"
	},
	{0x00930001,
	 "PMU3: CAtrain_lp testing dly %d\n"
	},
	{0x00940001,
	 "PMU5: CA bitmap dump for cs %x\n"
	},
	{0x00950001,
	 "PMU5: CAA%d "
	},
	{0x00960001, "%02x"
	},
	{0x00970000, "\n"
	},
	{0x00980001,
	 "PMU5: CAB%d "
	},
	{0x00990001, "%02x"
	},
	{0x009a0000, "\n"
	},
	{0x009b0003,
	 "PMU3: anibi=%d, anibichan[anibi]=%d ,chan=%d\n"
	},
	{0x009c0001, "%02x"
	},
	{0x009d0001, "\nPMU3:Raw CA setting :%x"
	},
	{0x009e0002, "\nPMU3:ATxDly setting:%x margin:%d\n"
	},
	{0x009f0002, "\nPMU3:InvClk ATxDly setting:%x margin:%d\n"
	},
	{0x00a00000, "\nPMU3:No Range found!\n"
	},
	{0x00a10003,
	 "PMU3: 2 anibi=%d, anibichan[anibi]=%d ,chan=%d"
	},
	{0x00a20002, "\nPMU3: no neg clock => CA setting anib=%d, :%d\n"
	},
	{0x00a30001,
	 "PMU3:Normal margin:%d\n"
	},
	{0x00a40001,
	 "PMU3:Inverted margin:%d\n"
	},
	{0x00a50000,
	 "PMU3:Using Inverted clock\n"
	},
	{0x00a60000,
	 "PMU3:Using normal clk\n"
	},
	{0x00a70003,
	 "PMU3: 3 anibi=%d, anibichan[anibi]=%d ,chan=%d\n"
	},
	{0x00a80002,
	 "PMU3: Setting ATxDly for anib %x to %x\n"
	},
	{0x00a90000,
	 "PMU: Error: CA Training Failed.\n"
	},
	{0x00aa0000,
	 "PMU1: Writing MRs\n"
	},
	{0x00ab0000,
	 "PMU4:Using MR12 values from 1D CA VREF training.\n"
	},
	{0x00ac0000,
	 "PMU3:Writing all MRs to fsp 1\n"
	},
	{0x00ad0000,
	 "PMU10:Lp4Quickboot mode.\n"
	},
	{0x00ae0000,
	 "PMU3: Writing MRs\n"
	},
	{0x00af0001,
	 "PMU10: Setting boot clock divider to %d\n"
	},
	{0x00b00000,
	 "PMU3: Resetting DRAM\n"
	},
	{0x00b10000,
	 "PMU3: setup for RCD initalization\n"
	},
	{0x00b20000,
	 "PMU3: pmu_exit_SR from dev_init()\n"
	},
	{0x00b30000,
	 "PMU3: initializing RCD\n"
	},
	{0x00b40000,
	 "PMU10: **** Executing 2D Image ****\n"
	},
	{0x00b50001,
	 "PMU10: **** Start DDR4 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x00b60001,
	 "PMU10: **** Start DDR3 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x00b70001,
	 "PMU10: **** Start LPDDR3 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x00b80001,
	 "PMU10: **** Start LPDDR4 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x00b90000,
	 "PMU: Error: Mismatched internal revision between DCCM and ICCM images\n"
	},
	{0x00ba0001,
	 "PMU10: **** Testchip %d Specific Firmware ****\n"
	},
	{0x00bb0000,
	 "PMU1: LRDIMM with EncodedCS mode, one DIMM\n"
	},
	{0x00bc0000,
	 "PMU1: LRDIMM with EncodedCS mode, two DIMMs\n"
	},
	{0x00bd0000,
	 "PMU1: RDIMM with EncodedCS mode, one DIMM\n"
	},
	{0x00be0000,
	 "PMU2: Starting LRDIMM MREP training for all ranks\n"
	},
	{0x00bf0000,
	 "PMU199: LRDIMM MREP training for all ranks completed\n"
	},
	{0x00c00000,
	 "PMU2: Starting LRDIMM DWL training for all ranks\n"
	},
	{0x00c10000,
	 "PMU199: LRDIMM DWL training for all ranks completed\n"
	},
	{0x00c20000,
	 "PMU2: Starting LRDIMM MRD training for all ranks\n"
	},
	{0x00c30000,
	 "PMU199: LRDIMM MRD training for all ranks completed\n"
	},
	{0x00c40000,
	 "PMU2: Starting RXEN training for all ranks\n"
	},
	{0x00c50000,
	 "PMU2: Starting write leveling fine delay training for all ranks\n"
	},
	{0x00c60000,
	 "PMU2: Starting LRDIMM MWD training for all ranks\n"
	},
	{0x00c70000,
	 "PMU199: LRDIMM MWD training for all ranks completed\n"
	},
	{0x00c80000,
	 "PMU2: Starting write leveling fine delay training for all ranks\n"
	},
	{0x00c90000,
	 "PMU2: Starting read deskew training\n"
	},
	{0x00ca0000,
	 "PMU2: Starting SI friendly 1d RdDqs training for all ranks\n"
	},
	{0x00cb0000,
	 "PMU2: Starting write leveling coarse delay training for all ranks\n"
	},
	{0x00cc0000,
	 "PMU2: Starting 1d WrDq training for all ranks\n"
	},
	{0x00cd0000,
	 "PMU2: Running DQS2DQ Oscillator for all ranks\n"
	},
	{0x00ce0000,
	 "PMU2: Starting again read deskew training but with PRBS\n"
	},
	{0x00cf0000,
	 "PMU2: Starting 1d RdDqs training for all ranks\n"
	},
	{0x00d00000,
	 "PMU2: Starting again 1d WrDq training for all ranks\n"
	},
	{0x00d10000,
	 "PMU2: Starting MaxRdLat training\n"
	},
	{0x00d20000,
	 "PMU2: Starting 2d WrDq training for all ranks\n"
	},
	{0x00d30000,
	 "PMU2: Starting 2d RdDqs training for all ranks\n"
	},
	{0x00d40002,
	 "PMU3:read_fifo %x %x\n"
	},
	{0x00d50001,
	 "PMU: Error: Invalid PhyDrvImpedance of 0x%x specified in message block.\n"
	},
	{0x00d60001,
	 "PMU: Error: Invalid PhyOdtImpedance of 0x%x specified in message block.\n"
	},
	{0x00d70001,
	 "PMU: Error: Invalid BPZNResVal of 0x%x specified in message block.\n"
	},
	{0x00d80005,
	 "PMU3: fixRxEnBackOff csn:%d db:%d dn:%d bo:%d dly:%x\n"
	},
	{0x00d90001,
	 "PMU3: fixRxEnBackOff dly:%x\n"
	},
	{0x00da0000,
	 "PMU3: Entering setupPpt\n"
	},
	{0x00db0000,
	 "PMU3: Start lp4PopulateHighLowBytes\n"
	},
	{0x00dc0002,
	 "PMU3:Dbyte Detect: db%d received %x\n"
	},
	{0x00dd0002,
	 "PMU3:getDqs2Dq read %x from dbyte %d\n"
	},
	{0x00de0002,
	 "PMU3:getDqs2Dq(2) read %x from dbyte %d\n"
	},
	{0x00df0001,
	 "PMU: Error: Dbyte %d read 0 from the DQS oscillator it is connected to\n"
	},
	{0x00e00002,
	 "PMU4: Dbyte %d dqs2dq = %d/32 UI\n"
	},
	{0x00e10003,
	 "PMU3:getDqs2Dq set dqs2dq:%d/32 ui (%d ps) from dbyte %d\n"
	},
	{0x00e20003,
	 "PMU3: Setting coarse delay in AtxDly chiplet %d from 0x%02x to 0x%02x\n"
	},
	{0x00e30003,
	 "PMU3: Clearing coarse delay in AtxDly chiplet %d from 0x%02x to 0x%02x\n"
	},
	{0x00e40000,
	 "PMU3: Performing DDR4 geardown sync sequence\n"
	},
	{0x00e50000,
	 "PMU1: Enter self refresh\n"
	},
	{0x00e60000,
	 "PMU1: Exit self refresh\n"
	},
	{0x00e70000,
	 "PMU: Error: No dbiEnable with lp4\n"
	},
	{0x00e80000,
	 "PMU: Error: No dbiDisable with lp4\n"
	},
	{0x00e90001,
	 "PMU1: DDR4 update Rx DBI Setting disable %d\n"
	},
	{0x00ea0001,
	 "PMU1: DDR4 update 2nCk WPre Setting disable %d\n"
	},
	{0x00eb0005,
	 "PMU1: read_delay: db%d lane%d delays[%2d] = 0x%02x (max 0x%02x)\n"
	},
	{0x00ec0004,
	 "PMU1: write_delay: db%d lane%d delays[%2d] = 0x%04x\n"
	},
	{0x00ed0001,
	 "PMU5: ID=%d -- db0  db1  db2  db3  db4  db5  db6  db7  db8  db9 --\n"
	},
	{0x00ee000b,
	 "PMU5: [%d]:0x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n"
	},
	{0x00ef0003,
	 "PMU2: dump delays - pstate=%d dimm=%d csn=%d\n"
	},
	{0x00f00000,
	 "PMU3: Printing Mid-Training Delay Information\n"
	},
	{0x00f10001,
	 "PMU5: CS%d <<KEY>> 0 TrainingCntr <<KEY>> coarse(15:10) fine(9:0)\n"
	},
	{0x00f20001,
	 "PMU5: CS%d <<KEY>> 0 RxEnDly, 1 RxClkDly <<KEY>> coarse(10:6) fine(5:0)\n"
	},
	{0x00f30001,
	 "PMU5: CS%d <<KEY>> 0 TxDqsDly, 1 TxDqDly <<KEY>> coarse(9:6) fine(5:0)\n"
	},
	{0x00f40001,
	 "PMU5: CS%d <<KEY>> 0 RxPBDly <<KEY>> 1 Delay Unit ~= 7ps\n"
	},
	{0x00f50000,
	 "PMU5: all CS <<KEY>> 0 DFIMRL <<KEY>> Units = DFI clocks\n"
	},
	{0x00f60000,
	 "PMU5: all CS <<KEY>> VrefDACs <<KEY>> DAC(6:0)\n"
	},
	{0x00f70000,
	 "PMU1: Set DMD in MR13 and wrDBI in MR3 for training\n"
	},
	{0x00f80000,
	 "PMU: Error: getMaxRxen() failed to find largest rxen nibble delay\n"
	},
	{0x00f90003,
	 "PMU2: getMaxRxen(): maxDly %d maxTg %d maxNib %d\n"
	},
	{0x00fa0003,
	 "PMU2: getRankMaxRxen(): maxDly %d Tg %d maxNib %d\n"
	},
	{0x00fb0000,
	 "PMU1: skipping CDD calculation in 2D image\n"
	},
	{0x00fc0001,
	 "PMU3: Calculating CDDs for pstate %d\n"
	},
	{0x00fd0003,
	 "PMU3: rxFromDly[%d][%d] = %d\n"
	},
	{0x00fe0003,
	 "PMU3: rxToDly  [%d][%d] = %d\n"
	},
	{0x00ff0003,
	 "PMU3: rxDly    [%d][%d] = %d\n"
	},
	{0x01000003,
	 "PMU3: txDly    [%d][%d] = %d\n"
	},
	{0x01010003,
	 "PMU3: allFine CDD_RR_%d_%d = %d\n"
	},
	{0x01020003,
	 "PMU3: allFine CDD_WW_%d_%d = %d\n"
	},
	{0x01030003,
	 "PMU3: CDD_RR_%d_%d = %d\n"
	},
	{0x01040003,
	 "PMU3: CDD_WW_%d_%d = %d\n"
	},
	{0x01050003,
	 "PMU3: allFine CDD_RW_%d_%d = %d\n"
	},
	{0x01060003,
	 "PMU3: allFine CDD_WR_%d_%d = %d\n"
	},
	{0x01070003,
	 "PMU3: CDD_RW_%d_%d = %d\n"
	},
	{0x01080003,
	 "PMU3: CDD_WR_%d_%d = %d\n"
	},
	{0x01090004,
	 "PMU3: F%dBC2x_B%d_D%d = 0x%02x\n"
	},
	{0x010a0004,
	 "PMU3: F%dBC3x_B%d_D%d = 0x%02x\n"
	},
	{0x010b0004,
	 "PMU3: F%dBC4x_B%d_D%d = 0x%02x\n"
	},
	{0x010c0004,
	 "PMU3: F%dBC5x_B%d_D%d = 0x%02x\n"
	},
	{0x010d0004,
	 "PMU3: F%dBC8x_B%d_D%d = 0x%02x\n"
	},
	{0x010e0004,
	 "PMU3: F%dBC9x_B%d_D%d = 0x%02x\n"
	},
	{0x010f0004,
	 "PMU3: F%dBCAx_B%d_D%d = 0x%02x\n"
	},
	{0x01100004,
	 "PMU3: F%dBCBx_B%d_D%d = 0x%02x\n"
	},
	{0x01110000,
	 "PMU10: Entering context_switch_postamble\n"
	},
	{0x01120003,
	 "PMU10: context_switch_postamble is enabled for DIMM %d, RC0A=0x%x, RC3x=0x%x\n"
	},
	{0x01130000,
	 "PMU10: Setting bcw fspace 0\n"
	},
	{0x01140001,
	 "PMU10: Sending BC0A = 0x%x\n"
	},
	{0x01150001,
	 "PMU10: Sending BC6x = 0x%x\n"
	},
	{0x01160001,
	 "PMU10: Sending RC0A = 0x%x\n"
	},
	{0x01170001,
	 "PMU10: Sending RC3x = 0x%x\n"
	},
	{0x01180001,
	 "PMU10: Sending RC0A = 0x%x\n"
	},
	{0x01190001,
	 "PMU1: enter_lp3: DEBUG: pstate = %d\n"
	},
	{0x011a0001,
	 "PMU1: enter_lp3: DEBUG: dfifreqxlat_pstate = %d\n"
	},
	{0x011b0001,
	 "PMU1: enter_lp3: DEBUG: pllbypass = %d\n"
	},
	{0x011c0001,
	 "PMU1: enter_lp3: DEBUG: forcecal = %d\n"
	},
	{0x011d0001,
	 "PMU1: enter_lp3: DEBUG: pllmaxrange = 0x%x\n"
	},
	{0x011e0001,
	 "PMU1: enter_lp3: DEBUG: dacval_out = 0x%x\n"
	},
	{0x011f0001,
	 "PMU1: enter_lp3: DEBUG: pllctrl3 = 0x%x\n"
	},
	{0x01200000,
	 "PMU3: Loading DRAM with BIOS supplied MR values and entering self refresh prior to exiting PMU code.\n"
	},
	{0x01210002,
	 "PMU3: Setting DataBuffer function space of dimmcs 0x%02x to %d\n"
	},
	{0x01220002,
	 "PMU4: Setting RCW FxRC%Xx = 0x%02x\n"
	},
	{0x01230002,
	 "PMU4: Setting RCW FxRC%02x = 0x%02x\n"
	},
	{0x01240001,
	 "PMU1: DDR4 update Rd Pre Setting disable %d\n"
	},
	{0x01250002,
	 "PMU2: Setting BCW FxBC%Xx = 0x%02x\n"
	},
	{0x01260002,
	 "PMU2: Setting BCW BC%02x = 0x%02x\n"
	},
	{0x01270002,
	 "PMU2: Setting BCW PBA mode FxBC%Xx = 0x%02x\n"
	},
	{0x01280002,
	 "PMU2: Setting BCW PBA mode BC%02x = 0x%02x\n"
	},
	{0x01290003,
	 "PMU4: BCW value for dimm %d, fspace %d, addr 0x%04x\n"
	},
	{0x012a0002,
	 "PMU4: DB %d, value 0x%02x\n"
	},
	{0x012b0000,
	 "PMU6: WARNING MREP underflow, set to min value -2 coarse, 0 fine\n"
	},
	{0x012c0004,
	 "PMU6: LRDIMM Writing final data buffer fine delay value nib %2d, trainDly %3d, fineDly code %2d, new MREP fine %2d\n"
	},
	{0x012d0003,
	 "PMU6: LRDIMM Writing final data buffer fine delay value nib %2d, trainDly %3d, fineDly code %2d\n"
	},
	{0x012e0003,
	 "PMU6: LRDIMM Writing data buffer fine delay type %d nib %2d, code %2d\n"
	},
	{0x012f0002,
	 "PMU6: Writing final data buffer coarse delay value dbyte %2d, coarse = 0x%02x\n"
	},
	{0x01300003,
	 "PMU4: data 0x%04x at MB addr 0x%08x saved at CSR addr 0x%08x\n"
	},
	{0x01310003,
	 "PMU4: data 0x%04x at MB addr 0x%08x restored from CSR addr 0x%08x\n"
	},
	{0x01320003,
	 "PMU4: data 0x%04x at MB addr 0x%08x saved at CSR addr 0x%08x\n"
	},
	{0x01330003,
	 "PMU4: data 0x%04x at MB addr 0x%08x restored from CSR addr 0x%08x\n"
	},
	{0x01340001,
	 "PMU3: Update BC00, BC01, BC02 for rank-dimm 0x%02x\n"
	},
	{0x01350000,
	 "PMU3: Writing D4 RDIMM RCD Control words F0RC00 -> F0RC0F\n"
	},
	{0x01360000,
	 "PMU3: Disable parity in F0RC0E\n"
	},
	{0x01370000,
	 "PMU3: Writing D4 RDIMM RCD Control words F1RC00 -> F1RC05\n"
	},
	{0x01380000,
	 "PMU3: Writing D4 RDIMM RCD Control words F1RC1x -> F1RC9x\n"
	},
	{0x01390000,
	 "PMU3: Writing D4 Data buffer Control words BC00 -> BC0E\n"
	},
	{0x013a0002,
	 "PMU1: setAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x013b0002,
	 "PMU1: restoreFromAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x013c0002,
	 "PMU1: restoreAcsmFromAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x013d0002,
	 "PMU2: Setting D3R RC%d = 0x%01x\n"
	},
	{0x013e0000,
	 "PMU3: Writing D3 RDIMM RCD Control words RC0 -> RC11\n"
	},
	{0x013f0002,
	 "PMU0: VrefDAC0/1 vddqStart %d dacToVddq %d\n"
	},
	{0x01400001,
	 "PMU: Error: Messageblock phyVref=0x%x is above the limit for TSMC28's attenuated LPDDR4 receivers. Please see the pub databook\n"
	},
	{0x01410001,
	 "PMU: Error: Messageblock phyVref=0x%x is above the limit for TSMC28's attenuated DDR4 receivers. Please see the pub databook\n"
	},
	{0x01420001,
	 "PMU0: PHY VREF @ (%d/1000) VDDQ\n"
	},
	{0x01430002,
	 "PMU0: initalizing phy vrefDacs to %d ExtVrefRange %x\n"
	},
	{0x01440002,
	 "PMU0: initalizing global vref to %d range %d\n"
	},
	{0x01450002,
	 "PMU4: Setting initial device vrefDQ for CS%d to MR6 = 0x%04x\n"
	},
	{0x01460003,
	 "PMU1: In write_level_fine() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01470000,
	 "PMU3: Fine write leveling hardware search increasing TxDqsDly until full bursts are seen\n"
	},
	{0x01480000,
	 "PMU4: WL normalized pos   : ........................|........................\n"
	},
	{0x01490007,
	 "PMU4: WL margin for nib %2d: %08x%08x%08x%08x%08x%08x\n"
	},
	{0x014a0000,
	 "PMU4: WL normalized pos   : ........................|........................\n"
	},
	{0x014b0000,
	 "PMU3: Exiting write leveling mode\n"
	},
	{0x014c0001,
	 "PMU3: got %d for cl in load_wrlvl_acsm\n"
	},
	{0x014d0003,
	 "PMU1: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x014e0003,
	 "PMU3: left eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x014f0003,
	 "PMU3: right eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x01500004,
	 "PMU3: eye center db:%d ln:%d dly:0x%x (maxdq:%x)\n"
	},
	{0x01510003,
	 "PMU3: Wrote to TxDqDly db:%d ln:%d dly:0x%x\n"
	},
	{0x01520003,
	 "PMU3: Wrote to TxDqDly db:%d ln:%d dly:0x%x\n"
	},
	{0x01530002,
	 "PMU3: Coarse write leveling dbyte%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x01540002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x01550000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x01560001,
	 "PMU3: got %d for cl in load_wrlvl_acsm\n"
	},
	{0x01570003,
	 "PMU3: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01580003,
	 "PMU3: left eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x01590003,
	 "PMU3: right eye edge search db: %d ln: %d dly: 0x%x\n"
	},
	{0x015a0004,
	 "PMU3: eye center db: %d ln: %d dly: 0x%x (maxdq: 0x%x)\n"
	},
	{0x015b0003,
	 "PMU3: Wrote to TxDqDly db: %d ln: %d dly: 0x%x\n"
	},
	{0x015c0003,
	 "PMU3: Wrote to TxDqDly db: %d ln: %d dly: 0x%x\n"
	},
	{0x015d0002,
	 "PMU3: Coarse write leveling nibble%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x015e0002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x015f0000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x01600000,
	 "PMU4: WL normalized pos   : ................................|................................\n"
	},
	{0x01610009,
	 "PMU4: WL margin for nib %2d: %08x%08x%08x%08x%08x%08x%08x%08x\n"
	},
	{0x01620000,
	 "PMU4: WL normalized pos   : ................................|................................\n"
	},
	{0x01630001,
	 "PMU8: Adjust margin after WL coarse to be larger than %d\n"
	},
	{0x01640001,
	 "PMU: Error: All margin after write leveling coarse are smaller than minMargin %d\n"
	},
	{0x01650002,
	 "PMU8: Decrement nib %d TxDqsDly by %d fine step\n"
	},
	{0x01660003,
	 "PMU3: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01670005,
	 "PMU2: Write level: dbyte %d nib%d dq/dmbi %2d dqsfine 0x%04x dqDly 0x%04x\n"
	},
	{0x01680002,
	 "PMU3: Coarse write leveling nibble%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x01690002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x016a0000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x016b0001,
	 "PMU3: DWL delay = %d\n"
	},
	{0x016c0003,
	 "PMU3: Errcnt for DWL nib %2d delay = %2d is %d\n"
	},
	{0x016d0002,
	 "PMU3: DWL nibble %d sampled a 1 at delay %d\n"
	},
	{0x016e0003,
	 "PMU3: DWL nibble %d passed at delay %d. Rising edge was at %d\n"
	},
	{0x016f0000,
	 "PMU2: DWL did nto find a rising edge of memclk for all nibbles. Failing nibbles assumed to have rising edge close to fine delay 63\n"
	},
	{0x01700002,
	 "PMU2:  Rising edge found in alias window, setting wrlvlDly for nibble %d = %d\n"
	},
	{0x01710002,
	 "PMU: Error: Failed DWL for nib %d with %d one\n"
	},
	{0x01720003,
	 "PMU2:  Rising edge not found in alias window with %d one, leaving wrlvlDly for nibble %d = %d\n"
	},
	{0x04000000,
	 "PMU: Error:Mailbox Buffer Overflowed.\n"
	},
	{0x04010000,
	 "PMU: Error:Mailbox Buffer Overflowed.\n"
	},
	{0x04020000,
	 "PMU: ***** Assertion Error - terminating *****\n"
	},
	{0x04030002,
	 "PMU1: swapByte db %d by %d\n"
	},
	{0x04040003,
	 "PMU3: get_cmd_dly max(%d ps, %d memclk) = %d\n"
	},
	{0x04050002,
	 "PMU0: Write CSR 0x%06x 0x%04x\n"
	},
	{0x04060002,
	 "PMU0: hwt_init_ppgc_prbs(): Polynomial: %x, Deg: %d\n"
	},
	{0x04070001,
	 "PMU: Error: acsm_set_cmd to non existent instruction address %d\n"
	},
	{0x04080001,
	 "PMU: Error: acsm_set_cmd with unknown ddr cmd 0x%x\n"
	},
	{0x0409000c,
	 "PMU1: acsm_addr %02x, acsm_flgs %04x, ddr_cmd %02x, cmd_dly %02x, ddr_addr %04x, ddr_bnk %02x, ddr_cs %02x, cmd_rcnt %02x, AcsmSeq0/1/2/3 %04x %04x %04x %04x\n"
	},
	{0x040a0000,
	 "PMU: Error: Polling on ACSM done failed to complete in acsm_poll_done()...\n"
	},
	{0x040b0000,
	 "PMU1: acsm RUN\n"
	},
	{0x040c0000,
	 "PMU1: acsm STOPPED\n"
	},
	{0x040d0002,
	 "PMU1: acsm_init: acsm_mode %04x mxrdlat %04x\n"
	},
	{0x040e0002,
	 "PMU: Error: setAcsmCLCWL: cl and cwl must be each >= 2 and 5, resp. CL=%d CWL=%d\n"
	},
	{0x040f0002,
	 "PMU: Error: setAcsmCLCWL: cl and cwl must be each >= 5. CL=%d CWL=%d\n"
	},
	{0x04100002,
	 "PMU1: setAcsmCLCWL: CASL %04d WCASL %04d\n"
	},
	{0x04110001,
	 "PMU: Error: Reserved value of register F0RC0F found in message block: 0x%04x\n"
	},
	{0x04120001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x04130001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x04140000,
	 "PMU3: Entering Boot Freq Mode.\n"
	},
	{0x04150001,
	 "PMU: Error: Boot clock divider setting of %d is too small\n"
	},
	{0x04160000,
	 "PMU3: Exiting Boot Freq Mode.\n"
	},
	{0x04170002,
	 "PMU3: Writing MR%d OP=%x\n"
	},
	{0x04180000,
	 "PMU: Error: Delay too large in slomo\n"
	},
	{0x04190001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x041a0000,
	 "PMU3: Enable Channel A\n"
	},
	{0x041b0000,
	 "PMU3: Enable Channel B\n"
	},
	{0x041c0000,
	 "PMU3: Enable All Channels\n"
	},
	{0x041d0002,
	 "PMU2: Use PDA mode to set MR%d with value 0x%02x\n"
	},
	{0x041e0001,
	 "PMU3: Written Vref with PDA to CS=0x%02x\n"
	},
	{0x041f0000,
	 "PMU1: start_cal: DEBUG: setting CalRun to 1\n"
	},
	{0x04200000,
	 "PMU1: start_cal: DEBUG: setting CalRun to 0\n"
	},
	{0x04210001,
	 "PMU1: lock_pll_dll: DEBUG: pstate = %d\n"
	},
	{0x04220001,
	 "PMU1: lock_pll_dll: DEBUG: dfifreqxlat_pstate = %d\n"
	},
	{0x04230001,
	 "PMU1: lock_pll_dll: DEBUG: pllbypass = %d\n"
	},
	{0x04240001,
	 "PMU3: SaveLcdlSeed: Saving seed %d\n"
	},
	{0x04250000,
	 "PMU1: in phy_defaults()\n"
	},
	{0x04260003,
	 "PMU3: ACXConf:%d MaxNumDbytes:%d NumDfi:%d\n"
	},
	{0x04270005,
	 "PMU1: setAltAcsmCLCWL setting cl=%d cwl=%d\n"
	},
};

static const struct phy_msg messages_2d[] = {
	{0x00000001,
	 "PMU0: Converting %d into an MR\n"
	},
	{0x00010003,
	 "PMU DEBUG: vref_idx %d -= %d, range_idx = %d\n"
	},
	{0x00020002,
	 "PMU0: vrefIdx. Passing range %d, remaining vrefidx = %d\n"
	},
	{0x00030002,
	 "PMU0: VrefIdx %d -> MR[6:0] 0x%02x\n"
	},
	{0x00040001,
	 "PMU0: Converting MR 0x%04x to vrefIdx\n"
	},
	{0x00050002,
	 "PMU0: DAC %d Range %d\n"
	},
	{0x00060003,
	 "PMU0: Range %d, Range_idx %d, vref_idx offset %d\n"
	},
	{0x00070002,
	 "PMU0: MR 0x%04x -> VrefIdx %d\n"
	},
	{0x00080001,
	 "PMU: Error: Illegal timing group number ,%d, in getPtrVrefDq\n"
	},
	{0x00090003,
	 "PMU1: VrefDqR%dNib%d = %d\n"
	},
	{0x000a0003,
	 "PMU0: VrefDqR%dNib%d = %d\n"
	},
	{0x000b0000,
	 "PMU0: ----------------MARGINS-------\n"
	},
	{0x000c0002,
	 "PMU0: R%d_RxClkDly_Margin = %d\n"
	},
	{0x000d0002,
	 "PMU0: R%d_VrefDac_Margin = %d\n"
	},
	{0x000e0002,
	 "PMU0: R%d_TxDqDly_Margin = %d\n"
	},
	{0x000f0002,
	 "PMU0: R%d_DeviceVref_Margin = %d\n"
	},
	{0x00100000,
	 "PMU0: -----------------------\n"
	},
	{0x00110003,
	 "PMU0: eye %d's for all TG's is [%d ... %d]\n"
	},
	{0x00120000,
	 "PMU0: ------- settingWeight -----\n"
	},
	{0x00130002,
	 "PMU0: Weight %d @ Setting %d\n"
	},
	{0x0014001f,
	 "PMU4: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d >%3d< %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n"
	},
	{0x00150002,
	 "PMU3: Voltage Range = [%d, %d]\n"
	},
	{0x00160004,
	 "PMU4: -- DB%d L%d -- centers: delay = %d, voltage = %d\n"
	},
	{0x00170001,
	 "PMU5: <<KEY>> 0 TxDqDlyTg%d <<KEY>> coarse(6:6) fine(5:0)\n"
	},
	{0x00180001,
	 "PMU5: <<KEY>> 0 messageBlock VrefDqR%d <<KEY>> MR6(6:0)\n"
	},
	{0x00190001,
	 "PMU5: <<KEY>> 0 RxClkDlyTg%d <<KEY>> fine(5:0)\n"
	},
	{0x001a0003,
	 "PMU0: tgToCsn: tg %d + 0x%04x -> csn %d\n"
	},
	{0x001b0002,
	 "PMU: Error: LP4 rank %d cannot be mapped on tg %d\n"
	},
	{0x001c0002,
	 "PMU3: Sending vref %d,  Mr = 0X%05x, to all devices\n"
	},
	{0x001d0004,
	 "PMU4: -------- %dD Write Scanning TG %d (CS 0x%x) Lanes 0x%03x --------\n"
	},
	{0x001e0002,
	 "PMU0: training lanes 0x%03x using lanes 0x%03x\n"
	},
	{0x001f0003,
	 "PMU4: ------- 2D-DFE Read Scanning TG %d (CS 0x%x) Lanes 0x%03x -------\n"
	},
	{0x00200004,
	 "PMU4: ------- %dD Read Scanning TG %d (CS 0x%x) Lanes 0x%03x -------\n"
	},
	{0x00210003,
	 "PMU4: TG%d MR1[13,6,5]=0x%x MR6[13,9,8]=0x%x\n"
	},
	{0x00220002,
	 "PMU0: training lanes 0x%03x using lanes 0x%03x\n"
	},
	{0x00230003,
	 "PMU4: ------- 2D-DFE Read Scanning TG %d (CS 0x%x) Lanes 0x%03x -------\n"
	},
	{0x00240004,
	 "PMU4: ------- %dD Read Scanning TG %d (CS 0x%x) Lanes 0x%03x -------\n"
	},
	{0x00250002,
	 "PMU0: training lanes 0x%03x using lanes 0x%03x\n"
	},
	{0x00260002,
	 "PMU3: Sending vref %d,  Mr = 0X%05x, to all devices\n"
	},
	{0x00270004,
	 "PMU4: -------- %dD Write Scanning TG %d (CS 0x%x) Lanes 0x%03x --------\n"
	},
	{0x00280001,
	 "PMU0: input %d\n"
	},
	{0x00290002,
	 "PMU4: Programmed Voltage Search Range [%d, %d]\n"
	},
	{0x002a0002,
	 "PMU3: Delay Stepsize = %d Fine, Voltage Stepsize = %d DAC\n"
	},
	{0x002b0002,
	 "PMU4: Delay Weight = %d, Voltage Weight = %d\n"
	},
	{0x002c0003,
	 "PMU0: raw 0x%x allFine %d incDec %d"
	},
	{0x002d0008,
	 "PMU0: db%d l%d, voltage 0x%x (u_r %d) delay 0x%x (u_r %d) - lcdl %d mask 0x%x\n"
	},
	{0x002e0005,
	 "PMU0: DB%d L%d, Eye %d, Seed = (0x%x, 0x%x)\n"
	},
	{0x002f0002,
	 "PMU3: 2D Enables       : %d,                    1,                %d\n"
	},
	{0x00300006,
	 "PMU3: 2D Delay   Ranges: OOPL[0x%04x,0x%04x], IP[0x%04x,0x%04x], OOPR[0x%04x,0x%04x]\n"
	},
	{0x00310002,
	 "PMU3: 2D Voltage Search Range : [%d, %d]\n"
	},
	{0x00320002,
	 "PMU4: Found Voltage Search Range [%d, %d]\n"
	},
	{0x00330002,
	 "PMU0: User Weight = %d, Voltage Weight = %d\n"
	},
	{0x00340005,
	 "PMU0: D(%d,%d) V(%d,%d | %d)\n"
	},
	{0x00350002,
	 "PMU0: Norm Weight = %d, Voltage Weight = %d\n"
	},
	{0x00360002,
	 "PMU0: seed 0 = (%d,%d) (center)\n"
	},
	{0x00370003,
	 "PMU0: seed 1 = (%d,%d).min edge at idx %d\n"
	},
	{0x00380003,
	 "PMU0: seed 2 = (%d,%d) max edge at idx %d\n"
	},
	{0x00390003,
	 "PMU0: Search point %d = (%d,%d)\n"
	},
	{0x003a0005,
	 "PMU0: YMARGIN: ^ %d, - %d, v %d. rate %d = %d\n"
	},
	{0x003b0003,
	 "PMU0: XMARGIN: center %d, edge %d. = %d\n"
	},
	{0x003c0002,
	 "PMU0: ----------- weighting (%d,%d) ----------------\n"
	},
	{0x003d0003,
	 "PMU0: X margin - L %d R %d - Min %d\n"
	},
	{0x003e0003,
	 "PMU0: Y margin - L %d R %d - Min %d\n"
	},
	{0x003f0003,
	 "PMU0: center (%d,%d) weight = %d\n"
	},
	{0x00400003,
	 "PMU4: Eye argest blob area %d from %d to %d\n"
	},
	{0x00410002,
	 "PMU0: Compute centroid min_x %d max_x %d\n"
	},
	{0x00420003,
	 "PMU0: Compute centroid sumLnDlyWidth %d sumLnVrefWidth %d sumLnWidht %d\n"
	},
	{0x00430000,
	 "PMU: Error: No passing region found for 1 or more lanes. Set hdtCtrl=4 to see passing regions\n"
	},
	{0x00440003,
	 "PMU0: Centroid ( %d, %d ) found with sumLnWidht %d\n"
	},
	{0x00450003,
	 "PMU0: Optimal allFine Center ( %d + %d ,%d )\n"
	},
	{0x00460003,
	 "PMU3: point %d starting at (%d,%d)\n"
	},
	{0x00470002,
	 "PMU0: picking left (%d > %d)\n"
	},
	{0x00480002,
	 "PMU0: picking right (%d > %d)\n"
	},
	{0x00490002,
	 "PMU0: picking down (%d > %d)\n"
	},
	{0x004a0002,
	 "PMU0: picking up (%d > %d)\n"
	},
	{0x004b0009,
	 "PMU3: new center @ (%3d, %3d). Moved (%2i, %2i) -- L %d, R %d, C %d, U %d, D %d\n"
	},
	{0x004c0003,
	 "PMU3: cordNum %d imporved %d to %d\n"
	},
	{0x004d0000,
	 "PMU: Error: No passing region found for 1 or more lanes. Set hdtCtrl=4 to see passing regions\n"
	},
	{0x004e0004,
	 "PMU0: Optimal allFine Center ( %d + %d ,%d ), found with weight %d.\n"
	},
	{0x004f0003,
	 "PMU0: merging lanes=%d..%d, centerMerge_t %d\n"
	},
	{0x00500001,
	 "PMU0: laneVal %d is disable\n"
	},
	{0x00510002,
	 "PMU0: checking common center %d against current center %d\n"
	},
	{0x00520001,
	 "PMU: Error: getCompoundEye Called on lane%d eye with non-compatible centers\n"
	},
	{0x00530001,
	 "PMU0: laneItr %d is disable\n"
	},
	{0x00540005,
	 "PMU0: lane %d, data_idx %d, offset_idx %d, = [%d..%d]\n"
	},
	{0x00550003,
	 "PMU0: lane %d, data_idx %d, offset_idx %d, offset_idx out of range!\n"
	},
	{0x00560003,
	 "PMU0: mergeData[%d] = max_v_low %d, min_v_high %d\n"
	},
	{0x00570005,
	 "PMU1: writing merged center (%d,%d) back to dataBlock[%d]. doDelay %d, doVoltage %d\n"
	},
	{0x00580005,
	 "PMU0: applying relative (%i,%i) back to dataBlock[%d]. doDelay %d, doVoltage %d\n"
	},
	{0x00590002,
	 "PMU0: drvstren %x is idx %d in the table\n"
	},
	{0x005a0000,
	 "PMU4: truncating FFE drive strength search range. Out of drive strengths to check.\n"
	},
	{0x005b0002,
	 "PMU5: Weak 1 changed to pull-up %5d ohms, pull-down %5d ohms\n"
	},
	{0x005c0002,
	 "PMU5: Weak 0 changed to pull-up %5d ohms, pull-down %5d ohms\n"
	},
	{0x005d0003,
	 "PMU0: dlyMargin L %02d R %02d, min %02d\n"
	},
	{0x005e0003,
	 "PMU0: vrefMargin T %02d B %02d, min %02d\n"
	},
	{0x005f0002,
	 "PMU3: new minimum VrefMargin (%d < %d) recorded\n"
	},
	{0x00600002,
	 "PMU3: new minimum DlyMargin (%d < %d) recorded\n"
	},
	{0x00610000,
	 "PMU0: RX finding the per-nibble, per-tg rxClkDly values\n"
	},
	{0x00620003,
	 "PMU0: Merging collected eyes [%d..%d) and analyzing for nibble %d's optimal rxClkDly\n"
	},
	{0x00630002,
	 "PMU0: -- centers: delay = %d, voltage = %d\n"
	},
	{0x00640003,
	 "PMU0: dumping optimized eye -- centers: delay = %d (%d), voltage = %d\n"
	},
	{0x00650000,
	 "PMU0: TX optimizing txDqDelays\n"
	},
	{0x00660001,
	 "PMU3: Analyzing collected eye %d for a lane's optimal TxDqDly\n"
	},
	{0x00670001,
	 "PMU0: eye-lane %d is disable\n"
	},
	{0x00680000,
	 "PMU0: TX optimizing device voltages\n"
	},
	{0x00690002,
	 "PMU0: Merging collected eyes [%d..%d) and analyzing for optimal device txVref\n"
	},
	{0x006a0002,
	 "PMU0: -- centers: delay = %d, voltage = %d\n"
	},
	{0x006b0003,
	 "PMU0: dumping optimized eye -- centers: delay = %d (%d), voltage = %d\n"
	},
	{0x006c0000,
	 "PMU4: VrefDac (compound all TG) Bottom Top -> Center\n"
	},
	{0x006d0005,
	 "PMU4: DB%d L%d   %3d   %3d  ->  %3d (DISCONNECTED)\n"
	},
	{0x006e0005,
	 "PMU4: DB%d L%d   %3d   %3d  ->  %3d\n"
	},
	{0x006f0005,
	 "PMU0: writing rxClkDelay for tg%d db%1d nib%1d to 0x%02x from eye[%02d] (DISCONNECTED)\n"
	},
	{0x00700003,
	 "PMU: Error: Dbyte %d nibble %d's optimal rxClkDly of 0x%x is out of bounds\n"
	},
	{0x00710005,
	 "PMU0: writing rxClkDelay for tg%d db%1d nib%1d to 0x%02x from eye[%02d]\n"
	},
	{0x00720005,
	 "PMU0: tx voltage for tg%2d nib%2d to %3d (%d) from eye[%02d]\n"
	},
	{0x00730001,
	 "PMU0: vref Sum = %d\n"
	},
	{0x00740004,
	 "PMU0: tx voltage total is %d/%d -> %d -> %d\n"
	},
	{0x00750007,
	 "PMU0: writing txDqDelay for tg%1d db%1d ln%1d to  0x%02x (%d coarse, %d fine) from eye[%02d] (DISCONNECTED)\n"
	},
	{0x00760003,
	 "PMU: Error: Dbyte %d lane %d's optimal txDqDly of 0x%x is out of bounds\n"
	},
	{0x00770007,
	 "PMU0: writing txDqDelay for tg%1d db%1d l%1d to  0x%02x (%d coarse, %d fine) from eye[%02d]\n"
	},
	{0x00780002,
	 "PMU0: %d (0=tx, 1=rx) TgMask for this simulation: %x\n"
	},
	{0x00790001,
	 "PMU0: eye-byte %d is disable\n"
	},
	{0x007a0001,
	 "PMU0: eye-lane %d is disable\n"
	},
	{0x007b0003,
	 "PMU10: Start d4_2d_lrdimm_rx_dfe dimm %d nbTap %d biasStepMode %d\n"
	},
	{0x007c0001,
	 "PMU10: DB DFE feature not fully supported, F2BCEx value is 0x%02x\n"
	},
	{0x007d0001,
	 "PMU10: DB DFE feature fully supported, F2BCEx value is 0x%02x\n"
	},
	{0x007e0002,
	 "PMU8: Start d4_2d_lrdimm_rx_dfe for tap %d biasStepInc %d\n"
	},
	{0x007f0001,
	 "PMU7: Start d4_2d_lrdimm_rx_dfe tapCoff 0x%0x\n"
	},
	{0x00800003,
	 "PMU6: d4_2d_lrdimm_rx_dfe db %d lane %d area %d\n"
	},
	{0x00810004,
	 "PMU7: d4_2d_lrdimm_rx_dfe db %d lane %d max area %d best bias 0x%0x\n"
	},
	{0x00820001,
	 "PMU0: eye-lane %d is disable\n"
	},
	{0x00830003,
	 "PMU5: Setting 0x%x improved rank weight (%4d < %4d)\n"
	},
	{0x00840001,
	 "PMU4: Setting 0x%x still optimal\n"
	},
	{0x00850002,
	 "PMU5: ---- Training CS%d MR%d DRAM Equalization ----\n"
	},
	{0x00860001,
	 "PMU0: eye-lane %d is disable\n"
	},
	{0x00870003,
	 "PMU0: eye %d weight %d allTgWeight %d\n"
	},
	{0x00880002,
	 "PMU5: FFE figure of merit improved from %d to %d\n"
	},
	{0x00890002,
	 "PMU: Error: LP4 rank %d cannot be mapped on tg %d\n"
	},
	{0x008a0000,
	 "PMU4: Adjusting vrefDac0 for just 1->x transitions\n"
	},
	{0x008b0000,
	 "PMU4: Adjusting vrefDac1 for just 0->x transitions\n"
	},
	{0x008c0001,
	 "PMU5: Strong 1, pull-up %d ohms\n"
	},
	{0x008d0001,
	 "PMU5: Strong 0, pull-down %d ohms\n"
	},
	{0x008e0000,
	 "PMU4: Enabling weak drive strengths (FFE)\n"
	},
	{0x008f0000,
	 "PMU5: Changing all weak driver strengths\n"
	},
	{0x00900000,
	 "PMU5: Finalizing weak drive strengths\n"
	},
	{0x00910000,
	 "PMU4: retraining with optimal drive strength settings\n"
	},
	{0x00920002,
	 "PMU0: targeting CsX = %d and CsY = %d\n"
	},
	{0x00930001,
	 "PMU1:prbsGenCtl:%x\n"
	},
	{0x00940000,
	 "PMU1: loading 2D acsm sequence\n"
	},
	{0x00950000,
	 "PMU1: loading 1D acsm sequence\n"
	},
	{0x00960002,
	 "PMU3: %d memclocks @ %d to get half of 300ns\n"
	},
	{0x00970000,
	 "PMU: Error: User requested MPR read pattern for read DQS training in DDR3 Mode\n"
	},
	{0x00980000,
	 "PMU3: Running 1D search for left eye edge\n"
	},
	{0x00990001,
	 "PMU1: In Phase Left Edge Search cs %d\n"
	},
	{0x009a0001,
	 "PMU1: Out of Phase Left Edge Search cs %d\n"
	},
	{0x009b0000,
	 "PMU3: Running 1D search for right eye edge\n"
	},
	{0x009c0001,
	 "PMU1: In Phase Right Edge Search cs %d\n"
	},
	{0x009d0001,
	 "PMU1: Out of Phase Right Edge Search cs %d\n"
	},
	{0x009e0001,
	 "PMU1: mxRdLat training pstate %d\n"
	},
	{0x009f0001,
	 "PMU1: mxRdLat search for cs %d\n"
	},
	{0x00a00001,
	 "PMU0: MaxRdLat non consistent DtsmLoThldXingInd 0x%03x\n"
	},
	{0x00a10003,
	 "PMU4: CS %d Dbyte %d worked with DFIMRL = %d DFICLKs\n"
	},
	{0x00a20004,
	 "PMU3: MaxRdLat Read Lane err mask for csn %d, DFIMRL %2d DFIClks, dbyte %d = 0x%03x\n"
	},
	{0x00a30003,
	 "PMU3: MaxRdLat Read Lane err mask for csn %d DFIMRL %2d, All dbytes = 0x%03x\n"
	},
	{0x00a40001,
	 "PMU: Error: CS%d failed to find a DFIMRL setting that worked for all bytes during MaxRdLat training\n"
	},
	{0x00a50002,
	 "PMU3: Smallest passing DFIMRL for all dbytes in CS%d = %d DFIClks\n"
	},
	{0x00a60000,
	 "PMU: Error: No passing DFIMRL value found for any chip select during MaxRdLat training\n"
	},
	{0x00a70003,
	 "PMU: Error: Dbyte %d lane %d txDqDly passing region is too small (width = %d)\n"
	},
	{0x00a80006,
	 "PMU10: Adjusting rxclkdly db %d nib %d from %d+%d=%d->%d\n"
	},
	{0x00a90000,
	 "PMU4: TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x00aa0005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00ab0002,
	 "PMU2: TXDQ delayLeft[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00ac0004,
	 "PMU2: TXDQ delayLeft[%2d] = %3d oopScaled = %3d selectOop %d\n"
	},
	{0x00ad0002,
	 "PMU2: TXDQ delayRight[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00ae0004,
	 "PMU2: TXDQ delayRight[%2d] = %3d oopScaled = %3d selectOop %d\n"
	},
	{0x00af0003,
	 "PMU: Error: Dbyte %d lane %d txDqDly passing region is too small (width = %d)\n"
	},
	{0x00b00000,
	 "PMU4: TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x00b10002,
	 "PMU4: DB %d Lane %d: (DISCONNECTED)\n"
	},
	{0x00b20005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00b30002,
	 "PMU3: Running 1D search csn %d for DM Right/NotLeft(%d) eye edge\n"
	},
	{0x00b40002,
	 "PMU3: WrDq DM byte%2d with Errcnt %d\n"
	},
	{0x00b50002,
	 "PMU3: WrDq DM byte%2d avgDly 0x%04x\n"
	},
	{0x00b60002,
	 "PMU1: WrDq DM byte%2d with Errcnt %d\n"
	},
	{0x00b70001,
	 "PMU: Error: Dbyte %d txDqDly DM training did not start inside the eye\n"
	},
	{0x00b80000,
	 "PMU4: DM TxDqDly Passing Regions (EyeLeft EyeRight -> EyeCenter) Units=1/32 UI\n"
	},
	{0x00b90002,
	 "PMU4: DB %d Lane %d: (DISCONNECTED)\n"
	},
	{0x00ba0005,
	 "PMU4: DB %d Lane %d: %3d %3d -> %3d\n"
	},
	{0x00bb0003,
	 "PMU: Error: Dbyte %d lane %d txDqDly DM passing region is too small (width = %d)\n"
	},
	{0x00bc0004,
	 "PMU3: Errcnt for MRD/MWD search nib %2d delay = (%d, 0x%02x) = %d\n"
	},
	{0x00bd0000,
	 "PMU3: Precharge all open banks\n"
	},
	{0x00be0002,
	 "PMU: Error: Dbyte %d nibble %d found mutliple working coarse delay setting for MRD/MWD\n"
	},
	{0x00bf0000,
	 "PMU4: MRD Passing Regions (coarseVal, fineLeft fineRight -> fineCenter)\n"
	},
	{0x00c00000,
	 "PMU4: MWD Passing Regions (coarseVal, fineLeft fineRight -> fineCenter)\n"
	},
	{0x00c10004,
	 "PMU10: Warning: DB %d nibble %d has multiple working coarse delays, %d and %d, choosing the smaller delay\n"
	},
	{0x00c20003,
	 "PMU: Error: Dbyte %d nibble %d MRD/MWD passing region is too small (width = %d)\n"
	},
	{0x00c30006,
	 "PMU4: DB %d nibble %d: %3d, %3d %3d -> %3d\n"
	},
	{0x00c40002,
	 "PMU1: Start MRD/nMWD %d for csn %d\n"
	},
	{0x00c50002,
	 "PMU2: RXDQS delayLeft[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00c60006,
	 "PMU2: RXDQS delayLeft[%2d] = %3d delayOop[%2d] = %3d OopScaled %4d, selectOop %d\n"
	},
	{0x00c70002,
	 "PMU2: RXDQS delayRight[%2d] = %3d (DISCONNECTED)\n"
	},
	{0x00c80006,
	 "PMU2: RXDQS delayRight[%2d] = %3d delayOop[%2d] = %4d OopScaled %4d, selectOop %d\n"
	},
	{0x00c90000,
	 "PMU4: RxClkDly Passing Regions (EyeLeft EyeRight -> EyeCenter)\n"
	},
	{0x00ca0002,
	 "PMU4: DB %d nibble %d: (DISCONNECTED)\n"
	},
	{0x00cb0005,
	 "PMU4: DB %d nibble %d: %3d %3d -> %3d\n"
	},
	{0x00cc0003,
	 "PMU: Error: Dbyte %d nibble %d rxClkDly passing region is too small (width = %d)\n"
	},
	{0x00cd0002,
	 "PMU0: goodbar = %d for RDWR_BLEN %d\n"
	},
	{0x00ce0001,
	 "PMU3: RxClkDly = %d\n"
	},
	{0x00cf0005,
	 "PMU0: db %d l %d absLane %d -> bottom %d top %d\n"
	},
	{0x00d00009,
	 "PMU3: BYTE %d - %3d %3d %3d %3d %3d %3d %3d %3d\n"
	},
	{0x00d10002,
	 "PMU: Error: dbyte %d lane %d's per-lane vrefDAC's had no passing region\n"
	},
	{0x00d20004,
	 "PMU0: db%d l%d - %d %d\n"
	},
	{0x00d30002,
	 "PMU0: goodbar = %d for RDWR_BLEN %d\n"
	},
	{0x00d40004,
	 "PMU3: db%d l%d saw %d issues at rxClkDly %d\n"
	},
	{0x00d50003,
	 "PMU3: db%d l%d first saw a pass->fail edge at rxClkDly %d\n"
	},
	{0x00d60002,
	 "PMU3: lane %d PBD = %d\n"
	},
	{0x00d70003,
	 "PMU3: db%d l%d first saw a DBI pass->fail edge at rxClkDly %d\n"
	},
	{0x00d80003,
	 "PMU2: db%d l%d already passed rxPBD = %d\n"
	},
	{0x00d90003,
	 "PMU0: db%d l%d, PBD = %d\n"
	},
	{0x00da0002,
	 "PMU: Error: dbyte %d lane %d failed read deskew\n"
	},
	{0x00db0003,
	 "PMU0: db%d l%d, inc PBD = %d\n"
	},
	{0x00dc0003,
	 "PMU1: Running lane deskew on pstate %d csn %d rdDBIEn %d\n"
	},
	{0x00dd0000,
	 "PMU: Error: Read deskew training has been requested, but csrMajorModeDbyte[2] is set\n"
	},
	{0x00de0002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00df0002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00e00001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D3U Type\n"
	},
	{0x00e10001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D3R Type\n"
	},
	{0x00e20001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4U Type\n"
	},
	{0x00e30001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4R Type\n"
	},
	{0x00e40001,
	 "PMU: Error: Wrong PMU image loaded. message Block DramType = 0x%02x, but image built for D4LR Type\n"
	},
	{0x00e50000,
	 "PMU: Error: Both 2t timing mode and ddr4 geardown mode specified in the messageblock's PhyCfg and MR3 fields. Only one can be enabled\n"
	},
	{0x00e60003,
	 "PMU10: PHY TOTALS - NUM_DBYTES %d NUM_NIBBLES %d NUM_ANIBS %d\n"
	},
	{0x00e70006,
	 "PMU10: CSA=0x%02x, CSB=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, MMISC=%d DRAMFreq=%dMT DramType=LPDDR3\n"
	},
	{0x00e80006,
	 "PMU10: CSA=0x%02x, CSB=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, MMISC=%d DRAMFreq=%dMT DramType=LPDDR4\n"
	},
	{0x00e90008,
	 "PMU10: CS=0x%02x, TSTAGES=0x%04x, HDTOUT=%d, 2T=%d, MMISC=%d AddrMirror=%d DRAMFreq=%dMT DramType=%d\n"
	},
	{0x00ea0004,
	 "PMU10: Pstate%d MR0=0x%04x MR1=0x%04x MR2=0x%04x\n"
	},
	{0x00eb0008,
	 "PMU10: Pstate%d MRS MR0=0x%04x MR1=0x%04x MR2=0x%04x MR3=0x%04x MR4=0x%04x MR5=0x%04x MR6=0x%04x\n"
	},
	{0x00ec0005,
	 "PMU10: Pstate%d MRS MR1_A0=0x%04x MR2_A0=0x%04x MR3_A0=0x%04x MR11_A0=0x%04x\n"
	},
	{0x00ed0000,
	 "PMU10: UseBroadcastMR set. All ranks and channels use MRXX_A0 for MR settings.\n"
	},
	{0x00ee0005,
	 "PMU10: Pstate%d MRS MR01_A0=0x%02x MR02_A0=0x%02x MR03_A0=0x%02x MR11_A0=0x%02x\n"
	},
	{0x00ef0005,
	 "PMU10: Pstate%d MRS MR12_A0=0x%02x MR13_A0=0x%02x MR14_A0=0x%02x MR22_A0=0x%02x\n"
	},
	{0x00f00005,
	 "PMU10: Pstate%d MRS MR01_A1=0x%02x MR02_A1=0x%02x MR03_A1=0x%02x MR11_A1=0x%02x\n"
	},
	{0x00f10005,
	 "PMU10: Pstate%d MRS MR12_A1=0x%02x MR13_A1=0x%02x MR14_A1=0x%02x MR22_A1=0x%02x\n"
	},
	{0x00f20005,
	 "PMU10: Pstate%d MRS MR01_B0=0x%02x MR02_B0=0x%02x MR03_B0=0x%02x MR11_B0=0x%02x\n"
	},
	{0x00f30005,
	 "PMU10: Pstate%d MRS MR12_B0=0x%02x MR13_B0=0x%02x MR14_B0=0x%02x MR22_B0=0x%02x\n"
	},
	{0x00f40005,
	 "PMU10: Pstate%d MRS MR01_B1=0x%02x MR02_B1=0x%02x MR03_B1=0x%02x MR11_B1=0x%02x\n"
	},
	{0x00f50005,
	 "PMU10: Pstate%d MRS MR12_B1=0x%02x MR13_B1=0x%02x MR14_B1=0x%02x MR22_B1=0x%02x\n"
	},
	{0x00f60002,
	 "PMU1: AcsmOdtCtrl%02d 0x%02x\n"
	},
	{0x00f70002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00f80002,
	 "PMU1: AcsmCsMapCtrl%02d 0x%04x\n"
	},
	{0x00f90000,
	 "PMU1: HwtCAMode set\n"
	},
	{0x00fa0001,
	 "PMU3: DDR4 infinite preamble enter/exit mode %d\n"
	},
	{0x00fb0002,
	 "PMU1: In rxenb_train() csn=%d pstate=%d\n"
	},
	{0x00fc0000,
	 "PMU3: Finding DQS falling edge\n"
	},
	{0x00fd0000,
	 "PMU3: Searching for DDR3/LPDDR3/LPDDR4 read preamble\n"
	},
	{0x00fe0009,
	 "PMU3: dtsm fails Even Nibbles : %2x %2x %2x %2x %2x %2x %2x %2x %2x\n"
	},
	{0x00ff0009,
	 "PMU3: dtsm fails Odd  Nibbles : %2x %2x %2x %2x %2x %2x %2x %2x %2x\n"
	},
	{0x01000002,
	 "PMU3: Preamble search pass=%d anyfail=%d\n"
	},
	{0x01010000,
	 "PMU: Error: RxEn training preamble not found\n"
	},
	{0x01020000,
	 "PMU3: Found DQS pre-amble\n"
	},
	{0x01030001,
	 "PMU: Error: Dbyte %d couldn't find the rising edge of DQS during RxEn Training\n"
	},
	{0x01040000,
	 "PMU3: RxEn aligning to first rising edge of burst\n"
	},
	{0x01050001,
	 "PMU3: Decreasing RxEn delay by %d fine step to allow full capture of reads\n"
	},
	{0x01060001,
	 "PMU3: MREP Delay = %d\n"
	},
	{0x01070003,
	 "PMU3: Errcnt for MREP nib %2d delay = %2d is %d\n"
	},
	{0x01080002,
	 "PMU3: MREP nibble %d sampled a 1 at data buffer delay %d\n"
	},
	{0x01090002,
	 "PMU3: MREP nibble %d saw a 0 to 1 transition at data buffer delay %d\n"
	},
	{0x010a0000,
	 "PMU2:  MREP did not find a 0 to 1 transition for all nibbles. Failing nibbles assumed to have rising edge close to fine delay 63\n"
	},
	{0x010b0002,
	 "PMU2:  Rising edge found in alias window, setting rxDly for nibble %d = %d\n"
	},
	{0x010c0002,
	 "PMU: Error: Failed MREP for nib %d with %d one\n"
	},
	{0x010d0003,
	 "PMU2:  Rising edge not found in alias window with %d one, leaving rxDly for nibble %d = %d\n"
	},
	{0x010e0002,
	 "PMU3: Training DIMM %d CSn %d\n"
	},
	{0x010f0001,
	 "PMU3: exitCAtrain_lp3 cs 0x%x\n"
	},
	{0x01100001,
	 "PMU3: enterCAtrain_lp3 cs 0x%x\n"
	},
	{0x01110001,
	 "PMU3: CAtrain_switchmsb_lp3 cs 0x%x\n"
	},
	{0x01120001,
	 "PMU3: CATrain_rdwr_lp3 looking for pattern %x\n"
	},
	{0x01130000,
	 "PMU3: exitCAtrain_lp4\n"
	},
	{0x01140001,
	 "PMU3: DEBUG enterCAtrain_lp4 1: cs 0x%x\n"
	},
	{0x01150001,
	 "PMU3: DEBUG enterCAtrain_lp4 3: Put dbyte %d in async mode\n"
	},
	{0x01160000,
	 "PMU3: DEBUG enterCAtrain_lp4 5: Send MR13 to turn on CA training\n"
	},
	{0x01170003,
	 "PMU3: DEBUG enterCAtrain_lp4 7: idx = %d vref = %x mr12 = %x\n"
	},
	{0x01180001,
	 "PMU3: CATrain_rdwr_lp4 looking for pattern %x\n"
	},
	{0x01190004,
	 "PMU3: Phase %d CAreadbackA db:%d %x xo:%x\n"
	},
	{0x011a0005,
	 "PMU3: DEBUG lp4SetCatrVref 1: cs=%d chan=%d mr12=%x vref=%d.%d%%\n"
	},
	{0x011b0003,
	 "PMU3: DEBUG lp4SetCatrVref 3: mr12 = %x send vref= %x to db=%d\n"
	},
	{0x011c0000,
	 "PMU10:Optimizing vref\n"
	},
	{0x011d0004,
	 "PMU4:mr12:%2x cs:%d chan %d r:%4x\n"
	},
	{0x011e0005,
	 "PMU3: i:%2d bstr:%2d bsto:%2d st:%d r:%d\n"
	},
	{0x011f0002,
	 "Failed to find sufficient CA Vref Passing Region for CS %d ch. %d\n"
	},
	{0x01200005,
	 "PMU3:Found %d.%d%% MR12:%x for cs:%d chan %d\n"
	},
	{0x01210002,
	 "PMU3:Calculated %d for AtxImpedence from acx %d.\n"
	},
	{0x01220000,
	 "PMU3:CA Odt impedence ==0.  Use default vref.\n"
	},
	{0x01230003,
	 "PMU3:Calculated %d.%d%% for Vref MR12=0x%x.\n"
	},
	{0x01240000,
	 "PMU3: CAtrain_lp\n"
	},
	{0x01250000,
	 "PMU3: CAtrain Begins.\n"
	},
	{0x01260001,
	 "PMU3: CAtrain_lp testing dly %d\n"
	},
	{0x01270001,
	 "PMU5: CA bitmap dump for cs %x\n"
	},
	{0x01280001,
	 "PMU5: CAA%d "
	},
	{0x01290001, "%02x"
	},
	{0x012a0000, "\n"
	},
	{0x012b0001,
	 "PMU5: CAB%d "
	},
	{0x012c0001, "%02x"
	},
	{0x012d0000, "\n"
	},
	{0x012e0003,
	 "PMU3: anibi=%d, anibichan[anibi]=%d ,chan=%d\n"
	},
	{0x012f0001, "%02x"
	},
	{0x01300001, "\nPMU3:Raw CA setting :%x"
	},
	{0x01310002, "\nPMU3:ATxDly setting:%x margin:%d\n"
	},
	{0x01320002, "\nPMU3:InvClk ATxDly setting:%x margin:%d\n"
	},
	{0x01330000, "\nPMU3:No Range found!\n"
	},
	{0x01340003,
	 "PMU3: 2 anibi=%d, anibichan[anibi]=%d ,chan=%d"
	},
	{0x01350002, "\nPMU3: no neg clock => CA setting anib=%d, :%d\n"
	},
	{0x01360001,
	 "PMU3:Normal margin:%d\n"
	},
	{0x01370001,
	 "PMU3:Inverted margin:%d\n"
	},
	{0x01380000,
	 "PMU3:Using Inverted clock\n"
	},
	{0x01390000,
	 "PMU3:Using normal clk\n"
	},
	{0x013a0003,
	 "PMU3: 3 anibi=%d, anibichan[anibi]=%d ,chan=%d\n"
	},
	{0x013b0002,
	 "PMU3: Setting ATxDly for anib %x to %x\n"
	},
	{0x013c0000,
	 "PMU: Error: CA Training Failed.\n"
	},
	{0x013d0000,
	 "PMU1: Writing MRs\n"
	},
	{0x013e0000,
	 "PMU4:Using MR12 values from 1D CA VREF training.\n"
	},
	{0x013f0000,
	 "PMU3:Writing all MRs to fsp 1\n"
	},
	{0x01400000,
	 "PMU10:Lp4Quickboot mode.\n"
	},
	{0x01410000,
	 "PMU3: Writing MRs\n"
	},
	{0x01420001,
	 "PMU10: Setting boot clock divider to %d\n"
	},
	{0x01430000,
	 "PMU3: Resetting DRAM\n"
	},
	{0x01440000,
	 "PMU3: setup for RCD initalization\n"
	},
	{0x01450000,
	 "PMU3: pmu_exit_SR from dev_init()\n"
	},
	{0x01460000,
	 "PMU3: initializing RCD\n"
	},
	{0x01470000,
	 "PMU10: **** Executing 2D Image ****\n"
	},
	{0x01480001,
	 "PMU10: **** Start DDR4 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x01490001,
	 "PMU10: **** Start DDR3 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x014a0001,
	 "PMU10: **** Start LPDDR3 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x014b0001,
	 "PMU10: **** Start LPDDR4 Training. PMU Firmware Revision 0x%04x ****\n"
	},
	{0x014c0000,
	 "PMU: Error: Mismatched internal revision between DCCM and ICCM images\n"
	},
	{0x014d0001,
	 "PMU10: **** Testchip %d Specific Firmware ****\n"
	},
	{0x014e0000,
	 "PMU1: LRDIMM with EncodedCS mode, one DIMM\n"
	},
	{0x014f0000,
	 "PMU1: LRDIMM with EncodedCS mode, two DIMMs\n"
	},
	{0x01500000,
	 "PMU1: RDIMM with EncodedCS mode, one DIMM\n"
	},
	{0x01510000,
	 "PMU2: Starting LRDIMM MREP training for all ranks\n"
	},
	{0x01520000,
	 "PMU199: LRDIMM MREP training for all ranks completed\n"
	},
	{0x01530000,
	 "PMU2: Starting LRDIMM DWL training for all ranks\n"
	},
	{0x01540000,
	 "PMU199: LRDIMM DWL training for all ranks completed\n"
	},
	{0x01550000,
	 "PMU2: Starting LRDIMM MRD training for all ranks\n"
	},
	{0x01560000,
	 "PMU199: LRDIMM MRD training for all ranks completed\n"
	},
	{0x01570000,
	 "PMU2: Starting RXEN training for all ranks\n"
	},
	{0x01580000,
	 "PMU2: Starting write leveling fine delay training for all ranks\n"
	},
	{0x01590000,
	 "PMU2: Starting LRDIMM MWD training for all ranks\n"
	},
	{0x015a0000,
	 "PMU199: LRDIMM MWD training for all ranks completed\n"
	},
	{0x015b0000,
	 "PMU2: Starting write leveling fine delay training for all ranks\n"
	},
	{0x015c0000,
	 "PMU2: Starting read deskew training\n"
	},
	{0x015d0000,
	 "PMU2: Starting SI friendly 1d RdDqs training for all ranks\n"
	},
	{0x015e0000,
	 "PMU2: Starting write leveling coarse delay training for all ranks\n"
	},
	{0x015f0000,
	 "PMU2: Starting 1d WrDq training for all ranks\n"
	},
	{0x01600000,
	 "PMU2: Running DQS2DQ Oscillator for all ranks\n"
	},
	{0x01610000,
	 "PMU2: Starting again read deskew training but with PRBS\n"
	},
	{0x01620000,
	 "PMU2: Starting 1d RdDqs training for all ranks\n"
	},
	{0x01630000,
	 "PMU2: Starting again 1d WrDq training for all ranks\n"
	},
	{0x01640000,
	 "PMU2: Starting MaxRdLat training\n"
	},
	{0x01650000,
	 "PMU2: Starting 2d WrDq training for all ranks\n"
	},
	{0x01660000,
	 "PMU2: Starting 2d RdDqs training for all ranks\n"
	},
	{0x01670002,
	 "PMU3:read_fifo %x %x\n"
	},
	{0x01680001,
	 "PMU: Error: Invalid PhyDrvImpedance of 0x%x specified in message block.\n"
	},
	{0x01690001,
	 "PMU: Error: Invalid PhyOdtImpedance of 0x%x specified in message block.\n"
	},
	{0x016a0001,
	 "PMU: Error: Invalid BPZNResVal of 0x%x specified in message block.\n"
	},
	{0x016b0005,
	 "PMU3: fixRxEnBackOff csn:%d db:%d dn:%d bo:%d dly:%x\n"
	},
	{0x016c0001,
	 "PMU3: fixRxEnBackOff dly:%x\n"
	},
	{0x016d0000,
	 "PMU3: Entering setupPpt\n"
	},
	{0x016e0000,
	 "PMU3: Start lp4PopulateHighLowBytes\n"
	},
	{0x016f0002,
	 "PMU3:Dbyte Detect: db%d received %x\n"
	},
	{0x01700002,
	 "PMU3:getDqs2Dq read %x from dbyte %d\n"
	},
	{0x01710002,
	 "PMU3:getDqs2Dq(2) read %x from dbyte %d\n"
	},
	{0x01720001,
	 "PMU: Error: Dbyte %d read 0 from the DQS oscillator it is connected to\n"
	},
	{0x01730002,
	 "PMU4: Dbyte %d dqs2dq = %d/32 UI\n"
	},
	{0x01740003,
	 "PMU3:getDqs2Dq set dqs2dq:%d/32 ui (%d ps) from dbyte %d\n"
	},
	{0x01750003,
	 "PMU3: Setting coarse delay in AtxDly chiplet %d from 0x%02x to 0x%02x\n"
	},
	{0x01760003,
	 "PMU3: Clearing coarse delay in AtxDly chiplet %d from 0x%02x to 0x%02x\n"
	},
	{0x01770000,
	 "PMU3: Performing DDR4 geardown sync sequence\n"
	},
	{0x01780000,
	 "PMU1: Enter self refresh\n"
	},
	{0x01790000,
	 "PMU1: Exit self refresh\n"
	},
	{0x017a0000,
	 "PMU: Error: No dbiEnable with lp4\n"
	},
	{0x017b0000,
	 "PMU: Error: No dbiDisable with lp4\n"
	},
	{0x017c0001,
	 "PMU1: DDR4 update Rx DBI Setting disable %d\n"
	},
	{0x017d0001,
	 "PMU1: DDR4 update 2nCk WPre Setting disable %d\n"
	},
	{0x017e0005,
	 "PMU1: read_delay: db%d lane%d delays[%2d] = 0x%02x (max 0x%02x)\n"
	},
	{0x017f0004,
	 "PMU1: write_delay: db%d lane%d delays[%2d] = 0x%04x\n"
	},
	{0x01800001,
	 "PMU5: ID=%d -- db0  db1  db2  db3  db4  db5  db6  db7  db8  db9 --\n"
	},
	{0x0181000b,
	 "PMU5: [%d]:0x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n"
	},
	{0x01820003,
	 "PMU2: dump delays - pstate=%d dimm=%d csn=%d\n"
	},
	{0x01830000,
	 "PMU3: Printing Mid-Training Delay Information\n"
	},
	{0x01840001,
	 "PMU5: CS%d <<KEY>> 0 TrainingCntr <<KEY>> coarse(15:10) fine(9:0)\n"
	},
	{0x01850001,
	 "PMU5: CS%d <<KEY>> 0 RxEnDly, 1 RxClkDly <<KEY>> coarse(10:6) fine(5:0)\n"
	},
	{0x01860001,
	 "PMU5: CS%d <<KEY>> 0 TxDqsDly, 1 TxDqDly <<KEY>> coarse(9:6) fine(5:0)\n"
	},
	{0x01870001,
	 "PMU5: CS%d <<KEY>> 0 RxPBDly <<KEY>> 1 Delay Unit ~= 7ps\n"
	},
	{0x01880000,
	 "PMU5: all CS <<KEY>> 0 DFIMRL <<KEY>> Units = DFI clocks\n"
	},
	{0x01890000,
	 "PMU5: all CS <<KEY>> VrefDACs <<KEY>> DAC(6:0)\n"
	},
	{0x018a0000,
	 "PMU1: Set DMD in MR13 and wrDBI in MR3 for training\n"
	},
	{0x018b0000,
	 "PMU: Error: getMaxRxen() failed to find largest rxen nibble delay\n"
	},
	{0x018c0003,
	 "PMU2: getMaxRxen(): maxDly %d maxTg %d maxNib %d\n"
	},
	{0x018d0003,
	 "PMU2: getRankMaxRxen(): maxDly %d Tg %d maxNib %d\n"
	},
	{0x018e0000,
	 "PMU1: skipping CDD calculation in 2D image\n"
	},
	{0x018f0001,
	 "PMU3: Calculating CDDs for pstate %d\n"
	},
	{0x01900003,
	 "PMU3: rxFromDly[%d][%d] = %d\n"
	},
	{0x01910003,
	 "PMU3: rxToDly  [%d][%d] = %d\n"
	},
	{0x01920003,
	 "PMU3: rxDly    [%d][%d] = %d\n"
	},
	{0x01930003,
	 "PMU3: txDly    [%d][%d] = %d\n"
	},
	{0x01940003,
	 "PMU3: allFine CDD_RR_%d_%d = %d\n"
	},
	{0x01950003,
	 "PMU3: allFine CDD_WW_%d_%d = %d\n"
	},
	{0x01960003,
	 "PMU3: CDD_RR_%d_%d = %d\n"
	},
	{0x01970003,
	 "PMU3: CDD_WW_%d_%d = %d\n"
	},
	{0x01980003,
	 "PMU3: allFine CDD_RW_%d_%d = %d\n"
	},
	{0x01990003,
	 "PMU3: allFine CDD_WR_%d_%d = %d\n"
	},
	{0x019a0003,
	 "PMU3: CDD_RW_%d_%d = %d\n"
	},
	{0x019b0003,
	 "PMU3: CDD_WR_%d_%d = %d\n"
	},
	{0x019c0004,
	 "PMU3: F%dBC2x_B%d_D%d = 0x%02x\n"
	},
	{0x019d0004,
	 "PMU3: F%dBC3x_B%d_D%d = 0x%02x\n"
	},
	{0x019e0004,
	 "PMU3: F%dBC4x_B%d_D%d = 0x%02x\n"
	},
	{0x019f0004,
	 "PMU3: F%dBC5x_B%d_D%d = 0x%02x\n"
	},
	{0x01a00004,
	 "PMU3: F%dBC8x_B%d_D%d = 0x%02x\n"
	},
	{0x01a10004,
	 "PMU3: F%dBC9x_B%d_D%d = 0x%02x\n"
	},
	{0x01a20004,
	 "PMU3: F%dBCAx_B%d_D%d = 0x%02x\n"
	},
	{0x01a30004,
	 "PMU3: F%dBCBx_B%d_D%d = 0x%02x\n"
	},
	{0x01a40000,
	 "PMU10: Entering context_switch_postamble\n"
	},
	{0x01a50003,
	 "PMU10: context_switch_postamble is enabled for DIMM %d, RC0A=0x%x, RC3x=0x%x\n"
	},
	{0x01a60000,
	 "PMU10: Setting bcw fspace 0\n"
	},
	{0x01a70001,
	 "PMU10: Sending BC0A = 0x%x\n"
	},
	{0x01a80001,
	 "PMU10: Sending BC6x = 0x%x\n"
	},
	{0x01a90001,
	 "PMU10: Sending RC0A = 0x%x\n"
	},
	{0x01aa0001,
	 "PMU10: Sending RC3x = 0x%x\n"
	},
	{0x01ab0001,
	 "PMU10: Sending RC0A = 0x%x\n"
	},
	{0x01ac0001,
	 "PMU1: enter_lp3: DEBUG: pstate = %d\n"
	},
	{0x01ad0001,
	 "PMU1: enter_lp3: DEBUG: dfifreqxlat_pstate = %d\n"
	},
	{0x01ae0001,
	 "PMU1: enter_lp3: DEBUG: pllbypass = %d\n"
	},
	{0x01af0001,
	 "PMU1: enter_lp3: DEBUG: forcecal = %d\n"
	},
	{0x01b00001,
	 "PMU1: enter_lp3: DEBUG: pllmaxrange = 0x%x\n"
	},
	{0x01b10001,
	 "PMU1: enter_lp3: DEBUG: dacval_out = 0x%x\n"
	},
	{0x01b20001,
	 "PMU1: enter_lp3: DEBUG: pllctrl3 = 0x%x\n"
	},
	{0x01b30000,
	 "PMU3: Loading DRAM with BIOS supplied MR values and entering self refresh prior to exiting PMU code.\n"
	},
	{0x01b40002,
	 "PMU3: Setting DataBuffer function space of dimmcs 0x%02x to %d\n"
	},
	{0x01b50002,
	 "PMU4: Setting RCW FxRC%Xx = 0x%02x\n"
	},
	{0x01b60002,
	 "PMU4: Setting RCW FxRC%02x = 0x%02x\n"
	},
	{0x01b70001,
	 "PMU1: DDR4 update Rd Pre Setting disable %d\n"
	},
	{0x01b80002,
	 "PMU2: Setting BCW FxBC%Xx = 0x%02x\n"
	},
	{0x01b90002,
	 "PMU2: Setting BCW BC%02x = 0x%02x\n"
	},
	{0x01ba0002,
	 "PMU2: Setting BCW PBA mode FxBC%Xx = 0x%02x\n"
	},
	{0x01bb0002,
	 "PMU2: Setting BCW PBA mode BC%02x = 0x%02x\n"
	},
	{0x01bc0003,
	 "PMU4: BCW value for dimm %d, fspace %d, addr 0x%04x\n"
	},
	{0x01bd0002,
	 "PMU4: DB %d, value 0x%02x\n"
	},
	{0x01be0000,
	 "PMU6: WARNING MREP underflow, set to min value -2 coarse, 0 fine\n"
	},
	{0x01bf0004,
	 "PMU6: LRDIMM Writing final data buffer fine delay value nib %2d, trainDly %3d, fineDly code %2d, new MREP fine %2d\n"
	},
	{0x01c00003,
	 "PMU6: LRDIMM Writing final data buffer fine delay value nib %2d, trainDly %3d, fineDly code %2d\n"
	},
	{0x01c10003,
	 "PMU6: LRDIMM Writing data buffer fine delay type %d nib %2d, code %2d\n"
	},
	{0x01c20002,
	 "PMU6: Writing final data buffer coarse delay value dbyte %2d, coarse = 0x%02x\n"
	},
	{0x01c30003,
	 "PMU4: data 0x%04x at MB addr 0x%08x saved at CSR addr 0x%08x\n"
	},
	{0x01c40003,
	 "PMU4: data 0x%04x at MB addr 0x%08x restored from CSR addr 0x%08x\n"
	},
	{0x01c50003,
	 "PMU4: data 0x%04x at MB addr 0x%08x saved at CSR addr 0x%08x\n"
	},
	{0x01c60003,
	 "PMU4: data 0x%04x at MB addr 0x%08x restored from CSR addr 0x%08x\n"
	},
	{0x01c70001,
	 "PMU3: Update BC00, BC01, BC02 for rank-dimm 0x%02x\n"
	},
	{0x01c80000,
	 "PMU3: Writing D4 RDIMM RCD Control words F0RC00 -> F0RC0F\n"
	},
	{0x01c90000,
	 "PMU3: Disable parity in F0RC0E\n"
	},
	{0x01ca0000,
	 "PMU3: Writing D4 RDIMM RCD Control words F1RC00 -> F1RC05\n"
	},
	{0x01cb0000,
	 "PMU3: Writing D4 RDIMM RCD Control words F1RC1x -> F1RC9x\n"
	},
	{0x01cc0000,
	 "PMU3: Writing D4 Data buffer Control words BC00 -> BC0E\n"
	},
	{0x01cd0002,
	 "PMU1: setAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x01ce0002,
	 "PMU1: restoreFromAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x01cf0002,
	 "PMU1: restoreAcsmFromAltCL Sending MR0 0x%x cl=%d\n"
	},
	{0x01d00002,
	 "PMU2: Setting D3R RC%d = 0x%01x\n"
	},
	{0x01d10000,
	 "PMU3: Writing D3 RDIMM RCD Control words RC0 -> RC11\n"
	},
	{0x01d20002,
	 "PMU0: VrefDAC0/1 vddqStart %d dacToVddq %d\n"
	},
	{0x01d30001,
	 "PMU: Error: Messageblock phyVref=0x%x is above the limit for TSMC28's attenuated LPDDR4 receivers. Please see the pub databook\n"
	},
	{0x01d40001,
	 "PMU: Error: Messageblock phyVref=0x%x is above the limit for TSMC28's attenuated DDR4 receivers. Please see the pub databook\n"
	},
	{0x01d50001,
	 "PMU0: PHY VREF @ (%d/1000) VDDQ\n"
	},
	{0x01d60002,
	 "PMU0: initalizing phy vrefDacs to %d ExtVrefRange %x\n"
	},
	{0x01d70002,
	 "PMU0: initalizing global vref to %d range %d\n"
	},
	{0x01d80002,
	 "PMU4: Setting initial device vrefDQ for CS%d to MR6 = 0x%04x\n"
	},
	{0x01d90003,
	 "PMU1: In write_level_fine() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01da0000,
	 "PMU3: Fine write leveling hardware search increasing TxDqsDly until full bursts are seen\n"
	},
	{0x01db0000,
	 "PMU4: WL normalized pos   : ........................|........................\n"
	},
	{0x01dc0007,
	 "PMU4: WL margin for nib %2d: %08x%08x%08x%08x%08x%08x\n"
	},
	{0x01dd0000,
	 "PMU4: WL normalized pos   : ........................|........................\n"
	},
	{0x01de0000,
	 "PMU3: Exiting write leveling mode\n"
	},
	{0x01df0001,
	 "PMU3: got %d for cl in load_wrlvl_acsm\n"
	},
	{0x01e00003,
	 "PMU1: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01e10003,
	 "PMU3: left eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x01e20003,
	 "PMU3: right eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x01e30004,
	 "PMU3: eye center db:%d ln:%d dly:0x%x (maxdq:%x)\n"
	},
	{0x01e40003,
	 "PMU3: Wrote to TxDqDly db:%d ln:%d dly:0x%x\n"
	},
	{0x01e50003,
	 "PMU3: Wrote to TxDqDly db:%d ln:%d dly:0x%x\n"
	},
	{0x01e60002,
	 "PMU3: Coarse write leveling dbyte%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x01e70002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x01e80000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x01e90001,
	 "PMU3: got %d for cl in load_wrlvl_acsm\n"
	},
	{0x01ea0003,
	 "PMU3: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01eb0003,
	 "PMU3: left eye edge search db:%d ln:%d dly:0x%x\n"
	},
	{0x01ec0003,
	 "PMU3: right eye edge search db: %d ln: %d dly: 0x%x\n"
	},
	{0x01ed0004,
	 "PMU3: eye center db: %d ln: %d dly: 0x%x (maxdq: 0x%x)\n"
	},
	{0x01ee0003,
	 "PMU3: Wrote to TxDqDly db: %d ln: %d dly: 0x%x\n"
	},
	{0x01ef0003,
	 "PMU3: Wrote to TxDqDly db: %d ln: %d dly: 0x%x\n"
	},
	{0x01f00002,
	 "PMU3: Coarse write leveling nibble%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x01f10002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x01f20000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x01f30000,
	 "PMU4: WL normalized pos   : ................................|................................\n"
	},
	{0x01f40009,
	 "PMU4: WL margin for nib %2d: %08x%08x%08x%08x%08x%08x%08x%08x\n"
	},
	{0x01f50000,
	 "PMU4: WL normalized pos   : ................................|................................\n"
	},
	{0x01f60001,
	 "PMU8: Adjust margin after WL coarse to be larger than %d\n"
	},
	{0x01f70001,
	 "PMU: Error: All margin after write leveling coarse are smaller than minMargin %d\n"
	},
	{0x01f80002,
	 "PMU8: Decrement nib %d TxDqsDly by %d fine step\n"
	},
	{0x01f90003,
	 "PMU3: In write_level_coarse() csn=%d dimm=%d pstate=%d\n"
	},
	{0x01fa0005,
	 "PMU2: Write level: dbyte %d nib%d dq/dmbi %2d dqsfine 0x%04x dqDly 0x%04x\n"
	},
	{0x01fb0002,
	 "PMU3: Coarse write leveling nibble%2d is still failing for TxDqsDly=0x%04x\n"
	},
	{0x01fc0002,
	 "PMU4: Coarse write leveling iteration %d saw %d data miscompares across the entire phy\n"
	},
	{0x01fd0000,
	 "PMU: Error: Failed write leveling coarse\n"
	},
	{0x01fe0001,
	 "PMU3: DWL delay = %d\n"
	},
	{0x01ff0003,
	 "PMU3: Errcnt for DWL nib %2d delay = %2d is %d\n"
	},
	{0x02000002,
	 "PMU3: DWL nibble %d sampled a 1 at delay %d\n"
	},
	{0x02010003,
	 "PMU3: DWL nibble %d passed at delay %d. Rising edge was at %d\n"
	},
	{0x02020000,
	 "PMU2: DWL did nto find a rising edge of memclk for all nibbles. Failing nibbles assumed to have rising edge close to fine delay 63\n"
	},
	{0x02030002,
	 "PMU2:  Rising edge found in alias window, setting wrlvlDly for nibble %d = %d\n"
	},
	{0x02040002,
	 "PMU: Error: Failed DWL for nib %d with %d one\n"
	},
	{0x02050003,
	 "PMU2:  Rising edge not found in alias window with %d one, leaving wrlvlDly for nibble %d = %d\n"
	},
	{0x04000000,
	 "PMU: Error:Mailbox Buffer Overflowed.\n"
	},
	{0x04010000,
	 "PMU: Error:Mailbox Buffer Overflowed.\n"
	},
	{0x04020000,
	 "PMU: ***** Assertion Error - terminating *****\n"
	},
	{0x04030002,
	 "PMU1: swapByte db %d by %d\n"
	},
	{0x04040003,
	 "PMU3: get_cmd_dly max(%d ps, %d memclk) = %d\n"
	},
	{0x04050002,
	 "PMU0: Write CSR 0x%06x 0x%04x\n"
	},
	{0x04060002,
	 "PMU0: hwt_init_ppgc_prbs(): Polynomial: %x, Deg: %d\n"
	},
	{0x04070001,
	 "PMU: Error: acsm_set_cmd to non existent instruction address %d\n"
	},
	{0x04080001,
	 "PMU: Error: acsm_set_cmd with unknown ddr cmd 0x%x\n"
	},
	{0x0409000c,
	 "PMU1: acsm_addr %02x, acsm_flgs %04x, ddr_cmd %02x, cmd_dly %02x, ddr_addr %04x, ddr_bnk %02x, ddr_cs %02x, cmd_rcnt %02x, AcsmSeq0/1/2/3 %04x %04x %04x %04x\n"
	},
	{0x040a0000,
	 "PMU: Error: Polling on ACSM done failed to complete in acsm_poll_done()...\n"
	},
	{0x040b0000,
	 "PMU1: acsm RUN\n"
	},
	{0x040c0000,
	 "PMU1: acsm STOPPED\n"
	},
	{0x040d0002,
	 "PMU1: acsm_init: acsm_mode %04x mxrdlat %04x\n"
	},
	{0x040e0002,
	 "PMU: Error: setAcsmCLCWL: cl and cwl must be each >= 2 and 5, resp. CL=%d CWL=%d\n"
	},
	{0x040f0002,
	 "PMU: Error: setAcsmCLCWL: cl and cwl must be each >= 5. CL=%d CWL=%d\n"
	},
	{0x04100002,
	 "PMU1: setAcsmCLCWL: CASL %04d WCASL %04d\n"
	},
	{0x04110001,
	 "PMU: Error: Reserved value of register F0RC0F found in message block: 0x%04x\n"
	},
	{0x04120001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x04130001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x04140000,
	 "PMU3: Entering Boot Freq Mode.\n"
	},
	{0x04150001,
	 "PMU: Error: Boot clock divider setting of %d is too small\n"
	},
	{0x04160000,
	 "PMU3: Exiting Boot Freq Mode.\n"
	},
	{0x04170002,
	 "PMU3: Writing MR%d OP=%x\n"
	},
	{0x04180000,
	 "PMU: Error: Delay too large in slomo\n"
	},
	{0x04190001,
	 "PMU3: Written MRS to CS=0x%02x\n"
	},
	{0x041a0000,
	 "PMU3: Enable Channel A\n"
	},
	{0x041b0000,
	 "PMU3: Enable Channel B\n"
	},
	{0x041c0000,
	 "PMU3: Enable All Channels\n"
	},
	{0x041d0002,
	 "PMU2: Use PDA mode to set MR%d with value 0x%02x\n"
	},
	{0x041e0001,
	 "PMU3: Written Vref with PDA to CS=0x%02x\n"
	},
	{0x041f0000,
	 "PMU1: start_cal: DEBUG: setting CalRun to 1\n"
	},
	{0x04200000,
	 "PMU1: start_cal: DEBUG: setting CalRun to 0\n"
	},
	{0x04210001,
	 "PMU1: lock_pll_dll: DEBUG: pstate = %d\n"
	},
	{0x04220001,
	 "PMU1: lock_pll_dll: DEBUG: dfifreqxlat_pstate = %d\n"
	},
	{0x04230001,
	 "PMU1: lock_pll_dll: DEBUG: pllbypass = %d\n"
	},
	{0x04240001,
	 "PMU3: SaveLcdlSeed: Saving seed %d\n"
	},
	{0x04250000,
	 "PMU1: in phy_defaults()\n"
	},
	{0x04260003,
	 "PMU3: ACXConf:%d MaxNumDbytes:%d NumDfi:%d\n"
	},
	{0x04270005,
	 "PMU1: setAltAcsmCLCWL setting cl=%d cwl=%d\n"
	},
};
#endif /* DEBUG */
#endif

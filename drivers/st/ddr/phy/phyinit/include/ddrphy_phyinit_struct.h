/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDRPHY_PHYINIT_STRUCT_H
#define DDRPHY_PHYINIT_STRUCT_H

/* This file defines the internal data structures used in PhyInit to store user configuration */

/* DIMM Type definitions */
#define DDR_DIMMTYPE_NODIMM 4U /* No DIMM (Soldered-on) */

/*
 * Structure for basic user inputs
 *
 * The following basic data structure must be set and completed correctly so
 * that the PhyInit software package can accurate program PHY registers.
 */
struct user_input_basic {
	uint32_t dramtype;		/*
					 * DRAM module type.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | DDR4
					 *   0x1 | DDR3
					 *   0x2 | LPDDR4
					 */

	uint32_t dimmtype;		/*
					 * DIMM type.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x4 | No DIMM (Soldered-on) (DDR_DIMMTYPE_NODIMM)
					 */

	uint32_t lp4xmode;		/*
					 * LPDDR4X mode support.
					 * Only used for LPDDR4, but not valid here.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | LPDDR4 mode, when dramtype is LPDDR4
					 */

	uint32_t numdbyte;		/* Number of dbytes physically instantiated */

	uint32_t numactivedbytedfi0;	/* Number of active dbytes to be controlled by dfi0 */

	uint32_t numactivedbytedfi1;	/*
					 * Number of active dbytes to be controlled by dfi1.
					 * Only used for LPDDR4.
					 */

	uint32_t numanib;		/* Number of ANIBs physically instantiated */

	uint32_t numrank_dfi0;		/* Number of ranks in DFI0 channel */

	uint32_t numrank_dfi1;		/* Number of ranks in DFI1 channel (if DFI1 exists) */

	uint32_t dramdatawidth;		/*
					 * Width of the DRAM device.
					 *
					 * Enter 4,8,16 or 32 depending on protocol and dram type
					 * according below table.
					 *
					 * Protocol | Valid Options | Default
					 * -------- | ------------- | ---
					 * DDR3     | 4,8,16        | 8
					 * DDR4     | 4,8,16        | 8
					 * LPDDR4   | 8,16          | 16
					 *
					 * For mixed x8 and x16 width devices, set variable to x8.
					 */

	uint32_t numpstates;		/* Number of p-states used. Must be set to 1 */

	uint32_t frequency;		/*
					 * Memclk frequency for each PState.
					 * Memclk frequency in MHz round up to next highest integer.
					 * Enter 334 for 333.333, etc.
					 */

	uint32_t pllbypass;		/*
					 * Indicates if PLL should be in Bypass mode.
					 * If DDR datarate < 333, PLL must be in Bypass Mode.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x1 | Enabled
					 *   0x0 | Disabled
					 */

	uint32_t dfifreqratio;		/*
					 * Selected Dfi Frequency ratio.
					 * Used to program the dfifreqratio register. This register
					 * controls how dfi_freq_ratio input pin should be driven
					 * inaccordance with DFI Spec.
					 *
					 * Binary Value | Description
					 *        ----- | ------
					 *        2'b01 | 1:2 DFI Frequency Ratio (default)
					 */

	uint32_t dfi1exists;		/* Indicates if the PHY configuration has Dfi1 channel */

	uint32_t train2d;		/* Obsolete. Not used. */

	uint32_t hardmacrover;		/*
					 * Hard Macro Family version in use.
					 *
					 * Value | Description
					 * ----- | ------
					 *   3   | hardmacro family D
					 */

	uint32_t readdbienable;		/* Obsolete. Not Used. */

	uint32_t dfimode;		/* Obsolete. Not Used. */
};

/*
 * Structure for advanced user inputs
 */
struct user_input_advanced {
	uint32_t lp4rxpreamblemode;	/*
					 * Selects between DRAM read static vs toggle preamble.
					 * Determine desired DRAM Read Preamble Mode based on SI
					 * Analysis and DRAM Part in use.
					 * The PHY training firmware will program DRAM mr1-OP[3]
					 * after training based on setting.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x1 | toggling preamble
					 *   0x0 | static preamble
					 */

	uint32_t lp4postambleext;	/*
					 * Extend write postamble in LPDDR4.
					 * Only used for LPDDR4.
					 * This variable is used to calculate LPDDR4 mr3-OP[1] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Set value according to your SI analysis and DRAM
					 * requirement.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | half Memclk postamble
					 *   0x1 | 1.5 Memclk postabmle (default)
					 */

	uint32_t d4rxpreamblelength;	/*
					 * Length of read preamble in DDR4 mode.
					 * Only used for DDR4.
					 * This variable is used to calculate DDR4 mr4-OP[11] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Set value according to your SI analysis and DRAM
					 * requirement.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 |  1 Tck
					 *   0x1 |  2 Tck (default)
					 */

	uint32_t d4txpreamblelength;	/*
					 * Length of write preamble in DDR4 mode.
					 * Only used for DDR4.
					 * This variable is used to calculate DDR4 mr4-OP[12] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Set value according to your SI analysis and DRAM
					 * requirement.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | 1 Tck (default)
					 *   0x1 | 2 Tck
					 */

	uint32_t extcalresval;		/*
					 * External Impedance calibration pull-down resistor value
					 * select.
					 * Indicates value of impedance calibration pull-down
					 * resistor connected to BP_ZN pin of the PHY.
					 * Value | Description
					 * ----- | ------
					 *   0x0 | 240 ohm (default)
					 */

	uint32_t is2ttiming;		/*
					 * Set to 1 to use 2T timing for address/command, otherwise
					 * 1T timing will be used.
					 * Determine 1T or 2T Timing operation mode based on SI
					 * Analysis and DRAM Timing.
					 *   - In 1T mode, CK, CS, CA all have the same nominal
					 *     timing, ie. ATxDly[6:0] will have same value for all
					 *     ANIBs.
					 *   - In 2T mode, CK, CS,have the same nominal timing
					 *     (e.g. AtxDly[6:0]=0x00), while CA is delayed by 1UI
					 *     (e.g. ATxDly[6:0]=0x40)
					 * Used to program phycfg setting in messageBlock.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | 1T Timing (default)
					 *   0x1 | 2T Timing
					 */

	uint32_t odtimpedance;		/*
					 * ODT impedance in ohm.
					 * Used for programming TxOdtDrvStren registers.
					 * Enter 0 for open/high-impedance.
					 * Default value: 60
					 */

	uint32_t tximpedance;		/*
					 * Tx Drive Impedance for DQ/DQS in ohm.
					 * Used for programming TxImpedanceCtrl1 registers.
					 * Enter 0 for open/high-impedance.
					 * Default value: 60
					 */

	uint32_t atximpedance;		/*
					 * Tx Drive Impedance for AC in ohm.
					 * Used for programming ATxImpedance register.
					 * Enter 0 for open/high-impedance
					 * Default value: 20 (HMA,HMB,HMC,HMD), 40 (HME)
					 */

	uint32_t memalerten;		/*
					 * Enables BP_ALERT programming of PHY registers.
					 * Only used for DDR3 and DDR4.
					 * Used for programming MemAlertControl and MemAlertControl2
					 * registers.
					 * Program if you require using BP_ALERT pin (to receive or
					 * terminate signal) of the PHY otherwise leave at default
					 * value to save power.
					 *
					 * Value | Description
					 * ----- | ------
					 * 0x0 | Disable BP_ALERT (default)
					 */

	uint32_t memalertpuimp;		/*
					 * Specify MemAlert Pull-up Termination Impedance.
					 * Programs the pull-up termination on BP_ALERT.
					 * Not valid here (fixed 0 value).
					 */

	uint32_t memalertvreflevel;	/*
					 * Specify the Vref level for BP_ALERT(MemAlert) Receiver.
					 * Not valid here (fixed 0 value).
					 */

	uint32_t memalertsyncbypass;	/*
					 * When set, this bit bypasses the DfiClk synchronizer on
					 * dfi_alert_n.
					 * Not valid here (fixed 0 value).
					 */

	uint32_t disdynadrtri;		/*
					 * Disable Dynamic Per-MEMCLK Address Tristate feature.
					 * Program this variable if you require to disable this
					 * feature.
					 *   - In DDR3/2T and DDR4/2T/2N modes, the dynamic tristate
					 *     feature should be disabled if the controller cannot
					 *     follow the 2T PHY tristate protocol.
					 *   - In LPDDR4 mode, the dynamic tristate feature should
					 *     be disabled.
					 *
					 * Value | Description
					 * ----- | ------
					 *  0x1  | Disable Dynamic Tristate
					 */

	uint32_t phymstrtraininterval;	/*
					 * Specifies the how frequent dfi_phymstr_req is issued by
					 * PHY.
					 * Only required in LPDDR4.
					 * Based on SI analysis determine how frequent DRAM drift
					 * compensation and re-training is required.
					 * Determine if Memory controller supports DFI PHY Master
					 * Interface.
					 * Program based on desired setting for
					 * PPTTrainSetup.PhyMstrTrainInterval register.
					 * Default value: 0xa
					 *
					 * Example:
					 * Value | Description
					 * ----- | ------
					 *   0xa | PPT Train Interval = 268435456 MEMCLKs (default)
					 */

	uint32_t phymstrmaxreqtoack;	/*
					 * Max time from dfi_phymstr_req asserted to dfi_phymstr_ack
					 * asserted.
					 * Only required in LPDDR4.
					 * Based on your Memory controller's(MC) specification
					 * determine how long the PHY should wait for the assertion
					 * of dfi_phymstr_ack once dfi_phymstr_req has been issued
					 * by the PHY. If the MC does not ack the PHY's request, PHY
					 * may issue dfi_error.
					 * This value will be used to program
					 * PPTTrainSetup.PhyMstrMaxReqToAck register.
					 * Default value: 0x5
					 *
					 * Example:
					 * Value | Description
					 * ----- | ------
					 *   0x5 | PPT Max. Req to Ack. = 8192 MEMCLKs (default)
					 */

	uint32_t wdqsext;		/*
					 * Enable Write DQS Extension feature of PHY.
					 *
					 * Value | Description
					 * ----- | ------
					 *   0x0 | Disable Write DQS Extension feature. (default)
					 *   0x1 | Enable Write DQS Extension feature.
					 */

	uint32_t calinterval;		/*
					 * Specifies the interval between successive calibrations,
					 * in mS.
					 * Program variable based on desired setting for
					 * CalRate.CalInterval register.
					 * - Fixed 0x9 value (20mS interval)
					 */

	uint32_t calonce;		/*
					 * This setting changes the behaviour of CalRun register.
					 * If you desire to manually trigger impedance calibration
					 * in mission mode set this variable to 1, and toggle CalRun
					 * in mission mode.
					 *
					 * Value | Description
					 * ----- | ------
					 * 0x0   | Calibration will proceed at the rate determined
					 *       | by CalInterval. This field should only be changed
					 *       | while the calibrator is idle. ie before csr
					 *       | CalRun is set.
					 */

	uint32_t lp4rl;			/*
					 * LPDDR4 Dram Read Latency.
					 * Applicable only if dramtype == LPDDR4.
					 * This variable is used to calculate LPDDR4 mr2-OP[2:0]
					 * set in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 * Determine values based on your DRAM part's supported
					 * speed and latency bin.
					 * Default: calculated based on user_input_basic.frequency
					 * and "JEDEC JESD209-4A (LPDDR4)" Table 28 "Read and Write
					 * Latencies".
					 * Lowest latency selected when more than one latency can be
					 * used. For example given configuration for LPDDR4, x16,
					 * NoDbi and DDR533, RL=10 is selected rather than 14.
					 */

	uint32_t lp4wl;			/*
					 * LPDDR4 Dram Write Latency.
					 * Applicable only if dramtype == LPDDR4.
					 * This variable is used to calculate LPDDR4 mr2-OP[5:3]
					 * set in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 * Determine values based on your DRAM part's supported
					 * speed and latency bin.
					 * Default: calculated based on user_input_basic.frequency
					 * and "JEDEC JESD209-4A (LPDDR4)" Table 28 "Read and Write
					 * Latencies".
					 * Lowest latency selected when more than one latency can be
					 * used.
					 */

	uint32_t lp4wls;		/*
					 * LPDDR4 Dram WL Set.
					 * Applicable only if dramtype == LPDDR4.
					 * This variable is used to calculate LPDDR4 mr2-OP[6] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 * Determine value based on Memory controllers requirement
					 * of DRAM State after PHY training.
					 *
					 * Value | Description
					 *   --- | ---
					 *   0x0 | WL Set "A" (default)
					 */

	uint32_t lp4dbird;		/*
					 * LPDDR4 Dram DBI-Read Enable.
					 * Applicable only if dramtype == LPDDR4.
					 * Determine if you require to using DBI for the given
					 * PState.
					 * If Read DBI is not used PHY receivers are turned off to
					 * save power.
					 * This variable is used to calculate LPDDR4 mr3-OP[6] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * PHY register DMIPinPresent is programmed based on this
					 * parameter.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 *
					 * Value | Description
					 *   --- | ---
					 *   0x0 | Disabled (default)
					 *   0x1 | Enabled
					 */

	uint32_t lp4dbiwr;		/*
					 * LPDDR4 Dram DBI-Write Enable.
					 * Applicable only if dramtype == LPDDR4.
					 * This variable is used to calculate LPDDR4 mr3-OP[7] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 *
					 * Value | Description
					 *   --- | ---
					 *   0x0 | Disabled (default)
					 *   0x1 | Enabled
					 */

	uint32_t lp4nwr;		/*
					 * LPDDR4 Write-Recovery for Auto- Pre-charge commands.
					 * Applicable only if dramtype == LPDDR4.
					 * This variable is used to calculate LPDDR4 mr1-OP[6:4] set
					 * in the messageBlock.
					 * The training firmware will set DRAM MR according to MR
					 * value in the messageBlock at the end of training.
					 * Please refer to JEDEC JESD209-4A (LPDDR4) Spec for
					 * definition of MR.
					 * Determine values based on your DRAM part's supported
					 * speed and latency bin.
					 * Default: calculated based on user_input_basic.frequency
					 * and "JEDEC JESD209-4A (LPDDR4)" Table 28 "Read and Write
					 * Latencies".
					 * Lowest latency selected when more than one latency can be
					 * used.
					 *
					 * Binary Value | Description
					 * --- | ---
					 * 000 | nWR = 6 (default)
					 * 001 | nWR = 10
					 * 010 | nWR = 16
					 * 011 | nWR = 20
					 * 100 | nWR = 24
					 * 101 | nWR = 30
					 * 110 | nWR = 34
					 * 111 | nWR = 40
					 */

	uint32_t lp4lowpowerdrv;	/*
					 * Configure output Driver in Low power mode.
					 * Feature only supported for Hard Macro Family E (HME).
					 * Use NMOS Pull-up for Low-Power IO.
					 * Not valid here
					 */

	uint32_t drambyteswap;		/*
					 * DRAM Oscillator count source mapping for skip_training.
					 * The PHY supports swapping of DRAM oscillator count values
					 * between paired DBytes for the purpose of tDQSDQ DRAM
					 * Drift Compensation(DDC).
					 * Each DByte has a register bit to control the source of
					 * the oscillator count value used to perform tDQSDQ Drift
					 * compensation.
					 * On silicon the training firmware will determine the DByte
					 * swap and program PptCtlStatic register to select
					 * oscillator count source. When skip_train is used,
					 * training firmware is skipped thus manual programming may
					 * be required depending on configuration.
					 * The default hardware configuration is for odd Dbyte
					 * instance n to use oscillator count values from its paired
					 * Dbyte instance n-1. So Dbyte1 will use the oscillator
					 * count values from Dbyte0, Dbyte3 will use Dbyte2 and so
					 * on. This is required for DRAM Data width =16.
					 * Each bit of this field corresponds to a DBYTE:
					 *   - bit-0 = setting for DBYTE0
					 *   - bit-1 = setting for DBYTE1
					 *   - bit-2 = setting for DBYTE2
					 *   - . . .
					 *   - bit-n = setting for DBYTEn
					 * By setting the associated bit for each DByte to 1, PHY
					 * will use non-default source for count value.
					 *   - for even Dbytes, non-default source is to use the odd
					 *     pair count value.
					 *   - for odd Dbytes, no-default source to use data
					 *     received directly from the DRAM.
					 * Byte swapping must be the same across different ranks.
					 * Default value: 0x0
					 * If Byte mode devices are indicated via the x8mode
					 * messageBlock parameter, this variable is ignored as PHY
					 * only supports a limited configuration set based on Byte
					 * mode configuration.
					 *
					 * Example:
					 * DramByteSwap = 0x03 - Dbyte0: use count values from
					 * Dbyte1, Dbyte1 uses count values received directly
					 * received from DRAM.
					 * Rest of Dbytes have default source for DRAM oscilator
					 * count.
					 */

	uint32_t rxenbackoff;		/*
					 * Determines the Placement of PHY Read Gate signal.
					 * Only used in LPDDR4 when lp4rxpreamblemode==0 (static
					 * preamble) for skip_train==true.
					 * For other dramtypes or LPDDR4-toggling-preamble no
					 * options are available and PhyInit will set position as
					 * required. See source code in
					 * ddrphy_phyinit_c_initphyconfig() to see how the
					 * RxEnBackOff register is set.
					 * For skip_train==false, FW will set the position based on
					 * Preamble.
					 * We recommend keeping this setting at default value.
					 * SI analysis is required to determine if default value
					 * needs to be changed.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x1 | Position read gate 1UI from the first valid edge
					 *       | of DQS_t (LPDDR4 Static preamble only) (default)
					 */

	uint32_t trainsequencectrl;	/*
					 * Firmware Training Sequence Control.
					 * This input is used to program sequencectrl in
					 * messageBlock.
					 * It controls the training stages executed by firmware.
					 * For production silicon we recommend to use default value
					 * programmed by PhyInit.
					 */

	uint32_t snpsumctlopt;		/*
					 * Enable Fast Frequency Change (FFC) Optimizations
					 * specific to UMCTL2 (DDRCTRL).
					 * Not valid for dimmtype=NODIMM.
					 * Consult DDRCTRL documentation in Reference Manual to
					 * ensure when optimizations can be enabled.
					 *
					 * Value | Description
					 * ----- | ---
					 * 0 | Disable FFC MRW optimization (default)
					 */

	uint32_t snpsumctlf0rc5x;	/*
					 * F0RX5x RCD Control Word when using Fast Frequency
					 * Change(FFC) optimizations specific to UMCTL2
					 * Not valid for dimmtype=NODIMM.
					 * Only valid for when SnpsUmctlOpt=1.
					 * When UMCTL2 optimizations are enabled PHY will perform
					 * RCD MRW during fast frequency change request.
					 * The correct RCD control word value for each PState must
					 * be programmed in this field.
					 * Consult the RCD spec and UMCTL documentation to
					 * determine the correct value based on DRAM configuration
					 * and operating speed.
					 */

	uint32_t txslewrisedq;		/*
					 * Pull-up slew rate control for DBYTE Tx.
					 * Value specified here will be written to register
					 * TxSlewRate.TxPreP by PhyInit.
					 * See register description for more information.
					 */

	uint32_t txslewfalldq;		/*
					 * Pull-down slew rate control for DBYTE Tx.
					 * Value specified here will be written to
					 * TxSlewRate.TxPreN by PhyInit.
					 * See register description for more information.
					 */

	uint32_t txslewriseac;		/*
					 * Pull-up slew rate control for ANIB Tx.
					 * Value specified here will be written to
					 * ATxSlewRate.ATxPreP.
					 * See register description for more information.
					 */

	uint32_t txslewfallac;		/*
					 * Pull-down slew rate control for ANIB Tx.
					 * Value specified here will be written to
					 * ATxSlewRate.ATxPreN.
					 * See register description for more information.
					 */

	uint32_t disableretraining;	/*
					 * Disable PHY DRAM Drift compensation re-training.
					 * Only applied to LPDDR4. No retraining is required in
					 * DDR4/3.
					 * Disable PHY re-training during DFI frequency change
					 * requests in LPDDR4.
					 * The purpose of retraining is to compensate for drift in
					 * the DRAM.
					 * Determine based on SI analysis and DRAM datasheet if
					 * retraining can be disabled.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x1 | Disable retraining
					 *   0x0 | Enable retraining
					 */

	uint32_t disablephyupdate;	/*
					 * Disable DFI PHY Update feature.
					 * Only effects LPDDR4.
					 * Disable DFI PHY Update feature. When set PHY will not
					 * assert dfi0/1_phyupd_req.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x1 | Disable DFI PHY Update
					 *   0x0 | Enable DFI PHY Update
					 */

	uint32_t enablehighclkskewfix;	/*
					 * Enable alternative PIE program.
					 * If enabled the PIE reinitializes the FIFO pointers a
					 * second time due for designs with large skew between
					 * chiplet DfiClk branches. If enabled PIE latencies in all
					 * protocols are increased by 60 DfiClks.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x0 | Disable (default)
					 */

	uint32_t disableunusedaddrlns;  /*
					 * Turn off or tristate Address Lanes when possible.
					 *
					 * When enabled, PHY will tristate unused address lanes to
					 * save power when possible by using Acx4AnibDis and
					 * AForceTriCont registers.
					 * This feature is only implemented for the default PHY
					 * Address bump mapping and Ranks must be populated in
					 * order. ie Rank1 cannot be used if Rank0 is unpopulated.
					 * For alternative bump mapping follow the following
					 * guideline to achieve maximum power savings:
					 *   - For each unused BP_A bump program AForceTriCont[4:0]
					 *     bits based on register description.
					 *   - if all lanes of an Anib are unused _AND_ ANIB is not
					 *     the first or last instance set bit associated with
					 *     the instance in Acs4AnibDis registers. see register
					 *     description for details.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x1 | Enable
					 */

	uint32_t phyinitsequencenum;	/*
					 * Switches between supported phyinit training sequences.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x0 | Minimizes number of Imem/Dmem loads (default)
					 */

	uint32_t enabledficspolarityfix;/*
					 * Enable alternative PIE program.
					 * Set to 1 if PUB_VERSION <2.43a, otherwise set to 0. If
					 * enabled the PIE programs Dfi{Rd,Wr}DataCsDestMap CSR's
					 * to default values 0x00E4 before running PPT.
					 * Before exiting PPT, PIE will restore
					 * Dfi{Rd,Wr}DataCsDestMap CSR's to 0x00E1.
					 *
					 * Value | Description
					 * ----- | ---
					 *   0x0 | Disable (default)
					 */

	uint32_t phyvref;		/*
					 * Must be programmed with the Vref level to be used by the
					 * PHY during reads.
					 * The units of this field are a percentage of VDDQ
					 * according to the following equation:
					 * Receiver Vref = VDDQ*phyvref[6:0]/128
					 * For example to set Vref at 0.75*VDDQ, set this field to
					 * 0x60.
					 * For digital simulation, any legal value can be used. For
					 * silicon, the users must calculate the analytical Vref by
					 * using the impedances, terminations, and series resistance
					 * present in the system.
					 */

	uint32_t sequencectrl;		/*
					 * Controls the training steps to be run. Each bit
					 * corresponds to a training step.
					 * If the bit is set to 1, the training step will run.
					 * If the bit is set to 0, the training step will be
					 * skipped.
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
};

/*
 * Structure for mode register user inputs
 *
 * The following data structure must be set and completed correctly so that the PhyInit software
 * package can accurate fill message block structure.
 * Only some mrx are used per DDR type, on related width:
 * - DDR3: mr0..2 are used (16-bits values)
 * - DDR4: mr0..6 are used (16-bits values)
 * - LPDDR4: mr1..4 and mr11..22 are used (8-bits values)
 */
struct user_input_mode_register {
	uint32_t mr0;
	uint32_t mr1;
	uint32_t mr2;
	uint32_t mr3;
	uint32_t mr4;
	uint32_t mr5;
	uint32_t mr6;
	uint32_t mr11;
	uint32_t mr12;
	uint32_t mr13;
	uint32_t mr14;
	uint32_t mr22;
};

/*
 * Structure for swizzle user inputs
 *
 * The following data structure must be set and completed correctly sothat the PhyInit software
 * package can accurate set swizzle (IO muxing) config.
 * Only some swizzles are used per DDR type:
 * - DDR3/DDR4: swizzle 0..32 are used
 *   - 26 for hwtswizzle
 *   - 7 for acswizzle
 * - LPDDR4:  swizzle 0..43 are used
 *   - 8 per byte for dqlnsel (total 32)
 *   - 6 for mapcaatodfi
 *   - 6 for mapcabtodfi
 */
#define NB_HWT_SWIZZLE			26U
#define NB_AC_SWIZZLE			7U
#define NB_DQLNSEL_SWIZZLE_PER_BYTE	8U
#define NB_MAPCAATODFI_SWIZZLE		6U
#define NB_MAPCABTODFI_SWIZZLE		6U
#define NB_SWIZZLE	44
struct user_input_swizzle {
	uint32_t swizzle[NB_SWIZZLE];
};

#endif /* DDRPHY_PHYINIT_STRUCT_H */

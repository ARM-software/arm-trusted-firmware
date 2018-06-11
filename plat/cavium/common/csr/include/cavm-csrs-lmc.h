#ifndef __CAVM_CSRS_LMC_H__
#define __CAVM_CSRS_LMC_H__
/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * Cavium LMC.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration lmc_bar_e
 *
 * LMC Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_LMC_BAR_E_LMCX_PF_BAR0(a) (0x87e088000000ll + 0x1000000ll * (a))
#define CAVM_LMC_BAR_E_LMCX_PF_BAR0_SIZE 0x800000ull
#define CAVM_LMC_BAR_E_LMCX_PF_BAR4(a) (0x87e088f00000ll + 0x1000000ll * (a))
#define CAVM_LMC_BAR_E_LMCX_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration lmc_int_vec_e
 *
 * LMC MSI-X Vector Enumeration
 * Enumerates the MSI-X interrupt vectors.
 */
#define CAVM_LMC_INT_VEC_E_INTS (0)

/**
 * Enumeration lmc_seq_sel_e
 *
 * LMC Sequence Select Enumeration
 * Enumerates the different values of LMC()_SEQ_CTL[SEQ_SEL].
 */
#define CAVM_LMC_SEQ_SEL_E_INIT (0)
#define CAVM_LMC_SEQ_SEL_E_MPR_RW (9)
#define CAVM_LMC_SEQ_SEL_E_MRW (8)
#define CAVM_LMC_SEQ_SEL_E_OFFSET_TRAINING (0xb)
#define CAVM_LMC_SEQ_SEL_E_PPR (0xf)
#define CAVM_LMC_SEQ_SEL_E_RCD_INIT (7)
#define CAVM_LMC_SEQ_SEL_E_READ_LEVEL (1)
#define CAVM_LMC_SEQ_SEL_E_RW_TRAINING (0xe)
#define CAVM_LMC_SEQ_SEL_E_SREF_ENTRY (2)
#define CAVM_LMC_SEQ_SEL_E_SREF_EXIT (3)
#define CAVM_LMC_SEQ_SEL_E_VREF_INT (0xa)
#define CAVM_LMC_SEQ_SEL_E_WRITE_LEVEL (6)

/**
 * Register (RSL) lmc#_config
 *
 * LMC Memory Configuration Register
 * This register controls certain parameters required for memory configuration. Note the
 * following:
 * * Priority order for hardware write operations to
 * LMC()_CONFIG/LMC()_FADR/LMC()_ECC_SYND: DED error > SEC error.
 * * The self-refresh entry sequence(s) power the DLL up/down (depending on
 * LMC()_MODEREG_PARAMS0[DLL]) when LMC()_CONFIG[SREF_WITH_DLL] is set.
 * * Prior to the self-refresh exit sequence, LMC()_MODEREG_PARAMS0 should be reprogrammed
 * (if needed) to the appropriate values.
 *
 * See LMC Initialization Sequence for the LMC bring-up sequence.
 */
union cavm_lmcx_config
{
    uint64_t u;
    struct cavm_lmcx_config_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t lrdimm_ena            : 1;  /**< [ 63: 63](R/W) Reserved.
                                                                 Internal:
                                                                 Load Reduced DIMM Enable. When set allows the use of JEDEC DDR4 LRDIMMs. */
        uint64_t bg2_enable            : 1;  /**< [ 62: 62](R/W) BG1 enable bit. Only has an effect when LMC()_CONFIG[MODEDDR4] = 1.
                                                                 Set to 1 when using DDR4 x4 or x8 parts.
                                                                 Clear to 0 when using DDR4 x16 parts. */
        uint64_t mode_x4dev            : 1;  /**< [ 61: 61](R/W) DDR *4 device mode. */
        uint64_t mode32b               : 1;  /**< [ 60: 60](R/W) 32-bit datapath mode. When set, only 32 DQ pins are used. */
        uint64_t scrz                  : 1;  /**< [ 59: 59](R/W1S/H) Hide LMC()_SCRAMBLE_CFG0 and LMC()_SCRAMBLE_CFG1 when set. */
        uint64_t early_unload_d1_r1    : 1;  /**< [ 58: 58](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 3 reads.
                                                                 The recommended EARLY_UNLOAD_D1_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK3[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 3 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK3[BYTEi]) across all i), then set EARLY_UNLOAD_D1_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r0    : 1;  /**< [ 57: 57](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 2 reads.
                                                                 The recommended EARLY_UNLOAD_D1_RO value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK2[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 2 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK2[BYTEi]) across all i), then set EARLY_UNLOAD_D1_RO when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_RO = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r1    : 1;  /**< [ 56: 56](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 1 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK1[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 1 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK1[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r0    : 1;  /**< [ 55: 55](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 0 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R0 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK0[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 0 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK0[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R0 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R0 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t init_status           : 4;  /**< [ 54: 51](RO/H) Indicates status of initialization. [INIT_STATUS][n] = 1 implies rank n has been
                                                                 initialized.
                                                                 Software must set necessary [RANKMASK] bits before executing the initialization sequence
                                                                 using LMC()_SEQ_CTL. If the rank has been selected for init with the [RANKMASK] bits,
                                                                 the [INIT_STATUS] bits will be set after successful initialization and after self-refresh
                                                                 exit. [INIT_STATUS] determines the chip-selects that assert during refresh, ZQCS,
                                                                 precharge
                                                                 power-down entry/exit, and self-refresh entry SEQ_SELs. */
        uint64_t mirrmask              : 4;  /**< [ 50: 47](R/W) "Mask determining which ranks are address-mirrored.
                                                                 [MIRRMASK]<n> = 1 means rank n addresses are mirrored for
                                                                 0 <= n <= 3.
                                                                 In DDR3, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 When RANK_ENA = 0, MIRRMASK<1> MBZ.

                                                                 In DDR4, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BG<1> is swapped with DDR#_BG<0>.
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<13> is swapped with DDR#_A<11>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 For CN70XX, MIRRMASK<3:2> MBZ.
                                                                 * When RANK_ENA = 0, MIRRMASK<1> MBZ." */
        uint64_t rankmask              : 4;  /**< [ 46: 43](R/W) Mask to select rank to be leveled/initialized. To write-level/read-level/initialize rank
                                                                 i, set [RANKMASK]<i>:

                                                                 <pre>
                                                                               RANK_ENA = 1   RANK_ENA = 0
                                                                 RANKMASK<0> = DIMM0_CS0      DIMM0_CS0
                                                                 RANKMASK<1> = DIMM0_CS1      MBZ
                                                                 RANKMASK<2> = Reserved       Reserved
                                                                 RANKMASK<3> = Reserved       Reserved
                                                                 </pre>

                                                                 For read/write leveling, each rank has to be leveled separately, so [RANKMASK] should only
                                                                 have one bit set. [RANKMASK] is not used during self-refresh entry/exit and precharge
                                                                 power-
                                                                 down entry/exit instruction sequences. When [RANK_ENA] = 0, [RANKMASK]<1> and
                                                                 [RANKMASK]<3> MBZ. */
        uint64_t rank_ena              : 1;  /**< [ 42: 42](R/W) "RANK enable (for use with dual-rank DIMMs).
                                                                 * For dual-rank DIMMs, the [RANK_ENA] bit will enable the drive of the DDR#_DIMM*_CS*_L
                                                                 and
                                                                 ODT_<1:0> pins differently based on the ([PBANK_LSB] - 1) address bit.
                                                                 * Write 0 for SINGLE ranked DIMMs." */
        uint64_t sref_with_dll         : 1;  /**< [ 41: 41](R/W) Self-refresh entry/exit write mode registers. When set, self-refresh entry sequence writes
                                                                 MR2 and MR1 (in this order, in all ranks), and self-refresh exit sequence writes MR1, MR0,
                                                                 MR2, and MR3 (in this order, for all ranks). The write operations occur before self-
                                                                 refresh entry, and after self-refresh exit. When clear, self-refresh entry and exit
                                                                 instruction sequences do not write any mode registers in the DDR3/4 parts. */
        uint64_t early_dqx             : 1;  /**< [ 40: 40](R/W) Set this bit to send DQx signals one CK cycle earlier for the case when the shortest DQx
                                                                 lines have a larger delay than the CK line. */
        uint64_t ref_zqcs_int          : 22; /**< [ 39: 18](R/W) Refresh interval is represented in number of 512 CK cycle increments. To get more precise
                                                                 control of the refresh interval, LMC()_EXT_CONFIG[REF_INT_LSBS] can be set to a
                                                                 nonzero value.
                                                                 ZQCS interval is represented in a number of refresh intervals. A refresh sequence is
                                                                 triggered when bits <24:18> are equal to 0x0, and a ZQCS sequence is triggered when
                                                                 <39:18>
                                                                 are equal to 0x0.

                                                                 The ZQCS timer only decrements when the refresh timer is 0.

                                                                 Program <24:18> to RND-DN(TREFI/clkPeriod/512).

                                                                 A value of 0 in bits <24:18> will effectively turn off refresh.

                                                                 Program <36:25> to (RND-DN(ZQCS_Period / Refresh_Period) - 1), where Refresh_Period is the
                                                                 effective period programmed in bits <24:18>. Note that this value should always be greater
                                                                 than 32, to account for resistor calibration delays.

                                                                 000_00000000_0000000: Reserved

                                                                 Max refresh interval = 127 * 512= 65024 CK cycles.

                                                                 Max ZQCS interval = 32768 * 127 * 512 = 2130706432 CK cycles.

                                                                 If refresh interval is programmed to ~8us, max ZQCS interval is ~262ms, or ~4 ZQCS
                                                                 operations per second.
                                                                 LMC()_CONFIG[INIT_STATUS] determines which ranks receive the REF / ZQCS. LMC does not
                                                                 send any refreshes / ZQCS's when LMC()_CONFIG[INIT_STATUS]=0. */
        uint64_t reset                 : 1;  /**< [ 17: 17](R/W) Reset one-shot pulse for LMC()_OPS_CNT, LMC()_IFB_CNT, and LMC()_DCLK_CNT.
                                                                 To cause the reset, software writes this to a 1, then rewrites it to a 0. */
        uint64_t ecc_adr               : 1;  /**< [ 16: 16](R/W) Include memory reference address in the ECC calculation.
                                                                 0 = disabled, 1 = enabled. */
        uint64_t forcewrite            : 4;  /**< [ 15: 12](R/W) Force the oldest outstanding write to complete after having waited for 2^[FORCEWRITE] CK
                                                                 cycles. 0 = disabled. */
        uint64_t idlepower             : 3;  /**< [ 11:  9](R/W) Enter precharge power-down mode after the memory controller has been idle for
                                                                 2^(2+[IDLEPOWER]) CK cycles. 0 = disabled.

                                                                 This field should only be programmed after initialization.
                                                                 LMC()_MODEREG_PARAMS0[PPD] determines whether the DRAM DLL is disabled during the
                                                                 precharge power-down. */
        uint64_t pbank_lsb             : 4;  /**< [  8:  5](R/W) DIMM address bit select. Reverting to the explanation for [ROW_LSB], [PBANK_LSB] would be:
                                                                 [ROW_LSB] bit + num_rowbits + num_rankbits

                                                                 Decoding for PBANK_LSB:

                                                                 0x0: DIMM = mem_adr<28>; if [RANK_ENA]=1, rank = mem_adr<27>.
                                                                 0x1: DIMM = mem_adr<29>; if [RANK_ENA]=1, rank = mem_adr<28>.
                                                                 0x2: DIMM = mem_adr<30>; if [RANK_ENA]=1, rank = mem_adr<29>.
                                                                 0x3: DIMM = mem_adr<31>; if [RANK_ENA]=1, rank = mem_adr<30>.
                                                                 0x4: DIMM = mem_adr<32>; if [RANK_ENA]=1, rank = mem_adr<31>.
                                                                 0x5: DIMM = mem_adr<33>; if [RANK_ENA]=1, rank = mem_adr<32>.
                                                                 0x6: DIMM = mem_adr<34>; if [RANK_ENA]=1, rank = mem_adr<33>.
                                                                 0x7: DIMM = mem_adr<35>; if [RANK_ENA]=1, rank = mem_adr<34>.
                                                                 0x8: DIMM = mem_adr<36>; if [RANK_ENA]=1, rank = mem_adr<35>.
                                                                 0x9: DIMM = mem_adr<37>; if [RANK_ENA]=1, rank = mem_adr<36>.
                                                                 0xA: DIMM = 0;           if [RANK_ENA]=1, rank = mem_adr<37>.
                                                                 0xB-0xF: Reserved.

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1Gb (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 With [RANK_ENA] = 0, [PBANK_LSB] = 2.
                                                                 With [RANK_ENA] = 1, [PBANK_LSB] = 3.

                                                                 Internal:
                                                                 When interfacing with 8H 3DS, set this 0xA regardless of [RANK_ENA] value. */
        uint64_t row_lsb               : 3;  /**< [  4:  2](R/W) Row address bit select.
                                                                 0x0 = Address bit 14 is LSB.
                                                                 0x1 = Address bit 15 is LSB.
                                                                 0x2 = Address bit 16 is LSB.
                                                                 0x3 = Address bit 17 is LSB.
                                                                 0x4 = Address bit 18 is LSB.
                                                                 0x5 = Address bit 19 is LSB.
                                                                 0x6 = Address bit 20 is LSB.
                                                                 0x6 = Reserved.

                                                                 Encoding used to determine which memory address bit position represents the low order DDR
                                                                 ROW address. The processor's memory address<34:7> needs to be translated to DRAM addresses
                                                                 (bnk,row,col,rank and DIMM) and that is a function of the following:
                                                                 * Datapath width (64).
                                                                 * Number of banks (8).
                                                                 * Number of column bits of the memory part--specified indirectly by this register.
                                                                 * Number of row bits of the memory part--specified indirectly by [PBANK_LSB].
                                                                 * Number of ranks in a DIMM--specified by [RANK_ENA].
                                                                 * Number of DIMMs in the system by the register below ([PBANK_LSB]).

                                                                 Column address starts from mem_addr[3] for 64b (8Bytes) DQ width. [ROW_LSB] is
                                                                 mem_adr[15] for 64b mode. Therefore, the [ROW_LSB] parameter should be set to
                                                                 0x1 (64b).

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1GB (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 Refer to Cache-block Read Transaction Example, Cache-block Read Transaction Example. */
        uint64_t ecc_ena               : 1;  /**< [  1:  1](R/W) ECC enable. When set, enables the 8b ECC check/correct logic. Should be 1 when used with
                                                                 DIMMs with ECC; 0, otherwise.

                                                                 * When this mode is turned on, DQ<71:64> on write operations contains the ECC code
                                                                 generated for the 64 bits of data which will be written in the memory. Later on read
                                                                 operations, will be used to check for single-bit error (which will be auto-corrected) and
                                                                 double-bit error (which will be reported).

                                                                 * When not turned on, DQ<71:64> are driven to 0. Please refer to SEC_ERR, DED_ERR,
                                                                 LMC()_FADR, and LMC()_ECC_SYND registers for diagnostics information when there is
                                                                 an error. */
        uint64_t reserved_0            : 1;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0            : 1;
        uint64_t ecc_ena               : 1;  /**< [  1:  1](R/W) ECC enable. When set, enables the 8b ECC check/correct logic. Should be 1 when used with
                                                                 DIMMs with ECC; 0, otherwise.

                                                                 * When this mode is turned on, DQ<71:64> on write operations contains the ECC code
                                                                 generated for the 64 bits of data which will be written in the memory. Later on read
                                                                 operations, will be used to check for single-bit error (which will be auto-corrected) and
                                                                 double-bit error (which will be reported).

                                                                 * When not turned on, DQ<71:64> are driven to 0. Please refer to SEC_ERR, DED_ERR,
                                                                 LMC()_FADR, and LMC()_ECC_SYND registers for diagnostics information when there is
                                                                 an error. */
        uint64_t row_lsb               : 3;  /**< [  4:  2](R/W) Row address bit select.
                                                                 0x0 = Address bit 14 is LSB.
                                                                 0x1 = Address bit 15 is LSB.
                                                                 0x2 = Address bit 16 is LSB.
                                                                 0x3 = Address bit 17 is LSB.
                                                                 0x4 = Address bit 18 is LSB.
                                                                 0x5 = Address bit 19 is LSB.
                                                                 0x6 = Address bit 20 is LSB.
                                                                 0x6 = Reserved.

                                                                 Encoding used to determine which memory address bit position represents the low order DDR
                                                                 ROW address. The processor's memory address<34:7> needs to be translated to DRAM addresses
                                                                 (bnk,row,col,rank and DIMM) and that is a function of the following:
                                                                 * Datapath width (64).
                                                                 * Number of banks (8).
                                                                 * Number of column bits of the memory part--specified indirectly by this register.
                                                                 * Number of row bits of the memory part--specified indirectly by [PBANK_LSB].
                                                                 * Number of ranks in a DIMM--specified by [RANK_ENA].
                                                                 * Number of DIMMs in the system by the register below ([PBANK_LSB]).

                                                                 Column address starts from mem_addr[3] for 64b (8Bytes) DQ width. [ROW_LSB] is
                                                                 mem_adr[15] for 64b mode. Therefore, the [ROW_LSB] parameter should be set to
                                                                 0x1 (64b).

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1GB (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 Refer to Cache-block Read Transaction Example, Cache-block Read Transaction Example. */
        uint64_t pbank_lsb             : 4;  /**< [  8:  5](R/W) DIMM address bit select. Reverting to the explanation for [ROW_LSB], [PBANK_LSB] would be:
                                                                 [ROW_LSB] bit + num_rowbits + num_rankbits

                                                                 Decoding for PBANK_LSB:

                                                                 0x0: DIMM = mem_adr<28>; if [RANK_ENA]=1, rank = mem_adr<27>.
                                                                 0x1: DIMM = mem_adr<29>; if [RANK_ENA]=1, rank = mem_adr<28>.
                                                                 0x2: DIMM = mem_adr<30>; if [RANK_ENA]=1, rank = mem_adr<29>.
                                                                 0x3: DIMM = mem_adr<31>; if [RANK_ENA]=1, rank = mem_adr<30>.
                                                                 0x4: DIMM = mem_adr<32>; if [RANK_ENA]=1, rank = mem_adr<31>.
                                                                 0x5: DIMM = mem_adr<33>; if [RANK_ENA]=1, rank = mem_adr<32>.
                                                                 0x6: DIMM = mem_adr<34>; if [RANK_ENA]=1, rank = mem_adr<33>.
                                                                 0x7: DIMM = mem_adr<35>; if [RANK_ENA]=1, rank = mem_adr<34>.
                                                                 0x8: DIMM = mem_adr<36>; if [RANK_ENA]=1, rank = mem_adr<35>.
                                                                 0x9: DIMM = mem_adr<37>; if [RANK_ENA]=1, rank = mem_adr<36>.
                                                                 0xA: DIMM = 0;           if [RANK_ENA]=1, rank = mem_adr<37>.
                                                                 0xB-0xF: Reserved.

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1Gb (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 With [RANK_ENA] = 0, [PBANK_LSB] = 2.
                                                                 With [RANK_ENA] = 1, [PBANK_LSB] = 3.

                                                                 Internal:
                                                                 When interfacing with 8H 3DS, set this 0xA regardless of [RANK_ENA] value. */
        uint64_t idlepower             : 3;  /**< [ 11:  9](R/W) Enter precharge power-down mode after the memory controller has been idle for
                                                                 2^(2+[IDLEPOWER]) CK cycles. 0 = disabled.

                                                                 This field should only be programmed after initialization.
                                                                 LMC()_MODEREG_PARAMS0[PPD] determines whether the DRAM DLL is disabled during the
                                                                 precharge power-down. */
        uint64_t forcewrite            : 4;  /**< [ 15: 12](R/W) Force the oldest outstanding write to complete after having waited for 2^[FORCEWRITE] CK
                                                                 cycles. 0 = disabled. */
        uint64_t ecc_adr               : 1;  /**< [ 16: 16](R/W) Include memory reference address in the ECC calculation.
                                                                 0 = disabled, 1 = enabled. */
        uint64_t reset                 : 1;  /**< [ 17: 17](R/W) Reset one-shot pulse for LMC()_OPS_CNT, LMC()_IFB_CNT, and LMC()_DCLK_CNT.
                                                                 To cause the reset, software writes this to a 1, then rewrites it to a 0. */
        uint64_t ref_zqcs_int          : 22; /**< [ 39: 18](R/W) Refresh interval is represented in number of 512 CK cycle increments. To get more precise
                                                                 control of the refresh interval, LMC()_EXT_CONFIG[REF_INT_LSBS] can be set to a
                                                                 nonzero value.
                                                                 ZQCS interval is represented in a number of refresh intervals. A refresh sequence is
                                                                 triggered when bits <24:18> are equal to 0x0, and a ZQCS sequence is triggered when
                                                                 <39:18>
                                                                 are equal to 0x0.

                                                                 The ZQCS timer only decrements when the refresh timer is 0.

                                                                 Program <24:18> to RND-DN(TREFI/clkPeriod/512).

                                                                 A value of 0 in bits <24:18> will effectively turn off refresh.

                                                                 Program <36:25> to (RND-DN(ZQCS_Period / Refresh_Period) - 1), where Refresh_Period is the
                                                                 effective period programmed in bits <24:18>. Note that this value should always be greater
                                                                 than 32, to account for resistor calibration delays.

                                                                 000_00000000_0000000: Reserved

                                                                 Max refresh interval = 127 * 512= 65024 CK cycles.

                                                                 Max ZQCS interval = 32768 * 127 * 512 = 2130706432 CK cycles.

                                                                 If refresh interval is programmed to ~8us, max ZQCS interval is ~262ms, or ~4 ZQCS
                                                                 operations per second.
                                                                 LMC()_CONFIG[INIT_STATUS] determines which ranks receive the REF / ZQCS. LMC does not
                                                                 send any refreshes / ZQCS's when LMC()_CONFIG[INIT_STATUS]=0. */
        uint64_t early_dqx             : 1;  /**< [ 40: 40](R/W) Set this bit to send DQx signals one CK cycle earlier for the case when the shortest DQx
                                                                 lines have a larger delay than the CK line. */
        uint64_t sref_with_dll         : 1;  /**< [ 41: 41](R/W) Self-refresh entry/exit write mode registers. When set, self-refresh entry sequence writes
                                                                 MR2 and MR1 (in this order, in all ranks), and self-refresh exit sequence writes MR1, MR0,
                                                                 MR2, and MR3 (in this order, for all ranks). The write operations occur before self-
                                                                 refresh entry, and after self-refresh exit. When clear, self-refresh entry and exit
                                                                 instruction sequences do not write any mode registers in the DDR3/4 parts. */
        uint64_t rank_ena              : 1;  /**< [ 42: 42](R/W) "RANK enable (for use with dual-rank DIMMs).
                                                                 * For dual-rank DIMMs, the [RANK_ENA] bit will enable the drive of the DDR#_DIMM*_CS*_L
                                                                 and
                                                                 ODT_<1:0> pins differently based on the ([PBANK_LSB] - 1) address bit.
                                                                 * Write 0 for SINGLE ranked DIMMs." */
        uint64_t rankmask              : 4;  /**< [ 46: 43](R/W) Mask to select rank to be leveled/initialized. To write-level/read-level/initialize rank
                                                                 i, set [RANKMASK]<i>:

                                                                 <pre>
                                                                               RANK_ENA = 1   RANK_ENA = 0
                                                                 RANKMASK<0> = DIMM0_CS0      DIMM0_CS0
                                                                 RANKMASK<1> = DIMM0_CS1      MBZ
                                                                 RANKMASK<2> = Reserved       Reserved
                                                                 RANKMASK<3> = Reserved       Reserved
                                                                 </pre>

                                                                 For read/write leveling, each rank has to be leveled separately, so [RANKMASK] should only
                                                                 have one bit set. [RANKMASK] is not used during self-refresh entry/exit and precharge
                                                                 power-
                                                                 down entry/exit instruction sequences. When [RANK_ENA] = 0, [RANKMASK]<1> and
                                                                 [RANKMASK]<3> MBZ. */
        uint64_t mirrmask              : 4;  /**< [ 50: 47](R/W) "Mask determining which ranks are address-mirrored.
                                                                 [MIRRMASK]<n> = 1 means rank n addresses are mirrored for
                                                                 0 <= n <= 3.
                                                                 In DDR3, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 When RANK_ENA = 0, MIRRMASK<1> MBZ.

                                                                 In DDR4, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BG<1> is swapped with DDR#_BG<0>.
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<13> is swapped with DDR#_A<11>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 For CN70XX, MIRRMASK<3:2> MBZ.
                                                                 * When RANK_ENA = 0, MIRRMASK<1> MBZ." */
        uint64_t init_status           : 4;  /**< [ 54: 51](RO/H) Indicates status of initialization. [INIT_STATUS][n] = 1 implies rank n has been
                                                                 initialized.
                                                                 Software must set necessary [RANKMASK] bits before executing the initialization sequence
                                                                 using LMC()_SEQ_CTL. If the rank has been selected for init with the [RANKMASK] bits,
                                                                 the [INIT_STATUS] bits will be set after successful initialization and after self-refresh
                                                                 exit. [INIT_STATUS] determines the chip-selects that assert during refresh, ZQCS,
                                                                 precharge
                                                                 power-down entry/exit, and self-refresh entry SEQ_SELs. */
        uint64_t early_unload_d0_r0    : 1;  /**< [ 55: 55](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 0 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R0 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK0[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 0 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK0[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R0 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R0 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r1    : 1;  /**< [ 56: 56](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 1 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK1[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 1 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK1[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r0    : 1;  /**< [ 57: 57](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 2 reads.
                                                                 The recommended EARLY_UNLOAD_D1_RO value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK2[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 2 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK2[BYTEi]) across all i), then set EARLY_UNLOAD_D1_RO when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_RO = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r1    : 1;  /**< [ 58: 58](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 3 reads.
                                                                 The recommended EARLY_UNLOAD_D1_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK3[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 3 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK3[BYTEi]) across all i), then set EARLY_UNLOAD_D1_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t scrz                  : 1;  /**< [ 59: 59](R/W1S/H) Hide LMC()_SCRAMBLE_CFG0 and LMC()_SCRAMBLE_CFG1 when set. */
        uint64_t mode32b               : 1;  /**< [ 60: 60](R/W) 32-bit datapath mode. When set, only 32 DQ pins are used. */
        uint64_t mode_x4dev            : 1;  /**< [ 61: 61](R/W) DDR *4 device mode. */
        uint64_t bg2_enable            : 1;  /**< [ 62: 62](R/W) BG1 enable bit. Only has an effect when LMC()_CONFIG[MODEDDR4] = 1.
                                                                 Set to 1 when using DDR4 x4 or x8 parts.
                                                                 Clear to 0 when using DDR4 x16 parts. */
        uint64_t lrdimm_ena            : 1;  /**< [ 63: 63](R/W) Reserved.
                                                                 Internal:
                                                                 Load Reduced DIMM Enable. When set allows the use of JEDEC DDR4 LRDIMMs. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_lmcx_config_s cn81xx; */
    struct cavm_lmcx_config_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t lrdimm_ena            : 1;  /**< [ 63: 63](R/W) Reserved.
                                                                 Internal:
                                                                 Load Reduced DIMM Enable. When set allows the use of JEDEC DDR4 LRDIMMs. */
        uint64_t bg2_enable            : 1;  /**< [ 62: 62](R/W) BG1 enable bit. Only has an effect when LMC()_CONFIG[MODEDDR4] = 1.
                                                                 Set to 1 when using DDR4 x4 or x8 parts.
                                                                 Clear to 0 when using DDR4 x16 parts. */
        uint64_t mode_x4dev            : 1;  /**< [ 61: 61](R/W) DDR *4 device mode. */
        uint64_t mode32b               : 1;  /**< [ 60: 60](R/W) 32-bit datapath mode. When set, only 32 DQ pins are used. */
        uint64_t scrz                  : 1;  /**< [ 59: 59](R/W1S/H) Hide LMC()_SCRAMBLE_CFG0 and LMC()_SCRAMBLE_CFG1 when set. */
        uint64_t early_unload_d1_r1    : 1;  /**< [ 58: 58](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 3 reads.
                                                                 The recommended EARLY_UNLOAD_D1_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK3[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 3 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK3[BYTEi]) across all i), then set EARLY_UNLOAD_D1_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r0    : 1;  /**< [ 57: 57](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 2 reads.
                                                                 The recommended EARLY_UNLOAD_D1_RO value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK2[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 2 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK2[BYTEi]) across all i), then set EARLY_UNLOAD_D1_RO when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_RO = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r1    : 1;  /**< [ 56: 56](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 1 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK1[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 1 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK1[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r0    : 1;  /**< [ 55: 55](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 0 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R0 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK0[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 0 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK0[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R0 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R0 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t init_status           : 4;  /**< [ 54: 51](RO/H) Indicates status of initialization. [INIT_STATUS][n] = 1 implies rank n has been
                                                                 initialized.
                                                                 Software must set necessary [RANKMASK] bits before executing the initialization sequence
                                                                 using LMC()_SEQ_CTL. If the rank has been selected for init with the [RANKMASK] bits,
                                                                 the [INIT_STATUS] bits will be set after successful initialization and after self-refresh
                                                                 exit. [INIT_STATUS] determines the chip-selects that assert during refresh, ZQCS,
                                                                 precharge
                                                                 power-down entry/exit, and self-refresh entry SEQ_SELs. */
        uint64_t mirrmask              : 4;  /**< [ 50: 47](R/W) "Mask determining which ranks are address-mirrored.
                                                                 [MIRRMASK]<n> = 1 means rank n addresses are mirrored for
                                                                 0 <= n <= 3.
                                                                 In DDR3, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 When RANK_ENA = 0, MIRRMASK<1> MBZ.

                                                                 In DDR4, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BG<1> is swapped with DDR#_BG<0>.
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<13> is swapped with DDR#_A<11>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 For CN70XX, MIRRMASK<3:2> MBZ.
                                                                 * When RANK_ENA = 0, MIRRMASK<1> MBZ." */
        uint64_t rankmask              : 4;  /**< [ 46: 43](R/W) Mask to select rank to be leveled/initialized. To write-level/read-level/initialize rank
                                                                 i, set [RANKMASK]<i>:

                                                                 <pre>
                                                                               RANK_ENA = 1   RANK_ENA = 0
                                                                 RANKMASK<0> = DIMM0_CS0      DIMM0_CS0
                                                                 RANKMASK<1> = DIMM0_CS1      MBZ
                                                                 RANKMASK<2> = DIMM1_CS0      DIMM1_CS0
                                                                 RANKMASK<3> = DIMM1_CS1      MBZ
                                                                 </pre>

                                                                 For read/write leveling, each rank has to be leveled separately, so [RANKMASK] should only
                                                                 have one bit set. [RANKMASK] is not used during self-refresh entry/exit and precharge
                                                                 power-
                                                                 down entry/exit instruction sequences. When [RANK_ENA] = 0, [RANKMASK]<1> and
                                                                 [RANKMASK]<3> MBZ. */
        uint64_t rank_ena              : 1;  /**< [ 42: 42](R/W) "RANK enable (for use with dual-rank DIMMs).
                                                                 * For dual-rank DIMMs, the [RANK_ENA] bit will enable the drive of the DDR#_DIMM*_CS*_L
                                                                 and
                                                                 ODT_<1:0> pins differently based on the ([PBANK_LSB] - 1) address bit.
                                                                 * Write 0 for SINGLE ranked DIMMs." */
        uint64_t sref_with_dll         : 1;  /**< [ 41: 41](R/W) Self-refresh entry/exit write mode registers. When set, self-refresh entry sequence writes
                                                                 MR2 and MR1 (in this order, in all ranks), and self-refresh exit sequence writes MR1, MR0,
                                                                 MR2, and MR3 (in this order, for all ranks). The write operations occur before self-
                                                                 refresh entry, and after self-refresh exit. When clear, self-refresh entry and exit
                                                                 instruction sequences do not write any mode registers in the DDR3/4 parts. */
        uint64_t early_dqx             : 1;  /**< [ 40: 40](R/W) Set this bit to send DQx signals one CK cycle earlier for the case when the shortest DQx
                                                                 lines have a larger delay than the CK line. */
        uint64_t ref_zqcs_int          : 22; /**< [ 39: 18](R/W) Refresh interval is represented in number of 512 CK cycle increments. To get more precise
                                                                 control of the refresh interval, LMC()_EXT_CONFIG[REF_INT_LSBS] can be set to a
                                                                 nonzero value.
                                                                 ZQCS interval is represented in a number of refresh intervals. A refresh sequence is
                                                                 triggered when bits <24:18> are equal to 0x0, and a ZQCS sequence is triggered when
                                                                 <39:18>
                                                                 are equal to 0x0.

                                                                 The ZQCS timer only decrements when the refresh timer is 0.

                                                                 Program <24:18> to RND-DN(TREFI/clkPeriod/512).

                                                                 A value of 0 in bits <24:18> will effectively turn off refresh.

                                                                 Program <36:25> to (RND-DN(ZQCS_Period / Refresh_Period) - 1), where Refresh_Period is the
                                                                 effective period programmed in bits <24:18>. Note that this value should always be greater
                                                                 than 32, to account for resistor calibration delays.

                                                                 000_00000000_0000000: Reserved

                                                                 Max refresh interval = 127 * 512= 65024 CK cycles.

                                                                 Max ZQCS interval = 32768 * 127 * 512 = 2130706432 CK cycles.

                                                                 If refresh interval is programmed to ~8us, max ZQCS interval is ~262ms, or ~4 ZQCS
                                                                 operations per second.
                                                                 LMC()_CONFIG[INIT_STATUS] determines which ranks receive the REF / ZQCS. LMC does not
                                                                 send any refreshes / ZQCS's when LMC()_CONFIG[INIT_STATUS]=0. */
        uint64_t reset                 : 1;  /**< [ 17: 17](R/W) Reset one-shot pulse for LMC()_OPS_CNT, LMC()_IFB_CNT, and LMC()_DCLK_CNT.
                                                                 To cause the reset, software writes this to a 1, then rewrites it to a 0. */
        uint64_t ecc_adr               : 1;  /**< [ 16: 16](R/W) Include memory reference address in the ECC calculation.
                                                                 0 = disabled, 1 = enabled. */
        uint64_t forcewrite            : 4;  /**< [ 15: 12](R/W) Force the oldest outstanding write to complete after having waited for 2^[FORCEWRITE] CK
                                                                 cycles. 0 = disabled. */
        uint64_t idlepower             : 3;  /**< [ 11:  9](R/W) Enter precharge power-down mode after the memory controller has been idle for
                                                                 2^(2+[IDLEPOWER]) CK cycles. 0 = disabled.

                                                                 This field should only be programmed after initialization.
                                                                 LMC()_MODEREG_PARAMS0[PPD] determines whether the DRAM DLL is disabled during the
                                                                 precharge power-down. */
        uint64_t pbank_lsb             : 4;  /**< [  8:  5](R/W) DIMM address bit select. Reverting to the explanation for [ROW_LSB], [PBANK_LSB] would be:
                                                                 [ROW_LSB] bit + num_rowbits + num_rankbits

                                                                 Decoding for PBANK_LSB:

                                                                 0x0: DIMM = mem_adr<28>; if [RANK_ENA]=1, rank = mem_adr<27>.
                                                                 0x1: DIMM = mem_adr<29>; if [RANK_ENA]=1, rank = mem_adr<28>.
                                                                 0x2: DIMM = mem_adr<30>; if [RANK_ENA]=1, rank = mem_adr<29>.
                                                                 0x3: DIMM = mem_adr<31>; if [RANK_ENA]=1, rank = mem_adr<30>.
                                                                 0x4: DIMM = mem_adr<32>; if [RANK_ENA]=1, rank = mem_adr<31>.
                                                                 0x5: DIMM = mem_adr<33>; if [RANK_ENA]=1, rank = mem_adr<32>.
                                                                 0x6: DIMM = mem_adr<34>; if [RANK_ENA]=1, rank = mem_adr<33>.
                                                                 0x7: DIMM = mem_adr<35>; if [RANK_ENA]=1, rank = mem_adr<34>.
                                                                 0x8: DIMM = mem_adr<36>; if [RANK_ENA]=1, rank = mem_adr<35>.
                                                                 0x9: DIMM = mem_adr<37>; if [RANK_ENA]=1, rank = mem_adr<36>.
                                                                 0xA: DIMM = 0;           if [RANK_ENA]=1, rank = mem_adr<37>.
                                                                 0xB-0xF: Reserved.

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1Gb (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 With [RANK_ENA] = 0, [PBANK_LSB] = 2.
                                                                 With [RANK_ENA] = 1, [PBANK_LSB] = 3.

                                                                 Internal:
                                                                 When interfacing with 8H 3DS, set this 0xA regardless of [RANK_ENA] value. */
        uint64_t row_lsb               : 3;  /**< [  4:  2](R/W) Row address bit select.
                                                                 0x0 = Address bit 14 is LSB.
                                                                 0x1 = Address bit 15 is LSB.
                                                                 0x2 = Address bit 16 is LSB.
                                                                 0x3 = Address bit 17 is LSB.
                                                                 0x4 = Address bit 18 is LSB.
                                                                 0x5 = Address bit 19 is LSB.
                                                                 0x6 = Address bit 20 is LSB.
                                                                 0x6 = Reserved.

                                                                 Encoding used to determine which memory address bit position represents the low order DDR
                                                                 ROW address. The processor's memory address<34:7> needs to be translated to DRAM addresses
                                                                 (bnk,row,col,rank and DIMM) and that is a function of the following:
                                                                 * Datapath width (64).
                                                                 * Number of banks (8).
                                                                 * Number of column bits of the memory part--specified indirectly by this register.
                                                                 * Number of row bits of the memory part--specified indirectly by [PBANK_LSB].
                                                                 * Number of ranks in a DIMM--specified by [RANK_ENA].
                                                                 * Number of DIMMs in the system by the register below ([PBANK_LSB]).

                                                                 Column address starts from mem_addr[3] for 64b (8Bytes) DQ width. [ROW_LSB] is
                                                                 mem_adr[15] for 64b mode. Therefore, the [ROW_LSB] parameter should be set to
                                                                 0x1 (64b).

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1GB (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 Refer to Cache-block Read Transaction Example, Cache-block Read Transaction Example. */
        uint64_t ecc_ena               : 1;  /**< [  1:  1](R/W) ECC enable. When set, enables the 8b ECC check/correct logic. Should be 1 when used with
                                                                 DIMMs with ECC; 0, otherwise.

                                                                 * When this mode is turned on, DQ<71:64> on write operations contains the ECC code
                                                                 generated for the 64 bits of data which will be written in the memory. Later on read
                                                                 operations, will be used to check for single-bit error (which will be auto-corrected) and
                                                                 double-bit error (which will be reported).

                                                                 * When not turned on, DQ<71:64> are driven to 0. Please refer to SEC_ERR, DED_ERR,
                                                                 LMC()_FADR, and LMC()_ECC_SYND registers for diagnostics information when there is
                                                                 an error. */
        uint64_t reserved_0            : 1;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0            : 1;
        uint64_t ecc_ena               : 1;  /**< [  1:  1](R/W) ECC enable. When set, enables the 8b ECC check/correct logic. Should be 1 when used with
                                                                 DIMMs with ECC; 0, otherwise.

                                                                 * When this mode is turned on, DQ<71:64> on write operations contains the ECC code
                                                                 generated for the 64 bits of data which will be written in the memory. Later on read
                                                                 operations, will be used to check for single-bit error (which will be auto-corrected) and
                                                                 double-bit error (which will be reported).

                                                                 * When not turned on, DQ<71:64> are driven to 0. Please refer to SEC_ERR, DED_ERR,
                                                                 LMC()_FADR, and LMC()_ECC_SYND registers for diagnostics information when there is
                                                                 an error. */
        uint64_t row_lsb               : 3;  /**< [  4:  2](R/W) Row address bit select.
                                                                 0x0 = Address bit 14 is LSB.
                                                                 0x1 = Address bit 15 is LSB.
                                                                 0x2 = Address bit 16 is LSB.
                                                                 0x3 = Address bit 17 is LSB.
                                                                 0x4 = Address bit 18 is LSB.
                                                                 0x5 = Address bit 19 is LSB.
                                                                 0x6 = Address bit 20 is LSB.
                                                                 0x6 = Reserved.

                                                                 Encoding used to determine which memory address bit position represents the low order DDR
                                                                 ROW address. The processor's memory address<34:7> needs to be translated to DRAM addresses
                                                                 (bnk,row,col,rank and DIMM) and that is a function of the following:
                                                                 * Datapath width (64).
                                                                 * Number of banks (8).
                                                                 * Number of column bits of the memory part--specified indirectly by this register.
                                                                 * Number of row bits of the memory part--specified indirectly by [PBANK_LSB].
                                                                 * Number of ranks in a DIMM--specified by [RANK_ENA].
                                                                 * Number of DIMMs in the system by the register below ([PBANK_LSB]).

                                                                 Column address starts from mem_addr[3] for 64b (8Bytes) DQ width. [ROW_LSB] is
                                                                 mem_adr[15] for 64b mode. Therefore, the [ROW_LSB] parameter should be set to
                                                                 0x1 (64b).

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1GB (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 Refer to Cache-block Read Transaction Example, Cache-block Read Transaction Example. */
        uint64_t pbank_lsb             : 4;  /**< [  8:  5](R/W) DIMM address bit select. Reverting to the explanation for [ROW_LSB], [PBANK_LSB] would be:
                                                                 [ROW_LSB] bit + num_rowbits + num_rankbits

                                                                 Decoding for PBANK_LSB:

                                                                 0x0: DIMM = mem_adr<28>; if [RANK_ENA]=1, rank = mem_adr<27>.
                                                                 0x1: DIMM = mem_adr<29>; if [RANK_ENA]=1, rank = mem_adr<28>.
                                                                 0x2: DIMM = mem_adr<30>; if [RANK_ENA]=1, rank = mem_adr<29>.
                                                                 0x3: DIMM = mem_adr<31>; if [RANK_ENA]=1, rank = mem_adr<30>.
                                                                 0x4: DIMM = mem_adr<32>; if [RANK_ENA]=1, rank = mem_adr<31>.
                                                                 0x5: DIMM = mem_adr<33>; if [RANK_ENA]=1, rank = mem_adr<32>.
                                                                 0x6: DIMM = mem_adr<34>; if [RANK_ENA]=1, rank = mem_adr<33>.
                                                                 0x7: DIMM = mem_adr<35>; if [RANK_ENA]=1, rank = mem_adr<34>.
                                                                 0x8: DIMM = mem_adr<36>; if [RANK_ENA]=1, rank = mem_adr<35>.
                                                                 0x9: DIMM = mem_adr<37>; if [RANK_ENA]=1, rank = mem_adr<36>.
                                                                 0xA: DIMM = 0;           if [RANK_ENA]=1, rank = mem_adr<37>.
                                                                 0xB-0xF: Reserved.

                                                                 For example, for a DIMM made of Samsung's K4B1G0846C-F7 1Gb (16M * 8 bit * 8 bank)
                                                                 parts, the column address width = 10, so with 10b of col, 3b of bus, 3b of bank, ROW_LSB =
                                                                 16. So, row = mem_adr<29:16>.

                                                                 With [RANK_ENA] = 0, [PBANK_LSB] = 2.
                                                                 With [RANK_ENA] = 1, [PBANK_LSB] = 3.

                                                                 Internal:
                                                                 When interfacing with 8H 3DS, set this 0xA regardless of [RANK_ENA] value. */
        uint64_t idlepower             : 3;  /**< [ 11:  9](R/W) Enter precharge power-down mode after the memory controller has been idle for
                                                                 2^(2+[IDLEPOWER]) CK cycles. 0 = disabled.

                                                                 This field should only be programmed after initialization.
                                                                 LMC()_MODEREG_PARAMS0[PPD] determines whether the DRAM DLL is disabled during the
                                                                 precharge power-down. */
        uint64_t forcewrite            : 4;  /**< [ 15: 12](R/W) Force the oldest outstanding write to complete after having waited for 2^[FORCEWRITE] CK
                                                                 cycles. 0 = disabled. */
        uint64_t ecc_adr               : 1;  /**< [ 16: 16](R/W) Include memory reference address in the ECC calculation.
                                                                 0 = disabled, 1 = enabled. */
        uint64_t reset                 : 1;  /**< [ 17: 17](R/W) Reset one-shot pulse for LMC()_OPS_CNT, LMC()_IFB_CNT, and LMC()_DCLK_CNT.
                                                                 To cause the reset, software writes this to a 1, then rewrites it to a 0. */
        uint64_t ref_zqcs_int          : 22; /**< [ 39: 18](R/W) Refresh interval is represented in number of 512 CK cycle increments. To get more precise
                                                                 control of the refresh interval, LMC()_EXT_CONFIG[REF_INT_LSBS] can be set to a
                                                                 nonzero value.
                                                                 ZQCS interval is represented in a number of refresh intervals. A refresh sequence is
                                                                 triggered when bits <24:18> are equal to 0x0, and a ZQCS sequence is triggered when
                                                                 <39:18>
                                                                 are equal to 0x0.

                                                                 The ZQCS timer only decrements when the refresh timer is 0.

                                                                 Program <24:18> to RND-DN(TREFI/clkPeriod/512).

                                                                 A value of 0 in bits <24:18> will effectively turn off refresh.

                                                                 Program <36:25> to (RND-DN(ZQCS_Period / Refresh_Period) - 1), where Refresh_Period is the
                                                                 effective period programmed in bits <24:18>. Note that this value should always be greater
                                                                 than 32, to account for resistor calibration delays.

                                                                 000_00000000_0000000: Reserved

                                                                 Max refresh interval = 127 * 512= 65024 CK cycles.

                                                                 Max ZQCS interval = 32768 * 127 * 512 = 2130706432 CK cycles.

                                                                 If refresh interval is programmed to ~8us, max ZQCS interval is ~262ms, or ~4 ZQCS
                                                                 operations per second.
                                                                 LMC()_CONFIG[INIT_STATUS] determines which ranks receive the REF / ZQCS. LMC does not
                                                                 send any refreshes / ZQCS's when LMC()_CONFIG[INIT_STATUS]=0. */
        uint64_t early_dqx             : 1;  /**< [ 40: 40](R/W) Set this bit to send DQx signals one CK cycle earlier for the case when the shortest DQx
                                                                 lines have a larger delay than the CK line. */
        uint64_t sref_with_dll         : 1;  /**< [ 41: 41](R/W) Self-refresh entry/exit write mode registers. When set, self-refresh entry sequence writes
                                                                 MR2 and MR1 (in this order, in all ranks), and self-refresh exit sequence writes MR1, MR0,
                                                                 MR2, and MR3 (in this order, for all ranks). The write operations occur before self-
                                                                 refresh entry, and after self-refresh exit. When clear, self-refresh entry and exit
                                                                 instruction sequences do not write any mode registers in the DDR3/4 parts. */
        uint64_t rank_ena              : 1;  /**< [ 42: 42](R/W) "RANK enable (for use with dual-rank DIMMs).
                                                                 * For dual-rank DIMMs, the [RANK_ENA] bit will enable the drive of the DDR#_DIMM*_CS*_L
                                                                 and
                                                                 ODT_<1:0> pins differently based on the ([PBANK_LSB] - 1) address bit.
                                                                 * Write 0 for SINGLE ranked DIMMs." */
        uint64_t rankmask              : 4;  /**< [ 46: 43](R/W) Mask to select rank to be leveled/initialized. To write-level/read-level/initialize rank
                                                                 i, set [RANKMASK]<i>:

                                                                 <pre>
                                                                               RANK_ENA = 1   RANK_ENA = 0
                                                                 RANKMASK<0> = DIMM0_CS0      DIMM0_CS0
                                                                 RANKMASK<1> = DIMM0_CS1      MBZ
                                                                 RANKMASK<2> = DIMM1_CS0      DIMM1_CS0
                                                                 RANKMASK<3> = DIMM1_CS1      MBZ
                                                                 </pre>

                                                                 For read/write leveling, each rank has to be leveled separately, so [RANKMASK] should only
                                                                 have one bit set. [RANKMASK] is not used during self-refresh entry/exit and precharge
                                                                 power-
                                                                 down entry/exit instruction sequences. When [RANK_ENA] = 0, [RANKMASK]<1> and
                                                                 [RANKMASK]<3> MBZ. */
        uint64_t mirrmask              : 4;  /**< [ 50: 47](R/W) "Mask determining which ranks are address-mirrored.
                                                                 [MIRRMASK]<n> = 1 means rank n addresses are mirrored for
                                                                 0 <= n <= 3.
                                                                 In DDR3, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 When RANK_ENA = 0, MIRRMASK<1> MBZ.

                                                                 In DDR4, a mirrored read/write operation has the following differences:
                                                                 * DDR#_BG<1> is swapped with DDR#_BG<0>.
                                                                 * DDR#_BA<1> is swapped with DDR#_BA<0>.
                                                                 * DDR#_A<13> is swapped with DDR#_A<11>.
                                                                 * DDR#_A<8> is swapped with DDR#_A<7>.
                                                                 * DDR#_A<6> is swapped with DDR#_A<5>.
                                                                 * DDR#_A<4> is swapped with DDR#_A<3>.

                                                                 For CN70XX, MIRRMASK<3:2> MBZ.
                                                                 * When RANK_ENA = 0, MIRRMASK<1> MBZ." */
        uint64_t init_status           : 4;  /**< [ 54: 51](RO/H) Indicates status of initialization. [INIT_STATUS][n] = 1 implies rank n has been
                                                                 initialized.
                                                                 Software must set necessary [RANKMASK] bits before executing the initialization sequence
                                                                 using LMC()_SEQ_CTL. If the rank has been selected for init with the [RANKMASK] bits,
                                                                 the [INIT_STATUS] bits will be set after successful initialization and after self-refresh
                                                                 exit. [INIT_STATUS] determines the chip-selects that assert during refresh, ZQCS,
                                                                 precharge
                                                                 power-down entry/exit, and self-refresh entry SEQ_SELs. */
        uint64_t early_unload_d0_r0    : 1;  /**< [ 55: 55](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 0 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R0 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK0[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 0 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK0[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R0 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R0 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d0_r1    : 1;  /**< [ 56: 56](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 1 reads.
                                                                 The recommended EARLY_UNLOAD_D0_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK1[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 1 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK1[BYTEi]) across all i), then set EARLY_UNLOAD_D0_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D0_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r0    : 1;  /**< [ 57: 57](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 2 reads.
                                                                 The recommended EARLY_UNLOAD_D1_RO value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK2[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 2 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK2[BYTEi]) across all i), then set EARLY_UNLOAD_D1_RO when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_RO = (maxset<1:0>
                                                                 !=3)). */
        uint64_t early_unload_d1_r1    : 1;  /**< [ 58: 58](R/W) Reserved, MBZ.
                                                                 Internal:
                                                                 When set, unload the PHY silo one cycle early for Rank 3 reads.
                                                                 The recommended EARLY_UNLOAD_D1_R1 value can be calculated after the final
                                                                 LMC()_RLEVEL_RANK3[BYTE*] values are selected (as part of read-leveling initialization).
                                                                 Then, determine the largest read-leveling setting for rank 3 (i.e. calculate
                                                                 maxset=MAX(LMC()_RLEVEL_RANK3[BYTEi]) across all i), then set EARLY_UNLOAD_D1_R1 when the
                                                                 low two bits of this largest setting is not 3 (i.e. EARLY_UNLOAD_D1_R1 = (maxset<1:0>
                                                                 !=3)). */
        uint64_t scrz                  : 1;  /**< [ 59: 59](R/W1S/H) Hide LMC()_SCRAMBLE_CFG0 and LMC()_SCRAMBLE_CFG1 when set. */
        uint64_t mode32b               : 1;  /**< [ 60: 60](R/W) 32-bit datapath mode. When set, only 32 DQ pins are used. */
        uint64_t mode_x4dev            : 1;  /**< [ 61: 61](R/W) DDR *4 device mode. */
        uint64_t bg2_enable            : 1;  /**< [ 62: 62](R/W) BG1 enable bit. Only has an effect when LMC()_CONFIG[MODEDDR4] = 1.
                                                                 Set to 1 when using DDR4 x4 or x8 parts.
                                                                 Clear to 0 when using DDR4 x16 parts. */
        uint64_t lrdimm_ena            : 1;  /**< [ 63: 63](R/W) Reserved.
                                                                 Internal:
                                                                 Load Reduced DIMM Enable. When set allows the use of JEDEC DDR4 LRDIMMs. */
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_lmcx_config_cn88xx cn83xx; */
};
typedef union cavm_lmcx_config cavm_lmcx_config_t;

static inline uint64_t CAVM_LMCX_CONFIG(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_LMCX_CONFIG(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e088000188ll + 0x1000000ll * ((a) & 0x0);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x87e088000188ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=3))
        return 0x87e088000188ll + 0x1000000ll * ((a) & 0x3);
    __cavm_csr_fatal("LMCX_CONFIG", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_LMCX_CONFIG(a) cavm_lmcx_config_t
#define bustype_CAVM_LMCX_CONFIG(a) CSR_TYPE_RSL
#define basename_CAVM_LMCX_CONFIG(a) "LMCX_CONFIG"
#define device_bar_CAVM_LMCX_CONFIG(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_LMCX_CONFIG(a) (a)
#define arguments_CAVM_LMCX_CONFIG(a) (a),-1,-1,-1

/**
 * Register (RSL) lmc#_ddr_pll_ctl
 *
 * LMC DDR PLL Control Register
 * This register controls the DDR_CK frequency. For details, refer to CK Speed Programming. See
 * LMC Initialization Sequence for the initialization sequence.
 * DDR PLL Bringup sequence:
 *
 * 1. Write [CLKF], [DDR_PS_EN], DFM_PS_EN, DIFFAMP, CPS, CPB.
 *
 * 2. Wait 128 ref clock cycles (7680 core-clock cycles).
 *
 * 3. Write 1 to RESET_N.
 *
 * 4. Wait 1152 ref clocks (1152*16 core-clock cycles).
 *
 * 5. Write 0 to DDR_DIV_RESET and DFM_DIV_RESET.
 *
 * 6. Wait 10 ref clock cycles (160 core-clock cycles) before bringing up the DDR interface
 *
 * Internal:
 * If test mode is going to be activated:
 *
 * * In step 1, also write jtg__ddr_pll_tm_en1, jtg__ddr_pll_tm_en2,
 * jtg__ddr_pll_tm_en3, jtg__ddr_pll_tm_en4, jtg__dfa_pll_tm_en1, jtg__dfa_pll_tm_en2,
 * jtg__dfa_pll_tm_en3, jtg__dfa_pll_tm_en4, JTAG_TEST_MODE.
 *
 * * Add final step, wait an additional 8191 ref clocks (8191*16+ core-clock cycles) to
 * allow PLL clock alignment.
 */
union cavm_lmcx_ddr_pll_ctl
{
    uint64_t u;
    struct cavm_lmcx_ddr_pll_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_45_63        : 19;
        uint64_t dclk_alt_refclk_sel   : 1;  /**< [ 44: 44](R/W) Select alternate reference clock for DCLK PLL. */
        uint64_t bwadj                 : 12; /**< [ 43: 32](R/W) Bandwidth control for DCLK PLLs. */
        uint64_t dclk_invert           : 1;  /**< [ 31: 31](R/W) Invert DCLK that feeds LMC/DDR at the south side of the chip. */
        uint64_t phy_dcok              : 1;  /**< [ 30: 30](R/W) Set to power up PHY logic after setting LMC()_DDR_PLL_CTL[DDR4_MODE]. */
        uint64_t ddr4_mode             : 1;  /**< [ 29: 29](R/W) DDR4 mode select: 1 = DDR4, 0 = DDR3. */
        uint64_t pll_fbslip            : 1;  /**< [ 28: 28](RO/H) PLL FBSLIP indication. */
        uint64_t pll_lock              : 1;  /**< [ 27: 27](RO/H) PLL LOCK indication. */
        uint64_t pll_rfslip            : 1;  /**< [ 26: 26](RO/H) PLL RFSLIP indication. */
        uint64_t clkr                  : 2;  /**< [ 25: 24](R/W) PLL post-divider control. */
        uint64_t jtg_test_mode         : 1;  /**< [ 23: 23](R/W) Reserved; must be zero.
                                                                 Internal:
                                                                 JTAG test mode. Clock alignment between DCLK & REFCLK as
                                                                 well as FCLK & REFCLK can only be performed after the ddr_pll_divider_reset is deasserted.
                                                                 Software needs to wait at least 10 reference clock cycles after deasserting
                                                                 pll_divider_reset
                                                                 before asserting LMC()_DDR_PLL_CTL[JTG_TEST_MODE]. During alignment (which can take up
                                                                 to 160 microseconds) DCLK and FCLK can exhibit some high-frequency pulses. Therefore, all
                                                                 bring up activities in that clock domain need to be delayed (when the chip operates in
                                                                 jtg_test_mode) by about 160 microseconds to ensure that lock is achieved. */
        uint64_t ddr_div_reset         : 1;  /**< [ 22: 22](R/W) DDR postscalar divider reset. */
        uint64_t ddr_ps_en             : 4;  /**< [ 21: 18](R/W) DDR postscalar divide ratio. Determines the LMC CK speed.
                                                                 0x0 = divide LMC PLL by 1.
                                                                 0x1 = divide LMC PLL by 2.
                                                                 0x2 = divide LMC PLL by 3.
                                                                 0x3 = divide LMC PLL by 4.
                                                                 0x4 = divide LMC PLL by 5.
                                                                 0x5 = divide LMC PLL by 6.
                                                                 0x6 = divide LMC PLL by 7.
                                                                 0x7 = divide LMC PLL by 8.
                                                                 0x8 = divide LMC PLL by 10.
                                                                 0x9 = divide LMC PLL by 12.
                                                                 0xA = Reserved.
                                                                 0xB = Reserved.
                                                                 0xC = Reserved.
                                                                 0xD = Reserved.
                                                                 0xE = Reserved.
                                                                 0xF = Reserved.

                                                                 [DDR_PS_EN] is not used when [DDR_DIV_RESET] = 1. */
        uint64_t reserved_9_17         : 9;
        uint64_t clkf_ext              : 1;  /**< [  8:  8](R/W) A 1-bit extension to the [CLKF] register to support for DDR4-2666. */
        uint64_t reset_n               : 1;  /**< [  7:  7](R/W) PLL reset */
        uint64_t clkf                  : 7;  /**< [  6:  0](R/W) Multiply reference by [CLKF]. 31 <= [CLKF] <= 99. LMC PLL frequency = 50 * [CLKF]. min =
                                                                 1.6
                                                                 GHz, max = 5 GHz. */
#else /* Word 0 - Little Endian */
        uint64_t clkf                  : 7;  /**< [  6:  0](R/W) Multiply reference by [CLKF]. 31 <= [CLKF] <= 99. LMC PLL frequency = 50 * [CLKF]. min =
                                                                 1.6
                                                                 GHz, max = 5 GHz. */
        uint64_t reset_n               : 1;  /**< [  7:  7](R/W) PLL reset */
        uint64_t clkf_ext              : 1;  /**< [  8:  8](R/W) A 1-bit extension to the [CLKF] register to support for DDR4-2666. */
        uint64_t reserved_9_17         : 9;
        uint64_t ddr_ps_en             : 4;  /**< [ 21: 18](R/W) DDR postscalar divide ratio. Determines the LMC CK speed.
                                                                 0x0 = divide LMC PLL by 1.
                                                                 0x1 = divide LMC PLL by 2.
                                                                 0x2 = divide LMC PLL by 3.
                                                                 0x3 = divide LMC PLL by 4.
                                                                 0x4 = divide LMC PLL by 5.
                                                                 0x5 = divide LMC PLL by 6.
                                                                 0x6 = divide LMC PLL by 7.
                                                                 0x7 = divide LMC PLL by 8.
                                                                 0x8 = divide LMC PLL by 10.
                                                                 0x9 = divide LMC PLL by 12.
                                                                 0xA = Reserved.
                                                                 0xB = Reserved.
                                                                 0xC = Reserved.
                                                                 0xD = Reserved.
                                                                 0xE = Reserved.
                                                                 0xF = Reserved.

                                                                 [DDR_PS_EN] is not used when [DDR_DIV_RESET] = 1. */
        uint64_t ddr_div_reset         : 1;  /**< [ 22: 22](R/W) DDR postscalar divider reset. */
        uint64_t jtg_test_mode         : 1;  /**< [ 23: 23](R/W) Reserved; must be zero.
                                                                 Internal:
                                                                 JTAG test mode. Clock alignment between DCLK & REFCLK as
                                                                 well as FCLK & REFCLK can only be performed after the ddr_pll_divider_reset is deasserted.
                                                                 Software needs to wait at least 10 reference clock cycles after deasserting
                                                                 pll_divider_reset
                                                                 before asserting LMC()_DDR_PLL_CTL[JTG_TEST_MODE]. During alignment (which can take up
                                                                 to 160 microseconds) DCLK and FCLK can exhibit some high-frequency pulses. Therefore, all
                                                                 bring up activities in that clock domain need to be delayed (when the chip operates in
                                                                 jtg_test_mode) by about 160 microseconds to ensure that lock is achieved. */
        uint64_t clkr                  : 2;  /**< [ 25: 24](R/W) PLL post-divider control. */
        uint64_t pll_rfslip            : 1;  /**< [ 26: 26](RO/H) PLL RFSLIP indication. */
        uint64_t pll_lock              : 1;  /**< [ 27: 27](RO/H) PLL LOCK indication. */
        uint64_t pll_fbslip            : 1;  /**< [ 28: 28](RO/H) PLL FBSLIP indication. */
        uint64_t ddr4_mode             : 1;  /**< [ 29: 29](R/W) DDR4 mode select: 1 = DDR4, 0 = DDR3. */
        uint64_t phy_dcok              : 1;  /**< [ 30: 30](R/W) Set to power up PHY logic after setting LMC()_DDR_PLL_CTL[DDR4_MODE]. */
        uint64_t dclk_invert           : 1;  /**< [ 31: 31](R/W) Invert DCLK that feeds LMC/DDR at the south side of the chip. */
        uint64_t bwadj                 : 12; /**< [ 43: 32](R/W) Bandwidth control for DCLK PLLs. */
        uint64_t dclk_alt_refclk_sel   : 1;  /**< [ 44: 44](R/W) Select alternate reference clock for DCLK PLL. */
        uint64_t reserved_45_63        : 19;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_lmcx_ddr_pll_ctl_s cn81xx; */
    /* struct cavm_lmcx_ddr_pll_ctl_s cn88xx; */
    struct cavm_lmcx_ddr_pll_ctl_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_45_63        : 19;
        uint64_t dclk_alt_refclk_sel   : 1;  /**< [ 44: 44](R/W) Select alternate reference clock for DCLK PLL. */
        uint64_t bwadj                 : 12; /**< [ 43: 32](R/W) Bandwidth control for DCLK PLLs. */
        uint64_t dclk_invert           : 1;  /**< [ 31: 31](R/W) Invert dclk that feeds LMC/DDR at the south side of the chip. */
        uint64_t phy_dcok              : 1;  /**< [ 30: 30](R/W) Set to power up PHY logic after setting LMC()_DDR_PLL_CTL[DDR4_MODE]. */
        uint64_t ddr4_mode             : 1;  /**< [ 29: 29](R/W) DDR4 mode select: 1 = DDR4, 0 = DDR3. */
        uint64_t pll_fbslip            : 1;  /**< [ 28: 28](RO/H) PLL FBSLIP indication. */
        uint64_t pll_lock              : 1;  /**< [ 27: 27](RO/H) PLL LOCK indication. */
        uint64_t pll_rfslip            : 1;  /**< [ 26: 26](RO/H) PLL RFSLIP indication. */
        uint64_t clkr                  : 2;  /**< [ 25: 24](R/W) PLL post-divider control. */
        uint64_t jtg_test_mode         : 1;  /**< [ 23: 23](R/W) Reserved; must be zero.
                                                                 Internal:
                                                                 JTAG test mode. Clock alignment between DCLK & REFCLK as
                                                                 well as FCLK & REFCLK can only be performed after the ddr_pll_divider_reset is deasserted.
                                                                 Software needs to wait at least 10 reference clock cycles after deasserting
                                                                 pll_divider_reset
                                                                 before asserting LMC()_DDR_PLL_CTL[JTG_TEST_MODE]. During alignment (which can take up
                                                                 to 160 microseconds) DCLK and FCLK can exhibit some high-frequency pulses. Therefore, all
                                                                 bring up activities in that clock domain need to be delayed (when the chip operates in
                                                                 jtg_test_mode) by about 160 microseconds to ensure that lock is achieved. */
        uint64_t ddr_div_reset         : 1;  /**< [ 22: 22](R/W) DDR postscalar divider reset. */
        uint64_t ddr_ps_en             : 4;  /**< [ 21: 18](R/W) DDR postscalar divide ratio. Determines the LMC CK speed.
                                                                 0x0 = divide LMC PLL by 1.
                                                                 0x1 = divide LMC PLL by 2.
                                                                 0x2 = divide LMC PLL by 3.
                                                                 0x3 = divide LMC PLL by 4.
                                                                 0x4 = divide LMC PLL by 5.
                                                                 0x5 = divide LMC PLL by 6.
                                                                 0x6 = divide LMC PLL by 7.
                                                                 0x7 = divide LMC PLL by 8.
                                                                 0x8 = divide LMC PLL by 10.
                                                                 0x9 = divide LMC PLL by 12.
                                                                 0xA = Reserved.
                                                                 0xB = Reserved.
                                                                 0xC = Reserved.
                                                                 0xD = Reserved.
                                                                 0xE = Reserved.
                                                                 0xF = Reserved.

                                                                 [DDR_PS_EN] is not used when [DDR_DIV_RESET] = 1. */
        uint64_t reserved_9_17         : 9;
        uint64_t clkf_ext              : 1;  /**< [  8:  8](R/W) A 1-bit extension to the [CLKF] register to support for DDR4-2666. */
        uint64_t reset_n               : 1;  /**< [  7:  7](R/W) PLL reset */
        uint64_t clkf                  : 7;  /**< [  6:  0](R/W) Multiply reference by [CLKF]. 31 <= [CLKF] <= 99. LMC PLL frequency = 50 * [CLKF]. min =
                                                                 1.6
                                                                 GHz, max = 5 GHz. */
#else /* Word 0 - Little Endian */
        uint64_t clkf                  : 7;  /**< [  6:  0](R/W) Multiply reference by [CLKF]. 31 <= [CLKF] <= 99. LMC PLL frequency = 50 * [CLKF]. min =
                                                                 1.6
                                                                 GHz, max = 5 GHz. */
        uint64_t reset_n               : 1;  /**< [  7:  7](R/W) PLL reset */
        uint64_t clkf_ext              : 1;  /**< [  8:  8](R/W) A 1-bit extension to the [CLKF] register to support for DDR4-2666. */
        uint64_t reserved_9_17         : 9;
        uint64_t ddr_ps_en             : 4;  /**< [ 21: 18](R/W) DDR postscalar divide ratio. Determines the LMC CK speed.
                                                                 0x0 = divide LMC PLL by 1.
                                                                 0x1 = divide LMC PLL by 2.
                                                                 0x2 = divide LMC PLL by 3.
                                                                 0x3 = divide LMC PLL by 4.
                                                                 0x4 = divide LMC PLL by 5.
                                                                 0x5 = divide LMC PLL by 6.
                                                                 0x6 = divide LMC PLL by 7.
                                                                 0x7 = divide LMC PLL by 8.
                                                                 0x8 = divide LMC PLL by 10.
                                                                 0x9 = divide LMC PLL by 12.
                                                                 0xA = Reserved.
                                                                 0xB = Reserved.
                                                                 0xC = Reserved.
                                                                 0xD = Reserved.
                                                                 0xE = Reserved.
                                                                 0xF = Reserved.

                                                                 [DDR_PS_EN] is not used when [DDR_DIV_RESET] = 1. */
        uint64_t ddr_div_reset         : 1;  /**< [ 22: 22](R/W) DDR postscalar divider reset. */
        uint64_t jtg_test_mode         : 1;  /**< [ 23: 23](R/W) Reserved; must be zero.
                                                                 Internal:
                                                                 JTAG test mode. Clock alignment between DCLK & REFCLK as
                                                                 well as FCLK & REFCLK can only be performed after the ddr_pll_divider_reset is deasserted.
                                                                 Software needs to wait at least 10 reference clock cycles after deasserting
                                                                 pll_divider_reset
                                                                 before asserting LMC()_DDR_PLL_CTL[JTG_TEST_MODE]. During alignment (which can take up
                                                                 to 160 microseconds) DCLK and FCLK can exhibit some high-frequency pulses. Therefore, all
                                                                 bring up activities in that clock domain need to be delayed (when the chip operates in
                                                                 jtg_test_mode) by about 160 microseconds to ensure that lock is achieved. */
        uint64_t clkr                  : 2;  /**< [ 25: 24](R/W) PLL post-divider control. */
        uint64_t pll_rfslip            : 1;  /**< [ 26: 26](RO/H) PLL RFSLIP indication. */
        uint64_t pll_lock              : 1;  /**< [ 27: 27](RO/H) PLL LOCK indication. */
        uint64_t pll_fbslip            : 1;  /**< [ 28: 28](RO/H) PLL FBSLIP indication. */
        uint64_t ddr4_mode             : 1;  /**< [ 29: 29](R/W) DDR4 mode select: 1 = DDR4, 0 = DDR3. */
        uint64_t phy_dcok              : 1;  /**< [ 30: 30](R/W) Set to power up PHY logic after setting LMC()_DDR_PLL_CTL[DDR4_MODE]. */
        uint64_t dclk_invert           : 1;  /**< [ 31: 31](R/W) Invert dclk that feeds LMC/DDR at the south side of the chip. */
        uint64_t bwadj                 : 12; /**< [ 43: 32](R/W) Bandwidth control for DCLK PLLs. */
        uint64_t dclk_alt_refclk_sel   : 1;  /**< [ 44: 44](R/W) Select alternate reference clock for DCLK PLL. */
        uint64_t reserved_45_63        : 19;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_lmcx_ddr_pll_ctl cavm_lmcx_ddr_pll_ctl_t;

static inline uint64_t CAVM_LMCX_DDR_PLL_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_LMCX_DDR_PLL_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x87e088000258ll + 0x1000000ll * ((a) & 0x0);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x87e088000258ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=3))
        return 0x87e088000258ll + 0x1000000ll * ((a) & 0x3);
    __cavm_csr_fatal("LMCX_DDR_PLL_CTL", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_LMCX_DDR_PLL_CTL(a) cavm_lmcx_ddr_pll_ctl_t
#define bustype_CAVM_LMCX_DDR_PLL_CTL(a) CSR_TYPE_RSL
#define basename_CAVM_LMCX_DDR_PLL_CTL(a) "LMCX_DDR_PLL_CTL"
#define device_bar_CAVM_LMCX_DDR_PLL_CTL(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_LMCX_DDR_PLL_CTL(a) (a)
#define arguments_CAVM_LMCX_DDR_PLL_CTL(a) (a),-1,-1,-1

#endif /* __CAVM_CSRS_LMC_H__ */

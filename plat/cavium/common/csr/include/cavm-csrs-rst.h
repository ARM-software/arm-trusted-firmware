#ifndef __CAVM_CSRS_RST_H__
#define __CAVM_CSRS_RST_H__
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
 * Cavium RST.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration rst_bar_e
 *
 * RST Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_RST_BAR_E_RST_PF_BAR0 (0x87e006000000ll)
#define CAVM_RST_BAR_E_RST_PF_BAR0_SIZE 0x800000ull
#define CAVM_RST_BAR_E_RST_PF_BAR4 (0x87e006f00000ll)
#define CAVM_RST_BAR_E_RST_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration rst_boot_fail_e
 *
 * RST Boot Failure Code Enumeration
 * Enumerates the reasons for boot failure, returned to post-boot code
 * in argument register 0 and blinked on GPIO<11>.
 */
#define CAVM_RST_BOOT_FAIL_E_AUTH (6)
#define CAVM_RST_BOOT_FAIL_E_DEVICE (3)
#define CAVM_RST_BOOT_FAIL_E_GOOD (0)
#define CAVM_RST_BOOT_FAIL_E_MAGIC (4)
#define CAVM_RST_BOOT_FAIL_E_MCORE (5)
#define CAVM_RST_BOOT_FAIL_E_METH (2)

/**
 * Enumeration rst_boot_method_e
 *
 * RST Boot-strap Method Enumeration
 * Enumerates GPIO_STRAP<3:0>, which determines the method used to boot the cores.
 */
#define CAVM_RST_BOOT_METHOD_E_CCPI0 (9)
#define CAVM_RST_BOOT_METHOD_E_CCPI1 (0xa)
#define CAVM_RST_BOOT_METHOD_E_CCPI2 (0xb)
#define CAVM_RST_BOOT_METHOD_E_EMMC_LS (3)
#define CAVM_RST_BOOT_METHOD_E_EMMC_SS (2)
#define CAVM_RST_BOOT_METHOD_E_PCIE0 (0xc)
#define CAVM_RST_BOOT_METHOD_E_PCIE2 (0xd)
#define CAVM_RST_BOOT_METHOD_E_REMOTE (8)
#define CAVM_RST_BOOT_METHOD_E_SPI24 (5)
#define CAVM_RST_BOOT_METHOD_E_SPI32 (6)

/**
 * Register (RSL) rst_boot
 *
 * RST Boot Register
 */
union cavm_rst_boot
{
    uint64_t u;
    struct cavm_rst_boot_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) Reserved. */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t reserved_47_54        : 8;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t lboot_pf_flr          : 4;  /**< [ 29: 26](R/W1C/H) Last boot cause was caused by a PF Function Level Reset event.
                                                                 <29> = Warm reset due to PF FLR on PEM3.
                                                                 <28> = Warm reset due to PF FLR on PEM2.
                                                                 <27> = Warm reset due to PF FLR on PEM1.
                                                                 <26> = Warm reset due to PF FLR on PEM0. */
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Last boot cause mask for PEM5 and PEM4; resets only with PLL_DC_OK.
                                                                 <23> = Warm reset due to Cntl5 link-down or hot-reset.
                                                                 <22> = Warm reset due to Cntl4 link-down or hot-reset.
                                                                 <21> = Cntl5 reset due to PERST5_L pin.
                                                                 <20> = Cntl4 reset due to PERST4_L pin.
                                                                 <19> = Warm reset due to PERST5_L pin.
                                                                 <18> = Warm reset due to PERST4_L pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
#else /* Word 0 - Little Endian */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Last boot cause mask for PEM5 and PEM4; resets only with PLL_DC_OK.
                                                                 <23> = Warm reset due to Cntl5 link-down or hot-reset.
                                                                 <22> = Warm reset due to Cntl4 link-down or hot-reset.
                                                                 <21> = Cntl5 reset due to PERST5_L pin.
                                                                 <20> = Cntl4 reset due to PERST4_L pin.
                                                                 <19> = Warm reset due to PERST5_L pin.
                                                                 <18> = Warm reset due to PERST4_L pin. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t lboot_pf_flr          : 4;  /**< [ 29: 26](R/W1C/H) Last boot cause was caused by a PF Function Level Reset event.
                                                                 <29> = Warm reset due to PF FLR on PEM3.
                                                                 <28> = Warm reset due to PF FLR on PEM2.
                                                                 <27> = Warm reset due to PF FLR on PEM1.
                                                                 <26> = Warm reset due to PF FLR on PEM0. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_47_54        : 8;
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) Reserved. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
#endif /* Word 0 - End */
    } s;
    struct cavm_rst_boot_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) Reserved. */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t reserved_47_54        : 8;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t reserved_26_29        : 4;
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Reserved. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Reserved
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Reserved
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Reserved
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
#else /* Word 0 - Little Endian */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Reserved
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Reserved
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Reserved
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Reserved. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t reserved_26_29        : 4;
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_47_54        : 8;
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) Reserved. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_rst_boot_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) VRM error. VRM did not complete operations within 5.25ms of PLL_DC_OK being
                                                                 asserted. PLLs were released automatically. */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t reserved_47_54        : 8;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t reserved_26_29        : 4;
        uint64_t reserved_24_25        : 2;
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Last boot cause mask for PEM5 and PEM4; resets only with PLL_DC_OK.
                                                                 <23> = Warm reset due to Cntl5 link-down or hot-reset.
                                                                 <22> = Warm reset due to Cntl4 link-down or hot-reset.
                                                                 <21> = Cntl5 reset due to PERST5_L pin.
                                                                 <20> = Cntl4 reset due to PERST4_L pin.
                                                                 <19> = Warm reset due to PERST5_L pin.
                                                                 <18> = Warm reset due to PERST4_L pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
#else /* Word 0 - Little Endian */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Last boot cause mask for PEM5 and PEM4; resets only with PLL_DC_OK.
                                                                 <23> = Warm reset due to Cntl5 link-down or hot-reset.
                                                                 <22> = Warm reset due to Cntl4 link-down or hot-reset.
                                                                 <21> = Cntl5 reset due to PERST5_L pin.
                                                                 <20> = Cntl4 reset due to PERST4_L pin.
                                                                 <19> = Warm reset due to PERST5_L pin.
                                                                 <18> = Warm reset due to PERST4_L pin. */
        uint64_t reserved_24_25        : 2;
        uint64_t reserved_26_29        : 4;
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_47_54        : 8;
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) VRM error. VRM did not complete operations within 5.25ms of PLL_DC_OK being
                                                                 asserted. PLLs were released automatically. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_rst_boot_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) VRM error. VRM did not complete operations within 5.25ms of PLL_DC_OK being
                                                                 asserted. PLLs were released automatically. */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t reserved_47_54        : 8;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t lboot_pf_flr          : 4;  /**< [ 29: 26](R/W1C/H) Last boot cause was caused by a PF Function Level Reset event.
                                                                 <29> = Warm reset due to PF FLR on PEM3.
                                                                 <28> = Warm reset due to PF FLR on PEM2.
                                                                 <27> = Warm reset due to PF FLR on PEM1.
                                                                 <26> = Warm reset due to PF FLR on PEM0. */
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Reserved. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
#else /* Word 0 - Little Endian */
        uint64_t rboot_pin             : 1;  /**< [  0:  0](RO/H) Remote Boot strap. Indicates the state of remote boot as initially determined by
                                                                 GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE. If set core 0 will remain in reset
                                                                 for the cold reset. */
        uint64_t rboot                 : 1;  /**< [  1:  1](R/W) Remote boot. If set, indicates that core 0 will remain in reset after a
                                                                 chip warm/soft reset. The initial value mimics the setting of the [RBOOT_PIN]. */
        uint64_t lboot                 : 10; /**< [ 11:  2](R/W1C/H) Last boot cause mask for PEM1 and PEM0; resets only with PLL_DC_OK.
                                                                 <11> = Soft reset due to watchdog.
                                                                 <10> = Soft reset due to RST_SOFT_RST write.
                                                                 <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                                 <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                                 <7> = Cntl1 reset due to PERST1_L pin.
                                                                 <6> = Cntl0 reset due to PERST0_L pin.
                                                                 <5> = Warm reset due to PERST1_L pin.
                                                                 <4> = Warm reset due to PERST0_L pin.
                                                                 <3> = Warm reset due to CHIP_RESET_L pin.
                                                                 <2> = Cold reset due to PLL_DC_OK pin. */
        uint64_t lboot_ext23           : 6;  /**< [ 17: 12](R/W1C/H) Last boot cause mask for PEM3 and PEM2; resets only with PLL_DC_OK.
                                                                 <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                                 <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                                 <15> = Cntl3 reset due to PERST3_L pin.
                                                                 <14> = Cntl2 reset due to PERST2_L pin.
                                                                 <13> = Warm reset due to PERST3_L pin.
                                                                 <12> = Warm reset due to PERST2_L pin. */
        uint64_t lboot_ext45           : 6;  /**< [ 23: 18](R/W1C/H) Reserved. */
        uint64_t lboot_jtg             : 1;  /**< [ 24: 24](R/W1C/H) Last boot cause was write to JTG reset. */
        uint64_t lboot_ckill           : 1;  /**< [ 25: 25](R/W1C/H) Last boot cause was chip kill timer expiring.  See RST_BOOT[CHIPKILL]. */
        uint64_t lboot_pf_flr          : 4;  /**< [ 29: 26](R/W1C/H) Last boot cause was caused by a PF Function Level Reset event.
                                                                 <29> = Warm reset due to PF FLR on PEM3.
                                                                 <28> = Warm reset due to PF FLR on PEM2.
                                                                 <27> = Warm reset due to PF FLR on PEM1.
                                                                 <26> = Warm reset due to PF FLR on PEM0. */
        uint64_t lboot_oci             : 3;  /**< [ 32: 30](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 Last boot cause mask for CCPI; resets only with PLL_DC_OK.
                                                                 <32> = Warm reset due to CCPI link 2 going down.
                                                                 <31> = Warm reset due to CCPI link 1 going down.
                                                                 <30> = Warm reset due to CCPI link 0 going down. */
        uint64_t pnr_mul               : 6;  /**< [ 38: 33](RO/H) Coprocessor-clock multiplier. [PNR_MUL] = (coprocessor-clock speed) /(ref-clock speed).
                                                                 The value ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [PNR_MUL] is set from the pi_pnr_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[122:119]. If the fuse value is > 0, it is compared with the pi_pnr_pll_mul[4:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_39           : 1;
        uint64_t c_mul                 : 7;  /**< [ 46: 40](RO/H) Core-clock multiplier. [C_MUL] = (core-clock speed) / (ref-clock speed). The value
                                                                 ref-clock speed should always be 50 MHz.

                                                                 Internal:
                                                                 [C_MUL] is set from the pi_pll_mul pins plus 6 and is limited by a set of
                                                                 fuses[127:123]. If the fuse value is > 0, it is compared with the pi_pll_mul[5:1]
                                                                 pins and the smaller value is used. */
        uint64_t reserved_47_54        : 8;
        uint64_t dis_scan              : 1;  /**< [ 55: 55](R/W1S) Disable scan. When written to 1, and FUSF_CTL[ROT_LCK] = 1, reads as 1 and scan is not
                                                                 allowed in the part.
                                                                 This state persists across soft and warm resets.

                                                                 Internal:
                                                                 This state will persist across a simulation */
        uint64_t dis_huk               : 1;  /**< [ 56: 56](R/W1S) Disable HUK. Secure only and W1S set-only. When set FUSF_SSK(),
                                                                 FUSF_HUK(), FUSF_EK(), and FUSF_SW() cannot be read.
                                                                 Resets to (!trusted_mode && FUSF_CTL[FJ_DIS_HUK]).

                                                                 Software must write a one to this bit when the chain of trust is broken. */
        uint64_t vrm_err               : 1;  /**< [ 57: 57](RO) VRM error. VRM did not complete operations within 5.25ms of PLL_DC_OK being
                                                                 asserted. PLLs were released automatically. */
        uint64_t jt_tstmode            : 1;  /**< [ 58: 58](RO) JTAG test mode. */
        uint64_t ckill_ppdis           : 1;  /**< [ 59: 59](R/W) Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                                 be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                                 RST_BOOT[CHIPKILL]=1. */
        uint64_t trusted_mode          : 1;  /**< [ 60: 60](RO) When set, chip is operating as a trusted device. This bit is asserted when
                                                                 either MIO_FUS_DAT2[TRUSTZONE_EN], FUSF_CTL[TZ_FORCE2], or the trusted-mode
                                                                 strap GPIO_STRAP<10> are set. */
        uint64_t ejtagdis              : 1;  /**< [ 61: 61](R/W) Reserved. */
        uint64_t jtcsrdis              : 1;  /**< [ 62: 62](R/W) JTAG CSR disable. When set to 1, during the next warm or soft reset the JTAG TAP
                                                                 controller will be disabled, i.e. DAP_IMP_DAR will be 0. This field resets to 1
                                                                 in trusted-mode, else 0. */
        uint64_t chipkill              : 1;  /**< [ 63: 63](R/W1S) A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                                 expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                                 a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                                 RST_CKILL[TIMER]. This feature is effectively a delayed warm reset. */
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_rst_boot cavm_rst_boot_t;

#define CAVM_RST_BOOT CAVM_RST_BOOT_FUNC()
static inline uint64_t CAVM_RST_BOOT_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_BOOT_FUNC(void)
{
    return 0x87e006001600ll;
}

#define typedef_CAVM_RST_BOOT cavm_rst_boot_t
#define bustype_CAVM_RST_BOOT CSR_TYPE_RSL
#define basename_CAVM_RST_BOOT "RST_BOOT"
#define device_bar_CAVM_RST_BOOT 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_BOOT 0
#define arguments_CAVM_RST_BOOT -1,-1,-1,-1

/**
 * Register (RSL) rst_cfg
 *
 * RST Configuration Register
 */
union cavm_rst_cfg
{
    uint64_t u;
    struct cavm_rst_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t bist_delay            : 58; /**< [ 63:  6](RO/H) Reserved. */
        uint64_t reserved_3_5          : 3;
        uint64_t cntl_clr_bist         : 1;  /**< [  2:  2](R/W) Perform clear BIST during control-only reset, instead of a full BIST. A warm/soft reset
                                                                 does not change this field. */
        uint64_t warm_clr_bist         : 1;  /**< [  1:  1](R/W) Perform clear BIST during warm reset, instead of a full BIST. A warm/soft reset does not
                                                                 change this field. Note that a cold reset always performs a full BIST. */
        uint64_t soft_clr_bist         : 1;  /**< [  0:  0](R/W) Perform clear BIST during soft reset, instead of a full BIST. A warm/soft reset does not
                                                                 change this field. Note that a cold reset always performs a full BIST. */
#else /* Word 0 - Little Endian */
        uint64_t soft_clr_bist         : 1;  /**< [  0:  0](R/W) Perform clear BIST during soft reset, instead of a full BIST. A warm/soft reset does not
                                                                 change this field. Note that a cold reset always performs a full BIST. */
        uint64_t warm_clr_bist         : 1;  /**< [  1:  1](R/W) Perform clear BIST during warm reset, instead of a full BIST. A warm/soft reset does not
                                                                 change this field. Note that a cold reset always performs a full BIST. */
        uint64_t cntl_clr_bist         : 1;  /**< [  2:  2](R/W) Perform clear BIST during control-only reset, instead of a full BIST. A warm/soft reset
                                                                 does not change this field. */
        uint64_t reserved_3_5          : 3;
        uint64_t bist_delay            : 58; /**< [ 63:  6](RO/H) Reserved. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_rst_cfg_s cn; */
};
typedef union cavm_rst_cfg cavm_rst_cfg_t;

#define CAVM_RST_CFG CAVM_RST_CFG_FUNC()
static inline uint64_t CAVM_RST_CFG_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_CFG_FUNC(void)
{
    return 0x87e006001610ll;
}

#define typedef_CAVM_RST_CFG cavm_rst_cfg_t
#define bustype_CAVM_RST_CFG CSR_TYPE_RSL
#define basename_CAVM_RST_CFG "RST_CFG"
#define device_bar_CAVM_RST_CFG 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_CFG 0
#define arguments_CAVM_RST_CFG -1,-1,-1,-1

/**
 * Register (RSL) rst_ocx
 *
 * RST OCX Register
 */
union cavm_rst_ocx
{
    uint64_t u;
    struct cavm_rst_ocx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_3_63         : 61;
        uint64_t rst_link              : 3;  /**< [  2:  0](R/W) Controls whether corresponding OCX link going down causes a chip reset. A warm/soft reset
                                                                 does not change this field. On cold reset, this field is initialized to 0. See
                                                                 OCX_COM_LINK()_CTL for a description of what events can contribute to the link_down
                                                                 condition. */
#else /* Word 0 - Little Endian */
        uint64_t rst_link              : 3;  /**< [  2:  0](R/W) Controls whether corresponding OCX link going down causes a chip reset. A warm/soft reset
                                                                 does not change this field. On cold reset, this field is initialized to 0. See
                                                                 OCX_COM_LINK()_CTL for a description of what events can contribute to the link_down
                                                                 condition. */
        uint64_t reserved_3_63         : 61;
#endif /* Word 0 - End */
    } s;
    struct cavm_rst_ocx_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_3_63         : 61;
        uint64_t rst_link              : 3;  /**< [  2:  0](R/W) Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t rst_link              : 3;  /**< [  2:  0](R/W) Reserved. */
        uint64_t reserved_3_63         : 61;
#endif /* Word 0 - End */
    } cn81xx;
    /* struct cavm_rst_ocx_s cn88xx; */
    /* struct cavm_rst_ocx_cn81xx cn83xx; */
};
typedef union cavm_rst_ocx cavm_rst_ocx_t;

#define CAVM_RST_OCX CAVM_RST_OCX_FUNC()
static inline uint64_t CAVM_RST_OCX_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_OCX_FUNC(void)
{
    return 0x87e006001618ll;
}

#define typedef_CAVM_RST_OCX cavm_rst_ocx_t
#define bustype_CAVM_RST_OCX CSR_TYPE_RSL
#define basename_CAVM_RST_OCX "RST_OCX"
#define device_bar_CAVM_RST_OCX 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_OCX 0
#define arguments_CAVM_RST_OCX -1,-1,-1,-1

/**
 * Register (RSL) rst_pp_pending
 *
 * RST Cores Reset Pending Register
 * This register contains the reset status for each core.
 */
union cavm_rst_pp_pending
{
    uint64_t u;
    struct cavm_rst_pp_pending_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t pend                  : 48; /**< [ 47:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
#else /* Word 0 - Little Endian */
        uint64_t pend                  : 48; /**< [ 47:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    struct cavm_rst_pp_pending_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t pend                  : 4;  /**< [  3:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
#else /* Word 0 - Little Endian */
        uint64_t pend                  : 4;  /**< [  3:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } cn81xx;
    /* struct cavm_rst_pp_pending_s cn88xx; */
    struct cavm_rst_pp_pending_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_24_63        : 40;
        uint64_t pend                  : 24; /**< [ 23:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
#else /* Word 0 - Little Endian */
        uint64_t pend                  : 24; /**< [ 23:  0](RO/H) Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                                 occurs immediately but if RST_PP_POWER[GATE] = 1 and the core is released from
                                                                 reset a delay of 64K core-clock cycles between each core reset applies to satisfy power
                                                                 management.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_24_63        : 40;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_rst_pp_pending cavm_rst_pp_pending_t;

#define CAVM_RST_PP_PENDING CAVM_RST_PP_PENDING_FUNC()
static inline uint64_t CAVM_RST_PP_PENDING_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_PP_PENDING_FUNC(void)
{
    return 0x87e006001748ll;
}

#define typedef_CAVM_RST_PP_PENDING cavm_rst_pp_pending_t
#define bustype_CAVM_RST_PP_PENDING CSR_TYPE_RSL
#define basename_CAVM_RST_PP_PENDING "RST_PP_PENDING"
#define device_bar_CAVM_RST_PP_PENDING 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_PP_PENDING 0
#define arguments_CAVM_RST_PP_PENDING -1,-1,-1,-1

/**
 * Register (RSL) rst_pp_reset
 *
 * RST Core Reset Register
 * This register contains the reset control for each core: 1 = hold core in reset, 0 = release
 * core
 * from reset. It resets to all 1s when GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE
 * or all 1s excluding bit 0 otherwise.
 * Write operations to this register should occur only if RST_PP_PENDING is cleared.
 */
union cavm_rst_pp_reset
{
    uint64_t u;
    struct cavm_rst_pp_reset_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t rst                   : 47; /**< [ 47:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
#else /* Word 0 - Little Endian */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
        uint64_t rst                   : 47; /**< [ 47:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    struct cavm_rst_pp_reset_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t rst                   : 3;  /**< [  3:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
#else /* Word 0 - Little Endian */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
        uint64_t rst                   : 3;  /**< [  3:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } cn81xx;
    /* struct cavm_rst_pp_reset_s cn88xx; */
    struct cavm_rst_pp_reset_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_24_63        : 40;
        uint64_t rst                   : 23; /**< [ 23:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
#else /* Word 0 - Little Endian */
        uint64_t rst0                  : 1;  /**< [  0:  0](R/W/H) Core reset for core 0, depends on if GPIO_STRAP<2:0> = RST_BOOT_METHOD_E::REMOTE.
                                                                 This bit may also be cleared by either DAP or CIC activity. */
        uint64_t rst                   : 23; /**< [ 23:  1](R/W/H) Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                                 writing a 0 releases from reset. These bits may also be cleared by either DAP or CIC
                                                                 activity.

                                                                 The upper bits of this field remain accessible but will have no effect if the cores
                                                                 are disabled. The number of bits set in RST_PP_AVAILABLE indicate the number of cores. */
        uint64_t reserved_24_63        : 40;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_rst_pp_reset cavm_rst_pp_reset_t;

#define CAVM_RST_PP_RESET CAVM_RST_PP_RESET_FUNC()
static inline uint64_t CAVM_RST_PP_RESET_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_PP_RESET_FUNC(void)
{
    return 0x87e006001740ll;
}

#define typedef_CAVM_RST_PP_RESET cavm_rst_pp_reset_t
#define bustype_CAVM_RST_PP_RESET CSR_TYPE_RSL
#define basename_CAVM_RST_PP_RESET "RST_PP_RESET"
#define device_bar_CAVM_RST_PP_RESET 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_PP_RESET 0
#define arguments_CAVM_RST_PP_RESET -1,-1,-1,-1

/**
 * Register (RSL) rst_ref_cntr
 *
 * RST Reference-Counter Register
 */
union cavm_rst_ref_cntr
{
    uint64_t u;
    struct cavm_rst_ref_cntr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](R/W/H) Count. The counter is initialized to 0x0 during a cold reset and is otherwise continuously
                                                                 running.
                                                                 CNT is incremented every reference-clock cycle (i.e. at 50 MHz). */
#else /* Word 0 - Little Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](R/W/H) Count. The counter is initialized to 0x0 during a cold reset and is otherwise continuously
                                                                 running.
                                                                 CNT is incremented every reference-clock cycle (i.e. at 50 MHz). */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_rst_ref_cntr_s cn; */
};
typedef union cavm_rst_ref_cntr cavm_rst_ref_cntr_t;

#define CAVM_RST_REF_CNTR CAVM_RST_REF_CNTR_FUNC()
static inline uint64_t CAVM_RST_REF_CNTR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_REF_CNTR_FUNC(void)
{
    return 0x87e006001758ll;
}

#define typedef_CAVM_RST_REF_CNTR cavm_rst_ref_cntr_t
#define bustype_CAVM_RST_REF_CNTR CSR_TYPE_RSL
#define basename_CAVM_RST_REF_CNTR "RST_REF_CNTR"
#define device_bar_CAVM_RST_REF_CNTR 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_REF_CNTR 0
#define arguments_CAVM_RST_REF_CNTR -1,-1,-1,-1

/**
 * Register (RSL) rst_soft_rst
 *
 * RST Soft Reset Register
 */
union cavm_rst_soft_rst
{
    uint64_t u;
    struct cavm_rst_soft_rst_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t soft_rst              : 1;  /**< [  0:  0](WO) Soft reset. When set to 1, resets the CNXXXX core. When performing a soft reset from a
                                                                 remote PCIe host,
                                                                 always read this register and wait for the results before setting [SOFT_RST] to 1. */
#else /* Word 0 - Little Endian */
        uint64_t soft_rst              : 1;  /**< [  0:  0](WO) Soft reset. When set to 1, resets the CNXXXX core. When performing a soft reset from a
                                                                 remote PCIe host,
                                                                 always read this register and wait for the results before setting [SOFT_RST] to 1. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_rst_soft_rst_s cn; */
};
typedef union cavm_rst_soft_rst cavm_rst_soft_rst_t;

#define CAVM_RST_SOFT_RST CAVM_RST_SOFT_RST_FUNC()
static inline uint64_t CAVM_RST_SOFT_RST_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_RST_SOFT_RST_FUNC(void)
{
    return 0x87e006001680ll;
}

#define typedef_CAVM_RST_SOFT_RST cavm_rst_soft_rst_t
#define bustype_CAVM_RST_SOFT_RST CSR_TYPE_RSL
#define basename_CAVM_RST_SOFT_RST "RST_SOFT_RST"
#define device_bar_CAVM_RST_SOFT_RST 0x0 /* PF_BAR0 */
#define busnum_CAVM_RST_SOFT_RST 0
#define arguments_CAVM_RST_SOFT_RST -1,-1,-1,-1

#endif /* __CAVM_CSRS_RST_H__ */

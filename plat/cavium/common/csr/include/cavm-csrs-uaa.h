#ifndef __CAVM_CSRS_UAA_H__
#define __CAVM_CSRS_UAA_H__
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
 * Cavium UAA.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration uaa_bar_e
 *
 * UART Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN81XX(a) (0x87e028000000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN81XX_SIZE 0x100000ull
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN88XX(a) (0x87e024000000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN88XX_SIZE 0x100000ull
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN83XX(a) (0x87e028000000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR0_CN83XX_SIZE 0x100000ull
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN81XX(a) (0x87e028f00000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN81XX_SIZE 0x100000ull
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN88XX(a) (0x87e024f00000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN88XX_SIZE 0x100000ull
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN83XX(a) (0x87e028f00000ll + 0x1000000ll * (a))
#define CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN83XX_SIZE 0x100000ull

/**
 * Register (RSL) uaa#_uctl_ctl
 *
 * UART UCTL Control Register
 */
union cavm_uaax_uctl_ctl
{
    uint64_t u;
    struct cavm_uaax_uctl_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_31_63        : 33;
        uint64_t h_clk_en              : 1;  /**< [ 30: 30](R/W) UART controller clock enable. When set to 1, the UART controller clock is generated. This
                                                                 also enables access to UCTL registers 0x30-0xF8. */
        uint64_t h_clk_byp_sel         : 1;  /**< [ 29: 29](R/W) Select the bypass input to the UART controller clock divider.
                                                                 0 = Use the divided coprocessor clock from the H_CLKDIV divider.
                                                                 1 = Use the bypass clock from the GPIO pins.

                                                                 This signal is just a multiplexer-select signal; it does not enable the UART
                                                                 controller and APB clock. Software must still set [H_CLK_EN]
                                                                 separately. [H_CLK_BYP_SEL] select should not be changed unless [H_CLK_EN] is
                                                                 disabled.  The bypass clock can be selected and running even if the UART
                                                                 controller clock dividers are not running.

                                                                 Internal:
                                                                 Generally bypass is only used for scan purposes. */
        uint64_t h_clkdiv_rst          : 1;  /**< [ 28: 28](R/W) UART controller clock divider reset. Divided clocks are not generated while the divider is
                                                                 being reset.
                                                                 This also resets the suspend-clock divider. */
        uint64_t reserved_27           : 1;
        uint64_t h_clkdiv_sel          : 3;  /**< [ 26: 24](R/W) The UARTCLK and APB CLK frequency select.
                                                                 The divider values are the following:
                                                                 0x0 = Divide by 1.
                                                                 0x1 = Divide by 2.
                                                                 0x2 = Divide by 4.
                                                                 0x3 = Divide by 6.
                                                                 0x4 = Divide by 8.
                                                                 0x5 = Divide by 16.
                                                                 0x6 = Divide by 24.
                                                                 0x7 = Divide by 32.

                                                                 The max and min frequency of the UARTCLK is determined by the following:
                                                                 _ f_uartclk(min) >= 16 * baud_rate(max)
                                                                 _ f_uartclk(max) <= 16 * 65535 * baud_rate(min) */
        uint64_t reserved_5_23         : 19;
        uint64_t csclk_en              : 1;  /**< [  4:  4](R/W) Turns on the UCTL interface clock (coprocessor clock).
                                                                 This enables the UCTL registers starting from 0x30 via the RSL bus. */
        uint64_t reserved_2_3          : 2;
        uint64_t uaa_rst               : 1;  /**< [  1:  1](R/W) Software reset; resets UAA controller; active-high.
                                                                 Internal:
                                                                 Note that soft-resetting the UAHC while it is active may cause violations of RSL
                                                                 protocols. */
        uint64_t uctl_rst              : 1;  /**< [  0:  0](R/W) Software reset; resets UCTL; active-high.
                                                                 Resets UCTL RSL registers 0x30-0xF8.
                                                                 Does not reset UCTL RSL registers 0x0-0x28.
                                                                 UCTL RSL registers starting from 0x30 can be accessed only after the UART controller clock
                                                                 is active and [UCTL_RST] is deasserted.

                                                                 Internal:
                                                                 Note that soft-resetting the UCTL while it is active may cause violations of
                                                                 RSL and CIB protocols. */
#else /* Word 0 - Little Endian */
        uint64_t uctl_rst              : 1;  /**< [  0:  0](R/W) Software reset; resets UCTL; active-high.
                                                                 Resets UCTL RSL registers 0x30-0xF8.
                                                                 Does not reset UCTL RSL registers 0x0-0x28.
                                                                 UCTL RSL registers starting from 0x30 can be accessed only after the UART controller clock
                                                                 is active and [UCTL_RST] is deasserted.

                                                                 Internal:
                                                                 Note that soft-resetting the UCTL while it is active may cause violations of
                                                                 RSL and CIB protocols. */
        uint64_t uaa_rst               : 1;  /**< [  1:  1](R/W) Software reset; resets UAA controller; active-high.
                                                                 Internal:
                                                                 Note that soft-resetting the UAHC while it is active may cause violations of RSL
                                                                 protocols. */
        uint64_t reserved_2_3          : 2;
        uint64_t csclk_en              : 1;  /**< [  4:  4](R/W) Turns on the UCTL interface clock (coprocessor clock).
                                                                 This enables the UCTL registers starting from 0x30 via the RSL bus. */
        uint64_t reserved_5_23         : 19;
        uint64_t h_clkdiv_sel          : 3;  /**< [ 26: 24](R/W) The UARTCLK and APB CLK frequency select.
                                                                 The divider values are the following:
                                                                 0x0 = Divide by 1.
                                                                 0x1 = Divide by 2.
                                                                 0x2 = Divide by 4.
                                                                 0x3 = Divide by 6.
                                                                 0x4 = Divide by 8.
                                                                 0x5 = Divide by 16.
                                                                 0x6 = Divide by 24.
                                                                 0x7 = Divide by 32.

                                                                 The max and min frequency of the UARTCLK is determined by the following:
                                                                 _ f_uartclk(min) >= 16 * baud_rate(max)
                                                                 _ f_uartclk(max) <= 16 * 65535 * baud_rate(min) */
        uint64_t reserved_27           : 1;
        uint64_t h_clkdiv_rst          : 1;  /**< [ 28: 28](R/W) UART controller clock divider reset. Divided clocks are not generated while the divider is
                                                                 being reset.
                                                                 This also resets the suspend-clock divider. */
        uint64_t h_clk_byp_sel         : 1;  /**< [ 29: 29](R/W) Select the bypass input to the UART controller clock divider.
                                                                 0 = Use the divided coprocessor clock from the H_CLKDIV divider.
                                                                 1 = Use the bypass clock from the GPIO pins.

                                                                 This signal is just a multiplexer-select signal; it does not enable the UART
                                                                 controller and APB clock. Software must still set [H_CLK_EN]
                                                                 separately. [H_CLK_BYP_SEL] select should not be changed unless [H_CLK_EN] is
                                                                 disabled.  The bypass clock can be selected and running even if the UART
                                                                 controller clock dividers are not running.

                                                                 Internal:
                                                                 Generally bypass is only used for scan purposes. */
        uint64_t h_clk_en              : 1;  /**< [ 30: 30](R/W) UART controller clock enable. When set to 1, the UART controller clock is generated. This
                                                                 also enables access to UCTL registers 0x30-0xF8. */
        uint64_t reserved_31_63        : 33;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_uaax_uctl_ctl_s cn; */
};
typedef union cavm_uaax_uctl_ctl cavm_uaax_uctl_ctl_t;

static inline uint64_t CAVM_UAAX_UCTL_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_UAAX_UCTL_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e028001000ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=3))
        return 0x87e028001000ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=1))
        return 0x87e024001000ll + 0x1000000ll * ((a) & 0x1);
    __cavm_csr_fatal("UAAX_UCTL_CTL", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_UAAX_UCTL_CTL(a) cavm_uaax_uctl_ctl_t
#define bustype_CAVM_UAAX_UCTL_CTL(a) CSR_TYPE_RSL
#define basename_CAVM_UAAX_UCTL_CTL(a) "UAAX_UCTL_CTL"
#define device_bar_CAVM_UAAX_UCTL_CTL(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_UAAX_UCTL_CTL(a) (a)
#define arguments_CAVM_UAAX_UCTL_CTL(a) (a),-1,-1,-1

#endif /* __CAVM_CSRS_UAA_H__ */

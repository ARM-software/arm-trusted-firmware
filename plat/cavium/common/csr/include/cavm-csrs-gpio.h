#ifndef __CAVM_CSRS_GPIO_H__
#define __CAVM_CSRS_GPIO_H__
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
 * Cavium GPIO.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration gpio_bar_e
 *
 * GPIO Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_GPIO_BAR_E_GPIO_PF_BAR0 (0x803000000000ll)
#define CAVM_GPIO_BAR_E_GPIO_PF_BAR0_SIZE 0x800000ull
#define CAVM_GPIO_BAR_E_GPIO_PF_BAR4 (0x803000f00000ll)
#define CAVM_GPIO_BAR_E_GPIO_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration gpio_int_vec_e
 *
 * GPIO MSI-X Vector Enumeration
 * Enumerates the MSI-X interrupt vectors.
 */
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CN81XX(a) (4 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CN88XX(a) (0x30 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CN83XX(a) (0x18 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CLEAR_CN81XX(a) (5 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CLEAR_CN88XX(a) (0x31 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_INTR_PINX_CLEAR_CN83XX(a) (0x19 + 2 * (a))
#define CAVM_GPIO_INT_VEC_E_MC_INTR_PPX(a) (0 + (a))

/**
 * Enumeration gpio_pin_sel_e
 *
 * GPIO Output Select Enumeration
 * Enumerates the GPIO pin function selections for GPIO_BIT_CFG()[PIN_SEL].
 * The GPIO pins can be configured as either input or output depending on selected function's
 * definition.
 * When GPIO pin is used as input pin, GPIO input is reported to selected function as well as
 * GPIO_RX_DAT.
 */
#define CAVM_GPIO_PIN_SEL_E_GPIO_CLKX(a) (5 + (a))
#define CAVM_GPIO_PIN_SEL_E_GPIO_CLK_SYNCEX(a) (3 + (a))
#define CAVM_GPIO_PIN_SEL_E_GPIO_PTP_CKOUT (1)
#define CAVM_GPIO_PIN_SEL_E_GPIO_PTP_PPS (2)
#define CAVM_GPIO_PIN_SEL_E_GPIO_SW (0)
#define CAVM_GPIO_PIN_SEL_E_LMCX_ECC_CN81XX(a) (0x237 + (a))
#define CAVM_GPIO_PIN_SEL_E_LMCX_ECC_CN88XX(a) (0x70 + (a))
#define CAVM_GPIO_PIN_SEL_E_LMCX_ECC_CN83XX(a) (0x237 + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_IN_CN81XX(a) (0x23f + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_IN_CN88XX(a) (0xb0 + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_IN_CN83XX(a) (0x23f + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_OUT_CN81XX(a) (0x242 + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_OUT_CN88XX(a) (0x14 + (a))
#define CAVM_GPIO_PIN_SEL_E_MCDX_OUT_CN83XX(a) (0x242 + (a))
#define CAVM_GPIO_PIN_SEL_E_NCSI_CRS_DV (0x258)
#define CAVM_GPIO_PIN_SEL_E_NCSI_REF_CLK (0x25c)
#define CAVM_GPIO_PIN_SEL_E_NCSI_RXDX(a) (0x25a + (a))
#define CAVM_GPIO_PIN_SEL_E_NCSI_RX_ER (0x259)
#define CAVM_GPIO_PIN_SEL_E_NCSI_TXDX(a) (0x25e + (a))
#define CAVM_GPIO_PIN_SEL_E_NCSI_TX_EN (0x25d)
#define CAVM_GPIO_PIN_SEL_E_OCLA_EXT_TRIGGER_CN81XX (0x231)
#define CAVM_GPIO_PIN_SEL_E_OCLA_EXT_TRIGGER_CN88XX (0x8a)
#define CAVM_GPIO_PIN_SEL_E_OCLA_EXT_TRIGGER_CN83XX (0x231)
#define CAVM_GPIO_PIN_SEL_E_PBUS_ADX(a) (0xfa + (a))
#define CAVM_GPIO_PIN_SEL_E_PBUS_ALEX(a) (0xe8 + (a))
#define CAVM_GPIO_PIN_SEL_E_PBUS_CEX(a) (0xec + (a))
#define CAVM_GPIO_PIN_SEL_E_PBUS_CLE (0xe0)
#define CAVM_GPIO_PIN_SEL_E_PBUS_DIR (0xe4)
#define CAVM_GPIO_PIN_SEL_E_PBUS_DMACKX(a) (0xe6 + (a))
#define CAVM_GPIO_PIN_SEL_E_PBUS_DMARQX(a) (0x11a + (a))
#define CAVM_GPIO_PIN_SEL_E_PBUS_OE (0xe3)
#define CAVM_GPIO_PIN_SEL_E_PBUS_WAIT (0xe1)
#define CAVM_GPIO_PIN_SEL_E_PBUS_WE (0xe2)
#define CAVM_GPIO_PIN_SEL_E_PCM_BCLKX(a) (0x246 + (a))
#define CAVM_GPIO_PIN_SEL_E_PCM_DATAX(a) (0x24c + (a))
#define CAVM_GPIO_PIN_SEL_E_PCM_FSYNCX(a) (0x24a + (a))
#define CAVM_GPIO_PIN_SEL_E_PTP_EVTCNT (0x252)
#define CAVM_GPIO_PIN_SEL_E_PTP_EXT_CLK (0x250)
#define CAVM_GPIO_PIN_SEL_E_PTP_TSTMP (0x251)
#define CAVM_GPIO_PIN_SEL_E_SATAX_ACT_LED_CN81XX(a) (0x16a + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_ACT_LED_CN88XX(a) (0x60 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_ACT_LED_CN83XX(a) (0x16a + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_DET_CN81XX(a) (0x18b + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_DET_CN88XX(a) (0x90 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_DET_CN83XX(a) (0x18b + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_POD_CN81XX(a) (0x145 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_POD_CN88XX(a) (0x50 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_CP_POD_CN83XX(a) (0x145 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_MP_SWITCH_CN81XX(a) (0x200 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_MP_SWITCH_CN88XX(a) (0xa0 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATAX_MP_SWITCH_CN83XX(a) (0x200 + (a))
#define CAVM_GPIO_PIN_SEL_E_SATA_LAB_LB_CN81XX (0x18a)
#define CAVM_GPIO_PIN_SEL_E_SATA_LAB_LB_CN88XX (0x8e)
#define CAVM_GPIO_PIN_SEL_E_SATA_LAB_LB_CN83XX (0x18a)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ACT_LEDX_CN81XX(a) (0xf + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ACT_LEDX_CN88XX(a) (0x20 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ACT_LEDX_CN83XX(a) (0xf + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ERR_LEDX_CN81XX(a) (0x90 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ERR_LEDX_CN88XX(a) (0x30 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_ERR_LEDX_CN83XX(a) (0x90 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_LOC_LEDX_CN81XX(a) (0x50 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_LOC_LEDX_CN88XX(a) (0x40 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_LOC_LEDX_CN83XX(a) (0x50 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SCLOCK_CN81XX (9)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SCLOCK_CN88XX (0xb)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SCLOCK_CN83XX (9)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAINX_CN81XX(a) (0xd0 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAINX_CN88XX(a) (0x80 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAINX_CN83XX(a) (0xd0 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAOUTX_CN81XX(a) (0xb + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAOUTX_CN88XX(a) (0x10 + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SDATAOUTX_CN83XX(a) (0xb + (a))
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SLOAD_CN81XX (0xa)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SLOAD_CN88XX (0xc)
#define CAVM_GPIO_PIN_SEL_E_SGPIO_SLOAD_CN83XX (0xa)
#define CAVM_GPIO_PIN_SEL_E_SPI_CSX_CN81XX(a) (0x233 + (a))
#define CAVM_GPIO_PIN_SEL_E_SPI_CSX_CN88XX(a) (0x18 + (a))
#define CAVM_GPIO_PIN_SEL_E_SPI_CSX_CN83XX(a) (0x233 + (a))
#define CAVM_GPIO_PIN_SEL_E_TIMER_CN81XX (0x11c)
#define CAVM_GPIO_PIN_SEL_E_TIMER_CN88XX (0x8b)
#define CAVM_GPIO_PIN_SEL_E_TIMER_CN83XX (0x11c)
#define CAVM_GPIO_PIN_SEL_E_TIM_GPIO_CLK (0x230)
#define CAVM_GPIO_PIN_SEL_E_UARTX_CTS(a) (0x13f + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DCD(a) (0x131 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DCD_N(a) (0x84 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DSR_CN81XX(a) (0x139 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DSR_CN88XX(a) (0x88 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DSR_CN83XX(a) (0x139 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DTR(a) (0x11d + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_DTR_N(a) (9 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_RI_CN81XX(a) (0x129 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_RI_CN88XX(a) (0x86 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_RI_CN83XX(a) (0x129 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_RTS(a) (0x123 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_SIN(a) (0x141 + (a))
#define CAVM_GPIO_PIN_SEL_E_UARTX_SOUT(a) (0x125 + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_OVR_CRNT_CN81XX(a) (0x228 + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_OVR_CRNT_CN88XX(a) (0x8c + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_OVR_CRNT_CN83XX(a) (0x228 + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_VBUS_CTRL_CN81XX(a) (0x220 + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_VBUS_CTRL_CN88XX(a) (0x74 + (a))
#define CAVM_GPIO_PIN_SEL_E_USBX_VBUS_CTRL_CN83XX(a) (0x220 + (a))

/**
 * Register (NCB) gpio_bit_cfg#
 *
 * GPIO Bit Configuration Registers
 * Each register provides configuration information for the corresponding GPIO pin.
 */
union cavm_gpio_bit_cfgx
{
    uint64_t u;
    struct cavm_gpio_bit_cfgx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_26_63        : 38;
        uint64_t pin_sel               : 10; /**< [ 25: 16](R/W) Selects which signal is reported to GPIO output, or which signal GPIO input need to
                                                                 connect. Each GPIO pin can be configured either input-only or output-only depending
                                                                 function selected, as enumerated by GPIO_PIN_SEL_E. For GPIO input selects,
                                                                 the GPIO signal used is after glitch filter and XOR inverter (GPIO_BIT_CFG()[PIN_XOR]). */
        uint64_t reserved_13_15        : 3;
        uint64_t tx_od                 : 1;  /**< [ 12: 12](R/W) GPIO output open-drain conversion. This function is after PIN_SEL MUX
                                                                 and [PIN_XOR] inverter.
                                                                 When set, the pin output will connect to zero, and pin enable
                                                                 will connect to original pin output. With both TX_OD and PIN_XOR set, a transmit
                                                                 of 1 will tristate the pin output driver to archive open-drain function. */
        uint64_t fil_sel               : 4;  /**< [ 11:  8](R/W) Filter select. Global counter bit-select (controls sample rate).
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t fil_cnt               : 4;  /**< [  7:  4](R/W) Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                                 Zero to disable the filter.
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t int_type              : 1;  /**< [  3:  3](R/W) Type of interrupt when pin is an input and [INT_EN] set. When set, rising edge
                                                                 interrupt, else level interrupt. The GPIO signal used to generate interrupt is after
                                                                 the filter as well XOR inverter. */
        uint64_t int_en                : 1;  /**< [  2:  2](R/W) Pin controls interrupt.

                                                                 If set, assertions of this pin after [PIN_XOR] will set GPIO_INTR()[INTR].

                                                                 If set and [INT_TYPE] is clear (level-sensitive), deassertions of this pin
                                                                 after [PIN_XOR] will clear GPIO_INTR()[INTR].

                                                                 This does NOT control if interrupts are enabled towards the MSI-X vector,
                                                                 for that see GPIO_INTR()[INTR_ENA]. */
        uint64_t pin_xor               : 1;  /**< [  1:  1](R/W) GPIO data inversion. When set, inverts the receiving or transmitting GPIO signal.
                                                                 For GPIO output, this inversion is after all GPIO PIN_SEL muxes. This inversion function
                                                                 is applicable to any GPIO output mux signals, also used to control GPIO_RX_DAT.
                                                                 For GPIO input, this inversion is before the GPIO PIN_SEL muxes, as used to control
                                                                 GPIO interrupts. */
        uint64_t tx_oe                 : 1;  /**< [  0:  0](R/W) Transmit output enable. When set to 1, the GPIO pin can be driven as an output pin
                                                                 if GPIO_BIT_CFG()[PIN_SEL] selects GPIO_SW. TX_OE is only used along with GPIO_TX_SET
                                                                 or GPIO_TX_CLR, and TX_OE function is before GPIO_BIT_CFG()[PIN_SEL] mux. */
#else /* Word 0 - Little Endian */
        uint64_t tx_oe                 : 1;  /**< [  0:  0](R/W) Transmit output enable. When set to 1, the GPIO pin can be driven as an output pin
                                                                 if GPIO_BIT_CFG()[PIN_SEL] selects GPIO_SW. TX_OE is only used along with GPIO_TX_SET
                                                                 or GPIO_TX_CLR, and TX_OE function is before GPIO_BIT_CFG()[PIN_SEL] mux. */
        uint64_t pin_xor               : 1;  /**< [  1:  1](R/W) GPIO data inversion. When set, inverts the receiving or transmitting GPIO signal.
                                                                 For GPIO output, this inversion is after all GPIO PIN_SEL muxes. This inversion function
                                                                 is applicable to any GPIO output mux signals, also used to control GPIO_RX_DAT.
                                                                 For GPIO input, this inversion is before the GPIO PIN_SEL muxes, as used to control
                                                                 GPIO interrupts. */
        uint64_t int_en                : 1;  /**< [  2:  2](R/W) Pin controls interrupt.

                                                                 If set, assertions of this pin after [PIN_XOR] will set GPIO_INTR()[INTR].

                                                                 If set and [INT_TYPE] is clear (level-sensitive), deassertions of this pin
                                                                 after [PIN_XOR] will clear GPIO_INTR()[INTR].

                                                                 This does NOT control if interrupts are enabled towards the MSI-X vector,
                                                                 for that see GPIO_INTR()[INTR_ENA]. */
        uint64_t int_type              : 1;  /**< [  3:  3](R/W) Type of interrupt when pin is an input and [INT_EN] set. When set, rising edge
                                                                 interrupt, else level interrupt. The GPIO signal used to generate interrupt is after
                                                                 the filter as well XOR inverter. */
        uint64_t fil_cnt               : 4;  /**< [  7:  4](R/W) Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                                 Zero to disable the filter.
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t fil_sel               : 4;  /**< [ 11:  8](R/W) Filter select. Global counter bit-select (controls sample rate).
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t tx_od                 : 1;  /**< [ 12: 12](R/W) GPIO output open-drain conversion. This function is after PIN_SEL MUX
                                                                 and [PIN_XOR] inverter.
                                                                 When set, the pin output will connect to zero, and pin enable
                                                                 will connect to original pin output. With both TX_OD and PIN_XOR set, a transmit
                                                                 of 1 will tristate the pin output driver to archive open-drain function. */
        uint64_t reserved_13_15        : 3;
        uint64_t pin_sel               : 10; /**< [ 25: 16](R/W) Selects which signal is reported to GPIO output, or which signal GPIO input need to
                                                                 connect. Each GPIO pin can be configured either input-only or output-only depending
                                                                 function selected, as enumerated by GPIO_PIN_SEL_E. For GPIO input selects,
                                                                 the GPIO signal used is after glitch filter and XOR inverter (GPIO_BIT_CFG()[PIN_XOR]). */
        uint64_t reserved_26_63        : 38;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_bit_cfgx_s cn81xx; */
    struct cavm_gpio_bit_cfgx_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_24_63        : 40;
        uint64_t pin_sel               : 8;  /**< [ 23: 16](R/W) Selects which signal is reported to GPIO output, or which signal GPIO input need to
                                                                 connect. Each GPIO pin can be configured either input-only or output-only depending
                                                                 function selected, as enumerated by GPIO_PIN_SEL_E. For GPIO input selects,
                                                                 the GPIO signal used is after glitch filter and XOR inverter (GPIO_BIT_CFG()[PIN_XOR]). */
        uint64_t reserved_13_15        : 3;
        uint64_t tx_od                 : 1;  /**< [ 12: 12](R/W) GPIO output open-drain conversion. This function is after PIN_SEL MUX
                                                                 and [PIN_XOR] inverter.
                                                                 When set, the pin output will connect to zero, and pin enable
                                                                 will connect to original pin output. With both TX_OD and PIN_XOR set, a transmit
                                                                 of 1 will tristate the pin output driver to archive open-drain function. */
        uint64_t fil_sel               : 4;  /**< [ 11:  8](R/W) Filter select. Global counter bit-select (controls sample rate).
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t fil_cnt               : 4;  /**< [  7:  4](R/W) Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                                 Zero to disable the filter.
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t int_type              : 1;  /**< [  3:  3](R/W) Type of interrupt when pin is an input and [INT_EN] set. When set, rising edge
                                                                 interrupt, else level interrupt. The GPIO signal used to generate interrupt is after
                                                                 the filter as well XOR inverter. */
        uint64_t int_en                : 1;  /**< [  2:  2](R/W) Pin controls interrupt.

                                                                 If set, assertions of this pin after [PIN_XOR] will set GPIO_INTR()[INTR].

                                                                 If set and [INT_TYPE] is clear (level-sensitive), deassertions of this pin
                                                                 after [PIN_XOR] will clear GPIO_INTR()[INTR].

                                                                 This does NOT control if interrupts are enabled towards the MSI-X vector,
                                                                 for that see GPIO_INTR()[INTR_ENA]. */
        uint64_t pin_xor               : 1;  /**< [  1:  1](R/W) GPIO data inversion. When set, inverts the receiving or transmitting GPIO signal.
                                                                 For GPIO output, this inversion is after all GPIO PIN_SEL muxes. This inversion function
                                                                 is applicable to any GPIO output mux signals, also used to control GPIO_RX_DAT.
                                                                 For GPIO input, this inversion is before the GPIO PIN_SEL muxes, as used to control
                                                                 GPIO interrupts. */
        uint64_t tx_oe                 : 1;  /**< [  0:  0](R/W) Transmit output enable. When set to 1, the GPIO pin can be driven as an output pin
                                                                 if GPIO_BIT_CFG()[PIN_SEL] selects GPIO_SW. TX_OE is only used along with GPIO_TX_SET
                                                                 or GPIO_TX_CLR, and TX_OE function is before GPIO_BIT_CFG()[PIN_SEL] mux. */
#else /* Word 0 - Little Endian */
        uint64_t tx_oe                 : 1;  /**< [  0:  0](R/W) Transmit output enable. When set to 1, the GPIO pin can be driven as an output pin
                                                                 if GPIO_BIT_CFG()[PIN_SEL] selects GPIO_SW. TX_OE is only used along with GPIO_TX_SET
                                                                 or GPIO_TX_CLR, and TX_OE function is before GPIO_BIT_CFG()[PIN_SEL] mux. */
        uint64_t pin_xor               : 1;  /**< [  1:  1](R/W) GPIO data inversion. When set, inverts the receiving or transmitting GPIO signal.
                                                                 For GPIO output, this inversion is after all GPIO PIN_SEL muxes. This inversion function
                                                                 is applicable to any GPIO output mux signals, also used to control GPIO_RX_DAT.
                                                                 For GPIO input, this inversion is before the GPIO PIN_SEL muxes, as used to control
                                                                 GPIO interrupts. */
        uint64_t int_en                : 1;  /**< [  2:  2](R/W) Pin controls interrupt.

                                                                 If set, assertions of this pin after [PIN_XOR] will set GPIO_INTR()[INTR].

                                                                 If set and [INT_TYPE] is clear (level-sensitive), deassertions of this pin
                                                                 after [PIN_XOR] will clear GPIO_INTR()[INTR].

                                                                 This does NOT control if interrupts are enabled towards the MSI-X vector,
                                                                 for that see GPIO_INTR()[INTR_ENA]. */
        uint64_t int_type              : 1;  /**< [  3:  3](R/W) Type of interrupt when pin is an input and [INT_EN] set. When set, rising edge
                                                                 interrupt, else level interrupt. The GPIO signal used to generate interrupt is after
                                                                 the filter as well XOR inverter. */
        uint64_t fil_cnt               : 4;  /**< [  7:  4](R/W) Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                                 Zero to disable the filter.
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t fil_sel               : 4;  /**< [ 11:  8](R/W) Filter select. Global counter bit-select (controls sample rate).
                                                                 Filter and XOR inverter are also applicable to GPIO input muxing signals and interrupts. */
        uint64_t tx_od                 : 1;  /**< [ 12: 12](R/W) GPIO output open-drain conversion. This function is after PIN_SEL MUX
                                                                 and [PIN_XOR] inverter.
                                                                 When set, the pin output will connect to zero, and pin enable
                                                                 will connect to original pin output. With both TX_OD and PIN_XOR set, a transmit
                                                                 of 1 will tristate the pin output driver to archive open-drain function. */
        uint64_t reserved_13_15        : 3;
        uint64_t pin_sel               : 8;  /**< [ 23: 16](R/W) Selects which signal is reported to GPIO output, or which signal GPIO input need to
                                                                 connect. Each GPIO pin can be configured either input-only or output-only depending
                                                                 function selected, as enumerated by GPIO_PIN_SEL_E. For GPIO input selects,
                                                                 the GPIO signal used is after glitch filter and XOR inverter (GPIO_BIT_CFG()[PIN_XOR]). */
        uint64_t reserved_24_63        : 40;
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_gpio_bit_cfgx_s cn83xx; */
};
typedef union cavm_gpio_bit_cfgx cavm_gpio_bit_cfgx_t;

static inline uint64_t CAVM_GPIO_BIT_CFGX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_BIT_CFGX(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=47))
        return 0x803000000400ll + 8ll * ((a) & 0x3f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=79))
        return 0x803000000400ll + 8ll * ((a) & 0x7f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=50))
        return 0x803000000400ll + 8ll * ((a) & 0x3f);
    __cavm_csr_fatal("GPIO_BIT_CFGX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_BIT_CFGX(a) cavm_gpio_bit_cfgx_t
#define bustype_CAVM_GPIO_BIT_CFGX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_BIT_CFGX(a) "GPIO_BIT_CFGX"
#define device_bar_CAVM_GPIO_BIT_CFGX(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_BIT_CFGX(a) (a)
#define arguments_CAVM_GPIO_BIT_CFGX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_clk_gen#
 *
 * GPIO Clock Generator Registers
 */
union cavm_gpio_clk_genx
{
    uint64_t u;
    struct cavm_gpio_clk_genx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_32_63        : 32;
        uint64_t n                     : 32; /**< [ 31:  0](R/W) Determines the frequency of the GPIO clock generator. N should be less than or equal to
                                                                 2^31-1.
                                                                 The frequency of the GPIO clock generator equals the coprocessor-clock frequency times N
                                                                 divided by 2^32.
                                                                 Writing N = 0x0 stops the clock generator. */
#else /* Word 0 - Little Endian */
        uint64_t n                     : 32; /**< [ 31:  0](R/W) Determines the frequency of the GPIO clock generator. N should be less than or equal to
                                                                 2^31-1.
                                                                 The frequency of the GPIO clock generator equals the coprocessor-clock frequency times N
                                                                 divided by 2^32.
                                                                 Writing N = 0x0 stops the clock generator. */
        uint64_t reserved_32_63        : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_clk_genx_s cn; */
};
typedef union cavm_gpio_clk_genx cavm_gpio_clk_genx_t;

static inline uint64_t CAVM_GPIO_CLK_GENX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_CLK_GENX(unsigned long a)
{
    if (a<=3)
        return 0x803000000040ll + 8ll * ((a) & 0x3);
    __cavm_csr_fatal("GPIO_CLK_GENX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_CLK_GENX(a) cavm_gpio_clk_genx_t
#define bustype_CAVM_GPIO_CLK_GENX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_CLK_GENX(a) "GPIO_CLK_GENX"
#define device_bar_CAVM_GPIO_CLK_GENX(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_CLK_GENX(a) (a)
#define arguments_CAVM_GPIO_CLK_GENX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_clk_synce#
 *
 * GPIO Clock SyncE Registers
 * A GSER can be configured as a clock source. The GPIO block can support up to two
 * unique clocks to send out any GPIO pin as configured when GPIO_BIT_CFG()[PIN_SEL] =
 * GPIO_PIN_SEL_E::GPIO_CLK_SYNCE(0..1). The clock can be divided by 20, 40, 80 or 160
 * of the selected GSER SerDes clock.
 */
union cavm_gpio_clk_syncex
{
    uint64_t u;
    struct cavm_gpio_clk_syncex_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..3) to select from. */
        uint64_t reserved_4_7          : 4;
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock. */
#else /* Word 0 - Little Endian */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock. */
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t reserved_4_7          : 4;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..3) to select from. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_clk_syncex_s cn81xx; */
    struct cavm_gpio_clk_syncex_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..7) to select from. */
        uint64_t reserved_4_7          : 4;
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock. */
#else /* Word 0 - Little Endian */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock. */
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t reserved_4_7          : 4;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..7) to select from. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_gpio_clk_syncex_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..6) to select from. */
        uint64_t reserved_4_7          : 4;
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock.  Note that GSER 0..3 have 4 selections each while
                                                                 GSER 4..6 have 2 selections each. */
#else /* Word 0 - Little Endian */
        uint64_t lane_sel              : 2;  /**< [  1:  0](R/W) Which RX lane within the GSER selected with [QLM_SEL] to use as the GPIO
                                                                 internal QLMx clock.  Note that GSER 0..3 have 4 selections each while
                                                                 GSER 4..6 have 2 selections each. */
        uint64_t div                   : 2;  /**< [  3:  2](R/W) GPIO internal clock divider setting relative to GSER SerDes CLOCK_SYNCE. The maximum
                                                                 supported GPIO output frequency is 125 MHz.
                                                                 0x0 = Divide by 20.
                                                                 0x1 = Divide by 40.
                                                                 0x2 = Divide by 80.
                                                                 0x3 = Divide by 160. */
        uint64_t reserved_4_7          : 4;
        uint64_t qlm_sel               : 4;  /**< [ 11:  8](R/W) Selects which GSER(0..6) to select from. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_clk_syncex cavm_gpio_clk_syncex_t;

static inline uint64_t CAVM_GPIO_CLK_SYNCEX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_CLK_SYNCEX(unsigned long a)
{
    if (a<=1)
        return 0x803000000060ll + 8ll * ((a) & 0x1);
    __cavm_csr_fatal("GPIO_CLK_SYNCEX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_CLK_SYNCEX(a) cavm_gpio_clk_syncex_t
#define bustype_CAVM_GPIO_CLK_SYNCEX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_CLK_SYNCEX(a) "GPIO_CLK_SYNCEX"
#define device_bar_CAVM_GPIO_CLK_SYNCEX(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_CLK_SYNCEX(a) (a)
#define arguments_CAVM_GPIO_CLK_SYNCEX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_comp
 *
 * GPIO Compensation Register
 */
union cavm_gpio_comp
{
    uint64_t u;
    struct cavm_gpio_comp_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_11_63        : 53;
        uint64_t pctl                  : 3;  /**< [ 10:  8](R/W) GPIO bus driver PCTL. Suggested values:
                                                                 0x4 = 60 ohm.
                                                                 0x6 = 40 ohm.
                                                                 0x7 = 30 ohm. */
        uint64_t reserved_3_7          : 5;
        uint64_t nctl                  : 3;  /**< [  2:  0](R/W) GPIO bus driver NCTL. Suggested values:
                                                                 0x4 = 60 ohm.
                                                                 0x6 = 40 ohm.
                                                                 0x7 = 30 ohm. */
#else /* Word 0 - Little Endian */
        uint64_t nctl                  : 3;  /**< [  2:  0](R/W) GPIO bus driver NCTL. Suggested values:
                                                                 0x4 = 60 ohm.
                                                                 0x6 = 40 ohm.
                                                                 0x7 = 30 ohm. */
        uint64_t reserved_3_7          : 5;
        uint64_t pctl                  : 3;  /**< [ 10:  8](R/W) GPIO bus driver PCTL. Suggested values:
                                                                 0x4 = 60 ohm.
                                                                 0x6 = 40 ohm.
                                                                 0x7 = 30 ohm. */
        uint64_t reserved_11_63        : 53;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_comp_s cn; */
};
typedef union cavm_gpio_comp cavm_gpio_comp_t;

#define CAVM_GPIO_COMP CAVM_GPIO_COMP_FUNC()
static inline uint64_t CAVM_GPIO_COMP_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_COMP_FUNC(void)
{
    return 0x803000000080ll;
}

#define typedef_CAVM_GPIO_COMP cavm_gpio_comp_t
#define bustype_CAVM_GPIO_COMP CSR_TYPE_NCB
#define basename_CAVM_GPIO_COMP "GPIO_COMP"
#define device_bar_CAVM_GPIO_COMP 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_COMP 0
#define arguments_CAVM_GPIO_COMP -1,-1,-1,-1

/**
 * Register (NCB) gpio_const
 *
 * GPIO Constants Register
 * This register contains constants for software discovery.
 */
union cavm_gpio_const
{
    uint64_t u;
    struct cavm_gpio_const_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t pp                    : 8;  /**< [ 15:  8](RO) Number of PP vectors in GPIO_INT_VEC_E::MC_INTR_PP(). */
        uint64_t gpios                 : 8;  /**< [  7:  0](RO) Number of GPIOs implemented. */
#else /* Word 0 - Little Endian */
        uint64_t gpios                 : 8;  /**< [  7:  0](RO) Number of GPIOs implemented. */
        uint64_t pp                    : 8;  /**< [ 15:  8](RO) Number of PP vectors in GPIO_INT_VEC_E::MC_INTR_PP(). */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_const_s cn; */
};
typedef union cavm_gpio_const cavm_gpio_const_t;

#define CAVM_GPIO_CONST CAVM_GPIO_CONST_FUNC()
static inline uint64_t CAVM_GPIO_CONST_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_CONST_FUNC(void)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX))
        return 0x803000000090ll;
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX))
        return 0x803000000090ll;
    __cavm_csr_fatal("GPIO_CONST", 0, 0, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_CONST cavm_gpio_const_t
#define bustype_CAVM_GPIO_CONST CSR_TYPE_NCB
#define basename_CAVM_GPIO_CONST "GPIO_CONST"
#define device_bar_CAVM_GPIO_CONST 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_CONST 0
#define arguments_CAVM_GPIO_CONST -1,-1,-1,-1

/**
 * Register (NCB) gpio_intr#
 *
 * GPIO Bit Interrupt Registers
 * Each register provides interrupt information for the corresponding GPIO pin.
 * GPIO_INTR interrupts can be level or edge interrupts depending on GPIO_BIT_CFG()[INT_TYPE].
 */
union cavm_gpio_intrx
{
    uint64_t u;
    struct cavm_gpio_intrx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t intr_ena_w1s          : 1;  /**< [  3:  3](R/W1S) GPIO signalled interrupt enable. Write one to set interrupt enable.
                                                                 [INTR_ENA_W1S] and [INTR_ENA_W1C] both read the interrupt enable state. */
        uint64_t intr_ena_w1c          : 1;  /**< [  2:  2](R/W1C) GPIO signalled interrupt enable. Write one to clear interrupt enable.
                                                                 [INTR_ENA_W1S] and [INTR_ENA_W1C] both read the interrupt enable state. */
        uint64_t intr_w1s              : 1;  /**< [  1:  1](R/W1S/H) GPIO signalled interrupt. If interrupts are edge sensitive, write one to set, otherwise
                                                                 will clear automatically when GPIO pin de-asserts.
                                                                 [INTR_W1S] and [INTR] both read the interrupt state. */
        uint64_t intr                  : 1;  /**< [  0:  0](R/W1C/H) GPIO signalled interrupt. If interrupts are edge sensitive, write one to clear, otherwise
                                                                 will clear automatically when GPIO pin de-asserts.
                                                                 [INTR_W1S] and [INTR] both read the interrupt state.
                                                                 An interrupt set event is sent when [INTR_ENA_W1S] reads as set. */
#else /* Word 0 - Little Endian */
        uint64_t intr                  : 1;  /**< [  0:  0](R/W1C/H) GPIO signalled interrupt. If interrupts are edge sensitive, write one to clear, otherwise
                                                                 will clear automatically when GPIO pin de-asserts.
                                                                 [INTR_W1S] and [INTR] both read the interrupt state.
                                                                 An interrupt set event is sent when [INTR_ENA_W1S] reads as set. */
        uint64_t intr_w1s              : 1;  /**< [  1:  1](R/W1S/H) GPIO signalled interrupt. If interrupts are edge sensitive, write one to set, otherwise
                                                                 will clear automatically when GPIO pin de-asserts.
                                                                 [INTR_W1S] and [INTR] both read the interrupt state. */
        uint64_t intr_ena_w1c          : 1;  /**< [  2:  2](R/W1C) GPIO signalled interrupt enable. Write one to clear interrupt enable.
                                                                 [INTR_ENA_W1S] and [INTR_ENA_W1C] both read the interrupt enable state. */
        uint64_t intr_ena_w1s          : 1;  /**< [  3:  3](R/W1S) GPIO signalled interrupt enable. Write one to set interrupt enable.
                                                                 [INTR_ENA_W1S] and [INTR_ENA_W1C] both read the interrupt enable state. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_intrx_s cn; */
};
typedef union cavm_gpio_intrx cavm_gpio_intrx_t;

static inline uint64_t CAVM_GPIO_INTRX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_INTRX(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=47))
        return 0x803000000800ll + 8ll * ((a) & 0x3f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=79))
        return 0x803000000800ll + 8ll * ((a) & 0x7f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=50))
        return 0x803000000800ll + 8ll * ((a) & 0x3f);
    __cavm_csr_fatal("GPIO_INTRX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_INTRX(a) cavm_gpio_intrx_t
#define bustype_CAVM_GPIO_INTRX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_INTRX(a) "GPIO_INTRX"
#define device_bar_CAVM_GPIO_INTRX(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_INTRX(a) (a)
#define arguments_CAVM_GPIO_INTRX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_mc_intr#
 *
 * GPIO Bit Multicast Interrupt Registers
 * Each register provides interrupt multicasting for GPIO(4..7),
 * and all GPIO_MC_INTR interrupts are edge interrupts.
 */
union cavm_gpio_mc_intrx
{
    uint64_t u;
    struct cavm_gpio_mc_intrx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1C/H) GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MULTI_CAST
                                                                 is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                                 interrupt a different core using the CIU, and each core will then write one to clear its
                                                                 corresponding bit in this register. */
#else /* Word 0 - Little Endian */
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1C/H) GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MULTI_CAST
                                                                 is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                                 interrupt a different core using the CIU, and each core will then write one to clear its
                                                                 corresponding bit in this register. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_mc_intrx_s cn; */
};
typedef union cavm_gpio_mc_intrx cavm_gpio_mc_intrx_t;

static inline uint64_t CAVM_GPIO_MC_INTRX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MC_INTRX(unsigned long a)
{
    if ((a>=4)&&(a<=7))
        return 0x803000001000ll + 8ll * ((a) & 0x7);
    __cavm_csr_fatal("GPIO_MC_INTRX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MC_INTRX(a) cavm_gpio_mc_intrx_t
#define bustype_CAVM_GPIO_MC_INTRX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MC_INTRX(a) "GPIO_MC_INTRX"
#define device_bar_CAVM_GPIO_MC_INTRX(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_MC_INTRX(a) (a)
#define arguments_CAVM_GPIO_MC_INTRX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_mc_intr#_ena_w1c
 *
 * GPIO Bit Multicast Interrupt Registers
 * This register clears interrupt enable bits.
 */
union cavm_gpio_mc_intrx_ena_w1c
{
    uint64_t u;
    struct cavm_gpio_mc_intrx_ena_w1c_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1C/H) Reads or clears enable for GPIO_MC_INTR(4..7)[INTR]. */
#else /* Word 0 - Little Endian */
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1C/H) Reads or clears enable for GPIO_MC_INTR(4..7)[INTR]. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_mc_intrx_ena_w1c_s cn; */
};
typedef union cavm_gpio_mc_intrx_ena_w1c cavm_gpio_mc_intrx_ena_w1c_t;

static inline uint64_t CAVM_GPIO_MC_INTRX_ENA_W1C(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MC_INTRX_ENA_W1C(unsigned long a)
{
    if ((a>=4)&&(a<=7))
        return 0x803000001200ll + 8ll * ((a) & 0x7);
    __cavm_csr_fatal("GPIO_MC_INTRX_ENA_W1C", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MC_INTRX_ENA_W1C(a) cavm_gpio_mc_intrx_ena_w1c_t
#define bustype_CAVM_GPIO_MC_INTRX_ENA_W1C(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MC_INTRX_ENA_W1C(a) "GPIO_MC_INTRX_ENA_W1C"
#define device_bar_CAVM_GPIO_MC_INTRX_ENA_W1C(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_MC_INTRX_ENA_W1C(a) (a)
#define arguments_CAVM_GPIO_MC_INTRX_ENA_W1C(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_mc_intr#_ena_w1s
 *
 * GPIO Bit Multicast Interrupt Registers
 * This register sets interrupt enable bits.
 */
union cavm_gpio_mc_intrx_ena_w1s
{
    uint64_t u;
    struct cavm_gpio_mc_intrx_ena_w1s_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1S/H) Reads or sets enable for GPIO_MC_INTR(4..7)[INTR]. */
#else /* Word 0 - Little Endian */
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1S/H) Reads or sets enable for GPIO_MC_INTR(4..7)[INTR]. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_mc_intrx_ena_w1s_s cn; */
};
typedef union cavm_gpio_mc_intrx_ena_w1s cavm_gpio_mc_intrx_ena_w1s_t;

static inline uint64_t CAVM_GPIO_MC_INTRX_ENA_W1S(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MC_INTRX_ENA_W1S(unsigned long a)
{
    if ((a>=4)&&(a<=7))
        return 0x803000001300ll + 8ll * ((a) & 0x7);
    __cavm_csr_fatal("GPIO_MC_INTRX_ENA_W1S", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MC_INTRX_ENA_W1S(a) cavm_gpio_mc_intrx_ena_w1s_t
#define bustype_CAVM_GPIO_MC_INTRX_ENA_W1S(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MC_INTRX_ENA_W1S(a) "GPIO_MC_INTRX_ENA_W1S"
#define device_bar_CAVM_GPIO_MC_INTRX_ENA_W1S(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_MC_INTRX_ENA_W1S(a) (a)
#define arguments_CAVM_GPIO_MC_INTRX_ENA_W1S(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_mc_intr#_w1s
 *
 * GPIO Bit Multicast Interrupt Registers
 * This register sets interrupt bits.
 */
union cavm_gpio_mc_intrx_w1s
{
    uint64_t u;
    struct cavm_gpio_mc_intrx_w1s_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1S/H) Reads or sets GPIO_MC_INTR(4..7)[INTR]. */
#else /* Word 0 - Little Endian */
        uint64_t intr                  : 48; /**< [ 47:  0](R/W1S/H) Reads or sets GPIO_MC_INTR(4..7)[INTR]. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_mc_intrx_w1s_s cn; */
};
typedef union cavm_gpio_mc_intrx_w1s cavm_gpio_mc_intrx_w1s_t;

static inline uint64_t CAVM_GPIO_MC_INTRX_W1S(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MC_INTRX_W1S(unsigned long a)
{
    if ((a>=4)&&(a<=7))
        return 0x803000001100ll + 8ll * ((a) & 0x7);
    __cavm_csr_fatal("GPIO_MC_INTRX_W1S", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MC_INTRX_W1S(a) cavm_gpio_mc_intrx_w1s_t
#define bustype_CAVM_GPIO_MC_INTRX_W1S(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MC_INTRX_W1S(a) "GPIO_MC_INTRX_W1S"
#define device_bar_CAVM_GPIO_MC_INTRX_W1S(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_MC_INTRX_W1S(a) (a)
#define arguments_CAVM_GPIO_MC_INTRX_W1S(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_msix_pba#
 *
 * GPIO MSI-X Pending Bit Array Registers
 * This register is the MSI-X PBA table; the bit number is indexed by the GPIO_INT_VEC_E enumeration.
 */
union cavm_gpio_msix_pbax
{
    uint64_t u;
    struct cavm_gpio_msix_pbax_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t pend                  : 64; /**< [ 63:  0](RO) Pending message for the associated GPIO_MSIX_VEC()_CTL, enumerated by
                                                                 GPIO_INT_VEC_E. Bits
                                                                 that have no associated GPIO_INT_VEC_E are 0. */
#else /* Word 0 - Little Endian */
        uint64_t pend                  : 64; /**< [ 63:  0](RO) Pending message for the associated GPIO_MSIX_VEC()_CTL, enumerated by
                                                                 GPIO_INT_VEC_E. Bits
                                                                 that have no associated GPIO_INT_VEC_E are 0. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_msix_pbax_s cn; */
};
typedef union cavm_gpio_msix_pbax cavm_gpio_msix_pbax_t;

static inline uint64_t CAVM_GPIO_MSIX_PBAX(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MSIX_PBAX(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x803000ff0000ll + 8ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=2))
        return 0x803000ff0000ll + 8ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=2))
        return 0x803000ff0000ll + 8ll * ((a) & 0x3);
    __cavm_csr_fatal("GPIO_MSIX_PBAX", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MSIX_PBAX(a) cavm_gpio_msix_pbax_t
#define bustype_CAVM_GPIO_MSIX_PBAX(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MSIX_PBAX(a) "GPIO_MSIX_PBAX"
#define device_bar_CAVM_GPIO_MSIX_PBAX(a) 0x4 /* PF_BAR4 */
#define busnum_CAVM_GPIO_MSIX_PBAX(a) (a)
#define arguments_CAVM_GPIO_MSIX_PBAX(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_msix_vec#_addr
 *
 * GPIO MSI-X Vector-Table Address Register
 * This register is the MSI-X vector table, indexed by the GPIO_INT_VEC_E enumeration.
 */
union cavm_gpio_msix_vecx_addr
{
    uint64_t u;
    struct cavm_gpio_msix_vecx_addr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_49_63        : 15;
        uint64_t addr                  : 47; /**< [ 48:  2](R/W) IOVA to use for MSI-X delivery of this vector. */
        uint64_t reserved_1            : 1;
        uint64_t secvec                : 1;  /**< [  0:  0](SR/W) Secure vector.
                                                                 0 = This vector may be read or written by either secure or nonsecure states.
                                                                 1 = This vector's GPIO_MSIX_VEC()_ADDR, GPIO_MSIX_VEC()_CTL, and corresponding
                                                                 bit of GPIO_MSIX_PBA() are RAZ/WI and does not cause a fault when accessed
                                                                 by the nonsecure world.

                                                                 If PCCPF_GPIO_VSEC_SCTL[MSIX_SEC] (for documentation, see PCCPF_XXX_VSEC_SCTL[MSIX_SEC])
                                                                 is set, all vectors are secure and function as if [SECVEC] was set. */
#else /* Word 0 - Little Endian */
        uint64_t secvec                : 1;  /**< [  0:  0](SR/W) Secure vector.
                                                                 0 = This vector may be read or written by either secure or nonsecure states.
                                                                 1 = This vector's GPIO_MSIX_VEC()_ADDR, GPIO_MSIX_VEC()_CTL, and corresponding
                                                                 bit of GPIO_MSIX_PBA() are RAZ/WI and does not cause a fault when accessed
                                                                 by the nonsecure world.

                                                                 If PCCPF_GPIO_VSEC_SCTL[MSIX_SEC] (for documentation, see PCCPF_XXX_VSEC_SCTL[MSIX_SEC])
                                                                 is set, all vectors are secure and function as if [SECVEC] was set. */
        uint64_t reserved_1            : 1;
        uint64_t addr                  : 47; /**< [ 48:  2](R/W) IOVA to use for MSI-X delivery of this vector. */
        uint64_t reserved_49_63        : 15;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_msix_vecx_addr_s cn; */
};
typedef union cavm_gpio_msix_vecx_addr cavm_gpio_msix_vecx_addr_t;

static inline uint64_t CAVM_GPIO_MSIX_VECX_ADDR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MSIX_VECX_ADDR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=99))
        return 0x803000f00000ll + 0x10ll * ((a) & 0x7f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=183))
        return 0x803000f00000ll + 0x10ll * ((a) & 0xff);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=149))
        return 0x803000f00000ll + 0x10ll * ((a) & 0xff);
    __cavm_csr_fatal("GPIO_MSIX_VECX_ADDR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MSIX_VECX_ADDR(a) cavm_gpio_msix_vecx_addr_t
#define bustype_CAVM_GPIO_MSIX_VECX_ADDR(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MSIX_VECX_ADDR(a) "GPIO_MSIX_VECX_ADDR"
#define device_bar_CAVM_GPIO_MSIX_VECX_ADDR(a) 0x4 /* PF_BAR4 */
#define busnum_CAVM_GPIO_MSIX_VECX_ADDR(a) (a)
#define arguments_CAVM_GPIO_MSIX_VECX_ADDR(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_msix_vec#_ctl
 *
 * GPIO MSI-X Vector-Table Control and Data Register
 * This register is the MSI-X vector table, indexed by the GPIO_INT_VEC_E enumeration.
 */
union cavm_gpio_msix_vecx_ctl
{
    uint64_t u;
    struct cavm_gpio_msix_vecx_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_33_63        : 31;
        uint64_t mask                  : 1;  /**< [ 32: 32](R/W) When set, no MSI-X interrupts are sent to this vector. */
        uint64_t reserved_20_31        : 12;
        uint64_t data                  : 20; /**< [ 19:  0](R/W) Data to use for MSI-X delivery of this vector. */
#else /* Word 0 - Little Endian */
        uint64_t data                  : 20; /**< [ 19:  0](R/W) Data to use for MSI-X delivery of this vector. */
        uint64_t reserved_20_31        : 12;
        uint64_t mask                  : 1;  /**< [ 32: 32](R/W) When set, no MSI-X interrupts are sent to this vector. */
        uint64_t reserved_33_63        : 31;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_msix_vecx_ctl_s cn; */
};
typedef union cavm_gpio_msix_vecx_ctl cavm_gpio_msix_vecx_ctl_t;

static inline uint64_t CAVM_GPIO_MSIX_VECX_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MSIX_VECX_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=99))
        return 0x803000f00008ll + 0x10ll * ((a) & 0x7f);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=183))
        return 0x803000f00008ll + 0x10ll * ((a) & 0xff);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=149))
        return 0x803000f00008ll + 0x10ll * ((a) & 0xff);
    __cavm_csr_fatal("GPIO_MSIX_VECX_CTL", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_MSIX_VECX_CTL(a) cavm_gpio_msix_vecx_ctl_t
#define bustype_CAVM_GPIO_MSIX_VECX_CTL(a) CSR_TYPE_NCB
#define basename_CAVM_GPIO_MSIX_VECX_CTL(a) "GPIO_MSIX_VECX_CTL"
#define device_bar_CAVM_GPIO_MSIX_VECX_CTL(a) 0x4 /* PF_BAR4 */
#define busnum_CAVM_GPIO_MSIX_VECX_CTL(a) (a)
#define arguments_CAVM_GPIO_MSIX_VECX_CTL(a) (a),-1,-1,-1

/**
 * Register (NCB) gpio_multi_cast
 *
 * GPIO Multicast Register
 * This register enables multicast GPIO interrupts.
 */
union cavm_gpio_multi_cast
{
    uint64_t u;
    struct cavm_gpio_multi_cast_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t en                    : 1;  /**< [  0:  0](R/W) Enable GPIO interrupt multicast mode. When [EN] is set, GPIO<7:4> functions in multicast
                                                                 mode allowing these four GPIOs to interrupt multiple cores. Multicast functionality allows
                                                                 the GPIO to exist as per-core interrupts as opposed to a global interrupt. */
#else /* Word 0 - Little Endian */
        uint64_t en                    : 1;  /**< [  0:  0](R/W) Enable GPIO interrupt multicast mode. When [EN] is set, GPIO<7:4> functions in multicast
                                                                 mode allowing these four GPIOs to interrupt multiple cores. Multicast functionality allows
                                                                 the GPIO to exist as per-core interrupts as opposed to a global interrupt. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_multi_cast_s cn; */
};
typedef union cavm_gpio_multi_cast cavm_gpio_multi_cast_t;

#define CAVM_GPIO_MULTI_CAST CAVM_GPIO_MULTI_CAST_FUNC()
static inline uint64_t CAVM_GPIO_MULTI_CAST_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_MULTI_CAST_FUNC(void)
{
    return 0x803000000018ll;
}

#define typedef_CAVM_GPIO_MULTI_CAST cavm_gpio_multi_cast_t
#define bustype_CAVM_GPIO_MULTI_CAST CSR_TYPE_NCB
#define basename_CAVM_GPIO_MULTI_CAST "GPIO_MULTI_CAST"
#define device_bar_CAVM_GPIO_MULTI_CAST 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_MULTI_CAST 0
#define arguments_CAVM_GPIO_MULTI_CAST -1,-1,-1,-1

/**
 * Register (NCB) gpio_ocla_exten_trig
 *
 * GPIO OCLA External Trigger Register
 */
union cavm_gpio_ocla_exten_trig
{
    uint64_t u;
    struct cavm_gpio_ocla_exten_trig_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t m_trig                : 1;  /**< [  0:  0](R/W) Manual trigger. Assert the OCLA trigger for GPIO-based triggering. This manual
                                                                 trigger is ORed with the optional GPIO input pin selected with
                                                                 GPIO_BIT_CFG()[PIN_SEL] = GPIO_PIN_SEL_E::OCLA_EXT_TRIGGER. */
#else /* Word 0 - Little Endian */
        uint64_t m_trig                : 1;  /**< [  0:  0](R/W) Manual trigger. Assert the OCLA trigger for GPIO-based triggering. This manual
                                                                 trigger is ORed with the optional GPIO input pin selected with
                                                                 GPIO_BIT_CFG()[PIN_SEL] = GPIO_PIN_SEL_E::OCLA_EXT_TRIGGER. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_ocla_exten_trig_s cn; */
};
typedef union cavm_gpio_ocla_exten_trig cavm_gpio_ocla_exten_trig_t;

#define CAVM_GPIO_OCLA_EXTEN_TRIG CAVM_GPIO_OCLA_EXTEN_TRIG_FUNC()
static inline uint64_t CAVM_GPIO_OCLA_EXTEN_TRIG_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_OCLA_EXTEN_TRIG_FUNC(void)
{
    return 0x803000000020ll;
}

#define typedef_CAVM_GPIO_OCLA_EXTEN_TRIG cavm_gpio_ocla_exten_trig_t
#define bustype_CAVM_GPIO_OCLA_EXTEN_TRIG CSR_TYPE_NCB
#define basename_CAVM_GPIO_OCLA_EXTEN_TRIG "GPIO_OCLA_EXTEN_TRIG"
#define device_bar_CAVM_GPIO_OCLA_EXTEN_TRIG 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_OCLA_EXTEN_TRIG 0
#define arguments_CAVM_GPIO_OCLA_EXTEN_TRIG -1,-1,-1,-1

/**
 * Register (NCB) gpio_rx1_dat
 *
 * GPIO Receive Data Register
 * This register contains the state of the GPIO pins, which is after glitch filter and XOR
 * inverter (GPIO_BIT_CFG()[PIN_XOR]). GPIO inputs always report to GPIO_RX1_DAT despite of
 * the value of GPIO_BIT_CFG()[PIN_SEL].
 */
union cavm_gpio_rx1_dat
{
    uint64_t u;
    struct cavm_gpio_rx1_dat_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_29_63        : 35;
        uint64_t dat                   : 29; /**< [ 28:  0](RO/H) GPIO read data. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 29; /**< [ 28:  0](RO/H) GPIO read data. */
        uint64_t reserved_29_63        : 35;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_rx1_dat_s cn81xx; */
    struct cavm_gpio_rx1_dat_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t dat                   : 16; /**< [ 15:  0](RO/H) GPIO read data. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 16; /**< [ 15:  0](RO/H) GPIO read data. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_rx1_dat cavm_gpio_rx1_dat_t;

#define CAVM_GPIO_RX1_DAT CAVM_GPIO_RX1_DAT_FUNC()
static inline uint64_t CAVM_GPIO_RX1_DAT_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_RX1_DAT_FUNC(void)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX))
        return 0x803000001400ll;
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX))
        return 0x803000001400ll;
    __cavm_csr_fatal("GPIO_RX1_DAT", 0, 0, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_RX1_DAT cavm_gpio_rx1_dat_t
#define bustype_CAVM_GPIO_RX1_DAT CSR_TYPE_NCB
#define basename_CAVM_GPIO_RX1_DAT "GPIO_RX1_DAT"
#define device_bar_CAVM_GPIO_RX1_DAT 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_RX1_DAT 0
#define arguments_CAVM_GPIO_RX1_DAT -1,-1,-1,-1

/**
 * Register (NCB) gpio_rx_dat
 *
 * GPIO Receive Data Register
 * This register contains the state of the GPIO pins, which is after glitch filter and XOR
 * inverter (GPIO_BIT_CFG()[PIN_XOR]). GPIO inputs always report to GPIO_RX_DAT despite of
 * the value of GPIO_BIT_CFG()[PIN_SEL].
 */
union cavm_gpio_rx_dat
{
    uint64_t u;
    struct cavm_gpio_rx_dat_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t dat                   : 64; /**< [ 63:  0](RO/H) GPIO read data. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 64; /**< [ 63:  0](RO/H) GPIO read data. */
#endif /* Word 0 - End */
    } s;
    struct cavm_gpio_rx_dat_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t dat                   : 48; /**< [ 47:  0](RO/H) GPIO read data. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 48; /**< [ 47:  0](RO/H) GPIO read data. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gpio_rx_dat_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_51_63        : 13;
        uint64_t dat                   : 51; /**< [ 50:  0](RO/H) GPIO read data. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 51; /**< [ 50:  0](RO/H) GPIO read data. */
        uint64_t reserved_51_63        : 13;
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_gpio_rx_dat_s cn83xx; */
};
typedef union cavm_gpio_rx_dat cavm_gpio_rx_dat_t;

#define CAVM_GPIO_RX_DAT CAVM_GPIO_RX_DAT_FUNC()
static inline uint64_t CAVM_GPIO_RX_DAT_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_RX_DAT_FUNC(void)
{
    return 0x803000000000ll;
}

#define typedef_CAVM_GPIO_RX_DAT cavm_gpio_rx_dat_t
#define bustype_CAVM_GPIO_RX_DAT CSR_TYPE_NCB
#define basename_CAVM_GPIO_RX_DAT "GPIO_RX_DAT"
#define device_bar_CAVM_GPIO_RX_DAT 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_RX_DAT 0
#define arguments_CAVM_GPIO_RX_DAT -1,-1,-1,-1

/**
 * Register (NCB) gpio_strap
 *
 * GPIO Strap Value Register
 * This register contains GPIO strap data captured at the rising edge of DC_OK.
 */
union cavm_gpio_strap
{
    uint64_t u;
    struct cavm_gpio_strap_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t strap                 : 64; /**< [ 63:  0](RO/H) GPIO strap data. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 64; /**< [ 63:  0](RO/H) GPIO strap data. */
#endif /* Word 0 - End */
    } s;
    struct cavm_gpio_strap_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t strap                 : 48; /**< [ 47:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 48; /**< [ 47:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gpio_strap_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_51_63        : 13;
        uint64_t strap                 : 51; /**< [ 50:  0](RO/H) GPIO strap data. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 51; /**< [ 50:  0](RO/H) GPIO strap data. */
        uint64_t reserved_51_63        : 13;
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_gpio_strap_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t strap                 : 64; /**< [ 63:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 64; /**< [ 63:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_strap cavm_gpio_strap_t;

#define CAVM_GPIO_STRAP CAVM_GPIO_STRAP_FUNC()
static inline uint64_t CAVM_GPIO_STRAP_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_STRAP_FUNC(void)
{
    return 0x803000000028ll;
}

#define typedef_CAVM_GPIO_STRAP cavm_gpio_strap_t
#define bustype_CAVM_GPIO_STRAP CSR_TYPE_NCB
#define basename_CAVM_GPIO_STRAP "GPIO_STRAP"
#define device_bar_CAVM_GPIO_STRAP 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_STRAP 0
#define arguments_CAVM_GPIO_STRAP -1,-1,-1,-1

/**
 * Register (NCB) gpio_strap1
 *
 * GPIO Strap Value Register
 * This register contains GPIO strap data captured at the rising edge of DC_OK.
 */
union cavm_gpio_strap1
{
    uint64_t u;
    struct cavm_gpio_strap1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_29_63        : 35;
        uint64_t strap                 : 29; /**< [ 28:  0](RO/H) GPIO strap data of GPIO pins 64-79. Unimplemented pins bits read as 0. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 29; /**< [ 28:  0](RO/H) GPIO strap data of GPIO pins 64-79. Unimplemented pins bits read as 0. */
        uint64_t reserved_29_63        : 35;
#endif /* Word 0 - End */
    } s;
    struct cavm_gpio_strap1_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_29_63        : 35;
        uint64_t strap                 : 29; /**< [ 28:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 29; /**< [ 28:  0](RO/H) GPIO strap data of GPIO pins less than 64. Unimplemented pins bits read as 0. */
        uint64_t reserved_29_63        : 35;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gpio_strap1_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t strap                 : 16; /**< [ 15:  0](RO/H) GPIO strap data of GPIO pins 64-79. Unimplemented pins bits read as 0. */
#else /* Word 0 - Little Endian */
        uint64_t strap                 : 16; /**< [ 15:  0](RO/H) GPIO strap data of GPIO pins 64-79. Unimplemented pins bits read as 0. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_strap1 cavm_gpio_strap1_t;

#define CAVM_GPIO_STRAP1 CAVM_GPIO_STRAP1_FUNC()
static inline uint64_t CAVM_GPIO_STRAP1_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_STRAP1_FUNC(void)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX))
        return 0x803000001418ll;
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX))
        return 0x803000001418ll;
    __cavm_csr_fatal("GPIO_STRAP1", 0, 0, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_STRAP1 cavm_gpio_strap1_t
#define bustype_CAVM_GPIO_STRAP1 CSR_TYPE_NCB
#define basename_CAVM_GPIO_STRAP1 "GPIO_STRAP1"
#define device_bar_CAVM_GPIO_STRAP1 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_STRAP1 0
#define arguments_CAVM_GPIO_STRAP1 -1,-1,-1,-1

/**
 * Register (NCB) gpio_tx1_clr
 *
 * GPIO Transmit Clear Mask Register
 */
union cavm_gpio_tx1_clr
{
    uint64_t u;
    struct cavm_gpio_tx1_clr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_29_63        : 35;
        uint64_t clr                   : 29; /**< [ 28:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 0. When read, CLR
                                                                 returns the GPIO_TX1_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t clr                   : 29; /**< [ 28:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 0. When read, CLR
                                                                 returns the GPIO_TX1_DAT storage. */
        uint64_t reserved_29_63        : 35;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_tx1_clr_s cn81xx; */
    struct cavm_gpio_tx1_clr_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t clr                   : 16; /**< [ 15:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 0. When read, CLR
                                                                 returns the GPIO_TX1_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t clr                   : 16; /**< [ 15:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 0. When read, CLR
                                                                 returns the GPIO_TX1_DAT storage. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_tx1_clr cavm_gpio_tx1_clr_t;

#define CAVM_GPIO_TX1_CLR CAVM_GPIO_TX1_CLR_FUNC()
static inline uint64_t CAVM_GPIO_TX1_CLR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_TX1_CLR_FUNC(void)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX))
        return 0x803000001410ll;
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX))
        return 0x803000001410ll;
    __cavm_csr_fatal("GPIO_TX1_CLR", 0, 0, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_TX1_CLR cavm_gpio_tx1_clr_t
#define bustype_CAVM_GPIO_TX1_CLR CSR_TYPE_NCB
#define basename_CAVM_GPIO_TX1_CLR "GPIO_TX1_CLR"
#define device_bar_CAVM_GPIO_TX1_CLR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_TX1_CLR 0
#define arguments_CAVM_GPIO_TX1_CLR -1,-1,-1,-1

/**
 * Register (NCB) gpio_tx1_set
 *
 * GPIO Transmit Set Mask Register
 */
union cavm_gpio_tx1_set
{
    uint64_t u;
    struct cavm_gpio_tx1_set_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_29_63        : 35;
        uint64_t set                   : 29; /**< [ 28:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 1. When read, SET
                                                                 returns the GPIO_TX1_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t set                   : 29; /**< [ 28:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 1. When read, SET
                                                                 returns the GPIO_TX1_DAT storage. */
        uint64_t reserved_29_63        : 35;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gpio_tx1_set_s cn81xx; */
    struct cavm_gpio_tx1_set_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t set                   : 16; /**< [ 15:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 1. When read, SET
                                                                 returns the GPIO_TX1_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t set                   : 16; /**< [ 15:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX1_DAT bits to set to 1. When read, SET
                                                                 returns the GPIO_TX1_DAT storage. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gpio_tx1_set cavm_gpio_tx1_set_t;

#define CAVM_GPIO_TX1_SET CAVM_GPIO_TX1_SET_FUNC()
static inline uint64_t CAVM_GPIO_TX1_SET_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_TX1_SET_FUNC(void)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX))
        return 0x803000001408ll;
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX))
        return 0x803000001408ll;
    __cavm_csr_fatal("GPIO_TX1_SET", 0, 0, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GPIO_TX1_SET cavm_gpio_tx1_set_t
#define bustype_CAVM_GPIO_TX1_SET CSR_TYPE_NCB
#define basename_CAVM_GPIO_TX1_SET "GPIO_TX1_SET"
#define device_bar_CAVM_GPIO_TX1_SET 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_TX1_SET 0
#define arguments_CAVM_GPIO_TX1_SET -1,-1,-1,-1

/**
 * Register (NCB) gpio_tx_clr
 *
 * GPIO Transmit Clear Mask Register
 */
union cavm_gpio_tx_clr
{
    uint64_t u;
    struct cavm_gpio_tx_clr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t clr                   : 64; /**< [ 63:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t clr                   : 64; /**< [ 63:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
#endif /* Word 0 - End */
    } s;
    struct cavm_gpio_tx_clr_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t clr                   : 48; /**< [ 47:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t clr                   : 48; /**< [ 47:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gpio_tx_clr_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_51_63        : 13;
        uint64_t clr                   : 51; /**< [ 50:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t clr                   : 51; /**< [ 50:  0](R/W1C/H) Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                                 returns the GPIO_TX_DAT storage. */
        uint64_t reserved_51_63        : 13;
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_gpio_tx_clr_s cn83xx; */
};
typedef union cavm_gpio_tx_clr cavm_gpio_tx_clr_t;

#define CAVM_GPIO_TX_CLR CAVM_GPIO_TX_CLR_FUNC()
static inline uint64_t CAVM_GPIO_TX_CLR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_TX_CLR_FUNC(void)
{
    return 0x803000000010ll;
}

#define typedef_CAVM_GPIO_TX_CLR cavm_gpio_tx_clr_t
#define bustype_CAVM_GPIO_TX_CLR CSR_TYPE_NCB
#define basename_CAVM_GPIO_TX_CLR "GPIO_TX_CLR"
#define device_bar_CAVM_GPIO_TX_CLR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_TX_CLR 0
#define arguments_CAVM_GPIO_TX_CLR -1,-1,-1,-1

/**
 * Register (NCB) gpio_tx_set
 *
 * GPIO Transmit Set Mask Register
 */
union cavm_gpio_tx_set
{
    uint64_t u;
    struct cavm_gpio_tx_set_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t set                   : 64; /**< [ 63:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t set                   : 64; /**< [ 63:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
#endif /* Word 0 - End */
    } s;
    struct cavm_gpio_tx_set_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_48_63        : 16;
        uint64_t set                   : 48; /**< [ 47:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t set                   : 48; /**< [ 47:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
        uint64_t reserved_48_63        : 16;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gpio_tx_set_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_51_63        : 13;
        uint64_t set                   : 51; /**< [ 50:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
#else /* Word 0 - Little Endian */
        uint64_t set                   : 51; /**< [ 50:  0](R/W1S/H) Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, [SET]
                                                                 returns the GPIO_TX_DAT storage. */
        uint64_t reserved_51_63        : 13;
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_gpio_tx_set_s cn83xx; */
};
typedef union cavm_gpio_tx_set cavm_gpio_tx_set_t;

#define CAVM_GPIO_TX_SET CAVM_GPIO_TX_SET_FUNC()
static inline uint64_t CAVM_GPIO_TX_SET_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GPIO_TX_SET_FUNC(void)
{
    return 0x803000000008ll;
}

#define typedef_CAVM_GPIO_TX_SET cavm_gpio_tx_set_t
#define bustype_CAVM_GPIO_TX_SET CSR_TYPE_NCB
#define basename_CAVM_GPIO_TX_SET "GPIO_TX_SET"
#define device_bar_CAVM_GPIO_TX_SET 0x0 /* PF_BAR0 */
#define busnum_CAVM_GPIO_TX_SET 0
#define arguments_CAVM_GPIO_TX_SET -1,-1,-1,-1

#endif /* __CAVM_CSRS_GPIO_H__ */

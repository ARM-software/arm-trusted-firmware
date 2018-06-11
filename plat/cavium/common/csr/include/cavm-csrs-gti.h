#ifndef __CAVM_CSRS_GTI_H__
#define __CAVM_CSRS_GTI_H__
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
 * Cavium GTI.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration gti_bar_e
 *
 * GTI Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_GTI_BAR_E_GTI_PF_BAR0 (0x844000000000ll)
#define CAVM_GTI_BAR_E_GTI_PF_BAR0_SIZE 0x800000ull
#define CAVM_GTI_BAR_E_GTI_PF_BAR4 (0x84400f000000ll)
#define CAVM_GTI_BAR_E_GTI_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration gti_int_vec_e
 *
 * GTI MSI-X Vector Enumeration
 * Enumerates the MSI-X interrupt vectors.
 */
#define CAVM_GTI_INT_VEC_E_CORE_WDOGX_DEL3T(a) (0xa + (a))
#define CAVM_GTI_INT_VEC_E_CORE_WDOGX_INT(a) (0x3a + (a))
#define CAVM_GTI_INT_VEC_E_ERROR (8)
#define CAVM_GTI_INT_VEC_E_MAILBOX_RX (7)
#define CAVM_GTI_INT_VEC_E_SECURE_WATCHDOG (4)
#define CAVM_GTI_INT_VEC_E_SECURE_WATCHDOG_CLEAR (5)
#define CAVM_GTI_INT_VEC_E_SPARE (9)
#define CAVM_GTI_INT_VEC_E_TX_TIMESTAMP (6)
#define CAVM_GTI_INT_VEC_E_WAKE (0)
#define CAVM_GTI_INT_VEC_E_WAKE_CLEAR (1)
#define CAVM_GTI_INT_VEC_E_WATCHDOG (2)
#define CAVM_GTI_INT_VEC_E_WATCHDOG_CLEAR (3)

/**
 * Register (NCB) gti_cc_cntadd
 *
 * GTI Counter Control Atomic Add Secure Register
 * Implementation defined register.
 */
union cavm_gti_cc_cntadd
{
    uint64_t u;
    struct cavm_gti_cc_cntadd_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t cntadd                : 64; /**< [ 63:  0](SWO) The value written to CNTADD is atomically added to GTI_CC_CNTCV. */
#else /* Word 0 - Little Endian */
        uint64_t cntadd                : 64; /**< [ 63:  0](SWO) The value written to CNTADD is atomically added to GTI_CC_CNTCV. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_cntadd_s cn; */
};
typedef union cavm_gti_cc_cntadd cavm_gti_cc_cntadd_t;

#define CAVM_GTI_CC_CNTADD CAVM_GTI_CC_CNTADD_FUNC()
static inline uint64_t CAVM_GTI_CC_CNTADD_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_CNTADD_FUNC(void)
{
    return 0x8440000000c8ll;
}

#define typedef_CAVM_GTI_CC_CNTADD cavm_gti_cc_cntadd_t
#define bustype_CAVM_GTI_CC_CNTADD CSR_TYPE_NCB
#define basename_CAVM_GTI_CC_CNTADD "GTI_CC_CNTADD"
#define device_bar_CAVM_GTI_CC_CNTADD 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_CNTADD 0
#define arguments_CAVM_GTI_CC_CNTADD -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_cntcr
 *
 * GTI Counter Control Secure Register
 */
union cavm_gti_cc_cntcr
{
    uint32_t u;
    struct cavm_gti_cc_cntcr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_9_31         : 23;
        uint32_t fcreq                 : 1;  /**< [  8:  8](SR/W) Frequency change request. Indicates the number of the entry in the frequency
                                                                 table to select. Selecting an unimplemented entry, or an entry that contains
                                                                 0x0, has no effect on the counter.

                                                                 For CNXXXX, which implements a single frequency table entry, must be 0x0. */
        uint32_t reserved_2_7          : 6;
        uint32_t hdbg                  : 1;  /**< [  1:  1](SR/W) System counter halt-on-debug enable.
                                                                 0 = System counter ignores halt-on-debug.
                                                                 1 = Asserted halt-on-debug signal halts system counter update. */
        uint32_t en                    : 1;  /**< [  0:  0](SR/W) Enables the system counter. */
#else /* Word 0 - Little Endian */
        uint32_t en                    : 1;  /**< [  0:  0](SR/W) Enables the system counter. */
        uint32_t hdbg                  : 1;  /**< [  1:  1](SR/W) System counter halt-on-debug enable.
                                                                 0 = System counter ignores halt-on-debug.
                                                                 1 = Asserted halt-on-debug signal halts system counter update. */
        uint32_t reserved_2_7          : 6;
        uint32_t fcreq                 : 1;  /**< [  8:  8](SR/W) Frequency change request. Indicates the number of the entry in the frequency
                                                                 table to select. Selecting an unimplemented entry, or an entry that contains
                                                                 0x0, has no effect on the counter.

                                                                 For CNXXXX, which implements a single frequency table entry, must be 0x0. */
        uint32_t reserved_9_31         : 23;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_cntcr_s cn; */
};
typedef union cavm_gti_cc_cntcr cavm_gti_cc_cntcr_t;

#define CAVM_GTI_CC_CNTCR CAVM_GTI_CC_CNTCR_FUNC()
static inline uint64_t CAVM_GTI_CC_CNTCR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_CNTCR_FUNC(void)
{
    return 0x844000000000ll;
}

#define typedef_CAVM_GTI_CC_CNTCR cavm_gti_cc_cntcr_t
#define bustype_CAVM_GTI_CC_CNTCR CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_CNTCR "GTI_CC_CNTCR"
#define device_bar_CAVM_GTI_CC_CNTCR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_CNTCR 0
#define arguments_CAVM_GTI_CC_CNTCR -1,-1,-1,-1

/**
 * Register (NCB) gti_cc_cntcv
 *
 * GTI Counter Control Count Value Secure Register
 */
union cavm_gti_cc_cntcv
{
    uint64_t u;
    struct cavm_gti_cc_cntcv_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](SR/W/H) System counter count value. The counter is also read-only accessible by the
                                                                 nonsecure world with GTI_RD_CNTCV. */
#else /* Word 0 - Little Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](SR/W/H) System counter count value. The counter is also read-only accessible by the
                                                                 nonsecure world with GTI_RD_CNTCV. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_cntcv_s cn; */
};
typedef union cavm_gti_cc_cntcv cavm_gti_cc_cntcv_t;

#define CAVM_GTI_CC_CNTCV CAVM_GTI_CC_CNTCV_FUNC()
static inline uint64_t CAVM_GTI_CC_CNTCV_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_CNTCV_FUNC(void)
{
    return 0x844000000008ll;
}

#define typedef_CAVM_GTI_CC_CNTCV cavm_gti_cc_cntcv_t
#define bustype_CAVM_GTI_CC_CNTCV CSR_TYPE_NCB
#define basename_CAVM_GTI_CC_CNTCV "GTI_CC_CNTCV"
#define device_bar_CAVM_GTI_CC_CNTCV 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_CNTCV 0
#define arguments_CAVM_GTI_CC_CNTCV -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_cntrate
 *
 * GTI Counter Control Count Rate Secure Register
 * Implementation defined register.
 */
union cavm_gti_cc_cntrate
{
    uint32_t u;
    struct cavm_gti_cc_cntrate_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t cntrate               : 32; /**< [ 31:  0](SR/W) Sets the system counter count rate.
                                                                 The contents of CNTRATE is a 32-bit fraction that is added to CTI_CC_CNTRACC every source
                                                                 clock. */
#else /* Word 0 - Little Endian */
        uint32_t cntrate               : 32; /**< [ 31:  0](SR/W) Sets the system counter count rate.
                                                                 The contents of CNTRATE is a 32-bit fraction that is added to CTI_CC_CNTRACC every source
                                                                 clock. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_cntrate_s cn; */
};
typedef union cavm_gti_cc_cntrate cavm_gti_cc_cntrate_t;

#define CAVM_GTI_CC_CNTRATE CAVM_GTI_CC_CNTRATE_FUNC()
static inline uint64_t CAVM_GTI_CC_CNTRATE_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_CNTRATE_FUNC(void)
{
    return 0x8440000000c0ll;
}

#define typedef_CAVM_GTI_CC_CNTRATE cavm_gti_cc_cntrate_t
#define bustype_CAVM_GTI_CC_CNTRATE CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_CNTRATE "GTI_CC_CNTRATE"
#define device_bar_CAVM_GTI_CC_CNTRATE 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_CNTRATE 0
#define arguments_CAVM_GTI_CC_CNTRATE -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_cntsr
 *
 * GTI Counter Control Status Secure Register
 */
union cavm_gti_cc_cntsr
{
    uint32_t u;
    struct cavm_gti_cc_cntsr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_9_31         : 23;
        uint32_t fcack                 : 1;  /**< [  8:  8](SRO/H) Frequency change acknowledge. Indicates the currently selected entry in the frequency
                                                                 table.

                                                                 For CNXXXX, which implements a single frequency table entry, always 0x0. */
        uint32_t reserved_2_7          : 6;
        uint32_t dbgh                  : 1;  /**< [  1:  1](SRO/H) Indicates whether the counter is halted because the halt-on-debug signal is asserted.
                                                                   0 = Counter is not halted.
                                                                   1 = Counter is halted. */
        uint32_t reserved_0            : 1;
#else /* Word 0 - Little Endian */
        uint32_t reserved_0            : 1;
        uint32_t dbgh                  : 1;  /**< [  1:  1](SRO/H) Indicates whether the counter is halted because the halt-on-debug signal is asserted.
                                                                   0 = Counter is not halted.
                                                                   1 = Counter is halted. */
        uint32_t reserved_2_7          : 6;
        uint32_t fcack                 : 1;  /**< [  8:  8](SRO/H) Frequency change acknowledge. Indicates the currently selected entry in the frequency
                                                                 table.

                                                                 For CNXXXX, which implements a single frequency table entry, always 0x0. */
        uint32_t reserved_9_31         : 23;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_cntsr_s cn; */
};
typedef union cavm_gti_cc_cntsr cavm_gti_cc_cntsr_t;

#define CAVM_GTI_CC_CNTSR CAVM_GTI_CC_CNTSR_FUNC()
static inline uint64_t CAVM_GTI_CC_CNTSR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_CNTSR_FUNC(void)
{
    return 0x844000000004ll;
}

#define typedef_CAVM_GTI_CC_CNTSR cavm_gti_cc_cntsr_t
#define bustype_CAVM_GTI_CC_CNTSR CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_CNTSR "GTI_CC_CNTSR"
#define device_bar_CAVM_GTI_CC_CNTSR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_CNTSR 0
#define arguments_CAVM_GTI_CC_CNTSR -1,-1,-1,-1

/**
 * Register (NCB) gti_cc_imp_ctl
 *
 * GTI Counter Control Implementation Control Register
 * Implementation defined register.
 */
union cavm_gti_cc_imp_ctl
{
    uint64_t u;
    struct cavm_gti_cc_imp_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t clk_src               : 1;  /**< [  0:  0](SR/W) Count source clock for GTI_CC_CNTRATE.
                                                                 0 = Coprocessor clock.
                                                                 1 = PTP PPS clock. See MIO_PTP_CLOCK_CFG[PPS]. */
#else /* Word 0 - Little Endian */
        uint64_t clk_src               : 1;  /**< [  0:  0](SR/W) Count source clock for GTI_CC_CNTRATE.
                                                                 0 = Coprocessor clock.
                                                                 1 = PTP PPS clock. See MIO_PTP_CLOCK_CFG[PPS]. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_imp_ctl_s cn; */
};
typedef union cavm_gti_cc_imp_ctl cavm_gti_cc_imp_ctl_t;

#define CAVM_GTI_CC_IMP_CTL CAVM_GTI_CC_IMP_CTL_FUNC()
static inline uint64_t CAVM_GTI_CC_IMP_CTL_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_IMP_CTL_FUNC(void)
{
    return 0x844000000100ll;
}

#define typedef_CAVM_GTI_CC_IMP_CTL cavm_gti_cc_imp_ctl_t
#define bustype_CAVM_GTI_CC_IMP_CTL CSR_TYPE_NCB
#define basename_CAVM_GTI_CC_IMP_CTL "GTI_CC_IMP_CTL"
#define device_bar_CAVM_GTI_CC_IMP_CTL 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_IMP_CTL 0
#define arguments_CAVM_GTI_CC_IMP_CTL -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr0
 *
 * GTI Counter Control Peripheral Identification Secure Register 0
 */
union cavm_gti_cc_pidr0
{
    uint32_t u;
    struct cavm_gti_cc_pidr0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t partnum0              : 8;  /**< [  7:  0](SRO) Part number <7:0>.  Indicates PCC_PIDR_PARTNUM0_E::GTI_CC. */
#else /* Word 0 - Little Endian */
        uint32_t partnum0              : 8;  /**< [  7:  0](SRO) Part number <7:0>.  Indicates PCC_PIDR_PARTNUM0_E::GTI_CC. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr0_s cn; */
};
typedef union cavm_gti_cc_pidr0 cavm_gti_cc_pidr0_t;

#define CAVM_GTI_CC_PIDR0 CAVM_GTI_CC_PIDR0_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR0_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR0_FUNC(void)
{
    return 0x844000000fe0ll;
}

#define typedef_CAVM_GTI_CC_PIDR0 cavm_gti_cc_pidr0_t
#define bustype_CAVM_GTI_CC_PIDR0 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR0 "GTI_CC_PIDR0"
#define device_bar_CAVM_GTI_CC_PIDR0 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR0 0
#define arguments_CAVM_GTI_CC_PIDR0 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr1
 *
 * GTI Counter Control Peripheral Identification Secure Register 1
 */
union cavm_gti_cc_pidr1
{
    uint32_t u;
    struct cavm_gti_cc_pidr1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t idcode                : 4;  /**< [  7:  4](SRO) JEP106 identification code <3:0>. Cavium code is 0x4C. */
        uint32_t partnum1              : 4;  /**< [  3:  0](SRO) Part number <11:8>.  Indicates PCC_PIDR_PARTNUM1_E::COMP. */
#else /* Word 0 - Little Endian */
        uint32_t partnum1              : 4;  /**< [  3:  0](SRO) Part number <11:8>.  Indicates PCC_PIDR_PARTNUM1_E::COMP. */
        uint32_t idcode                : 4;  /**< [  7:  4](SRO) JEP106 identification code <3:0>. Cavium code is 0x4C. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr1_s cn; */
};
typedef union cavm_gti_cc_pidr1 cavm_gti_cc_pidr1_t;

#define CAVM_GTI_CC_PIDR1 CAVM_GTI_CC_PIDR1_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR1_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR1_FUNC(void)
{
    return 0x844000000fe4ll;
}

#define typedef_CAVM_GTI_CC_PIDR1 cavm_gti_cc_pidr1_t
#define bustype_CAVM_GTI_CC_PIDR1 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR1 "GTI_CC_PIDR1"
#define device_bar_CAVM_GTI_CC_PIDR1 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR1 0
#define arguments_CAVM_GTI_CC_PIDR1 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr2
 *
 * GTI Counter Control Peripheral Identification Secure Register 2
 */
union cavm_gti_cc_pidr2
{
    uint32_t u;
    struct cavm_gti_cc_pidr2_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t revision              : 4;  /**< [  7:  4](SRO) Architectural revision, as assigned by ARM. */
        uint32_t jedec                 : 1;  /**< [  3:  3](SRO) JEDEC assigned. */
        uint32_t idcode                : 3;  /**< [  2:  0](SRO) JEP106 identification code <6:4>. Cavium code is 0x4C. */
#else /* Word 0 - Little Endian */
        uint32_t idcode                : 3;  /**< [  2:  0](SRO) JEP106 identification code <6:4>. Cavium code is 0x4C. */
        uint32_t jedec                 : 1;  /**< [  3:  3](SRO) JEDEC assigned. */
        uint32_t revision              : 4;  /**< [  7:  4](SRO) Architectural revision, as assigned by ARM. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr2_s cn; */
};
typedef union cavm_gti_cc_pidr2 cavm_gti_cc_pidr2_t;

#define CAVM_GTI_CC_PIDR2 CAVM_GTI_CC_PIDR2_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR2_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR2_FUNC(void)
{
    return 0x844000000fe8ll;
}

#define typedef_CAVM_GTI_CC_PIDR2 cavm_gti_cc_pidr2_t
#define bustype_CAVM_GTI_CC_PIDR2 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR2 "GTI_CC_PIDR2"
#define device_bar_CAVM_GTI_CC_PIDR2 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR2 0
#define arguments_CAVM_GTI_CC_PIDR2 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr3
 *
 * GTI Counter Control Peripheral Identification Secure Register 3
 */
union cavm_gti_cc_pidr3
{
    uint32_t u;
    struct cavm_gti_cc_pidr3_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t revand                : 4;  /**< [  7:  4](SRO) Manufacturer revision number. For CNXXXX always 0x0. */
        uint32_t cust                  : 4;  /**< [  3:  0](SRO) Customer modified. 0x1 = Overall product information should be consulted for
                                                                 product, major and minor pass numbers. */
#else /* Word 0 - Little Endian */
        uint32_t cust                  : 4;  /**< [  3:  0](SRO) Customer modified. 0x1 = Overall product information should be consulted for
                                                                 product, major and minor pass numbers. */
        uint32_t revand                : 4;  /**< [  7:  4](SRO) Manufacturer revision number. For CNXXXX always 0x0. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr3_s cn; */
};
typedef union cavm_gti_cc_pidr3 cavm_gti_cc_pidr3_t;

#define CAVM_GTI_CC_PIDR3 CAVM_GTI_CC_PIDR3_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR3_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR3_FUNC(void)
{
    return 0x844000000fecll;
}

#define typedef_CAVM_GTI_CC_PIDR3 cavm_gti_cc_pidr3_t
#define bustype_CAVM_GTI_CC_PIDR3 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR3 "GTI_CC_PIDR3"
#define device_bar_CAVM_GTI_CC_PIDR3 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR3 0
#define arguments_CAVM_GTI_CC_PIDR3 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr4
 *
 * GTI Counter Control Peripheral Identification Secure Register 4
 */
union cavm_gti_cc_pidr4
{
    uint32_t u;
    struct cavm_gti_cc_pidr4_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t pagecnt               : 4;  /**< [  7:  4](SRO) Number of log-2 4 KB blocks occupied. */
        uint32_t jepcont               : 4;  /**< [  3:  0](SRO) JEP106 continuation code. Indicates Cavium. */
#else /* Word 0 - Little Endian */
        uint32_t jepcont               : 4;  /**< [  3:  0](SRO) JEP106 continuation code. Indicates Cavium. */
        uint32_t pagecnt               : 4;  /**< [  7:  4](SRO) Number of log-2 4 KB blocks occupied. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr4_s cn; */
};
typedef union cavm_gti_cc_pidr4 cavm_gti_cc_pidr4_t;

#define CAVM_GTI_CC_PIDR4 CAVM_GTI_CC_PIDR4_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR4_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR4_FUNC(void)
{
    return 0x844000000fd0ll;
}

#define typedef_CAVM_GTI_CC_PIDR4 cavm_gti_cc_pidr4_t
#define bustype_CAVM_GTI_CC_PIDR4 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR4 "GTI_CC_PIDR4"
#define device_bar_CAVM_GTI_CC_PIDR4 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR4 0
#define arguments_CAVM_GTI_CC_PIDR4 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr5
 *
 * GTI Counter Control Peripheral Identification Secure Register 5
 */
union cavm_gti_cc_pidr5
{
    uint32_t u;
    struct cavm_gti_cc_pidr5_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_0_31         : 32;
#else /* Word 0 - Little Endian */
        uint32_t reserved_0_31         : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr5_s cn; */
};
typedef union cavm_gti_cc_pidr5 cavm_gti_cc_pidr5_t;

#define CAVM_GTI_CC_PIDR5 CAVM_GTI_CC_PIDR5_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR5_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR5_FUNC(void)
{
    return 0x844000000fd4ll;
}

#define typedef_CAVM_GTI_CC_PIDR5 cavm_gti_cc_pidr5_t
#define bustype_CAVM_GTI_CC_PIDR5 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR5 "GTI_CC_PIDR5"
#define device_bar_CAVM_GTI_CC_PIDR5 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR5 0
#define arguments_CAVM_GTI_CC_PIDR5 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr6
 *
 * GTI Counter Control Peripheral Identification Secure Register 6
 */
union cavm_gti_cc_pidr6
{
    uint32_t u;
    struct cavm_gti_cc_pidr6_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_0_31         : 32;
#else /* Word 0 - Little Endian */
        uint32_t reserved_0_31         : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr6_s cn; */
};
typedef union cavm_gti_cc_pidr6 cavm_gti_cc_pidr6_t;

#define CAVM_GTI_CC_PIDR6 CAVM_GTI_CC_PIDR6_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR6_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR6_FUNC(void)
{
    return 0x844000000fd8ll;
}

#define typedef_CAVM_GTI_CC_PIDR6 cavm_gti_cc_pidr6_t
#define bustype_CAVM_GTI_CC_PIDR6 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR6 "GTI_CC_PIDR6"
#define device_bar_CAVM_GTI_CC_PIDR6 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR6 0
#define arguments_CAVM_GTI_CC_PIDR6 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_cc_pidr7
 *
 * GTI Counter Control Peripheral Identification Secure Register 7
 */
union cavm_gti_cc_pidr7
{
    uint32_t u;
    struct cavm_gti_cc_pidr7_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_0_31         : 32;
#else /* Word 0 - Little Endian */
        uint32_t reserved_0_31         : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_cc_pidr7_s cn; */
};
typedef union cavm_gti_cc_pidr7 cavm_gti_cc_pidr7_t;

#define CAVM_GTI_CC_PIDR7 CAVM_GTI_CC_PIDR7_FUNC()
static inline uint64_t CAVM_GTI_CC_PIDR7_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CC_PIDR7_FUNC(void)
{
    return 0x844000000fdcll;
}

#define typedef_CAVM_GTI_CC_PIDR7 cavm_gti_cc_pidr7_t
#define bustype_CAVM_GTI_CC_PIDR7 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CC_PIDR7 "GTI_CC_PIDR7"
#define device_bar_CAVM_GTI_CC_PIDR7 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CC_PIDR7 0
#define arguments_CAVM_GTI_CC_PIDR7 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cidr0
 *
 * GTI Control Component Identification Register 0
 */
union cavm_gti_ctl_cidr0
{
    uint32_t u;
    struct cavm_gti_ctl_cidr0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value. */
#else /* Word 0 - Little Endian */
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cidr0_s cn; */
};
typedef union cavm_gti_ctl_cidr0 cavm_gti_ctl_cidr0_t;

#define CAVM_GTI_CTL_CIDR0 CAVM_GTI_CTL_CIDR0_FUNC()
static inline uint64_t CAVM_GTI_CTL_CIDR0_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CIDR0_FUNC(void)
{
    return 0x844000020ff0ll;
}

#define typedef_CAVM_GTI_CTL_CIDR0 cavm_gti_ctl_cidr0_t
#define bustype_CAVM_GTI_CTL_CIDR0 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CIDR0 "GTI_CTL_CIDR0"
#define device_bar_CAVM_GTI_CTL_CIDR0 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CIDR0 0
#define arguments_CAVM_GTI_CTL_CIDR0 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cidr1
 *
 * GTI Control Component Identification Register 1
 */
union cavm_gti_ctl_cidr1
{
    uint32_t u;
    struct cavm_gti_ctl_cidr1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t cclass                : 4;  /**< [  7:  4](RO) Component class. */
        uint32_t preamble              : 4;  /**< [  3:  0](RO) Preamble identification value. */
#else /* Word 0 - Little Endian */
        uint32_t preamble              : 4;  /**< [  3:  0](RO) Preamble identification value. */
        uint32_t cclass                : 4;  /**< [  7:  4](RO) Component class. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cidr1_s cn; */
};
typedef union cavm_gti_ctl_cidr1 cavm_gti_ctl_cidr1_t;

#define CAVM_GTI_CTL_CIDR1 CAVM_GTI_CTL_CIDR1_FUNC()
static inline uint64_t CAVM_GTI_CTL_CIDR1_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CIDR1_FUNC(void)
{
    return 0x844000020ff4ll;
}

#define typedef_CAVM_GTI_CTL_CIDR1 cavm_gti_ctl_cidr1_t
#define bustype_CAVM_GTI_CTL_CIDR1 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CIDR1 "GTI_CTL_CIDR1"
#define device_bar_CAVM_GTI_CTL_CIDR1 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CIDR1 0
#define arguments_CAVM_GTI_CTL_CIDR1 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cidr2
 *
 * GTI Control Component Identification Register 2
 */
union cavm_gti_ctl_cidr2
{
    uint32_t u;
    struct cavm_gti_ctl_cidr2_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value. */
#else /* Word 0 - Little Endian */
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cidr2_s cn; */
};
typedef union cavm_gti_ctl_cidr2 cavm_gti_ctl_cidr2_t;

#define CAVM_GTI_CTL_CIDR2 CAVM_GTI_CTL_CIDR2_FUNC()
static inline uint64_t CAVM_GTI_CTL_CIDR2_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CIDR2_FUNC(void)
{
    return 0x844000020ff8ll;
}

#define typedef_CAVM_GTI_CTL_CIDR2 cavm_gti_ctl_cidr2_t
#define bustype_CAVM_GTI_CTL_CIDR2 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CIDR2 "GTI_CTL_CIDR2"
#define device_bar_CAVM_GTI_CTL_CIDR2 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CIDR2 0
#define arguments_CAVM_GTI_CTL_CIDR2 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cidr3
 *
 * GTI Control Component Identification Register 3
 */
union cavm_gti_ctl_cidr3
{
    uint32_t u;
    struct cavm_gti_ctl_cidr3_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value */
#else /* Word 0 - Little Endian */
        uint32_t preamble              : 8;  /**< [  7:  0](RO) Preamble identification value */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cidr3_s cn; */
};
typedef union cavm_gti_ctl_cidr3 cavm_gti_ctl_cidr3_t;

#define CAVM_GTI_CTL_CIDR3 CAVM_GTI_CTL_CIDR3_FUNC()
static inline uint64_t CAVM_GTI_CTL_CIDR3_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CIDR3_FUNC(void)
{
    return 0x844000020ffcll;
}

#define typedef_CAVM_GTI_CTL_CIDR3 cavm_gti_ctl_cidr3_t
#define bustype_CAVM_GTI_CTL_CIDR3 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CIDR3 "GTI_CTL_CIDR3"
#define device_bar_CAVM_GTI_CTL_CIDR3 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CIDR3 0
#define arguments_CAVM_GTI_CTL_CIDR3 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cntacr0
 *
 * GTI Control Access Control 0 Register
 */
union cavm_gti_ctl_cntacr0
{
    uint32_t u;
    struct cavm_gti_ctl_cntacr0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t constant              : 32; /**< [ 31:  0](RO) Access Control 0. */
#else /* Word 0 - Little Endian */
        uint32_t constant              : 32; /**< [ 31:  0](RO) Access Control 0. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cntacr0_s cn; */
};
typedef union cavm_gti_ctl_cntacr0 cavm_gti_ctl_cntacr0_t;

#define CAVM_GTI_CTL_CNTACR0 CAVM_GTI_CTL_CNTACR0_FUNC()
static inline uint64_t CAVM_GTI_CTL_CNTACR0_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CNTACR0_FUNC(void)
{
    return 0x844000020040ll;
}

#define typedef_CAVM_GTI_CTL_CNTACR0 cavm_gti_ctl_cntacr0_t
#define bustype_CAVM_GTI_CTL_CNTACR0 CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CNTACR0 "GTI_CTL_CNTACR0"
#define device_bar_CAVM_GTI_CTL_CNTACR0 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CNTACR0 0
#define arguments_CAVM_GTI_CTL_CNTACR0 -1,-1,-1,-1

/**
 * Register (NCB32b) gti_ctl_cntfrq
 *
 * GTI Control Counter Frequency Secure Register
 */
union cavm_gti_ctl_cntfrq
{
    uint32_t u;
    struct cavm_gti_ctl_cntfrq_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t data                  : 32; /**< [ 31:  0](SR/W) Programmed by boot software with the system counter clock frequency in Hertz.
                                                                 See also GTI_CC_CNTFID0. */
#else /* Word 0 - Little Endian */
        uint32_t data                  : 32; /**< [ 31:  0](SR/W) Programmed by boot software with the system counter clock frequency in Hertz.
                                                                 See also GTI_CC_CNTFID0. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_ctl_cntfrq_s cn; */
};
typedef union cavm_gti_ctl_cntfrq cavm_gti_ctl_cntfrq_t;

#define CAVM_GTI_CTL_CNTFRQ CAVM_GTI_CTL_CNTFRQ_FUNC()
static inline uint64_t CAVM_GTI_CTL_CNTFRQ_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_CTL_CNTFRQ_FUNC(void)
{
    return 0x844000020000ll;
}

#define typedef_CAVM_GTI_CTL_CNTFRQ cavm_gti_ctl_cntfrq_t
#define bustype_CAVM_GTI_CTL_CNTFRQ CSR_TYPE_NCB32b
#define basename_CAVM_GTI_CTL_CNTFRQ "GTI_CTL_CNTFRQ"
#define device_bar_CAVM_GTI_CTL_CNTFRQ 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_CTL_CNTFRQ 0
#define arguments_CAVM_GTI_CTL_CNTFRQ -1,-1,-1,-1

/**
 * Register (NCB) gti_rd_cntcv
 *
 * GTI Counter Read Value Register
 */
union cavm_gti_rd_cntcv
{
    uint64_t u;
    struct cavm_gti_rd_cntcv_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](RO/H) System counter count value.  The counter is writable with GTI_CC_CNTCV. */
#else /* Word 0 - Little Endian */
        uint64_t cnt                   : 64; /**< [ 63:  0](RO/H) System counter count value.  The counter is writable with GTI_CC_CNTCV. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gti_rd_cntcv_s cn; */
};
typedef union cavm_gti_rd_cntcv cavm_gti_rd_cntcv_t;

#define CAVM_GTI_RD_CNTCV CAVM_GTI_RD_CNTCV_FUNC()
static inline uint64_t CAVM_GTI_RD_CNTCV_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GTI_RD_CNTCV_FUNC(void)
{
    return 0x844000010000ll;
}

#define typedef_CAVM_GTI_RD_CNTCV cavm_gti_rd_cntcv_t
#define bustype_CAVM_GTI_RD_CNTCV CSR_TYPE_NCB
#define basename_CAVM_GTI_RD_CNTCV "GTI_RD_CNTCV"
#define device_bar_CAVM_GTI_RD_CNTCV 0x0 /* PF_BAR0 */
#define busnum_CAVM_GTI_RD_CNTCV 0
#define arguments_CAVM_GTI_RD_CNTCV -1,-1,-1,-1

#endif /* __CAVM_CSRS_GTI_H__ */

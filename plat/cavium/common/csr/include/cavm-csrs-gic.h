#ifndef __CAVM_CSRS_GIC_H__
#define __CAVM_CSRS_GIC_H__
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
 * Cavium GIC.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration gic_bar_e
 *
 * GIC Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_GIC_BAR_E_GIC_PF_BAR0 (0x801000000000ll)
#define CAVM_GIC_BAR_E_GIC_PF_BAR0_SIZE 0x20000ull
#define CAVM_GIC_BAR_E_GIC_PF_BAR2 (0x801000020000ll)
#define CAVM_GIC_BAR_E_GIC_PF_BAR2_SIZE 0x20000ull
#define CAVM_GIC_BAR_E_GIC_PF_BAR4 (0x801080000000ll)
#define CAVM_GIC_BAR_E_GIC_PF_BAR4_SIZE 0x1000000ull

/**
 * Register (NCB) gic_cfg_ctlr
 *
 * GIC Implementation Secure Configuration Control Register
 * This register configures GIC features.
 */
union cavm_gic_cfg_ctlr
{
    uint64_t u;
    struct cavm_gic_cfg_ctlr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_34_63        : 30;
        uint64_t dis_redist_lpi_aggr_merge : 1;/**< [ 33: 33](SR/W) Disable aggressive SETLPIR merging in redistributors. */
        uint64_t dis_cpu_if_load_balancer : 1;/**< [ 32: 32](SR/W) Disable the CPU interface load balancer. */
        uint64_t reserved_10_31        : 22;
        uint64_t dis_lpi_pend_cache    : 1;  /**< [  9:  9](SR/W) Disable the LPI pending table cache. */
        uint64_t dis_lpi_cfg_cache     : 1;  /**< [  8:  8](SR/W) Disable the LPI configuration cache. */
        uint64_t dis_inv_hct           : 1;  /**< [  7:  7](SR/W) Disable H/W invalidating ITS HCT during ITS disable process. */
        uint64_t dis_its_cdtc          : 1;  /**< [  6:  6](SR/W) Disable 1-entry device table cache in ITS CEU. */
        uint64_t dis_its_itlb          : 1;  /**< [  5:  5](SR/W) Disable ITS ITLB (interrupt translation entry lookup buffer). */
        uint64_t dis_its_dtlb          : 1;  /**< [  4:  4](SR/W) Disable ITS DTLB (device table entry lookup buffer). */
        uint64_t reserved_3            : 1;
        uint64_t root_dist             : 1;  /**< [  2:  2](SR/W) Specifies whether the distributor on this socket is root.
                                                                 0 = Distributor is not root.
                                                                 1 = Distributor is root.

                                                                 Out of reset, this field is set. EL3 firmware will clear this field as required for multi-
                                                                 socket operation. */
        uint64_t om                    : 2;  /**< [  1:  0](SR/W) Operation mode.
                                                                 0x0 = Single-socket single-root mode.
                                                                 0x1 = Reserved.
                                                                 0x2 = Multisocket single-root mode.
                                                                 0x3 = Multisocket multiroot mode. */
#else /* Word 0 - Little Endian */
        uint64_t om                    : 2;  /**< [  1:  0](SR/W) Operation mode.
                                                                 0x0 = Single-socket single-root mode.
                                                                 0x1 = Reserved.
                                                                 0x2 = Multisocket single-root mode.
                                                                 0x3 = Multisocket multiroot mode. */
        uint64_t root_dist             : 1;  /**< [  2:  2](SR/W) Specifies whether the distributor on this socket is root.
                                                                 0 = Distributor is not root.
                                                                 1 = Distributor is root.

                                                                 Out of reset, this field is set. EL3 firmware will clear this field as required for multi-
                                                                 socket operation. */
        uint64_t reserved_3            : 1;
        uint64_t dis_its_dtlb          : 1;  /**< [  4:  4](SR/W) Disable ITS DTLB (device table entry lookup buffer). */
        uint64_t dis_its_itlb          : 1;  /**< [  5:  5](SR/W) Disable ITS ITLB (interrupt translation entry lookup buffer). */
        uint64_t dis_its_cdtc          : 1;  /**< [  6:  6](SR/W) Disable 1-entry device table cache in ITS CEU. */
        uint64_t dis_inv_hct           : 1;  /**< [  7:  7](SR/W) Disable H/W invalidating ITS HCT during ITS disable process. */
        uint64_t dis_lpi_cfg_cache     : 1;  /**< [  8:  8](SR/W) Disable the LPI configuration cache. */
        uint64_t dis_lpi_pend_cache    : 1;  /**< [  9:  9](SR/W) Disable the LPI pending table cache. */
        uint64_t reserved_10_31        : 22;
        uint64_t dis_cpu_if_load_balancer : 1;/**< [ 32: 32](SR/W) Disable the CPU interface load balancer. */
        uint64_t dis_redist_lpi_aggr_merge : 1;/**< [ 33: 33](SR/W) Disable aggressive SETLPIR merging in redistributors. */
        uint64_t reserved_34_63        : 30;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gic_cfg_ctlr_s cn81xx; */
    struct cavm_gic_cfg_ctlr_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_33_63        : 31;
        uint64_t dis_cpu_if_load_balancer : 1;/**< [ 32: 32](SR/W) Disable the CPU interface load balancer. */
        uint64_t reserved_10_31        : 22;
        uint64_t dis_lpi_pend_cache    : 1;  /**< [  9:  9](SR/W) Disable the LPI pending table cache. */
        uint64_t dis_lpi_cfg_cache     : 1;  /**< [  8:  8](SR/W) Disable the LPI configuration cache. */
        uint64_t dis_inv_hct           : 1;  /**< [  7:  7](SR/W) Disable H/W invalidating ITS HCT during ITS disable process. */
        uint64_t dis_its_cdtc          : 1;  /**< [  6:  6](SR/W) Disable 1-entry device table cache in ITS CEU. */
        uint64_t dis_its_itlb          : 1;  /**< [  5:  5](SR/W) Disable ITS ITLB (interrupt translation entry lookup buffer). */
        uint64_t dis_its_dtlb          : 1;  /**< [  4:  4](SR/W) Disable ITS DTLB (device table entry lookup buffer). */
        uint64_t reserved_3            : 1;
        uint64_t root_dist             : 1;  /**< [  2:  2](SR/W) Specifies whether the distributor on this socket is root.
                                                                 0 = Distributor is not root.
                                                                 1 = Distributor is root.

                                                                 Out of reset, this field is set. EL3 firmware will clear this field as required for multi-
                                                                 socket operation. */
        uint64_t om                    : 2;  /**< [  1:  0](SR/W) Operation mode.
                                                                 0x0 = Single-socket single-root mode.
                                                                 0x1 = Reserved.
                                                                 0x2 = Multisocket single-root mode.
                                                                 0x3 = Multisocket multiroot mode. */
#else /* Word 0 - Little Endian */
        uint64_t om                    : 2;  /**< [  1:  0](SR/W) Operation mode.
                                                                 0x0 = Single-socket single-root mode.
                                                                 0x1 = Reserved.
                                                                 0x2 = Multisocket single-root mode.
                                                                 0x3 = Multisocket multiroot mode. */
        uint64_t root_dist             : 1;  /**< [  2:  2](SR/W) Specifies whether the distributor on this socket is root.
                                                                 0 = Distributor is not root.
                                                                 1 = Distributor is root.

                                                                 Out of reset, this field is set. EL3 firmware will clear this field as required for multi-
                                                                 socket operation. */
        uint64_t reserved_3            : 1;
        uint64_t dis_its_dtlb          : 1;  /**< [  4:  4](SR/W) Disable ITS DTLB (device table entry lookup buffer). */
        uint64_t dis_its_itlb          : 1;  /**< [  5:  5](SR/W) Disable ITS ITLB (interrupt translation entry lookup buffer). */
        uint64_t dis_its_cdtc          : 1;  /**< [  6:  6](SR/W) Disable 1-entry device table cache in ITS CEU. */
        uint64_t dis_inv_hct           : 1;  /**< [  7:  7](SR/W) Disable H/W invalidating ITS HCT during ITS disable process. */
        uint64_t dis_lpi_cfg_cache     : 1;  /**< [  8:  8](SR/W) Disable the LPI configuration cache. */
        uint64_t dis_lpi_pend_cache    : 1;  /**< [  9:  9](SR/W) Disable the LPI pending table cache. */
        uint64_t reserved_10_31        : 22;
        uint64_t dis_cpu_if_load_balancer : 1;/**< [ 32: 32](SR/W) Disable the CPU interface load balancer. */
        uint64_t reserved_33_63        : 31;
#endif /* Word 0 - End */
    } cn88xx;
    /* struct cavm_gic_cfg_ctlr_s cn83xx; */
};
typedef union cavm_gic_cfg_ctlr cavm_gic_cfg_ctlr_t;

#define CAVM_GIC_CFG_CTLR CAVM_GIC_CFG_CTLR_FUNC()
static inline uint64_t CAVM_GIC_CFG_CTLR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GIC_CFG_CTLR_FUNC(void)
{
    return 0x801000010000ll;
}

#define typedef_CAVM_GIC_CFG_CTLR cavm_gic_cfg_ctlr_t
#define bustype_CAVM_GIC_CFG_CTLR CSR_TYPE_NCB
#define basename_CAVM_GIC_CFG_CTLR "GIC_CFG_CTLR"
#define device_bar_CAVM_GIC_CFG_CTLR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GIC_CFG_CTLR 0
#define arguments_CAVM_GIC_CFG_CTLR -1,-1,-1,-1

/**
 * Register (NCB32b) gicd_clrspi_nsr
 *
 * GIC Distributor Clear SPI Pending Register
 */
union cavm_gicd_clrspi_nsr
{
    uint32_t u;
    struct cavm_gicd_clrspi_nsr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_10_31        : 22;
        uint32_t spi_id                : 10; /**< [  9:  0](WO) Clear an SPI pending state (write-only). If the SPI is not pending, then the write has no
                                                                 effect.

                                                                 If the SPI ID is invalid, then the write has no effect.

                                                                 If the register is written using a nonsecure access and the value specifies a secure SPI
                                                                 and the value of the corresponding GICD_NSACR() register is less than 0x2 (i.e. does not
                                                                 permit nonsecure accesses to clear the interrupt pending state), the write has no effect. */
#else /* Word 0 - Little Endian */
        uint32_t spi_id                : 10; /**< [  9:  0](WO) Clear an SPI pending state (write-only). If the SPI is not pending, then the write has no
                                                                 effect.

                                                                 If the SPI ID is invalid, then the write has no effect.

                                                                 If the register is written using a nonsecure access and the value specifies a secure SPI
                                                                 and the value of the corresponding GICD_NSACR() register is less than 0x2 (i.e. does not
                                                                 permit nonsecure accesses to clear the interrupt pending state), the write has no effect. */
        uint32_t reserved_10_31        : 22;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gicd_clrspi_nsr_s cn; */
};
typedef union cavm_gicd_clrspi_nsr cavm_gicd_clrspi_nsr_t;

#define CAVM_GICD_CLRSPI_NSR CAVM_GICD_CLRSPI_NSR_FUNC()
static inline uint64_t CAVM_GICD_CLRSPI_NSR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GICD_CLRSPI_NSR_FUNC(void)
{
    return 0x801000000048ll;
}

#define typedef_CAVM_GICD_CLRSPI_NSR cavm_gicd_clrspi_nsr_t
#define bustype_CAVM_GICD_CLRSPI_NSR CSR_TYPE_NCB32b
#define basename_CAVM_GICD_CLRSPI_NSR "GICD_CLRSPI_NSR"
#define device_bar_CAVM_GICD_CLRSPI_NSR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GICD_CLRSPI_NSR 0
#define arguments_CAVM_GICD_CLRSPI_NSR -1,-1,-1,-1

/**
 * Register (NCB32b) gicd_clrspi_sr
 *
 * GIC Distributor Clear Secure SPI Pending Register
 */
union cavm_gicd_clrspi_sr
{
    uint32_t u;
    struct cavm_gicd_clrspi_sr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_10_31        : 22;
        uint32_t spi_id                : 10; /**< [  9:  0](SWO) Clear an SPI pending state (write-only). If the SPI is not pending, then the write has no
                                                                 effect.

                                                                 If the SPI ID is invalid, then the write has no effect.

                                                                 If the register is written using a nonsecure access, the write has no effect. */
#else /* Word 0 - Little Endian */
        uint32_t spi_id                : 10; /**< [  9:  0](SWO) Clear an SPI pending state (write-only). If the SPI is not pending, then the write has no
                                                                 effect.

                                                                 If the SPI ID is invalid, then the write has no effect.

                                                                 If the register is written using a nonsecure access, the write has no effect. */
        uint32_t reserved_10_31        : 22;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gicd_clrspi_sr_s cn; */
};
typedef union cavm_gicd_clrspi_sr cavm_gicd_clrspi_sr_t;

#define CAVM_GICD_CLRSPI_SR CAVM_GICD_CLRSPI_SR_FUNC()
static inline uint64_t CAVM_GICD_CLRSPI_SR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GICD_CLRSPI_SR_FUNC(void)
{
    return 0x801000000058ll;
}

#define typedef_CAVM_GICD_CLRSPI_SR cavm_gicd_clrspi_sr_t
#define bustype_CAVM_GICD_CLRSPI_SR CSR_TYPE_NCB32b
#define basename_CAVM_GICD_CLRSPI_SR "GICD_CLRSPI_SR"
#define device_bar_CAVM_GICD_CLRSPI_SR 0x0 /* PF_BAR0 */
#define busnum_CAVM_GICD_CLRSPI_SR 0
#define arguments_CAVM_GICD_CLRSPI_SR -1,-1,-1,-1

#endif /* __CAVM_CSRS_GIC_H__ */

#ifndef __CAVM_CSRS_GSER_H__
#define __CAVM_CSRS_GSER_H__
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
 * Cavium GSER.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration gser_bar_e
 *
 * GSER Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_GSER_BAR_E_GSERX_PF_BAR0(a) (0x87e090000000ll + 0x1000000ll * (a))
#define CAVM_GSER_BAR_E_GSERX_PF_BAR0_SIZE 0x800000ull

/**
 * Register (RSL) gser#_cfg
 *
 * GSER Configuration Register
 */
union cavm_gserx_cfg
{
    uint64_t u;
    struct cavm_gserx_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_6_63         : 58;
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
#else /* Word 0 - Little Endian */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t reserved_6_63         : 58;
#endif /* Word 0 - End */
    } s;
    struct cavm_gserx_cfg_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_6_63         : 58;
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. [SATA] must only be set for DLM3 (i.e. GSER3). */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
#else /* Word 0 - Little Endian */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. [SATA] must only be set for DLM3 (i.e. GSER3). */
        uint64_t reserved_6_63         : 58;
#endif /* Word 0 - End */
    } cn81xx;
    struct cavm_gserx_cfg_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_6_63         : 58;
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI and XLAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set. For CCPI links, [BGX] must be clear.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set. For CCPI QLMs, [PCIE] must be clear.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
#else /* Word 0 - Little Endian */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set. For CCPI QLMs, [PCIE] must be clear.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set. For CCPI links, [BGX] must be clear.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI and XLAUI protocols.

                                                                 Internal:
                                                                 Not used in CCPI QLMs. */
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t reserved_6_63         : 58;
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_gserx_cfg_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_6_63         : 58;
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI and XLAUI protocols.

                                                                 Internal:
                                                                 There is hardware to pair DLM 5 and 6 together when [BGX_QUAD] is set in DLM5.
                                                                 But we currently do not support XAUI/DXAUI/XLAUI on DLM's. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 [BGX_DUAL] must not be set in a DLM.

                                                                 Internal:
                                                                 [BGX_DUAL] should work in a DLM (lanes 0 and 1 bundled for one BGX controller), but
                                                                 we currently do not support RXAUI in a DLM. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set. */
#else /* Word 0 - Little Endian */
        uint64_t pcie                  : 1;  /**< [  0:  0](R/W/H) When set, indicates the GSER is configured for PCIE mode. [PCIE] must not be
                                                                 set when either of [BGX,SATA] is set. */
        uint64_t ila                   : 1;  /**< [  1:  1](R/W) Reserved. */
        uint64_t bgx                   : 1;  /**< [  2:  2](R/W) When set, indicates the GSER is configured for BGX mode. [BGX] must not be set
                                                                 when either of [PCIE,SATA] are set.

                                                                 When [BGX] is set and both [BGX_DUAL,BGX_QUAD] are clear, GSER exposes each lane to an
                                                                 independent BGX controller. */
        uint64_t bgx_dual              : 1;  /**< [  3:  3](R/W) When set, indicates the QLM is in BGX dual aggregation mode. [BGX_DUAL] must only be
                                                                 set when [BGX] is also set and [BGX_QUAD] is clear.

                                                                 When [BGX_DUAL] is set, GSER bundles lanes 0 and 1 for one BGX controller and bundles
                                                                 lanes 2 and 3 for another BGX controller. [BGX_DUAL] must only be set for the RXAUI
                                                                 protocol.

                                                                 [BGX_DUAL] must not be set in a DLM.

                                                                 Internal:
                                                                 [BGX_DUAL] should work in a DLM (lanes 0 and 1 bundled for one BGX controller), but
                                                                 we currently do not support RXAUI in a DLM. */
        uint64_t bgx_quad              : 1;  /**< [  4:  4](R/W) When set, indicates the QLM is in BGX quad aggregation mode. [BGX_QUAD] must only be
                                                                 set when [BGX] is set and [BGX_DUAL] is clear.

                                                                 When [BGX_QUAD] is set, GSER bundles all four lanes for one BGX controller.
                                                                 [BGX_QUAD] must only be set for the XAUI/DXAUI and XLAUI protocols.

                                                                 Internal:
                                                                 There is hardware to pair DLM 5 and 6 together when [BGX_QUAD] is set in DLM5.
                                                                 But we currently do not support XAUI/DXAUI/XLAUI on DLM's. */
        uint64_t sata                  : 1;  /**< [  5:  5](R/W) When set, indicates the GSER is configured for SATA mode. [SATA] must not be set
                                                                 when either of [BGX,PCIE] are set. */
        uint64_t reserved_6_63         : 58;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gserx_cfg cavm_gserx_cfg_t;

static inline uint64_t CAVM_GSERX_CFG(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_CFG(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090000080ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090000080ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090000080ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_CFG", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_CFG(a) cavm_gserx_cfg_t
#define bustype_CAVM_GSERX_CFG(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_CFG(a) "GSERX_CFG"
#define device_bar_CAVM_GSERX_CFG(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_CFG(a) (a)
#define arguments_CAVM_GSERX_CFG(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_dbg
 *
 * GSER Debug Control Register
 */
union cavm_gserx_dbg
{
    uint64_t u;
    struct cavm_gserx_dbg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t rxqtm_on              : 1;  /**< [  0:  0](R/W) For non-BGX configurations, setting this bit enables the RX FIFOs. This allows
                                                                 received data to become visible to the RSL debug port. For diagnostic use only. */
#else /* Word 0 - Little Endian */
        uint64_t rxqtm_on              : 1;  /**< [  0:  0](R/W) For non-BGX configurations, setting this bit enables the RX FIFOs. This allows
                                                                 received data to become visible to the RSL debug port. For diagnostic use only. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_dbg_s cn; */
};
typedef union cavm_gserx_dbg cavm_gserx_dbg_t;

static inline uint64_t CAVM_GSERX_DBG(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_DBG(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090000098ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090000098ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090000098ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_DBG", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_DBG(a) cavm_gserx_dbg_t
#define bustype_CAVM_GSERX_DBG(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_DBG(a) "GSERX_DBG"
#define device_bar_CAVM_GSERX_DBG(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_DBG(a) (a)
#define arguments_CAVM_GSERX_DBG(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_eq_wait_time
 *
 * GSER TX and RX Equalization Wait Times Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_eq_wait_time
{
    uint64_t u;
    struct cavm_gserx_eq_wait_time_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_8_63         : 56;
        uint64_t rxeq_wait_cnt         : 4;  /**< [  7:  4](R/W) Determines the wait time after VMA RX-EQ completes and before sampling
                                                                 tap1 and starting the precorrelation check. */
        uint64_t txeq_wait_cnt         : 4;  /**< [  3:  0](R/W) Determines the wait time from applying the TX-EQ controls (swing/pre/post)
                                                                 to the sampling of the sds_pcs_tx_comp_out. */
#else /* Word 0 - Little Endian */
        uint64_t txeq_wait_cnt         : 4;  /**< [  3:  0](R/W) Determines the wait time from applying the TX-EQ controls (swing/pre/post)
                                                                 to the sampling of the sds_pcs_tx_comp_out. */
        uint64_t rxeq_wait_cnt         : 4;  /**< [  7:  4](R/W) Determines the wait time after VMA RX-EQ completes and before sampling
                                                                 tap1 and starting the precorrelation check. */
        uint64_t reserved_8_63         : 56;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_eq_wait_time_s cn; */
};
typedef union cavm_gserx_eq_wait_time cavm_gserx_eq_wait_time_t;

static inline uint64_t CAVM_GSERX_EQ_WAIT_TIME(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_EQ_WAIT_TIME(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e0904e0000ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e0904e0000ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e0904e0000ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_EQ_WAIT_TIME", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_EQ_WAIT_TIME(a) cavm_gserx_eq_wait_time_t
#define bustype_CAVM_GSERX_EQ_WAIT_TIME(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_EQ_WAIT_TIME(a) "GSERX_EQ_WAIT_TIME"
#define device_bar_CAVM_GSERX_EQ_WAIT_TIME(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_EQ_WAIT_TIME(a) (a)
#define arguments_CAVM_GSERX_EQ_WAIT_TIME(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_misc_config_1
 *
 * GSER Global Miscellaneous Configuration 1 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_misc_config_1
{
    uint64_t u;
    struct cavm_gserx_glbl_misc_config_1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_10_63        : 54;
        uint64_t pcs_sds_vref_tr       : 4;  /**< [  9:  6](R/W) Trim the BGR (band gap reference) reference (all external and internal currents
                                                                 are affected).
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_trim_chp_reg  : 2;  /**< [  5:  4](R/W) Trim current going to CML-CMOS stage at output of VCO.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_vco_reg_tr    : 2;  /**< [  3:  2](R/W) Trims regulator voltage.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_cvbg_en       : 1;  /**< [  1:  1](R/W) Forces 0.6 V from VDDHV onto VBG node.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_extvbg_en     : 1;  /**< [  0:  0](R/W) Force external VBG through AMON pin in TMA5 mode.
                                                                 For diagnostic use only. */
#else /* Word 0 - Little Endian */
        uint64_t pcs_sds_extvbg_en     : 1;  /**< [  0:  0](R/W) Force external VBG through AMON pin in TMA5 mode.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_cvbg_en       : 1;  /**< [  1:  1](R/W) Forces 0.6 V from VDDHV onto VBG node.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_vco_reg_tr    : 2;  /**< [  3:  2](R/W) Trims regulator voltage.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_trim_chp_reg  : 2;  /**< [  5:  4](R/W) Trim current going to CML-CMOS stage at output of VCO.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_vref_tr       : 4;  /**< [  9:  6](R/W) Trim the BGR (band gap reference) reference (all external and internal currents
                                                                 are affected).
                                                                 For diagnostic use only. */
        uint64_t reserved_10_63        : 54;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_misc_config_1_s cn; */
};
typedef union cavm_gserx_glbl_misc_config_1 cavm_gserx_glbl_misc_config_1_t;

static inline uint64_t CAVM_GSERX_GLBL_MISC_CONFIG_1(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_MISC_CONFIG_1(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460030ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460030ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460030ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_MISC_CONFIG_1", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) cavm_gserx_glbl_misc_config_1_t
#define bustype_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) "GSERX_GLBL_MISC_CONFIG_1"
#define device_bar_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) (a)
#define arguments_CAVM_GSERX_GLBL_MISC_CONFIG_1(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_pll_cfg_0
 *
 * GSER Global PLL Configuration 0 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_pll_cfg_0
{
    uint64_t u;
    struct cavm_gserx_glbl_pll_cfg_0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_14_63        : 50;
        uint64_t pcs_sds_pll_vco_reset_b : 1;/**< [ 13: 13](R/W) VCO reset, active low.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_strt_cal_b : 1; /**< [ 12: 12](R/W) Start PLL calibration, active low.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_cripple   : 1;  /**< [ 11: 11](R/W) Ripple capacitor tuning.
                                                                 For diagnostic use only. */
        uint64_t reserved_8_10         : 3;
        uint64_t pcs_sds_pll_fthresh   : 2;  /**< [  7:  6](R/W/H) PLL frequency comparison threshold.
                                                                 For diagnostic use only. */
        uint64_t reserved_0_5          : 6;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_5          : 6;
        uint64_t pcs_sds_pll_fthresh   : 2;  /**< [  7:  6](R/W/H) PLL frequency comparison threshold.
                                                                 For diagnostic use only. */
        uint64_t reserved_8_10         : 3;
        uint64_t pcs_sds_pll_cripple   : 1;  /**< [ 11: 11](R/W) Ripple capacitor tuning.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_strt_cal_b : 1; /**< [ 12: 12](R/W) Start PLL calibration, active low.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_vco_reset_b : 1;/**< [ 13: 13](R/W) VCO reset, active low.
                                                                 For diagnostic use only. */
        uint64_t reserved_14_63        : 50;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_pll_cfg_0_s cn; */
};
typedef union cavm_gserx_glbl_pll_cfg_0 cavm_gserx_glbl_pll_cfg_0_t;

static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_0(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_0(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460000ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460000ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460000ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_PLL_CFG_0", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_PLL_CFG_0(a) cavm_gserx_glbl_pll_cfg_0_t
#define bustype_CAVM_GSERX_GLBL_PLL_CFG_0(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_PLL_CFG_0(a) "GSERX_GLBL_PLL_CFG_0"
#define device_bar_CAVM_GSERX_GLBL_PLL_CFG_0(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_PLL_CFG_0(a) (a)
#define arguments_CAVM_GSERX_GLBL_PLL_CFG_0(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_pll_cfg_1
 *
 * GSER Global PLL Configuration 1 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_pll_cfg_1
{
    uint64_t u;
    struct cavm_gserx_glbl_pll_cfg_1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_10_63        : 54;
        uint64_t cfg_pll_ctrl_en       : 1;  /**< [  9:  9](R/W) PLL reset control enable.
                                                                 0 = PLL RESETs/cal start are not active.
                                                                 1 = All PLL RESETs/cal start are enabled.

                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_calmode   : 3;  /**< [  8:  6](R/W) PLL calibration mode.
                                                                 0 = Force PLL loop into calibration mode.
                                                                 1 = Normal operation.

                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_cal_ovrd_en : 1;/**< [  5:  5](R/W) Manual PLL coarse calibration override enable.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_cal_ovrd  : 5;  /**< [  4:  0](R/W) Manual PLL coarse calibration override value.
                                                                 For diagnostic use only. */
#else /* Word 0 - Little Endian */
        uint64_t pcs_sds_pll_cal_ovrd  : 5;  /**< [  4:  0](R/W) Manual PLL coarse calibration override value.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_cal_ovrd_en : 1;/**< [  5:  5](R/W) Manual PLL coarse calibration override enable.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_calmode   : 3;  /**< [  8:  6](R/W) PLL calibration mode.
                                                                 0 = Force PLL loop into calibration mode.
                                                                 1 = Normal operation.

                                                                 For diagnostic use only. */
        uint64_t cfg_pll_ctrl_en       : 1;  /**< [  9:  9](R/W) PLL reset control enable.
                                                                 0 = PLL RESETs/cal start are not active.
                                                                 1 = All PLL RESETs/cal start are enabled.

                                                                 For diagnostic use only. */
        uint64_t reserved_10_63        : 54;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_pll_cfg_1_s cn; */
};
typedef union cavm_gserx_glbl_pll_cfg_1 cavm_gserx_glbl_pll_cfg_1_t;

static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_1(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_1(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460008ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460008ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460008ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_PLL_CFG_1", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_PLL_CFG_1(a) cavm_gserx_glbl_pll_cfg_1_t
#define bustype_CAVM_GSERX_GLBL_PLL_CFG_1(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_PLL_CFG_1(a) "GSERX_GLBL_PLL_CFG_1"
#define device_bar_CAVM_GSERX_GLBL_PLL_CFG_1(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_PLL_CFG_1(a) (a)
#define arguments_CAVM_GSERX_GLBL_PLL_CFG_1(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_pll_cfg_2
 *
 * GSER Global PLL Configuration 2 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_pll_cfg_2
{
    uint64_t u;
    struct cavm_gserx_glbl_pll_cfg_2_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_15_63        : 49;
        uint64_t pll_div_ovrrd_en      : 1;  /**< [ 14: 14](R/W) Override global power state machine and mac_pcs_pll_div control signal.
                                                                 When asserted, pcs_sds_pll_div is specified from
                                                                 GSER()_LANE()_PCS_PLL_CTLIFC_0[PLL_DIV_OVRRD_VAL],
                                                                 global power state machine and mac_pcs_pll_div control signals are ignored.
                                                                 For diagnostic use only. */
        uint64_t reserved_10_13        : 4;
        uint64_t pcs_sds_pll_lock_override : 1;/**< [  9:  9](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_counter_resetn : 1;/**< [  8:  8](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t pll_sdsck_pd_ovrrd_val : 1; /**< [  7:  7](R/W) Clock tree powerdown override value.
                                                                 For diagnostic use only. */
        uint64_t pll_sdsck_pd_ovrrd_en : 1;  /**< [  6:  6](R/W) Clock tree powerdown override enable.
                                                                 For diagnostic use only. */
        uint64_t pll_pd_ovrrd_val      : 1;  /**< [  5:  5](R/W) PLL powerdown override value.
                                                                 For diagnostic use only. */
        uint64_t pll_pd_ovrrd_en       : 1;  /**< [  4:  4](R/W) When asserted, overrides PLL powerdown from state machine.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_div5_byp  : 1;  /**< [  3:  3](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t pll_band_sel_ovrrd_val : 1; /**< [  2:  2](R/W) State machine override value for VCO band select.
                                                                 0 = Low band VCO0 (RO-VCO).
                                                                 1 = High band VCO1 (LC-VCO).

                                                                 For diagnostic use only. */
        uint64_t pll_band_sel_ovrrd_en : 1;  /**< [  1:  1](R/W) PLL band select override enable.
                                                                 For diagnostic use only. */
        uint64_t pll_pcs_div_ovrrd_en  : 1;  /**< [  0:  0](R/W) Override global power state machine and mac_pcs_pll_div control signal.
                                                                 When asserted, pcs_sds_pll_div is specified from
                                                                 GSER()_LANE()_PCS_PLL_CTLIFC_1[PLL_PCS_DIV_OVRRD_VAL],
                                                                 global power state machine and mac_pcs_pll_div control signals are ignored.
                                                                 For diagnostic use only. */
#else /* Word 0 - Little Endian */
        uint64_t pll_pcs_div_ovrrd_en  : 1;  /**< [  0:  0](R/W) Override global power state machine and mac_pcs_pll_div control signal.
                                                                 When asserted, pcs_sds_pll_div is specified from
                                                                 GSER()_LANE()_PCS_PLL_CTLIFC_1[PLL_PCS_DIV_OVRRD_VAL],
                                                                 global power state machine and mac_pcs_pll_div control signals are ignored.
                                                                 For diagnostic use only. */
        uint64_t pll_band_sel_ovrrd_en : 1;  /**< [  1:  1](R/W) PLL band select override enable.
                                                                 For diagnostic use only. */
        uint64_t pll_band_sel_ovrrd_val : 1; /**< [  2:  2](R/W) State machine override value for VCO band select.
                                                                 0 = Low band VCO0 (RO-VCO).
                                                                 1 = High band VCO1 (LC-VCO).

                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_div5_byp  : 1;  /**< [  3:  3](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t pll_pd_ovrrd_en       : 1;  /**< [  4:  4](R/W) When asserted, overrides PLL powerdown from state machine.
                                                                 For diagnostic use only. */
        uint64_t pll_pd_ovrrd_val      : 1;  /**< [  5:  5](R/W) PLL powerdown override value.
                                                                 For diagnostic use only. */
        uint64_t pll_sdsck_pd_ovrrd_en : 1;  /**< [  6:  6](R/W) Clock tree powerdown override enable.
                                                                 For diagnostic use only. */
        uint64_t pll_sdsck_pd_ovrrd_val : 1; /**< [  7:  7](R/W) Clock tree powerdown override value.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_counter_resetn : 1;/**< [  8:  8](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_lock_override : 1;/**< [  9:  9](R/W) Not used.
                                                                 For diagnostic use only. */
        uint64_t reserved_10_13        : 4;
        uint64_t pll_div_ovrrd_en      : 1;  /**< [ 14: 14](R/W) Override global power state machine and mac_pcs_pll_div control signal.
                                                                 When asserted, pcs_sds_pll_div is specified from
                                                                 GSER()_LANE()_PCS_PLL_CTLIFC_0[PLL_DIV_OVRRD_VAL],
                                                                 global power state machine and mac_pcs_pll_div control signals are ignored.
                                                                 For diagnostic use only. */
        uint64_t reserved_15_63        : 49;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_pll_cfg_2_s cn; */
};
typedef union cavm_gserx_glbl_pll_cfg_2 cavm_gserx_glbl_pll_cfg_2_t;

static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_2(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_2(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460010ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460010ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460010ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_PLL_CFG_2", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_PLL_CFG_2(a) cavm_gserx_glbl_pll_cfg_2_t
#define bustype_CAVM_GSERX_GLBL_PLL_CFG_2(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_PLL_CFG_2(a) "GSERX_GLBL_PLL_CFG_2"
#define device_bar_CAVM_GSERX_GLBL_PLL_CFG_2(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_PLL_CFG_2(a) (a)
#define arguments_CAVM_GSERX_GLBL_PLL_CFG_2(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_pll_cfg_3
 *
 * GSER Global PLL Configuration 3 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_pll_cfg_3
{
    uint64_t u;
    struct cavm_gserx_glbl_pll_cfg_3_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_10_63        : 54;
        uint64_t pcs_sds_pll_vco_amp   : 2;  /**< [  9:  8](R/W) Adjusts the VCO amplitude control current.
                                                                 For diagnostic use only.
                                                                 0x0 = Add 25 uA.
                                                                 0x1 = OFF (default).
                                                                 0x2 = Sink 25 uA.
                                                                 0x3 = Sink 50 uA. */
        uint64_t pll_bypass_uq         : 1;  /**< [  7:  7](R/W) PLL bypass enable. When asserted, multiplexes in the feedback divider clock.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_ovrrd_en : 1; /**< [  6:  6](R/W) Override enable for selecting current for Vctrl in open loop operation.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_ovrrd_val : 2;/**< [  5:  4](R/W) Override value for selecting current for Vctrl in open loop operation.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_lcvco_val : 2;/**< [  3:  2](R/W) Selects current for Vctrl in open loop operation for LC-tank VCO.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_rovco_val : 2;/**< [  1:  0](R/W) Selects current for Vctrl in open loop operation for ring oscillator VCO.
                                                                 For diagnostic use only. */
#else /* Word 0 - Little Endian */
        uint64_t pll_vctrl_sel_rovco_val : 2;/**< [  1:  0](R/W) Selects current for Vctrl in open loop operation for ring oscillator VCO.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_lcvco_val : 2;/**< [  3:  2](R/W) Selects current for Vctrl in open loop operation for LC-tank VCO.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_ovrrd_val : 2;/**< [  5:  4](R/W) Override value for selecting current for Vctrl in open loop operation.
                                                                 For diagnostic use only. */
        uint64_t pll_vctrl_sel_ovrrd_en : 1; /**< [  6:  6](R/W) Override enable for selecting current for Vctrl in open loop operation.
                                                                 For diagnostic use only. */
        uint64_t pll_bypass_uq         : 1;  /**< [  7:  7](R/W) PLL bypass enable. When asserted, multiplexes in the feedback divider clock.
                                                                 For diagnostic use only. */
        uint64_t pcs_sds_pll_vco_amp   : 2;  /**< [  9:  8](R/W) Adjusts the VCO amplitude control current.
                                                                 For diagnostic use only.
                                                                 0x0 = Add 25 uA.
                                                                 0x1 = OFF (default).
                                                                 0x2 = Sink 25 uA.
                                                                 0x3 = Sink 50 uA. */
        uint64_t reserved_10_63        : 54;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_pll_cfg_3_s cn; */
};
typedef union cavm_gserx_glbl_pll_cfg_3 cavm_gserx_glbl_pll_cfg_3_t;

static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_3(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_PLL_CFG_3(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460018ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460018ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460018ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_PLL_CFG_3", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_PLL_CFG_3(a) cavm_gserx_glbl_pll_cfg_3_t
#define bustype_CAVM_GSERX_GLBL_PLL_CFG_3(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_PLL_CFG_3(a) "GSERX_GLBL_PLL_CFG_3"
#define device_bar_CAVM_GSERX_GLBL_PLL_CFG_3(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_PLL_CFG_3(a) (a)
#define arguments_CAVM_GSERX_GLBL_PLL_CFG_3(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_pll_monitor
 *
 * GSER Monitor for SerDes Global to Raw PCS Global interface Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_pll_monitor
{
    uint64_t u;
    struct cavm_gserx_glbl_pll_monitor_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_14_63        : 50;
        uint64_t sds_pcs_glbl_status   : 6;  /**< [ 13:  8](RO/H) Spare reserved for future use. Read data should be ignored. */
        uint64_t sds_pcs_pll_lock      : 1;  /**< [  7:  7](RO/H) Status signal from global indicates that PLL is locked. Not a true "lock" signal.
                                                                 Used to debug/test the PLL. */
        uint64_t sds_pcs_clock_ready   : 1;  /**< [  6:  6](RO/H) Clock status signal, can be overridden with (I_PLL_CTRL_EN == 1).
                                                                 0 = Clock not ready.
                                                                 1 = Clock ready. */
        uint64_t sds_pcs_pll_calstates : 5;  /**< [  5:  1](RO/H) PLL calibration code. */
        uint64_t sds_pcs_pll_caldone   : 1;  /**< [  0:  0](RO/H) PLL calibration done signal. */
#else /* Word 0 - Little Endian */
        uint64_t sds_pcs_pll_caldone   : 1;  /**< [  0:  0](RO/H) PLL calibration done signal. */
        uint64_t sds_pcs_pll_calstates : 5;  /**< [  5:  1](RO/H) PLL calibration code. */
        uint64_t sds_pcs_clock_ready   : 1;  /**< [  6:  6](RO/H) Clock status signal, can be overridden with (I_PLL_CTRL_EN == 1).
                                                                 0 = Clock not ready.
                                                                 1 = Clock ready. */
        uint64_t sds_pcs_pll_lock      : 1;  /**< [  7:  7](RO/H) Status signal from global indicates that PLL is locked. Not a true "lock" signal.
                                                                 Used to debug/test the PLL. */
        uint64_t sds_pcs_glbl_status   : 6;  /**< [ 13:  8](RO/H) Spare reserved for future use. Read data should be ignored. */
        uint64_t reserved_14_63        : 50;
#endif /* Word 0 - End */
    } s;
    struct cavm_gserx_glbl_pll_monitor_cn
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t reserved_14_15        : 2;
        uint64_t sds_pcs_glbl_status   : 6;  /**< [ 13:  8](RO/H) Spare reserved for future use. Read data should be ignored. */
        uint64_t sds_pcs_pll_lock      : 1;  /**< [  7:  7](RO/H) Status signal from global indicates that PLL is locked. Not a true "lock" signal.
                                                                 Used to debug/test the PLL. */
        uint64_t sds_pcs_clock_ready   : 1;  /**< [  6:  6](RO/H) Clock status signal, can be overridden with (I_PLL_CTRL_EN == 1).
                                                                 0 = Clock not ready.
                                                                 1 = Clock ready. */
        uint64_t sds_pcs_pll_calstates : 5;  /**< [  5:  1](RO/H) PLL calibration code. */
        uint64_t sds_pcs_pll_caldone   : 1;  /**< [  0:  0](RO/H) PLL calibration done signal. */
#else /* Word 0 - Little Endian */
        uint64_t sds_pcs_pll_caldone   : 1;  /**< [  0:  0](RO/H) PLL calibration done signal. */
        uint64_t sds_pcs_pll_calstates : 5;  /**< [  5:  1](RO/H) PLL calibration code. */
        uint64_t sds_pcs_clock_ready   : 1;  /**< [  6:  6](RO/H) Clock status signal, can be overridden with (I_PLL_CTRL_EN == 1).
                                                                 0 = Clock not ready.
                                                                 1 = Clock ready. */
        uint64_t sds_pcs_pll_lock      : 1;  /**< [  7:  7](RO/H) Status signal from global indicates that PLL is locked. Not a true "lock" signal.
                                                                 Used to debug/test the PLL. */
        uint64_t sds_pcs_glbl_status   : 6;  /**< [ 13:  8](RO/H) Spare reserved for future use. Read data should be ignored. */
        uint64_t reserved_14_15        : 2;
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } cn;
};
typedef union cavm_gserx_glbl_pll_monitor cavm_gserx_glbl_pll_monitor_t;

static inline uint64_t CAVM_GSERX_GLBL_PLL_MONITOR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_PLL_MONITOR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460100ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460100ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460100ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_PLL_MONITOR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_PLL_MONITOR(a) cavm_gserx_glbl_pll_monitor_t
#define bustype_CAVM_GSERX_GLBL_PLL_MONITOR(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_PLL_MONITOR(a) "GSERX_GLBL_PLL_MONITOR"
#define device_bar_CAVM_GSERX_GLBL_PLL_MONITOR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_PLL_MONITOR(a) (a)
#define arguments_CAVM_GSERX_GLBL_PLL_MONITOR(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_tad
 *
 * GSER Global Test Analog and Digital Monitor Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_tad
{
    uint64_t u;
    struct cavm_gserx_glbl_tad_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_9_63         : 55;
        uint64_t pcs_sds_tad_8_5       : 4;  /**< [  8:  5](R/W) AMON specific mode selection.
                                                                 Set GSER()_GLBL_TM_ADMON[AMON_ON].
                                                                 Decodes 0x0 - 0x4 require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 Decodes 0x5 - 0x5 do not require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 In both cases, the resulting signals can be observed on the AMON pin.

                                                                 0x0 = TX txdrv DAC 100ua sink current monitor.
                                                                 0x1 = TX vcnt precision dcc.
                                                                 0x2 = RX sdll topregout.
                                                                 0x3 = RX ldll vctrl_i.
                                                                 0x4 = RX RX term VCM voltage.
                                                                 0x5 = Global bandgap voltage.
                                                                 0x6 = Global CTAT voltage.
                                                                 0x7 = Global internal 100ua reference current.
                                                                 0x8 = Global external 100ua reference current.
                                                                 0x9 = Global Rterm calibration reference voltage.
                                                                 0xA = Global Rterm calibration comparator voltage.
                                                                 0xB = Global force VCNT through DAC.
                                                                 0xC = Global VDD voltage.
                                                                 0xD = Global VDDCLK voltage.
                                                                 0xE = Global PLL regulate VCO supply.
                                                                 0xF = Global VCTRL for VCO varactor control. */
        uint64_t pcs_sds_tad_4_0       : 5;  /**< [  4:  0](R/W) DMON specific mode selection.
                                                                 Set GSER()_GLBL_TM_ADMON[DMON_ON].
                                                                 Decodes 0x0 - 0xe require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 Decodes 0xf - 0x1f do not require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 In both cases, the resulting signals can be observed on the DMON pin.

                                                                 0x00 = DFE Data Q.
                                                                 0x01 = DFE Edge I.
                                                                 0x02 = DFE CK Q.
                                                                 0x03 = DFE CK I.
                                                                 0x04 = DLL use GSER()_SLICE()_RX_SDLL_CTRL.PCS_SDS_RX_SDLL_SWSEL to select signal
                                                                 in the slice DLL.
                                                                 0x05-0x7 = Reserved.
                                                                 0x08 = RX ld_rx[0].
                                                                 0x09 = RX rx_clk.
                                                                 0x0A = RX q_error_stg.
                                                                 0x0B = RX q_data_stg.
                                                                 0x0C-0x0E = Reserved.
                                                                 0x0F = Special case to observe supply in global. Sds_vdda and a internal regulated supply
                                                                 can be observed on DMON and DMONB
                                                                 respectively.  sds_vss can be observed on AMON. GSER()_GLBL_TM_ADMON[AMON_ON]
                                                                 must not be set.
                                                                 0x10 = PLL_CLK 0 degree.
                                                                 0x11 = Sds_tst_fb_clk.
                                                                 0x12 = Buffered refclk.
                                                                 0x13 = Div 8 of core clock (core_clk_out).
                                                                 0x14-0x1F: Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t pcs_sds_tad_4_0       : 5;  /**< [  4:  0](R/W) DMON specific mode selection.
                                                                 Set GSER()_GLBL_TM_ADMON[DMON_ON].
                                                                 Decodes 0x0 - 0xe require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 Decodes 0xf - 0x1f do not require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 In both cases, the resulting signals can be observed on the DMON pin.

                                                                 0x00 = DFE Data Q.
                                                                 0x01 = DFE Edge I.
                                                                 0x02 = DFE CK Q.
                                                                 0x03 = DFE CK I.
                                                                 0x04 = DLL use GSER()_SLICE()_RX_SDLL_CTRL.PCS_SDS_RX_SDLL_SWSEL to select signal
                                                                 in the slice DLL.
                                                                 0x05-0x7 = Reserved.
                                                                 0x08 = RX ld_rx[0].
                                                                 0x09 = RX rx_clk.
                                                                 0x0A = RX q_error_stg.
                                                                 0x0B = RX q_data_stg.
                                                                 0x0C-0x0E = Reserved.
                                                                 0x0F = Special case to observe supply in global. Sds_vdda and a internal regulated supply
                                                                 can be observed on DMON and DMONB
                                                                 respectively.  sds_vss can be observed on AMON. GSER()_GLBL_TM_ADMON[AMON_ON]
                                                                 must not be set.
                                                                 0x10 = PLL_CLK 0 degree.
                                                                 0x11 = Sds_tst_fb_clk.
                                                                 0x12 = Buffered refclk.
                                                                 0x13 = Div 8 of core clock (core_clk_out).
                                                                 0x14-0x1F: Reserved. */
        uint64_t pcs_sds_tad_8_5       : 4;  /**< [  8:  5](R/W) AMON specific mode selection.
                                                                 Set GSER()_GLBL_TM_ADMON[AMON_ON].
                                                                 Decodes 0x0 - 0x4 require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 Decodes 0x5 - 0x5 do not require GSER()_GLBL_TM_ADMON[LSEL] set.
                                                                 In both cases, the resulting signals can be observed on the AMON pin.

                                                                 0x0 = TX txdrv DAC 100ua sink current monitor.
                                                                 0x1 = TX vcnt precision dcc.
                                                                 0x2 = RX sdll topregout.
                                                                 0x3 = RX ldll vctrl_i.
                                                                 0x4 = RX RX term VCM voltage.
                                                                 0x5 = Global bandgap voltage.
                                                                 0x6 = Global CTAT voltage.
                                                                 0x7 = Global internal 100ua reference current.
                                                                 0x8 = Global external 100ua reference current.
                                                                 0x9 = Global Rterm calibration reference voltage.
                                                                 0xA = Global Rterm calibration comparator voltage.
                                                                 0xB = Global force VCNT through DAC.
                                                                 0xC = Global VDD voltage.
                                                                 0xD = Global VDDCLK voltage.
                                                                 0xE = Global PLL regulate VCO supply.
                                                                 0xF = Global VCTRL for VCO varactor control. */
        uint64_t reserved_9_63         : 55;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_tad_s cn; */
};
typedef union cavm_gserx_glbl_tad cavm_gserx_glbl_tad_t;

static inline uint64_t CAVM_GSERX_GLBL_TAD(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_TAD(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460400ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460400ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460400ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_TAD", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_TAD(a) cavm_gserx_glbl_tad_t
#define bustype_CAVM_GSERX_GLBL_TAD(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_TAD(a) "GSERX_GLBL_TAD"
#define device_bar_CAVM_GSERX_GLBL_TAD(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_TAD(a) (a)
#define arguments_CAVM_GSERX_GLBL_TAD(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_glbl_tm_admon
 *
 * GSER Global Test Mode Analog/Digital Monitor Enable Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_glbl_tm_admon
{
    uint64_t u;
    struct cavm_gserx_glbl_tm_admon_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_8_63         : 56;
        uint64_t amon_on               : 1;  /**< [  7:  7](R/W) When set, AMON test mode is enabled; see GSER()_GLBL_TAD. */
        uint64_t dmon_on               : 1;  /**< [  6:  6](R/W) When set, DMON test mode is enabled; see GSER()_GLBL_TAD. */
        uint64_t reserved_3_5          : 3;
        uint64_t lsel                  : 3;  /**< [  2:  0](R/W) Three bits to select 1 out of 4 lanes for AMON/DMON test.
                                                                 0x0 = Selects lane 0.
                                                                 0x1 = Selects lane 1.
                                                                 0x2 = Selects lane 2.
                                                                 0x3 = Selects lane 3.
                                                                 0x4-0x7 = Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t lsel                  : 3;  /**< [  2:  0](R/W) Three bits to select 1 out of 4 lanes for AMON/DMON test.
                                                                 0x0 = Selects lane 0.
                                                                 0x1 = Selects lane 1.
                                                                 0x2 = Selects lane 2.
                                                                 0x3 = Selects lane 3.
                                                                 0x4-0x7 = Reserved. */
        uint64_t reserved_3_5          : 3;
        uint64_t dmon_on               : 1;  /**< [  6:  6](R/W) When set, DMON test mode is enabled; see GSER()_GLBL_TAD. */
        uint64_t amon_on               : 1;  /**< [  7:  7](R/W) When set, AMON test mode is enabled; see GSER()_GLBL_TAD. */
        uint64_t reserved_8_63         : 56;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_glbl_tm_admon_s cn; */
};
typedef union cavm_gserx_glbl_tm_admon cavm_gserx_glbl_tm_admon_t;

static inline uint64_t CAVM_GSERX_GLBL_TM_ADMON(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_GLBL_TM_ADMON(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090460408ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090460408ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090460408ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_GLBL_TM_ADMON", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_GLBL_TM_ADMON(a) cavm_gserx_glbl_tm_admon_t
#define bustype_CAVM_GSERX_GLBL_TM_ADMON(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_GLBL_TM_ADMON(a) "GSERX_GLBL_TM_ADMON"
#define device_bar_CAVM_GSERX_GLBL_TM_ADMON(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_GLBL_TM_ADMON(a) (a)
#define arguments_CAVM_GSERX_GLBL_TM_ADMON(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_iddq_mode
 *
 * GSER IDDQ Mode Register
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_iddq_mode
{
    uint64_t u;
    struct cavm_gserx_iddq_mode_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t phy_iddq_mode         : 1;  /**< [  0:  0](R/W) When set, power downs all circuitry in PHY for IDDQ testing */
#else /* Word 0 - Little Endian */
        uint64_t phy_iddq_mode         : 1;  /**< [  0:  0](R/W) When set, power downs all circuitry in PHY for IDDQ testing */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_iddq_mode_s cn; */
};
typedef union cavm_gserx_iddq_mode cavm_gserx_iddq_mode_t;

static inline uint64_t CAVM_GSERX_IDDQ_MODE(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_IDDQ_MODE(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090000018ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090000018ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090000018ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_IDDQ_MODE", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_IDDQ_MODE(a) cavm_gserx_iddq_mode_t
#define bustype_CAVM_GSERX_IDDQ_MODE(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_IDDQ_MODE(a) "GSERX_IDDQ_MODE"
#define device_bar_CAVM_GSERX_IDDQ_MODE(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_IDDQ_MODE(a) (a)
#define arguments_CAVM_GSERX_IDDQ_MODE(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_lane#_lbert_cfg
 *
 * GSER Lane LBERT Configuration Registers
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_lanex_lbert_cfg
{
    uint64_t u;
    struct cavm_gserx_lanex_lbert_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t lbert_pg_err_insert   : 1;  /**< [ 15: 15](WO/H) Insert one bit error into the LSB of the LBERT generated
                                                                 stream.  A single write to this bit inserts a single bit
                                                                 error. */
        uint64_t lbert_pm_sync_start   : 1;  /**< [ 14: 14](WO/H) Synchronize the pattern matcher LFSR with the incoming
                                                                 data.  Writing this bit resets the error counter and
                                                                 starts a synchronization of the PM.  There is no need
                                                                 to write this bit back to a zero to run normally. */
        uint64_t lbert_pg_en           : 1;  /**< [ 13: 13](R/W) Enable the LBERT pattern generator. */
        uint64_t lbert_pg_width        : 2;  /**< [ 12: 11](R/W) LBERT pattern generator data width:
                                                                 0x0 = 8-bit data.
                                                                 0x1 = 10-bit data.
                                                                 0x2 = 16-bit data.
                                                                 0x3 = 20-bit data. */
        uint64_t lbert_pg_mode         : 4;  /**< [ 10:  7](R/W) LBERT pattern generator mode; when changing modes,
                                                                 must be disabled first:
                                                                 0x0 = Disabled.
                                                                 0x1 = lfsr31 = X^31 + X^28 + 1.
                                                                 0x2 = lfsr23 = X^23 + X^18 + 1.
                                                                 0x3 = lfsr23 = X^23 + X^21 + X^16 + X^8 + X^5 + X^2 + 1.
                                                                 0x4 = lfsr16 = X^16 + X^5 + X^4 + X^3 + 1.
                                                                 0x5 = lfsr15 = X^15 + X^14 + 1.
                                                                 0x6 = lfsr11 = X^11 + X^9 + 1.
                                                                 0x7 = lfsr7  = X^7 + X^6 + 1.
                                                                 0x8 = Fixed word (PAT0).
                                                                 0x9 = DC-balanced word (PAT0, ~PAT0).
                                                                 0xA = Fixed Pattern (000, PAT0, 3ff, ~PAT0).
                                                                 0xB-F = Reserved. */
        uint64_t lbert_pm_en           : 1;  /**< [  6:  6](R/W) Enable LBERT pattern matcher. */
        uint64_t lbert_pm_width        : 2;  /**< [  5:  4](R/W) LBERT pattern matcher data width.
                                                                 0x0 = 8-bit data.
                                                                 0x1 = 10-bit data.
                                                                 0x2 = 16-bit data.
                                                                 0x3 = 20-bit data. */
        uint64_t lbert_pm_mode         : 4;  /**< [  3:  0](R/W) LBERT pattern matcher mode; when changing modes,
                                                                 must be disabled first:
                                                                 0x0 = Disabled.
                                                                 0x1 = lfsr31 = X^31 + X^28 + 1.
                                                                 0x2 = lfsr23 = X^23 + X^18 + 1.
                                                                 0x3 = lfsr23 = X^23 + X^21 + X^16 + X^8 + X^5 + X^2 + 1.
                                                                 0x4 = lfsr16 = X^16 + X^5 + X^4 + X^3 + 1.
                                                                 0x5 = lfsr15 = X^15 + X^14 + 1.
                                                                 0x6 = lfsr11 = X^11 + X^9 + 1.
                                                                 0x7 = lfsr7  = X^7 + X^6 + 1.
                                                                 0x8 = Fixed word (PAT0).
                                                                 0x9 = DC-balanced word (PAT0, ~PAT0).
                                                                 0xA = Fixed Pattern: (000, PAT0, 3ff, ~PAT0).
                                                                 0xB-F = Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t lbert_pm_mode         : 4;  /**< [  3:  0](R/W) LBERT pattern matcher mode; when changing modes,
                                                                 must be disabled first:
                                                                 0x0 = Disabled.
                                                                 0x1 = lfsr31 = X^31 + X^28 + 1.
                                                                 0x2 = lfsr23 = X^23 + X^18 + 1.
                                                                 0x3 = lfsr23 = X^23 + X^21 + X^16 + X^8 + X^5 + X^2 + 1.
                                                                 0x4 = lfsr16 = X^16 + X^5 + X^4 + X^3 + 1.
                                                                 0x5 = lfsr15 = X^15 + X^14 + 1.
                                                                 0x6 = lfsr11 = X^11 + X^9 + 1.
                                                                 0x7 = lfsr7  = X^7 + X^6 + 1.
                                                                 0x8 = Fixed word (PAT0).
                                                                 0x9 = DC-balanced word (PAT0, ~PAT0).
                                                                 0xA = Fixed Pattern: (000, PAT0, 3ff, ~PAT0).
                                                                 0xB-F = Reserved. */
        uint64_t lbert_pm_width        : 2;  /**< [  5:  4](R/W) LBERT pattern matcher data width.
                                                                 0x0 = 8-bit data.
                                                                 0x1 = 10-bit data.
                                                                 0x2 = 16-bit data.
                                                                 0x3 = 20-bit data. */
        uint64_t lbert_pm_en           : 1;  /**< [  6:  6](R/W) Enable LBERT pattern matcher. */
        uint64_t lbert_pg_mode         : 4;  /**< [ 10:  7](R/W) LBERT pattern generator mode; when changing modes,
                                                                 must be disabled first:
                                                                 0x0 = Disabled.
                                                                 0x1 = lfsr31 = X^31 + X^28 + 1.
                                                                 0x2 = lfsr23 = X^23 + X^18 + 1.
                                                                 0x3 = lfsr23 = X^23 + X^21 + X^16 + X^8 + X^5 + X^2 + 1.
                                                                 0x4 = lfsr16 = X^16 + X^5 + X^4 + X^3 + 1.
                                                                 0x5 = lfsr15 = X^15 + X^14 + 1.
                                                                 0x6 = lfsr11 = X^11 + X^9 + 1.
                                                                 0x7 = lfsr7  = X^7 + X^6 + 1.
                                                                 0x8 = Fixed word (PAT0).
                                                                 0x9 = DC-balanced word (PAT0, ~PAT0).
                                                                 0xA = Fixed Pattern (000, PAT0, 3ff, ~PAT0).
                                                                 0xB-F = Reserved. */
        uint64_t lbert_pg_width        : 2;  /**< [ 12: 11](R/W) LBERT pattern generator data width:
                                                                 0x0 = 8-bit data.
                                                                 0x1 = 10-bit data.
                                                                 0x2 = 16-bit data.
                                                                 0x3 = 20-bit data. */
        uint64_t lbert_pg_en           : 1;  /**< [ 13: 13](R/W) Enable the LBERT pattern generator. */
        uint64_t lbert_pm_sync_start   : 1;  /**< [ 14: 14](WO/H) Synchronize the pattern matcher LFSR with the incoming
                                                                 data.  Writing this bit resets the error counter and
                                                                 starts a synchronization of the PM.  There is no need
                                                                 to write this bit back to a zero to run normally. */
        uint64_t lbert_pg_err_insert   : 1;  /**< [ 15: 15](WO/H) Insert one bit error into the LSB of the LBERT generated
                                                                 stream.  A single write to this bit inserts a single bit
                                                                 error. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_lanex_lbert_cfg_s cn; */
};
typedef union cavm_gserx_lanex_lbert_cfg cavm_gserx_lanex_lbert_cfg_t;

static inline uint64_t CAVM_GSERX_LANEX_LBERT_CFG(unsigned long a, unsigned long b) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_LANEX_LBERT_CFG(unsigned long a, unsigned long b)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && ((a<=3) && (b<=1)))
        return 0x87e0904c0020ll + 0x1000000ll * ((a) & 0x3) + 0x100000ll * ((b) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && ((a<=6) && (b<=3)))
        return 0x87e0904c0020ll + 0x1000000ll * ((a) & 0x7) + 0x100000ll * ((b) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && ((a<=13) && (b<=3)))
        return 0x87e0904c0020ll + 0x1000000ll * ((a) & 0xf) + 0x100000ll * ((b) & 0x3);
    __cavm_csr_fatal("GSERX_LANEX_LBERT_CFG", 2, a, b, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_LANEX_LBERT_CFG(a,b) cavm_gserx_lanex_lbert_cfg_t
#define bustype_CAVM_GSERX_LANEX_LBERT_CFG(a,b) CSR_TYPE_RSL
#define basename_CAVM_GSERX_LANEX_LBERT_CFG(a,b) "GSERX_LANEX_LBERT_CFG"
#define device_bar_CAVM_GSERX_LANEX_LBERT_CFG(a,b) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_LANEX_LBERT_CFG(a,b) (a)
#define arguments_CAVM_GSERX_LANEX_LBERT_CFG(a,b) (a),(b),-1,-1

/**
 * Register (RSL) gser#_lane#_lbert_pat_cfg
 *
 * GSER Lane LBERT Pattern Configuration Registers
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_lanex_lbert_pat_cfg
{
    uint64_t u;
    struct cavm_gserx_lanex_lbert_pat_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_10_63        : 54;
        uint64_t lbert_pg_pat          : 10; /**< [  9:  0](R/W) Programmable 10-bit pattern to be used in the LBERT pattern mode;
                                                                 applies when GSER()_LANE()_LBERT_CFG[LBERT_PG_MODE]
                                                                 is equal to 8, 9, or 10. */
#else /* Word 0 - Little Endian */
        uint64_t lbert_pg_pat          : 10; /**< [  9:  0](R/W) Programmable 10-bit pattern to be used in the LBERT pattern mode;
                                                                 applies when GSER()_LANE()_LBERT_CFG[LBERT_PG_MODE]
                                                                 is equal to 8, 9, or 10. */
        uint64_t reserved_10_63        : 54;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_lanex_lbert_pat_cfg_s cn; */
};
typedef union cavm_gserx_lanex_lbert_pat_cfg cavm_gserx_lanex_lbert_pat_cfg_t;

static inline uint64_t CAVM_GSERX_LANEX_LBERT_PAT_CFG(unsigned long a, unsigned long b) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_LANEX_LBERT_PAT_CFG(unsigned long a, unsigned long b)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && ((a<=3) && (b<=1)))
        return 0x87e0904c0018ll + 0x1000000ll * ((a) & 0x3) + 0x100000ll * ((b) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && ((a<=6) && (b<=3)))
        return 0x87e0904c0018ll + 0x1000000ll * ((a) & 0x7) + 0x100000ll * ((b) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && ((a<=13) && (b<=3)))
        return 0x87e0904c0018ll + 0x1000000ll * ((a) & 0xf) + 0x100000ll * ((b) & 0x3);
    __cavm_csr_fatal("GSERX_LANEX_LBERT_PAT_CFG", 2, a, b, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) cavm_gserx_lanex_lbert_pat_cfg_t
#define bustype_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) CSR_TYPE_RSL
#define basename_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) "GSERX_LANEX_LBERT_PAT_CFG"
#define device_bar_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) (a)
#define arguments_CAVM_GSERX_LANEX_LBERT_PAT_CFG(a,b) (a),(b),-1,-1

/**
 * Register (RSL) gser#_lane#_misc_cfg_0
 *
 * GSER Lane Miscellaneous Configuration 0 Register
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_lanex_misc_cfg_0
{
    uint64_t u;
    struct cavm_gserx_lanex_misc_cfg_0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_16_63        : 48;
        uint64_t use_pma_polarity      : 1;  /**< [ 15: 15](R/W) If set, the PMA control is used to define the polarity.
                                                                 If not set, GSER()_LANE()_RX_CFG_0[CFG_RX_POL_INVERT]
                                                                 is used. */
        uint64_t cfg_pcs_loopback      : 1;  /**< [ 14: 14](R/W) Assert for parallel loopback raw PCS TX to Raw PCS RX. */
        uint64_t pcs_tx_mode_ovrrd_en  : 1;  /**< [ 13: 13](R/W) Override enable for raw PCS TX data width. */
        uint64_t pcs_rx_mode_ovrrd_en  : 1;  /**< [ 12: 12](R/W) Override enable for raw PCS RX data width. */
        uint64_t cfg_eie_det_cnt       : 4;  /**< [ 11:  8](R/W) EIE detect state machine required number of consecutive
                                                                 PHY EIE status assertions to determine EIE and assert Raw
                                                                 PCS output pcs_mac_rx_eie_det_sts. */
        uint64_t eie_det_stl_on_time   : 3;  /**< [  7:  5](R/W) EIE detect state machine "on" delay prior to sampling
                                                                 PHY EIE status.  Software needs to set this field to 0x4 if
                                                                 in SATA mode (GSER()_CFG[SATA] is set). */
        uint64_t eie_det_stl_off_time  : 3;  /**< [  4:  2](R/W) EIE detect state machine "off" delay prior to sampling
                                                                 PHY EIE status. */
        uint64_t tx_bit_order          : 1;  /**< [  1:  1](R/W) Specify transmit bit order.
                                                                 0 = Maintain bit order of parallel data to SerDes TX.
                                                                 1 = Reverse bit order of parallel data to SerDes TX. */
        uint64_t rx_bit_order          : 1;  /**< [  0:  0](R/W) Specify receive bit order:
                                                                 0 = Maintain bit order of parallel data to SerDes RX.
                                                                 1 = Reverse bit order of parallel data to SerDes RX. */
#else /* Word 0 - Little Endian */
        uint64_t rx_bit_order          : 1;  /**< [  0:  0](R/W) Specify receive bit order:
                                                                 0 = Maintain bit order of parallel data to SerDes RX.
                                                                 1 = Reverse bit order of parallel data to SerDes RX. */
        uint64_t tx_bit_order          : 1;  /**< [  1:  1](R/W) Specify transmit bit order.
                                                                 0 = Maintain bit order of parallel data to SerDes TX.
                                                                 1 = Reverse bit order of parallel data to SerDes TX. */
        uint64_t eie_det_stl_off_time  : 3;  /**< [  4:  2](R/W) EIE detect state machine "off" delay prior to sampling
                                                                 PHY EIE status. */
        uint64_t eie_det_stl_on_time   : 3;  /**< [  7:  5](R/W) EIE detect state machine "on" delay prior to sampling
                                                                 PHY EIE status.  Software needs to set this field to 0x4 if
                                                                 in SATA mode (GSER()_CFG[SATA] is set). */
        uint64_t cfg_eie_det_cnt       : 4;  /**< [ 11:  8](R/W) EIE detect state machine required number of consecutive
                                                                 PHY EIE status assertions to determine EIE and assert Raw
                                                                 PCS output pcs_mac_rx_eie_det_sts. */
        uint64_t pcs_rx_mode_ovrrd_en  : 1;  /**< [ 12: 12](R/W) Override enable for raw PCS RX data width. */
        uint64_t pcs_tx_mode_ovrrd_en  : 1;  /**< [ 13: 13](R/W) Override enable for raw PCS TX data width. */
        uint64_t cfg_pcs_loopback      : 1;  /**< [ 14: 14](R/W) Assert for parallel loopback raw PCS TX to Raw PCS RX. */
        uint64_t use_pma_polarity      : 1;  /**< [ 15: 15](R/W) If set, the PMA control is used to define the polarity.
                                                                 If not set, GSER()_LANE()_RX_CFG_0[CFG_RX_POL_INVERT]
                                                                 is used. */
        uint64_t reserved_16_63        : 48;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_lanex_misc_cfg_0_s cn; */
};
typedef union cavm_gserx_lanex_misc_cfg_0 cavm_gserx_lanex_misc_cfg_0_t;

static inline uint64_t CAVM_GSERX_LANEX_MISC_CFG_0(unsigned long a, unsigned long b) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_LANEX_MISC_CFG_0(unsigned long a, unsigned long b)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && ((a<=3) && (b<=1)))
        return 0x87e0904c0000ll + 0x1000000ll * ((a) & 0x3) + 0x100000ll * ((b) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && ((a<=6) && (b<=3)))
        return 0x87e0904c0000ll + 0x1000000ll * ((a) & 0x7) + 0x100000ll * ((b) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && ((a<=13) && (b<=3)))
        return 0x87e0904c0000ll + 0x1000000ll * ((a) & 0xf) + 0x100000ll * ((b) & 0x3);
    __cavm_csr_fatal("GSERX_LANEX_MISC_CFG_0", 2, a, b, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) cavm_gserx_lanex_misc_cfg_0_t
#define bustype_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) CSR_TYPE_RSL
#define basename_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) "GSERX_LANEX_MISC_CFG_0"
#define device_bar_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) (a)
#define arguments_CAVM_GSERX_LANEX_MISC_CFG_0(a,b) (a),(b),-1,-1

/**
 * Register (RSL) gser#_rx_eie_deten
 *
 * GSER RX Electrical Idle Detect Enable Register
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cavm_gserx_rx_eie_deten
{
    uint64_t u;
    struct cavm_gserx_rx_eie_deten_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode, these bits enable per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.  Reserved.
                                                                 <2>: Lane 2.  Reserved.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode, these bits enable per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.  Reserved.
                                                                 <2>: Lane 2.  Reserved.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_rx_eie_deten_s cn81xx; */
    struct cavm_gserx_rx_eie_deten_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode (including all CCPI links), these bits enable
                                                                 per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.
                                                                 <2>: Lane 2.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode (including all CCPI links), these bits enable
                                                                 per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.
                                                                 <2>: Lane 2.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_gserx_rx_eie_deten_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode, these bits enable per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <2>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eiede                 : 4;  /**< [  3:  0](R/W) For links that are not in PCIE or SATA mode, these bits enable per lane
                                                                 electrical idle exit (EIE) detection. When EIE is detected,
                                                                 GSER()_RX_EIE_DETSTS[EIELTCH] is asserted. [EIEDE] defaults to the enabled state. Once
                                                                 EIE has been detected, [EIEDE] must be disabled, and then enabled again to perform another
                                                                 EIE detection.
                                                                 <3>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <2>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gserx_rx_eie_deten cavm_gserx_rx_eie_deten_t;

static inline uint64_t CAVM_GSERX_RX_EIE_DETEN(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_RX_EIE_DETEN(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090000148ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090000148ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090000148ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_RX_EIE_DETEN", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_RX_EIE_DETEN(a) cavm_gserx_rx_eie_deten_t
#define bustype_CAVM_GSERX_RX_EIE_DETEN(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_RX_EIE_DETEN(a) "GSERX_RX_EIE_DETEN"
#define device_bar_CAVM_GSERX_RX_EIE_DETEN(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_RX_EIE_DETEN(a) (a)
#define arguments_CAVM_GSERX_RX_EIE_DETEN(a) (a),-1,-1,-1

/**
 * Register (RSL) gser#_rx_eie_detsts
 *
 * GSER RX Electrical Idle Detect Status Register
 */
union cavm_gserx_rx_eie_detsts
{
    uint64_t u;
    struct cavm_gserx_rx_eie_detsts_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.  Reserved.
                                                                 <10>: Lane 2.  Reserved.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.  Reserved.
                                                                 <6>: Lane 2.  Reserved.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.  Reserved.
                                                                 <2>: Lane 2.  Reserved.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.  Reserved.
                                                                 <2>: Lane 2.  Reserved.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.  Reserved.
                                                                 <6>: Lane 2.  Reserved.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.  Reserved.
                                                                 <10>: Lane 2.  Reserved.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_gserx_rx_eie_detsts_s cn81xx; */
    struct cavm_gserx_rx_eie_detsts_cn88xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.
                                                                 <10>: Lane 2.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.
                                                                 <6>: Lane 2.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.
                                                                 <2>: Lane 2.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.
                                                                 <2>: Lane 2.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.
                                                                 <6>: Lane 2.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.
                                                                 <10>: Lane 2.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } cn88xx;
    struct cavm_gserx_rx_eie_detsts_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_12_63        : 52;
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <10>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <6>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <2>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
#else /* Word 0 - Little Endian */
        uint64_t eieltch               : 4;  /**< [  3:  0](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. Once an
                                                                 EIE condition has been detected, the per-lane [EIELTCH] will stay set until
                                                                 GSER()_RX_EIE_DETEN[EIEDE] is deasserted. Note that there may be RX bit errors until
                                                                 CDRLOCK
                                                                 is set.
                                                                 <3>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <2>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <1>: Lane 1.
                                                                 <0>: Lane 0. */
        uint64_t eiests                : 4;  /**< [  7:  4](RO/H) When electrical idle exit detection is enabled (GSER()_RX_EIE_DETEN[EIEDE] is
                                                                 asserted), indicates that an electrical idle exit condition (EIE) was detected. For higher
                                                                 data rates, the received data needs to have sufficient low frequency content (for example,
                                                                 idle symbols) for data transitions to be detected and for [EIESTS] to stay set
                                                                 accordingly.
                                                                 Under most conditions, [EIESTS]
                                                                 will stay asserted until GSER()_RX_EIE_DETEN[EIEDE] is deasserted.
                                                                 <7>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <6>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <5>: Lane 1.
                                                                 <4>: Lane 0. */
        uint64_t cdrlock               : 4;  /**< [ 11:  8](RO/H) After an electrical idle exit condition (EIE) has been detected, the CDR needs 10000 UI to
                                                                 lock. During this time, there may be RX bit errors. These bits will set when the CDR is
                                                                 guaranteed to be locked. Note that link training can't start until the lane CDRLOCK is
                                                                 set. Software can use CDRLOCK to determine when to expect error free RX data.
                                                                 <11>: Lane 3.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <10>: Lane 2.  Not supported in GSER4, GSER5, or GSER6.
                                                                 <9>: Lane 1.
                                                                 <8>: Lane 0. */
        uint64_t reserved_12_63        : 52;
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_gserx_rx_eie_detsts cavm_gserx_rx_eie_detsts_t;

static inline uint64_t CAVM_GSERX_RX_EIE_DETSTS(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_GSERX_RX_EIE_DETSTS(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=3))
        return 0x87e090000150ll + 0x1000000ll * ((a) & 0x3);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=6))
        return 0x87e090000150ll + 0x1000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=13))
        return 0x87e090000150ll + 0x1000000ll * ((a) & 0xf);
    __cavm_csr_fatal("GSERX_RX_EIE_DETSTS", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_GSERX_RX_EIE_DETSTS(a) cavm_gserx_rx_eie_detsts_t
#define bustype_CAVM_GSERX_RX_EIE_DETSTS(a) CSR_TYPE_RSL
#define basename_CAVM_GSERX_RX_EIE_DETSTS(a) "GSERX_RX_EIE_DETSTS"
#define device_bar_CAVM_GSERX_RX_EIE_DETSTS(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_GSERX_RX_EIE_DETSTS(a) (a)
#define arguments_CAVM_GSERX_RX_EIE_DETSTS(a) (a),-1,-1,-1

#endif /* __CAVM_CSRS_GSER_H__ */

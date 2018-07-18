#ifndef __CAVM_CSRS_L2C_H__
#define __CAVM_CSRS_L2C_H__
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
 * Cavium L2C.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration l2c_bar_e
 *
 * L2C Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_L2C_BAR_E_L2C_PF_BAR0 (0x87e080800000ll)
#define CAVM_L2C_BAR_E_L2C_PF_BAR0_SIZE 0x800000ull

/**
 * Register (RSL) l2c_asc_region#_attr
 *
 * Level 2 Cache Address Space Control Region Attributes Registers
 */
union cavm_l2c_asc_regionx_attr
{
    uint64_t u;
    struct cavm_l2c_asc_regionx_attr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_2_63         : 62;
        uint64_t s_en                  : 1;  /**< [  1:  1](R/W) Enables secure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region. */
        uint64_t ns_en                 : 1;  /**< [  0:  0](R/W) Enables nonsecure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region.
                                                                 See also DFA_ASC_REGION()_ATTR[NS_EN]. */
#else /* Word 0 - Little Endian */
        uint64_t ns_en                 : 1;  /**< [  0:  0](R/W) Enables nonsecure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region.
                                                                 See also DFA_ASC_REGION()_ATTR[NS_EN]. */
        uint64_t s_en                  : 1;  /**< [  1:  1](R/W) Enables secure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region. */
        uint64_t reserved_2_63         : 62;
#endif /* Word 0 - End */
    } s;
    struct cavm_l2c_asc_regionx_attr_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_2_63         : 62;
        uint64_t s_en                  : 1;  /**< [  1:  1](R/W) Enables secure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region. */
        uint64_t ns_en                 : 1;  /**< [  0:  0](R/W) Enables nonsecure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region.

                                                                 Internal:
                                                                 See also DFA_ASC_REGION()_ATTR[NS_EN]. */
#else /* Word 0 - Little Endian */
        uint64_t ns_en                 : 1;  /**< [  0:  0](R/W) Enables nonsecure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region.

                                                                 Internal:
                                                                 See also DFA_ASC_REGION()_ATTR[NS_EN]. */
        uint64_t s_en                  : 1;  /**< [  1:  1](R/W) Enables secure access to region.
                                                                 Undefined if both [S_EN] and [NS_EN] are set for the same region. */
        uint64_t reserved_2_63         : 62;
#endif /* Word 0 - End */
    } cn81xx;
    /* struct cavm_l2c_asc_regionx_attr_s cn88xx; */
    /* struct cavm_l2c_asc_regionx_attr_cn81xx cn83xx; */
};
typedef union cavm_l2c_asc_regionx_attr cavm_l2c_asc_regionx_attr_t;

static inline uint64_t CAVM_L2C_ASC_REGIONX_ATTR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_L2C_ASC_REGIONX_ATTR(unsigned long a)
{
    if (a<=3)
        return 0x87e080801010ll + 0x40ll * ((a) & 0x3);
    __cavm_csr_fatal("L2C_ASC_REGIONX_ATTR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_L2C_ASC_REGIONX_ATTR(a) cavm_l2c_asc_regionx_attr_t
#define bustype_CAVM_L2C_ASC_REGIONX_ATTR(a) CSR_TYPE_RSL
#define basename_CAVM_L2C_ASC_REGIONX_ATTR(a) "L2C_ASC_REGIONX_ATTR"
#define device_bar_CAVM_L2C_ASC_REGIONX_ATTR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_L2C_ASC_REGIONX_ATTR(a) (a)
#define arguments_CAVM_L2C_ASC_REGIONX_ATTR(a) (a),-1,-1,-1

/**
 * Register (RSL) l2c_asc_region#_end
 *
 * Level 2 Cache Address Space Control Region End Address Registers
 */
union cavm_l2c_asc_regionx_end
{
    uint64_t u;
    struct cavm_l2c_asc_regionx_end_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_40_63        : 24;
        uint64_t addr                  : 20; /**< [ 39: 20](R/W) Node-local physical address <39:20> marking the inclusive end of the corresponding ASC
                                                                 region.
                                                                 Note that the region includes this address.
                                                                 Software must ensure that regions do not overlap.
                                                                 To specify an empty region, clear both the [S_EN] and [NS_EN] fields of
                                                                 the corresponding L2C_ASC_REGION()_ATTR register. */
        uint64_t reserved_0_19         : 20;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_19         : 20;
        uint64_t addr                  : 20; /**< [ 39: 20](R/W) Node-local physical address <39:20> marking the inclusive end of the corresponding ASC
                                                                 region.
                                                                 Note that the region includes this address.
                                                                 Software must ensure that regions do not overlap.
                                                                 To specify an empty region, clear both the [S_EN] and [NS_EN] fields of
                                                                 the corresponding L2C_ASC_REGION()_ATTR register. */
        uint64_t reserved_40_63        : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_l2c_asc_regionx_end_s cn; */
};
typedef union cavm_l2c_asc_regionx_end cavm_l2c_asc_regionx_end_t;

static inline uint64_t CAVM_L2C_ASC_REGIONX_END(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_L2C_ASC_REGIONX_END(unsigned long a)
{
    if (a<=3)
        return 0x87e080801008ll + 0x40ll * ((a) & 0x3);
    __cavm_csr_fatal("L2C_ASC_REGIONX_END", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_L2C_ASC_REGIONX_END(a) cavm_l2c_asc_regionx_end_t
#define bustype_CAVM_L2C_ASC_REGIONX_END(a) CSR_TYPE_RSL
#define basename_CAVM_L2C_ASC_REGIONX_END(a) "L2C_ASC_REGIONX_END"
#define device_bar_CAVM_L2C_ASC_REGIONX_END(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_L2C_ASC_REGIONX_END(a) (a)
#define arguments_CAVM_L2C_ASC_REGIONX_END(a) (a),-1,-1,-1

/**
 * Register (RSL) l2c_asc_region#_start
 *
 * Level 2 Cache Address Space Control Region Start Address Registers
 */
union cavm_l2c_asc_regionx_start
{
    uint64_t u;
    struct cavm_l2c_asc_regionx_start_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_40_63        : 24;
        uint64_t addr                  : 20; /**< [ 39: 20](R/W) Node-local physical address <39:20> marking the start of the corresponding ASC region.
                                                                 Software must ensure that regions do not overlap. */
        uint64_t reserved_0_19         : 20;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_19         : 20;
        uint64_t addr                  : 20; /**< [ 39: 20](R/W) Node-local physical address <39:20> marking the start of the corresponding ASC region.
                                                                 Software must ensure that regions do not overlap. */
        uint64_t reserved_40_63        : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_l2c_asc_regionx_start_s cn; */
};
typedef union cavm_l2c_asc_regionx_start cavm_l2c_asc_regionx_start_t;

static inline uint64_t CAVM_L2C_ASC_REGIONX_START(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_L2C_ASC_REGIONX_START(unsigned long a)
{
    if (a<=3)
        return 0x87e080801000ll + 0x40ll * ((a) & 0x3);
    __cavm_csr_fatal("L2C_ASC_REGIONX_START", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_L2C_ASC_REGIONX_START(a) cavm_l2c_asc_regionx_start_t
#define bustype_CAVM_L2C_ASC_REGIONX_START(a) CSR_TYPE_RSL
#define basename_CAVM_L2C_ASC_REGIONX_START(a) "L2C_ASC_REGIONX_START"
#define device_bar_CAVM_L2C_ASC_REGIONX_START(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_L2C_ASC_REGIONX_START(a) (a)
#define arguments_CAVM_L2C_ASC_REGIONX_START(a) (a),-1,-1,-1

/**
 * Register (RSL) l2c_oci_ctl
 *
 * L2C CCPI Control Register
 */
union cavm_l2c_oci_ctl
{
    uint64_t u;
    struct cavm_l2c_oci_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_31_63        : 33;
        uint64_t ncpend                : 1;  /**< [ 30: 30](RO/H) An indication that a node change is pending.  Hardware sets this bit when
                                                                 OCX_COM_NODE[ID] is changed and clears the bit when the node change has taken
                                                                 effect. */
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](RO) Reserved. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](RO) Reserved. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](RO) Reserved. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Reserved.
                                                                 Internal:
                                                                 This only controls the GSYNC timeout in the L2C_CBCs in non-OCI chips. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](RO) Reserved. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](RO) Reserved. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](RO) Reserved. */
        uint64_t shto                  : 1;  /**< [ 12: 12](RO) Reserved. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](RO) Reserved. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](RO) Reserved. */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](RO) Reserved. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](RO) Reserved. */
        uint64_t iofrcl                : 1;  /**< [  6:  6](RO) Reserved. */
        uint64_t reserved_4_5          : 2;
        uint64_t enaoci                : 4;  /**< [  3:  0](RO) CCPI is not present. Any attempt to enable it will be ignored. */
#else /* Word 0 - Little Endian */
        uint64_t enaoci                : 4;  /**< [  3:  0](RO) CCPI is not present. Any attempt to enable it will be ignored. */
        uint64_t reserved_4_5          : 2;
        uint64_t iofrcl                : 1;  /**< [  6:  6](RO) Reserved. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](RO) Reserved. */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](RO) Reserved. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](RO) Reserved. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](RO) Reserved. */
        uint64_t shto                  : 1;  /**< [ 12: 12](RO) Reserved. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](RO) Reserved. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](RO) Reserved. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](RO) Reserved. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Reserved.
                                                                 Internal:
                                                                 This only controls the GSYNC timeout in the L2C_CBCs in non-OCI chips. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](RO) Reserved. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](RO) Reserved. */
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](RO) Reserved. */
        uint64_t ncpend                : 1;  /**< [ 30: 30](RO/H) An indication that a node change is pending.  Hardware sets this bit when
                                                                 OCX_COM_NODE[ID] is changed and clears the bit when the node change has taken
                                                                 effect. */
        uint64_t reserved_31_63        : 33;
#endif /* Word 0 - End */
    } s;
    struct cavm_l2c_oci_ctl_cn88xxp1
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_31_63        : 33;
        uint64_t reserved_30           : 1;
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](R/W) When set, L2 CAS operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the CAS request to be performed on the home node. For STC ops [LOCK_LOCAL_STC]. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](R/W) When set, L2 STC operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the STC request to be performed on the home node. For CAS ops [LOCK_LOCAL_CAS]. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](R/W) When clear, L2 atomic operations (excluding CAS/STC) core initiated requests to remote
                                                                 addresses which miss at the requester will send the atomic request to be performed on the
                                                                 home node. Default operation will instead  be performed locally on the requesting node.
                                                                 For request initiated by IOB & for STC & CAS ops, see
                                                                 [LOCK_LOCAL_IOB]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS]. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Selects the bit in the counter for the long timeout value (timeout used when [SHTO] is
                                                                 clear). Values supported are between 11 and 29 (for a timeout values between 2^11 and
                                                                 2^29). Actual timeout is between 1x and 2x this interval. For example if [LNGTOLEN] = 28
                                                                 (the reset value), the timeout is between 256M and 512M core clocks. Note: a value of 0
                                                                 disables this timer. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](R/W) Selects the bit in the counter for the short timeout value (timeout used when [SHTO] is
                                                                 set). Values supported are between 9 and 29 (for a timeout values between 2^9 and 2^29).
                                                                 Actual timeout is between 1x and 2x this interval. For example if [SHTOLEN] = 14 (the
                                                                 reset
                                                                 value), the timeout is between 16K and 32K core clocks. Note: a value of 0 disables this
                                                                 timer. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](R/W) When set, any core issues any of an IO load, atomic, acking store, acking IOBADDR, or
                                                                 acking LMTST to a node that doesn't exist (existence defined by the ENAOCI bits), then the
                                                                 hardware sets [SHTO]. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](R/W) When set, if the corresponding CCPI link is down, the hardware sets [SHTO].
                                                                 See OCX_COM_LINK(0..2)_CTL for a description of what events can contribute to the
                                                                 link_down condition. */
        uint64_t shto                  : 1;  /**< [ 12: 12](R/W/H) Use short timeout intervals. When set, core uses SDIDTTO for both DID and commit counter
                                                                 timeouts, rather than DIDTTO/DIDTTO2. Similarly, L2C will use short instead of long
                                                         CAVM_L2C_CBCX_BIST_STATUS        timeout. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](R/W) Describes how aggressive to be when waiting for local invalidates before sending CCPI
                                                                 responses which act like commits at the remote.
                                                                 0x0 = Conservative mode, waits until all local invalidates have been sent by their
                                                                 respective CBCs to the cores.
                                                                 0x1 = Moderate mode, waits until all local invalidates have been sent to their respective
                                                                 CBCs, but not necessarily actually sent to the cores themselves.
                                                                 0x2 = Aggressive mode, does not wait for local invalidates to begin their processing. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](R/W) When set, L2 STC/CAS operations performed at the home will immediately bring the block
                                                                 exclusive into the home. Default operation is to first request the block shared and only
                                                                 invalidate the remote if the compare succeeds. */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](R/W) When set, RLDD is assumed to return a shared response (PSHA). Default operation assumes an
                                                                 exclusive response (PEMD). Note that an incorrect assumption only causes an extra tag
                                                                 write to be done upon receiving the response. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](R/W) When set, L2 atomic operations (excluding CAS/STC) initiated by IOB to remote addresses
                                                                 which miss at the requester are performed locally on the requesting node. When clear the
                                                                 operation instead sends the atomic request to be performed on the home node. For request
                                                                 initiated by core for STC and CAS ops; see
                                                                 [LOCK_LOCAL_PP]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS].
                                                                 Default is set to 1 (local locks). */
        uint64_t iofrcl                : 1;  /**< [  6:  6](R/W) When set, L2C services all I/O read and write operations on the local node, regardless of
                                                                 the value of the node ID bits in the physical address. During normal operation this bit is
                                                                 expected to be 0. Will only transition from 1 to 0, never from 0 to 1. */
        uint64_t reserved_4_5          : 2;
        uint64_t enaoci                : 4;  /**< [  3:  0](R/W) Enable CCPI processing (one bit per node_id). When set, perform CCPI
                                                                 processing. When clear, CCPI memory writes are blocked and CCPI memory reads
                                                                 return unpredictable data. When clear,
                                                                 CCPI I/O requests and MOC references are processed and sent to OCX where they are
                                                                 ultimately discarded. RDDISOCI/WRDISOCI/IORDDISOCI/IOWRDISOCI interrupts occur if and only
                                                                 if the corresponding ENAOCI<node> bit is clear. References to the local node (configured
                                                                 via OCX_COM_NODE[ID]) ignore the value of ENAOCI<node> because no CCPI processing is
                                                                 required. Similarly, all I/O references ignore the value of ENAOCI when
                                                                 L2C_OCI_CTL[IOFRCL] is set. */
#else /* Word 0 - Little Endian */
        uint64_t enaoci                : 4;  /**< [  3:  0](R/W) Enable CCPI processing (one bit per node_id). When set, perform CCPI
                                                                 processing. When clear, CCPI memory writes are blocked and CCPI memory reads
                                                                 return unpredictable data. When clear,
                                                                 CCPI I/O requests and MOC references are processed and sent to OCX where they are
                                                                 ultimately discarded. RDDISOCI/WRDISOCI/IORDDISOCI/IOWRDISOCI interrupts occur if and only
                                                                 if the corresponding ENAOCI<node> bit is clear. References to the local node (configured
                                                                 via OCX_COM_NODE[ID]) ignore the value of ENAOCI<node> because no CCPI processing is
                                                                 required. Similarly, all I/O references ignore the value of ENAOCI when
                                                                 L2C_OCI_CTL[IOFRCL] is set. */
        uint64_t reserved_4_5          : 2;
        uint64_t iofrcl                : 1;  /**< [  6:  6](R/W) When set, L2C services all I/O read and write operations on the local node, regardless of
                                                                 the value of the node ID bits in the physical address. During normal operation this bit is
                                                                 expected to be 0. Will only transition from 1 to 0, never from 0 to 1. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](R/W) When set, L2 atomic operations (excluding CAS/STC) initiated by IOB to remote addresses
                                                                 which miss at the requester are performed locally on the requesting node. When clear the
                                                                 operation instead sends the atomic request to be performed on the home node. For request
                                                                 initiated by core for STC and CAS ops; see
                                                                 [LOCK_LOCAL_PP]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS].
                                                                 Default is set to 1 (local locks). */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](R/W) When set, RLDD is assumed to return a shared response (PSHA). Default operation assumes an
                                                                 exclusive response (PEMD). Note that an incorrect assumption only causes an extra tag
                                                                 write to be done upon receiving the response. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](R/W) When set, L2 STC/CAS operations performed at the home will immediately bring the block
                                                                 exclusive into the home. Default operation is to first request the block shared and only
                                                                 invalidate the remote if the compare succeeds. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](R/W) Describes how aggressive to be when waiting for local invalidates before sending CCPI
                                                                 responses which act like commits at the remote.
                                                                 0x0 = Conservative mode, waits until all local invalidates have been sent by their
                                                                 respective CBCs to the cores.
                                                                 0x1 = Moderate mode, waits until all local invalidates have been sent to their respective
                                                                 CBCs, but not necessarily actually sent to the cores themselves.
                                                                 0x2 = Aggressive mode, does not wait for local invalidates to begin their processing. */
        uint64_t shto                  : 1;  /**< [ 12: 12](R/W/H) Use short timeout intervals. When set, core uses SDIDTTO for both DID and commit counter
                                                                 timeouts, rather than DIDTTO/DIDTTO2. Similarly, L2C will use short instead of long
                                                                 timeout. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](R/W) When set, if the corresponding CCPI link is down, the hardware sets [SHTO].
                                                                 See OCX_COM_LINK(0..2)_CTL for a description of what events can contribute to the
                                                                 link_down condition. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](R/W) When set, any core issues any of an IO load, atomic, acking store, acking IOBADDR, or
                                                                 acking LMTST to a node that doesn't exist (existence defined by the ENAOCI bits), then the
                                                                 hardware sets [SHTO]. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](R/W) Selects the bit in the counter for the short timeout value (timeout used when [SHTO] is
                                                                 set). Values supported are between 9 and 29 (for a timeout values between 2^9 and 2^29).
                                                                 Actual timeout is between 1x and 2x this interval. For example if [SHTOLEN] = 14 (the
                                                                 reset
                                                                 value), the timeout is between 16K and 32K core clocks. Note: a value of 0 disables this
                                                                 timer. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Selects the bit in the counter for the long timeout value (timeout used when [SHTO] is
                                                                 clear). Values supported are between 11 and 29 (for a timeout values between 2^11 and
                                                                 2^29). Actual timeout is between 1x and 2x this interval. For example if [LNGTOLEN] = 28
                                                                 (the reset value), the timeout is between 256M and 512M core clocks. Note: a value of 0
                                                                 disables this timer. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](R/W) When clear, L2 atomic operations (excluding CAS/STC) core initiated requests to remote
                                                                 addresses which miss at the requester will send the atomic request to be performed on the
                                                                 home node. Default operation will instead  be performed locally on the requesting node.
                                                                 For request initiated by IOB & for STC & CAS ops, see
                                                                 [LOCK_LOCAL_IOB]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS]. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](R/W) When set, L2 STC operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the STC request to be performed on the home node. For CAS ops [LOCK_LOCAL_CAS]. */
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](R/W) When set, L2 CAS operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the CAS request to be performed on the home node. For STC ops [LOCK_LOCAL_STC]. */
        uint64_t reserved_30           : 1;
        uint64_t reserved_31_63        : 33;
#endif /* Word 0 - End */
    } cn88xxp1;
    /* struct cavm_l2c_oci_ctl_s cn81xx; */
    /* struct cavm_l2c_oci_ctl_s cn83xx; */
    struct cavm_l2c_oci_ctl_cn88xxp2
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_31_63        : 33;
        uint64_t ncpend                : 1;  /**< [ 30: 30](RO/H) An indication that a node change is pending.  Hardware sets this bit when
                                                                 OCX_COM_NODE[ID] is changed and clears the bit when the node change has taken
                                                                 effect. */
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](R/W) When set, L2 CAS operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the CAS request to be performed on the home node. For STC ops [LOCK_LOCAL_STC]. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](R/W) When set, L2 STC operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the STC request to be performed on the home node. For CAS ops [LOCK_LOCAL_CAS]. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](R/W) When clear, L2 atomic operations (excluding CAS/STC) core initiated requests to remote
                                                                 addresses which miss at the requester will send the atomic request to be performed on the
                                                                 home node. Default operation will instead  be performed locally on the requesting node.
                                                                 For request initiated by IOB & for STC & CAS ops, see
                                                                 [LOCK_LOCAL_IOB]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS]. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Selects the bit in the counter for the long timeout value (timeout used when [SHTO] is
                                                                 clear). Values supported are between 11 and 29 (for a timeout values between 2^11 and
                                                                 2^29). Actual timeout is between 1x and 2x this interval. For example if [LNGTOLEN] = 28
                                                                 (the reset value), the timeout is between 256M and 512M core clocks. Note: a value of 0
                                                                 disables this timer. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](R/W) Selects the bit in the counter for the short timeout value (timeout used when [SHTO] is
                                                                 set). Values supported are between 9 and 29 (for a timeout values between 2^9 and 2^29).
                                                                 Actual timeout is between 1x and 2x this interval. For example if [SHTOLEN] = 14 (the
                                                                 reset
                                                                 value), the timeout is between 16K and 32K core clocks. Note: a value of 0 disables this
                                                                 timer. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](R/W) When set, any core issues any of an IO load, atomic, acking store, acking IOBADDR, or
                                                                 acking LMTST to a node that doesn't exist (existence defined by the ENAOCI bits), then the
                                                                 hardware sets [SHTO]. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](R/W) When set, if the corresponding CCPI link is down, the hardware sets [SHTO].
                                                                 See OCX_COM_LINK(0..2)_CTL for a description of what events can contribute to the
                                                                 link_down condition. */
        uint64_t shto                  : 1;  /**< [ 12: 12](R/W/H) Use short timeout intervals. When set, core uses SDIDTTO for both DID and commit counter
                                                                 timeouts, rather than DIDTTO/DIDTTO2. Similarly, L2C will use short instead of long
                                                                 timeout. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](R/W) Describes how aggressive to be when waiting for local invalidates before sending CCPI
                                                                 responses which act like commits at the remote.
                                                                 0x0 = Conservative mode, waits until all local invalidates have been sent by their
                                                                 respective CBCs to the cores.
                                                                 0x1 = Moderate mode, waits until all local invalidates have been sent to their respective
                                                                 CBCs, but not necessarily actually sent to the cores themselves.
                                                                 0x2 = Aggressive mode, does not wait for local invalidates to begin their processing. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](R/W) When set, L2 STC/CAS operations performed at the home will immediately bring the block
                                                                 exclusive into the home. Default operation is to first request the block shared and only
                                                                 invalidate the remote if the compare succeeds. */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](R/W) When set, RLDD is assumed to return a shared response (PSHA). Default operation assumes an
                                                                 exclusive response (PEMD). Note that an incorrect assumption only causes an extra tag
                                                                 write to be done upon receiving the response. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](R/W) When set, L2 atomic operations (excluding CAS/STC) initiated by IOB to remote addresses
                                                                 which miss at the requester are performed locally on the requesting node. When clear the
                                                                 operation instead sends the atomic request to be performed on the home node. For request
                                                                 initiated by core for STC and CAS ops; see
                                                                 [LOCK_LOCAL_PP]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS].
                                                                 Default is set to 1 (local locks). */
        uint64_t iofrcl                : 1;  /**< [  6:  6](R/W) When set, L2C services all I/O read and write operations on the local node, regardless of
                                                                 the value of the node ID bits in the physical address. During normal operation this bit is
                                                                 expected to be 0. Will only transition from 1 to 0, never from 0 to 1. */
        uint64_t reserved_4_5          : 2;
        uint64_t enaoci                : 4;  /**< [  3:  0](R/W) Enable CCPI processing (one bit per node_id). When set, perform CCPI
                                                                 processing. When clear, CCPI memory writes are blocked and CCPI memory reads
                                                                 return unpredictable data. When clear,
                                                                 CCPI I/O requests and MOC references are processed and sent to OCX where they are
                                                                 ultimately discarded. RDDISOCI/WRDISOCI/IORDDISOCI/IOWRDISOCI interrupts occur if and only
                                                                 if the corresponding ENAOCI<node> bit is clear. References to the local node (configured
                                                                 via OCX_COM_NODE[ID]) ignore the value of ENAOCI<node> because no CCPI processing is
                                                                 required. Similarly, all I/O references ignore the value of ENAOCI when
                                                                 L2C_OCI_CTL[IOFRCL] is set. */
#else /* Word 0 - Little Endian */
        uint64_t enaoci                : 4;  /**< [  3:  0](R/W) Enable CCPI processing (one bit per node_id). When set, perform CCPI
                                                                 processing. When clear, CCPI memory writes are blocked and CCPI memory reads
                                                                 return unpredictable data. When clear,
                                                                 CCPI I/O requests and MOC references are processed and sent to OCX where they are
                                                                 ultimately discarded. RDDISOCI/WRDISOCI/IORDDISOCI/IOWRDISOCI interrupts occur if and only
                                                                 if the corresponding ENAOCI<node> bit is clear. References to the local node (configured
                                                                 via OCX_COM_NODE[ID]) ignore the value of ENAOCI<node> because no CCPI processing is
                                                                 required. Similarly, all I/O references ignore the value of ENAOCI when
                                                                 L2C_OCI_CTL[IOFRCL] is set. */
        uint64_t reserved_4_5          : 2;
        uint64_t iofrcl                : 1;  /**< [  6:  6](R/W) When set, L2C services all I/O read and write operations on the local node, regardless of
                                                                 the value of the node ID bits in the physical address. During normal operation this bit is
                                                                 expected to be 0. Will only transition from 1 to 0, never from 0 to 1. */
        uint64_t lock_local_iob        : 1;  /**< [  7:  7](R/W) When set, L2 atomic operations (excluding CAS/STC) initiated by IOB to remote addresses
                                                                 which miss at the requester are performed locally on the requesting node. When clear the
                                                                 operation instead sends the atomic request to be performed on the home node. For request
                                                                 initiated by core for STC and CAS ops; see
                                                                 [LOCK_LOCAL_PP]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS].
                                                                 Default is set to 1 (local locks). */
        uint64_t rldd_psha             : 1;  /**< [  8:  8](R/W) When set, RLDD is assumed to return a shared response (PSHA). Default operation assumes an
                                                                 exclusive response (PEMD). Note that an incorrect assumption only causes an extra tag
                                                                 write to be done upon receiving the response. */
        uint64_t cas_fdx               : 1;  /**< [  9:  9](R/W) When set, L2 STC/CAS operations performed at the home will immediately bring the block
                                                                 exclusive into the home. Default operation is to first request the block shared and only
                                                                 invalidate the remote if the compare succeeds. */
        uint64_t inv_mode              : 2;  /**< [ 11: 10](R/W) Describes how aggressive to be when waiting for local invalidates before sending CCPI
                                                                 responses which act like commits at the remote.
                                                                 0x0 = Conservative mode, waits until all local invalidates have been sent by their
                                                                 respective CBCs to the cores.
                                                                 0x1 = Moderate mode, waits until all local invalidates have been sent to their respective
                                                                 CBCs, but not necessarily actually sent to the cores themselves.
                                                                 0x2 = Aggressive mode, does not wait for local invalidates to begin their processing. */
        uint64_t shto                  : 1;  /**< [ 12: 12](R/W/H) Use short timeout intervals. When set, core uses SDIDTTO for both DID and commit counter
                                                                 timeouts, rather than DIDTTO/DIDTTO2. Similarly, L2C will use short instead of long
                                                                 timeout. */
        uint64_t shtoen                : 3;  /**< [ 15: 13](R/WCAVM_L2C_CBCX_BIST_STATUS) When set, if the corresponding CCPI link is down, the hardware sets [SHTO].
                                                                 See OCX_COM_LINK(0..2)_CTL for a description of what events can contribute to the
                                                                 link_down condition. */
        uint64_t shtoioen              : 1;  /**< [ 16: 16](R/W) When set, any core issues any of an IO load, atomic, acking store, acking IOBADDR, or
                                                                 acking LMTST to a node that doesn't exist (existence defined by the ENAOCI bits), then the
                                                                 hardware sets [SHTO]. */
        uint64_t shtolen               : 5;  /**< [ 21: 17](R/W) Selects the bit in the counter for the short timeout value (timeout used when [SHTO] is
                                                                 set). Values supported are between 9 and 29 (for a timeout values between 2^9 and 2^29).
                                                                 Actual timeout is between 1x and 2x this interval. For example if [SHTOLEN] = 14 (the
                                                                 reset
                                                                 value), the timeout is between 16K and 32K core clocks. Note: a value of 0 disables this
                                                                 timer. */
        uint64_t lngtolen              : 5;  /**< [ 26: 22](R/W) Selects the bit in the counter for the long timeout value (timeout used when [SHTO] is
                                                                 clear). Values supported are between 11 and 29 (for a timeout values between 2^11 and
                                                                 2^29). Actual timeout is between 1x and 2x this interval. For example if [LNGTOLEN] = 28
                                                                 (the reset value), the timeout is between 256M and 512M core clocks. Note: a value of 0
                                                                 disables this timer. */
        uint64_t lock_local_pp         : 1;  /**< [ 27: 27](R/W) When clear, L2 atomic operations (excluding CAS/STC) core initiated requests to remote
                                                                 addresses which miss at the requester will send the atomic request to be performed on the
                                                                 home node. Default operation will instead  be performed locally on the requesting node.
                                                                 For request initiated by IOB & for STC & CAS ops, see
                                                                 [LOCK_LOCAL_IOB]/[LOCK_LOCAL_STC]/[LOCK_LOCAL_CAS]. */
        uint64_t lock_local_stc        : 1;  /**< [ 28: 28](R/W) When set, L2 STC operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the STC request to be performed on the home node. For CAS ops [LOCK_LOCAL_CAS]. */
        uint64_t lock_local_cas        : 1;  /**< [ 29: 29](R/W) When set, L2 CAS operations to remote addresses which miss at the requester will be
                                                                 performed locally (if possible) on the requesting node. Default operation will instead
                                                                 send the CAS request to be performed on the home node. For STC ops [LOCK_LOCAL_STC]. */
        uint64_t ncpend                : 1;  /**< [ 30: 30](RO/H) An indication that a node change is pending.  Hardware sets this bit when
                                                                 OCX_COM_NODE[ID] is changed and clears the bit when the node change has taken
                                                                 effect. */
        uint64_t reserved_31_63        : 33;
#endif /* Word 0 - End */
    } cn88xxp2;
};
typedef union cavm_l2c_oci_ctl cavm_l2c_oci_ctl_t;

#define CAVM_L2C_OCI_CTL CAVM_L2C_OCI_CTL_FUNC()
static inline uint64_t CAVM_L2C_OCI_CTL_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_L2C_OCI_CTL_FUNC(void)
{
    return 0x87e080800020ll;
}

#define typedef_CAVM_L2C_OCI_CTL cavm_l2c_oci_ctl_t
#define bustype_CAVM_L2C_OCI_CTL CSR_TYPE_RSL
#define basename_CAVM_L2C_OCI_CTL "L2C_OCI_CTL"
#define device_bar_CAVM_L2C_OCI_CTL 0x0 /* PF_BAR0 */
#define busnum_CAVM_L2C_OCI_CTL 0
#define arguments_CAVM_L2C_OCI_CTL -1,-1,-1,-1

#endif /* __CAVM_CSRS_L2C_H__ */

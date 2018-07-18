#ifndef __CAVM_CSRS_AP_H__
#define __CAVM_CSRS_AP_H__
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
 * Cavium AP.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Register (SYSREG) ap_ccsidr_el1
 *
 * AP Current Cache Size ID Register
 * This register provides information about the architecture of the currently selected
 * cache.
 */
union cavm_ap_ccsidr_el1
{
    uint32_t u;
    struct cavm_ap_ccsidr_el1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN88XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 8191. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN88XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15. */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
#else /* Word 0 - Little Endian */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN88XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN88XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 8191. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
#endif /* Word 0 - End */
    } s;
    struct cavm_ap_ccsidr_el1_cn81xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN81XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 1023.

                                                                 For CN80XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 1023. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN81XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15.

                                                                 For CN80XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 7. */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
#else /* Word 0 - Little Endian */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN81XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15.

                                                                 For CN80XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 7. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN81XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 1023.

                                                                 For CN80XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 1023. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
#endif /* Word 0 - End */
    } cn81xx;
    /* struct cavm_ap_ccsidr_el1_s cn88xx; */
    struct cavm_ap_ccsidr_el1_cn83xx
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN83XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 4095. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN83XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15. */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
#else /* Word 0 - Little Endian */
        uint32_t linesize              : 3;  /**< [  2:  0](RO) Cache-line size, in (Log2(Number of bytes in cache line)) - 4.

                                                                 For CNXXXX, 128 bytes. */
        uint32_t associativity         : 10; /**< [ 12:  3](RO) Associativity of cache minus 1, therefore a value of 0 indicates
                                                                 an associativity of 1. The associativity does not have to be a
                                                                 power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 31.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 38.

                                                                 For CN83XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 15. */
        uint32_t numsets               : 15; /**< [ 27: 13](RO) Number of sets in cache minus 1, therefore a value of 0
                                                                 indicates 1 set in the cache. The number of sets does not have
                                                                 to be a power of 2.

                                                                 For CNXXXX L1D (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 0), is 7.

                                                                 For CNXXXX L1I (AP_CSSELR_EL1[LEVEL] = 0x0, AP_CSSELR_EL1[IND] = 1), is 15.

                                                                 For CN83XX L2 (AP_CSSELR_EL1[LEVEL] = 0x1, AP_CSSELR_EL1[IND] = 0), is 4095. */
        uint32_t wa                    : 1;  /**< [ 28: 28](RO) Indicates whether the selected cache level supports write-allocation.
                                                                 0 = Write-allocation not supported.
                                                                 1 = Write-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t ra                    : 1;  /**< [ 29: 29](RO) Indicates whether the selected cache level supports read-allocation.
                                                                 0 = Read-allocation not supported.
                                                                 1 = Read-allocation supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wb                    : 1;  /**< [ 30: 30](RO) Indicates whether the selected cache level supports write-back.
                                                                 0 = Write-back not supported.
                                                                 1 = Write-back supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
        uint32_t wt                    : 1;  /**< [ 31: 31](RO) Indicates whether the selected cache level supports write-through.
                                                                 0 = Write-through not supported.
                                                                 1 = Write-through supported.

                                                                 For CNXXXX does not apply as hardware managed coherence. */
#endif /* Word 0 - End */
    } cn83xx;
};
typedef union cavm_ap_ccsidr_el1 cavm_ap_ccsidr_el1_t;

#define CAVM_AP_CCSIDR_EL1 CAVM_AP_CCSIDR_EL1_FUNC()
static inline uint64_t CAVM_AP_CCSIDR_EL1_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_AP_CCSIDR_EL1_FUNC(void)
{
    return 0x30100000000ll;
}

#define typedef_CAVM_AP_CCSIDR_EL1 cavm_ap_ccsidr_el1_t
#define bustype_CAVM_AP_CCSIDR_EL1 CSR_TYPE_SYSREG
#define basename_CAVM_AP_CCSIDR_EL1 "AP_CCSIDR_EL1"
#define busnum_CAVM_AP_CCSIDR_EL1 0
#define arguments_CAVM_AP_CCSIDR_EL1 -1,-1,-1,-1

/**
 * Register (SYSREG) ap_csselr_el1
 *
 * AP Cache Size Selection Register
 * Selects the current Cache Size ID Register, AP_CCSIDR_EL1, by
 *     specifying the required cache level and the cache type (either
 *     instruction or data cache).
 */
union cavm_ap_csselr_el1
{
    uint32_t u;
    struct cavm_ap_csselr_el1_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_4_31         : 28;
        uint32_t level                 : 3;  /**< [  3:  1](R/W) Cache level of required cache. */
        uint32_t ind                   : 1;  /**< [  0:  0](R/W) Instruction not Data bit.
                                                                 0 = Data or unified cache.
                                                                 1 = Instruction cache. */
#else /* Word 0 - Little Endian */
        uint32_t ind                   : 1;  /**< [  0:  0](R/W) Instruction not Data bit.
                                                                 0 = Data or unified cache.
                                                                 1 = Instruction cache. */
        uint32_t level                 : 3;  /**< [  3:  1](R/W) Cache level of required cache. */
        uint32_t reserved_4_31         : 28;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_ap_csselr_el1_s cn; */
};
typedef union cavm_ap_csselr_el1 cavm_ap_csselr_el1_t;

#define CAVM_AP_CSSELR_EL1 CAVM_AP_CSSELR_EL1_FUNC()
static inline uint64_t CAVM_AP_CSSELR_EL1_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_AP_CSSELR_EL1_FUNC(void)
{
    return 0x30200000000ll;
}

#define typedef_CAVM_AP_CSSELR_EL1 cavm_ap_csselr_el1_t
#define bustype_CAVM_AP_CSSELR_EL1 CSR_TYPE_SYSREG
#define basename_CAVM_AP_CSSELR_EL1 "AP_CSSELR_EL1"
#define busnum_CAVM_AP_CSSELR_EL1 0
#define arguments_CAVM_AP_CSSELR_EL1 -1,-1,-1,-1

#endif /* __CAVM_CSRS_AP_H__ */

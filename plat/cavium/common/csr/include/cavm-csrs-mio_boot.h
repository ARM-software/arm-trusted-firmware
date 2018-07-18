#ifndef __CAVM_CSRS_MIO_BOOT_H__
#define __CAVM_CSRS_MIO_BOOT_H__
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
 * Cavium MIO_BOOT.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration mio_boot_bar_e
 *
 * MIO Boot Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_MIO_BOOT_BAR_E_MIO_BOOT_PF_BAR0 (0x87e000000000ll)
#define CAVM_MIO_BOOT_BAR_E_MIO_BOOT_PF_BAR0_SIZE 0x800000ull

/**
 * Register (RSL) mio_boot_ap_jump
 *
 * MIO Boot AP Jump Address Register
 */
union cavm_mio_boot_ap_jump
{
    uint64_t u;
    struct cavm_mio_boot_ap_jump_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t addr                  : 64; /**< [ 63:  0](SR/W) Boot address.  This register contains the address the internal boot loader
                                                                 will jump to after reset. */
#else /* Word 0 - Little Endian */
        uint64_t addr                  : 64; /**< [ 63:  0](SR/W) Boot address.  This register contains the address the internal boot loader
                                                                 will jump to after reset. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_boot_ap_jump_s cn; */
};
typedef union cavm_mio_boot_ap_jump cavm_mio_boot_ap_jump_t;

#define CAVM_MIO_BOOT_AP_JUMP CAVM_MIO_BOOT_AP_JUMP_FUNC()
static inline uint64_t CAVM_MIO_BOOT_AP_JUMP_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_BOOT_AP_JUMP_FUNC(void)
{
    return 0x87e0000000d0ll;
}

#define typedef_CAVM_MIO_BOOT_AP_JUMP cavm_mio_boot_ap_jump_t
#define bustype_CAVM_MIO_BOOT_AP_JUMP CSR_TYPE_RSL
#define basename_CAVM_MIO_BOOT_AP_JUMP "MIO_BOOT_AP_JUMP"
#define device_bar_CAVM_MIO_BOOT_AP_JUMP 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_BOOT_AP_JUMP 0
#define arguments_CAVM_MIO_BOOT_AP_JUMP -1,-1,-1,-1

#endif /* __CAVM_CSRS_MIO_BOOT_H__ */

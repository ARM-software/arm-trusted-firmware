#ifndef __CAVM_CSRS_MIO_FUS_H__
#define __CAVM_CSRS_MIO_FUS_H__
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
 * Cavium MIO_FUS.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration mio_fus_bar_e
 *
 * MIO FUS Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_MIO_FUS_BAR_E_MIO_FUS_PF_BAR0 (0x87e003000000ll)
#define CAVM_MIO_FUS_BAR_E_MIO_FUS_PF_BAR0_SIZE 0x800000ull

/**
 * Register (RSL) mio_fus_rcmd
 *
 * MIO Fuse Read Command Register
 * To read an efuse, software writes [ADDR,PEND] with
 * the byte address of the fuse in question, then software can poll
 * [PEND]. When [PEND] = 0, then [DAT] is valid.
 * In addition, if the efuse read went to the efuse banks, software can
 * read MIO_FUS_BNK_DAT() which contains all 128 fuses in the bank
 * associated in ADDR.  Fuses 1023..960 are never accessible on pass 1 parts.
 * In addition, fuses 1023..960 are not accessible if
 * MIO_FUS_DAT2[DORM_CRYPTO] is enabled.
 */
union cavm_mio_fus_rcmd
{
    uint64_t u;
    struct cavm_mio_fus_rcmd_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_24_63        : 40;
        uint64_t dat                   : 8;  /**< [ 23: 16](RO/H) Eight bits of fuse data. */
        uint64_t reserved_13_15        : 3;
        uint64_t pend                  : 1;  /**< [ 12: 12](R/W/H) Software sets this bit to 1 on a write operation that starts
                                                                 the fuse read operation. Hardware clears this bit when the read
                                                                 operation is complete and [DAT] is valid. MIO_FUS_READ_TIMES[RDSTB_WH]
                                                                 determines the time for the operation to complete. */
        uint64_t reserved_11           : 1;
        uint64_t addr_hi               : 2;  /**< [ 10:  9](R/W) Upper fuse address bits to extend space beyond 2k fuses.
                                                                 Valid range is 0x0-0x3.  Software should not change this
                                                                 field while [PEND] is set.  It should wait for
                                                                 the hardware to clear it. */
        uint64_t efuse                 : 1;  /**< [  8:  8](R/W) Efuse storage. When set, the return data is from the efuse
                                                                 directly rather than the local storage. */
        uint64_t addr                  : 8;  /**< [  7:  0](R/W) Address. Specifies the byte address of the fuse to read.
                                                                 Software should not change this field while [PEND]
                                                                 is set. It must wait for the hardware to clear it. */
#else /* Word 0 - Little Endian */
        uint64_t addr                  : 8;  /**< [  7:  0](R/W) Address. Specifies the byte address of the fuse to read.
                                                                 Software should not change this field while [PEND]
                                                                 is set. It must wait for the hardware to clear it. */
        uint64_t efuse                 : 1;  /**< [  8:  8](R/W) Efuse storage. When set, the return data is from the efuse
                                                                 directly rather than the local storage. */
        uint64_t addr_hi               : 2;  /**< [ 10:  9](R/W) Upper fuse address bits to extend space beyond 2k fuses.
                                                                 Valid range is 0x0-0x3.  Software should not change this
                                                                 field while [PEND] is set.  It should wait for
                                                                 the hardware to clear it. */
        uint64_t reserved_11           : 1;
        uint64_t pend                  : 1;  /**< [ 12: 12](R/W/H) Software sets this bit to 1 on a write operation that starts
                                                                 the fuse read operation. Hardware clears this bit when the read
                                                                 operation is complete and [DAT] is valid. MIO_FUS_READ_TIMES[RDSTB_WH]
                                                                 determines the time for the operation to complete. */
        uint64_t reserved_13_15        : 3;
        uint64_t dat                   : 8;  /**< [ 23: 16](RO/H) Eight bits of fuse data. */
        uint64_t reserved_24_63        : 40;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_fus_rcmd_s cn; */
};
typedef union cavm_mio_fus_rcmd cavm_mio_fus_rcmd_t;

#define CAVM_MIO_FUS_RCMD CAVM_MIO_FUS_RCMD_FUNC()
static inline uint64_t CAVM_MIO_FUS_RCMD_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_FUS_RCMD_FUNC(void)
{
    return 0x87e003001500ll;
}

#define typedef_CAVM_MIO_FUS_RCMD cavm_mio_fus_rcmd_t
#define bustype_CAVM_MIO_FUS_RCMD CSR_TYPE_RSL
#define basename_CAVM_MIO_FUS_RCMD "MIO_FUS_RCMD"
#define device_bar_CAVM_MIO_FUS_RCMD 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_FUS_RCMD 0
#define arguments_CAVM_MIO_FUS_RCMD -1,-1,-1,-1

#endif /* __CAVM_CSRS_MIO_FUS_H__ */

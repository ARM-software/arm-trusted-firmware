#ifndef __CAVM_CSRS_MIO_TWS_H__
#define __CAVM_CSRS_MIO_TWS_H__
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
 * Cavium MIO_TWS.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration mio_tws_bar_e
 *
 * TWSI Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR0(a) (0x87e0d0000000ll + 0x1000000ll * (a))
#define CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR0_SIZE 0x800000ull
#define CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR4(a) (0x87e0d0f00000ll + 0x1000000ll * (a))
#define CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR4_SIZE 0x100000ull

/**
 * Register (RSL) mio_tws#_sw_twsi
 *
 * TWSI Software to TWSI Register
 * This register allows software to:
 * * Initiate master-mode operations with a write operation, and read the result with a
 * read operation.
 * * Load four bytes for later retrieval (slave mode) with a write operation and check validity
 * with a read operation.
 * * Launch a configuration read/write operation with a write operation and read the result with
 * a read operation.
 *
 * This register should be read or written by software, and read by the TWSI device. The TWSI
 * device can use either two-byte or five-byte read operations to reference this register.
 * The TWSI device considers this register valid when [V] = 1 and [SLONLY] = 1.
 */
union cavm_mio_twsx_sw_twsi
{
    uint64_t u;
    struct cavm_mio_twsx_sw_twsi_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t v                     : 1;  /**< [ 63: 63](RC/W/H) Valid bit. Set on a write operation (should always be written with a 1). Cleared when a
                                                                 TWSI master-mode operation completes, a TWSI configuration register access completes, or
                                                                 when the TWSI device reads the register if [SLONLY] = 1. */
        uint64_t slonly                : 1;  /**< [ 62: 62](R/W) Slave only mode.

                                                                 When this bit is set, no operations are initiated with a write operation. Only the D field
                                                                 is updated in this case.

                                                                 When this bit is clear, a write operation initiates either a master-mode operation or a
                                                                 TWSI configuration register access. */
        uint64_t eia                   : 1;  /**< [ 61: 61](R/W) Extended internal address. Sends an additional internal address byte (the MSB of IA is
                                                                 from MIO_TWS()_SW_TWSI_EXT[IA]). */
        uint64_t op                    : 4;  /**< [ 60: 57](R/W) Opcode field. When the register is written with [SLONLY] = 0, this field initiates one of
                                                                 the following read or write operations:
                                                                 0x0 = 7-bit byte master-mode operation.
                                                                 0x1 = 7-bit byte combined-read master-mode operation, 7-bit byte write-with-IA master-mode
                                                                 operation.
                                                                 0x2 = 10-bit byte master-mode operation.
                                                                 0x3 = 10-bit byte combined-read master-mode operation, 10-bit byte write-with-IA master-
                                                                 mode operation.
                                                                 0x4 = TWSI master-clock register, TWSI_CLK in TWSI Master Clock Register.
                                                                 0x6 = See [EOP_IA] field.
                                                                 0x8 = 7-bit 4-byte master-mode operation.
                                                                 0x9 = 7-bit 4-byte combined-read master-mode operation, 7-bit 4-byte write-with-IA master-
                                                                 mode operation.
                                                                 0xA = 10-bit 4-byte master-mode operation.
                                                                 0xB = 10-bit 4-byte combined-read master-mode operation, 10-bit 4-byte write-with-IA
                                                                 master-mode operation. */
        uint64_t r                     : 1;  /**< [ 56: 56](R/W/H) Read bit or result. If this bit is set on a CSR write when [SLONLY] = 0, the
                                                                 operation is a read operation (if clear, it is a write operation).
                                                                 On a CSR read, this bit returns the result indication for the most recent
                                                                 master-mode operation, 1 = success, 0 = failure. */
        uint64_t sovr                  : 1;  /**< [ 55: 55](R/W) Size override. If this bit is set, use the SIZE field to determine the master-mode
                                                                 operation size rather than what OP specifies. For operations greater than four bytes, the
                                                                 additional data is contained in MIO_TWS()_SW_TWSI_EXT[DATA]. */
        uint64_t size                  : 3;  /**< [ 54: 52](R/W) Size minus one. Specifies the size in bytes of the master-mode operation if SOVR = 1. (0 =
                                                                 1 byte, 1 = 2 bytes, ... 7 = 8 bytes). */
        uint64_t scr                   : 2;  /**< [ 51: 50](R/W) Scratch. Unused, but retain state. */
        uint64_t addr                  : 10; /**< [ 49: 40](R/W) Address field. The address of the remote device for a master-mode operation. ADDR<9:7> are
                                                                 only used for 10-bit addressing.

                                                                 Note that when mastering a 7-bit OP, ADDR<6:0> should not take any of the values 0x78,
                                                                 0x79, 0x7A nor 0x7B. (These 7-bit addresses are reserved to extend to 10-bit addressing). */
        uint64_t ia                    : 5;  /**< [ 39: 35](R/W) Internal address. Used when launching a combined master-mode operation. The lower 3
                                                                 address bits are contained in [EOP_IA]. */
        uint64_t eop_ia                : 3;  /**< [ 34: 32](R/W) Extra opcode, used when OP<3:0> = 0x6 and [SLONLY] = 0.
                                                                 0x0 = TWSI slave address register (TWSI_SLAVE_ADD).
                                                                 0x1 = TWSI data register (TWSI_DATA).
                                                                 0x2 = TWSI control register (TWSI_CTL).
                                                                 0x3 = (when R =  0) TWSI clock control register (TWSI_CLKCTL).
                                                                 0x3 = (when R = 1) TWSI status register (TWSI_STAT).
                                                                 0x4 = TWSI extended slave register (TWSI_SLAVE_ADD_EXT).
                                                                 0x7 = TWSI soft reset register (TWSI_RST).

                                                                 Also provides the lower 3 bits of internal address when launching a combined master-mode
                                                                 operation. */
        uint64_t data                  : 32; /**< [ 31:  0](R/W/H) Data field.
                                                                 Used on a write operation when:
                                                                 * Initiating a master-mode write operation ([SLONLY] = 0).
                                                                 * Writing a TWSI configuration register ([SLONLY] = 0).
                                                                 * A slave-mode write operation ([SLONLY] = 1).

                                                                 The read value is updated by:
                                                                 * A write operation to this register.
                                                                 * Master-mode completion (contains error code).
                                                                 * TWSI configuration-register read (contains result). */
#else /* Word 0 - Little Endian */
        uint64_t data                  : 32; /**< [ 31:  0](R/W/H) Data field.
                                                                 Used on a write operation when:
                                                                 * Initiating a master-mode write operation ([SLONLY] = 0).
                                                                 * Writing a TWSI configuration register ([SLONLY] = 0).
                                                                 * A slave-mode write operation ([SLONLY] = 1).

                                                                 The read value is updated by:
                                                                 * A write operation to this register.
                                                                 * Master-mode completion (contains error code).
                                                                 * TWSI configuration-register read (contains result). */
        uint64_t eop_ia                : 3;  /**< [ 34: 32](R/W) Extra opcode, used when OP<3:0> = 0x6 and [SLONLY] = 0.
                                                                 0x0 = TWSI slave address register (TWSI_SLAVE_ADD).
                                                                 0x1 = TWSI data register (TWSI_DATA).
                                                                 0x2 = TWSI control register (TWSI_CTL).
                                                                 0x3 = (when R =  0) TWSI clock control register (TWSI_CLKCTL).
                                                                 0x3 = (when R = 1) TWSI status register (TWSI_STAT).
                                                                 0x4 = TWSI extended slave register (TWSI_SLAVE_ADD_EXT).
                                                                 0x7 = TWSI soft reset register (TWSI_RST).

                                                                 Also provides the lower 3 bits of internal address when launching a combined master-mode
                                                                 operation. */
        uint64_t ia                    : 5;  /**< [ 39: 35](R/W) Internal address. Used when launching a combined master-mode operation. The lower 3
                                                                 address bits are contained in [EOP_IA]. */
        uint64_t addr                  : 10; /**< [ 49: 40](R/W) Address field. The address of the remote device for a master-mode operation. ADDR<9:7> are
                                                                 only used for 10-bit addressing.

                                                                 Note that when mastering a 7-bit OP, ADDR<6:0> should not take any of the values 0x78,
                                                                 0x79, 0x7A nor 0x7B. (These 7-bit addresses are reserved to extend to 10-bit addressing). */
        uint64_t scr                   : 2;  /**< [ 51: 50](R/W) Scratch. Unused, but retain state. */
        uint64_t size                  : 3;  /**< [ 54: 52](R/W) Size minus one. Specifies the size in bytes of the master-mode operation if SOVR = 1. (0 =
                                                                 1 byte, 1 = 2 bytes, ... 7 = 8 bytes). */
        uint64_t sovr                  : 1;  /**< [ 55: 55](R/W) Size override. If this bit is set, use the SIZE field to determine the master-mode
                                                                 operation size rather than what OP specifies. For operations greater than four bytes, the
                                                                 additional data is contained in MIO_TWS()_SW_TWSI_EXT[DATA]. */
        uint64_t r                     : 1;  /**< [ 56: 56](R/W/H) Read bit or result. If this bit is set on a CSR write when [SLONLY] = 0, the
                                                                 operation is a read operation (if clear, it is a write operation).
                                                                 On a CSR read, this bit returns the result indication for the most recent
                                                                 master-mode operation, 1 = success, 0 = failure. */
        uint64_t op                    : 4;  /**< [ 60: 57](R/W) Opcode field. When the register is written with [SLONLY] = 0, this field initiates one of
                                                                 the following read or write operations:
                                                                 0x0 = 7-bit byte master-mode operation.
                                                                 0x1 = 7-bit byte combined-read master-mode operation, 7-bit byte write-with-IA master-mode
                                                                 operation.
                                                                 0x2 = 10-bit byte master-mode operation.
                                                                 0x3 = 10-bit byte combined-read master-mode operation, 10-bit byte write-with-IA master-
                                                                 mode operation.
                                                                 0x4 = TWSI master-clock register, TWSI_CLK in TWSI Master Clock Register.
                                                                 0x6 = See [EOP_IA] field.
                                                                 0x8 = 7-bit 4-byte master-mode operation.
                                                                 0x9 = 7-bit 4-byte combined-read master-mode operation, 7-bit 4-byte write-with-IA master-
                                                                 mode operation.
                                                                 0xA = 10-bit 4-byte master-mode operation.
                                                                 0xB = 10-bit 4-byte combined-read master-mode operation, 10-bit 4-byte write-with-IA
                                                                 master-mode operation. */
        uint64_t eia                   : 1;  /**< [ 61: 61](R/W) Extended internal address. Sends an additional internal address byte (the MSB of IA is
                                                                 from MIO_TWS()_SW_TWSI_EXT[IA]). */
        uint64_t slonly                : 1;  /**< [ 62: 62](R/W) Slave only mode.

                                                                 When this bit is set, no operations are initiated with a write operation. Only the D field
                                                                 is updated in this case.

                                                                 When this bit is clear, a write operation initiates either a master-mode operation or a
                                                                 TWSI configuration register access. */
        uint64_t v                     : 1;  /**< [ 63: 63](RC/W/H) Valid bit. Set on a write operation (should always be written with a 1). Cleared when a
                                                                 TWSI master-mode operation completes, a TWSI configuration register access completes, or
                                                                 when the TWSI device reads the register if [SLONLY] = 1. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_twsx_sw_twsi_s cn; */
};
typedef union cavm_mio_twsx_sw_twsi cavm_mio_twsx_sw_twsi_t;

static inline uint64_t CAVM_MIO_TWSX_SW_TWSI(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_TWSX_SW_TWSI(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x87e0d0001000ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x87e0d0001000ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=5))
        return 0x87e0d0001000ll + 0x1000000ll * ((a) & 0x7);
    __cavm_csr_fatal("MIO_TWSX_SW_TWSI", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_MIO_TWSX_SW_TWSI(a) cavm_mio_twsx_sw_twsi_t
#define bustype_CAVM_MIO_TWSX_SW_TWSI(a) CSR_TYPE_RSL
#define basename_CAVM_MIO_TWSX_SW_TWSI(a) "MIO_TWSX_SW_TWSI"
#define device_bar_CAVM_MIO_TWSX_SW_TWSI(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_TWSX_SW_TWSI(a) (a)
#define arguments_CAVM_MIO_TWSX_SW_TWSI(a) (a),-1,-1,-1

/**
 * Register (RSL) mio_tws#_sw_twsi_ext
 *
 * TWSI Software to TWSI Extension Register
 * This register contains an additional byte of internal address and four additional bytes of
 * data to be used with TWSI master-mode operations.
 *
 * The IA field is sent as the first byte of internal address when performing master-mode
 * combined-read/write-with-IA operations and MIO_TWS()_SW_TWSI[EIA] is set. The D field
 * extends the data field of MIO_TWS()_SW_TWSI for a total of 8 bytes (SOVR must be set to
 * perform operations greater than 4 bytes).
 */
union cavm_mio_twsx_sw_twsi_ext
{
    uint64_t u;
    struct cavm_mio_twsx_sw_twsi_ext_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_40_63        : 24;
        uint64_t ia                    : 8;  /**< [ 39: 32](R/W) Extended internal address. Sent as the first byte of internal address when performing
                                                                 master-mode combined-read/write-with-IA operations and MIO_TWS()_SW_TWSI[EIA] is set. */
        uint64_t data                  : 32; /**< [ 31:  0](R/W/H) Extended data. Extends the data field of MIO_TWS()_SW_TWSI for a total of eight bytes
                                                                 (MIO_TWS()_SW_TWSI[SOVR] must be set to 1 to perform operations greater than four
                                                                 bytes). */
#else /* Word 0 - Little Endian */
        uint64_t data                  : 32; /**< [ 31:  0](R/W/H) Extended data. Extends the data field of MIO_TWS()_SW_TWSI for a total of eight bytes
                                                                 (MIO_TWS()_SW_TWSI[SOVR] must be set to 1 to perform operations greater than four
                                                                 bytes). */
        uint64_t ia                    : 8;  /**< [ 39: 32](R/W) Extended internal address. Sent as the first byte of internal address when performing
                                                                 master-mode combined-read/write-with-IA operations and MIO_TWS()_SW_TWSI[EIA] is set. */
        uint64_t reserved_40_63        : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_twsx_sw_twsi_ext_s cn; */
};
typedef union cavm_mio_twsx_sw_twsi_ext cavm_mio_twsx_sw_twsi_ext_t;

static inline uint64_t CAVM_MIO_TWSX_SW_TWSI_EXT(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_TWSX_SW_TWSI_EXT(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x87e0d0001018ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x87e0d0001018ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=5))
        return 0x87e0d0001018ll + 0x1000000ll * ((a) & 0x7);
    __cavm_csr_fatal("MIO_TWSX_SW_TWSI_EXT", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_MIO_TWSX_SW_TWSI_EXT(a) cavm_mio_twsx_sw_twsi_ext_t
#define bustype_CAVM_MIO_TWSX_SW_TWSI_EXT(a) CSR_TYPE_RSL
#define basename_CAVM_MIO_TWSX_SW_TWSI_EXT(a) "MIO_TWSX_SW_TWSI_EXT"
#define device_bar_CAVM_MIO_TWSX_SW_TWSI_EXT(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_TWSX_SW_TWSI_EXT(a) (a)
#define arguments_CAVM_MIO_TWSX_SW_TWSI_EXT(a) (a),-1,-1,-1

/**
 * Register (RSL) mio_tws#_twsi_sw
 *
 * TWSI to Software Register
 * This register allows the TWSI device to transfer data to software and later check that
 * software has received the information.
 *
 * This register should be read or written by the TWSI device, and read by software. The TWSI
 * device can use one-byte or four-byte payload write operations, and two-byte payload read
 * operations. The TWSI device considers this register valid when V = 1.
 */
union cavm_mio_twsx_twsi_sw
{
    uint64_t u;
    struct cavm_mio_twsx_twsi_sw_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t v                     : 2;  /**< [ 63: 62](RC/W/H) Valid bits. These bits are not directly writable. They are set to 11 on any write
                                                                 operation by the TWSI device. They are cleared to 00 on any read operation by software. */
        uint64_t reserved_32_61        : 30;
        uint64_t data                  : 32; /**< [ 31:  0](RO/H) Data field. Updated on a write operation by the TWSI device. */
#else /* Word 0 - Little Endian */
        uint64_t data                  : 32; /**< [ 31:  0](RO/H) Data field. Updated on a write operation by the TWSI device. */
        uint64_t reserved_32_61        : 30;
        uint64_t v                     : 2;  /**< [ 63: 62](RC/W/H) Valid bits. These bits are not directly writable. They are set to 11 on any write
                                                                 operation by the TWSI device. They are cleared to 00 on any read operation by software. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_twsx_twsi_sw_s cn; */
};
typedef union cavm_mio_twsx_twsi_sw cavm_mio_twsx_twsi_sw_t;

static inline uint64_t CAVM_MIO_TWSX_TWSI_SW(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_TWSX_TWSI_SW(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x87e0d0001008ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x87e0d0001008ll + 0x1000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=5))
        return 0x87e0d0001008ll + 0x1000000ll * ((a) & 0x7);
    __cavm_csr_fatal("MIO_TWSX_TWSI_SW", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_MIO_TWSX_TWSI_SW(a) cavm_mio_twsx_twsi_sw_t
#define bustype_CAVM_MIO_TWSX_TWSI_SW(a) CSR_TYPE_RSL
#define basename_CAVM_MIO_TWSX_TWSI_SW(a) "MIO_TWSX_TWSI_SW"
#define device_bar_CAVM_MIO_TWSX_TWSI_SW(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_TWSX_TWSI_SW(a) (a)
#define arguments_CAVM_MIO_TWSX_TWSI_SW(a) (a),-1,-1,-1

#endif /* __CAVM_CSRS_MIO_TWS_H__ */

#ifndef __CAVM_CSRS_MIO_EMM_H__
#define __CAVM_CSRS_MIO_EMM_H__
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
 * Cavium MIO_EMM.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration mio_emm_bar_e
 *
 * eMMC Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_MIO_EMM_BAR_E_MIO_EMM_PF_BAR0 (0x87e009000000ll)
#define CAVM_MIO_EMM_BAR_E_MIO_EMM_PF_BAR0_SIZE 0x800000ull
#define CAVM_MIO_EMM_BAR_E_MIO_EMM_PF_BAR4 (0x87e009f00000ll)
#define CAVM_MIO_EMM_BAR_E_MIO_EMM_PF_BAR4_SIZE 0x100000ull

/**
 * Register (RSL) mio_emm_access_wdog
 *
 * eMMC Access Watchdog Register
 */
union cavm_mio_emm_access_wdog
{
    uint64_t u;
    struct cavm_mio_emm_access_wdog_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_32_63        : 32;
        uint64_t clk_cnt               : 32; /**< [ 31:  0](R/W) Number of coprocessor-clocks to allow for a store operation to the device to complete
                                                                 before hardware will halt the operation.
                                                                 Hardware will inject an error on the next 512-byte block boundary.   The pending DMA
                                                                 operation can be resumed or terminated. A value of zero disables timer. */
#else /* Word 0 - Little Endian */
        uint64_t clk_cnt               : 32; /**< [ 31:  0](R/W) Number of coprocessor-clocks to allow for a store operation to the device to complete
                                                                 before hardware will halt the operation.
                                                                 Hardware will inject an error on the next 512-byte block boundary.   The pending DMA
                                                                 operation can be resumed or terminated. A value of zero disables timer. */
        uint64_t reserved_32_63        : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_access_wdog_s cn; */
};
typedef union cavm_mio_emm_access_wdog cavm_mio_emm_access_wdog_t;

#define CAVM_MIO_EMM_ACCESS_WDOG CAVM_MIO_EMM_ACCESS_WDOG_FUNC()
static inline uint64_t CAVM_MIO_EMM_ACCESS_WDOG_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_ACCESS_WDOG_FUNC(void)
{
    return 0x87e0090020f0ll;
}

#define typedef_CAVM_MIO_EMM_ACCESS_WDOG cavm_mio_emm_access_wdog_t
#define bustype_CAVM_MIO_EMM_ACCESS_WDOG CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_ACCESS_WDOG "MIO_EMM_ACCESS_WDOG"
#define device_bar_CAVM_MIO_EMM_ACCESS_WDOG 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_ACCESS_WDOG 0
#define arguments_CAVM_MIO_EMM_ACCESS_WDOG -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_buf_dat
 *
 * eMMC Data Buffer Access Register
 */
union cavm_mio_emm_buf_dat
{
    uint64_t u;
    struct cavm_mio_emm_buf_dat_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t dat                   : 64; /**< [ 63:  0](R/W/H) Direct access to the 1 KB data buffer memory. Address specified by MIO_EMM_BUF_IDX. */
#else /* Word 0 - Little Endian */
        uint64_t dat                   : 64; /**< [ 63:  0](R/W/H) Direct access to the 1 KB data buffer memory. Address specified by MIO_EMM_BUF_IDX. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_buf_dat_s cn; */
};
typedef union cavm_mio_emm_buf_dat cavm_mio_emm_buf_dat_t;

#define CAVM_MIO_EMM_BUF_DAT CAVM_MIO_EMM_BUF_DAT_FUNC()
static inline uint64_t CAVM_MIO_EMM_BUF_DAT_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_BUF_DAT_FUNC(void)
{
    return 0x87e0090020e8ll;
}

#define typedef_CAVM_MIO_EMM_BUF_DAT cavm_mio_emm_buf_dat_t
#define bustype_CAVM_MIO_EMM_BUF_DAT CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_BUF_DAT "MIO_EMM_BUF_DAT"
#define device_bar_CAVM_MIO_EMM_BUF_DAT 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_BUF_DAT 0
#define arguments_CAVM_MIO_EMM_BUF_DAT -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_buf_idx
 *
 * eMMC Data Buffer Address Register
 */
union cavm_mio_emm_buf_idx
{
    uint64_t u;
    struct cavm_mio_emm_buf_idx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_17_63        : 47;
        uint64_t inc                   : 1;  /**< [ 16: 16](R/W) Automatically advance [BUF_NUM]/[OFFSET] after each access to
                                                                 MIO_EMM_BUF_DAT. Wraps after the last offset of the last data buffer. */
        uint64_t reserved_7_15         : 9;
        uint64_t buf_num               : 1;  /**< [  6:  6](R/W/H) Specify the data buffer for the next access to MIO_EMM_BUF_DAT. */
        uint64_t offset                : 6;  /**< [  5:  0](R/W/H) Specify the 8B data buffer offset for the next access to MIO_EMM_BUF_DAT. */
#else /* Word 0 - Little Endian */
        uint64_t offset                : 6;  /**< [  5:  0](R/W/H) Specify the 8B data buffer offset for the next access to MIO_EMM_BUF_DAT. */
        uint64_t buf_num               : 1;  /**< [  6:  6](R/W/H) Specify the data buffer for the next access to MIO_EMM_BUF_DAT. */
        uint64_t reserved_7_15         : 9;
        uint64_t inc                   : 1;  /**< [ 16: 16](R/W) Automatically advance [BUF_NUM]/[OFFSET] after each access to
                                                                 MIO_EMM_BUF_DAT. Wraps after the last offset of the last data buffer. */
        uint64_t reserved_17_63        : 47;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_buf_idx_s cn; */
};
typedef union cavm_mio_emm_buf_idx cavm_mio_emm_buf_idx_t;

#define CAVM_MIO_EMM_BUF_IDX CAVM_MIO_EMM_BUF_IDX_FUNC()
static inline uint64_t CAVM_MIO_EMM_BUF_IDX_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_BUF_IDX_FUNC(void)
{
    return 0x87e0090020e0ll;
}

#define typedef_CAVM_MIO_EMM_BUF_IDX cavm_mio_emm_buf_idx_t
#define bustype_CAVM_MIO_EMM_BUF_IDX CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_BUF_IDX "MIO_EMM_BUF_IDX"
#define device_bar_CAVM_MIO_EMM_BUF_IDX 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_BUF_IDX 0
#define arguments_CAVM_MIO_EMM_BUF_IDX -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_cfg
 *
 * eMMC Configuration Register
 */
union cavm_mio_emm_cfg
{
    uint64_t u;
    struct cavm_mio_emm_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t bus_ena               : 4;  /**< [  3:  0](R/W) eMMC bus enable mask.

                                                                 Setting bits 0..2 enable the corresponding EMMC bus by allowing EMMC_CMD[BUS_ENA]
                                                                 operation. Bit 3 is reserved.

                                                                 Clearing all bits of this field will reset the other MIO_EMM_* registers.
                                                                 To ensure a proper reset the BUS_ENA bits should be cleared for a minimum of 2 EMMC_CLK
                                                                 periods.  This period be determined by waiting twice the number of coprocessor clocks
                                                                 specified in MIO_EMM_MODE0[CLK_HI] and MIO_EMM_MODE0[CLK_LO].

                                                                 Setting one or more bits will enable EMMC_CLK operation. */
#else /* Word 0 - Little Endian */
        uint64_t bus_ena               : 4;  /**< [  3:  0](R/W) eMMC bus enable mask.

                                                                 Setting bits 0..2 enable the corresponding EMMC bus by allowing EMMC_CMD[BUS_ENA]
                                                                 operation. Bit 3 is reserved.

                                                                 Clearing all bits of this field will reset the other MIO_EMM_* registers.
                                                                 To ensure a proper reset the BUS_ENA bits should be cleared for a minimum of 2 EMMC_CLK
                                                                 periods.  This period be determined by waiting twice the number of coprocessor clocks
                                                                 specified in MIO_EMM_MODE0[CLK_HI] and MIO_EMM_MODE0[CLK_LO].

                                                                 Setting one or more bits will enable EMMC_CLK operation. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_cfg_s cn; */
};
typedef union cavm_mio_emm_cfg cavm_mio_emm_cfg_t;

#define CAVM_MIO_EMM_CFG CAVM_MIO_EMM_CFG_FUNC()
static inline uint64_t CAVM_MIO_EMM_CFG_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_CFG_FUNC(void)
{
    return 0x87e009002000ll;
}

#define typedef_CAVM_MIO_EMM_CFG cavm_mio_emm_cfg_t
#define bustype_CAVM_MIO_EMM_CFG CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_CFG "MIO_EMM_CFG"
#define device_bar_CAVM_MIO_EMM_CFG 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_CFG 0
#define arguments_CAVM_MIO_EMM_CFG -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_cmd
 *
 * eMMC Command Register
 */
union cavm_mio_emm_cmd
{
    uint64_t u;
    struct cavm_mio_emm_cmd_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_63           : 1;
        uint64_t skip_busy             : 1;  /**< [ 62: 62](R/W) Controls when command is completed.
                                                                 0 = Command doesn't complete until card has dropped the BUSY signal.
                                                                 1 = Complete command regardless of the BUSY signal. Status of signal can be read in
                                                                 MIO_EMM_RSP_STS[RSP_BUSYBIT]. */
        uint64_t bus_id                : 2;  /**< [ 61: 60](R/W) Specify the eMMC bus */
        uint64_t cmd_val               : 1;  /**< [ 59: 59](R/W/H) Request valid. Software writes this bit to a 1. Hardware clears it when the operation completes. */
        uint64_t reserved_56_58        : 3;
        uint64_t dbuf                  : 1;  /**< [ 55: 55](R/W) Specify the data buffer to be used for a block transfer. */
        uint64_t offset                : 6;  /**< [ 54: 49](R/W/H) Debug only. Specify the number of 8-byte transfers used in the command. Value is
                                                                 64-OFFSET. The block transfer still starts at the first byte in the 512 B data buffer.
                                                                 Software must ensure CMD16 has updated the card block length. */
        uint64_t reserved_43_48        : 6;
        uint64_t ctype_xor             : 2;  /**< [ 42: 41](R/W) Command type override; typically zero. Value is XOR'd with the default command type. See
                                                                 Command and Response Types for command types per command index. Types are:
                                                                 0x0 = No data.
                                                                 0x1 = Read data into Dbuf.
                                                                 0x2 = Write data from Dbuf.
                                                                 0x3 = Reserved. */
        uint64_t rtype_xor             : 3;  /**< [ 40: 38](R/W) Response type override; typically zero. Value is XOR'd with default response type. See
                                                                 Command and Response Types for response types per command index. Types are:
                                                                 0x0 = No Response.
                                                                 0x1 = R1, 48 bits.
                                                                 0x2 = R2, 136 bits.
                                                                 0x3 = R3, 48 bits.
                                                                 0x4 = R4, 48 bits.
                                                                 0x5 = R5, 48 bits.
                                                                 0x6, 0x7 = Reserved. */
        uint64_t cmd_idx               : 6;  /**< [ 37: 32](R/W/H) eMMC command */
        uint64_t arg                   : 32; /**< [ 31:  0](R/W/H) eMMC command argument */
#else /* Word 0 - Little Endian */
        uint64_t arg                   : 32; /**< [ 31:  0](R/W/H) eMMC command argument */
        uint64_t cmd_idx               : 6;  /**< [ 37: 32](R/W/H) eMMC command */
        uint64_t rtype_xor             : 3;  /**< [ 40: 38](R/W) Response type override; typically zero. Value is XOR'd with default response type. See
                                                                 Command and Response Types for response types per command index. Types are:
                                                                 0x0 = No Response.
                                                                 0x1 = R1, 48 bits.
                                                                 0x2 = R2, 136 bits.
                                                                 0x3 = R3, 48 bits.
                                                                 0x4 = R4, 48 bits.
                                                                 0x5 = R5, 48 bits.
                                                                 0x6, 0x7 = Reserved. */
        uint64_t ctype_xor             : 2;  /**< [ 42: 41](R/W) Command type override; typically zero. Value is XOR'd with the default command type. See
                                                                 Command and Response Types for command types per command index. Types are:
                                                                 0x0 = No data.
                                                                 0x1 = Read data into Dbuf.
                                                                 0x2 = Write data from Dbuf.
                                                                 0x3 = Reserved. */
        uint64_t reserved_43_48        : 6;
        uint64_t offset                : 6;  /**< [ 54: 49](R/W/H) Debug only. Specify the number of 8-byte transfers used in the command. Value is
                                                                 64-OFFSET. The block transfer still starts at the first byte in the 512 B data buffer.
                                                                 Software must ensure CMD16 has updated the card block length. */
        uint64_t dbuf                  : 1;  /**< [ 55: 55](R/W) Specify the data buffer to be used for a block transfer. */
        uint64_t reserved_56_58        : 3;
        uint64_t cmd_val               : 1;  /**< [ 59: 59](R/W/H) Request valid. Software writes this bit to a 1. Hardware clears it when the operation completes. */
        uint64_t bus_id                : 2;  /**< [ 61: 60](R/W) Specify the eMMC bus */
        uint64_t skip_busy             : 1;  /**< [ 62: 62](R/W) Controls when command is completed.
                                                                 0 = Command doesn't complete until card has dropped the BUSY signal.
                                                                 1 = Complete command regardless of the BUSY signal. Status of signal can be read in
                                                                 MIO_EMM_RSP_STS[RSP_BUSYBIT]. */
        uint64_t reserved_63           : 1;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_cmd_s cn; */
};
typedef union cavm_mio_emm_cmd cavm_mio_emm_cmd_t;

#define CAVM_MIO_EMM_CMD CAVM_MIO_EMM_CMD_FUNC()
static inline uint64_t CAVM_MIO_EMM_CMD_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_CMD_FUNC(void)
{
    return 0x87e009002058ll;
}

#define typedef_CAVM_MIO_EMM_CMD cavm_mio_emm_cmd_t
#define bustype_CAVM_MIO_EMM_CMD CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_CMD "MIO_EMM_CMD"
#define device_bar_CAVM_MIO_EMM_CMD 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_CMD 0
#define arguments_CAVM_MIO_EMM_CMD -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_dma
 *
 * eMMC External DMA Configuration Register
 */
union cavm_mio_emm_dma
{
    uint64_t u;
    struct cavm_mio_emm_dma_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_63           : 1;
        uint64_t skip_busy             : 1;  /**< [ 62: 62](R/W) Controls when DMA is completed.
                                                                 0 = DMA doesn't complete until card has dropped the BUSY signal.
                                                                 1 = Complete DMA after last transfer regardless of the BUSY signal. Status of signal can
                                                                 be read in MIO_EMM_RSP_STS[RSP_BUSYBIT]. */
        uint64_t bus_id                : 2;  /**< [ 61: 60](R/W) Specify the eMMC bus */
        uint64_t dma_val               : 1;  /**< [ 59: 59](R/W/H) Software writes this bit to a 1 to indicate that hardware should perform the DMA transfer.
                                                                 Hardware clears this bit when the DMA operation completes or is terminated. */
        uint64_t sector                : 1;  /**< [ 58: 58](R/W/H) Specify CARD_ADDR and eMMC are using sector (512 B) addressing. */
        uint64_t dat_null              : 1;  /**< [ 57: 57](R/W) Do not perform any eMMC commands. A DMA read returns all 0s. A DMA write tosses the data.
                                                                 In the case of a failure, this can be used to unwind the DMA engine. */
        uint64_t thres                 : 6;  /**< [ 56: 51](R/W) Number of 8-byte blocks of data that must exist in the DBUF before starting the 512-byte
                                                                 block transfer. Zero indicates to wait for the entire block. */
        uint64_t rel_wr                : 1;  /**< [ 50: 50](R/W) Set the reliable write parameter when performing CMD23 (SET_BLOCK_COUNT) for a multiple block. */
        uint64_t rw                    : 1;  /**< [ 49: 49](R/W) Read/write bit (0 = read, 1 = write). */
        uint64_t multi                 : 1;  /**< [ 48: 48](R/W) Perform operation using a multiple block command instead of a series of single block commands. */
        uint64_t block_cnt             : 16; /**< [ 47: 32](R/W/H) Number of blocks to read/write. Hardware decrements the block count after each successful
                                                                 block transfer. */
        uint64_t card_addr             : 32; /**< [ 31:  0](R/W/H) Data address for media <= 2 GB is a 32-bit byte address, and data address for media > 2 GB
                                                                 is a 32-bit sector (512 B) address. Hardware advances the card address after each
                                                                 successful block transfer by 512 for byte addressing and by 1 for sector addressing. */
#else /* Word 0 - Little Endian */
        uint64_t card_addr             : 32; /**< [ 31:  0](R/W/H) Data address for media <= 2 GB is a 32-bit byte address, and data address for media > 2 GB
                                                                 is a 32-bit sector (512 B) address. Hardware advances the card address after each
                                                                 successful block transfer by 512 for byte addressing and by 1 for sector addressing. */
        uint64_t block_cnt             : 16; /**< [ 47: 32](R/W/H) Number of blocks to read/write. Hardware decrements the block count after each successful
                                                                 block transfer. */
        uint64_t multi                 : 1;  /**< [ 48: 48](R/W) Perform operation using a multiple block command instead of a series of single block commands. */
        uint64_t rw                    : 1;  /**< [ 49: 49](R/W) Read/write bit (0 = read, 1 = write). */
        uint64_t rel_wr                : 1;  /**< [ 50: 50](R/W) Set the reliable write parameter when performing CMD23 (SET_BLOCK_COUNT) for a multiple block. */
        uint64_t thres                 : 6;  /**< [ 56: 51](R/W) Number of 8-byte blocks of data that must exist in the DBUF before starting the 512-byte
                                                                 block transfer. Zero indicates to wait for the entire block. */
        uint64_t dat_null              : 1;  /**< [ 57: 57](R/W) Do not perform any eMMC commands. A DMA read returns all 0s. A DMA write tosses the data.
                                                                 In the case of a failure, this can be used to unwind the DMA engine. */
        uint64_t sector                : 1;  /**< [ 58: 58](R/W/H) Specify CARD_ADDR and eMMC are using sector (512 B) addressing. */
        uint64_t dma_val               : 1;  /**< [ 59: 59](R/W/H) Software writes this bit to a 1 to indicate that hardware should perform the DMA transfer.
                                                                 Hardware clears this bit when the DMA operation completes or is terminated. */
        uint64_t bus_id                : 2;  /**< [ 61: 60](R/W) Specify the eMMC bus */
        uint64_t skip_busy             : 1;  /**< [ 62: 62](R/W) Controls when DMA is completed.
                                                                 0 = DMA doesn't complete until card has dropped the BUSY signal.
                                                                 1 = Complete DMA after last transfer regardless of the BUSY signal. Status of signal can
                                                                 be read in MIO_EMM_RSP_STS[RSP_BUSYBIT]. */
        uint64_t reserved_63           : 1;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_dma_s cn; */
};
typedef union cavm_mio_emm_dma cavm_mio_emm_dma_t;

#define CAVM_MIO_EMM_DMA CAVM_MIO_EMM_DMA_FUNC()
static inline uint64_t CAVM_MIO_EMM_DMA_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_DMA_FUNC(void)
{
    return 0x87e009002050ll;
}

#define typedef_CAVM_MIO_EMM_DMA cavm_mio_emm_dma_t
#define bustype_CAVM_MIO_EMM_DMA CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_DMA "MIO_EMM_DMA"
#define device_bar_CAVM_MIO_EMM_DMA 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_DMA 0
#define arguments_CAVM_MIO_EMM_DMA -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_dma_adr
 *
 * eMMC DMA Address Register
 * This register sets the address for eMMC/SD flash transfers to/from memory. Sixty-four-bit
 * operations must be used to access this register. This register is updated by the DMA
 * hardware and can be reloaded by the values placed in the MIO_EMM_DMA_FIFO_ADR.
 */
union cavm_mio_emm_dma_adr
{
    uint64_t u;
    struct cavm_mio_emm_dma_adr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_49_63        : 15;
        uint64_t adr                   : 49; /**< [ 48:  0](R/W/H) DMA engine IOVA. Must be 64-bit aligned. */
#else /* Word 0 - Little Endian */
        uint64_t adr                   : 49; /**< [ 48:  0](R/W/H) DMA engine IOVA. Must be 64-bit aligned. */
        uint64_t reserved_49_63        : 15;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_dma_adr_s cn; */
};
typedef union cavm_mio_emm_dma_adr cavm_mio_emm_dma_adr_t;

#define CAVM_MIO_EMM_DMA_ADR CAVM_MIO_EMM_DMA_ADR_FUNC()
static inline uint64_t CAVM_MIO_EMM_DMA_ADR_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_DMA_ADR_FUNC(void)
{
    return 0x87e009000188ll;
}

#define typedef_CAVM_MIO_EMM_DMA_ADR cavm_mio_emm_dma_adr_t
#define bustype_CAVM_MIO_EMM_DMA_ADR CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_DMA_ADR "MIO_EMM_DMA_ADR"
#define device_bar_CAVM_MIO_EMM_DMA_ADR 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_DMA_ADR 0
#define arguments_CAVM_MIO_EMM_DMA_ADR -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_dma_cfg
 *
 * eMMC DMA Configuration Register
 * This register controls the internal DMA engine used with the eMMC/SD flash controller. Sixty-
 * four-bit operations must be used to access this register. This register is updated by the
 * hardware DMA engine and can also be reloaded by writes to the MIO_EMM_DMA_FIFO_CMD register.
 */
union cavm_mio_emm_dma_cfg
{
    uint64_t u;
    struct cavm_mio_emm_dma_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t en                    : 1;  /**< [ 63: 63](R/W/H) DMA engine enable. */
        uint64_t rw                    : 1;  /**< [ 62: 62](R/W/H) DMA engine R/W bit: 0 = read, 1 = write. */
        uint64_t clr                   : 1;  /**< [ 61: 61](R/W/H) DMA engine abort. When set to 1, DMA is aborted and EN is cleared on completion. */
        uint64_t reserved_60           : 1;
        uint64_t swap32                : 1;  /**< [ 59: 59](R/W/H) DMA engine 32-bit swap. */
        uint64_t swap16                : 1;  /**< [ 58: 58](R/W/H) DMA engine enable 16-bit swap. */
        uint64_t swap8                 : 1;  /**< [ 57: 57](R/W/H) DMA engine enable 8-bit swap. */
        uint64_t endian                : 1;  /**< [ 56: 56](R/W/H) DMA engine endian mode: 0 = big-endian, 1 = little-endian. */
        uint64_t size                  : 20; /**< [ 55: 36](R/W/H) DMA engine size. Specified in the number of 64-bit transfers (encoded in -1 notation). For
                                                                 example, to transfer 512 bytes, SIZE = 64 - 1 = 63. */
        uint64_t reserved_0_35         : 36;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_35         : 36;
        uint64_t size                  : 20; /**< [ 55: 36](R/W/H) DMA engine size. Specified in the number of 64-bit transfers (encoded in -1 notation). For
                                                                 example, to transfer 512 bytes, SIZE = 64 - 1 = 63. */
        uint64_t endian                : 1;  /**< [ 56: 56](R/W/H) DMA engine endian mode: 0 = big-endian, 1 = little-endian. */
        uint64_t swap8                 : 1;  /**< [ 57: 57](R/W/H) DMA engine enable 8-bit swap. */
        uint64_t swap16                : 1;  /**< [ 58: 58](R/W/H) DMA engine enable 16-bit swap. */
        uint64_t swap32                : 1;  /**< [ 59: 59](R/W/H) DMA engine 32-bit swap. */
        uint64_t reserved_60           : 1;
        uint64_t clr                   : 1;  /**< [ 61: 61](R/W/H) DMA engine abort. When set to 1, DMA is aborted and EN is cleared on completion. */
        uint64_t rw                    : 1;  /**< [ 62: 62](R/W/H) DMA engine R/W bit: 0 = read, 1 = write. */
        uint64_t en                    : 1;  /**< [ 63: 63](R/W/H) DMA engine enable. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_dma_cfg_s cn; */
};
typedef union cavm_mio_emm_dma_cfg cavm_mio_emm_dma_cfg_t;

#define CAVM_MIO_EMM_DMA_CFG CAVM_MIO_EMM_DMA_CFG_FUNC()
static inline uint64_t CAVM_MIO_EMM_DMA_CFG_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_DMA_CFG_FUNC(void)
{
    return 0x87e009000180ll;
}

#define typedef_CAVM_MIO_EMM_DMA_CFG cavm_mio_emm_dma_cfg_t
#define bustype_CAVM_MIO_EMM_DMA_CFG CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_DMA_CFG "MIO_EMM_DMA_CFG"
#define device_bar_CAVM_MIO_EMM_DMA_CFG 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_DMA_CFG 0
#define arguments_CAVM_MIO_EMM_DMA_CFG -1,-1,-1,-1

/**
 * Register (RSL) mio_emm_rsp_sts
 *
 * eMMC Response Status Register
 */
union cavm_mio_emm_rsp_sts
{
    uint64_t u;
    struct cavm_mio_emm_rsp_sts_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_62_63        : 2;
        uint64_t bus_id                : 2;  /**< [ 61: 60](RO/H) eMMC bus ID to which the response status corresponds. */
        uint64_t cmd_val               : 1;  /**< [ 59: 59](RO/H) Read-only copy of MIO_EMM_CMD[CMD_VAL]. [CMD_VAL] = 1 indicates that a direct operation is
                                                                 in progress. */
        uint64_t switch_val            : 1;  /**< [ 58: 58](RO/H) Read-only copy of MIO_EMM_SWITCH[SWITCH_EXE]. [SWITCH_VAL] = 1 indicates that a switch
                                                                 operation is in progress. */
        uint64_t dma_val               : 1;  /**< [ 57: 57](RO/H) Read-only copy of MIO_EMM_DMA[DMA_VAL]. [DMA_VAL] = 1 indicates that a DMA operation is in
                                                                 progress. */
        uint64_t dma_pend              : 1;  /**< [ 56: 56](RO/H) The DMA engine has a pending transfer resulting from an error. Software can resume the
                                                                 transfer by writing MIO_EMM_DMA[DMA_VAL] = 1.

                                                                 Software can terminate the transfer by writing MIO_EMM_DMA[DMA_VAL] = 1 and
                                                                 MIO_EMM_DMA[DAT_NULL] = 1. Hardware will then clear [DMA_PEND] and perform the DMA
                                                                 operation. */
        uint64_t acc_timeout           : 1;  /**< [ 55: 55](RO/H) The store operation to the device took longer than MIO_EMM_ACCESS_WDOG[CLK_CNT]
                                                                 coprocessor-clocks to complete.
                                                                 Valid when [DMA_PEND] is set. */
        uint64_t reserved_29_54        : 26;
        uint64_t dbuf_err              : 1;  /**< [ 28: 28](RO/H) For [CMD_TYPE] = 1, indicates that a DMA read data arrived from the card without a free
                                                                 DBUF. For [CMD_TYPE] = 2, indicates a DBUF underflow occurred during a DMA write. See
                                                                 MIO_EMM_DMA[THRES]. */
        uint64_t reserved_24_27        : 4;
        uint64_t dbuf                  : 1;  /**< [ 23: 23](RO/H) DBUF corresponding to the most recently attempted block transfer. */
        uint64_t blk_timeout           : 1;  /**< [ 22: 22](RO/H) Timeout waiting for read data or 3-bit CRC token. */
        uint64_t blk_crc_err           : 1;  /**< [ 21: 21](RO/H) For [CMD_TYPE] = 1, indicates a card read data CRC mismatch. MIO_EMM_RSP_STS[DBUF]
                                                                 indicates
                                                                 the failing data buffer.

                                                                 For [CMD_TYPE] = 2, indicates card returned 3-bit CRC status token indicating that the
                                                                 card
                                                                 encountered a write data CRC check mismatch. MIO_EMM_RSP_STS[DBUF] indicates the failing
                                                                 data buffer. */
        uint64_t rsp_busybit           : 1;  /**< [ 20: 20](RO/H) Debug only. eMMC protocol utilizes DAT0 as a busy signal during block writes and R1b
                                                                 responses. This bit should read zero before any DMA or Command with data is executed. */
        uint64_t stp_timeout           : 1;  /**< [ 19: 19](RO/H) Stop transmission response timeout. */
        uint64_t stp_crc_err           : 1;  /**< [ 18: 18](RO/H) Stop transmission response had a CRC error. */
        uint64_t stp_bad_sts           : 1;  /**< [ 17: 17](RO/H) Stop transmission response had bad status. */
        uint64_t stp_val               : 1;  /**< [ 16: 16](RO/H) Stop transmission response valid. */
        uint64_t rsp_timeout           : 1;  /**< [ 15: 15](RO/H) Response timeout. */
        uint64_t rsp_crc_err           : 1;  /**< [ 14: 14](RO/H) Response CRC error. */
        uint64_t rsp_bad_sts           : 1;  /**< [ 13: 13](RO/H) Response bad status. */
        uint64_t rsp_val               : 1;  /**< [ 12: 12](RO/H) Response ID. See MIO_EMM_RSP_HI/MIO_EMM_RSP_LO. */
        uint64_t rsp_type              : 3;  /**< [ 11:  9](RO/H) Indicates the response type. See MIO_EMM_RSP_HI/MIO_EMM_RSP_LO. */
        uint64_t cmd_type              : 2;  /**< [  8:  7](RO/H) eMMC command type.
                                                                 0x0 = No data.
                                                                 0x1 = Read.
                                                                 0x2 = Write. */
        uint64_t cmd_idx               : 6;  /**< [  6:  1](RO/H) eMMC command index most recently attempted. */
        uint64_t cmd_done              : 1;  /**< [  0:  0](RO/H) eMMC command completed. Once the command has completed, the status is final and can be
                                                                 examined by software. */
#else /* Word 0 - Little Endian */
        uint64_t cmd_done              : 1;  /**< [  0:  0](RO/H) eMMC command completed. Once the command has completed, the status is final and can be
                                                                 examined by software. */
        uint64_t cmd_idx               : 6;  /**< [  6:  1](RO/H) eMMC command index most recently attempted. */
        uint64_t cmd_type              : 2;  /**< [  8:  7](RO/H) eMMC command type.
                                                                 0x0 = No data.
                                                                 0x1 = Read.
                                                                 0x2 = Write. */
        uint64_t rsp_type              : 3;  /**< [ 11:  9](RO/H) Indicates the response type. See MIO_EMM_RSP_HI/MIO_EMM_RSP_LO. */
        uint64_t rsp_val               : 1;  /**< [ 12: 12](RO/H) Response ID. See MIO_EMM_RSP_HI/MIO_EMM_RSP_LO. */
        uint64_t rsp_bad_sts           : 1;  /**< [ 13: 13](RO/H) Response bad status. */
        uint64_t rsp_crc_err           : 1;  /**< [ 14: 14](RO/H) Response CRC error. */
        uint64_t rsp_timeout           : 1;  /**< [ 15: 15](RO/H) Response timeout. */
        uint64_t stp_val               : 1;  /**< [ 16: 16](RO/H) Stop transmission response valid. */
        uint64_t stp_bad_sts           : 1;  /**< [ 17: 17](RO/H) Stop transmission response had bad status. */
        uint64_t stp_crc_err           : 1;  /**< [ 18: 18](RO/H) Stop transmission response had a CRC error. */
        uint64_t stp_timeout           : 1;  /**< [ 19: 19](RO/H) Stop transmission response timeout. */
        uint64_t rsp_busybit           : 1;  /**< [ 20: 20](RO/H) Debug only. eMMC protocol utilizes DAT0 as a busy signal during block writes and R1b
                                                                 responses. This bit should read zero before any DMA or Command with data is executed. */
        uint64_t blk_crc_err           : 1;  /**< [ 21: 21](RO/H) For [CMD_TYPE] = 1, indicates a card read data CRC mismatch. MIO_EMM_RSP_STS[DBUF]
                                                                 indicates
                                                                 the failing data buffer.

                                                                 For [CMD_TYPE] = 2, indicates card returned 3-bit CRC status token indicating that the
                                                                 card
                                                                 encountered a write data CRC check mismatch. MIO_EMM_RSP_STS[DBUF] indicates the failing
                                                                 data buffer. */
        uint64_t blk_timeout           : 1;  /**< [ 22: 22](RO/H) Timeout waiting for read data or 3-bit CRC token. */
        uint64_t dbuf                  : 1;  /**< [ 23: 23](RO/H) DBUF corresponding to the most recently attempted block transfer. */
        uint64_t reserved_24_27        : 4;
        uint64_t dbuf_err              : 1;  /**< [ 28: 28](RO/H) For [CMD_TYPE] = 1, indicates that a DMA read data arrived from the card without a free
                                                                 DBUF. For [CMD_TYPE] = 2, indicates a DBUF underflow occurred during a DMA write. See
                                                                 MIO_EMM_DMA[THRES]. */
        uint64_t reserved_29_54        : 26;
        uint64_t acc_timeout           : 1;  /**< [ 55: 55](RO/H) The store operation to the device took longer than MIO_EMM_ACCESS_WDOG[CLK_CNT]
                                                                 coprocessor-clocks to complete.
                                                                 Valid when [DMA_PEND] is set. */
        uint64_t dma_pend              : 1;  /**< [ 56: 56](RO/H) The DMA engine has a pending transfer resulting from an error. Software can resume the
                                                                 transfer by writing MIO_EMM_DMA[DMA_VAL] = 1.

                                                                 Software can terminate the transfer by writing MIO_EMM_DMA[DMA_VAL] = 1 and
                                                                 MIO_EMM_DMA[DAT_NULL] = 1. Hardware will then clear [DMA_PEND] and perform the DMA
                                                                 operation. */
        uint64_t dma_val               : 1;  /**< [ 57: 57](RO/H) Read-only copy of MIO_EMM_DMA[DMA_VAL]. [DMA_VAL] = 1 indicates that a DMA operation is in
                                                                 progress. */
        uint64_t switch_val            : 1;  /**< [ 58: 58](RO/H) Read-only copy of MIO_EMM_SWITCH[SWITCH_EXE]. [SWITCH_VAL] = 1 indicates that a switch
                                                                 operation is in progress. */
        uint64_t cmd_val               : 1;  /**< [ 59: 59](RO/H) Read-only copy of MIO_EMM_CMD[CMD_VAL]. [CMD_VAL] = 1 indicates that a direct operation is
                                                                 in progress. */
        uint64_t bus_id                : 2;  /**< [ 61: 60](RO/H) eMMC bus ID to which the response status corresponds. */
        uint64_t reserved_62_63        : 2;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_mio_emm_rsp_sts_s cn; */
};
typedef union cavm_mio_emm_rsp_sts cavm_mio_emm_rsp_sts_t;

#define CAVM_MIO_EMM_RSP_STS CAVM_MIO_EMM_RSP_STS_FUNC()
static inline uint64_t CAVM_MIO_EMM_RSP_STS_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_MIO_EMM_RSP_STS_FUNC(void)
{
    return 0x87e009002060ll;
}

#define typedef_CAVM_MIO_EMM_RSP_STS cavm_mio_emm_rsp_sts_t
#define bustype_CAVM_MIO_EMM_RSP_STS CSR_TYPE_RSL
#define basename_CAVM_MIO_EMM_RSP_STS "MIO_EMM_RSP_STS"
#define device_bar_CAVM_MIO_EMM_RSP_STS 0x0 /* PF_BAR0 */
#define busnum_CAVM_MIO_EMM_RSP_STS 0
#define arguments_CAVM_MIO_EMM_RSP_STS -1,-1,-1,-1

#endif /* __CAVM_CSRS_MIO_EMM_H__ */

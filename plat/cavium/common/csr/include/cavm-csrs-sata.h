#ifndef __CAVM_CSRS_SATA_H__
#define __CAVM_CSRS_SATA_H__
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
 * Cavium SATA.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration sata_bar_e
 *
 * SATA Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_SATA_BAR_E_SATAX_PF_BAR0(a) (0x810000000000ll + 0x1000000000ll * (a))
#define CAVM_SATA_BAR_E_SATAX_PF_BAR0_SIZE 0x200000ull
#define CAVM_SATA_BAR_E_SATAX_PF_BAR4(a) (0x810000200000ll + 0x1000000000ll * (a))
#define CAVM_SATA_BAR_E_SATAX_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration sata_int_vec_e
 *
 * SATA MSI-X Vector Enumeration
 * Enumerates the MSI-X interrupt vectors.
 */
#define CAVM_SATA_INT_VEC_E_UAHC_INTRQ_IP (0)
#define CAVM_SATA_INT_VEC_E_UAHC_INTRQ_IP_CLEAR (1)
#define CAVM_SATA_INT_VEC_E_UAHC_PME_REQ_IP (2)
#define CAVM_SATA_INT_VEC_E_UAHC_PME_REQ_IP_CLEAR (3)
#define CAVM_SATA_INT_VEC_E_UCTL_INTSTAT_CN88XXP1 (4)
#define CAVM_SATA_INT_VEC_E_UCTL_INTSTAT_CN81XX (1)
#define CAVM_SATA_INT_VEC_E_UCTL_INTSTAT_CN83XX (1)
#define CAVM_SATA_INT_VEC_E_UCTL_INTSTAT_CN88XXP2 (1)

/**
 * Enumeration sata_uctl_dma_read_cmd_e
 *
 * SATA UCTL DMA Read Command Enumeration
 * Enumerates NCB inbound command selections for DMA read operations.
 */
#define CAVM_SATA_UCTL_DMA_READ_CMD_E_LDI (0)
#define CAVM_SATA_UCTL_DMA_READ_CMD_E_LDT (1)
#define CAVM_SATA_UCTL_DMA_READ_CMD_E_LDY (2)

/**
 * Enumeration sata_uctl_dma_write_cmd_e
 *
 * SATA UCTL DMA Write Command Enumeration
 * Enumerate NCB inbound command selections for DMA writes.
 */
#define CAVM_SATA_UCTL_DMA_WRITE_CMD_E_RSTP (1)
#define CAVM_SATA_UCTL_DMA_WRITE_CMD_E_STP (0)

/**
 * Enumeration sata_uctl_ecc_err_source_e
 *
 * SATA UCTL ECC Error Source Enumeration
 * Enumerate sources of ECC error log information.
 */
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_FB_DBE (0xf)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_FB_SBE (7)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_NONE (0)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_RX_DBE (0xd)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_RX_SBE (5)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_TX_DBE (0xe)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_TX_SBE (6)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_XM_R_DBE (0xa)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_XM_R_SBE (2)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_XM_W_DBE (9)
#define CAVM_SATA_UCTL_ECC_ERR_SOURCE_E_XM_W_SBE (1)

/**
 * Enumeration sata_uctl_xm_bad_dma_type_e
 *
 * SATA UCTL XM Bad DMA Type Enumeration
 * Enumerates the type of DMA error seen.
 */
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_ADDR_OOB (1)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_LEN_GT_8 (2)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_MULTIBEAT_BYTE (3)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_MULTIBEAT_HALFWORD (4)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_MULTIBEAT_QWORD (6)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_MULTIBEAT_WORD (5)
#define CAVM_SATA_UCTL_XM_BAD_DMA_TYPE_E_NONE (0)

/**
 * Register (NCB32b) sata#_uahc_gbl_ghc
 *
 * SATA AHCI Global HBA Control Register
 * See AHCI specification v1.3 section 3.1.
 */
union cavm_satax_uahc_gbl_ghc
{
    uint32_t u;
    struct cavm_satax_uahc_gbl_ghc_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t ae                    : 1;  /**< [ 31: 31](RO) AHCI enable. */
        uint32_t reserved_2_30         : 29;
        uint32_t ie                    : 1;  /**< [  1:  1](R/W) Interrupt enable. */
        uint32_t hr                    : 1;  /**< [  0:  0](R/W1/H) HBA reset. Writing a 1 resets the UAHC. Hardware clears this bit once reset is complete. */
#else /* Word 0 - Little Endian */
        uint32_t hr                    : 1;  /**< [  0:  0](R/W1/H) HBA reset. Writing a 1 resets the UAHC. Hardware clears this bit once reset is complete. */
        uint32_t ie                    : 1;  /**< [  1:  1](R/W) Interrupt enable. */
        uint32_t reserved_2_30         : 29;
        uint32_t ae                    : 1;  /**< [ 31: 31](RO) AHCI enable. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_gbl_ghc_s cn; */
};
typedef union cavm_satax_uahc_gbl_ghc cavm_satax_uahc_gbl_ghc_t;

static inline uint64_t CAVM_SATAX_UAHC_GBL_GHC(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_GBL_GHC(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000004ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000004ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000004ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_GBL_GHC", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_GBL_GHC(a) cavm_satax_uahc_gbl_ghc_t
#define bustype_CAVM_SATAX_UAHC_GBL_GHC(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_GBL_GHC(a) "SATAX_UAHC_GBL_GHC"
#define device_bar_CAVM_SATAX_UAHC_GBL_GHC(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_GBL_GHC(a) (a)
#define arguments_CAVM_SATAX_UAHC_GBL_GHC(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_ci
 *
 * SATA UAHC Command Issue Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_ci
{
    uint32_t u;
    struct cavm_satax_uahc_p0_ci_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t ci                    : 32; /**< [ 31:  0](R/W1S/H) Command issued. */
#else /* Word 0 - Little Endian */
        uint32_t ci                    : 32; /**< [ 31:  0](R/W1S/H) Command issued. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_ci_s cn; */
};
typedef union cavm_satax_uahc_p0_ci cavm_satax_uahc_p0_ci_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_CI(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_CI(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000138ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000138ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000138ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_CI", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_CI(a) cavm_satax_uahc_p0_ci_t
#define bustype_CAVM_SATAX_UAHC_P0_CI(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_CI(a) "SATAX_UAHC_P0_CI"
#define device_bar_CAVM_SATAX_UAHC_P0_CI(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_CI(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_CI(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uahc_p0_clb
 *
 * SATA UAHC Command-List Base-Address Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_clb
{
    uint64_t u;
    struct cavm_satax_uahc_p0_clb_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t clb                   : 54; /**< [ 63: 10](R/W) Command-list base address. */
        uint64_t reserved_0_9          : 10;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_9          : 10;
        uint64_t clb                   : 54; /**< [ 63: 10](R/W) Command-list base address. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_clb_s cn; */
};
typedef union cavm_satax_uahc_p0_clb cavm_satax_uahc_p0_clb_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_CLB(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_CLB(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000100ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000100ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000100ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_CLB", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_CLB(a) cavm_satax_uahc_p0_clb_t
#define bustype_CAVM_SATAX_UAHC_P0_CLB(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UAHC_P0_CLB(a) "SATAX_UAHC_P0_CLB"
#define device_bar_CAVM_SATAX_UAHC_P0_CLB(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_CLB(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_CLB(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_cmd
 *
 * SATA UAHC Command Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_cmd
{
    uint32_t u;
    struct cavm_satax_uahc_p0_cmd_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t icc                   : 4;  /**< [ 31: 28](R/W) Interface communication control. */
        uint32_t asp                   : 1;  /**< [ 27: 27](R/W) Aggressive slumber/partial. */
        uint32_t alpe                  : 1;  /**< [ 26: 26](R/W) Aggressive link-power-management enable. */
        uint32_t dlae                  : 1;  /**< [ 25: 25](R/W) Drive LED on ATAPI enable. */
        uint32_t atapi                 : 1;  /**< [ 24: 24](R/W) Device is ATAPI. */
        uint32_t apste                 : 1;  /**< [ 23: 23](R/W) Automatic partial to slumber transitions enable. */
        uint32_t fbscp                 : 1;  /**< [ 22: 22](R/W) FIS-based switching capable port. Write-once. */
        uint32_t esp                   : 1;  /**< [ 21: 21](R/W) External SATA port. Write-once. */
        uint32_t cpd                   : 1;  /**< [ 20: 20](R/W) Cold-presence detection. Write-once. */
        uint32_t mpsp                  : 1;  /**< [ 19: 19](R/W) Mechanical presence switch attached to port. Write-once. */
        uint32_t hpcp                  : 1;  /**< [ 18: 18](R/W) Hot-plug-capable support. Write-once. */
        uint32_t pma                   : 1;  /**< [ 17: 17](R/W) Port multiplier attached. */
        uint32_t cps                   : 1;  /**< [ 16: 16](RO) Cold presence state. */
        uint32_t cr                    : 1;  /**< [ 15: 15](RO) Command list running. */
        uint32_t fr                    : 1;  /**< [ 14: 14](RO/H) FIS receive running. */
        uint32_t mpss                  : 1;  /**< [ 13: 13](RO) Mechanical presence switch state. */
        uint32_t ccs                   : 5;  /**< [ 12:  8](RO) Current-command slot. */
        uint32_t reserved_5_7          : 3;
        uint32_t fre                   : 1;  /**< [  4:  4](R/W) FIS-receive enable. */
        uint32_t clo                   : 1;  /**< [  3:  3](WO) Command-list override. */
        uint32_t pod                   : 1;  /**< [  2:  2](R/W) Power-on device. R/W only if CPD = 1, else read only. */
        uint32_t sud                   : 1;  /**< [  1:  1](R/W) Spin-up device. R/W only if SATA()_UAHC_GBL_CAP[SSS]=1, else read only.
                                                                 Setting this bit triggers a COMRESET initialization sequence. */
        uint32_t st                    : 1;  /**< [  0:  0](R/W) Start. */
#else /* Word 0 - Little Endian */
        uint32_t st                    : 1;  /**< [  0:  0](R/W) Start. */
        uint32_t sud                   : 1;  /**< [  1:  1](R/W) Spin-up device. R/W only if SATA()_UAHC_GBL_CAP[SSS]=1, else read only.
                                                                 Setting this bit triggers a COMRESET initialization sequence. */
        uint32_t pod                   : 1;  /**< [  2:  2](R/W) Power-on device. R/W only if CPD = 1, else read only. */
        uint32_t clo                   : 1;  /**< [  3:  3](WO) Command-list override. */
        uint32_t fre                   : 1;  /**< [  4:  4](R/W) FIS-receive enable. */
        uint32_t reserved_5_7          : 3;
        uint32_t ccs                   : 5;  /**< [ 12:  8](RO) Current-command slot. */
        uint32_t mpss                  : 1;  /**< [ 13: 13](RO) Mechanical presence switch state. */
        uint32_t fr                    : 1;  /**< [ 14: 14](RO/H) FIS receive running. */
        uint32_t cr                    : 1;  /**< [ 15: 15](RO) Command list running. */
        uint32_t cps                   : 1;  /**< [ 16: 16](RO) Cold presence state. */
        uint32_t pma                   : 1;  /**< [ 17: 17](R/W) Port multiplier attached. */
        uint32_t hpcp                  : 1;  /**< [ 18: 18](R/W) Hot-plug-capable support. Write-once. */
        uint32_t mpsp                  : 1;  /**< [ 19: 19](R/W) Mechanical presence switch attached to port. Write-once. */
        uint32_t cpd                   : 1;  /**< [ 20: 20](R/W) Cold-presence detection. Write-once. */
        uint32_t esp                   : 1;  /**< [ 21: 21](R/W) External SATA port. Write-once. */
        uint32_t fbscp                 : 1;  /**< [ 22: 22](R/W) FIS-based switching capable port. Write-once. */
        uint32_t apste                 : 1;  /**< [ 23: 23](R/W) Automatic partial to slumber transitions enable. */
        uint32_t atapi                 : 1;  /**< [ 24: 24](R/W) Device is ATAPI. */
        uint32_t dlae                  : 1;  /**< [ 25: 25](R/W) Drive LED on ATAPI enable. */
        uint32_t alpe                  : 1;  /**< [ 26: 26](R/W) Aggressive link-power-management enable. */
        uint32_t asp                   : 1;  /**< [ 27: 27](R/W) Aggressive slumber/partial. */
        uint32_t icc                   : 4;  /**< [ 31: 28](R/W) Interface communication control. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_cmd_s cn; */
};
typedef union cavm_satax_uahc_p0_cmd cavm_satax_uahc_p0_cmd_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_CMD(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_CMD(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000118ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000118ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000118ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_CMD", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_CMD(a) cavm_satax_uahc_p0_cmd_t
#define bustype_CAVM_SATAX_UAHC_P0_CMD(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_CMD(a) "SATAX_UAHC_P0_CMD"
#define device_bar_CAVM_SATAX_UAHC_P0_CMD(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_CMD(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_CMD(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_dmacr
 *
 * SATA UAHC DMA Control Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_dmacr
{
    uint32_t u;
    struct cavm_satax_uahc_p0_dmacr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_8_31         : 24;
        uint32_t rxts                  : 4;  /**< [  7:  4](R/W) Receive transaction size. This field is R/W when SATA()_UAHC_P0_CMD[ST] = 0
                                                                 and read only when SATA()_UAHC_P0_CMD[ST] = 1. */
        uint32_t txts                  : 4;  /**< [  3:  0](R/W) Transmit transaction size. This field is R/W when SATA()_UAHC_P0_CMD[ST] = 0
                                                                 and read only when SATA()_UAHC_P0_CMD[ST] = 1. */
#else /* Word 0 - Little Endian */
        uint32_t txts                  : 4;  /**< [  3:  0](R/W) Transmit transaction size. This field is R/W when SATA()_UAHC_P0_CMD[ST] = 0
                                                                 and read only when SATA()_UAHC_P0_CMD[ST] = 1. */
        uint32_t rxts                  : 4;  /**< [  7:  4](R/W) Receive transaction size. This field is R/W when SATA()_UAHC_P0_CMD[ST] = 0
                                                                 and read only when SATA()_UAHC_P0_CMD[ST] = 1. */
        uint32_t reserved_8_31         : 24;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_dmacr_s cn; */
};
typedef union cavm_satax_uahc_p0_dmacr cavm_satax_uahc_p0_dmacr_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_DMACR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_DMACR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000170ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000170ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000170ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_DMACR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_DMACR(a) cavm_satax_uahc_p0_dmacr_t
#define bustype_CAVM_SATAX_UAHC_P0_DMACR(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_DMACR(a) "SATAX_UAHC_P0_DMACR"
#define device_bar_CAVM_SATAX_UAHC_P0_DMACR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_DMACR(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_DMACR(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uahc_p0_fb
 *
 * SATA UAHC FIS Base-Address Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_fb
{
    uint64_t u;
    struct cavm_satax_uahc_p0_fb_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t fb                    : 56; /**< [ 63:  8](R/W) FIS base address. */
        uint64_t reserved_0_7          : 8;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_7          : 8;
        uint64_t fb                    : 56; /**< [ 63:  8](R/W) FIS base address. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_fb_s cn; */
};
typedef union cavm_satax_uahc_p0_fb cavm_satax_uahc_p0_fb_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_FB(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_FB(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000108ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000108ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000108ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_FB", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_FB(a) cavm_satax_uahc_p0_fb_t
#define bustype_CAVM_SATAX_UAHC_P0_FB(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UAHC_P0_FB(a) "SATAX_UAHC_P0_FB"
#define device_bar_CAVM_SATAX_UAHC_P0_FB(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_FB(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_FB(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_fbs
 *
 * SATA UAHC FIS-Based Switching Control Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_fbs
{
    uint32_t u;
    struct cavm_satax_uahc_p0_fbs_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_20_31        : 12;
        uint32_t dwe                   : 4;  /**< [ 19: 16](RO) Device with error. */
        uint32_t ado                   : 4;  /**< [ 15: 12](RO) Active device optimization. */
        uint32_t dev                   : 4;  /**< [ 11:  8](R/W) Device to issue. */
        uint32_t reserved_3_7          : 5;
        uint32_t sde                   : 1;  /**< [  2:  2](RO) Single device error. */
        uint32_t dec                   : 1;  /**< [  1:  1](R/W1C/H) Device error clear. */
        uint32_t en                    : 1;  /**< [  0:  0](R/W) Enable. */
#else /* Word 0 - Little Endian */
        uint32_t en                    : 1;  /**< [  0:  0](R/W) Enable. */
        uint32_t dec                   : 1;  /**< [  1:  1](R/W1C/H) Device error clear. */
        uint32_t sde                   : 1;  /**< [  2:  2](RO) Single device error. */
        uint32_t reserved_3_7          : 5;
        uint32_t dev                   : 4;  /**< [ 11:  8](R/W) Device to issue. */
        uint32_t ado                   : 4;  /**< [ 15: 12](RO) Active device optimization. */
        uint32_t dwe                   : 4;  /**< [ 19: 16](RO) Device with error. */
        uint32_t reserved_20_31        : 12;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_fbs_s cn; */
};
typedef union cavm_satax_uahc_p0_fbs cavm_satax_uahc_p0_fbs_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_FBS(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_FBS(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000140ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000140ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000140ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_FBS", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_FBS(a) cavm_satax_uahc_p0_fbs_t
#define bustype_CAVM_SATAX_UAHC_P0_FBS(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_FBS(a) "SATAX_UAHC_P0_FBS"
#define device_bar_CAVM_SATAX_UAHC_P0_FBS(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_FBS(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_FBS(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_ie
 *
 * SATA UAHC Interrupt Enable Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_ie
{
    uint32_t u;
    struct cavm_satax_uahc_p0_ie_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t cpde                  : 1;  /**< [ 31: 31](R/W) Cold-port-detect enable. */
        uint32_t tfee                  : 1;  /**< [ 30: 30](R/W) Task-file-error enable. */
        uint32_t hbfe                  : 1;  /**< [ 29: 29](R/W) Host-bus fatal-error enable. */
        uint32_t hbde                  : 1;  /**< [ 28: 28](R/W) Host-bus data-error enable. */
        uint32_t ife                   : 1;  /**< [ 27: 27](R/W) Interface fatal-error enable. */
        uint32_t infe                  : 1;  /**< [ 26: 26](R/W) Interface non-fatal-error enable. */
        uint32_t reserved_25           : 1;
        uint32_t ofe                   : 1;  /**< [ 24: 24](R/W) Overflow enable. */
        uint32_t impe                  : 1;  /**< [ 23: 23](R/W) Incorrect port-multiplier enable. */
        uint32_t prce                  : 1;  /**< [ 22: 22](R/W) PHY-ready-change enable. */
        uint32_t reserved_8_21         : 14;
        uint32_t dmpe                  : 1;  /**< [  7:  7](R/W) Device mechanical-presence enable. */
        uint32_t pce                   : 1;  /**< [  6:  6](R/W) Port-connect-change enable. */
        uint32_t dpe                   : 1;  /**< [  5:  5](R/W) Descriptor-processed enable. */
        uint32_t ufe                   : 1;  /**< [  4:  4](R/W) Unknown-FIS-interrupt enable. */
        uint32_t sdbe                  : 1;  /**< [  3:  3](R/W) Set device-bits-interrupt enable. */
        uint32_t dse                   : 1;  /**< [  2:  2](R/W) DMA-setup FIS interrupt enable. */
        uint32_t pse                   : 1;  /**< [  1:  1](R/W) PIO-setup FIS interrupt enable. */
        uint32_t dhre                  : 1;  /**< [  0:  0](R/W) Device-to-host register FIS interrupt enable. */
#else /* Word 0 - Little Endian */
        uint32_t dhre                  : 1;  /**< [  0:  0](R/W) Device-to-host register FIS interrupt enable. */
        uint32_t pse                   : 1;  /**< [  1:  1](R/W) PIO-setup FIS interrupt enable. */
        uint32_t dse                   : 1;  /**< [  2:  2](R/W) DMA-setup FIS interrupt enable. */
        uint32_t sdbe                  : 1;  /**< [  3:  3](R/W) Set device-bits-interrupt enable. */
        uint32_t ufe                   : 1;  /**< [  4:  4](R/W) Unknown-FIS-interrupt enable. */
        uint32_t dpe                   : 1;  /**< [  5:  5](R/W) Descriptor-processed enable. */
        uint32_t pce                   : 1;  /**< [  6:  6](R/W) Port-connect-change enable. */
        uint32_t dmpe                  : 1;  /**< [  7:  7](R/W) Device mechanical-presence enable. */
        uint32_t reserved_8_21         : 14;
        uint32_t prce                  : 1;  /**< [ 22: 22](R/W) PHY-ready-change enable. */
        uint32_t impe                  : 1;  /**< [ 23: 23](R/W) Incorrect port-multiplier enable. */
        uint32_t ofe                   : 1;  /**< [ 24: 24](R/W) Overflow enable. */
        uint32_t reserved_25           : 1;
        uint32_t infe                  : 1;  /**< [ 26: 26](R/W) Interface non-fatal-error enable. */
        uint32_t ife                   : 1;  /**< [ 27: 27](R/W) Interface fatal-error enable. */
        uint32_t hbde                  : 1;  /**< [ 28: 28](R/W) Host-bus data-error enable. */
        uint32_t hbfe                  : 1;  /**< [ 29: 29](R/W) Host-bus fatal-error enable. */
        uint32_t tfee                  : 1;  /**< [ 30: 30](R/W) Task-file-error enable. */
        uint32_t cpde                  : 1;  /**< [ 31: 31](R/W) Cold-port-detect enable. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_ie_s cn; */
};
typedef union cavm_satax_uahc_p0_ie cavm_satax_uahc_p0_ie_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_IE(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_IE(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000114ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000114ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000114ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_IE", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_IE(a) cavm_satax_uahc_p0_ie_t
#define bustype_CAVM_SATAX_UAHC_P0_IE(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_IE(a) "SATAX_UAHC_P0_IE"
#define device_bar_CAVM_SATAX_UAHC_P0_IE(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_IE(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_IE(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_is
 *
 * SATA UAHC Interrupt Status Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_is
{
    uint32_t u;
    struct cavm_satax_uahc_p0_is_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t cpds                  : 1;  /**< [ 31: 31](R/W1C/H) Cold-port detect status. */
        uint32_t tfes                  : 1;  /**< [ 30: 30](R/W1C/H) Task-file error status. */
        uint32_t hbfs                  : 1;  /**< [ 29: 29](R/W1C/H) Host-bus fatal-error status. */
        uint32_t hbds                  : 1;  /**< [ 28: 28](R/W1C/H) Host-bus data-error status. */
        uint32_t ifs                   : 1;  /**< [ 27: 27](R/W1C/H) Interface fatal-error status. */
        uint32_t infs                  : 1;  /**< [ 26: 26](R/W1C/H) Interface non-fatal-error status. */
        uint32_t reserved_25           : 1;
        uint32_t ofs                   : 1;  /**< [ 24: 24](R/W1C/H) Overflow status. */
        uint32_t imps                  : 1;  /**< [ 23: 23](R/W1C/H) Incorrect port-multiplier status. */
        uint32_t prcs                  : 1;  /**< [ 22: 22](RO/H) PHY-ready change status. */
        uint32_t reserved_8_21         : 14;
        uint32_t dmps                  : 1;  /**< [  7:  7](R/W1C/H) Device mechanical-presence status. */
        uint32_t pcs                   : 1;  /**< [  6:  6](RO/H) Port-connect-change status. */
        uint32_t dps                   : 1;  /**< [  5:  5](R/W1C/H) Descriptor processed. */
        uint32_t ufs                   : 1;  /**< [  4:  4](RO) Unknown FIS interrupt. */
        uint32_t sdbs                  : 1;  /**< [  3:  3](R/W1C/H) Set device bits interrupt. */
        uint32_t dss                   : 1;  /**< [  2:  2](R/W1C/H) DMA setup FIS interrupt. */
        uint32_t pss                   : 1;  /**< [  1:  1](R/W1C/H) PIO setup FIS interrupt. */
        uint32_t dhrs                  : 1;  /**< [  0:  0](R/W1C/H) Device-to-host register FIS interrupt. */
#else /* Word 0 - Little Endian */
        uint32_t dhrs                  : 1;  /**< [  0:  0](R/W1C/H) Device-to-host register FIS interrupt. */
        uint32_t pss                   : 1;  /**< [  1:  1](R/W1C/H) PIO setup FIS interrupt. */
        uint32_t dss                   : 1;  /**< [  2:  2](R/W1C/H) DMA setup FIS interrupt. */
        uint32_t sdbs                  : 1;  /**< [  3:  3](R/W1C/H) Set device bits interrupt. */
        uint32_t ufs                   : 1;  /**< [  4:  4](RO) Unknown FIS interrupt. */
        uint32_t dps                   : 1;  /**< [  5:  5](R/W1C/H) Descriptor processed. */
        uint32_t pcs                   : 1;  /**< [  6:  6](RO/H) Port-connect-change status. */
        uint32_t dmps                  : 1;  /**< [  7:  7](R/W1C/H) Device mechanical-presence status. */
        uint32_t reserved_8_21         : 14;
        uint32_t prcs                  : 1;  /**< [ 22: 22](RO/H) PHY-ready change status. */
        uint32_t imps                  : 1;  /**< [ 23: 23](R/W1C/H) Incorrect port-multiplier status. */
        uint32_t ofs                   : 1;  /**< [ 24: 24](R/W1C/H) Overflow status. */
        uint32_t reserved_25           : 1;
        uint32_t infs                  : 1;  /**< [ 26: 26](R/W1C/H) Interface non-fatal-error status. */
        uint32_t ifs                   : 1;  /**< [ 27: 27](R/W1C/H) Interface fatal-error status. */
        uint32_t hbds                  : 1;  /**< [ 28: 28](R/W1C/H) Host-bus data-error status. */
        uint32_t hbfs                  : 1;  /**< [ 29: 29](R/W1C/H) Host-bus fatal-error status. */
        uint32_t tfes                  : 1;  /**< [ 30: 30](R/W1C/H) Task-file error status. */
        uint32_t cpds                  : 1;  /**< [ 31: 31](R/W1C/H) Cold-port detect status. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_is_s cn; */
};
typedef union cavm_satax_uahc_p0_is cavm_satax_uahc_p0_is_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_IS(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_IS(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000110ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000110ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000110ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_IS", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_IS(a) cavm_satax_uahc_p0_is_t
#define bustype_CAVM_SATAX_UAHC_P0_IS(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_IS(a) "SATAX_UAHC_P0_IS"
#define device_bar_CAVM_SATAX_UAHC_P0_IS(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_IS(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_IS(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_phycr
 *
 * SATA UAHC PHY Control Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_phycr
{
    uint32_t u;
    struct cavm_satax_uahc_p0_phycr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t ctrl                  : 32; /**< [ 31:  0](R/W) Port PHY control. */
#else /* Word 0 - Little Endian */
        uint32_t ctrl                  : 32; /**< [ 31:  0](R/W) Port PHY control. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_phycr_s cn; */
};
typedef union cavm_satax_uahc_p0_phycr cavm_satax_uahc_p0_phycr_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_PHYCR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_PHYCR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000178ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000178ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000178ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_PHYCR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_PHYCR(a) cavm_satax_uahc_p0_phycr_t
#define bustype_CAVM_SATAX_UAHC_P0_PHYCR(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_PHYCR(a) "SATAX_UAHC_P0_PHYCR"
#define device_bar_CAVM_SATAX_UAHC_P0_PHYCR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_PHYCR(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_PHYCR(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_physr
 *
 * SATA UAHC PHY Status Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_physr
{
    uint32_t u;
    struct cavm_satax_uahc_p0_physr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t stat                  : 32; /**< [ 31:  0](RO) Port PHY status. */
#else /* Word 0 - Little Endian */
        uint32_t stat                  : 32; /**< [ 31:  0](RO) Port PHY status. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_physr_s cn; */
};
typedef union cavm_satax_uahc_p0_physr cavm_satax_uahc_p0_physr_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_PHYSR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_PHYSR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x81000000017cll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x81000000017cll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x81000000017cll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_PHYSR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_PHYSR(a) cavm_satax_uahc_p0_physr_t
#define bustype_CAVM_SATAX_UAHC_P0_PHYSR(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_PHYSR(a) "SATAX_UAHC_P0_PHYSR"
#define device_bar_CAVM_SATAX_UAHC_P0_PHYSR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_PHYSR(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_PHYSR(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_sact
 *
 * SATA UAHC SATA Active Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_sact
{
    uint32_t u;
    struct cavm_satax_uahc_p0_sact_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t ds                    : 32; /**< [ 31:  0](R/W1S/H) Device status. */
#else /* Word 0 - Little Endian */
        uint32_t ds                    : 32; /**< [ 31:  0](R/W1S/H) Device status. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_sact_s cn; */
};
typedef union cavm_satax_uahc_p0_sact cavm_satax_uahc_p0_sact_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SACT(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SACT(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000134ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000134ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000134ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SACT", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SACT(a) cavm_satax_uahc_p0_sact_t
#define bustype_CAVM_SATAX_UAHC_P0_SACT(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SACT(a) "SATAX_UAHC_P0_SACT"
#define device_bar_CAVM_SATAX_UAHC_P0_SACT(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SACT(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SACT(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_sctl
 *
 * SATA UAHC SATA Control Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_sctl
{
    uint32_t u;
    struct cavm_satax_uahc_p0_sctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_10_31        : 22;
        uint32_t ipm                   : 2;  /**< [  9:  8](R/W) Interface power-management transitions allowed. */
        uint32_t reserved_6_7          : 2;
        uint32_t spd                   : 2;  /**< [  5:  4](R/W) Speed allowed. */
        uint32_t reserved_3            : 1;
        uint32_t det                   : 3;  /**< [  2:  0](R/W) Device-detection initialization. */
#else /* Word 0 - Little Endian */
        uint32_t det                   : 3;  /**< [  2:  0](R/W) Device-detection initialization. */
        uint32_t reserved_3            : 1;
        uint32_t spd                   : 2;  /**< [  5:  4](R/W) Speed allowed. */
        uint32_t reserved_6_7          : 2;
        uint32_t ipm                   : 2;  /**< [  9:  8](R/W) Interface power-management transitions allowed. */
        uint32_t reserved_10_31        : 22;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_sctl_s cn; */
};
typedef union cavm_satax_uahc_p0_sctl cavm_satax_uahc_p0_sctl_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SCTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SCTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x81000000012cll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x81000000012cll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x81000000012cll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SCTL", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SCTL(a) cavm_satax_uahc_p0_sctl_t
#define bustype_CAVM_SATAX_UAHC_P0_SCTL(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SCTL(a) "SATAX_UAHC_P0_SCTL"
#define device_bar_CAVM_SATAX_UAHC_P0_SCTL(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SCTL(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SCTL(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_serr
 *
 * SATA UAHC SATA Error Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_serr
{
    uint32_t u;
    struct cavm_satax_uahc_p0_serr_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_27_31        : 5;
        uint32_t diag_x                : 1;  /**< [ 26: 26](R/W1C/H) Exchanged. */
        uint32_t diag_f                : 1;  /**< [ 25: 25](R/W1C/H) Unknown FIS type. */
        uint32_t diag_t                : 1;  /**< [ 24: 24](R/W1C/H) Transport state transition error. */
        uint32_t diag_s                : 1;  /**< [ 23: 23](R/W1C/H) Link sequence error. */
        uint32_t diag_h                : 1;  /**< [ 22: 22](R/W1C/H) Handshake error. */
        uint32_t diag_c                : 1;  /**< [ 21: 21](R/W1C/H) CRC error. */
        uint32_t diag_d                : 1;  /**< [ 20: 20](R/W1C/H) Disparity error. */
        uint32_t diag_b                : 1;  /**< [ 19: 19](R/W1C/H) 10/8 bit decode error. */
        uint32_t diag_w                : 1;  /**< [ 18: 18](R/W1C/H) COMWAKE detected. */
        uint32_t diag_i                : 1;  /**< [ 17: 17](R/W1C/H) PHY internal error. */
        uint32_t diag_n                : 1;  /**< [ 16: 16](R/W1C/H) PHY ready change. */
        uint32_t reserved_12_15        : 4;
        uint32_t err_e                 : 1;  /**< [ 11: 11](R/W1C/H) Internal error. */
        uint32_t err_p                 : 1;  /**< [ 10: 10](R/W1C/H) Protocol error. */
        uint32_t err_c                 : 1;  /**< [  9:  9](R/W1C/H) Non-recovered persistent communication error. */
        uint32_t err_t                 : 1;  /**< [  8:  8](R/W1C/H) Non-recovered transient data integrity error. */
        uint32_t reserved_2_7          : 6;
        uint32_t err_m                 : 1;  /**< [  1:  1](R/W1C/H) Recovered communication error. */
        uint32_t err_i                 : 1;  /**< [  0:  0](R/W1C/H) Recovered data integrity. */
#else /* Word 0 - Little Endian */
        uint32_t err_i                 : 1;  /**< [  0:  0](R/W1C/H) Recovered data integrity. */
        uint32_t err_m                 : 1;  /**< [  1:  1](R/W1C/H) Recovered communication error. */
        uint32_t reserved_2_7          : 6;
        uint32_t err_t                 : 1;  /**< [  8:  8](R/W1C/H) Non-recovered transient data integrity error. */
        uint32_t err_c                 : 1;  /**< [  9:  9](R/W1C/H) Non-recovered persistent communication error. */
        uint32_t err_p                 : 1;  /**< [ 10: 10](R/W1C/H) Protocol error. */
        uint32_t err_e                 : 1;  /**< [ 11: 11](R/W1C/H) Internal error. */
        uint32_t reserved_12_15        : 4;
        uint32_t diag_n                : 1;  /**< [ 16: 16](R/W1C/H) PHY ready change. */
        uint32_t diag_i                : 1;  /**< [ 17: 17](R/W1C/H) PHY internal error. */
        uint32_t diag_w                : 1;  /**< [ 18: 18](R/W1C/H) COMWAKE detected. */
        uint32_t diag_b                : 1;  /**< [ 19: 19](R/W1C/H) 10/8 bit decode error. */
        uint32_t diag_d                : 1;  /**< [ 20: 20](R/W1C/H) Disparity error. */
        uint32_t diag_c                : 1;  /**< [ 21: 21](R/W1C/H) CRC error. */
        uint32_t diag_h                : 1;  /**< [ 22: 22](R/W1C/H) Handshake error. */
        uint32_t diag_s                : 1;  /**< [ 23: 23](R/W1C/H) Link sequence error. */
        uint32_t diag_t                : 1;  /**< [ 24: 24](R/W1C/H) Transport state transition error. */
        uint32_t diag_f                : 1;  /**< [ 25: 25](R/W1C/H) Unknown FIS type. */
        uint32_t diag_x                : 1;  /**< [ 26: 26](R/W1C/H) Exchanged. */
        uint32_t reserved_27_31        : 5;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_serr_s cn; */
};
typedef union cavm_satax_uahc_p0_serr cavm_satax_uahc_p0_serr_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SERR(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SERR(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000130ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000130ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000130ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SERR", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SERR(a) cavm_satax_uahc_p0_serr_t
#define bustype_CAVM_SATAX_UAHC_P0_SERR(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SERR(a) "SATAX_UAHC_P0_SERR"
#define device_bar_CAVM_SATAX_UAHC_P0_SERR(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SERR(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SERR(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_sig
 *
 * SATA UAHC Signature Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_sig
{
    uint32_t u;
    struct cavm_satax_uahc_p0_sig_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t sig                   : 32; /**< [ 31:  0](RO/H) Signature. */
#else /* Word 0 - Little Endian */
        uint32_t sig                   : 32; /**< [ 31:  0](RO/H) Signature. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_sig_s cn; */
};
typedef union cavm_satax_uahc_p0_sig cavm_satax_uahc_p0_sig_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SIG(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SIG(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000124ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000124ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000124ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SIG", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SIG(a) cavm_satax_uahc_p0_sig_t
#define bustype_CAVM_SATAX_UAHC_P0_SIG(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SIG(a) "SATAX_UAHC_P0_SIG"
#define device_bar_CAVM_SATAX_UAHC_P0_SIG(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SIG(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SIG(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_sntf
 *
 * SATA UAHC SATA Notification Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_sntf
{
    uint32_t u;
    struct cavm_satax_uahc_p0_sntf_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_16_31        : 16;
        uint32_t pmn                   : 16; /**< [ 15:  0](R/W1C/H) PM notify. */
#else /* Word 0 - Little Endian */
        uint32_t pmn                   : 16; /**< [ 15:  0](R/W1C/H) PM notify. */
        uint32_t reserved_16_31        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_sntf_s cn; */
};
typedef union cavm_satax_uahc_p0_sntf cavm_satax_uahc_p0_sntf_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SNTF(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SNTF(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x81000000013cll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x81000000013cll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x81000000013cll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SNTF", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SNTF(a) cavm_satax_uahc_p0_sntf_t
#define bustype_CAVM_SATAX_UAHC_P0_SNTF(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SNTF(a) "SATAX_UAHC_P0_SNTF"
#define device_bar_CAVM_SATAX_UAHC_P0_SNTF(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SNTF(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SNTF(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_ssts
 *
 * SATA UAHC SATA Status Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_ssts
{
    uint32_t u;
    struct cavm_satax_uahc_p0_ssts_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_12_31        : 20;
        uint32_t ipm                   : 4;  /**< [ 11:  8](RO/H) Interface power management. */
        uint32_t spd                   : 4;  /**< [  7:  4](RO/H) Current interface speed. */
        uint32_t det                   : 4;  /**< [  3:  0](RO/H) Device detection. */
#else /* Word 0 - Little Endian */
        uint32_t det                   : 4;  /**< [  3:  0](RO/H) Device detection. */
        uint32_t spd                   : 4;  /**< [  7:  4](RO/H) Current interface speed. */
        uint32_t ipm                   : 4;  /**< [ 11:  8](RO/H) Interface power management. */
        uint32_t reserved_12_31        : 20;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_ssts_s cn; */
};
typedef union cavm_satax_uahc_p0_ssts cavm_satax_uahc_p0_ssts_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_SSTS(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_SSTS(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000128ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000128ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000128ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_SSTS", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_SSTS(a) cavm_satax_uahc_p0_ssts_t
#define bustype_CAVM_SATAX_UAHC_P0_SSTS(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_SSTS(a) "SATAX_UAHC_P0_SSTS"
#define device_bar_CAVM_SATAX_UAHC_P0_SSTS(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_SSTS(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_SSTS(a) (a),-1,-1,-1

/**
 * Register (NCB32b) sata#_uahc_p0_tfd
 *
 * SATA UAHC Task File Data Registers
 * Internal:
 * See DWC_ahsata databook v4.20a.
 */
union cavm_satax_uahc_p0_tfd
{
    uint32_t u;
    struct cavm_satax_uahc_p0_tfd_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t reserved_16_31        : 16;
        uint32_t tferr                 : 8;  /**< [ 15:  8](RO) Copy of task-file error register. */
        uint32_t sts                   : 8;  /**< [  7:  0](RO/H) Copy of task-file status register. */
#else /* Word 0 - Little Endian */
        uint32_t sts                   : 8;  /**< [  7:  0](RO/H) Copy of task-file status register. */
        uint32_t tferr                 : 8;  /**< [ 15:  8](RO) Copy of task-file error register. */
        uint32_t reserved_16_31        : 16;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uahc_p0_tfd_s cn; */
};
typedef union cavm_satax_uahc_p0_tfd cavm_satax_uahc_p0_tfd_t;

static inline uint64_t CAVM_SATAX_UAHC_P0_TFD(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UAHC_P0_TFD(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000000120ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000000120ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000000120ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UAHC_P0_TFD", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UAHC_P0_TFD(a) cavm_satax_uahc_p0_tfd_t
#define bustype_CAVM_SATAX_UAHC_P0_TFD(a) CSR_TYPE_NCB32b
#define basename_CAVM_SATAX_UAHC_P0_TFD(a) "SATAX_UAHC_P0_TFD"
#define device_bar_CAVM_SATAX_UAHC_P0_TFD(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UAHC_P0_TFD(a) (a)
#define arguments_CAVM_SATAX_UAHC_P0_TFD(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uctl_bist_status
 *
 * SATA UCTL BIST Status Register
 * Results from BIST runs of SATA's memories.
 * Wait for NDONE==0, then look at defect indication.
 *
 * Accessible always.
 *
 * Reset by NCB reset.
 */
union cavm_satax_uctl_bist_status
{
    uint64_t u;
    struct cavm_satax_uctl_bist_status_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_42_63        : 22;
        uint64_t uctl_xm_r_bist_ndone  : 1;  /**< [ 41: 41](RO/H) BIST is not complete for the UCTL AxiMaster read-data FIFO. */
        uint64_t uctl_xm_w_bist_ndone  : 1;  /**< [ 40: 40](RO/H) BIST is not complete for the UCTL AxiMaster write-data FIFO. */
        uint64_t reserved_36_39        : 4;
        uint64_t uahc_p0_rxram_bist_ndone : 1;/**< [ 35: 35](RO/H) BIST is not complete for the UAHC Port 0 RxFIFO RAM. */
        uint64_t reserved_34           : 1;
        uint64_t uahc_p0_txram_bist_ndone : 1;/**< [ 33: 33](RO/H) BIST is not complete for the UAHC Port 0 TxFIFO RAM. */
        uint64_t reserved_10_32        : 23;
        uint64_t uctl_xm_r_bist_status : 1;  /**< [  9:  9](RO/H) BIST status of the UCTL AxiMaster read-data FIFO. */
        uint64_t uctl_xm_w_bist_status : 1;  /**< [  8:  8](RO/H) BIST status of the UCTL AxiMaster write-data FIFO. */
        uint64_t reserved_4_7          : 4;
        uint64_t uahc_p0_rxram_bist_status : 1;/**< [  3:  3](RO/H) BIST status of the UAHC Port0 RxFIFO RAM. */
        uint64_t reserved_2            : 1;
        uint64_t uahc_p0_txram_bist_status : 1;/**< [  1:  1](RO/H) BIST status of the UAHC Port0 TxFIFO RAM. */
        uint64_t reserved_0            : 1;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0            : 1;
        uint64_t uahc_p0_txram_bist_status : 1;/**< [  1:  1](RO/H) BIST status of the UAHC Port0 TxFIFO RAM. */
        uint64_t reserved_2            : 1;
        uint64_t uahc_p0_rxram_bist_status : 1;/**< [  3:  3](RO/H) BIST status of the UAHC Port0 RxFIFO RAM. */
        uint64_t reserved_4_7          : 4;
        uint64_t uctl_xm_w_bist_status : 1;  /**< [  8:  8](RO/H) BIST status of the UCTL AxiMaster write-data FIFO. */
        uint64_t uctl_xm_r_bist_status : 1;  /**< [  9:  9](RO/H) BIST status of the UCTL AxiMaster read-data FIFO. */
        uint64_t reserved_10_32        : 23;
        uint64_t uahc_p0_txram_bist_ndone : 1;/**< [ 33: 33](RO/H) BIST is not complete for the UAHC Port 0 TxFIFO RAM. */
        uint64_t reserved_34           : 1;
        uint64_t uahc_p0_rxram_bist_ndone : 1;/**< [ 35: 35](RO/H) BIST is not complete for the UAHC Port 0 RxFIFO RAM. */
        uint64_t reserved_36_39        : 4;
        uint64_t uctl_xm_w_bist_ndone  : 1;  /**< [ 40: 40](RO/H) BIST is not complete for the UCTL AxiMaster write-data FIFO. */
        uint64_t uctl_xm_r_bist_ndone  : 1;  /**< [ 41: 41](RO/H) BIST is not complete for the UCTL AxiMaster read-data FIFO. */
        uint64_t reserved_42_63        : 22;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uctl_bist_status_s cn; */
};
typedef union cavm_satax_uctl_bist_status cavm_satax_uctl_bist_status_t;

static inline uint64_t CAVM_SATAX_UCTL_BIST_STATUS(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UCTL_BIST_STATUS(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000100008ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000100008ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000100008ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UCTL_BIST_STATUS", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UCTL_BIST_STATUS(a) cavm_satax_uctl_bist_status_t
#define bustype_CAVM_SATAX_UCTL_BIST_STATUS(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UCTL_BIST_STATUS(a) "SATAX_UCTL_BIST_STATUS"
#define device_bar_CAVM_SATAX_UCTL_BIST_STATUS(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UCTL_BIST_STATUS(a) (a)
#define arguments_CAVM_SATAX_UCTL_BIST_STATUS(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uctl_ctl
 *
 * SATA UCTL Control Register
 * This register controls clocks, resets, power, and BIST for the SATA.
 *
 * Accessible always.
 *
 * Reset by NCB reset.
 */
union cavm_satax_uctl_ctl
{
    uint64_t u;
    struct cavm_satax_uctl_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t clear_bist            : 1;  /**< [ 63: 63](R/W) BIST fast-clear mode select. There are two major modes of BIST: FULL and CLEAR.
                                                                 0 = FULL BIST is run by the BIST state machine.
                                                                 1 = CLEAR BIST is run by the BIST state machine. A clear-BIST run clears all entries in
                                                                 SATA RAMs to 0x0.

                                                                 To avoid race conditions, software must first perform a CSR write operation that puts
                                                                 [CLEAR_BIST] into the correct state and then perform another CSR write operation to set
                                                                 [START_BIST] (keeping [CLEAR_BIST] constant). CLEAR BIST completion is indicated by
                                                                 SATA()_UCTL_BIST_STATUS[NDONE*] clear.

                                                                 A BIST clear operation takes almost 2,000 host-controller clock cycles for the largest
                                                                 RAM. */
        uint64_t start_bist            : 1;  /**< [ 62: 62](R/W) Start BIST. The rising edge starts BIST on the memories in SATA. To run BIST, the host-
                                                                 controller clock must be both configured and enabled, and should be configured to the
                                                                 maximum available frequency given the available coprocessor clock and dividers.

                                                                 Refer to Cold Reset for clock initialization procedures. BIST defect status can
                                                                 be checked after FULL BIST completion, both of which are indicated in
                                                                 SATA()_UCTL_BIST_STATUS. The FULL BIST run takes almost 80,000 host-controller
                                                                 clock cycles for the largest RAM. */
        uint64_t reserved_31_61        : 31;
        uint64_t a_clk_en              : 1;  /**< [ 30: 30](R/W) Host-controller clock enable. When set to 1, the host-controller clock is generated. This
                                                                 also enables access to UCTL registers 0x30-0xF8. */
        uint64_t a_clk_byp_sel         : 1;  /**< [ 29: 29](R/W) Select the bypass input to the host-controller clock divider.
                                                                 0 = Use the divided coprocessor clock from the A_CLKDIV divider.
                                                                 1 = use the bypass clock from the GPIO pins (generally bypass is only used for scan
                                                                 purposes).

                                                                 This signal is a multiplexer-select signal; it does not enable the host-controller clock.
                                                                 You must set [A_CLK_EN] separately. [A_CLK_BYP_SEL] select should not be changed unless
                                                                 [A_CLK_EN] is disabled. The bypass clock can be selected and running even if the host-
                                                                 controller clock dividers are not running. */
        uint64_t a_clkdiv_rst          : 1;  /**< [ 28: 28](R/W) Host-controller-clock divider reset. Divided clocks are not generated while the divider is
                                                                 being reset.
                                                                 This also resets the suspend-clock divider. */
        uint64_t reserved_27           : 1;
        uint64_t a_clkdiv_sel          : 3;  /**< [ 26: 24](R/W) The host-controller clock frequency is the coprocessor-clock frequency divided by
                                                                 [A_CLKDIV_SEL]. The host-controller clock frequency must be at or below 333MHz.
                                                                 This field can be changed only when [A_CLKDIV_RST] = 1. The divider values are the
                                                                 following:
                                                                 0x0 = divide by 1.
                                                                 0x1 = divide by 2.
                                                                 0x2 = divide by 3.
                                                                 0x3 = divide by 4.
                                                                 0x4 = divide by 6.
                                                                 0x5 = divide by 8.
                                                                 0x6 = divide by 16.
                                                                 0x7 = divide by 24. */
        uint64_t reserved_5_23         : 19;
        uint64_t csclk_en              : 1;  /**< [  4:  4](R/W) Turns on the SATA UCTL interface clock (coprocessor clock). This enables access to UAHC
                                                                 registers via the NCB, as well as UCTL registers starting from 0x10_0030. */
        uint64_t reserved_2_3          : 2;
        uint64_t sata_uahc_rst         : 1;  /**< [  1:  1](R/W) Software reset; resets UAHC; active-high.
                                                                 Internal:
                                                                 Note that soft-resetting the UAHC while it is active may cause violations of RSL
                                                                 or NCB protocols. */
        uint64_t sata_uctl_rst         : 1;  /**< [  0:  0](R/W) Software reset; resets UCTL; active-high. Resets UAHC DMA and register shims and the UCTL
                                                                 registers 0x10_0030-0x10_00F8.

                                                                 It does not reset UCTL registers 0x10_0000-0x10_0028.

                                                                 The UCTL registers starting from 0x10_0030 can be accessed only after the host-controller
                                                                 clock is active and UCTL_RST is deasserted.

                                                                 Internal:
                                                                 Note that soft-resetting the UCTL while it is active may cause violations of
                                                                 RSL, NCB, and GIB protocols. */
#else /* Word 0 - Little Endian */
        uint64_t sata_uctl_rst         : 1;  /**< [  0:  0](R/W) Software reset; resets UCTL; active-high. Resets UAHC DMA and register shims and the UCTL
                                                                 registers 0x10_0030-0x10_00F8.

                                                                 It does not reset UCTL registers 0x10_0000-0x10_0028.

                                                                 The UCTL registers starting from 0x10_0030 can be accessed only after the host-controller
                                                                 clock is active and UCTL_RST is deasserted.

                                                                 Internal:
                                                                 Note that soft-resetting the UCTL while it is active may cause violations of
                                                                 RSL, NCB, and GIB protocols. */
        uint64_t sata_uahc_rst         : 1;  /**< [  1:  1](R/W) Software reset; resets UAHC; active-high.
                                                                 Internal:
                                                                 Note that soft-resetting the UAHC while it is active may cause violations of RSL
                                                                 or NCB protocols. */
        uint64_t reserved_2_3          : 2;
        uint64_t csclk_en              : 1;  /**< [  4:  4](R/W) Turns on the SATA UCTL interface clock (coprocessor clock). This enables access to UAHC
                                                                 registers via the NCB, as well as UCTL registers starting from 0x10_0030. */
        uint64_t reserved_5_23         : 19;
        uint64_t a_clkdiv_sel          : 3;  /**< [ 26: 24](R/W) The host-controller clock frequency is the coprocessor-clock frequency divided by
                                                                 [A_CLKDIV_SEL]. The host-controller clock frequency must be at or below 333MHz.
                                                                 This field can be changed only when [A_CLKDIV_RST] = 1. The divider values are the
                                                                 following:
                                                                 0x0 = divide by 1.
                                                                 0x1 = divide by 2.
                                                                 0x2 = divide by 3.
                                                                 0x3 = divide by 4.
                                                                 0x4 = divide by 6.
                                                                 0x5 = divide by 8.
                                                                 0x6 = divide by 16.
                                                                 0x7 = divide by 24. */
        uint64_t reserved_27           : 1;
        uint64_t a_clkdiv_rst          : 1;  /**< [ 28: 28](R/W) Host-controller-clock divider reset. Divided clocks are not generated while the divider is
                                                                 being reset.
                                                                 This also resets the suspend-clock divider. */
        uint64_t a_clk_byp_sel         : 1;  /**< [ 29: 29](R/W) Select the bypass input to the host-controller clock divider.
                                                                 0 = Use the divided coprocessor clock from the A_CLKDIV divider.
                                                                 1 = use the bypass clock from the GPIO pins (generally bypass is only used for scan
                                                                 purposes).

                                                                 This signal is a multiplexer-select signal; it does not enable the host-controller clock.
                                                                 You must set [A_CLK_EN] separately. [A_CLK_BYP_SEL] select should not be changed unless
                                                                 [A_CLK_EN] is disabled. The bypass clock can be selected and running even if the host-
                                                                 controller clock dividers are not running. */
        uint64_t a_clk_en              : 1;  /**< [ 30: 30](R/W) Host-controller clock enable. When set to 1, the host-controller clock is generated. This
                                                                 also enables access to UCTL registers 0x30-0xF8. */
        uint64_t reserved_31_61        : 31;
        uint64_t start_bist            : 1;  /**< [ 62: 62](R/W) Start BIST. The rising edge starts BIST on the memories in SATA. To run BIST, the host-
                                                                 controller clock must be both configured and enabled, and should be configured to the
                                                                 maximum available frequency given the available coprocessor clock and dividers.

                                                                 Refer to Cold Reset for clock initialization procedures. BIST defect status can
                                                                 be checked after FULL BIST completion, both of which are indicated in
                                                                 SATA()_UCTL_BIST_STATUS. The FULL BIST run takes almost 80,000 host-controller
                                                                 clock cycles for the largest RAM. */
        uint64_t clear_bist            : 1;  /**< [ 63: 63](R/W) BIST fast-clear mode select. There are two major modes of BIST: FULL and CLEAR.
                                                                 0 = FULL BIST is run by the BIST state machine.
                                                                 1 = CLEAR BIST is run by the BIST state machine. A clear-BIST run clears all entries in
                                                                 SATA RAMs to 0x0.

                                                                 To avoid race conditions, software must first perform a CSR write operation that puts
                                                                 [CLEAR_BIST] into the correct state and then perform another CSR write operation to set
                                                                 [START_BIST] (keeping [CLEAR_BIST] constant). CLEAR BIST completion is indicated by
                                                                 SATA()_UCTL_BIST_STATUS[NDONE*] clear.

                                                                 A BIST clear operation takes almost 2,000 host-controller clock cycles for the largest
                                                                 RAM. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uctl_ctl_s cn; */
};
typedef union cavm_satax_uctl_ctl cavm_satax_uctl_ctl_t;

static inline uint64_t CAVM_SATAX_UCTL_CTL(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UCTL_CTL(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x810000100000ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x810000100000ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x810000100000ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UCTL_CTL", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UCTL_CTL(a) cavm_satax_uctl_ctl_t
#define bustype_CAVM_SATAX_UCTL_CTL(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UCTL_CTL(a) "SATAX_UCTL_CTL"
#define device_bar_CAVM_SATAX_UCTL_CTL(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UCTL_CTL(a) (a)
#define arguments_CAVM_SATAX_UCTL_CTL(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uctl_ecc
 *
 * SATA UCTL ECC Control/Debug Register
 * This register can be used to disable ECC correction, insert ECC errors, and debug ECC
 * failures.
 *
 * Fields ECC_ERR* are captured when there are no outstanding ECC errors indicated in INTSTAT
 * and a new ECC error arrives. Prioritization for multiple events occurring on the same cycle is
 * indicated by the ECC_ERR_SOURCE enumeration: highest encoded value has highest priority.
 *
 * Fields *ECC_DIS: Disables ECC correction, SBE and DBE errors are still reported.
 * If ECC_DIS is 0x1, then no data-correction occurs.
 *
 * Fields *ECC_FLIP_SYND:  Flip the syndrom[1:0] bits to generate 1-bit/2-bits error for testing.
 *
 * Accessible only when SATA()_UCTL_CTL[A_CLK_EN].
 *
 * Reset by NCB reset or SATA()_UCTL_CTL[SATA_UCTL_RST].
 */
union cavm_satax_uctl_ecc
{
    uint64_t u;
    struct cavm_satax_uctl_ecc_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_62_63        : 2;
        uint64_t ecc_err_source        : 4;  /**< [ 61: 58](RO/H) Source of ECC error, see SATA_UCTL_ECC_ERR_SOURCE_E. */
        uint64_t ecc_err_syndrome      : 18; /**< [ 57: 40](RO/H) Syndrome bits of the ECC error. */
        uint64_t ecc_err_address       : 8;  /**< [ 39: 32](RO/H) RAM address of the ECC error. */
        uint64_t reserved_21_31        : 11;
        uint64_t uctl_xm_r_ecc_flip_synd : 2;/**< [ 20: 19](R/W) Insert ECC error for testing purposes. */
        uint64_t uctl_xm_r_ecc_cor_dis : 1;  /**< [ 18: 18](R/W) Enables ECC correction on UCTL AxiMaster read-data FIFO. */
        uint64_t uctl_xm_w_ecc_flip_synd : 2;/**< [ 17: 16](R/W) Insert ECC error for testing purposes. */
        uint64_t uctl_xm_w_ecc_cor_dis : 1;  /**< [ 15: 15](R/W) Enables ECC correction on UCTL AxiMaster write-data FIFO. */
        uint64_t reserved_9_14         : 6;
        uint64_t uahc_rx_ecc_flip_synd : 2;  /**< [  8:  7](R/W) Insert ECC error for testing purposes. */
        uint64_t uahc_rx_ecc_cor_dis   : 1;  /**< [  6:  6](R/W) Enables ECC correction on UAHC RxFIFO RAMs. */
        uint64_t uahc_tx_ecc_flip_synd : 2;  /**< [  5:  4](R/W) Insert ECC error for testing purposes. */
        uint64_t uahc_tx_ecc_cor_dis   : 1;  /**< [  3:  3](R/W) Enables ECC correction on UAHC TxFIFO RAMs. */
        uint64_t uahc_fb_ecc_flip_synd : 2;  /**< [  2:  1](R/W) Insert ECC error for testing purposes. */
        uint64_t uahc_fb_ecc_cor_dis   : 1;  /**< [  0:  0](R/W) Enables ECC correction on UAHC FBS RAM. */
#else /* Word 0 - Little Endian */
        uint64_t uahc_fb_ecc_cor_dis   : 1;  /**< [  0:  0](R/W) Enables ECC correction on UAHC FBS RAM. */
        uint64_t uahc_fb_ecc_flip_synd : 2;  /**< [  2:  1](R/W) Insert ECC error for testing purposes. */
        uint64_t uahc_tx_ecc_cor_dis   : 1;  /**< [  3:  3](R/W) Enables ECC correction on UAHC TxFIFO RAMs. */
        uint64_t uahc_tx_ecc_flip_synd : 2;  /**< [  5:  4](R/W) Insert ECC error for testing purposes. */
        uint64_t uahc_rx_ecc_cor_dis   : 1;  /**< [  6:  6](R/W) Enables ECC correction on UAHC RxFIFO RAMs. */
        uint64_t uahc_rx_ecc_flip_synd : 2;  /**< [  8:  7](R/W) Insert ECC error for testing purposes. */
        uint64_t reserved_9_14         : 6;
        uint64_t uctl_xm_w_ecc_cor_dis : 1;  /**< [ 15: 15](R/W) Enables ECC correction on UCTL AxiMaster write-data FIFO. */
        uint64_t uctl_xm_w_ecc_flip_synd : 2;/**< [ 17: 16](R/W) Insert ECC error for testing purposes. */
        uint64_t uctl_xm_r_ecc_cor_dis : 1;  /**< [ 18: 18](R/W) Enables ECC correction on UCTL AxiMaster read-data FIFO. */
        uint64_t uctl_xm_r_ecc_flip_synd : 2;/**< [ 20: 19](R/W) Insert ECC error for testing purposes. */
        uint64_t reserved_21_31        : 11;
        uint64_t ecc_err_address       : 8;  /**< [ 39: 32](RO/H) RAM address of the ECC error. */
        uint64_t ecc_err_syndrome      : 18; /**< [ 57: 40](RO/H) Syndrome bits of the ECC error. */
        uint64_t ecc_err_source        : 4;  /**< [ 61: 58](RO/H) Source of ECC error, see SATA_UCTL_ECC_ERR_SOURCE_E. */
        uint64_t reserved_62_63        : 2;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uctl_ecc_s cn; */
};
typedef union cavm_satax_uctl_ecc cavm_satax_uctl_ecc_t;

static inline uint64_t CAVM_SATAX_UCTL_ECC(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UCTL_ECC(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x8100001000f0ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x8100001000f0ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x8100001000f0ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UCTL_ECC", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UCTL_ECC(a) cavm_satax_uctl_ecc_t
#define bustype_CAVM_SATAX_UCTL_ECC(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UCTL_ECC(a) "SATAX_UCTL_ECC"
#define device_bar_CAVM_SATAX_UCTL_ECC(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UCTL_ECC(a) (a)
#define arguments_CAVM_SATAX_UCTL_ECC(a) (a),-1,-1,-1

/**
 * Register (NCB) sata#_uctl_shim_cfg
 *
 * SATA UCTL Shim Configuration Register
 * This register allows configuration of various shim (UCTL) features.
 *
 * Fields XS_NCB_OOB_* are captured when there are no outstanding OOB errors indicated in INTSTAT
 * and a new OOB error arrives.
 *
 * Fields XS_BAD_DMA_* are captured when there are no outstanding DMA errors indicated in INTSTAT
 * and a new DMA error arrives.
 *
 * Accessible only when SATA()_UCTL_CTL[A_CLK_EN].
 *
 * Reset by NCB reset or SATA()_UCTL_CTL[SATA_UCTL_RST].
 */
union cavm_satax_uctl_shim_cfg
{
    uint64_t u;
    struct cavm_satax_uctl_shim_cfg_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t xs_ncb_oob_wrn        : 1;  /**< [ 63: 63](RO/H) Read/write error log for out-of-bound UAHC register access.
                                                                 0 = read, 1 = write. */
        uint64_t reserved_60_62        : 3;
        uint64_t xs_ncb_oob_osrc       : 12; /**< [ 59: 48](RO/H) SRCID error log for out-of-bound UAHC register access. The NCB outbound SRCID for the OOB
                                                                 error.
                                                                 <59:58> = chipID.
                                                                 <57> = Request source: 0 = core, 1 = NCB-device.
                                                                 <56:51> = core/NCB-device number. Note that for NCB devices, <56> is always 0.
                                                                 <50:48> = SubID. */
        uint64_t xm_bad_dma_wrn        : 1;  /**< [ 47: 47](RO/H) Read/write error log for bad DMA access from UAHC.
                                                                 0 = read error log, 1 = write error log. */
        uint64_t reserved_44_46        : 3;
        uint64_t xm_bad_dma_type       : 4;  /**< [ 43: 40](RO/H) ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                                 (error largest encoded value has priority). See SATA_UCTL_XM_BAD_DMA_TYPE_E. */
        uint64_t reserved_14_39        : 26;
        uint64_t dma_read_cmd          : 2;  /**< [ 13: 12](R/W) Selects the NCB read command used by DMA accesses. See SATA_UCTL_DMA_READ_CMD_E. */
        uint64_t reserved_11           : 1;
        uint64_t dma_write_cmd         : 1;  /**< [ 10: 10](R/W) Selects the NCB write command used by DMA accesses. See enum SATA_UCTL_DMA_WRITE_CMD_E. */
        uint64_t reserved_0_9          : 10;
#else /* Word 0 - Little Endian */
        uint64_t reserved_0_9          : 10;
        uint64_t dma_write_cmd         : 1;  /**< [ 10: 10](R/W) Selects the NCB write command used by DMA accesses. See enum SATA_UCTL_DMA_WRITE_CMD_E. */
        uint64_t reserved_11           : 1;
        uint64_t dma_read_cmd          : 2;  /**< [ 13: 12](R/W) Selects the NCB read command used by DMA accesses. See SATA_UCTL_DMA_READ_CMD_E. */
        uint64_t reserved_14_39        : 26;
        uint64_t xm_bad_dma_type       : 4;  /**< [ 43: 40](RO/H) ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                                 (error largest encoded value has priority). See SATA_UCTL_XM_BAD_DMA_TYPE_E. */
        uint64_t reserved_44_46        : 3;
        uint64_t xm_bad_dma_wrn        : 1;  /**< [ 47: 47](RO/H) Read/write error log for bad DMA access from UAHC.
                                                                 0 = read error log, 1 = write error log. */
        uint64_t xs_ncb_oob_osrc       : 12; /**< [ 59: 48](RO/H) SRCID error log for out-of-bound UAHC register access. The NCB outbound SRCID for the OOB
                                                                 error.
                                                                 <59:58> = chipID.
                                                                 <57> = Request source: 0 = core, 1 = NCB-device.
                                                                 <56:51> = core/NCB-device number. Note that for NCB devices, <56> is always 0.
                                                                 <50:48> = SubID. */
        uint64_t reserved_60_62        : 3;
        uint64_t xs_ncb_oob_wrn        : 1;  /**< [ 63: 63](RO/H) Read/write error log for out-of-bound UAHC register access.
                                                                 0 = read, 1 = write. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_satax_uctl_shim_cfg_s cn; */
};
typedef union cavm_satax_uctl_shim_cfg cavm_satax_uctl_shim_cfg_t;

static inline uint64_t CAVM_SATAX_UCTL_SHIM_CFG(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SATAX_UCTL_SHIM_CFG(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a<=1))
        return 0x8100001000e8ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=5))
        return 0x8100001000e8ll + 0x1000000000ll * ((a) & 0x7);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=15))
        return 0x8100001000e8ll + 0x1000000000ll * ((a) & 0xf);
    __cavm_csr_fatal("SATAX_UCTL_SHIM_CFG", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SATAX_UCTL_SHIM_CFG(a) cavm_satax_uctl_shim_cfg_t
#define bustype_CAVM_SATAX_UCTL_SHIM_CFG(a) CSR_TYPE_NCB
#define basename_CAVM_SATAX_UCTL_SHIM_CFG(a) "SATAX_UCTL_SHIM_CFG"
#define device_bar_CAVM_SATAX_UCTL_SHIM_CFG(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SATAX_UCTL_SHIM_CFG(a) (a)
#define arguments_CAVM_SATAX_UCTL_SHIM_CFG(a) (a),-1,-1,-1

#endif /* __CAVM_CSRS_SATA_H__ */

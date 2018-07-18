#ifndef __CAVM_CSRS_PCCPF_H__
#define __CAVM_CSRS_PCCPF_H__
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
 * Cavium PCCPF.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration pcc_dev_con_e
 *
 * PCC Device Connection Enumeration
 * Enumerates where the device is connected in the topology. Software must rely on discovery and
 * not use this enumeration as the values will vary by product, and the mnemonics are a super-set
 * of the devices available. The value of the enumeration is formatted as defined by
 * PCC_DEV_CON_S.
 */
#define CAVM_PCC_DEV_CON_E_BCH (0x300)
#define CAVM_PCC_DEV_CON_E_BGXX(a) (0x180 + (a))
#define CAVM_PCC_DEV_CON_E_CPT0 (0x400)
#define CAVM_PCC_DEV_CON_E_CPT1 (0x500)
#define CAVM_PCC_DEV_CON_E_DAP (0x102)
#define CAVM_PCC_DEV_CON_E_DDF0 (0x10500)
#define CAVM_PCC_DEV_CON_E_DFA (0x400)
#define CAVM_PCC_DEV_CON_E_DPI0 (0xb00)
#define CAVM_PCC_DEV_CON_E_FPA (0x900)
#define CAVM_PCC_DEV_CON_E_FUSF (0x104)
#define CAVM_PCC_DEV_CON_E_GIC (0x18)
#define CAVM_PCC_DEV_CON_E_GPIO (0x30)
#define CAVM_PCC_DEV_CON_E_GSERX(a) (0x1e0 + (a))
#define CAVM_PCC_DEV_CON_E_GTI (0x20)
#define CAVM_PCC_DEV_CON_E_IOBNX(a) (0x158 + (a))
#define CAVM_PCC_DEV_CON_E_KEY (0x10d)
#define CAVM_PCC_DEV_CON_E_L2C (0x109)
#define CAVM_PCC_DEV_CON_E_L2C_CBCX(a) (0x138 + (a))
#define CAVM_PCC_DEV_CON_E_L2C_MCIX(a) (0x13c + (a))
#define CAVM_PCC_DEV_CON_E_L2C_TADX(a) (0x130 + (a))
#define CAVM_PCC_DEV_CON_E_LBKX(a) (0x168 + (a))
#define CAVM_PCC_DEV_CON_E_LMCX(a) (0x150 + (a))
#define CAVM_PCC_DEV_CON_E_MIO_BOOT (0x10e)
#define CAVM_PCC_DEV_CON_E_MIO_EMM (0x10c)
#define CAVM_PCC_DEV_CON_E_MIO_FUS (0x103)
#define CAVM_PCC_DEV_CON_E_MIO_PTP (0x40)
#define CAVM_PCC_DEV_CON_E_MIO_TWSX(a) (0x148 + (a))
#define CAVM_PCC_DEV_CON_E_MPI (0x38)
#define CAVM_PCC_DEV_CON_E_MRML (0x100)
#define CAVM_PCC_DEV_CON_E_NCSI (0x108)
#define CAVM_PCC_DEV_CON_E_NDF (0x58)
#define CAVM_PCC_DEV_CON_E_NIC_CN81XX (0x500)
#define CAVM_PCC_DEV_CON_E_NIC_CN88XX (0x20100)
#define CAVM_PCC_DEV_CON_E_NIC_CN83XX (0x10100)
#define CAVM_PCC_DEV_CON_E_OCLAX(a) (0x160 + (a))
#define CAVM_PCC_DEV_CON_E_OCX (0x105)
#define CAVM_PCC_DEV_CON_E_PBUS (0x10f)
#define CAVM_PCC_DEV_CON_E_PCCBR_BCH (0x50)
#define CAVM_PCC_DEV_CON_E_PCCBR_CPT0 (0x60)
#define CAVM_PCC_DEV_CON_E_PCCBR_CPT1 (0x68)
#define CAVM_PCC_DEV_CON_E_PCCBR_DDF0 (0x100a0)
#define CAVM_PCC_DEV_CON_E_PCCBR_DFA (0xb0)
#define CAVM_PCC_DEV_CON_E_PCCBR_DPI0 (0xa0)
#define CAVM_PCC_DEV_CON_E_PCCBR_FPA (0x90)
#define CAVM_PCC_DEV_CON_E_PCCBR_MRML (8)
#define CAVM_PCC_DEV_CON_E_PCCBR_NIC_CN81XX (0x78)
#define CAVM_PCC_DEV_CON_E_PCCBR_NIC_CN88XX (0x20010)
#define CAVM_PCC_DEV_CON_E_PCCBR_NIC_CN83XX (0x10080)
#define CAVM_PCC_DEV_CON_E_PCCBR_PKI (0x10088)
#define CAVM_PCC_DEV_CON_E_PCCBR_PKO (0x10090)
#define CAVM_PCC_DEV_CON_E_PCCBR_RAD_CN88XX (0xa0)
#define CAVM_PCC_DEV_CON_E_PCCBR_RAD_CN83XX (0x70)
#define CAVM_PCC_DEV_CON_E_PCCBR_RNM (0x48)
#define CAVM_PCC_DEV_CON_E_PCCBR_SSO (0x80)
#define CAVM_PCC_DEV_CON_E_PCCBR_SSOW (0x88)
#define CAVM_PCC_DEV_CON_E_PCCBR_TIM (0x98)
#define CAVM_PCC_DEV_CON_E_PCCBR_ZIP_CN88XX (0xa8)
#define CAVM_PCC_DEV_CON_E_PCCBR_ZIP_CN83XX (0x10098)
#define CAVM_PCC_DEV_CON_E_PCIERC0_CN81XX (0xc0)
#define CAVM_PCC_DEV_CON_E_PCIERC0_CN88XX (0x10080)
#define CAVM_PCC_DEV_CON_E_PCIERC0_CN83XX (0xc8)
#define CAVM_PCC_DEV_CON_E_PCIERC1_CN81XX (0xc8)
#define CAVM_PCC_DEV_CON_E_PCIERC1_CN88XX (0x10090)
#define CAVM_PCC_DEV_CON_E_PCIERC1_CN83XX (0xd0)
#define CAVM_PCC_DEV_CON_E_PCIERC2_CN81XX (0xd0)
#define CAVM_PCC_DEV_CON_E_PCIERC2_CN88XX (0x100a0)
#define CAVM_PCC_DEV_CON_E_PCIERC2_CN83XX (0xd8)
#define CAVM_PCC_DEV_CON_E_PCIERC3_CN88XX (0x30080)
#define CAVM_PCC_DEV_CON_E_PCIERC3_CN83XX (0xe0)
#define CAVM_PCC_DEV_CON_E_PCIERC4 (0x30090)
#define CAVM_PCC_DEV_CON_E_PCIERC5 (0x300a0)
#define CAVM_PCC_DEV_CON_E_PCM (0x68)
#define CAVM_PCC_DEV_CON_E_PEMX(a) (0x170 + (a))
#define CAVM_PCC_DEV_CON_E_PKI (0x10200)
#define CAVM_PCC_DEV_CON_E_PKO (0x10300)
#define CAVM_PCC_DEV_CON_E_RAD_CN88XX (0x200)
#define CAVM_PCC_DEV_CON_E_RAD_CN83XX (0x600)
#define CAVM_PCC_DEV_CON_E_RGXX(a) (0x190 + (a))
#define CAVM_PCC_DEV_CON_E_RNM_CN81XX (0x200)
#define CAVM_PCC_DEV_CON_E_RNM_CN88XX (0x48)
#define CAVM_PCC_DEV_CON_E_RNM_CN83XX (0x200)
#define CAVM_PCC_DEV_CON_E_RST (0x101)
#define CAVM_PCC_DEV_CON_E_SATA0_CN81XX (0xb0)
#define CAVM_PCC_DEV_CON_E_SATA0_CN88XX (0x10020)
#define CAVM_PCC_DEV_CON_E_SATA0_CN83XX (0x10020)
#define CAVM_PCC_DEV_CON_E_SATA1_CN81XX (0xb8)
#define CAVM_PCC_DEV_CON_E_SATA1_CN88XX (0x10028)
#define CAVM_PCC_DEV_CON_E_SATA1_CN83XX (0x10028)
#define CAVM_PCC_DEV_CON_E_SATA10 (0x30030)
#define CAVM_PCC_DEV_CON_E_SATA11 (0x30038)
#define CAVM_PCC_DEV_CON_E_SATA12 (0x30040)
#define CAVM_PCC_DEV_CON_E_SATA13 (0x30048)
#define CAVM_PCC_DEV_CON_E_SATA14 (0x30050)
#define CAVM_PCC_DEV_CON_E_SATA15 (0x30058)
#define CAVM_PCC_DEV_CON_E_SATA2 (0x10030)
#define CAVM_PCC_DEV_CON_E_SATA3 (0x10038)
#define CAVM_PCC_DEV_CON_E_SATA4 (0x10040)
#define CAVM_PCC_DEV_CON_E_SATA5 (0x10048)
#define CAVM_PCC_DEV_CON_E_SATA6 (0x10050)
#define CAVM_PCC_DEV_CON_E_SATA7 (0x10058)
#define CAVM_PCC_DEV_CON_E_SATA8 (0x30020)
#define CAVM_PCC_DEV_CON_E_SATA9 (0x30028)
#define CAVM_PCC_DEV_CON_E_SGP (0x10a)
#define CAVM_PCC_DEV_CON_E_SLI0_CN81XX (0x70)
#define CAVM_PCC_DEV_CON_E_SLI0_CN88XX (0x10010)
#define CAVM_PCC_DEV_CON_E_SLI1 (0x30010)
#define CAVM_PCC_DEV_CON_E_SLIRE0 (0xc0)
#define CAVM_PCC_DEV_CON_E_SMI (0x10b)
#define CAVM_PCC_DEV_CON_E_SMMU0 (0x10)
#define CAVM_PCC_DEV_CON_E_SMMU1 (0x10008)
#define CAVM_PCC_DEV_CON_E_SMMU2 (0x20008)
#define CAVM_PCC_DEV_CON_E_SMMU3 (0x30008)
#define CAVM_PCC_DEV_CON_E_SSO (0x700)
#define CAVM_PCC_DEV_CON_E_SSOW (0x800)
#define CAVM_PCC_DEV_CON_E_TIM (0xa00)
#define CAVM_PCC_DEV_CON_E_TNS (0x20018)
#define CAVM_PCC_DEV_CON_E_UAAX(a) (0x140 + (a))
#define CAVM_PCC_DEV_CON_E_USBDRDX_CN81XX(a) (0x80 + 8 * (a))
#define CAVM_PCC_DEV_CON_E_USBDRDX_CN83XX(a) (0x10060 + 8 * (a))
#define CAVM_PCC_DEV_CON_E_USBHX(a) (0x80 + 8 * (a))
#define CAVM_PCC_DEV_CON_E_VRMX(a) (0x144 + (a))
#define CAVM_PCC_DEV_CON_E_XCVX(a) (0x110 + (a))
#define CAVM_PCC_DEV_CON_E_ZIP_CN88XX (0x300)
#define CAVM_PCC_DEV_CON_E_ZIP_CN83XX (0x10400)

/**
 * Enumeration pcc_dev_idl_e
 *
 * PCC Device ID Low Enumeration
 * Enumerates the values of the PCI configuration header Device ID and Subsystem Device ID bits
 * <7:0>. The value of the enumeration is formatted as defined by PCC_DEV_IDL_S.
 */
#define CAVM_PCC_DEV_IDL_E_BCH (0x43)
#define CAVM_PCC_DEV_IDL_E_BCH_VF (0x44)
#define CAVM_PCC_DEV_IDL_E_BGX (0x26)
#define CAVM_PCC_DEV_IDL_E_CHIP (0)
#define CAVM_PCC_DEV_IDL_E_CHIP_VF (3)
#define CAVM_PCC_DEV_IDL_E_CPT (0x40)
#define CAVM_PCC_DEV_IDL_E_CPT_VF (0x41)
#define CAVM_PCC_DEV_IDL_E_DAP (0x2c)
#define CAVM_PCC_DEV_IDL_E_DDF (0x45)
#define CAVM_PCC_DEV_IDL_E_DDF_VF (0x46)
#define CAVM_PCC_DEV_IDL_E_DFA (0x19)
#define CAVM_PCC_DEV_IDL_E_DPI (0x57)
#define CAVM_PCC_DEV_IDL_E_DPI_VF (0x58)
#define CAVM_PCC_DEV_IDL_E_FPA (0x52)
#define CAVM_PCC_DEV_IDL_E_FPA_VF (0x53)
#define CAVM_PCC_DEV_IDL_E_FUSF (0x32)
#define CAVM_PCC_DEV_IDL_E_GIC (9)
#define CAVM_PCC_DEV_IDL_E_GPIO (0xa)
#define CAVM_PCC_DEV_IDL_E_GSER (0x25)
#define CAVM_PCC_DEV_IDL_E_GTI (0x17)
#define CAVM_PCC_DEV_IDL_E_IOBN (0x27)
#define CAVM_PCC_DEV_IDL_E_KEY (0x16)
#define CAVM_PCC_DEV_IDL_E_L2C (0x21)
#define CAVM_PCC_DEV_IDL_E_L2C_CBC (0x2f)
#define CAVM_PCC_DEV_IDL_E_L2C_MCI (0x30)
#define CAVM_PCC_DEV_IDL_E_L2C_TAD (0x2e)
#define CAVM_PCC_DEV_IDL_E_LBK (0x42)
#define CAVM_PCC_DEV_IDL_E_LMC (0x22)
#define CAVM_PCC_DEV_IDL_E_MIO_BOOT (0x11)
#define CAVM_PCC_DEV_IDL_E_MIO_EMM (0x10)
#define CAVM_PCC_DEV_IDL_E_MIO_FUS (0x31)
#define CAVM_PCC_DEV_IDL_E_MIO_PTP (0xc)
#define CAVM_PCC_DEV_IDL_E_MIO_TWS (0x12)
#define CAVM_PCC_DEV_IDL_E_MIX (0xd)
#define CAVM_PCC_DEV_IDL_E_MPI (0xb)
#define CAVM_PCC_DEV_IDL_E_MRML (1)
#define CAVM_PCC_DEV_IDL_E_NCSI (0x29)
#define CAVM_PCC_DEV_IDL_E_NDF (0x4f)
#define CAVM_PCC_DEV_IDL_E_NIC (0x1e)
#define CAVM_PCC_DEV_IDL_E_NIC_VF (0x34)
#define CAVM_PCC_DEV_IDL_E_OCLA (0x23)
#define CAVM_PCC_DEV_IDL_E_OCX (0x13)
#define CAVM_PCC_DEV_IDL_E_OSM (0x24)
#define CAVM_PCC_DEV_IDL_E_PBUS (0x35)
#define CAVM_PCC_DEV_IDL_E_PCCBR (2)
#define CAVM_PCC_DEV_IDL_E_PCIERC (0x2d)
#define CAVM_PCC_DEV_IDL_E_PCM (0x4e)
#define CAVM_PCC_DEV_IDL_E_PEM (0x20)
#define CAVM_PCC_DEV_IDL_E_PKI (0x47)
#define CAVM_PCC_DEV_IDL_E_PKO (0x48)
#define CAVM_PCC_DEV_IDL_E_PKO_VF (0x49)
#define CAVM_PCC_DEV_IDL_E_RAD (0x1d)
#define CAVM_PCC_DEV_IDL_E_RAD_VF (0x36)
#define CAVM_PCC_DEV_IDL_E_RGX (0x54)
#define CAVM_PCC_DEV_IDL_E_RNM (0x18)
#define CAVM_PCC_DEV_IDL_E_RNM_VF (0x33)
#define CAVM_PCC_DEV_IDL_E_RST (0xe)
#define CAVM_PCC_DEV_IDL_E_SATA (0x1c)
#define CAVM_PCC_DEV_IDL_E_SGP (0x2a)
#define CAVM_PCC_DEV_IDL_E_SLI (0x15)
#define CAVM_PCC_DEV_IDL_E_SLIRE (0x38)
#define CAVM_PCC_DEV_IDL_E_SMI (0x2b)
#define CAVM_PCC_DEV_IDL_E_SMMU (8)
#define CAVM_PCC_DEV_IDL_E_SSO (0x4a)
#define CAVM_PCC_DEV_IDL_E_SSOW (0x4c)
#define CAVM_PCC_DEV_IDL_E_SSOW_VF (0x4d)
#define CAVM_PCC_DEV_IDL_E_SSO_VF (0x4b)
#define CAVM_PCC_DEV_IDL_E_TIM (0x50)
#define CAVM_PCC_DEV_IDL_E_TIM_VF (0x51)
#define CAVM_PCC_DEV_IDL_E_TNS (0x1f)
#define CAVM_PCC_DEV_IDL_E_UAA (0xf)
#define CAVM_PCC_DEV_IDL_E_USBDRD (0x55)
#define CAVM_PCC_DEV_IDL_E_USBH (0x1b)
#define CAVM_PCC_DEV_IDL_E_VRM (0x14)
#define CAVM_PCC_DEV_IDL_E_XCV (0x56)
#define CAVM_PCC_DEV_IDL_E_ZIP (0x1a)
#define CAVM_PCC_DEV_IDL_E_ZIP_VF (0x37)

/**
 * Enumeration pcc_pidr_partnum0_e
 *
 * PCC PIDR Part Number 0 Enumeration
 * When *_PIDR1[PARTNUM1] = PCC_PIDR_PARTNUM1_E::COMP, enumerates the values of Cavium-
 * assigned CoreSight PIDR part number 0 fields.
 * For example SMMU()_PIDR0[PARTNUM0].
 */
#define CAVM_PCC_PIDR_PARTNUM0_E_CTI (0xd)
#define CAVM_PCC_PIDR_PARTNUM0_E_DBG (0xe)
#define CAVM_PCC_PIDR_PARTNUM0_E_GICD (2)
#define CAVM_PCC_PIDR_PARTNUM0_E_GICR (1)
#define CAVM_PCC_PIDR_PARTNUM0_E_GITS (3)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_BZ (4)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_CC (5)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_CTL (6)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_RD (7)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_WC (8)
#define CAVM_PCC_PIDR_PARTNUM0_E_GTI_WR (9)
#define CAVM_PCC_PIDR_PARTNUM0_E_NONE (0)
#define CAVM_PCC_PIDR_PARTNUM0_E_PMU (0xa)
#define CAVM_PCC_PIDR_PARTNUM0_E_SMMU (0xb)
#define CAVM_PCC_PIDR_PARTNUM0_E_SYSCTI (0xf)
#define CAVM_PCC_PIDR_PARTNUM0_E_TRC (0x10)
#define CAVM_PCC_PIDR_PARTNUM0_E_UAA (0xc)

/**
 * Enumeration pcc_pidr_partnum1_e
 *
 * PCC PIDR Part Number 1 Enumeration
 * Enumerates the values of Cavium-assigned CoreSight PIDR PARTNUM1 fields, for example
 * SMMU()_PIDR1[PARTNUM1].
 */
#define CAVM_PCC_PIDR_PARTNUM1_E_COMP (2)
#define CAVM_PCC_PIDR_PARTNUM1_E_PROD (1)

/**
 * Enumeration pcc_prod_e
 *
 * PCC Device ID Product Enumeration
 * Enumerates the chip identifier.
 */
#define CAVM_PCC_PROD_E_CN81XX (0xa2)
#define CAVM_PCC_PROD_E_CN83XX (0xa3)
#define CAVM_PCC_PROD_E_CN88XX (0xa1)
#define CAVM_PCC_PROD_E_GEN (0xa0)

/**
 * Enumeration pcc_vendor_e
 *
 * PCC Vendor ID Enumeration
 * Enumerates the values of the PCI configuration header vendor ID.
 */
#define CAVM_PCC_VENDOR_E_CAVIUM (0x177d)
/**
 * Register (PCCPF) pccpf_xxx_id
 *
 * PCC PF Vendor and Device ID Register
 * This register is the header of the 64-byte PCI type 0 configuration structure.
 */
union cavm_pccpf_xxx_id
{
    uint32_t u;
    struct cavm_pccpf_xxx_id_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t devid                 : 16; /**< [ 31: 16](RO) Device ID. <15:8> is PCC_PROD_E::GEN. <7:0> enumerated by PCC_DEV_IDL_E.

                                                                 Internal:
                                                                 Unit
                                                                 from PCC's tie__pfunitid. */
        uint32_t vendid                : 16; /**< [ 15:  0](RO) Cavium's vendor ID. Enumerated by PCC_VENDOR_E::CAVIUM. */
#else /* Word 0 - Little Endian */
        uint32_t vendid                : 16; /**< [ 15:  0](RO) Cavium's vendor ID. Enumerated by PCC_VENDOR_E::CAVIUM. */
        uint32_t devid                 : 16; /**< [ 31: 16](RO) Device ID. <15:8> is PCC_PROD_E::GEN. <7:0> enumerated by PCC_DEV_IDL_E.

                                                                 Internal:
                                                                 Unit
                                                                 from PCC's tie__pfunitid. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_pccpf_xxx_id_s cn; */
};
typedef union cavm_pccpf_xxx_id cavm_pccpf_xxx_id_t;

#define CAVM_PCCPF_XXX_ID CAVM_PCCPF_XXX_ID_FUNC()
static inline uint64_t CAVM_PCCPF_XXX_ID_FUNC(void) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_PCCPF_XXX_ID_FUNC(void)
{
    return 0;
}

#define typedef_CAVM_PCCPF_XXX_ID cavm_pccpf_xxx_id_t
#define bustype_CAVM_PCCPF_XXX_ID CSR_TYPE_PCCPF
#define basename_CAVM_PCCPF_XXX_ID "PCCPF_XXX_ID"
#define busnum_CAVM_PCCPF_XXX_ID 0
#define arguments_CAVM_PCCPF_XXX_ID -1,-1,-1,-1

#endif /* __CAVM_CSRS_PCCPF_H__ */

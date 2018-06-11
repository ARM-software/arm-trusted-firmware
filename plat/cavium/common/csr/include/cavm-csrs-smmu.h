#ifndef __CAVM_CSRS_SMMU_H__
#define __CAVM_CSRS_SMMU_H__
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
 * Cavium SMMU.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Enumeration smmu_bar_e
 *
 * SMMU Base Address Register Enumeration
 * Enumerates the base address registers.
 */
#define CAVM_SMMU_BAR_E_SMMUX_PF_BAR0(a) (0x830000000000ll + 0x1000000000ll * (a))
#define CAVM_SMMU_BAR_E_SMMUX_PF_BAR0_SIZE 0x8000000ull
#define CAVM_SMMU_BAR_E_SMMUX_PF_BAR4(a) (0x83000f000000ll + 0x1000000000ll * (a))
#define CAVM_SMMU_BAR_E_SMMUX_PF_BAR4_SIZE 0x100000ull

/**
 * Register (NCB32b) smmu#_nscr0
 *
 * SMMU Secure Alias for Configuration Register 0
 * Provides secure alias for nonsecure register SMMU()_(S)CR0.
 */
union cavm_smmux_nscr0
{
    uint32_t u;
    struct cavm_smmux_nscr0_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t vmid16en              : 1;  /**< [ 31: 31](R/W) Handling of 16-bit VMID extension:
                                                                 0 = 8-bit VMIDs are in use.
                                                                 The VMID is held in SMMU()_CBAR()[VMID].
                                                                 The SMMU()_CBA2R()[VMID16] field is 0.

                                                                 1 = 16-bit VMIDs are in use.
                                                                 The VMID is held in SMMU()_CBA2R()[VMID16].
                                                                 The SMMU()_CBAR()[VMID] field is 0.

                                                                 Only exists in the nonsecure copy of this register.

                                                                 In pass 1, this field must be 0. */
        uint32_t hypmode               : 1;  /**< [ 30: 30](R/W) Hypervisor mode. Selects which hypervisor context is used:
                                                                   0 = When SMMU()_CBAR()[CTYPE] = 0x1, SMMU()_CBAR()<10> represents HYPC.
                                                                   1 = When SMMU()_CBAR()[CTYPE] = 0x1, SMMU()_CBAR()<10> represents E2HC.

                                                                 If this bit is changed, TLB maintenance is required.

                                                                 Only exists in the non secure copy of this register. */
        uint32_t nscfg                 : 2;  /**< [ 29: 28](SR/W) Nonsecure configuration. Only exist in secure copy of register, RES0 in nonsecure copy.
                                                                 This field only applies to secure transactions bypassing the SMMU stream mapping
                                                                 process.
                                                                 0x0 = Use default NS attribute.
                                                                 0x1 = Reserved.
                                                                 0x2 = Secure.
                                                                 0x3 = Nonsecure. */
        uint32_t wacfg                 : 2;  /**< [ 27: 26](RO) Write-allocate configuration, controls the allocation hint for write accesses. This field
                                                                 applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Write-allocate.
                                                                 0x3 = No write-allocate.

                                                                 Ignored in CNXXXX. */
        uint32_t racfg                 : 2;  /**< [ 25: 24](RO) Read-allocate configuration, controls the allocation hint for read accesses. This field
                                                                 applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Read-allocate.
                                                                 0x3 = No read-allocate.

                                                                 Ignored in CNXXXX. */
        uint32_t shcfg                 : 2;  /**< [ 23: 22](RO) Shared configuration. Applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default shareable attributes.
                                                                 0x1 = Outer sharable.
                                                                 0x2 = Inner sharable.
                                                                 0x3 = Non-sharable.

                                                                 Ignored in CNXXXX. */
        uint32_t smcfcfg               : 1;  /**< [ 21: 21](RO) Stream match conflict fault configuration. Controls transactions with multiple matches in
                                                                 the stream mapping table.
                                                                 0 = Permit the transaction to bypass the SMMU.
                                                                 1 = Raise a stream match conflict fault.

                                                                 CNXXXX detects all stream match conflicts and always faults. */
        uint32_t mtcfg                 : 1;  /**< [ 20: 20](R/W) Memory type configuration, applies to transactions that bypass the stream mapping table.
                                                                 0 = Use the default memory attributes.
                                                                 1 = Use the MEMATTR field for memory attributes. */
        uint32_t memattr               : 4;  /**< [ 19: 16](R/W) Memory attributes for bypass transactions if MTCFG == 1. */
        uint32_t bsu                   : 2;  /**< [ 15: 14](RO) Barrier sharability upgrade. Upgrades the sharability of barriers issued by client devices
                                                                 that are not mapped to a translation context bank by setting the minimum sharability
                                                                 domain applied to any barrier.  Applies to transactions bypassing the stream mapping
                                                                 table.
                                                                 0x0 = No effect.
                                                                 0x1 = Inner sharable.
                                                                 0x2 = Outer sharable.
                                                                 0x3 = Full system.

                                                                 Ignored in CNXXXX. */
        uint32_t fb                    : 1;  /**< [ 13: 13](R/W) Force broadcast of TLB and instruction cache maintenance operations. Applies to
                                                                 transactions bypassing the stream mapping table. Affects client TLB maintenance, BPIALL
                                                                 and ICIALLU operations. If FB=1, any affected operation is modified to the equivalent
                                                                 broadcast variant in the inner shareable domain.
                                                                 0 = Process affected operations as presented.
                                                                 1 = Upgrade affected operations to be broadcast within the inner sharable domain.

                                                                 Ignored in CNXXXX, as NCB clients do not initiate invalidates. */
        uint32_t ptm                   : 1;  /**< [ 12: 12](R/W) Private TLB maintenance hint.
                                                                 0 = Process affected operations as presented.
                                                                 1 = SMMU TLBs are privately managed and are not require to respond to broadcast TLB
                                                                 maintenance operations from the wider system.

                                                                 CNXXXX honors this hint, but both the secure and nonsecure version must be set for
                                                                 private management.

                                                                 Internal:
                                                                 Note when set SMMU always still sends a response for the
                                                                 invalidates, it just does not perform the invalidation action against the TLB/WCU. */
        uint32_t vmidpne               : 1;  /**< [ 11: 11](RO) Reserved. */
        uint32_t usfcfg                : 1;  /**< [ 10: 10](R/W) Unidentified stream fault configuration.
                                                                 0 = Permit any transaction that does not match any entries in the stream matching table to
                                                                 pass through.
                                                                 1 = Raise an unidentified stream fault on any transaction that does not match any stream
                                                                 matching table entry. */
        uint32_t gse                   : 1;  /**< [  9:  9](RO) Global stall enable.
                                                                 0 = Do not enforce global stalling across contexts.
                                                                 1 = Enforce global stalling across contexts.

                                                                 In CNXXXX always 0, no stalling. */
        uint32_t stalld                : 1;  /**< [  8:  8](RO) Stall disable.
                                                                 0 = Permit per-context stalling on context faults.
                                                                 1 = Disable per-context stalling on context faults, or not programmable.

                                                                 In CNXXXX always 1, no stalling. */
        uint32_t transientcfg          : 2;  /**< [  7:  6](RO) Transient configuration controls the transient allocation hint.
                                                                 0x0 = Default transient allocation attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Non-transient.
                                                                 0x3 = Transient.

                                                                 Not implemented in CNXXXX. */
        uint32_t gcfgfie               : 1;  /**< [  5:  5](RO) Global configuration fault interrupt enable.
                                                                 0 = Do not raise an interrupt on a global configuration fault.
                                                                 1 = Raise an interrupt on a global configuration fault.

                                                                 In CNXXXX always zero, as does not support configuration faults. */
        uint32_t gcfgfre               : 1;  /**< [  4:  4](RO) Global configuration fault report enable.
                                                                 0 = Do not return an abort on a global configuration fault
                                                                 1 = Return an abort on a global configuration fault.

                                                                 In CNXXXX always zero, as does not support configuration faults. */
        uint32_t exidenable            : 1;  /**< [  3:  3](R/W) Extended stream ID enable.
                                                                 0 = For this SSD, SMMU()_SMR() has the format with the VALID bit in the
                                                                 SMMU()_SMR(). The SMMU()_S2CR()[EXIDVALID] is ignored.
                                                                 1 = For this SSD, SMMU()_SMR() has the extended id format and the valid bit is
                                                                 held in the corresponding SMMU()_S2CR()[EXIDVALID].

                                                                 Software should only change [EXIDENABLE] when all SMMU()_S2CR()[EXIDVALID] == 0
                                                                 and SMMU()_SMR()[EXMASK[15]/VALID] == 0 for the appropriate security world,
                                                                 otherwise the effect is unpredictable. In particular, note that the reset values of
                                                                 SMMU()_S2CR() and SMMU()_SMR() are unknown and so need to be
                                                                 initialized before use. */
        uint32_t gfie                  : 1;  /**< [  2:  2](R/W) Global fault interrupt enable.
                                                                 0 = Do not raise an interrupt on a global fault.
                                                                 1 = Raise an interrupt on a global fault. */
        uint32_t gfre                  : 1;  /**< [  1:  1](R/W) Global fault report enable.
                                                                 0 = Do not return an abort on a global fault.
                                                                 1 = Return an abort on a global fault. */
        uint32_t clientpd              : 1;  /**< [  0:  0](R/W) Client port disable.
                                                                 0 = Each SMMU client access is subject to translation, access control and attribute
                                                                 generation.
                                                                 1 = Each SMMU client access bypasses translation, access control and attribute generation. */
#else /* Word 0 - Little Endian */
        uint32_t clientpd              : 1;  /**< [  0:  0](R/W) Client port disable.
                                                                 0 = Each SMMU client access is subject to translation, access control and attribute
                                                                 generation.
                                                                 1 = Each SMMU client access bypasses translation, access control and attribute generation. */
        uint32_t gfre                  : 1;  /**< [  1:  1](R/W) Global fault report enable.
                                                                 0 = Do not return an abort on a global fault.
                                                                 1 = Return an abort on a global fault. */
        uint32_t gfie                  : 1;  /**< [  2:  2](R/W) Global fault interrupt enable.
                                                                 0 = Do not raise an interrupt on a global fault.
                                                                 1 = Raise an interrupt on a global fault. */
        uint32_t exidenable            : 1;  /**< [  3:  3](R/W) Extended stream ID enable.
                                                                 0 = For this SSD, SMMU()_SMR() has the format with the VALID bit in the
                                                                 SMMU()_SMR(). The SMMU()_S2CR()[EXIDVALID] is ignored.
                                                                 1 = For this SSD, SMMU()_SMR() has the extended id format and the valid bit is
                                                                 held in the corresponding SMMU()_S2CR()[EXIDVALID].

                                                                 Software should only change [EXIDENABLE] when all SMMU()_S2CR()[EXIDVALID] == 0
                                                                 and SMMU()_SMR()[EXMASK[15]/VALID] == 0 for the appropriate security world,
                                                                 otherwise the effect is unpredictable. In particular, note that the reset values of
                                                                 SMMU()_S2CR() and SMMU()_SMR() are unknown and so need to be
                                                                 initialized before use. */
        uint32_t gcfgfre               : 1;  /**< [  4:  4](RO) Global configuration fault report enable.
                                                                 0 = Do not return an abort on a global configuration fault
                                                                 1 = Return an abort on a global configuration fault.

                                                                 In CNXXXX always zero, as does not support configuration faults. */
        uint32_t gcfgfie               : 1;  /**< [  5:  5](RO) Global configuration fault interrupt enable.
                                                                 0 = Do not raise an interrupt on a global configuration fault.
                                                                 1 = Raise an interrupt on a global configuration fault.

                                                                 In CNXXXX always zero, as does not support configuration faults. */
        uint32_t transientcfg          : 2;  /**< [  7:  6](RO) Transient configuration controls the transient allocation hint.
                                                                 0x0 = Default transient allocation attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Non-transient.
                                                                 0x3 = Transient.

                                                                 Not implemented in CNXXXX. */
        uint32_t stalld                : 1;  /**< [  8:  8](RO) Stall disable.
                                                                 0 = Permit per-context stalling on context faults.
                                                                 1 = Disable per-context stalling on context faults, or not programmable.

                                                                 In CNXXXX always 1, no stalling. */
        uint32_t gse                   : 1;  /**< [  9:  9](RO) Global stall enable.
                                                                 0 = Do not enforce global stalling across contexts.
                                                                 1 = Enforce global stalling across contexts.

                                                                 In CNXXXX always 0, no stalling. */
        uint32_t usfcfg                : 1;  /**< [ 10: 10](R/W) Unidentified stream fault configuration.
                                                                 0 = Permit any transaction that does not match any entries in the stream matching table to
                                                                 pass through.
                                                                 1 = Raise an unidentified stream fault on any transaction that does not match any stream
                                                                 matching table entry. */
        uint32_t vmidpne               : 1;  /**< [ 11: 11](RO) Reserved. */
        uint32_t ptm                   : 1;  /**< [ 12: 12](R/W) Private TLB maintenance hint.
                                                                 0 = Process affected operations as presented.
                                                                 1 = SMMU TLBs are privately managed and are not require to respond to broadcast TLB
                                                                 maintenance operations from the wider system.

                                                                 CNXXXX honors this hint, but both the secure and nonsecure version must be set for
                                                                 private management.

                                                                 Internal:
                                                                 Note when set SMMU always still sends a response for the
                                                                 invalidates, it just does not perform the invalidation action against the TLB/WCU. */
        uint32_t fb                    : 1;  /**< [ 13: 13](R/W) Force broadcast of TLB and instruction cache maintenance operations. Applies to
                                                                 transactions bypassing the stream mapping table. Affects client TLB maintenance, BPIALL
                                                                 and ICIALLU operations. If FB=1, any affected operation is modified to the equivalent
                                                                 broadcast variant in the inner shareable domain.
                                                                 0 = Process affected operations as presented.
                                                                 1 = Upgrade affected operations to be broadcast within the inner sharable domain.

                                                                 Ignored in CNXXXX, as NCB clients do not initiate invalidates. */
        uint32_t bsu                   : 2;  /**< [ 15: 14](RO) Barrier sharability upgrade. Upgrades the sharability of barriers issued by client devices
                                                                 that are not mapped to a translation context bank by setting the minimum sharability
                                                                 domain applied to any barrier.  Applies to transactions bypassing the stream mapping
                                                                 table.
                                                                 0x0 = No effect.
                                                                 0x1 = Inner sharable.
                                                                 0x2 = Outer sharable.
                                                                 0x3 = Full system.

                                                                 Ignored in CNXXXX. */
        uint32_t memattr               : 4;  /**< [ 19: 16](R/W) Memory attributes for bypass transactions if MTCFG == 1. */
        uint32_t mtcfg                 : 1;  /**< [ 20: 20](R/W) Memory type configuration, applies to transactions that bypass the stream mapping table.
                                                                 0 = Use the default memory attributes.
                                                                 1 = Use the MEMATTR field for memory attributes. */
        uint32_t smcfcfg               : 1;  /**< [ 21: 21](RO) Stream match conflict fault configuration. Controls transactions with multiple matches in
                                                                 the stream mapping table.
                                                                 0 = Permit the transaction to bypass the SMMU.
                                                                 1 = Raise a stream match conflict fault.

                                                                 CNXXXX detects all stream match conflicts and always faults. */
        uint32_t shcfg                 : 2;  /**< [ 23: 22](RO) Shared configuration. Applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default shareable attributes.
                                                                 0x1 = Outer sharable.
                                                                 0x2 = Inner sharable.
                                                                 0x3 = Non-sharable.

                                                                 Ignored in CNXXXX. */
        uint32_t racfg                 : 2;  /**< [ 25: 24](RO) Read-allocate configuration, controls the allocation hint for read accesses. This field
                                                                 applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Read-allocate.
                                                                 0x3 = No read-allocate.

                                                                 Ignored in CNXXXX. */
        uint32_t wacfg                 : 2;  /**< [ 27: 26](RO) Write-allocate configuration, controls the allocation hint for write accesses. This field
                                                                 applies to transactions that bypass the stream mapping table.
                                                                 0x0 = Default attributes.
                                                                 0x1 = Reserved.
                                                                 0x2 = Write-allocate.
                                                                 0x3 = No write-allocate.

                                                                 Ignored in CNXXXX. */
        uint32_t nscfg                 : 2;  /**< [ 29: 28](SR/W) Nonsecure configuration. Only exist in secure copy of register, RES0 in nonsecure copy.
                                                                 This field only applies to secure transactions bypassing the SMMU stream mapping
                                                                 process.
                                                                 0x0 = Use default NS attribute.
                                                                 0x1 = Reserved.
                                                                 0x2 = Secure.
                                                                 0x3 = Nonsecure. */
        uint32_t hypmode               : 1;  /**< [ 30: 30](R/W) Hypervisor mode. Selects which hypervisor context is used:
                                                                   0 = When SMMU()_CBAR()[CTYPE] = 0x1, SMMU()_CBAR()<10> represents HYPC.
                                                                   1 = When SMMU()_CBAR()[CTYPE] = 0x1, SMMU()_CBAR()<10> represents E2HC.

                                                                 If this bit is changed, TLB maintenance is required.

                                                                 Only exists in the non secure copy of this register. */
        uint32_t vmid16en              : 1;  /**< [ 31: 31](R/W) Handling of 16-bit VMID extension:
                                                                 0 = 8-bit VMIDs are in use.
                                                                 The VMID is held in SMMU()_CBAR()[VMID].
                                                                 The SMMU()_CBA2R()[VMID16] field is 0.

                                                                 1 = 16-bit VMIDs are in use.
                                                                 The VMID is held in SMMU()_CBA2R()[VMID16].
                                                                 The SMMU()_CBAR()[VMID] field is 0.

                                                                 Only exists in the nonsecure copy of this register.

                                                                 In pass 1, this field must be 0. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_smmux_nscr0_s cn; */
};
typedef union cavm_smmux_nscr0 cavm_smmux_nscr0_t;

static inline uint64_t CAVM_SMMUX_NSCR0(unsigned long a) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SMMUX_NSCR0(unsigned long a)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && (a==0))
        return 0x830000000400ll + 0x1000000000ll * ((a) & 0x0);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && (a<=1))
        return 0x830000000400ll + 0x1000000000ll * ((a) & 0x1);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && (a<=3))
        return 0x830000000400ll + 0x1000000000ll * ((a) & 0x3);
    __cavm_csr_fatal("SMMUX_NSCR0", 1, a, 0, 0, 0);
   return 0;
}

#define typedef_CAVM_SMMUX_NSCR0(a) cavm_smmux_nscr0_t
#define bustype_CAVM_SMMUX_NSCR0(a) CSR_TYPE_NCB32b
#define basename_CAVM_SMMUX_NSCR0(a) "SMMUX_NSCR0"
#define device_bar_CAVM_SMMUX_NSCR0(a) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SMMUX_NSCR0(a) (a)
#define arguments_CAVM_SMMUX_NSCR0(a) (a),-1,-1,-1

/**
 * Register (NCB32b) smmu#_ssdr#
 *
 * SMMU Security State Definition Register
 * Internal:
 * INTERNAL: Address offset from SMM_GSSD_BASE.
 */
union cavm_smmux_ssdrx
{
    uint32_t u;
    struct cavm_smmux_ssdrx_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint32_t ssd                   : 32; /**< [ 31:  0](SR/W) Security state definition, where each bit corresponds to a SSD index:
                                                                 0x0 = Corresponding SSD index is acting for the secure domain.
                                                                 0x1 = Nonsecure domain.

                                                                 Only accessible to secure software, else RAZ/WI. CNXXXX indexes this table by stream
                                                                 identifier, and uses the result of this table to also promote interrupts to secure
                                                                 interrupts. */
#else /* Word 0 - Little Endian */
        uint32_t ssd                   : 32; /**< [ 31:  0](SR/W) Security state definition, where each bit corresponds to a SSD index:
                                                                 0x0 = Corresponding SSD index is acting for the secure domain.
                                                                 0x1 = Nonsecure domain.

                                                                 Only accessible to secure software, else RAZ/WI. CNXXXX indexes this table by stream
                                                                 identifier, and uses the result of this table to also promote interrupts to secure
                                                                 interrupts. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_smmux_ssdrx_s cn; */
};
typedef union cavm_smmux_ssdrx cavm_smmux_ssdrx_t;

static inline uint64_t CAVM_SMMUX_SSDRX(unsigned long a, unsigned long b) __attribute__ ((pure, always_inline));
static inline uint64_t CAVM_SMMUX_SSDRX(unsigned long a, unsigned long b)
{
    if (CAVIUM_IS_MODEL(CAVIUM_CN81XX) && ((a==0) && (b<=2047)))
        return 0x830000040000ll + 0x1000000000ll * ((a) & 0x0) + 4ll * ((b) & 0x7ff);
    if (CAVIUM_IS_MODEL(CAVIUM_CN83XX) && ((a<=1) && (b<=2047)))
        return 0x830000040000ll + 0x1000000000ll * ((a) & 0x1) + 4ll * ((b) & 0x7ff);
    if (CAVIUM_IS_MODEL(CAVIUM_CN88XX) && ((a<=3) && (b<=2047)))
        return 0x830000040000ll + 0x1000000000ll * ((a) & 0x3) + 4ll * ((b) & 0x7ff);
    __cavm_csr_fatal("SMMUX_SSDRX", 2, a, b, 0, 0);
   return 0;
}

#define typedef_CAVM_SMMUX_SSDRX(a,b) cavm_smmux_ssdrx_t
#define bustype_CAVM_SMMUX_SSDRX(a,b) CSR_TYPE_NCB32b
#define basename_CAVM_SMMUX_SSDRX(a,b) "SMMUX_SSDRX"
#define device_bar_CAVM_SMMUX_SSDRX(a,b) 0x0 /* PF_BAR0 */
#define busnum_CAVM_SMMUX_SSDRX(a,b) (a)
#define arguments_CAVM_SMMUX_SSDRX(a,b) (a),(b),-1,-1

#endif /* __CAVM_CSRS_SMMU_H__ */

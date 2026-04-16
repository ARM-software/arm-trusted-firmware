/*
 * Copyright (c) 2025-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPER_H
#define CPER_H

#include <cdefs.h>
#include <stdbool.h>
#include <stdint.h>

#include <tools_share/uuid.h>

/* Generic Error Status Block */
#define ACPI_ESB_UE_VALID				(1U << 0)
#define ACPI_ESB_CE_VALID				(1U << 1)
#define ACPI_ESB_MULTI_UE				(1U << 2)
#define ACPI_ESB_MULTI_CE				(1U << 3)
#define ACPI_ESB_ENTRYCOUNT_SHIFT			U(4)
#define ACPI_ESB_ENTRYCOUNT_MASK			(U(0x3FF) << ACPI_ESB_ENTRYCOUNT_SHIFT)

#define ACPI_SEC_FLAG_LATENT				(1U << 5)

/* Error Data Entry */
#define ACPI_DATA_ENTRY_REV_0300			U(0x0300)
#define ACPI_DATA_ENTRY_SEV_RECOVERABLE			U(0)
#define ACPI_DATA_ENTRY_SEV_FATAL			U(1)
#define ACPI_DATA_ENTRY_SEV_CORRECTED			U(2)
#define ACPI_DATA_ENTRY_SEV_NONE			U(3)

/* Processor Error Section header flags */
#define EFI_ARM_PROC_ERROR_MPIDR_VALID			(1U << 0)
#define EFI_ARM_PROC_ERROR_AFFINITY_LEVEL_VALID		(1U << 1)
#define EFI_ARM_PROC_ERROR_RUNNING_STATE_VALID		(1U << 2)

/* Processor Error Information validation bits */
#define EFI_ARM_PROC_ERROR_INFO_MULTIPLE_ERROR_VALID	(1U << 0)
#define EFI_ARM_PROC_ERROR_INFO_FLAGS_VALID		(1U << 1)
#define EFI_ARM_PROC_ERROR_INFO_ERROR_INFO_VALID	(1U << 2)
#define EFI_ARM_PROC_ERROR_INFO_VIRT_ADDR_VALID		(1U << 3)
#define EFI_ARM_PROC_ERROR_INFO_PHYS_ADDR_VALID		(1U << 4)

/* Processor error types */
#define ARM_ERROR_TYPE_CACHE				U(2)
#define ARM_ERROR_TYPE_TLB				U(4)

/* Cache error encoding */
#define ARM_CACHE_ERR_TX_INSTR				U(0)
#define ARM_CACHE_ERR_TX_DATA				U(1)
#define ARM_CACHE_ERR_TX_GENERIC			U(2)
#define ARM_CACHE_ERR_OP_GENERIC			U(0)

#define ARM_CACHE_ERR_VLD_TX_TYPE			(1ULL << 0)
#define ARM_CACHE_ERR_VLD_OPERATION			(1ULL << 1)
#define ARM_CACHE_ERR_VLD_LEVEL				(1ULL << 2)
#define ARM_CACHE_ERR_VLD_PCC				(1ULL << 3)
#define ARM_CACHE_ERR_VLD_CORRECTED			(1ULL << 4)
#define ARM_CACHE_ERR_VLD_PRECISE_PC			(1ULL << 5)
#define ARM_CACHE_ERR_VLD_RESTART_PC			(1ULL << 6)
#define ARM_CACHE_ERR_TX_SHIFT				16
#define ARM_CACHE_ERR_TX_MASK				(0x3ULL << ARM_CACHE_ERR_TX_SHIFT)
#define ARM_CACHE_ERR_OP_SHIFT				18
#define ARM_CACHE_ERR_OP_MASK				(0xFULL << ARM_CACHE_ERR_OP_SHIFT)
#define ARM_CACHE_ERR_LEVEL_SHIFT			22
#define ARM_CACHE_ERR_LEVEL_MASK			(0x7ULL << ARM_CACHE_ERR_LEVEL_SHIFT)
#define ARM_CACHE_ERR_PCC_BIT				25
#define ARM_CACHE_ERR_CORR_BIT				26
#define ARM_CACHE_ERR_PRECISE_PC_BIT			27
#define ARM_CACHE_ERR_RESTART_PC_BIT			28

/* Context information */
#define EFI_ARM_CONTEXT_INFO_REG_TYPE_4			U(4)
#define EFI_ARM_CONTEXT_INFO_REG_TYPE_5			U(5)

/* Record sizing */
#define CPU_ERR_INFO_NUM				U(1)
#define CPU_CONTEXT_INFO_NUM				U(2)

/*
 * Error Status Block (ESB)
 * See UEFI Spec:
 * https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/18_ACPI_Platform_Error_Interfaces/
 * error-source-discovery.html#generic-error-status-block
 *
 * @BlockStatus:   Bitfield indicating UE/CE presence and number of entries
 * @RawDataOffset: Offset to data following this header+entry
 * @RawDataLength: Length of data (0 if none)
 * @DataLength:    Length of all following CPER data (entries + payloads)
 * @ErrorSeverity: Error severity (CE/UE/DE)
 */
struct __packed ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER {
	uint32_t BlockStatus;
	uint32_t RawDataOffset;
	uint32_t RawDataLength;
	uint32_t DataLength;
	uint32_t ErrorSeverity;
};

/*
 * ACPI Error Data Entry Header
 * See UEFI Spec:
 * https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/18_ACPI_Platform_Error_Interfaces/
 * error-source-discovery.html#generic-error-data-entry
 *
 * @SectionType:    GUID (ARM PROCESSOR ERROR SECTION GUID)
 * @ErrorSeverity:  Severity for this entry (CE/UE/DE)
 * @Revision:       0x300
 * @ValidationBits: FRU/time presence flags (not used here)
 * @Flags:          Misc flags (e.g., overflow/latent)
 * @ErrorDataLength:Length in bytes of the payload following this header
 * @FruId:          Optional FRU GUID
 * @FruText:        Optional FRU text
 * @TimestampBCD:   Optional timestamp (BCD)
 */
struct __packed ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER {
	struct efi_guid SectionType;
	uint32_t ErrorSeverity;
	uint16_t Revision;
	uint8_t ValidationBits;
	uint8_t Flags;
	uint32_t ErrorDataLength;
	struct efi_guid FruId;
	uint8_t FruText[20];
	uint64_t TimestampBCD;
};

static const struct efi_guid ARM_PROC_ERR_SECTION_GUID =
	(struct efi_guid){ 0xE19E3D16,
			   0xBC11,
			   0x11E4,
			   { 0x9C, 0xAA, 0xC2, 0x05, 0x1D, 0x5D, 0x46, 0xB0 } };

/*
 * ARM Processor Error Section
 * See UEFI Spec:
 * https://uefi.org/specs/UEFI/2.11/
 * Apx_N_Common_Platform_Error_Record.html#arm-processor-error-section-format
 *
 * @ValidationBit:  Bitmask of valid header fields
 * @ErrInfoNum:     Number of error-info records
 * @ContextInfoNum: Number of context records (Type-4, Type-5/6)
 * @SectionLength:  Total length of this CPU payload (header+info+contexts)
 * @AffinityLevel:  Optional affinity level
 * @MPIDR_EL1:      MPIDR of the affected PE
 * @MIDR_EL1:       MIDR of the affected PE
 * @RunState:       Firmware-defined running state (1=running)
 * @PsciState:      Optional PSCI power state
 */
struct __packed EFI_ARM_PROCESSOR_ERROR_SECTION_HEADER {
	uint32_t ValidationBit;
	uint16_t ErrInfoNum;
	uint16_t ContextInfoNum;
	uint32_t SectionLength;
	uint8_t AffinityLevel;
	uint8_t Rsvd0[3];
	uint64_t MPIDR_EL1;
	uint64_t MIDR_EL1;
	uint32_t RunState;
	uint32_t PsciState;
};

/*
 * ARM Processor Error Information
 * See UEFI Spec:
 * https://uefi.org/specs/UEFI/2.11/
 * Apx_N_Common_Platform_Error_Record.html#arm-processor-error-information-structure
 *
 * Describes a single error instance associated with the processor.
 * Multiple entries may appear; this struct is one element of that array.
 * @Version:			Payload version (0)
 * @Length:			Size
 * @ValidationBit:		Bitmask of valid members
 * @Type:			ARM error type (cache/TLB)
 * @MultipleError:		Optional multi-error count (e.g., CE count)
 * @Flags:			Optional flags (overflow/first-error)
 * @ErrorInfo:			Implementation-defined 64-bit error info
 * @VirtualFaultAddress:	VA implicated (if any)
 * @PhysicalFaultAddress:	PA implicated (if any)
 */
struct __packed EFI_ARM_PROCESSOR_ERROR_INFORMATION {
	uint8_t Version;
	uint8_t Length;
	uint16_t ValidationBit;
	uint8_t Type;
	uint16_t MultipleError;
	uint8_t Flags;
	uint64_t ErrorInfo;
	uint64_t VirtualFaultAddress;
	uint64_t PhysicalFaultAddress;
};

/*
 * ARM Processor Error Context Information
 * See UEFI Spec:
 * https://uefi.org/specs/UEFI/2.11/
 * Apx_N_Common_Platform_Error_Record.html#arm-processor-error-context-information-headerstructure
 *
 * Identifies which kind of register context follows and its size.
 * @Version:               Context structure version (0)
 * @RegisterContextType:   4=GPR, 5=EL1 sysregs
 * @RegisterArraySize:     Size of the union payload for this block
 */
struct __packed EFI_ARM_PROCESSOR_ERROR_CONTEXT_INFO_HEADER {
	uint16_t Version;
	uint16_t RegisterContextType;
	uint32_t RegisterArraySize;
};

struct __packed EFI_ARM_AARCH64_CONTEXT_GPR {
	uint64_t X[31];
	uint64_t SP;
};

struct __packed EFI_ARM_AARCH64_EL1_CONTEXT_SYSTEM_REGISTERS {
	uint64_t ELR_EL1;
	uint64_t ESR_EL1;
	uint64_t FAR_EL1;
	uint64_t ISR_EL1;
	uint64_t MAIR_EL1;
	uint64_t MIDR_EL1;
	uint64_t MPIDR_EL1;
	uint64_t SCTLR_EL1;
	uint64_t SP_EL0;
	uint64_t SP_EL1;
	uint64_t SPSR_EL1;
	uint64_t TCR_EL1;
	uint64_t TPIDR_EL0;
	uint64_t TPIDR_EL1;
	uint64_t TPIDRRO_EL0;
	uint64_t TTBR0_EL1;
	uint64_t TTBR1_EL1;
};

struct __packed EFI_ARM_PROCESSOR_CONTEXT_INFORMATION {
	struct EFI_ARM_PROCESSOR_ERROR_CONTEXT_INFO_HEADER Hdr;
	union {
		struct EFI_ARM_AARCH64_CONTEXT_GPR Type4Gpr;
		struct EFI_ARM_AARCH64_EL1_CONTEXT_SYSTEM_REGISTERS Type5SysRegs;
	} RegisterArray;
	uint8_t Pad[8];
};

struct __packed EFI_ARM_PROCESSOR_ERROR_RECORD_DATA {
	struct EFI_ARM_PROCESSOR_ERROR_SECTION_HEADER CpuInfo;
	struct EFI_ARM_PROCESSOR_ERROR_INFORMATION CpuErrInfo[CPU_ERR_INFO_NUM];
	struct EFI_ARM_PROCESSOR_CONTEXT_INFORMATION
		CpuContextInfo[CPU_CONTEXT_INFO_NUM];
};

static inline uint32_t esb_block_status(bool uc_valid, bool ce_valid,
					   bool multi_ue, bool multi_ce,
					   uint16_t entry_count)
{
	uint32_t v = 0;

	if (uc_valid)
		v |= ACPI_ESB_UE_VALID;
	if (ce_valid)
		v |= ACPI_ESB_CE_VALID;
	if (multi_ue)
		v |= ACPI_ESB_MULTI_UE;
	if (multi_ce)
		v |= ACPI_ESB_MULTI_CE;
	v |= ((uint32_t)entry_count << ACPI_ESB_ENTRYCOUNT_SHIFT) &
	     ACPI_ESB_ENTRYCOUNT_MASK;
	return v;
}

size_t cper_write_cpu_record(void *buf, size_t buf_size);

/* Dump the CPER buffer contents */
void print_cper(const void *buf);

#endif /* CPER_H */

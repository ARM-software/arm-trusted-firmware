/*
 * Copyright (c) 2025-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/ras.h>

#include <cper.h>
#include <rdaspen_ras.h>

static uint8_t cper_cpu_affinity_value(u_register_t mpidr)
{
	/*
	 * Match plat_arm_calc_core_pos() semantics: if MT bit is clear,
	 * treat MPIDR as if it encoded thread affinity so the returned
	 * value maps CPUs consistently regardless of threading support.
	 */
	if ((mpidr & MPIDR_MT_MASK) == 0U)
		mpidr <<= MPIDR_AFFINITY_BITS;

	if (MPIDR_AFFLVL3_VAL(mpidr) != 0U)
		return (uint8_t)MPIDR_AFFLVL3_VAL(mpidr);
	if (MPIDR_AFFLVL2_VAL(mpidr) != 0U)
		return (uint8_t)MPIDR_AFFLVL2_VAL(mpidr);
	if (MPIDR_AFFLVL1_VAL(mpidr) != 0U)
		return (uint8_t)MPIDR_AFFLVL1_VAL(mpidr);

	return (uint8_t)MPIDR_AFFLVL0_VAL(mpidr);
}

/* Convert ERXSTATUS_EL1 bits to CPER severity */
static inline uint32_t esb_severity_from_erx(uint64_t err_status)
{
	if (err_status & ERX_STATUS_CE)
		return ACPI_DATA_ENTRY_SEV_CORRECTED;
	if (err_status & ERX_STATUS_DE)
		return ACPI_DATA_ENTRY_SEV_RECOVERABLE;
	if (err_status & ERX_STATUS_UE)
		return ACPI_DATA_ENTRY_SEV_FATAL;
	return ACPI_DATA_ENTRY_SEV_NONE;
}

static inline uint8_t arm_error_type_from_unit(uint64_t err_misc0)
{
	unsigned int unit = ERXMISC0_UNIT(err_misc0);

	switch (unit) {
	case ERXMISC0_UNIT_L2_TLB:
		return ARM_ERROR_TYPE_TLB;
	default:
		return ARM_ERROR_TYPE_CACHE;
	}
}

/*
 * ARM CACHE ERROR STRUCTURE
 *
 *  - Validation Bit (bits 15:0): Set valid bits for the fields encoded
 *  - Transaction Type (bits 17:16): INSTR / DATA / GENERIC (derived from UNIT)
 *  - Operation (bits 21:18): set to GENERIC
 *  - Level (bits 24:22): cache level (1 for L1, 2 for L2), derived from UNIT
 *  - Processor Context Corrupt (bit 25): value = 0 (not corrupted)
 *  - Corrected (bit 26): value from 'corrected' parameter
 *  - Precise PC (bit 27): value = 0
 *  - Restartable PC (bit 28): value = 0
 *  - Reserved (63:29): zero
 */
static inline uint64_t arm_cache_error_structure(uint64_t err_misc0,
						 bool corrected)
{
	uint64_t info = 0;
	unsigned int unit = ERXMISC0_UNIT(err_misc0);

	uint64_t tx_value = ARM_CACHE_ERR_TX_GENERIC;
	uint64_t level_value = 0;

	switch (unit) {
	case ERXMISC0_UNIT_L1I:
		/* TxType = Instruction, Level = 1 */
		tx_value = ARM_CACHE_ERR_TX_INSTR;
		level_value = 1u;
		break;

	case ERXMISC0_UNIT_L1D:
		/* TxType = Data, Level = 1 */
		tx_value = ARM_CACHE_ERR_TX_DATA;
		level_value = 1u;
		break;

	case ERXMISC0_UNIT_L2_CACHE:
		/* TxType = Generic, Level = 2 */
		tx_value = ARM_CACHE_ERR_TX_GENERIC;
		level_value = 2u;
		break;

	default:
		break;
	}

	/* Validation bits (bit order: 0..6) */
	info |= ARM_CACHE_ERR_VLD_TX_TYPE;    /* Bit 0: Transaction Type Valid */
	info |= ARM_CACHE_ERR_VLD_OPERATION;  /* Bit 1: Operation Valid       */
	info |= ARM_CACHE_ERR_VLD_LEVEL;      /* Bit 2: Level Valid           */
	info |= ARM_CACHE_ERR_VLD_PCC;        /* Bit 3: PCC Valid             */
	info |= ARM_CACHE_ERR_VLD_CORRECTED;  /* Bit 4: Corrected Valid       */
	info |= ARM_CACHE_ERR_VLD_PRECISE_PC; /* Bit 5: Precise PC Valid      */
	info |= ARM_CACHE_ERR_VLD_RESTART_PC; /* Bit 6: Restartable PC Valid  */

	/* Transaction Type (bits 17:16) */
	info |= (tx_value << ARM_CACHE_ERR_TX_SHIFT) & ARM_CACHE_ERR_TX_MASK;

	/* Level (bits 24:22) */
	info |= (level_value << ARM_CACHE_ERR_LEVEL_SHIFT) & ARM_CACHE_ERR_LEVEL_MASK;

	/* Corrected (bit 26) */
	if (corrected)
		info |= (1ULL << ARM_CACHE_ERR_CORR_BIT);

	return info;
}

/*
 * ARM PROCESSOR ERROR CONTEXT INFORMATION
 *
 * Context[0] Type-4: AArch64 GPRs (x0..x30 + SP_EL0)
 * Context[1] Type-5: EL1 registers
 */
static bool
fill_ns_context_blocks(struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *sec)
{
	cpu_context_t *ns = cm_get_context(NON_SECURE);

	if (!ns) {
		VERBOSE("CPER: no NON_SECURE cpu_context\n");
		return false;
	}

	/* -------- Type 4: GPRs (x0..x30 + SP_EL0) -------- */
	gp_regs_t *gpr = get_gpregs_ctx(ns);

	if (!gpr)
		return false;

	sec->CpuContextInfo[0].Hdr.Version = 0;
	sec->CpuContextInfo[0].Hdr.RegisterContextType =
		EFI_ARM_CONTEXT_INFO_REG_TYPE_4;
	sec->CpuContextInfo[0].Hdr.RegisterArraySize =
		(uint32_t)sizeof(sec->CpuContextInfo[0].RegisterArray.Type4Gpr);
	struct EFI_ARM_AARCH64_CONTEXT_GPR *type4GprRegValues =
		&sec->CpuContextInfo[0].RegisterArray.Type4Gpr;

	type4GprRegValues->X[0] = read_ctx_reg(gpr, CTX_GPREG_X0);
	type4GprRegValues->X[1] = read_ctx_reg(gpr, CTX_GPREG_X1);
	type4GprRegValues->X[2] = read_ctx_reg(gpr, CTX_GPREG_X2);
	type4GprRegValues->X[3] = read_ctx_reg(gpr, CTX_GPREG_X3);
	type4GprRegValues->X[4] = read_ctx_reg(gpr, CTX_GPREG_X4);
	type4GprRegValues->X[5] = read_ctx_reg(gpr, CTX_GPREG_X5);
	type4GprRegValues->X[6] = read_ctx_reg(gpr, CTX_GPREG_X6);
	type4GprRegValues->X[7] = read_ctx_reg(gpr, CTX_GPREG_X7);
	type4GprRegValues->X[8] = read_ctx_reg(gpr, CTX_GPREG_X8);
	type4GprRegValues->X[9] = read_ctx_reg(gpr, CTX_GPREG_X9);
	type4GprRegValues->X[10] = read_ctx_reg(gpr, CTX_GPREG_X10);
	type4GprRegValues->X[11] = read_ctx_reg(gpr, CTX_GPREG_X11);
	type4GprRegValues->X[12] = read_ctx_reg(gpr, CTX_GPREG_X12);
	type4GprRegValues->X[13] = read_ctx_reg(gpr, CTX_GPREG_X13);
	type4GprRegValues->X[14] = read_ctx_reg(gpr, CTX_GPREG_X14);
	type4GprRegValues->X[15] = read_ctx_reg(gpr, CTX_GPREG_X15);
	type4GprRegValues->X[16] = read_ctx_reg(gpr, CTX_GPREG_X16);
	type4GprRegValues->X[17] = read_ctx_reg(gpr, CTX_GPREG_X17);
	type4GprRegValues->X[18] = read_ctx_reg(gpr, CTX_GPREG_X18);
	type4GprRegValues->X[19] = read_ctx_reg(gpr, CTX_GPREG_X19);
	type4GprRegValues->X[20] = read_ctx_reg(gpr, CTX_GPREG_X20);
	type4GprRegValues->X[21] = read_ctx_reg(gpr, CTX_GPREG_X21);
	type4GprRegValues->X[22] = read_ctx_reg(gpr, CTX_GPREG_X22);
	type4GprRegValues->X[23] = read_ctx_reg(gpr, CTX_GPREG_X23);
	type4GprRegValues->X[24] = read_ctx_reg(gpr, CTX_GPREG_X24);
	type4GprRegValues->X[25] = read_ctx_reg(gpr, CTX_GPREG_X25);
	type4GprRegValues->X[26] = read_ctx_reg(gpr, CTX_GPREG_X26);
	type4GprRegValues->X[27] = read_ctx_reg(gpr, CTX_GPREG_X27);
	type4GprRegValues->X[28] = read_ctx_reg(gpr, CTX_GPREG_X28);
	type4GprRegValues->X[29] = read_ctx_reg(gpr, CTX_GPREG_X29);
	type4GprRegValues->X[30] = read_ctx_reg(gpr, CTX_GPREG_LR);
	type4GprRegValues->SP = read_ctx_reg(gpr, CTX_GPREG_SP_EL0);

	/* -------- Type 5: EL1 sysregs -------- */
	el1_sysregs_t *el1 = get_el1_sysregs_ctx(ns);

	if (!el1)
		return false;

	sec->CpuContextInfo[1].Hdr.Version = 0;
	sec->CpuContextInfo[1].Hdr.RegisterContextType =
		EFI_ARM_CONTEXT_INFO_REG_TYPE_5;
	sec->CpuContextInfo[1].Hdr.RegisterArraySize = (uint32_t)sizeof(
		sec->CpuContextInfo[1].RegisterArray.Type5SysRegs);
	struct EFI_ARM_AARCH64_EL1_CONTEXT_SYSTEM_REGISTERS *type5SysRegValues =
		&sec->CpuContextInfo[1].RegisterArray.Type5SysRegs;

	type5SysRegValues->SPSR_EL1 = read_el1_ctx_common(el1, spsr_el1);
	type5SysRegValues->ELR_EL1 = read_el1_ctx_common(el1, elr_el1);
	type5SysRegValues->ESR_EL1 = read_el1_ctx_common(el1, esr_el1);
	type5SysRegValues->FAR_EL1 = read_el1_ctx_common(el1, far_el1);
	type5SysRegValues->ISR_EL1 = 0;

	type5SysRegValues->MAIR_EL1 = read_el1_ctx_common(el1, mair_el1);
	type5SysRegValues->MIDR_EL1 = read_midr_el1();
	type5SysRegValues->MPIDR_EL1 = read_mpidr_el1();
	type5SysRegValues->SCTLR_EL1 = read_ctx_sctlr_el1_reg_errata(ns);

	type5SysRegValues->SP_EL0 =
		read_ctx_reg(get_gpregs_ctx(ns), CTX_GPREG_SP_EL0);
	type5SysRegValues->SP_EL1 = read_el1_ctx_common(el1, sp_el1);
	type5SysRegValues->SPSR_EL1 = read_el1_ctx_common(el1, spsr_el1);

	type5SysRegValues->TCR_EL1 = read_ctx_tcr_el1_reg_errata(ns);
	type5SysRegValues->TPIDR_EL0 = read_el1_ctx_common(el1, tpidr_el0);
	type5SysRegValues->TPIDR_EL1 = read_el1_ctx_common(el1, tpidr_el1);
	type5SysRegValues->TPIDRRO_EL0 = read_el1_ctx_common(el1, tpidrro_el0);
	type5SysRegValues->TTBR0_EL1 = read_el1_ctx_common(el1, ttbr0_el1);
	type5SysRegValues->TTBR1_EL1 = read_el1_ctx_common(el1, ttbr1_el1);

	return true;
}

/*
 * cper_write_cpu_record - Build a single CPER ESB + ARM CPU section into a buffer
 *
 * This constructs a Common Platform Error Record (CPER) with:
 *   - 1 Error Status Block (ESB) header,
 *   - 1 Data Entry (ARM Processor Specific Section GUID),
 *   - 1 ARM CPU section payload containing:
 *       * Section header (EFI_ARM_PROCESSOR_ERROR_SECTION_HEADER),
 *       * One error info (EFI_ARM_PROCESSOR_ERROR_INFORMATION),
 *       * One context info (EFI_ARM_PROCESSOR_CONTEXT_INFORMATION), containing:
 *           Type-4 (GPRs) and Type-5 (EL1) registers.
 */

size_t cper_write_cpu_record(void *buf, size_t buf_size)
{
	if (!buf)
		return 0;

	uint64_t err_status = read_erxstatus_el1();
	uint64_t err_misc0 = read_erxmisc0_el1();
	uint64_t err_addr = read_erxaddr_el1();

	/* Compute size of CPU payload section */

	uint32_t context_header_size =
		sizeof(struct EFI_ARM_PROCESSOR_ERROR_CONTEXT_INFO_HEADER);
	uint32_t context0_size =
		(uint32_t)sizeof(struct EFI_ARM_PROCESSOR_CONTEXT_INFORMATION);
	uint32_t context1_size =
		context_header_size +
		(uint32_t)sizeof(struct EFI_ARM_AARCH64_EL1_CONTEXT_SYSTEM_REGISTERS);
	uint32_t contexts_total_size = context0_size + context1_size;

	uint32_t section_size =
		(uint32_t)sizeof(struct EFI_ARM_PROCESSOR_ERROR_SECTION_HEADER) +
		((uint32_t)sizeof(struct EFI_ARM_PROCESSOR_ERROR_INFORMATION) *
		 CPU_ERR_INFO_NUM) + contexts_total_size;

	/* Total size: ESB + DataEntry + CPU payload */
	size_t size = sizeof(struct ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER) +
		      sizeof(struct ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER) +
		      section_size;

	if (buf_size < size) {
		WARN("RAS: ESB buffer too small (need %zu, have %zu)\n", size,
		     buf_size);
		return 0;
	}

	struct ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER *esb =
		(struct ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER *)buf;
	struct ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER *de =
		(struct ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER *)(esb + 1);
	struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *sec =
		(struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *)(de + 1);

	/* Error Status Block Header */
	memset(esb, 0, sizeof(*esb));
	esb->BlockStatus = esb_block_status((err_status & ERX_STATUS_UE) != 0,
					    (err_status & ERX_STATUS_CE) != 0,
					    false, false, 1);
	esb->RawDataOffset = (uint32_t)(sizeof(*esb) + sizeof(*de));
	esb->RawDataLength = 0;
	esb->DataLength = (uint32_t)(sizeof(*de) + section_size);
	esb->ErrorSeverity = esb_severity_from_erx(err_status);

	/* Error Data Entry */
	memset(de, 0, sizeof(*de));
	de->SectionType = ARM_PROC_ERR_SECTION_GUID;
	de->ErrorSeverity = esb->ErrorSeverity;
	de->Revision = ACPI_DATA_ENTRY_REV_0300;
	de->ValidationBits = 0;
	de->Flags = (err_status & ERX_STATUS_DE) ? ACPI_SEC_FLAG_LATENT : 0;
	de->ErrorDataLength = section_size;

	/* Payload: CPU section */
	memset(sec, 0, section_size);

	/*  ARM Processor Error Section */
	u_register_t mpidr = read_mpidr_el1();

	sec->CpuInfo.ValidationBit =
		(EFI_ARM_PROC_ERROR_MPIDR_VALID |
		 EFI_ARM_PROC_ERROR_AFFINITY_LEVEL_VALID |
		 EFI_ARM_PROC_ERROR_RUNNING_STATE_VALID);
	sec->CpuInfo.ErrInfoNum = CPU_ERR_INFO_NUM;
	sec->CpuInfo.ContextInfoNum = CPU_CONTEXT_INFO_NUM;
	sec->CpuInfo.SectionLength = section_size;
	/*
	 * Record the affinity value from the highest populated MPIDR level.
	 * Together with MPIDR_EL1, this describes the reporting PE within
	 * the processor hierarchy for consumers of the error record.
	 */
	sec->CpuInfo.AffinityLevel = cper_cpu_affinity_value(mpidr);
	sec->CpuInfo.MPIDR_EL1 = mpidr;
	sec->CpuInfo.MIDR_EL1 = read_midr_el1();
	sec->CpuInfo.RunState = 1;
	sec->CpuInfo.PsciState = 0;

	/* ARM Processor Error Information */
	struct EFI_ARM_PROCESSOR_ERROR_INFORMATION *ei = &sec->CpuErrInfo[0];
	bool ce = (err_status & ERX_STATUS_CE) != 0;
	bool phys_addr_valid = ERX_STATUS_ADDRV(err_status);

	ei->Version = 0;
	ei->Length = (uint8_t)sizeof(*ei);
	ei->ValidationBit =
		(EFI_ARM_PROC_ERROR_INFO_MULTIPLE_ERROR_VALID |
		 EFI_ARM_PROC_ERROR_INFO_FLAGS_VALID |
		 (phys_addr_valid ? EFI_ARM_PROC_ERROR_INFO_PHYS_ADDR_VALID :
				    0));
	ei->Type = arm_error_type_from_unit(err_misc0);
	ei->MultipleError = 0;
	ei->Flags = 0;
	ei->ErrorInfo = 0;
	ei->VirtualFaultAddress = 0;
	ei->PhysicalFaultAddress = err_addr;
	if (ei->Type == ARM_ERROR_TYPE_CACHE) {
		ei->ValidationBit |= EFI_ARM_PROC_ERROR_INFO_ERROR_INFO_VALID;
		ei->ErrorInfo = arm_cache_error_structure(err_misc0, ce);
	}

	/*  ARM Processor Error Context Information (Type 4 and 5 contexts) */
	cm_el1_sysregs_context_save(NON_SECURE);
	if (!fill_ns_context_blocks(sec))
		WARN("CPER: Non-secure EL1 context unavailable; context blocks left zeroed\n");

	return size;
}

static void print_guid(struct efi_guid g)
{
	VERBOSE("  SectionType   = {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
		g.time_low, g.time_mid, g.time_hi_and_version,
		g.clock_seq_and_node[0], g.clock_seq_and_node[1],
		g.clock_seq_and_node[2], g.clock_seq_and_node[3],
		g.clock_seq_and_node[4], g.clock_seq_and_node[5],
		g.clock_seq_and_node[6], g.clock_seq_and_node[7]);
}

static void print_cache_error_info(uint64_t info)
{
	uint64_t validation = (info & 0x7FULL);
	unsigned int tx = (unsigned int)((info & ARM_CACHE_ERR_TX_MASK) >>
				 ARM_CACHE_ERR_TX_SHIFT);
	unsigned int op = (unsigned int)((info & ARM_CACHE_ERR_OP_MASK) >>
				 ARM_CACHE_ERR_OP_SHIFT);
	unsigned int lvl = (unsigned int)((info & ARM_CACHE_ERR_LEVEL_MASK) >>
				  ARM_CACHE_ERR_LEVEL_SHIFT);
	unsigned int pcc = (unsigned int)((info >> ARM_CACHE_ERR_PCC_BIT) & 1U);
	unsigned int corr = (unsigned int)((info >> ARM_CACHE_ERR_CORR_BIT) & 1U);
	unsigned int ppc = (unsigned int)((info >> ARM_CACHE_ERR_PRECISE_PC_BIT) & 1U);
	unsigned int rpc = (unsigned int)((info >> ARM_CACHE_ERR_RESTART_PC_BIT) & 1U);
	uint64_t reserved = (info >> 29);

	VERBOSE("  CacheErrorInfo:\n");
	VERBOSE("    ValidationBit           = 0x%04llx\n",
		(unsigned long long)validation);
	VERBOSE("    TransactionType         = %u\n", tx);
	VERBOSE("    Operation               = %u\n", op);
	VERBOSE("    Level                   = %u\n", lvl);
	VERBOSE("    ProcessorContextCorrupt = %u\n", pcc);
	VERBOSE("    Corrected               = %u\n", corr);
	VERBOSE("    PrecisePC               = %u\n", ppc);
	VERBOSE("    RestartablePC           = %u\n", rpc);
	VERBOSE("    Reserved[63:29]         = 0x%08llx\n",
		(unsigned long long)reserved);
}

static void
print_esb_header(const struct ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER *esb)
{
	if (esb == NULL) {
		WARN("CPER: null ESB header\n");
		return;
	}
	VERBOSE("ESB.Header:\n");
	VERBOSE("  BlockStatus   = 0x%08x\n", esb->BlockStatus);
	VERBOSE("  RawDataOffset = 0x%08x\n", esb->RawDataOffset);
	VERBOSE("  RawDataLength = %u\n", esb->RawDataLength);
	VERBOSE("  DataLength    = %u\n", esb->DataLength);
	VERBOSE("  ErrorSeverity = %u\n", esb->ErrorSeverity);
}

static void
print_data_entry_header(const struct ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER *de)
{
	if (de == NULL) {
		WARN("CPER: null Data Entry header\n");
		return;
	}
	VERBOSE("DataEntry:\n");
	print_guid(de->SectionType);
	VERBOSE("  ErrorSeverity = %u\n", de->ErrorSeverity);
	VERBOSE("  Revision      = 0x%04x\n", de->Revision);
	VERBOSE("  ValidationBits= 0x%02x\n", de->ValidationBits);
	VERBOSE("  Flags         = 0x%02x\n", de->Flags);
	VERBOSE("  ErrorDataLen  = %u\n", de->ErrorDataLength);
}

static void
print_cpu_section_header(const struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *sec)
{
	if (sec == NULL) {
		WARN("CPER: null CPU section\n");
		return;
	}
	VERBOSE("CPU Section:\n");
	VERBOSE("  ValidationBit = 0x%08x\n", sec->CpuInfo.ValidationBit);
	VERBOSE("  ErrInfoNum    = %u\n", sec->CpuInfo.ErrInfoNum);
	VERBOSE("  CtxtInfoNum   = %u\n", sec->CpuInfo.ContextInfoNum);
	VERBOSE("  SectionLength = %u\n", sec->CpuInfo.SectionLength);
	VERBOSE("  MPIDR_EL1     = 0x%016llx\n",
		(unsigned long long)sec->CpuInfo.MPIDR_EL1);
	VERBOSE("  MIDR_EL1      = 0x%016llx\n",
		(unsigned long long)sec->CpuInfo.MIDR_EL1);
	VERBOSE("  RunState      = %u\n", sec->CpuInfo.RunState);
	VERBOSE("  PsciState     = %u\n", sec->CpuInfo.PsciState);
}

static void
print_error_info(const struct EFI_ARM_PROCESSOR_ERROR_INFORMATION *ei)
{
	if (ei == NULL) {
		WARN("CPER: null ErrorInfo\n");
		return;
	}
	VERBOSE("ErrorInfo[0]:\n");
	VERBOSE("  Version       = %u\n", ei->Version);
	VERBOSE("  Length        = %u\n", ei->Length);
	VERBOSE("  ValidationBit = 0x%04x\n", ei->ValidationBit);
	VERBOSE("  Type          = %u\n", ei->Type);
	VERBOSE("  MultipleError = %u\n", ei->MultipleError);
	VERBOSE("  Flags         = 0x%02x\n", ei->Flags);
	VERBOSE("  ErrorInfo     = 0x%016llx\n",
		(unsigned long long)ei->ErrorInfo);

	if (ei->Type == ARM_ERROR_TYPE_CACHE &&
	    (ei->ValidationBit & EFI_ARM_PROC_ERROR_INFO_ERROR_INFO_VALID) &&
	    ei->ErrorInfo != 0) {
		print_cache_error_info(ei->ErrorInfo);
	}

	VERBOSE("  VirtFaultAddr = 0x%016llx\n",
		(unsigned long long)ei->VirtualFaultAddress);
	VERBOSE("  PhysFaultAddr = 0x%016llx\n",
		(unsigned long long)ei->PhysicalFaultAddress);
}

static void
print_context_block(const struct EFI_ARM_PROCESSOR_CONTEXT_INFORMATION *c,
		    unsigned int idx)
{
	if (c == NULL) {
		WARN("CPER: null Context block\n");
		return;
	}
	VERBOSE("Context[%u]: type=%u size=%u\n", idx,
		c->Hdr.RegisterContextType, c->Hdr.RegisterArraySize);

	switch (c->Hdr.RegisterContextType) {
	case EFI_ARM_CONTEXT_INFO_REG_TYPE_4:
		VERBOSE("  GPR x0..x3: %016llx %016llx %016llx %016llx\n",
			(unsigned long long)c->RegisterArray.Type4Gpr.X[0],
			(unsigned long long)c->RegisterArray.Type4Gpr.X[1],
			(unsigned long long)c->RegisterArray.Type4Gpr.X[2],
			(unsigned long long)c->RegisterArray.Type4Gpr.X[3]);
		VERBOSE("  SP = 0x%016llx\n",
			(unsigned long long)c->RegisterArray.Type4Gpr.SP);
		break;

	case EFI_ARM_CONTEXT_INFO_REG_TYPE_5:
		VERBOSE("  EL1: SPSR=0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.SPSR_EL1);
		VERBOSE("  EL1: ELR=0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.ELR_EL1);
		VERBOSE("  EL1: ESR=0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.ESR_EL1);
		VERBOSE("  EL1: FAR=0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.FAR_EL1);
		VERBOSE("  ISR_EL1    = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.ISR_EL1);
		VERBOSE("  MAIR_EL1   = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.MAIR_EL1);
		VERBOSE("  MIDR_EL1   = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.MIDR_EL1);
		VERBOSE("  MPIDR_EL1  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.MPIDR_EL1);
		VERBOSE("  SCTLR_EL1  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.SCTLR_EL1);
		VERBOSE("  SP_EL0     = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.SP_EL0);
		VERBOSE("  SP_EL1     = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.SP_EL1);
		VERBOSE("  TCR_EL1    = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TCR_EL1);
		VERBOSE("  TPIDR_EL0  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TPIDR_EL0);
		VERBOSE("  TPIDR_EL1  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TPIDR_EL1);
		VERBOSE("  TPIDRRO_EL0= 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TPIDRRO_EL0);
		VERBOSE("  TTBR0_EL1  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TTBR0_EL1);
		VERBOSE("  TTBR1_EL1  = 0x%016llx\n",
			(unsigned long long)
				c->RegisterArray.Type5SysRegs.TTBR1_EL1);
		break;

	default:
		VERBOSE("  Unknown context type %u\n",
			c->Hdr.RegisterContextType);
		break;
	}
}

static void
print_contexts(const struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *sec)
{
	for (unsigned int i = 0; i < sec->CpuInfo.ContextInfoNum; i++)
		print_context_block(&sec->CpuContextInfo[i], i);
}

/* Dump the CPER buffer contents */
void print_cper(const void *buf)
{
	if (!buf) {
		WARN("%s : null buffer\n", __func__);
		return;
	}

	const struct ACPI_GENERIC_ERROR_STATUS_BLOCK_HEADER *esb = buf;
	const struct ACPI_GENERIC_ERROR_DATA_ENTRY_HEADER *de =
		(const void *)(esb + 1);
	const struct EFI_ARM_PROCESSOR_ERROR_RECORD_DATA *sec =
		(const void *)(de + 1);
	const struct EFI_ARM_PROCESSOR_ERROR_INFORMATION *ei =
		&sec->CpuErrInfo[0];

	VERBOSE("\n========== CPER DUMP ==========\n");

	print_esb_header(esb);
	print_data_entry_header(de);
	print_cpu_section_header(sec);
	print_error_info(ei);
	print_contexts(sec);

	VERBOSE("========== END OF CPER DUMP ==========\n\n");
}

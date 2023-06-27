/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <bl31/interrupt_mgmt.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/ras.h>
#include <plat/common/platform.h>
#include <services/sdei.h>
#include <services/spm_mm_svc.h>

#include <sgi_ras.h>

#define CPU_CONTEXT_REG_GPR_ARR_SIZE 32
#define CPU_CONTEXT_REG_EL1_ARR_SIZE 17
#define CPU_CONTEXT_REG_EL2_ARR_SIZE 16
#define CPU_CONTEXT_REG_EL3_ARR_SIZE 10

/*
 * MM Communicate message header GUID to indicate the payload is intended for
 * CPU MM driver.
 */
struct efi_guid cpu_ecc_event_guid = {
	0x2c1b3bfc, 0x42cd, 0x4a66,
	{0xac, 0xd1, 0xa4, 0xd1, 0x63, 0xe9, 0x90, 0xf6}
	};

/*
 * CPU error information data structure communicated as part of MM
 * Communication data payload.
 */
typedef struct {
	uint64_t ErrStatus;
	uint64_t ErrMisc0;
	uint64_t ErrAddr;
	uint64_t SecurityState;
	uint64_t ErrCtxGpr[CPU_CONTEXT_REG_GPR_ARR_SIZE];
	uint64_t ErrCtxEl1Reg[CPU_CONTEXT_REG_EL1_ARR_SIZE];
	uint64_t ErrCtxEl2Reg[CPU_CONTEXT_REG_EL2_ARR_SIZE];
	uint64_t ErrCtxEl3Reg[CPU_CONTEXT_REG_EL3_ARR_SIZE];
} cpu_err_info;

/*
 * Reads the CPU context and error information from the relevant registers and
 * populates the CPU error information data structure.
 */
static void populate_cpu_err_data(cpu_err_info *cpu_info,
					uint64_t security_state)
{
	void *ctx;

	ctx = cm_get_context(security_state);

	cpu_info->ErrStatus = read_erxstatus_el1();
	cpu_info->ErrMisc0 = read_erxmisc0_el1();
	cpu_info->ErrAddr = read_erxaddr_el1();
	cpu_info->SecurityState = security_state;

	/* populate CPU EL1 context information. */
	cpu_info->ErrCtxEl1Reg[0]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_ELR_EL1);
	cpu_info->ErrCtxEl1Reg[1]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_ESR_EL1);
	cpu_info->ErrCtxEl1Reg[2]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_FAR_EL1);
	cpu_info->ErrCtxEl1Reg[3]  = read_isr_el1();
	cpu_info->ErrCtxEl1Reg[4]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_MAIR_EL1);
	cpu_info->ErrCtxEl1Reg[5]  = read_midr_el1();
	cpu_info->ErrCtxEl1Reg[6]  = read_mpidr_el1();
	cpu_info->ErrCtxEl1Reg[7]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_SCTLR_EL1);
	cpu_info->ErrCtxEl1Reg[8]  = read_ctx_reg(get_gpregs_ctx(ctx),
						  CTX_GPREG_SP_EL0);
	cpu_info->ErrCtxEl1Reg[9]  = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_SP_EL1);
	cpu_info->ErrCtxEl1Reg[10] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_SPSR_EL1);
	cpu_info->ErrCtxEl1Reg[11] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TCR_EL1);
	cpu_info->ErrCtxEl1Reg[12] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TPIDR_EL0);
	cpu_info->ErrCtxEl1Reg[13] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TPIDR_EL1);
	cpu_info->ErrCtxEl1Reg[14] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TPIDRRO_EL0);
	cpu_info->ErrCtxEl1Reg[15] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TTBR0_EL1);
	cpu_info->ErrCtxEl1Reg[16] = read_ctx_reg(get_el1_sysregs_ctx(ctx),
						  CTX_TTBR1_EL1);

#if CTX_INCLUDE_EL2_REGS
	cpu_info->ErrCtxEl2Reg[0]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_ELR_EL2);
	cpu_info->ErrCtxEl2Reg[1]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_ESR_EL2);
	cpu_info->ErrCtxEl2Reg[2]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_FAR_EL2);
	cpu_info->ErrCtxEl2Reg[3]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_HACR_EL2);
	cpu_info->ErrCtxEl2Reg[4]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_HCR_EL2);
	cpu_info->ErrCtxEl2Reg[5]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_HPFAR_EL2);
	cpu_info->ErrCtxEl2Reg[6]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_MAIR_EL2);
	cpu_info->ErrCtxEl2Reg[7]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_SCTLR_EL2);
	cpu_info->ErrCtxEl2Reg[8]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_SP_EL2);
	cpu_info->ErrCtxEl2Reg[9]   = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_SPSR_EL2);
	cpu_info->ErrCtxEl2Reg[10]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_TCR_EL2);
	cpu_info->ErrCtxEl2Reg[11]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_TPIDR_EL2);
	cpu_info->ErrCtxEl2Reg[12]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_TTBR0_EL2);
	cpu_info->ErrCtxEl2Reg[13]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_VTCR_EL2);
	cpu_info->ErrCtxEl2Reg[14]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_VTTBR_EL2);
	cpu_info->ErrCtxEl2Reg[15]  = read_ctx_reg(get_el2_sysregs_ctx(ctx),
						   CTX_ESR_EL2);
#endif

	cpu_info->ErrCtxEl3Reg[0]   = read_ctx_reg(get_el3state_ctx(ctx),
						   CTX_ELR_EL3);
	cpu_info->ErrCtxEl3Reg[1]   = read_ctx_reg(get_el3state_ctx(ctx),
						   CTX_ESR_EL3);
	cpu_info->ErrCtxEl3Reg[2]   = read_far_el3();
	cpu_info->ErrCtxEl3Reg[4]   = read_mair_el3();
	cpu_info->ErrCtxEl3Reg[5]   = read_sctlr_el3();
	cpu_info->ErrCtxEl3Reg[6]   = 0; /* sp_el3 */
	cpu_info->ErrCtxEl3Reg[7]   = read_tcr_el3();
	cpu_info->ErrCtxEl3Reg[8]   = read_tpidr_el3();
	cpu_info->ErrCtxEl3Reg[9]   = read_ttbr0_el3();
}

/* CPU RAS interrupt handler */
int sgi_ras_cpu_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data)
{
	struct sgi_ras_ev_map *ras_map;
	mm_communicate_header_t *header;
	cpu_err_info cpu_info = {0};
	uint64_t clear_status;
	uint32_t intr;
	int ret;

	cm_el1_sysregs_context_save(NON_SECURE);
	intr = data->interrupt;

	INFO("[CPU RAS] CPU intr received = %d on cpu_id = %d\n",
		intr, plat_my_core_pos());

	INFO("[CPU RAS] ERXMISC0_EL1 = 0x%lx\n", read_erxmisc0_el1());
	INFO("[CPU RAS] ERXSTATUS_EL1 = 0x%lx\n", read_erxstatus_el1());
	INFO("[CPU RAS] ERXADDR_EL1 = 0x%lx\n", read_erxaddr_el1());

	/* Populate CPU Error Source Information. */
	populate_cpu_err_data(&cpu_info, get_interrupt_src_ss(data->flags));

	/* Clear the interrupt. */
	clear_status = read_erxstatus_el1();
	write_erxstatus_el1(clear_status);
	plat_ic_end_of_interrupt(intr);

	header = (void *) PLAT_SPM_BUF_BASE;
	memset(header, 0, sizeof(*header));
	memcpy(&header->data, &cpu_info, sizeof(cpu_info));
	header->message_len = sizeof(cpu_info);
	memcpy(&header->header_guid, (void *) &cpu_ecc_event_guid,
		sizeof(struct efi_guid));

	spm_mm_sp_call(MM_COMMUNICATE_AARCH64, (uint64_t)header, 0,
		       plat_my_core_pos());

	/*
	 * Find if this is a RAS interrupt. There must be an event against
	 * this interrupt
	 */
	ras_map = sgi_find_ras_event_map_by_intr(intr);
	if (ras_map == NULL) {
		ERROR("SGI: RAS error info for interrupt id: %d not found\n",
			intr);
		return -1;
	}

	/* Dispatch the event to the SDEI client */
	ret = sdei_dispatch_event(ras_map->sdei_ev_num);
	if (ret != 0) {
		/*
		 * sdei_dispatch_event() may return failing result in some
		 * cases, for example kernel may not have registered a handler
		 * or RAS event may happen early during boot. We restore the NS
		 * context when sdei_dispatch_event() returns failing result.
		 */
		ERROR("SDEI dispatch failed: %d", ret);
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);
	}

	return ret;
}

/*
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMD_PRIVATE_H
#define SPMD_PRIVATE_H

#include <common/bl_common.h>
#include <context.h>

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define SPMD_C_RT_CTX_X19		0x0
#define SPMD_C_RT_CTX_X20		0x8
#define SPMD_C_RT_CTX_X21		0x10
#define SPMD_C_RT_CTX_X22		0x18
#define SPMD_C_RT_CTX_X23		0x20
#define SPMD_C_RT_CTX_X24		0x28
#define SPMD_C_RT_CTX_X25		0x30
#define SPMD_C_RT_CTX_X26		0x38
#define SPMD_C_RT_CTX_X27		0x40
#define SPMD_C_RT_CTX_X28		0x48
#define SPMD_C_RT_CTX_X29		0x50
#define SPMD_C_RT_CTX_X30		0x58

#define SPMD_C_RT_CTX_SIZE		0x60
#define SPMD_C_RT_CTX_ENTRIES		(SPMD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <lib/psci/psci_lib.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>

typedef enum spmc_state {
	SPMC_STATE_RESET = 0,
	SPMC_STATE_OFF,
	SPMC_STATE_ON_PENDING,
	SPMC_STATE_ON
} spmc_state_t;

/*
 * Data structure used by the SPM dispatcher (SPMD) in EL3 to track context of
 * the SPM core (SPMC) at the next lower EL.
 */
typedef struct spmd_spm_core_context {
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	spmc_state_t state;
	bool secure_interrupt_ongoing;
#if ENABLE_SPMD_LP
	uint8_t spmd_lp_sync_req_ongoing;
#endif
} spmd_spm_core_context_t;

/* Flags to indicate ongoing requests for SPMD EL3 logical partitions */
#define SPMD_LP_FFA_DIR_REQ_ONGOING		U(0x1)
#define SPMD_LP_FFA_INFO_GET_REG_ONGOING	U(0x2)

/*
 * Reserve ID for NS physical FFA Endpoint.
 */
#define FFA_NS_ENDPOINT_ID			U(0)

/* Define SPMD target function IDs for framework messages to the SPMC */
#define SPMD_FWK_MSG_FFA_VERSION_REQ		U(0x8)
#define SPMD_FWK_MSG_FFA_VERSION_RESP		U(0x9)

/* Function to build SPMD to SPMC message */
void spmd_build_spmc_message(gp_regs_t *gpregs, uint8_t target,
			     unsigned long long message);

/* Functions used to enter/exit SPMC synchronously */
uint64_t spmd_spm_core_sync_entry(spmd_spm_core_context_t *ctx);
__dead2 void spmd_spm_core_sync_exit(uint64_t rc);

bool is_spmd_logical_sp_dir_req_in_progress(const spmd_spm_core_context_t *ctx);

bool is_spmd_logical_sp_info_regs_req_in_progress(
		const spmd_spm_core_context_t *ctx);

/* Assembly helpers */
uint64_t spmd_spm_core_enter(uint64_t *c_rt_ctx);
void __dead2 spmd_spm_core_exit(uint64_t c_rt_ctx, uint64_t ret);

/* SPMD SPD power management handlers */
extern const spd_pm_ops_t spmd_pm;

/* SPMC entry point information helper */
entry_point_info_t *spmd_spmc_ep_info_get(void);

/* SPMC ID getter */
uint16_t spmd_spmc_id_get(void);

/* SPMC context on CPU based on mpidr */
spmd_spm_core_context_t *spmd_get_context_by_mpidr(uint64_t mpidr);

/* SPMC context on current CPU get helper */
spmd_spm_core_context_t *spmd_get_context(void);

int spmd_pm_secondary_ep_register(uintptr_t entry_point);
bool spmd_check_address_in_binary_image(uint64_t address);

/*
 * Platform hook in EL3 firmware to handle for Group0 secure interrupt.
 * Return values:
 *  0 = success
 *  otherwise it returns a negative value
 */
int plat_spmd_handle_group0_interrupt(uint32_t id);

uint64_t spmd_ffa_error_return(void *handle, int error_code);

#endif /* __ASSEMBLER__ */

#endif /* SPMD_PRIVATE_H */

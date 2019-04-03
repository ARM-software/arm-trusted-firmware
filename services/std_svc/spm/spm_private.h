/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_PRIVATE_H
#define SPM_PRIVATE_H

#include <context.h>

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define SP_C_RT_CTX_X19		0x0
#define SP_C_RT_CTX_X20		0x8
#define SP_C_RT_CTX_X21		0x10
#define SP_C_RT_CTX_X22		0x18
#define SP_C_RT_CTX_X23		0x20
#define SP_C_RT_CTX_X24		0x28
#define SP_C_RT_CTX_X25		0x30
#define SP_C_RT_CTX_X26		0x38
#define SP_C_RT_CTX_X27		0x40
#define SP_C_RT_CTX_X28		0x48
#define SP_C_RT_CTX_X29		0x50
#define SP_C_RT_CTX_X30		0x58

#define SP_C_RT_CTX_SIZE	0x60
#define SP_C_RT_CTX_ENTRIES	(SP_C_RT_CTX_SIZE >> DWORD_SHIFT)

/* Value returned by spm_sp_synchronous_entry() when a partition is preempted */
#define SPM_SECURE_PARTITION_PREEMPTED	U(0x1234)

#ifndef __ASSEMBLY__

#include <stdint.h>

#include <lib/xlat_tables/xlat_tables_v2.h>
#include <lib/spinlock.h>
#include <services/sp_res_desc.h>

typedef enum sp_state {
	SP_STATE_RESET = 0,
	SP_STATE_IDLE,
	SP_STATE_BUSY
} sp_state_t;

typedef struct sp_context {
	/* 1 if the partition is present, 0 otherwise */
	int is_present;

	/* Location of the image in physical memory */
	unsigned long long image_base;
	size_t image_size;

	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	struct sp_res_desc rd;

	/* Translation tables context */
	xlat_ctx_t *xlat_ctx_handle;
	spinlock_t xlat_ctx_lock;

	sp_state_t state;
	spinlock_t state_lock;

	unsigned int request_count;
	spinlock_t request_count_lock;

	/* Base and size of the shared SPM<->SP buffer */
	uintptr_t spm_sp_buffer_base;
	size_t spm_sp_buffer_size;
	spinlock_t spm_sp_buffer_lock;
} sp_context_t;

/* Functions used to enter/exit a Secure Partition synchronously */
uint64_t spm_sp_synchronous_entry(sp_context_t *sp_ctx, int can_preempt);
__dead2 void spm_sp_synchronous_exit(uint64_t rc);

/* Assembly helpers */
uint64_t spm_secure_partition_enter(uint64_t *c_rt_ctx);
void __dead2 spm_secure_partition_exit(uint64_t c_rt_ctx, uint64_t ret);

/* Secure Partition setup */
void spm_sp_setup(sp_context_t *sp_ctx);

/* Secure Partition state management helpers */
void sp_state_set(sp_context_t *sp_ptr, sp_state_t state);
void sp_state_wait_switch(sp_context_t *sp_ptr, sp_state_t from, sp_state_t to);
int sp_state_try_switch(sp_context_t *sp_ptr, sp_state_t from, sp_state_t to);

/* Functions to keep track of the number of active requests per SP */
void spm_sp_request_increase(sp_context_t *sp_ctx);
void spm_sp_request_decrease(sp_context_t *sp_ctx);
int spm_sp_request_increase_if_zero(sp_context_t *sp_ctx);

/* Functions related to the shim layer translation tables */
void spm_exceptions_xlat_init_context(void);
uint64_t *spm_exceptions_xlat_get_base_table(void);

/* Functions related to the translation tables management */
void spm_sp_xlat_context_alloc(sp_context_t *sp_ctx);
void sp_map_memory_regions(sp_context_t *sp_ctx);

/* Functions to handle Secure Partition contexts */
void spm_cpu_set_sp_ctx(unsigned int linear_id, sp_context_t *sp_ctx);
sp_context_t *spm_cpu_get_sp_ctx(unsigned int linear_id);
sp_context_t *spm_sp_get_by_uuid(const uint32_t (*svc_uuid)[4]);

/* Functions to manipulate response and requests buffers */
int spm_response_add(uint16_t client_id, uint16_t handle, uint32_t token,
		     u_register_t x1, u_register_t x2, u_register_t x3);
int spm_response_get(uint16_t client_id, uint16_t handle, uint32_t token,
		     u_register_t *x1, u_register_t *x2, u_register_t *x3);

#endif /* __ASSEMBLY__ */

#endif /* SPM_PRIVATE_H */

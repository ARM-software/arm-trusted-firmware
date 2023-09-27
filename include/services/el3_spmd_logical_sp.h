/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EL3_SPMD_LOGICAL_SP_H
#define EL3_SPMD_LOGICAL_SP_H

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <services/ffa_svc.h>

/*******************************************************************************
 * Structure definition, typedefs & constants for the SPMD Logical Partitions.
 ******************************************************************************/
typedef struct spmd_spm_core_context spmd_spm_core_context_t;

/* Prototype for SPMD logical partition initializing function. */
typedef int32_t (*ffa_spmd_lp_init_t)(void);

/* SPMD Logical Partition Descriptor. */
struct spmd_lp_desc {
	ffa_spmd_lp_init_t init;
	uint16_t sp_id;
	uint32_t properties;
	uint32_t uuid[4];  /* Little Endian. */
	const char *debug_name;
};

struct ffa_value {
	uint64_t func;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
	uint64_t arg4;
	uint64_t arg5;
	uint64_t arg6;
	uint64_t arg7;
	uint64_t arg8;
	uint64_t arg9;
	uint64_t arg10;
	uint64_t arg11;
	uint64_t arg12;
	uint64_t arg13;
	uint64_t arg14;
	uint64_t arg15;
	uint64_t arg16;
	uint64_t arg17;
};

/* Convenience macro to declare a SPMD logical partition descriptor. */
#define DECLARE_SPMD_LOGICAL_PARTITION(_name, _init, _sp_id, _uuid, _properties) \
	static const struct spmd_lp_desc __partition_desc_ ## _name	    \
		__section(".spmd_lp_descs") __used = {			    \
			.debug_name = #_name,				    \
			.init = (_init),				    \
			.sp_id = (_sp_id),				    \
			.uuid = _uuid,					    \
			.properties = (_properties),			    \
		}

IMPORT_SYM(uintptr_t, __SPMD_LP_DESCS_START__,	SPMD_LP_DESCS_START);
IMPORT_SYM(uintptr_t, __SPMD_LP_DESCS_END__,	SPMD_LP_DESCS_END);

#define SPMD_LP_DESCS_COUNT ((SPMD_LP_DESCS_END - SPMD_LP_DESCS_START) \
			  / sizeof(struct spmd_lp_desc))
CASSERT(sizeof(struct spmd_lp_desc) == 40, assert_spmd_lp_desc_size_mismatch);

/*
 * Reserve 63 IDs for SPMD Logical Partitions. Currently, 0xFFC0 to 0xFFFE
 * is reserved.
 */
#define SPMD_LP_ID_END		(SPMD_DIRECT_MSG_ENDPOINT_ID - 1)
#define SPMD_LP_ID_START	(SPMD_LP_ID_END - 62)

/*
 * TODO: Arbitrary number. Can make this platform specific in the future,
 * no known use cases for more LPs at this point.
 */
#define EL3_SPMD_MAX_NUM_LP	U(5)

static inline bool is_spmd_lp_id(unsigned int id)
{
#if ENABLE_SPMD_LP
	return (id >= SPMD_LP_ID_START && id <= SPMD_LP_ID_END);
#else
	return false;
#endif
}

static inline bool is_ffa_error(struct ffa_value *retval)
{
	return retval->func == FFA_ERROR;
}

static inline bool is_ffa_success(struct ffa_value *retval)
{
	return (retval->func == FFA_SUCCESS_SMC32) ||
		(retval->func == FFA_SUCCESS_SMC64);
}

static inline bool is_ffa_direct_msg_resp(struct ffa_value *retval)
{
	return (retval->func == FFA_MSG_SEND_DIRECT_RESP_SMC32) ||
		(retval->func == FFA_MSG_SEND_DIRECT_RESP_SMC64);
}

static inline uint16_t ffa_partition_info_regs_get_last_idx(
	struct ffa_value *args)
{
	return (uint16_t)(args->arg2 & 0xFFFFU);
}

static inline uint16_t ffa_partition_info_regs_get_curr_idx(
	struct ffa_value *args)
{
	return (uint16_t)((args->arg2 >> 16) & 0xFFFFU);
}

static inline uint16_t ffa_partition_info_regs_get_tag(struct ffa_value *args)
{
	return (uint16_t)((args->arg2 >> 32) & 0xFFFFU);
}

static inline uint16_t ffa_partition_info_regs_get_desc_size(
	struct ffa_value *args)
{
	return (uint16_t)(args->arg2 >> 48);
}

uint64_t spmd_el3_populate_logical_partition_info(void *handle, uint64_t x1,
						  uint64_t x2, uint64_t x3);

bool ffa_partition_info_regs_get_part_info(
	struct ffa_value *args, uint8_t idx,
	struct ffa_partition_info_v1_1 *partition_info);

bool spmd_el3_invoke_partition_info_get(
				const uint32_t target_uuid[4],
				const uint16_t start_index,
				const uint16_t tag,
				struct ffa_value *retval);
void spmd_logical_sp_set_spmc_initialized(void);
void spmc_logical_sp_set_spmc_failure(void);

int32_t spmd_logical_sp_init(void);
bool is_spmd_logical_sp_dir_req_in_progress(
		spmd_spm_core_context_t *ctx);

bool is_spmd_logical_sp_info_regs_req_in_progress(
		spmd_spm_core_context_t *ctx);

bool spmd_el3_ffa_msg_direct_req(uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *handle,
				 struct ffa_value *retval);

uintptr_t plat_spmd_logical_sp_smc_handler(unsigned int smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags);

#endif /* EL3_SPMD_LOGICAL_SP_H */

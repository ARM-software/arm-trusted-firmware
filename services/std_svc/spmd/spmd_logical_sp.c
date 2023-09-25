/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include "spmd_private.h"

#include <common/debug.h>
#include <common/uuid.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/spmc_svc.h>
#include <smccc_helpers.h>


/*
 * Maximum ffa_partition_info entries that can be returned by an invocation
 * of FFA_PARTITION_INFO_GET_REGS_64 is size in bytes, of available
 * registers/args in struct ffa_value divided by size of struct
 * ffa_partition_info. For this ABI, arg3-arg17 in ffa_value can be used, i.e.
 * 15 uint64_t fields. For FF-A v1.1, this value should be 5.
 */
#define MAX_INFO_REGS_ENTRIES_PER_CALL \
	(uint8_t)((15 * sizeof(uint64_t)) / \
		  sizeof(struct ffa_partition_info_v1_1))
CASSERT(MAX_INFO_REGS_ENTRIES_PER_CALL == 5, assert_too_many_info_reg_entries);

#if ENABLE_SPMD_LP
static bool is_spmd_lp_inited;
static bool is_spmc_inited;

/*
 * Helper function to obtain the array storing the EL3
 * SPMD Logical Partition descriptors.
 */
static struct spmd_lp_desc *get_spmd_el3_lp_array(void)
{
	return (struct spmd_lp_desc *) SPMD_LP_DESCS_START;
}

/*******************************************************************************
 * Validate any logical partition descriptors before we initialize.
 * Initialization of said partitions will be taken care of during SPMD boot.
 ******************************************************************************/
static int el3_spmd_sp_desc_validate(struct spmd_lp_desc *lp_array)
{
	/* Check the array bounds are valid. */
	assert(SPMD_LP_DESCS_END > SPMD_LP_DESCS_START);

	/*
	 * No support for SPMD logical partitions when SPMC is at EL3.
	 */
	assert(!is_spmc_at_el3());

	/* If no SPMD logical partitions are implemented then simply bail out. */
	if (SPMD_LP_DESCS_COUNT == 0U) {
		return -1;
	}

	for (uint32_t index = 0U; index < SPMD_LP_DESCS_COUNT; index++) {
		struct spmd_lp_desc *lp_desc = &lp_array[index];

		/* Validate our logical partition descriptors. */
		if (lp_desc == NULL) {
			ERROR("Invalid SPMD Logical SP Descriptor\n");
			return -EINVAL;
		}

		/*
		 * Ensure the ID follows the convention to indicate it resides
		 * in the secure world.
		 */
		if (!ffa_is_secure_world_id(lp_desc->sp_id)) {
			ERROR("Invalid SPMD Logical SP ID (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure SPMD logical partition is in valid range. */
		if (!is_spmd_lp_id(lp_desc->sp_id)) {
			ERROR("Invalid SPMD Logical Partition ID (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure the UUID is not the NULL UUID. */
		if (lp_desc->uuid[0] == 0 && lp_desc->uuid[1] == 0 &&
		    lp_desc->uuid[2] == 0 && lp_desc->uuid[3] == 0) {
			ERROR("Invalid UUID for SPMD Logical SP (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure init function callback is registered. */
		if (lp_desc->init == NULL) {
			ERROR("Missing init function for Logical SP(0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure that SPMD LP only supports sending direct requests. */
		if (lp_desc->properties != FFA_PARTITION_DIRECT_REQ_SEND) {
			ERROR("Invalid SPMD logical partition properties (0x%x)\n",
			      lp_desc->properties);
			return -EINVAL;
		}

		/* Ensure that all partition IDs are unique. */
		for (uint32_t inner_idx = index + 1;
		     inner_idx < SPMD_LP_DESCS_COUNT; inner_idx++) {
			if (lp_desc->sp_id == lp_array[inner_idx].sp_id) {
				ERROR("Duplicate SPMD logical SP ID Detected (0x%x)\n",
				      lp_desc->sp_id);
				return -EINVAL;
			}
		}
	}
	return 0;
}

static void spmd_encode_ffa_error(struct ffa_value *retval, int32_t error_code)
{
	retval->func = FFA_ERROR;
	retval->arg1 = FFA_TARGET_INFO_MBZ;
	retval->arg2 = (uint32_t)error_code;
	retval->arg3 = FFA_TARGET_INFO_MBZ;
	retval->arg4 = FFA_TARGET_INFO_MBZ;
	retval->arg5 = FFA_TARGET_INFO_MBZ;
	retval->arg6 = FFA_TARGET_INFO_MBZ;
	retval->arg7 = FFA_TARGET_INFO_MBZ;
}

static void spmd_build_direct_message_req(spmd_spm_core_context_t *ctx,
					  uint64_t x1, uint64_t x2,
					  uint64_t x3, uint64_t x4)
{
	gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);

	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_MSG_SEND_DIRECT_REQ_SMC32);
	write_ctx_reg(gpregs, CTX_GPREG_X1, x1);
	write_ctx_reg(gpregs, CTX_GPREG_X2, x2);
	write_ctx_reg(gpregs, CTX_GPREG_X3, x3);
	write_ctx_reg(gpregs, CTX_GPREG_X4, x4);
	write_ctx_reg(gpregs, CTX_GPREG_X5, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X6, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X7, 0U);
}

static void spmd_encode_ctx_to_ffa_value(spmd_spm_core_context_t *ctx,
					 struct ffa_value *retval)
{
	gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);

	retval->func = read_ctx_reg(gpregs, CTX_GPREG_X0);
	retval->arg1 = read_ctx_reg(gpregs, CTX_GPREG_X1);
	retval->arg2 = read_ctx_reg(gpregs, CTX_GPREG_X2);
	retval->arg3 = read_ctx_reg(gpregs, CTX_GPREG_X3);
	retval->arg4 = read_ctx_reg(gpregs, CTX_GPREG_X4);
	retval->arg5 = read_ctx_reg(gpregs, CTX_GPREG_X5);
	retval->arg6 = read_ctx_reg(gpregs, CTX_GPREG_X6);
	retval->arg7 = read_ctx_reg(gpregs, CTX_GPREG_X7);
	retval->arg8 = read_ctx_reg(gpregs, CTX_GPREG_X8);
	retval->arg9 = read_ctx_reg(gpregs, CTX_GPREG_X9);
	retval->arg10 = read_ctx_reg(gpregs, CTX_GPREG_X10);
	retval->arg11 = read_ctx_reg(gpregs, CTX_GPREG_X11);
	retval->arg12 = read_ctx_reg(gpregs, CTX_GPREG_X12);
	retval->arg13 = read_ctx_reg(gpregs, CTX_GPREG_X13);
	retval->arg14 = read_ctx_reg(gpregs, CTX_GPREG_X14);
	retval->arg15 = read_ctx_reg(gpregs, CTX_GPREG_X15);
	retval->arg16 = read_ctx_reg(gpregs, CTX_GPREG_X16);
	retval->arg17 = read_ctx_reg(gpregs, CTX_GPREG_X17);
}

static void spmd_logical_sp_set_dir_req_ongoing(spmd_spm_core_context_t *ctx)
{
	ctx->spmd_lp_sync_req_ongoing |= SPMD_LP_FFA_DIR_REQ_ONGOING;
}

static void spmd_logical_sp_reset_dir_req_ongoing(spmd_spm_core_context_t *ctx)
{
	ctx->spmd_lp_sync_req_ongoing &= ~SPMD_LP_FFA_DIR_REQ_ONGOING;
}

static void spmd_build_ffa_info_get_regs(spmd_spm_core_context_t *ctx,
					 const uint32_t uuid[4],
					 const uint16_t start_index,
					 const uint16_t tag)
{
	gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);

	uint64_t arg1 = (uint64_t)uuid[1] << 32 | uuid[0];
	uint64_t arg2 = (uint64_t)uuid[3] << 32 | uuid[2];
	uint64_t arg3 = start_index | (uint64_t)tag << 16;

	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_PARTITION_INFO_GET_REGS_SMC64);
	write_ctx_reg(gpregs, CTX_GPREG_X1, arg1);
	write_ctx_reg(gpregs, CTX_GPREG_X2, arg2);
	write_ctx_reg(gpregs, CTX_GPREG_X3, arg3);
	write_ctx_reg(gpregs, CTX_GPREG_X4, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X5, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X6, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X7, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X8, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X9, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X10, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X11, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X12, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X13, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X14, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X15, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X16, 0U);
	write_ctx_reg(gpregs, CTX_GPREG_X17, 0U);
}

static void spmd_logical_sp_set_info_regs_ongoing(spmd_spm_core_context_t *ctx)
{
	ctx->spmd_lp_sync_req_ongoing |= SPMD_LP_FFA_INFO_GET_REG_ONGOING;
}

static void spmd_logical_sp_reset_info_regs_ongoing(
		spmd_spm_core_context_t *ctx)
{
	ctx->spmd_lp_sync_req_ongoing &= ~SPMD_LP_FFA_INFO_GET_REG_ONGOING;
}

static void spmd_fill_lp_info_array(
	struct ffa_partition_info_v1_1 (*partitions)[EL3_SPMD_MAX_NUM_LP],
	uint32_t uuid[4], uint16_t *lp_count_out)
{
	uint16_t lp_count = 0;
	struct spmd_lp_desc *lp_array;
	bool uuid_is_null = is_null_uuid(uuid);

	if (SPMD_LP_DESCS_COUNT == 0U) {
		*lp_count_out = 0;
		return;
	}

	lp_array = get_spmd_el3_lp_array();
	for (uint16_t index = 0; index < SPMD_LP_DESCS_COUNT; ++index) {
		struct spmd_lp_desc *lp = &lp_array[index];

		if (uuid_is_null || uuid_match(uuid, lp->uuid)) {
			uint16_t array_index = lp_count;

			++lp_count;

			(*partitions)[array_index].ep_id = lp->sp_id;
			(*partitions)[array_index].execution_ctx_count = 1;
			(*partitions)[array_index].properties = lp->properties;
			(*partitions)[array_index].properties |=
				(FFA_PARTITION_INFO_GET_AARCH64_STATE <<
				 FFA_PARTITION_INFO_GET_EXEC_STATE_SHIFT);
			if (uuid_is_null) {
				memcpy(&((*partitions)[array_index].uuid),
					  &lp->uuid, sizeof(lp->uuid));
			}
		}
	}

	*lp_count_out = lp_count;
}

static inline void spmd_pack_lp_count_props(
	uint64_t *xn, uint16_t ep_id, uint16_t vcpu_count,
	uint32_t properties)
{
	*xn = (uint64_t)ep_id;
	*xn |= (uint64_t)vcpu_count << 16;
	*xn |= (uint64_t)properties << 32;
}

static inline void spmd_pack_lp_uuid(uint64_t *xn_1, uint64_t *xn_2,
				     uint32_t uuid[4])
{
	*xn_1 = (uint64_t)uuid[0];
	*xn_1 |= (uint64_t)uuid[1] << 32;
	*xn_2 = (uint64_t)uuid[2];
	*xn_2 |= (uint64_t)uuid[3] << 32;
}
#endif

/*
 * Initialize SPMD logical partitions. This function assumes that it is called
 * only after the SPMC has successfully initialized.
 */
int32_t spmd_logical_sp_init(void)
{
#if ENABLE_SPMD_LP
	int32_t rc = 0;
	struct spmd_lp_desc *spmd_lp_descs;

	assert(SPMD_LP_DESCS_COUNT <= EL3_SPMD_MAX_NUM_LP);

	if (is_spmd_lp_inited == true) {
		return 0;
	}

	if (is_spmc_inited == false) {
		return -1;
	}

	spmd_lp_descs = get_spmd_el3_lp_array();

	/* Perform initial validation of the SPMD Logical Partitions. */
	rc = el3_spmd_sp_desc_validate(spmd_lp_descs);
	if (rc != 0) {
		ERROR("Logical SPMD Partition validation failed!\n");
		return rc;
	}

	VERBOSE("SPMD Logical Secure Partition init start.\n");
	for (unsigned int i = 0U; i < SPMD_LP_DESCS_COUNT; i++) {
		rc = spmd_lp_descs[i].init();
		if (rc != 0) {
			ERROR("SPMD Logical SP (0x%x) failed to initialize\n",
			      spmd_lp_descs[i].sp_id);
			return rc;
		}
		VERBOSE("SPMD Logical SP (0x%x) Initialized\n",
			spmd_lp_descs[i].sp_id);
	}

	INFO("SPMD Logical Secure Partition init completed.\n");
	if (rc == 0) {
		is_spmd_lp_inited = true;
	}
	return rc;
#else
	return 0;
#endif
}

void spmd_logical_sp_set_spmc_initialized(void)
{
#if ENABLE_SPMD_LP
	is_spmc_inited = true;
#endif
}

void spmd_logical_sp_set_spmc_failure(void)
{
#if ENABLE_SPMD_LP
	is_spmc_inited = false;
#endif
}

/*
 * This function takes an ffa_value structure populated with partition
 * information from an FFA_PARTITION_INFO_GET_REGS ABI call, extracts
 * the values and writes it into a ffa_partition_info_v1_1 structure for
 * other code to consume.
 */
bool ffa_partition_info_regs_get_part_info(
	struct ffa_value *args, uint8_t idx,
	struct ffa_partition_info_v1_1 *partition_info)
{
	uint64_t *arg_ptrs;
	uint64_t info, uuid_lo, uuid_high;

	/*
	 * Each partition information is encoded in 3 registers, so there can be
	 * a maximum of 5 entries.
	 */
	if (idx >= 5 || partition_info == NULL) {
		return false;
	}

	/*
	 * List of pointers to args in return value. arg0/func encodes ff-a
	 * function, arg1 is reserved, arg2 encodes indices. arg3 and greater
	 * values reflect partition properties.
	 */
	arg_ptrs = (uint64_t *)args + ((idx * 3) + 3);
	info = *arg_ptrs;

	arg_ptrs++;
	uuid_lo = *arg_ptrs;

	arg_ptrs++;
	uuid_high = *arg_ptrs;

	partition_info->ep_id = (uint16_t)(info & 0xFFFFU);
	partition_info->execution_ctx_count = (uint16_t)((info >> 16) & 0xFFFFU);
	partition_info->properties = (uint32_t)(info >> 32);
	partition_info->uuid[0] = (uint32_t)(uuid_lo & 0xFFFFFFFFU);
	partition_info->uuid[1] = (uint32_t)((uuid_lo >> 32) & 0xFFFFFFFFU);
	partition_info->uuid[2] = (uint32_t)(uuid_high & 0xFFFFFFFFU);
	partition_info->uuid[3] = (uint32_t)((uuid_high >> 32) & 0xFFFFFFFFU);

	return true;
}

/*
 * This function is called by the SPMD in response to
 * an FFA_PARTITION_INFO_GET_REG ABI invocation by the SPMC. Secure partitions
 * are allowed to discover the presence of EL3 SPMD logical partitions by
 * invoking the aforementioned ABI and this function populates the required
 * information about EL3 SPMD logical partitions.
 */
uint64_t spmd_el3_populate_logical_partition_info(void *handle, uint64_t x1,
						  uint64_t x2, uint64_t x3)
{
#if ENABLE_SPMD_LP
	uint32_t target_uuid[4] = { 0 };
	uint32_t w0;
	uint32_t w1;
	uint32_t w2;
	uint32_t w3;
	uint16_t start_index;
	uint16_t tag;
	static struct ffa_partition_info_v1_1 partitions[EL3_SPMD_MAX_NUM_LP];
	uint16_t lp_count = 0;
	uint16_t max_idx = 0;
	uint16_t curr_idx = 0;
	uint8_t num_entries_to_ret = 0;
	struct ffa_value ret = { 0 };
	uint64_t *arg_ptrs = (uint64_t *)&ret + 3;

	w0 = (uint32_t)(x1 & 0xFFFFFFFFU);
	w1 = (uint32_t)(x1 >> 32);
	w2 = (uint32_t)(x2 & 0xFFFFFFFFU);
	w3 = (uint32_t)(x2 >> 32);

	target_uuid[0] = w0;
	target_uuid[1] = w1;
	target_uuid[2] = w2;
	target_uuid[3] = w3;

	start_index = (uint16_t)(x3 & 0xFFFFU);
	tag = (uint16_t)((x3 >> 16) & 0xFFFFU);

	assert(handle == cm_get_context(SECURE));

	if (tag != 0) {
		VERBOSE("Tag is not 0. Cannot return partition info.\n");
		return spmd_ffa_error_return(handle, FFA_ERROR_RETRY);
	}

	memset(&partitions, 0, sizeof(partitions));

	spmd_fill_lp_info_array(&partitions, target_uuid, &lp_count);

	if (lp_count == 0) {
		VERBOSE("No SPDM EL3 logical partitions exist.\n");
		return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	if (start_index >= lp_count) {
		VERBOSE("start_index = %d, lp_count = %d (start index must be"
			" less than partition count.\n",
			start_index, lp_count);
		return spmd_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	max_idx = lp_count - 1;
	num_entries_to_ret = (max_idx - start_index) + 1;
	num_entries_to_ret =
		MIN(num_entries_to_ret, MAX_INFO_REGS_ENTRIES_PER_CALL);
	curr_idx = start_index + num_entries_to_ret - 1;
	assert(curr_idx <= max_idx);

	ret.func = FFA_SUCCESS_SMC64;
	ret.arg2 = (uint64_t)((sizeof(struct ffa_partition_info_v1_1) & 0xFFFFU) << 48);
	ret.arg2 |= (uint64_t)(curr_idx << 16);
	ret.arg2 |= (uint64_t)max_idx;

	for (uint16_t idx = start_index; idx <= curr_idx; ++idx) {
		spmd_pack_lp_count_props(arg_ptrs, partitions[idx].ep_id,
					 partitions[idx].execution_ctx_count,
					 partitions[idx].properties);
		arg_ptrs++;
		if (is_null_uuid(target_uuid)) {
			spmd_pack_lp_uuid(arg_ptrs, (arg_ptrs + 1),
					  partitions[idx].uuid);
		}
		arg_ptrs += 2;
	}

	SMC_RET18(handle, ret.func, ret.arg1, ret.arg2, ret.arg3, ret.arg4,
		  ret.arg5, ret.arg6, ret.arg7, ret.arg8, ret.arg9, ret.arg10,
		  ret.arg11, ret.arg12, ret.arg13, ret.arg14, ret.arg15,
		  ret.arg16, ret.arg17);
#else
	return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
#endif
}

/* This function can be used by an SPMD logical partition to invoke the
 * FFA_PARTITION_INFO_GET_REGS ABI to the SPMC, to discover the secure
 * partitions in the system. The function takes a UUID, start index and
 * tag and the partition information are returned in an ffa_value structure
 * and can be consumed by using appropriate helper functions.
 */
bool spmd_el3_invoke_partition_info_get(
				const uint32_t target_uuid[4],
				const uint16_t start_index,
				const uint16_t tag,
				struct ffa_value *retval)
{
#if ENABLE_SPMD_LP
	uint64_t rc = UINT64_MAX;
	spmd_spm_core_context_t *ctx = spmd_get_context();

	if (retval == NULL) {
		return false;
	}

	memset(retval, 0, sizeof(*retval));

	if (!is_spmc_inited) {
		VERBOSE("Cannot discover partition before,"
			" SPMC is initialized.\n");
			spmd_encode_ffa_error(retval, FFA_ERROR_DENIED);
		return true;
	}

	if (tag != 0) {
		VERBOSE("Tag must be zero. other tags unsupported\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	/* Save the non-secure context before entering SPMC */
	cm_el1_sysregs_context_save(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(NON_SECURE);
#endif

	spmd_build_ffa_info_get_regs(ctx, target_uuid, start_index, tag);
	spmd_logical_sp_set_info_regs_ongoing(ctx);

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%lx) on CPU%u\n", __func__, rc,
		      plat_my_core_pos());
		panic();
	}

	spmd_logical_sp_reset_info_regs_ongoing(ctx);
	spmd_encode_ctx_to_ffa_value(ctx, retval);

	assert(is_ffa_error(retval) || is_ffa_success(retval));

	cm_el1_sysregs_context_restore(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(NON_SECURE);
#endif
	cm_set_next_eret_context(NON_SECURE);
	return true;
#else
	return false;
#endif
}

/*******************************************************************************
 * This function sends an FF-A Direct Request from a partition in EL3 to a
 * partition that may reside under an SPMC (only lower ELs supported). The main
 * use of this API is for SPMD logical partitions.
 * The API is expected to be used when there are platform specific SMCs that
 * need to be routed to a secure partition that is FF-A compliant or when
 * there are group 0 interrupts that need to be handled first in EL3 and then
 * forwarded to an FF-A compliant secure partition. Therefore, it is expected
 * that the handle to the context provided belongs to the non-secure context.
 * This also means that interrupts/SMCs that trap to EL3 during secure execution
 * cannot use this API.
 * x1, x2, x3 and x4 are encoded as specified in the FF-A specification.
 * retval is used to pass the direct response values to the caller.
 * The function returns true if retval has valid values, and false otherwise.
 ******************************************************************************/
bool spmd_el3_ffa_msg_direct_req(uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *handle,
				 struct ffa_value *retval)
{
#if ENABLE_SPMD_LP

	uint64_t rc = UINT64_MAX;
	spmd_spm_core_context_t *ctx = spmd_get_context();

	if (retval == NULL) {
		return false;
	}

	memset(retval, 0, sizeof(*retval));

	if (!is_spmd_lp_inited || !is_spmc_inited) {
		VERBOSE("Cannot send SPMD logical partition direct message,"
			" Partitions not initialized or SPMC not initialized.\n");
			spmd_encode_ffa_error(retval, FFA_ERROR_DENIED);
		return true;
	}

	/*
	 * x2 must be zero, since there is no support for framework message via
	 * an SPMD logical partition. This is sort of a useless check and it is
	 * possible to not take parameter. However, as the framework extends it
	 * may be useful to have x2 and extend this function later with
	 * functionality based on x2.
	 */
	if (x2 != 0) {
		VERBOSE("x2 must be zero. Cannot send framework message.\n");
			spmd_encode_ffa_error(retval, FFA_ERROR_DENIED);
		return true;
	}

	/*
	 * Current context must be non-secure. API is expected to be used
	 * when entry into EL3 and the SPMD logical partition is via an
	 * interrupt that occurs when execution is in normal world and
	 * SMCs from normal world. FF-A compliant SPMCs are expected to
	 * trap interrupts during secure execution in lower ELs since they
	 * are usually not re-entrant and SMCs from secure world can be
	 * handled synchronously. There is no known use case for an SPMD
	 * logical partition to send a direct message to another partition
	 * in response to a secure interrupt or SMCs from secure world.
	 */
	if (handle != cm_get_context(NON_SECURE)) {
		VERBOSE("Handle must be for the non-secure context.\n");
			spmd_encode_ffa_error(retval, FFA_ERROR_DENIED);
		return true;
	}

	if (!is_spmd_lp_id(ffa_endpoint_source(x1))) {
		VERBOSE("Source ID must be valid SPMD logical partition"
			" ID.\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	if (is_spmd_lp_id(ffa_endpoint_destination(x1))) {
		VERBOSE("Destination ID must not be SPMD logical partition"
			" ID.\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	if (!ffa_is_secure_world_id(ffa_endpoint_destination(x1))) {
		VERBOSE("Destination ID must be secure world ID.\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	if (ffa_endpoint_destination(x1) == SPMD_DIRECT_MSG_ENDPOINT_ID) {
		VERBOSE("Destination ID must not be SPMD ID.\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	if (ffa_endpoint_destination(x1) == spmd_spmc_id_get()) {
		VERBOSE("Destination ID must not be SPMC ID.\n");
			spmd_encode_ffa_error(retval,
					      FFA_ERROR_INVALID_PARAMETER);
		return true;
	}

	/* Save the non-secure context before entering SPMC */
	cm_el1_sysregs_context_save(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(NON_SECURE);
#endif

	/*
	 * Perform synchronous entry into the SPMC. Synchronous entry is
	 * required because the spec requires that a direct message request
	 * from an SPMD LP look like a function call from it's perspective.
	 */
	spmd_build_direct_message_req(ctx, x1, x2, x3, x4);
	spmd_logical_sp_set_dir_req_ongoing(ctx);

	rc = spmd_spm_core_sync_entry(ctx);

	spmd_logical_sp_reset_dir_req_ongoing(ctx);

	if (rc != 0ULL) {
		ERROR("%s failed (%lx) on CPU%u\n", __func__, rc,
		      plat_my_core_pos());
		panic();
	} else {
		spmd_encode_ctx_to_ffa_value(ctx, retval);

		/*
		 * Only expect error or direct response,
		 * spmd_spm_core_sync_exit should not be called on other paths.
		 * Checks are asserts since the LSP can fail gracefully if the
		 * source or destination ids are not the same. Panic'ing would
		 * not provide any benefit.
		 */
		assert(is_ffa_error(retval) || is_ffa_direct_msg_resp(retval));
		assert(is_ffa_error(retval) ||
			(ffa_endpoint_destination(retval->arg1) ==
				ffa_endpoint_source(x1)));
		assert(is_ffa_error(retval) ||
			(ffa_endpoint_source(retval->arg1) ==
				ffa_endpoint_destination(x1)));
	}

	cm_el1_sysregs_context_restore(NON_SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(NON_SECURE);
#endif
	cm_set_next_eret_context(NON_SECURE);

	return true;
#else
	return false;
#endif
}

bool is_spmd_logical_sp_info_regs_req_in_progress(
		spmd_spm_core_context_t *ctx)
{
#if ENABLE_SPMD_LP
	return ((ctx->spmd_lp_sync_req_ongoing & SPMD_LP_FFA_INFO_GET_REG_ONGOING)
			== SPMD_LP_FFA_INFO_GET_REG_ONGOING);
#else
	return false;
#endif
}

bool is_spmd_logical_sp_dir_req_in_progress(
		spmd_spm_core_context_t *ctx)
{
#if ENABLE_SPMD_LP
	return ((ctx->spmd_lp_sync_req_ongoing & SPMD_LP_FFA_DIR_REQ_ONGOING)
		== SPMD_LP_FFA_DIR_REQ_ONGOING);
#else
	return false;
#endif
}

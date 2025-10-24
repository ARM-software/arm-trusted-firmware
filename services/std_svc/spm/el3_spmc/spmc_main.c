/*
 * Copyright (c) 2022-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <common/runtime_svc.h>
#include <common/uuid.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include <plat/common/platform.h>
#include <services/el3_spmc_logical_sp.h>
#include <services/ffa_svc.h>
#include <services/spmc_svc.h>
#include <services/spmd_svc.h>
#include "spmc.h"
#include "spmc_shared_mem.h"
#if TRANSFER_LIST
#include <transfer_list.h>
#endif

#include <platform_def.h>

/* FFA_MEM_PERM_* helpers */
#define FFA_MEM_PERM_MASK		U(7)
#define FFA_MEM_PERM_DATA_MASK		U(3)
#define FFA_MEM_PERM_DATA_SHIFT		U(0)
#define FFA_MEM_PERM_DATA_NA		U(0)
#define FFA_MEM_PERM_DATA_RW		U(1)
#define FFA_MEM_PERM_DATA_RES		U(2)
#define FFA_MEM_PERM_DATA_RO		U(3)
#define FFA_MEM_PERM_INST_EXEC          (U(0) << 2)
#define FFA_MEM_PERM_INST_NON_EXEC      (U(1) << 2)

/* Declare the maximum number of SPs and El3 LPs. */
#define MAX_SP_LP_PARTITIONS (SECURE_PARTITION_COUNT + MAX_EL3_LP_DESCS_COUNT)

#define FFA_VERSION_SPMC_MAJOR U(1)
#define FFA_VERSION_SPMC_MINOR U(2)

/*
 * Allocate a secure partition descriptor to describe each SP in the system that
 * does not reside at EL3.
 */
static struct secure_partition_desc sp_desc[SECURE_PARTITION_COUNT];

/*
 * Allocate an NS endpoint descriptor to describe each VM and the Hypervisor in
 * the system that interacts with a SP. It is used to track the Hypervisor
 * buffer pair, version and ID for now. It could be extended to track VM
 * properties when the SPMC supports indirect messaging.
 */
static struct ns_endpoint_desc ns_ep_desc[NS_PARTITION_COUNT];

static uint64_t spmc_sp_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie);

/*
 * Helper function to obtain the array storing the EL3
 * Logical Partition descriptors.
 */
struct el3_lp_desc *get_el3_lp_array(void)
{
	return (struct el3_lp_desc *) EL3_LP_DESCS_START;
}

/*
 * Helper function to obtain the descriptor of the last SP to whom control was
 * handed to on this physical cpu. Currently, we assume there is only one SP.
 * TODO: Expand to track multiple partitions when required.
 */
struct secure_partition_desc *spmc_get_current_sp_ctx(void)
{
	return &(sp_desc[ACTIVE_SP_DESC_INDEX]);
}

/*
 * Helper function to obtain the execution context of an SP on the
 * current physical cpu.
 */
struct sp_exec_ctx *spmc_get_sp_ec(struct secure_partition_desc *sp)
{
	return &(sp->ec[get_ec_index(sp)]);
}

/* Helper function to get pointer to SP context from its ID. */
struct secure_partition_desc *spmc_get_sp_ctx(uint16_t id)
{
	/* Check for Secure World Partitions. */
	for (unsigned int i = 0U; i < SECURE_PARTITION_COUNT; i++) {
		if (sp_desc[i].sp_id == id) {
			return &(sp_desc[i]);
		}
	}
	return NULL;
}

/*
 * Helper function to obtain the descriptor of the Hypervisor or OS kernel.
 * We assume that the first descriptor is reserved for this entity.
 */
struct ns_endpoint_desc *spmc_get_hyp_ctx(void)
{
	return &(ns_ep_desc[0]);
}

/*
 * Helper function to obtain the RX/TX buffer pair descriptor of the Hypervisor
 * or OS kernel in the normal world or the last SP that was run.
 */
struct mailbox *spmc_get_mbox_desc(bool secure_origin)
{
	/* Obtain the RX/TX buffer pair descriptor. */
	if (secure_origin) {
		return &(spmc_get_current_sp_ctx()->mailbox);
	} else {
		return &(spmc_get_hyp_ctx()->mailbox);
	}
}

/******************************************************************************
 * This function returns to the place where spmc_sp_synchronous_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void spmc_sp_synchronous_exit(struct sp_exec_ctx *ec, uint64_t rc)
{
	/*
	 * The SPM must have initiated the original request through a
	 * synchronous entry into the secure partition. Jump back to the
	 * original C runtime context with the value of rc in x0;
	 */
	spm_secure_partition_exit(ec->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Return FFA_ERROR with specified error code.
 ******************************************************************************/
uint64_t spmc_ffa_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, FFA_ERROR,
		 FFA_TARGET_INFO_MBZ, error_code,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ);
}

/******************************************************************************
 * Helper function to validate a secure partition ID to ensure it does not
 * conflict with any other FF-A component and follows the convention to
 * indicate it resides within the secure world.
 ******************************************************************************/
bool is_ffa_secure_id_valid(uint16_t partition_id)
{
	struct el3_lp_desc *el3_lp_descs = get_el3_lp_array();

	/* Ensure the ID is not the invalid partition ID. */
	if (partition_id == INV_SP_ID) {
		return false;
	}

	/* Ensure the ID is not the SPMD ID. */
	if (partition_id == SPMD_DIRECT_MSG_ENDPOINT_ID) {
		return false;
	}

	/*
	 * Ensure the ID follows the convention to indicate it resides
	 * in the secure world.
	 */
	if (!ffa_is_secure_world_id(partition_id)) {
		return false;
	}

	/* Ensure we don't conflict with the SPMC partition ID. */
	if (partition_id == FFA_SPMC_ID) {
		return false;
	}

	/* Ensure we do not already have an SP context with this ID. */
	if (spmc_get_sp_ctx(partition_id)) {
		return false;
	}

	/* Ensure we don't clash with any Logical SP's. */
	for (unsigned int i = 0U; i < EL3_LP_DESCS_COUNT; i++) {
		if (el3_lp_descs[i].sp_id == partition_id) {
			return false;
		}
	}

	return true;
}

/*******************************************************************************
 * This function either forwards the request to the other world or returns
 * with an ERET depending on the source of the call.
 * We can assume that the destination is for an entity at a lower exception
 * level as any messages destined for a logical SP resident in EL3 will have
 * already been taken care of by the SPMC before entering this function.
 ******************************************************************************/
static uint64_t spmc_smc_return(uint32_t smc_fid,
				bool secure_origin,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *handle,
				void *cookie,
				uint64_t flags,
				uint16_t dst_id,
				uint32_t sp_ffa_version)
{
	/* If the destination is in the normal world always go via the SPMD. */
	if (ffa_is_normal_world_id(dst_id)) {
		return spmd_smc_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags, sp_ffa_version);
	}
	/*
	 * If the caller is secure and we want to return to the secure world,
	 * ERET directly.
	 */
	else if (secure_origin && ffa_is_secure_world_id(dst_id)) {
		SMC_RET5(handle, smc_fid, x1, x2, x3, x4);
	}
	/* If we originated in the normal world then switch contexts. */
	else if (!secure_origin && ffa_is_secure_world_id(dst_id)) {
		return spmd_smc_switch_state(smc_fid, secure_origin, x1, x2,
					     x3, x4, handle, flags, sp_ffa_version);
	} else {
		/* Unknown State. */
		panic();
	}

	/* Shouldn't be Reached. */
	return 0;
}

/*******************************************************************************
 * FF-A ABI Handlers.
 ******************************************************************************/

/*******************************************************************************
 * Helper function to validate arg2 as part of a direct message.
 ******************************************************************************/
static inline bool direct_msg_validate_arg2(uint64_t x2)
{
	/* Check message type. */
	if (x2 & FFA_FWK_MSG_BIT) {
		/* We have a framework message, ensure it is a known message. */
		if (x2 & ~(FFA_FWK_MSG_MASK | FFA_FWK_MSG_BIT)) {
			VERBOSE("Invalid message format 0x%lx.\n", x2);
			return false;
		}
	} else {
		/* We have a partition messages, ensure x2 is not set. */
		if (x2 != (uint64_t) 0) {
			VERBOSE("Arg2 MBZ for partition messages. (0x%lx).\n",
				x2);
			return false;
		}
	}
	return true;
}

/*******************************************************************************
 * Helper function to validate the destination ID of a direct response.
 ******************************************************************************/
static bool direct_msg_validate_dst_id(uint16_t dst_id)
{
	struct secure_partition_desc *sp;

	/* Check if we're targeting a normal world partition. */
	if (ffa_is_normal_world_id(dst_id)) {
		return true;
	}

	/* Or directed to the SPMC itself.*/
	if (dst_id == FFA_SPMC_ID) {
		return true;
	}

	/* Otherwise ensure the SP exists. */
	sp = spmc_get_sp_ctx(dst_id);
	if (sp != NULL) {
		return true;
	}

	return false;
}

/*******************************************************************************
 * Helper function to validate the response from a Logical Partition.
 ******************************************************************************/
static bool direct_msg_validate_lp_resp(uint16_t origin_id, uint16_t lp_id,
					void *handle)
{
	/* Retrieve populated Direct Response Arguments. */
	uint64_t smc_fid = SMC_GET_GP(handle, CTX_GPREG_X0);
	uint64_t x1 = SMC_GET_GP(handle, CTX_GPREG_X1);
	uint64_t x2 = SMC_GET_GP(handle, CTX_GPREG_X2);
	uint16_t src_id = ffa_endpoint_source(x1);
	uint16_t dst_id = ffa_endpoint_destination(x1);

	if (src_id != lp_id) {
		ERROR("Invalid EL3 LP source ID (0x%x).\n", src_id);
		return false;
	}

	/*
	 * Check the destination ID is valid and ensure the LP is responding to
	 * the original request.
	 */
	if ((!direct_msg_validate_dst_id(dst_id)) || (dst_id != origin_id)) {
		ERROR("Invalid EL3 LP destination ID (0x%x).\n", dst_id);
		return false;
	}

	if ((smc_fid != FFA_MSG_SEND_DIRECT_RESP2_SMC64) &&
			!direct_msg_validate_arg2(x2)) {
		ERROR("Invalid EL3 LP message encoding.\n");
		return false;
	}
	return true;
}

/*******************************************************************************
 * Helper function to check that partition can receive direct msg or not.
 ******************************************************************************/
static bool direct_msg_receivable(uint32_t properties, uint16_t dir_req_fnum)
{
	if ((dir_req_fnum == FFA_FNUM_MSG_SEND_DIRECT_REQ &&
			((properties & FFA_PARTITION_DIRECT_REQ_RECV) == 0U)) ||
			(dir_req_fnum == FFA_FNUM_MSG_SEND_DIRECT_REQ2 &&
			((properties & FFA_PARTITION_DIRECT_REQ2_RECV) == 0U))) {
		return false;
	}

	return true;
}

/*******************************************************************************
 * Helper function to obtain the FF-A version of the calling partition.
 ******************************************************************************/
uint32_t get_partition_ffa_version(bool secure_origin)
{
	if (secure_origin) {
		return spmc_get_current_sp_ctx()->ffa_version;
	} else {
		return spmc_get_hyp_ctx()->ffa_version;
	}
}

/*******************************************************************************
 * Handle direct request messages and route to the appropriate destination.
 ******************************************************************************/
static uint64_t direct_req_smc_handler(uint32_t smc_fid,
				       bool secure_origin,
				       uint64_t x1,
				       uint64_t x2,
				       uint64_t x3,
				       uint64_t x4,
				       void *cookie,
				       void *handle,
				       uint64_t flags)
{
	uint16_t src_id = ffa_endpoint_source(x1);
	uint16_t dst_id = ffa_endpoint_destination(x1);
	uint16_t dir_req_funcid;
	struct el3_lp_desc *el3_lp_descs;
	struct secure_partition_desc *sp;
	unsigned int idx;
	uint32_t ffa_version = get_partition_ffa_version(secure_origin);

	dir_req_funcid = (smc_fid != FFA_MSG_SEND_DIRECT_REQ2_SMC64) ?
		FFA_FNUM_MSG_SEND_DIRECT_REQ : FFA_FNUM_MSG_SEND_DIRECT_REQ2;

	if ((dir_req_funcid == FFA_FNUM_MSG_SEND_DIRECT_REQ2) &&
			ffa_version < MAKE_FFA_VERSION(U(1), U(2))) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/*
	 * Sanity check for DIRECT_REQ:
	 * Check if arg2 has been populated correctly based on message type
	 */
	if ((dir_req_funcid == FFA_FNUM_MSG_SEND_DIRECT_REQ) &&
			!direct_msg_validate_arg2(x2)) {
		return spmc_ffa_error_return(handle, FFA_ERROR_INVALID_PARAMETER);
	}

	/* Validate Sender is either the current SP or from the normal world. */
	if ((secure_origin && src_id != spmc_get_current_sp_ctx()->sp_id) ||
		(!secure_origin && !ffa_is_normal_world_id(src_id))) {
		ERROR("Invalid direct request source ID (0x%x).\n", src_id);
		return spmc_ffa_error_return(handle,
					FFA_ERROR_INVALID_PARAMETER);
	}

	el3_lp_descs = get_el3_lp_array();

	/* Check if the request is destined for a Logical Partition. */
	for (unsigned int i = 0U; i < MAX_EL3_LP_DESCS_COUNT; i++) {
		if (el3_lp_descs[i].sp_id == dst_id) {
			if (!direct_msg_receivable(el3_lp_descs[i].properties, dir_req_funcid)) {
				return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
			}

			uint64_t ret = el3_lp_descs[i].direct_req(
						smc_fid, secure_origin, x1, x2,
						x3, x4, cookie, handle, flags);
			if (!direct_msg_validate_lp_resp(src_id, dst_id,
							 handle)) {
				panic();
			}

			/* Message checks out. */
			return ret;
		}
	}

	/*
	 * If the request was not targeted to a LSP and from the secure world
	 * then it is invalid since a SP cannot call into the Normal world and
	 * there is no other SP to call into. If there are other SPs in future
	 * then the partition runtime model would need to be validated as well.
	 */
	if (secure_origin) {
		VERBOSE("Direct request not supported to the Normal World.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check if the SP ID is valid. */
	sp = spmc_get_sp_ctx(dst_id);
	if (sp == NULL) {
		VERBOSE("Direct request to unknown partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (!direct_msg_receivable(sp->properties, dir_req_funcid)) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Protect the runtime state of a UP S-EL0 SP with a lock. */
	if (sp->runtime_el == S_EL0) {
		spin_lock(&sp->rt_state_lock);
	}

	/*
	 * Check that the target execution context is in a waiting state before
	 * forwarding the direct request to it.
	 */
	idx = get_ec_index(sp);
	if (sp->ec[idx].rt_state != RT_STATE_WAITING) {
		VERBOSE("SP context on core%u is not waiting (%u).\n",
			idx, sp->ec[idx].rt_model);

		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}

		return spmc_ffa_error_return(handle, FFA_ERROR_BUSY);
	}

	/*
	 * Everything checks out so forward the request to the SP after updating
	 * its state and runtime model.
	 */
	sp->ec[idx].rt_state = RT_STATE_RUNNING;
	sp->ec[idx].rt_model = RT_MODEL_DIR_REQ;
	sp->ec[idx].dir_req_origin_id = src_id;
	sp->ec[idx].dir_req_funcid = dir_req_funcid;

	if (sp->runtime_el == S_EL0) {
		spin_unlock(&sp->rt_state_lock);
	}

	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, dst_id, sp->ffa_version);
}

/*******************************************************************************
 * Handle direct response messages and route to the appropriate destination.
 ******************************************************************************/
static uint64_t direct_resp_smc_handler(uint32_t smc_fid,
					bool secure_origin,
					uint64_t x1,
					uint64_t x2,
					uint64_t x3,
					uint64_t x4,
					void *cookie,
					void *handle,
					uint64_t flags)
{
	uint16_t dst_id = ffa_endpoint_destination(x1);
	uint16_t dir_req_funcid;
	struct secure_partition_desc *sp;
	unsigned int idx;

	dir_req_funcid = (smc_fid != FFA_MSG_SEND_DIRECT_RESP2_SMC64) ?
		FFA_FNUM_MSG_SEND_DIRECT_REQ : FFA_FNUM_MSG_SEND_DIRECT_REQ2;

	/* Check if arg2 has been populated correctly based on message type. */
	if (!direct_msg_validate_arg2(x2)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check that the response did not originate from the Normal world. */
	if (!secure_origin) {
		VERBOSE("Direct Response not supported from Normal World.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Check that the response is either targeted to the Normal world or the
	 * SPMC e.g. a PM response.
	 */
	if (!direct_msg_validate_dst_id(dst_id)) {
		VERBOSE("Direct response to invalid partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Obtain the SP descriptor and update its runtime state. */
	sp = spmc_get_sp_ctx(ffa_endpoint_source(x1));
	if (sp == NULL) {
		VERBOSE("Direct response to unknown partition ID (0x%x).\n",
			dst_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (sp->runtime_el == S_EL0) {
		spin_lock(&sp->rt_state_lock);
	}

	/* Sanity check state is being tracked correctly in the SPMC. */
	idx = get_ec_index(sp);
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model != RT_MODEL_DIR_REQ) {
		VERBOSE("SP context on core%u not handling direct req (%u).\n",
			idx, sp->ec[idx].rt_model);
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	if (dir_req_funcid != sp->ec[idx].dir_req_funcid) {
		WARN("Unmatched direct req/resp func id. req:%x, resp:%x on core%u.\n",
		     sp->ec[idx].dir_req_funcid, (smc_fid & FUNCID_NUM_MASK), idx);
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	if (sp->ec[idx].dir_req_origin_id != dst_id) {
		WARN("Invalid direct resp partition ID 0x%x != 0x%x on core%u.\n",
		     dst_id, sp->ec[idx].dir_req_origin_id, idx);
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

	/* Clear the ongoing direct request ID. */
	sp->ec[idx].dir_req_origin_id = INV_SP_ID;

	/* Clear the ongoing direct request message version. */
	sp->ec[idx].dir_req_funcid = 0U;

	if (sp->runtime_el == S_EL0) {
		spin_unlock(&sp->rt_state_lock);
	}

	/*
	 * If the receiver is not the SPMC then forward the response to the
	 * Normal world.
	 */
	if (dst_id == FFA_SPMC_ID) {
		spmc_sp_synchronous_exit(&sp->ec[idx], x4);
		/* Should not get here. */
		panic();
	}

	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, dst_id, sp->ffa_version);
}

/*******************************************************************************
 * This function handles the FFA_MSG_WAIT SMC to allow an SP to relinquish its
 * cycles.
 ******************************************************************************/
static uint64_t msg_wait_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;

	/*
	 * Check that the response did not originate from the Normal world as
	 * only the secure world can call this ABI.
	 */
	if (!secure_origin) {
		VERBOSE("Normal world cannot call FFA_MSG_WAIT.\n");
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the descriptor of the SP that invoked FFA_MSG_WAIT. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Get the execution context of the SP that invoked FFA_MSG_WAIT.
	 */
	idx = get_ec_index(sp);
	if (sp->runtime_el == S_EL0) {
		spin_lock(&sp->rt_state_lock);
	}

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model == RT_MODEL_DIR_REQ) {
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Sanity check the state is being tracked correctly in the SPMC. */
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/*
	 * Perform a synchronous exit if the partition was initialising. The
	 * state is updated after the exit.
	 */
	if (sp->ec[idx].rt_model == RT_MODEL_INIT) {
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		spmc_sp_synchronous_exit(&sp->ec[idx], x4);
		/* Should not get here */
		panic();
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

	/* Resume normal world if a secure interrupt was handled. */
	if (sp->ec[idx].rt_model == RT_MODEL_INTR) {
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}

		return spmd_smc_switch_state(FFA_NORMAL_WORLD_RESUME, secure_origin,
					     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					     handle, flags, sp->ffa_version);
	}

	/* Protect the runtime state of a S-EL0 SP with a lock. */
	if (sp->runtime_el == S_EL0) {
		spin_unlock(&sp->rt_state_lock);
	}

	/* Forward the response to the Normal world. */
	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, FFA_NWD_ID, sp->ffa_version);
}

static uint64_t ffa_error_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;
	uint16_t dst_id = ffa_endpoint_destination(x1);
	bool cancel_dir_req = false;

	/* Check that the response did not originate from the Normal world. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the descriptor of the SP that invoked FFA_ERROR. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Get the execution context of the SP that invoked FFA_ERROR. */
	idx = get_ec_index(sp);

	/*
	 * We only expect FFA_ERROR to be received during SP initialisation
	 * otherwise this is an invalid call.
	 */
	if (sp->ec[idx].rt_model == RT_MODEL_INIT) {
		ERROR("SP 0x%x failed to initialize.\n", sp->sp_id);
		spmc_sp_synchronous_exit(&sp->ec[idx], x2);
		/* Should not get here. */
		panic();
	}

	if (sp->runtime_el == S_EL0) {
		spin_lock(&sp->rt_state_lock);
	}

	if (sp->ec[idx].rt_state == RT_STATE_RUNNING &&
			sp->ec[idx].rt_model == RT_MODEL_DIR_REQ) {
		sp->ec[idx].rt_state = RT_STATE_WAITING;
		sp->ec[idx].dir_req_origin_id = INV_SP_ID;
		sp->ec[idx].dir_req_funcid = 0x00;
		cancel_dir_req = true;
	}

	if (sp->runtime_el == S_EL0) {
		spin_unlock(&sp->rt_state_lock);
	}

	if (cancel_dir_req) {
		if (dst_id == FFA_SPMC_ID) {
			spmc_sp_synchronous_exit(&sp->ec[idx], x4);
			/* Should not get here. */
			panic();
		} else
			return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
					       handle, cookie, flags, dst_id, sp->ffa_version);
	}

	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

static uint64_t ffa_version_handler(uint32_t smc_fid,
				    bool secure_origin,
				    uint64_t x1,
				    uint64_t x2,
				    uint64_t x3,
				    uint64_t x4,
				    void *cookie,
				    void *handle,
				    uint64_t flags)
{
	uint32_t requested_version = x1 & FFA_VERSION_MASK;

	if (requested_version & FFA_VERSION_BIT31_MASK) {
		/* Invalid encoding, return an error. */
		SMC_RET1(handle, FFA_ERROR_NOT_SUPPORTED);
		/* Execution stops here. */
	}

	/* Determine the caller to store the requested version. */
	if (secure_origin) {
		/*
		 * Ensure that the SP is reporting the same version as
		 * specified in its manifest. If these do not match there is
		 * something wrong with the SP.
		 * TODO: Should we abort the SP? For now assert this is not
		 *       case.
		 */
		assert(requested_version ==
		       spmc_get_current_sp_ctx()->ffa_version);
	} else {
		/*
		 * If this is called by the normal world, record this
		 * information in its descriptor.
		 */
		spmc_get_hyp_ctx()->ffa_version = requested_version;
	}

	SMC_RET1(handle, MAKE_FFA_VERSION(FFA_VERSION_SPMC_MAJOR,
					  FFA_VERSION_SPMC_MINOR));
}

static uint64_t rxtx_map_handler(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *cookie,
				 void *handle,
				 uint64_t flags)
{
	int ret;
	uint32_t error_code;
	uint32_t mem_atts = secure_origin ? MT_SECURE : MT_NS;
	struct mailbox *mbox;
	uintptr_t tx_address = x1;
	uintptr_t rx_address = x2;
	uint32_t page_count = x3 & FFA_RXTX_PAGE_COUNT_MASK; /* Bits [5:0] */
	uint32_t buf_size = page_count * FFA_PAGE_SIZE;

	/*
	 * The SPMC does not support mapping of VM RX/TX pairs to facilitate
	 * indirect messaging with SPs. Check if the Hypervisor has invoked this
	 * ABI on behalf of a VM and reject it if this is the case.
	 */
	if (tx_address == 0 || rx_address == 0) {
		WARN("Mapping RX/TX Buffers on behalf of VM not supported.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Ensure the specified buffers are not the same. */
	if (tx_address == rx_address) {
		WARN("TX Buffer must not be the same as RX Buffer.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Ensure the buffer size is not 0. */
	if (buf_size == 0U) {
		WARN("Buffer size must not be 0\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Ensure the buffer size is a multiple of the translation granule size
	 * in TF-A.
	 */
	if (buf_size % PAGE_SIZE != 0U) {
		WARN("Buffer size must be aligned to translation granule.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Obtain the RX/TX buffer pair descriptor. */
	mbox = spmc_get_mbox_desc(secure_origin);

	spin_lock(&mbox->lock);

	/* Check if buffers have already been mapped. */
	if (mbox->rx_buffer != 0 || mbox->tx_buffer != 0) {
		WARN("RX/TX Buffers already mapped (%p/%p)\n",
		     (void *) mbox->rx_buffer, (void *)mbox->tx_buffer);
		error_code = FFA_ERROR_DENIED;
		goto err;
	}

	/* memmap the TX buffer as read only. */
	ret = mmap_add_dynamic_region(tx_address, /* PA */
			tx_address, /* VA */
			buf_size, /* size */
			mem_atts | MT_RO_DATA); /* attrs */
	if (ret != 0) {
		/* Return the correct error code. */
		error_code = (ret == -ENOMEM) ? FFA_ERROR_NO_MEMORY :
						FFA_ERROR_INVALID_PARAMETER;
		WARN("Unable to map TX buffer: %d\n", error_code);
		goto err;
	}

	/* memmap the RX buffer as read write. */
	ret = mmap_add_dynamic_region(rx_address, /* PA */
			rx_address, /* VA */
			buf_size, /* size */
			mem_atts | MT_RW_DATA); /* attrs */

	if (ret != 0) {
		error_code = (ret == -ENOMEM) ? FFA_ERROR_NO_MEMORY :
						FFA_ERROR_INVALID_PARAMETER;
		WARN("Unable to map RX buffer: %d\n", error_code);
		/* Unmap the TX buffer again. */
		mmap_remove_dynamic_region(tx_address, buf_size);
		goto err;
	}

	mbox->tx_buffer = (void *) tx_address;
	mbox->rx_buffer = (void *) rx_address;
	mbox->rxtx_page_count = page_count;
	spin_unlock(&mbox->lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
	/* Execution stops here. */
err:
	spin_unlock(&mbox->lock);
	return spmc_ffa_error_return(handle, error_code);
}

static uint64_t rxtx_unmap_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);
	uint32_t buf_size = mbox->rxtx_page_count * FFA_PAGE_SIZE;

	/*
	 * The SPMC does not support mapping of VM RX/TX pairs to facilitate
	 * indirect messaging with SPs. Check if the Hypervisor has invoked this
	 * ABI on behalf of a VM and reject it if this is the case.
	 */
	if (x1 != 0UL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&mbox->lock);

	/* Check if buffers are currently mapped. */
	if (mbox->rx_buffer == 0 || mbox->tx_buffer == 0) {
		spin_unlock(&mbox->lock);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Unmap RX Buffer */
	if (mmap_remove_dynamic_region((uintptr_t) mbox->rx_buffer,
				       buf_size) != 0) {
		WARN("Unable to unmap RX buffer!\n");
	}

	mbox->rx_buffer = 0;

	/* Unmap TX Buffer */
	if (mmap_remove_dynamic_region((uintptr_t) mbox->tx_buffer,
				       buf_size) != 0) {
		WARN("Unable to unmap TX buffer!\n");
	}

	mbox->tx_buffer = 0;
	mbox->rxtx_page_count = 0;

	spin_unlock(&mbox->lock);
	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*
 * Helper function to populate the properties field of a Partition Info Get
 * descriptor.
 */
static uint32_t
partition_info_get_populate_properties(uint32_t sp_properties,
				       enum sp_execution_state sp_ec_state)
{
	uint32_t properties = sp_properties;
	uint32_t ec_state;

	/* Determine the execution state of the SP. */
	ec_state = sp_ec_state == SP_STATE_AARCH64 ?
		   FFA_PARTITION_INFO_GET_AARCH64_STATE :
		   FFA_PARTITION_INFO_GET_AARCH32_STATE;

	properties |= ec_state << FFA_PARTITION_INFO_GET_EXEC_STATE_SHIFT;

	return properties;
}

/*
 * Collate the partition information in a v1.1 partition information
 * descriptor format, this will be converter later if required.
 */
static int partition_info_get_handler_v1_1(uint32_t *uuid,
					   struct ffa_partition_info_v1_1
						  *partitions,
					   uint32_t max_partitions,
					   uint32_t *partition_count)
{
	uint32_t index;
	struct ffa_partition_info_v1_1 *desc;
	bool null_uuid = is_null_uuid(uuid);
	struct el3_lp_desc *el3_lp_descs = get_el3_lp_array();

	/* Deal with Logical Partitions. */
	for (index = 0U; index < EL3_LP_DESCS_COUNT; index++) {
		if (null_uuid || uuid_match(uuid, el3_lp_descs[index].uuid)) {
			/* Found a matching UUID, populate appropriately. */
			if (*partition_count >= max_partitions) {
				return FFA_ERROR_NO_MEMORY;
			}

			desc = &partitions[*partition_count];
			desc->ep_id = el3_lp_descs[index].sp_id;
			desc->execution_ctx_count = PLATFORM_CORE_COUNT;
			/* LSPs must be AArch64. */
			desc->properties =
				partition_info_get_populate_properties(
					el3_lp_descs[index].properties,
					SP_STATE_AARCH64);

			if (null_uuid) {
				copy_uuid(desc->uuid, el3_lp_descs[index].uuid);
			}
			(*partition_count)++;
		}
	}

	/* Deal with physical SP's. */
	for (index = 0U; index < SECURE_PARTITION_COUNT; index++) {
		uint32_t uuid_index;
		uint32_t *sp_uuid;

		for (uuid_index = 0; uuid_index < sp_desc[index].num_uuids;
		     uuid_index++) {
			sp_uuid = sp_desc[index].uuid_array[uuid_index].uuid;

			if (null_uuid || uuid_match(uuid, sp_uuid)) {
				/* Found a matching UUID, populate appropriately. */

				if (*partition_count >= max_partitions) {
					return FFA_ERROR_NO_MEMORY;
				}

				desc = &partitions[*partition_count];
				desc->ep_id = sp_desc[index].sp_id;
				/*
				 * Execution context count must match No. cores for
				 * S-EL1 SPs.
				 */
				desc->execution_ctx_count = PLATFORM_CORE_COUNT;
				desc->properties =
					partition_info_get_populate_properties(
						sp_desc[index].properties,
						sp_desc[index].execution_state);

				(*partition_count)++;
				if (null_uuid) {
					copy_uuid(desc->uuid, sp_uuid);
				} else {
					/* Found UUID in this SP, go to next SP */
					break;
				}
			}
		}
	}
	return 0;
}

/*
 * Handle the case where that caller only wants the count of partitions
 * matching a given UUID and does not want the corresponding descriptors
 * populated.
 */
static uint32_t partition_info_get_handler_count_only(uint32_t *uuid)
{
	uint32_t index = 0;
	uint32_t partition_count = 0;
	bool null_uuid = is_null_uuid(uuid);
	struct el3_lp_desc *el3_lp_descs = get_el3_lp_array();

	/* Deal with Logical Partitions. */
	for (index = 0U; index < EL3_LP_DESCS_COUNT; index++) {
		if (null_uuid ||
		    uuid_match(uuid, el3_lp_descs[index].uuid)) {
			(partition_count)++;
		}
	}

	/* Deal with physical SP's. */
	for (index = 0U; index < SECURE_PARTITION_COUNT; index++) {
		uint32_t uuid_index;

		for (uuid_index = 0; uuid_index < sp_desc[index].num_uuids;
		     uuid_index++) {
			uint32_t *sp_uuid =
				sp_desc[index].uuid_array[uuid_index].uuid;

			if (null_uuid) {
				(partition_count)++;
			} else if (uuid_match(uuid, sp_uuid)) {
				(partition_count)++;
				/* Found a match, go to next SP */
				break;
			}
		}
	}
	return partition_count;
}

/*
 * If the caller of the PARTITION_INFO_GET ABI was a v1.0 caller, populate
 * the corresponding descriptor format from the v1.1 descriptor array.
 */
static uint64_t partition_info_populate_v1_0(struct ffa_partition_info_v1_1
					     *partitions,
					     struct mailbox *mbox,
					     int partition_count)
{
	uint32_t index;
	uint32_t buf_size;
	uint32_t descriptor_size;
	struct ffa_partition_info_v1_0 *v1_0_partitions =
		(struct ffa_partition_info_v1_0 *) mbox->rx_buffer;

	buf_size = mbox->rxtx_page_count * FFA_PAGE_SIZE;
	descriptor_size = partition_count *
			  sizeof(struct ffa_partition_info_v1_0);

	if (descriptor_size > buf_size) {
		return FFA_ERROR_NO_MEMORY;
	}

	for (index = 0U; index < partition_count; index++) {
		v1_0_partitions[index].ep_id = partitions[index].ep_id;
		v1_0_partitions[index].execution_ctx_count =
			partitions[index].execution_ctx_count;
		/* Only report v1.0 properties. */
		v1_0_partitions[index].properties =
			(partitions[index].properties &
			FFA_PARTITION_INFO_GET_PROPERTIES_V1_0_MASK);
	}
	return 0;
}

/*
 * Main handler for FFA_PARTITION_INFO_GET which supports both FF-A v1.1 and
 * v1.0 implementations.
 */
static uint64_t partition_info_get_handler(uint32_t smc_fid,
					   bool secure_origin,
					   uint64_t x1,
					   uint64_t x2,
					   uint64_t x3,
					   uint64_t x4,
					   void *cookie,
					   void *handle,
					   uint64_t flags)
{
	int ret;
	uint32_t partition_count = 0;
	uint32_t size = 0;
	uint32_t ffa_version = get_partition_ffa_version(secure_origin);
	struct mailbox *mbox;
	uint64_t info_get_flags;
	bool count_only;
	uint32_t uuid[4];

	uuid[0] = x1;
	uuid[1] = x2;
	uuid[2] = x3;
	uuid[3] = x4;

	/* Determine if the Partition descriptors should be populated. */
	info_get_flags = SMC_GET_GP(handle, CTX_GPREG_X5);
	count_only = (info_get_flags & FFA_PARTITION_INFO_GET_COUNT_FLAG_MASK);

	/* Handle the case where we don't need to populate the descriptors. */
	if (count_only) {
		partition_count = partition_info_get_handler_count_only(uuid);
		if (partition_count == 0) {
			return spmc_ffa_error_return(handle,
						FFA_ERROR_INVALID_PARAMETER);
		}
	} else {
		struct ffa_partition_info_v1_1
			partitions[MAX_SP_LP_PARTITIONS *
				   SPMC_AT_EL3_PARTITION_MAX_UUIDS];
		/*
		 * Handle the case where the partition descriptors are required,
		 * check we have the buffers available and populate the
		 * appropriate structure version.
		 */

		/* Obtain the v1.1 format of the descriptors. */
		ret = partition_info_get_handler_v1_1(
			uuid, partitions,
			(MAX_SP_LP_PARTITIONS *
			 SPMC_AT_EL3_PARTITION_MAX_UUIDS),
			&partition_count);

		/* Check if an error occurred during discovery. */
		if (ret != 0) {
			goto err;
		}

		/* If we didn't find any matches the UUID is unknown. */
		if (partition_count == 0) {
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err;
		}

		/* Obtain the partition mailbox RX/TX buffer pair descriptor. */
		mbox = spmc_get_mbox_desc(secure_origin);

		/*
		 * If the caller has not bothered registering its RX/TX pair
		 * then return an error code.
		 */
		spin_lock(&mbox->lock);
		if (mbox->rx_buffer == NULL) {
			ret = FFA_ERROR_BUSY;
			goto err_unlock;
		}

		/* Ensure the RX buffer is currently free. */
		if (mbox->state != MAILBOX_STATE_EMPTY) {
			ret = FFA_ERROR_BUSY;
			goto err_unlock;
		}

		/* Zero the RX buffer before populating. */
		(void)memset(mbox->rx_buffer, 0,
			     mbox->rxtx_page_count * FFA_PAGE_SIZE);

		/*
		 * Depending on the FF-A version of the requesting partition
		 * we may need to convert to a v1.0 format otherwise we can copy
		 * directly.
		 */
		if (ffa_version == MAKE_FFA_VERSION(U(1), U(0))) {
			ret = partition_info_populate_v1_0(partitions,
							   mbox,
							   partition_count);
			if (ret != 0) {
				goto err_unlock;
			}
		} else {
			uint32_t buf_size = mbox->rxtx_page_count *
					    FFA_PAGE_SIZE;

			/* Ensure the descriptor will fit in the buffer. */
			size = sizeof(struct ffa_partition_info_v1_1);
			if (partition_count * size  > buf_size) {
				ret = FFA_ERROR_NO_MEMORY;
				goto err_unlock;
			}
			memcpy(mbox->rx_buffer, partitions,
			       partition_count * size);
		}

		mbox->state = MAILBOX_STATE_FULL;
		spin_unlock(&mbox->lock);
	}
	SMC_RET4(handle, FFA_SUCCESS_SMC32, 0, partition_count, size);

err_unlock:
	spin_unlock(&mbox->lock);
err:
	return spmc_ffa_error_return(handle, ret);
}

static uint64_t ffa_feature_success(void *handle, uint32_t arg2)
{
	SMC_RET3(handle, FFA_SUCCESS_SMC32, 0, arg2);
}

static uint64_t ffa_features_retrieve_request(bool secure_origin,
					      uint32_t input_properties,
					      void *handle)
{
	/*
	 * If we're called by the normal world we don't support any
	 * additional features.
	 */
	if (!secure_origin) {
		if ((input_properties & FFA_FEATURES_RET_REQ_NS_BIT) != 0U) {
			return spmc_ffa_error_return(handle,
						     FFA_ERROR_NOT_SUPPORTED);
		}

	} else {
		struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
		/*
		 * If v1.1 or higher the NS bit must be set otherwise it is
		 * an invalid call. If v1.0 check and store whether the SP
		 * has requested the use of the NS bit.
		 */
		if (spmc_compatible_version(sp->ffa_version, 1, 1)) {
			if ((input_properties &
			     FFA_FEATURES_RET_REQ_NS_BIT) == 0U) {
				return spmc_ffa_error_return(handle,
						       FFA_ERROR_NOT_SUPPORTED);
			}
			return ffa_feature_success(handle,
						   FFA_FEATURES_RET_REQ_NS_BIT);
		} else {
			sp->ns_bit_requested = (input_properties &
					       FFA_FEATURES_RET_REQ_NS_BIT) !=
					       0U;
		}
		if (sp->ns_bit_requested) {
			return ffa_feature_success(handle,
						   FFA_FEATURES_RET_REQ_NS_BIT);
		}
	}
	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

static uint64_t ffa_features_handler(uint32_t smc_fid,
				     bool secure_origin,
				     uint64_t x1,
				     uint64_t x2,
				     uint64_t x3,
				     uint64_t x4,
				     void *cookie,
				     void *handle,
				     uint64_t flags)
{
	uint32_t function_id = (uint32_t) x1;
	uint32_t input_properties = (uint32_t) x2;

	/* Check if a Feature ID was requested. */
	if ((function_id & FFA_FEATURES_BIT31_MASK) == 0U) {
		/* We currently don't support any additional features. */
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/*
	 * Handle the cases where we have separate handlers due to additional
	 * properties.
	 */
	switch (function_id) {
	case FFA_MEM_RETRIEVE_REQ_SMC32:
	case FFA_MEM_RETRIEVE_REQ_SMC64:
		return ffa_features_retrieve_request(secure_origin,
						     input_properties,
						     handle);
	}

	/*
	 * We don't currently support additional input properties for these
	 * other ABIs therefore ensure this value is set to 0.
	 */
	if (input_properties != 0U) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_NOT_SUPPORTED);
	}

	/* Report if any other FF-A ABI is supported. */
	switch (function_id) {
	/* Supported features from both worlds. */
	case FFA_ERROR:
	case FFA_SUCCESS_SMC32:
	case FFA_INTERRUPT:
	case FFA_SPM_ID_GET:
	case FFA_ID_GET:
	case FFA_FEATURES:
	case FFA_VERSION:
	case FFA_RX_RELEASE:
	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
	case FFA_MSG_SEND_DIRECT_REQ2_SMC64:
	case FFA_PARTITION_INFO_GET:
	case FFA_RXTX_MAP_SMC32:
	case FFA_RXTX_MAP_SMC64:
	case FFA_RXTX_UNMAP:
	case FFA_MEM_FRAG_TX:
	case FFA_MSG_RUN:

		/*
		 * We are relying on the fact that the other registers
		 * will be set to 0 as these values align with the
		 * currently implemented features of the SPMC. If this
		 * changes this function must be extended to handle
		 * reporting the additional functionality.
		 */

		SMC_RET1(handle, FFA_SUCCESS_SMC32);
		/* Execution stops here. */

	/* Supported ABIs only from the secure world. */
	case FFA_MEM_PERM_GET_SMC32:
	case FFA_MEM_PERM_GET_SMC64:
	case FFA_MEM_PERM_SET_SMC32:
	case FFA_MEM_PERM_SET_SMC64:
	/* these ABIs are only supported from S-EL0 SPs */
	#if !(SPMC_AT_EL3_SEL0_SP)
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	#endif
	/* fall through */

	case FFA_SECONDARY_EP_REGISTER_SMC64:
	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
	case FFA_MSG_SEND_DIRECT_RESP2_SMC64:
	case FFA_MEM_RELINQUISH:
	case FFA_MSG_WAIT:
	case FFA_CONSOLE_LOG_SMC32:
	case FFA_CONSOLE_LOG_SMC64:
		if (!secure_origin) {
			return spmc_ffa_error_return(handle,
				FFA_ERROR_NOT_SUPPORTED);
		}
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
		/* Execution stops here. */

	/* Supported features only from the normal world. */
	case FFA_MEM_SHARE_SMC32:
	case FFA_MEM_SHARE_SMC64:
	case FFA_MEM_LEND_SMC32:
	case FFA_MEM_LEND_SMC64:
	case FFA_MEM_RECLAIM:
	case FFA_MEM_FRAG_RX:

		if (secure_origin) {
			return spmc_ffa_error_return(handle,
					FFA_ERROR_NOT_SUPPORTED);
		}
		SMC_RET1(handle, FFA_SUCCESS_SMC32);
		/* Execution stops here. */

	default:
		return spmc_ffa_error_return(handle,
					FFA_ERROR_NOT_SUPPORTED);
	}
}

static uint64_t ffa_id_get_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	if (secure_origin) {
		SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0,
			 spmc_get_current_sp_ctx()->sp_id);
	} else {
		SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0,
			 spmc_get_hyp_ctx()->ns_ep_id);
	}
}

/*
 * Enable an SP to query the ID assigned to the SPMC.
 */
static uint64_t ffa_spm_id_get_handler(uint32_t smc_fid,
				       bool secure_origin,
				       uint64_t x1,
				       uint64_t x2,
				       uint64_t x3,
				       uint64_t x4,
				       void *cookie,
				       void *handle,
				       uint64_t flags)
{
	assert(x1 == 0UL);
	assert(x2 == 0UL);
	assert(x3 == 0UL);
	assert(x4 == 0UL);
	assert(SMC_GET_GP(handle, CTX_GPREG_X5) == 0UL);
	assert(SMC_GET_GP(handle, CTX_GPREG_X6) == 0UL);
	assert(SMC_GET_GP(handle, CTX_GPREG_X7) == 0UL);

	SMC_RET3(handle, FFA_SUCCESS_SMC32, 0x0, FFA_SPMC_ID);
}

static uint64_t ffa_run_handler(uint32_t smc_fid,
				bool secure_origin,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *cookie,
				void *handle,
				uint64_t flags)
{
	struct secure_partition_desc *sp;
	uint16_t target_id = FFA_RUN_EP_ID(x1);
	uint16_t vcpu_id = FFA_RUN_VCPU_ID(x1);
	unsigned int idx;
	unsigned int *rt_state;
	unsigned int *rt_model;

	/* Can only be called from the normal world. */
	if (secure_origin) {
		ERROR("FFA_RUN can only be called from NWd.\n");
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Cannot run a Normal world partition. */
	if (ffa_is_normal_world_id(target_id)) {
		ERROR("Cannot run a NWd partition (0x%x).\n", target_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Check that the target SP exists. */
	sp = spmc_get_sp_ctx(target_id);
	if (sp == NULL) {
		ERROR("Unknown partition ID (0x%x).\n", target_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	idx = get_ec_index(sp);

	if (idx != vcpu_id) {
		ERROR("Cannot run vcpu %d != %d.\n", idx, vcpu_id);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}
	if (sp->runtime_el == S_EL0) {
		spin_lock(&sp->rt_state_lock);
	}
	rt_state = &((sp->ec[idx]).rt_state);
	rt_model = &((sp->ec[idx]).rt_model);
	if (*rt_state == RT_STATE_RUNNING) {
		if (sp->runtime_el == S_EL0) {
			spin_unlock(&sp->rt_state_lock);
		}
		ERROR("Partition (0x%x) is already running.\n", target_id);
		return spmc_ffa_error_return(handle, FFA_ERROR_BUSY);
	}

	/*
	 * Sanity check that if the execution context was not waiting then it
	 * was either in the direct request or the run partition runtime model.
	 */
	if (*rt_state == RT_STATE_PREEMPTED || *rt_state == RT_STATE_BLOCKED) {
		assert(*rt_model == RT_MODEL_RUN ||
		       *rt_model == RT_MODEL_DIR_REQ);
	}

	/*
	 * If the context was waiting then update the partition runtime model.
	 */
	if (*rt_state == RT_STATE_WAITING) {
		*rt_model = RT_MODEL_RUN;
	}

	/*
	 * Forward the request to the correct SP vCPU after updating
	 * its state.
	 */
	*rt_state = RT_STATE_RUNNING;

	if (sp->runtime_el == S_EL0) {
		spin_unlock(&sp->rt_state_lock);
	}

	return spmc_smc_return(smc_fid, secure_origin, x1, 0, 0, 0,
			       handle, cookie, flags, target_id, sp->ffa_version);
}

static uint64_t rx_release_handler(uint32_t smc_fid,
				   bool secure_origin,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);

	spin_lock(&mbox->lock);

	if (mbox->state != MAILBOX_STATE_FULL) {
		spin_unlock(&mbox->lock);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	mbox->state = MAILBOX_STATE_EMPTY;
	spin_unlock(&mbox->lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

static uint64_t spmc_ffa_console_log(uint32_t smc_fid,
				     bool secure_origin,
				     uint64_t x1,
				     uint64_t x2,
				     uint64_t x3,
				     uint64_t x4,
				     void *cookie,
				     void *handle,
				     uint64_t flags)
{
	/* Maximum number of characters is 48: 6 registers of 8 bytes each. */
	char chars[48] = {0};
	size_t chars_max;
	size_t chars_count = x1;

	/* Does not support request from Nwd. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	assert(smc_fid == FFA_CONSOLE_LOG_SMC32 || smc_fid == FFA_CONSOLE_LOG_SMC64);
	if (smc_fid == FFA_CONSOLE_LOG_SMC32) {
		uint32_t *registers = (uint32_t *)chars;
		registers[0] = (uint32_t)x2;
		registers[1] = (uint32_t)x3;
		registers[2] = (uint32_t)x4;
		registers[3] = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X5);
		registers[4] = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X6);
		registers[5] = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X7);
		chars_max = 6 * sizeof(uint32_t);
	} else {
		uint64_t *registers = (uint64_t *)chars;
		registers[0] = x2;
		registers[1] = x3;
		registers[2] = x4;
		registers[3] = SMC_GET_GP(handle, CTX_GPREG_X5);
		registers[4] = SMC_GET_GP(handle, CTX_GPREG_X6);
		registers[5] = SMC_GET_GP(handle, CTX_GPREG_X7);
		chars_max = 6 * sizeof(uint64_t);
	}

	if ((chars_count == 0) || (chars_count > chars_max)) {
		return spmc_ffa_error_return(handle, FFA_ERROR_INVALID_PARAMETER);
	}

	for (size_t i = 0; (i < chars_count) && (chars[i] != '\0'); i++) {
		putchar(chars[i]);
	}

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*
 * Perform initial validation on the provided secondary entry point.
 * For now ensure it does not lie within the BL31 Image or the SP's
 * RX/TX buffers as these are mapped within EL3.
 * TODO: perform validation for additional invalid memory regions.
 */
static int validate_secondary_ep(uintptr_t ep, struct secure_partition_desc *sp)
{
	struct mailbox *mb;
	uintptr_t buffer_size;
	uintptr_t sp_rx_buffer;
	uintptr_t sp_tx_buffer;
	uintptr_t sp_rx_buffer_limit;
	uintptr_t sp_tx_buffer_limit;

	mb = &sp->mailbox;
	buffer_size = (uintptr_t) (mb->rxtx_page_count * FFA_PAGE_SIZE);
	sp_rx_buffer = (uintptr_t) mb->rx_buffer;
	sp_tx_buffer = (uintptr_t) mb->tx_buffer;
	sp_rx_buffer_limit = sp_rx_buffer + buffer_size;
	sp_tx_buffer_limit = sp_tx_buffer + buffer_size;

	/*
	 * Check if the entry point lies within BL31, or the
	 * SP's RX or TX buffer.
	 */
	if ((ep >= BL31_BASE && ep < BL31_LIMIT) ||
	    (ep >= sp_rx_buffer && ep < sp_rx_buffer_limit) ||
	    (ep >= sp_tx_buffer && ep < sp_tx_buffer_limit)) {
		return -EINVAL;
	}
	return 0;
}

/*******************************************************************************
 * This function handles the FFA_SECONDARY_EP_REGISTER SMC to allow an SP to
 *  register an entry point for initialization during a secondary cold boot.
 ******************************************************************************/
static uint64_t ffa_sec_ep_register_handler(uint32_t smc_fid,
					    bool secure_origin,
					    uint64_t x1,
					    uint64_t x2,
					    uint64_t x3,
					    uint64_t x4,
					    void *cookie,
					    void *handle,
					    uint64_t flags)
{
	struct secure_partition_desc *sp;
	struct sp_exec_ctx *sp_ctx;

	/* This request cannot originate from the Normal world. */
	if (!secure_origin) {
		WARN("%s: Can only be called from SWd.\n", __func__);
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the context of the current SP. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		WARN("%s: Cannot find SP context.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Only an S-EL1 SP should be invoking this ABI. */
	if (sp->runtime_el != S_EL1) {
		WARN("%s: Can only be called for a S-EL1 SP.\n", __func__);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Ensure the SP is in its initialization state. */
	sp_ctx = spmc_get_sp_ec(sp);
	if (sp_ctx->rt_model != RT_MODEL_INIT) {
		WARN("%s: Can only be called during SP initialization.\n",
		     __func__);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Perform initial validation of the secondary entry point. */
	if (validate_secondary_ep(x1, sp)) {
		WARN("%s: Invalid entry point provided (0x%lx).\n",
		     __func__, x1);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Update the secondary entrypoint in SP context.
	 * We don't need a lock here as during partition initialization there
	 * will only be a single core online.
	 */
	sp->secondary_ep = x1;
	VERBOSE("%s: 0x%lx\n", __func__, sp->secondary_ep);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*******************************************************************************
 * Permissions are encoded using a different format in the FFA_MEM_PERM_* ABIs
 * than in the Trusted Firmware, where the mmap_attr_t enum type is used. This
 * function converts a permission value from the FF-A format to the mmap_attr_t
 * format by setting MT_RW/MT_RO, MT_USER/MT_PRIVILEGED and
 * MT_EXECUTE/MT_EXECUTE_NEVER. The other fields are left as 0 because they are
 * ignored by the function xlat_change_mem_attributes_ctx().
 ******************************************************************************/
static unsigned int ffa_perm_to_mmap_perm(unsigned int perms)
{
	unsigned int tf_attr = 0U;
	unsigned int access;

	/* Deal with data access permissions first. */
	access = (perms & FFA_MEM_PERM_DATA_MASK) >> FFA_MEM_PERM_DATA_SHIFT;

	switch (access) {
	case FFA_MEM_PERM_DATA_RW:
		/* Return 0 if the execute is set with RW. */
		if ((perms & FFA_MEM_PERM_INST_NON_EXEC) != 0) {
			tf_attr |= MT_RW | MT_USER | MT_EXECUTE_NEVER;
		}
		break;

	case FFA_MEM_PERM_DATA_RO:
		tf_attr |= MT_RO | MT_USER;
		/* Deal with the instruction access permissions next. */
		if ((perms & FFA_MEM_PERM_INST_NON_EXEC) == 0) {
			tf_attr |= MT_EXECUTE;
		} else {
			tf_attr |= MT_EXECUTE_NEVER;
		}
		break;

	case FFA_MEM_PERM_DATA_NA:
	default:
		return tf_attr;
	}

	return tf_attr;
}

/*******************************************************************************
 * Handler to set the permissions of a set of contiguous pages of a S-EL0 SP
 ******************************************************************************/
static uint64_t ffa_mem_perm_set_handler(uint32_t smc_fid,
					 bool secure_origin,
					 uint64_t x1,
					 uint64_t x2,
					 uint64_t x3,
					 uint64_t x4,
					 void *cookie,
					 void *handle,
					 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;
	uintptr_t base_va = (uintptr_t) x1;
	size_t size = (size_t)(x2 * PAGE_SIZE);
	uint32_t tf_attr;
	int ret;

	/* This request cannot originate from the Normal world. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	if (size == 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Get the context of the current SP. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* A S-EL1 SP has no business invoking this ABI. */
	if (sp->runtime_el == S_EL1) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	if ((x3 & ~((uint64_t)FFA_MEM_PERM_MASK)) != 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Get the execution context of the calling SP. */
	idx = get_ec_index(sp);

	/*
	 * Ensure that the S-EL0 SP is initialising itself. We do not need to
	 * synchronise this operation through a spinlock since a S-EL0 SP is UP
	 * and can only be initialising on this cpu.
	 */
	if (sp->ec[idx].rt_model != RT_MODEL_INIT) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	VERBOSE("Setting memory permissions:\n");
	VERBOSE("  Start address  : 0x%lx\n", base_va);
	VERBOSE("  Number of pages: %lu (%zu bytes)\n", x2, size);
	VERBOSE("  Attributes     : 0x%x\n", (uint32_t)x3);

	/* Convert inbound permissions to TF-A permission attributes */
	tf_attr = ffa_perm_to_mmap_perm((unsigned int)x3);
	if (tf_attr == 0U) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* Request the change in permissions */
	ret = xlat_change_mem_attributes_ctx(sp->xlat_ctx_handle,
					     base_va, size, tf_attr);
	if (ret != 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	SMC_RET1(handle, FFA_SUCCESS_SMC32);
}

/*******************************************************************************
 * Permissions are encoded using a different format in the FFA_MEM_PERM_* ABIs
 * than in the Trusted Firmware, where the mmap_attr_t enum type is used. This
 * function converts a permission value from the mmap_attr_t format to the FF-A
 * format.
 ******************************************************************************/
static unsigned int mmap_perm_to_ffa_perm(unsigned int attr)
{
	unsigned int perms = 0U;
	unsigned int data_access;

	if ((attr & MT_USER) == 0) {
		/* No access from EL0. */
		data_access = FFA_MEM_PERM_DATA_NA;
	} else {
		if ((attr & MT_RW) != 0) {
			data_access = FFA_MEM_PERM_DATA_RW;
		} else {
			data_access = FFA_MEM_PERM_DATA_RO;
		}
	}

	perms |= (data_access & FFA_MEM_PERM_DATA_MASK)
		<< FFA_MEM_PERM_DATA_SHIFT;

	if ((attr & MT_EXECUTE_NEVER) != 0U) {
		perms |= FFA_MEM_PERM_INST_NON_EXEC;
	}

	return perms;
}

/*******************************************************************************
 * Handler to get the permissions of a set of contiguous pages of a S-EL0 SP
 ******************************************************************************/
static uint64_t ffa_mem_perm_get_handler(uint32_t smc_fid,
					 bool secure_origin,
					 uint64_t x1,
					 uint64_t x2,
					 uint64_t x3,
					 uint64_t x4,
					 void *cookie,
					 void *handle,
					 uint64_t flags)
{
	struct secure_partition_desc *sp;
	unsigned int idx;
	uintptr_t base_va = (uintptr_t)x1;
	uint64_t max_page_count = x2 + 1;
	uint64_t page_count = 0;
	uint32_t base_page_attr = 0;
	uint32_t page_attr = 0;
	unsigned int table_level;
	int ret;

	/* This request cannot originate from the Normal world. */
	if (!secure_origin) {
		return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}

	/* Get the context of the current SP. */
	sp = spmc_get_current_sp_ctx();
	if (sp == NULL) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/* A S-EL1 SP has no business invoking this ABI. */
	if (sp->runtime_el == S_EL1) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Get the execution context of the calling SP. */
	idx = get_ec_index(sp);

	/*
	 * Ensure that the S-EL0 SP is initialising itself. We do not need to
	 * synchronise this operation through a spinlock since a S-EL0 SP is UP
	 * and can only be initialising on this cpu.
	 */
	if (sp->ec[idx].rt_model != RT_MODEL_INIT) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	base_va &= ~(PAGE_SIZE_MASK);

	/* Request the permissions */
	ret = xlat_get_mem_attributes_ctx(sp->xlat_ctx_handle, base_va,
			&base_page_attr, &table_level);
	if (ret != 0) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	/*
	 * Caculate how many pages in this block entry from base_va including
	 * its page.
	 */
	page_count = ((XLAT_BLOCK_SIZE(table_level) -
			(base_va & XLAT_BLOCK_MASK(table_level))) >> PAGE_SIZE_SHIFT);
	base_va += XLAT_BLOCK_SIZE(table_level);

	while ((page_count < max_page_count) && (base_va != 0x00)) {
		ret = xlat_get_mem_attributes_ctx(sp->xlat_ctx_handle, base_va,
				&page_attr, &table_level);
		if (ret != 0) {
			return spmc_ffa_error_return(handle,
						     FFA_ERROR_INVALID_PARAMETER);
		}

		if (page_attr != base_page_attr) {
			break;
		}

		base_va += XLAT_BLOCK_SIZE(table_level);
		page_count += (XLAT_BLOCK_SIZE(table_level) >> PAGE_SIZE_SHIFT);
	}

	if (page_count > max_page_count) {
		page_count = max_page_count;
	}

	/* Convert TF-A permission to FF-A permissions attributes. */
	x2 = mmap_perm_to_ffa_perm(base_page_attr);

	/* x3 should be page count - 1 */
	SMC_RET4(handle, FFA_SUCCESS_SMC32, 0, x2, --page_count);
}

/*******************************************************************************
 * This function will parse the Secure Partition Manifest. From manifest, it
 * will fetch details for preparing Secure partition image context and secure
 * partition image boot arguments if any.
 ******************************************************************************/
static int sp_manifest_parse(void *sp_manifest, int offset,
			     struct secure_partition_desc *sp,
			     entry_point_info_t *ep_info,
			     int32_t *boot_info_reg)
{
	int32_t ret, node;
	uint32_t config_32;
	int uuid_size;
	const fdt32_t *prop;

	/*
	 * Look for the mandatory fields that are expected to be present in
	 * the SP manifests.
	 */
	node = fdt_path_offset(sp_manifest, "/");
	if (node < 0) {
		ERROR("Did not find root node.\n");
		return node;
	}

	prop = fdt_getprop(sp_manifest, node, "uuid", &uuid_size);
	if (prop == NULL) {
		ERROR("Couldn't find property uuid in manifest\n");
		return -FDT_ERR_NOTFOUND;
	}

	sp->num_uuids = (uint32_t)uuid_size / sizeof(struct ffa_uuid);
	if (sp->num_uuids > ARRAY_SIZE(sp->uuid_array)) {
		ERROR("Too many UUIDs (%d) in manifest, maximum is %zd\n",
		      sp->num_uuids, ARRAY_SIZE(sp->uuid_array));
		return -FDT_ERR_BADVALUE;
	}

	ret = fdt_read_uint32_array(sp_manifest, node, "uuid",
				    (uuid_size / sizeof(uint32_t)),
				    sp->uuid_array[0].uuid);
	if (ret != 0) {
		ERROR("Missing Secure Partition UUID.\n");
		return ret;
	}

	for (uint32_t i = 0; i < sp->num_uuids; i++) {
		for (uint32_t j = 0; j < i; j++) {
			if (memcmp(&sp->uuid_array[i], &sp->uuid_array[j],
				   sizeof(struct ffa_uuid)) == 0) {
				ERROR("Duplicate UUIDs in manifest: 0x%x 0x%x 0x%x 0x%x\n",
				      sp->uuid_array[i].uuid[0],
				      sp->uuid_array[i].uuid[1],
				      sp->uuid_array[i].uuid[2],
				      sp->uuid_array[i].uuid[3]);
				return -FDT_ERR_BADVALUE;
			}
		}
	}

	ret = fdt_read_uint32(sp_manifest, node, "exception-level", &config_32);
	if (ret != 0) {
		ERROR("Missing SP Exception Level information.\n");
		return ret;
	}

	sp->runtime_el = config_32;

	ret = fdt_read_uint32(sp_manifest, node, "ffa-version", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition FF-A Version.\n");
		return ret;
	}

	sp->ffa_version = config_32;

	ret = fdt_read_uint32(sp_manifest, node, "execution-state", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition Execution State.\n");
		return ret;
	}

	sp->execution_state = config_32;

	ret = fdt_read_uint32(sp_manifest, node,
			      "messaging-method", &config_32);
	if (ret != 0) {
		ERROR("Missing Secure Partition messaging method.\n");
		return ret;
	}

	/* Validate this entry, we currently only support direct messaging. */
	if ((config_32 & ~(FFA_PARTITION_DIRECT_REQ_RECV |
			  FFA_PARTITION_DIRECT_REQ_SEND |
			  FFA_PARTITION_DIRECT_REQ2_RECV |
			  FFA_PARTITION_DIRECT_REQ2_SEND)) != 0U) {
		WARN("Invalid Secure Partition messaging method (0x%x)\n",
		     config_32);
		return -EINVAL;
	}

	sp->properties = config_32;

	ret = fdt_read_uint32(sp_manifest, node,
			      "execution-ctx-count", &config_32);

	if (ret != 0) {
		ERROR("Missing SP Execution Context Count.\n");
		return ret;
	}

	/*
	 * Ensure this field is set correctly in the manifest however
	 * since this is currently a hardcoded value for S-EL1 partitions
	 * we don't need to save it here, just validate.
	 */
	if ((sp->runtime_el == S_EL1) && (config_32 != PLATFORM_CORE_COUNT)) {
		ERROR("SP Execution Context Count (%u) must be %u.\n",
			config_32, PLATFORM_CORE_COUNT);
		return -EINVAL;
	}

	/*
	 * Look for the optional fields that are expected to be present in
	 * an SP manifest.
	 */
	ret = fdt_read_uint32(sp_manifest, node, "id", &config_32);
	if (ret != 0) {
		WARN("Missing Secure Partition ID.\n");
	} else {
		if (!is_ffa_secure_id_valid(config_32)) {
			ERROR("Invalid Secure Partition ID (0x%x).\n",
			      config_32);
			return -EINVAL;
		}
		sp->sp_id = config_32;
	}

	ret = fdt_read_uint32(sp_manifest, node,
			      "power-management-messages", &config_32);
	if (ret != 0) {
		WARN("Missing Power Management Messages entry.\n");
	} else {
		if ((sp->runtime_el == S_EL0) && (config_32 != 0)) {
			ERROR("Power messages not supported for S-EL0 SP\n");
			return -EINVAL;
		}

		/*
		 * Ensure only the currently supported power messages have
		 * been requested.
		 */
		if (config_32 & ~(FFA_PM_MSG_SUB_CPU_OFF |
				  FFA_PM_MSG_SUB_CPU_SUSPEND |
				  FFA_PM_MSG_SUB_CPU_SUSPEND_RESUME)) {
			ERROR("Requested unsupported PM messages (%x)\n",
			      config_32);
			return -EINVAL;
		}
		sp->pwr_mgmt_msgs = config_32;
	}

	ret = fdt_read_uint32(sp_manifest, node,
			      "gp-register-num", &config_32);
	if (ret != 0) {
		WARN("Missing boot information register.\n");
	} else {
		/* Check if a register number between 0-3 is specified. */
		if (config_32 < 4) {
			*boot_info_reg = config_32;
		} else {
			WARN("Incorrect boot information register (%u).\n",
			     config_32);
		}
	}

	ret = fdt_read_uint32(sp_manifest, node,
			      "vm-availability-messages", &config_32);
	if (ret != 0) {
		WARN("Missing VM availability messaging.\n");
	} else if ((sp->properties & FFA_PARTITION_DIRECT_REQ_RECV) == 0) {
		ERROR("VM availability messaging requested without "
		      "direct message receive support.\n");
		return -EINVAL;
	} else {
		/* Validate this entry. */
		if ((config_32 & ~(FFA_VM_AVAILABILITY_CREATED |
				  FFA_VM_AVAILABILITY_DESTROYED)) != 0U) {
			WARN("Invalid VM availability messaging (0x%x)\n",
			     config_32);
			return -EINVAL;
		}

		if ((config_32 & FFA_VM_AVAILABILITY_CREATED) != 0U) {
			sp->properties |= FFA_PARTITION_VM_CREATED;
		}
		if ((config_32 & FFA_VM_AVAILABILITY_DESTROYED) != 0U) {
			sp->properties |= FFA_PARTITION_VM_DESTROYED;
		}
	}

	return 0;
}

/*******************************************************************************
 * This function gets the Secure Partition Manifest base and maps the manifest
 * region.
 * Currently only one Secure Partition manifest is considered which is used to
 * prepare the context for the single Secure Partition.
 ******************************************************************************/
static int find_and_prepare_sp_context(void)
{
	void *sp_manifest;
	uintptr_t manifest_base;
	uintptr_t manifest_base_align __maybe_unused;
	entry_point_info_t *next_image_ep_info;
	int32_t ret, boot_info_reg = -1;
	struct secure_partition_desc *sp;
	struct transfer_list_header *tl __maybe_unused;
	struct transfer_list_entry *te __maybe_unused;

	next_image_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (next_image_ep_info == NULL) {
		WARN("No Secure Partition image provided by BL2.\n");
		return -ENOENT;
	}


#if TRANSFER_LIST && !RESET_TO_BL31
	tl = (struct transfer_list_header *)next_image_ep_info->args.arg3;
	te = transfer_list_find(tl, TL_TAG_DT_FFA_MANIFEST);
	if (te == NULL) {
		WARN("Secure Partition manifest absent.\n");
		return -ENOENT;
	}

	sp_manifest = (void *)transfer_list_entry_data(te);
	manifest_base = (uintptr_t)sp_manifest;
#else
	sp_manifest = (void *)next_image_ep_info->args.arg0;
	if (sp_manifest == NULL) {
		WARN("Secure Partition manifest absent.\n");
		return -ENOENT;
	}

	manifest_base = (uintptr_t)sp_manifest;
	manifest_base_align = page_align(manifest_base, DOWN);

	/*
	 * Map the secure partition manifest region in the EL3 translation
	 * regime.
	 * Map an area equal to (2 * PAGE_SIZE) for now. During manifest base
	 * alignment the region of 1 PAGE_SIZE from manifest align base may
	 * not completely accommodate the secure partition manifest region.
	 */
	ret = mmap_add_dynamic_region((unsigned long long)manifest_base_align,
				      manifest_base_align,
				      PAGE_SIZE * 2,
				      MT_RO_DATA);
	if (ret != 0) {
		ERROR("Error while mapping SP manifest (%d).\n", ret);
		return ret;
	}
#endif

	ret = fdt_node_offset_by_compatible(sp_manifest, -1,
					    "arm,ffa-manifest-1.0");
	if (ret < 0) {
		ERROR("Error happened in SP manifest reading.\n");
		return -EINVAL;
	}

	/*
	 * Store the size of the manifest so that it can be used later to pass
	 * the manifest as boot information later.
	 */
	next_image_ep_info->args.arg1 = fdt_totalsize(sp_manifest);
	INFO("Manifest adr = %lx , size = %lu bytes\n", manifest_base,
	     next_image_ep_info->args.arg1);

	/*
	 * Select an SP descriptor for initialising the partition's execution
	 * context on the primary CPU.
	 */
	sp = spmc_get_current_sp_ctx();

#if SPMC_AT_EL3_SEL0_SP
	/* Assign translation tables context. */
	sp_desc->xlat_ctx_handle = spm_get_sp_xlat_context();

#endif /* SPMC_AT_EL3_SEL0_SP */
	/* Initialize entry point information for the SP */
	SET_PARAM_HEAD(next_image_ep_info, PARAM_EP, VERSION_1,
		       SECURE | EP_ST_ENABLE);

	/* Parse the SP manifest. */
	ret = sp_manifest_parse(sp_manifest, ret, sp, next_image_ep_info,
				&boot_info_reg);
	if (ret != 0) {
		ERROR("Error in Secure Partition manifest parsing.\n");
		return ret;
	}

	/* Perform any common initialisation. */
	spmc_sp_common_setup(sp, next_image_ep_info, boot_info_reg);

	/* Perform any initialisation specific to S-EL1 SPs. */
	if (sp->runtime_el == S_EL1) {
		spmc_el1_sp_setup(sp, next_image_ep_info);
		spmc_sp_common_ep_commit(sp, next_image_ep_info);
	}
#if SPMC_AT_EL3_SEL0_SP
	/* Perform any initialisation specific to S-EL0 SPs. */
	else if (sp->runtime_el == S_EL0) {
		/* Setup spsr in endpoint info for common context management routine. */
		spmc_el0_sp_spsr_setup(next_image_ep_info);

		spmc_sp_common_ep_commit(sp, next_image_ep_info);

		/*
		 * Perform any initialisation specific to S-EL0 not set by common
		 * context management routine.
		 */
		spmc_el0_sp_setup(sp, boot_info_reg, sp_manifest);
	}
#endif /* SPMC_AT_EL3_SEL0_SP */
	else {
		ERROR("Unexpected runtime EL: %u\n", sp->runtime_el);
		return -EINVAL;
	}

	return 0;
}

/*******************************************************************************
 * This function takes an SP context pointer and performs a synchronous entry
 * into it.
 ******************************************************************************/
static int32_t logical_sp_init(void)
{
	int32_t rc = 0;
	struct el3_lp_desc *el3_lp_descs;

	/* Perform initial validation of the Logical Partitions. */
	rc = el3_sp_desc_validate();
	if (rc != 0) {
		ERROR("Logical Partition validation failed!\n");
		return rc;
	}

	el3_lp_descs = get_el3_lp_array();

	INFO("Logical Secure Partition init start.\n");
	for (unsigned int i = 0U; i < EL3_LP_DESCS_COUNT; i++) {
		rc = el3_lp_descs[i].init();
		if (rc != 0) {
			ERROR("Logical SP (0x%x) Failed to Initialize\n",
			      el3_lp_descs[i].sp_id);
			return rc;
		}
		VERBOSE("Logical SP (0x%x) Initialized\n",
			      el3_lp_descs[i].sp_id);
	}

	INFO("Logical Secure Partition init completed.\n");

	return rc;
}

uint64_t spmc_sp_synchronous_entry(struct sp_exec_ctx *ec)
{
	uint64_t rc;

	assert(ec != NULL);

	/* Assign the context of the SP to this CPU */
	cm_set_context(&(ec->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();

	/* Enter Secure Partition */
	rc = spm_secure_partition_enter(&ec->c_rt_ctx);

	/* Save secure state */
	cm_el1_sysregs_context_save(SECURE);

	return rc;
}

/*******************************************************************************
 * SPMC Helper Functions.
 ******************************************************************************/
static int32_t sp_init(void)
{
	uint64_t rc;
	struct secure_partition_desc *sp;
	struct sp_exec_ctx *ec;

	sp = spmc_get_current_sp_ctx();
	ec = spmc_get_sp_ec(sp);
	ec->rt_model = RT_MODEL_INIT;
	ec->rt_state = RT_STATE_RUNNING;

	INFO("Secure Partition (0x%x) init start.\n", sp->sp_id);

	rc = spmc_sp_synchronous_entry(ec);
	if (rc != 0) {
		/* Indicate SP init was not successful. */
		ERROR("SP (0x%x) failed to initialize (%lu).\n",
		      sp->sp_id, rc);
		return 0;
	}

	ec->rt_state = RT_STATE_WAITING;
	INFO("Secure Partition initialized.\n");

	return 1;
}

static void initalize_sp_descs(void)
{
	struct secure_partition_desc *sp;

	for (unsigned int i = 0U; i < SECURE_PARTITION_COUNT; i++) {
		sp = &sp_desc[i];
		sp->sp_id = INV_SP_ID;
		sp->mailbox.rx_buffer = NULL;
		sp->mailbox.tx_buffer = NULL;
		sp->mailbox.state = MAILBOX_STATE_EMPTY;
		sp->secondary_ep = 0;
	}
}

static void initalize_ns_ep_descs(void)
{
	struct ns_endpoint_desc *ns_ep;

	for (unsigned int i = 0U; i < NS_PARTITION_COUNT; i++) {
		ns_ep = &ns_ep_desc[i];
		/*
		 * Clashes with the Hypervisor ID but will not be a
		 * problem in practice.
		 */
		ns_ep->ns_ep_id = 0;
		ns_ep->ffa_version = 0;
		ns_ep->mailbox.rx_buffer = NULL;
		ns_ep->mailbox.tx_buffer = NULL;
		ns_ep->mailbox.state = MAILBOX_STATE_EMPTY;
	}
}

/*******************************************************************************
 * Initialize SPMC attributes for the SPMD.
 ******************************************************************************/
void spmc_populate_attrs(spmc_manifest_attribute_t *spmc_attrs)
{
	spmc_attrs->major_version = FFA_VERSION_SPMC_MAJOR;
	spmc_attrs->minor_version = FFA_VERSION_SPMC_MINOR;
	spmc_attrs->exec_state = MODE_RW_64;
	spmc_attrs->spmc_id = FFA_SPMC_ID;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spmc_setup(void)
{
	int32_t ret;
	uint32_t flags;

	/* Initialize endpoint descriptors */
	initalize_sp_descs();
	initalize_ns_ep_descs();

	/*
	 * Retrieve the information of the datastore for tracking shared memory
	 * requests allocated by platform code and zero the region if available.
	 */
	ret = plat_spmc_shmem_datastore_get(&spmc_shmem_obj_state.data,
					    &spmc_shmem_obj_state.data_size);
	if (ret != 0) {
		ERROR("Failed to obtain memory descriptor backing store!\n");
		return ret;
	}
	memset(spmc_shmem_obj_state.data, 0, spmc_shmem_obj_state.data_size);

	/* Setup logical SPs. */
	ret = logical_sp_init();
	if (ret != 0) {
		ERROR("Failed to initialize Logical Partitions.\n");
		return ret;
	}

	/* Perform physical SP setup. */

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start.\n");

	ret = find_and_prepare_sp_context();
	if (ret != 0) {
		ERROR("Error in SP finding and context preparation.\n");
		return ret;
	}

	/* Register power management hooks with PSCI */
	psci_register_spd_pm_hook(&spmc_pm);

	/*
	 * Register an interrupt handler for S-EL1 interrupts
	 * when generated during code executing in the
	 * non-secure state.
	 */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	ret = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					      spmc_sp_interrupt_handler,
					      flags);
	if (ret != 0) {
		ERROR("Failed to register interrupt handler! (%d)\n", ret);
		panic();
	}

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&sp_init);

	INFO("Secure Partition setup done.\n");

	return 0;
}

/*******************************************************************************
 * Secure Partition Manager SMC handler.
 ******************************************************************************/
uint64_t spmc_smc_handler(uint32_t smc_fid,
			  bool secure_origin,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	switch (smc_fid) {

	case FFA_VERSION:
		return ffa_version_handler(smc_fid, secure_origin, x1, x2, x3,
					   x4, cookie, handle, flags);

	case FFA_SPM_ID_GET:
		return ffa_spm_id_get_handler(smc_fid, secure_origin, x1, x2,
					     x3, x4, cookie, handle, flags);

	case FFA_ID_GET:
		return ffa_id_get_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_FEATURES:
		return ffa_features_handler(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_SECONDARY_EP_REGISTER_SMC64:
		return ffa_sec_ep_register_handler(smc_fid, secure_origin, x1,
						   x2, x3, x4, cookie, handle,
						   flags);

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
	case FFA_MSG_SEND_DIRECT_REQ2_SMC64:
		return direct_req_smc_handler(smc_fid, secure_origin, x1, x2,
					      x3, x4, cookie, handle, flags);

	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
	case FFA_MSG_SEND_DIRECT_RESP2_SMC64:
		return direct_resp_smc_handler(smc_fid, secure_origin, x1, x2,
					       x3, x4, cookie, handle, flags);

	case FFA_RXTX_MAP_SMC32:
	case FFA_RXTX_MAP_SMC64:
		return rxtx_map_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_RXTX_UNMAP:
		return rxtx_unmap_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_PARTITION_INFO_GET:
		return partition_info_get_handler(smc_fid, secure_origin, x1,
						  x2, x3, x4, cookie, handle,
						  flags);

	case FFA_RX_RELEASE:
		return rx_release_handler(smc_fid, secure_origin, x1, x2, x3,
					  x4, cookie, handle, flags);

	case FFA_MSG_WAIT:
		return msg_wait_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_ERROR:
		return ffa_error_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_MSG_RUN:
		return ffa_run_handler(smc_fid, secure_origin, x1, x2, x3, x4,
				       cookie, handle, flags);

	case FFA_MEM_SHARE_SMC32:
	case FFA_MEM_SHARE_SMC64:
	case FFA_MEM_LEND_SMC32:
	case FFA_MEM_LEND_SMC64:
		return spmc_ffa_mem_send(smc_fid, secure_origin, x1, x2, x3, x4,
					 cookie, handle, flags);

	case FFA_MEM_FRAG_TX:
		return spmc_ffa_mem_frag_tx(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_MEM_FRAG_RX:
		return spmc_ffa_mem_frag_rx(smc_fid, secure_origin, x1, x2, x3,
					    x4, cookie, handle, flags);

	case FFA_MEM_RETRIEVE_REQ_SMC32:
	case FFA_MEM_RETRIEVE_REQ_SMC64:
		return spmc_ffa_mem_retrieve_req(smc_fid, secure_origin, x1, x2,
						 x3, x4, cookie, handle, flags);

	case FFA_MEM_RELINQUISH:
		return spmc_ffa_mem_relinquish(smc_fid, secure_origin, x1, x2,
					       x3, x4, cookie, handle, flags);

	case FFA_MEM_RECLAIM:
		return spmc_ffa_mem_reclaim(smc_fid, secure_origin, x1, x2, x3,
						x4, cookie, handle, flags);
	case FFA_CONSOLE_LOG_SMC32:
	case FFA_CONSOLE_LOG_SMC64:
		return spmc_ffa_console_log(smc_fid, secure_origin, x1, x2, x3,
						x4, cookie, handle, flags);

	case FFA_MEM_PERM_GET_SMC32:
	case FFA_MEM_PERM_GET_SMC64:
		return ffa_mem_perm_get_handler(smc_fid, secure_origin, x1, x2,
						x3, x4, cookie, handle, flags);

	case FFA_MEM_PERM_SET_SMC32:
	case FFA_MEM_PERM_SET_SMC64:
		return ffa_mem_perm_set_handler(smc_fid, secure_origin, x1, x2,
						x3, x4, cookie, handle, flags);

	default:
		WARN("Unsupported FF-A call 0x%08x.\n", smc_fid);
		break;
	}
	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the SPMC. It
 * validates the interrupt and upon success arranges entry into the SP for
 * handling the interrupt.
 ******************************************************************************/
static uint64_t spmc_sp_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	struct sp_exec_ctx *ec;
	uint32_t linear_id = plat_my_core_pos();

	/* Sanity check for a NULL pointer dereference. */
	assert(sp != NULL);

	/* Check the security state when the exception was generated. */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Panic if not an S-EL1 Partition. */
	if (sp->runtime_el != S_EL1) {
		ERROR("Interrupt received for a non S-EL1 SP on core%u.\n",
		      linear_id);
		panic();
	}

	/* Obtain a reference to the SP execution context. */
	ec = spmc_get_sp_ec(sp);

	/* Ensure that the execution context is in waiting state else panic. */
	if (ec->rt_state != RT_STATE_WAITING) {
		ERROR("SP EC on core%u is not waiting (%u), it is (%u).\n",
		      linear_id, RT_STATE_WAITING, ec->rt_state);
		panic();
	}

	/* Update the runtime model and state of the partition. */
	ec->rt_model = RT_MODEL_INTR;
	ec->rt_state = RT_STATE_RUNNING;

	VERBOSE("SP (0x%x) interrupt start on core%u.\n", sp->sp_id, linear_id);

	/*
	 * Forward the interrupt to the S-EL1 SP. The interrupt ID is not
	 * populated as the SP can determine this by itself.
	 * The flags field is forced to 0 mainly to pass the SVE hint bit
	 * cleared for consumption by the lower EL.
	 */
	return spmd_smc_switch_state(FFA_INTERRUPT, false,
				     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				     FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				     handle, 0ULL, sp->ffa_version);
}

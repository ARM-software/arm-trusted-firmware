/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <common/runtime_svc.h>
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

#include <platform_def.h>

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
	/* Check for SWd Partitions. */
	for (unsigned int i = 0U; i < SECURE_PARTITION_COUNT; i++) {
		if (sp_desc[i].sp_id == id) {
			return &(sp_desc[i]);
		}
	}
	return NULL;
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
				uint16_t dst_id)
{
	/* If the destination is in the normal world always go via the SPMD. */
	if (ffa_is_normal_world_id(dst_id)) {
		return spmd_smc_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags);
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
					     x3, x4, handle);
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
	/*
	 * We currently only support partition messages, therefore ensure x2 is
	 * not set.
	 */
	if (x2 != (uint64_t) 0) {
		VERBOSE("Arg2 MBZ for partition messages (0x%lx).\n", x2);
		return false;
	}
	return true;
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
	uint16_t dst_id = ffa_endpoint_destination(x1);
	struct el3_lp_desc *el3_lp_descs;
	struct secure_partition_desc *sp;
	unsigned int idx;

	/* Check if arg2 has been populated correctly based on message type. */
	if (!direct_msg_validate_arg2(x2)) {
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	el3_lp_descs = get_el3_lp_array();

	/* Check if the request is destined for a Logical Partition. */
	for (unsigned int i = 0U; i < MAX_EL3_LP_DESCS_COUNT; i++) {
		if (el3_lp_descs[i].sp_id == dst_id) {
			return el3_lp_descs[i].direct_req(
					smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);
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

	/*
	 * Check that the target execution context is in a waiting state before
	 * forwarding the direct request to it.
	 */
	idx = get_ec_index(sp);
	if (sp->ec[idx].rt_state != RT_STATE_WAITING) {
		VERBOSE("SP context on core%u is not waiting (%u).\n",
			idx, sp->ec[idx].rt_model);
		return spmc_ffa_error_return(handle, FFA_ERROR_BUSY);
	}

	/*
	 * Everything checks out so forward the request to the SP after updating
	 * its state and runtime model.
	 */
	sp->ec[idx].rt_state = RT_STATE_RUNNING;
	sp->ec[idx].rt_model = RT_MODEL_DIR_REQ;
	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, dst_id);
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
	struct secure_partition_desc *sp;
	unsigned int idx;

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
	if ((dst_id != FFA_SPMC_ID) && ffa_is_secure_world_id(dst_id)) {
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

	/* Sanity check state is being tracked correctly in the SPMC. */
	idx = get_ec_index(sp);
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model != RT_MODEL_DIR_REQ) {
		VERBOSE("SP context on core%u not handling direct req (%u).\n",
			idx, sp->ec[idx].rt_model);
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

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
			       handle, cookie, flags, dst_id);
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

	/* Ensure SP execution context was in the right runtime model. */
	if (sp->ec[idx].rt_model == RT_MODEL_DIR_REQ) {
		return spmc_ffa_error_return(handle, FFA_ERROR_DENIED);
	}

	/* Sanity check the state is being tracked correctly in the SPMC. */
	assert(sp->ec[idx].rt_state == RT_STATE_RUNNING);

	/*
	 * Perform a synchronous exit if the partition was initialising. The
	 * state is updated after the exit.
	 */
	if (sp->ec[idx].rt_model == RT_MODEL_INIT) {
		spmc_sp_synchronous_exit(&sp->ec[idx], x4);
		/* Should not get here */
		panic();
	}

	/* Update the state of the SP execution context. */
	sp->ec[idx].rt_state = RT_STATE_WAITING;

	/* Resume normal world if a secure interrupt was handled. */
	if (sp->ec[idx].rt_model == RT_MODEL_INTR) {
		/* FFA_MSG_WAIT can only be called from the secure world. */
		unsigned int secure_state_in = SECURE;
		unsigned int secure_state_out = NON_SECURE;

		cm_el1_sysregs_context_save(secure_state_in);
		cm_el1_sysregs_context_restore(secure_state_out);
		cm_set_next_eret_context(secure_state_out);
		SMC_RET0(cm_get_context(secure_state_out));
	}

	/* Forward the response to the Normal world. */
	return spmc_smc_return(smc_fid, secure_origin, x1, x2, x3, x4,
			       handle, cookie, flags, FFA_NWD_ID);
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

	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

/*******************************************************************************
 * This function will parse the Secure Partition Manifest. From manifest, it
 * will fetch details for preparing Secure partition image context and secure
 * partition image boot arguments if any.
 ******************************************************************************/
static int sp_manifest_parse(void *sp_manifest, int offset,
			     struct secure_partition_desc *sp,
			     entry_point_info_t *ep_info)
{
	int32_t ret, node;
	uint32_t config_32;

	/*
	 * Look for the mandatory fields that are expected to be present in
	 * the SP manifests.
	 */
	node = fdt_path_offset(sp_manifest, "/");
	if (node < 0) {
		ERROR("Did not find root node.\n");
		return node;
	}

	ret = fdt_read_uint32_array(sp_manifest, node, "uuid",
				    ARRAY_SIZE(sp->uuid), sp->uuid);
	if (ret != 0) {
		ERROR("Missing Secure Partition UUID.\n");
		return ret;
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
	if (config_32 != PLATFORM_CORE_COUNT) {
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
	uintptr_t manifest_base_align;
	entry_point_info_t *next_image_ep_info;
	int32_t ret;
	struct secure_partition_desc *sp;

	next_image_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (next_image_ep_info == NULL) {
		WARN("No Secure Partition image provided by BL2.\n");
		return -ENOENT;
	}

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
	INFO("Manifest size = %lu bytes.\n", next_image_ep_info->args.arg1);

	/*
	 * Select an SP descriptor for initialising the partition's execution
	 * context on the primary CPU.
	 */
	sp = spmc_get_current_sp_ctx();

	/* Initialize entry point information for the SP */
	SET_PARAM_HEAD(next_image_ep_info, PARAM_EP, VERSION_1,
		       SECURE | EP_ST_ENABLE);

	/* Parse the SP manifest. */
	ret = sp_manifest_parse(sp_manifest, ret, sp, next_image_ep_info);
	if (ret != 0) {
		ERROR("Error in Secure Partition manifest parsing.\n");
		return ret;
	}

	/* Check that the runtime EL in the manifest was correct. */
	if (sp->runtime_el != S_EL1) {
		ERROR("Unexpected runtime EL: %d\n", sp->runtime_el);
		return -EINVAL;
	}

	/* Perform any common initialisation. */
	spmc_sp_common_setup(sp, next_image_ep_info);

	/* Perform any initialisation specific to S-EL1 SPs. */
	spmc_el1_sp_setup(sp, next_image_ep_info);

	/* Initialize the SP context with the required ep info. */
	spmc_sp_common_ep_commit(sp, next_image_ep_info);

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
	spmc_attrs->major_version = FFA_VERSION_MAJOR;
	spmc_attrs->minor_version = FFA_VERSION_MINOR;
	spmc_attrs->exec_state = MODE_RW_64;
	spmc_attrs->spmc_id = FFA_SPMC_ID;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spmc_setup(void)
{
	int32_t ret;

	/* Initialize endpoint descriptors */
	initalize_sp_descs();
	initalize_ns_ep_descs();

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

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
		return direct_req_smc_handler(smc_fid, secure_origin, x1, x2,
					      x3, x4, cookie, handle, flags);

	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
		return direct_resp_smc_handler(smc_fid, secure_origin, x1, x2,
					       x3, x4, cookie, handle, flags);

	case FFA_MSG_WAIT:
		return msg_wait_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	case FFA_ERROR:
		return ffa_error_handler(smc_fid, secure_origin, x1, x2, x3, x4,
					cookie, handle, flags);

	default:
		WARN("Unsupported FF-A call 0x%08x.\n", smc_fid);
		break;
	}
	return spmc_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
}

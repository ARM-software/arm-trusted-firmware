/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include "ffa_helpers.h"
#include <services/ffa_svc.h>
#include "tsp_private.h"

/*******************************************************************************
 * Wrapper function to send a direct request.
 ******************************************************************************/
smc_args_t ffa_msg_send_direct_req(ffa_endpoint_id16_t sender,
				   ffa_endpoint_id16_t receiver,
				   uint32_t arg3,
				   uint32_t arg4,
				   uint32_t arg5,
				   uint32_t arg6,
				   uint32_t arg7)
{
	uint32_t src_dst_ids = (sender << FFA_DIRECT_MSG_SOURCE_SHIFT) |
			       (receiver << FFA_DIRECT_MSG_DESTINATION_SHIFT);


	/* Send Direct Request. */
	return smc_helper(FFA_MSG_SEND_DIRECT_REQ_SMC64, src_dst_ids,
			0, arg3, arg4, arg5, arg6, arg7);
}

/*******************************************************************************
 * Wrapper function to send a direct response.
 ******************************************************************************/
smc_args_t *ffa_msg_send_direct_resp(ffa_endpoint_id16_t sender,
				     ffa_endpoint_id16_t receiver,
				     uint32_t arg3,
				     uint32_t arg4,
				     uint32_t arg5,
				     uint32_t arg6,
				     uint32_t arg7)
{
	uint32_t src_dst_ids = (sender << FFA_DIRECT_MSG_SOURCE_SHIFT) |
			       (receiver << FFA_DIRECT_MSG_DESTINATION_SHIFT);

	return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC64, src_dst_ids,
			    0, arg3, arg4, arg5, arg6, arg7);
}

/*******************************************************************************
 * Memory Management Helpers.
 ******************************************************************************/

/**
 * Initialises the header of the given `ffa_mtd`, not including the
 * composite memory region offset.
 */
static void ffa_memory_region_init_header(
	struct ffa_mtd *memory_region, ffa_endpoint_id16_t sender,
	ffa_mem_attr16_t attributes, ffa_mtd_flag32_t flags,
	uint64_t handle, uint64_t tag, ffa_endpoint_id16_t *receivers,
	uint32_t receiver_count, ffa_mem_perm8_t permissions)
{
	struct ffa_emad_v1_0 *emad;

	memory_region->emad_offset = sizeof(struct ffa_mtd);
	memory_region->emad_size = sizeof(struct ffa_emad_v1_0);
	emad = (struct ffa_emad_v1_0 *)
		    ((uint8_t *) memory_region +
		     memory_region->emad_offset);
	memory_region->sender_id = sender;
	memory_region->memory_region_attributes = attributes;
	memory_region->reserved_36_39 = 0;
	memory_region->flags = flags;
	memory_region->handle = handle;
	memory_region->tag = tag;
	memory_region->reserved_40_47 = 0;
	memory_region->emad_count = receiver_count;
	for (uint32_t i = 0U; i < receiver_count; i++) {
		emad[i].mapd.endpoint_id = receivers[i];
		emad[i].mapd.memory_access_permissions = permissions;
		emad[i].mapd.flags = 0;
		emad[i].comp_mrd_offset = 0;
		emad[i].reserved_8_15 = 0;
	}
}
/**
 * Initialises the given `ffa_mtd` to be used for an
 * `FFA_MEM_RETRIEVE_REQ` by the receiver of a memory transaction.
 * TODO: Support differing attributes per receiver.
 *
 * Returns the size of the descriptor written.
 */
static uint32_t ffa_memory_retrieve_request_init(
	struct ffa_mtd *memory_region, uint64_t handle,
	ffa_endpoint_id16_t sender, ffa_endpoint_id16_t *receivers, uint32_t receiver_count,
	uint64_t tag, ffa_mtd_flag32_t flags,
	ffa_mem_perm8_t permissions,
	ffa_mem_attr16_t attributes)
{
	ffa_memory_region_init_header(memory_region, sender, attributes, flags,
				      handle, tag, receivers,
				      receiver_count, permissions);

	return sizeof(struct ffa_mtd) +
	       memory_region->emad_count * sizeof(struct ffa_emad_v1_0);
}

/* Relinquish access to memory region. */
bool ffa_mem_relinquish(void)
{
	smc_args_t ret;

	ret = smc_helper(FFA_MEM_RELINQUISH, 0, 0, 0, 0, 0, 0, 0);
	if (ffa_func_id(ret) != FFA_SUCCESS_SMC32) {
		ERROR("%s failed to relinquish memory! error: (%x) %x\n",
		      __func__, ffa_func_id(ret), ffa_error_code(ret));
		return false;
	}
	return true;
}

/* Retrieve memory shared by another partition. */
smc_args_t ffa_mem_retrieve_req(uint32_t descriptor_length,
				uint32_t fragment_length)
{
	return smc_helper(FFA_MEM_RETRIEVE_REQ_SMC32,
		      descriptor_length,
		      fragment_length,
		      0, 0, 0, 0, 0);
}

/* Retrieve the next memory descriptor fragment. */
smc_args_t ffa_mem_frag_rx(uint64_t handle, uint32_t recv_length)
{
	return smc_helper(FFA_MEM_FRAG_RX,
		       FFA_MEM_HANDLE_LOW(handle),
		       FFA_MEM_HANDLE_HIGH(handle),
		       recv_length,
		       0, 0, 0, 0);
}

bool memory_retrieve(struct mailbox *mb,
			    struct ffa_mtd **retrieved,
			    uint64_t handle, ffa_endpoint_id16_t sender,
			    ffa_endpoint_id16_t *receivers, uint32_t receiver_count,
			    ffa_mtd_flag32_t flags, uint32_t *frag_length,
			    uint32_t *total_length)
{
	smc_args_t ret;
	uint32_t descriptor_size;
	struct ffa_mtd *memory_region;

	if (retrieved == NULL || mb == NULL) {
		ERROR("Invalid parameters!\n");
		return false;
	}

	memory_region = (struct ffa_mtd *)mb->tx_buffer;

	/* Clear TX buffer. */
	memset(memory_region, 0, PAGE_SIZE);

	/* Clear local buffer. */
	memset(mem_region_buffer, 0, REGION_BUF_SIZE);

	descriptor_size = ffa_memory_retrieve_request_init(
	    memory_region, handle, sender, receivers, receiver_count, 0, flags,
	    FFA_MEM_PERM_RW | FFA_MEM_PERM_NX,
	    FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB |
	    FFA_MEM_ATTR_INNER_SHAREABLE);

	ret = ffa_mem_retrieve_req(descriptor_size, descriptor_size);

	if (ffa_func_id(ret) == FFA_ERROR) {
		ERROR("Couldn't retrieve the memory page. Error: %x\n",
		      ffa_error_code(ret));
		return false;
	}

	/*
	 * Following total_size and fragment_size are useful to keep track
	 * of the state of transaction. When the sum of all fragment_size of all
	 * fragments is equal to total_size, the memory transaction has been
	 * completed.
	 */
	*total_length = ret._regs[1];
	*frag_length = ret._regs[2];

	/* Validate frag_length is less than total_length and mailbox size. */
	if (*frag_length == 0U || *total_length == 0U ||
	    *frag_length > *total_length || *frag_length > (mb->rxtx_page_count * PAGE_SIZE)) {
		ERROR("Invalid parameters!\n");
		return false;
	}

	/* Copy response to local buffer. */
	memcpy(mem_region_buffer, mb->rx_buffer, *frag_length);

	if (ffa_rx_release()) {
		ERROR("Failed to release buffer!\n");
		return false;
	}

	*retrieved = (struct ffa_mtd *) mem_region_buffer;

	if ((*retrieved)->emad_count > MAX_MEM_SHARE_RECIPIENTS) {
		VERBOSE("SPMC memory sharing supports max of %u receivers!\n",
			MAX_MEM_SHARE_RECIPIENTS);
		return false;
	}

	/*
	 * We are sharing memory from the normal world therefore validate the NS
	 * bit was set by the SPMC.
	 */
	if (((*retrieved)->memory_region_attributes & FFA_MEM_ATTR_NS_BIT) == 0U) {
		ERROR("SPMC has not set the NS bit! 0x%x\n",
		      (*retrieved)->memory_region_attributes);
		return false;
	}

	VERBOSE("Memory Descriptor Retrieved!\n");

	return true;
}

/* Relinquish the memory region. */
bool memory_relinquish(struct ffa_mem_relinquish_descriptor *m, uint64_t handle,
		       ffa_endpoint_id16_t id)
{
	ffa_mem_relinquish_init(m, handle, 0, id);
	return ffa_mem_relinquish();
}

/* Query SPMC that the rx buffer of the partition can be released. */
bool ffa_rx_release(void)
{
	smc_args_t ret;

	ret = smc_helper(FFA_RX_RELEASE, 0, 0, 0, 0, 0, 0, 0);
	return ret._regs[SMC_ARG0] != FFA_SUCCESS_SMC32;
}

/* Map the provided buffers with the SPMC. */
bool ffa_rxtx_map(uintptr_t send, uintptr_t recv, uint32_t pages)
{
	smc_args_t ret;

	ret = smc_helper(FFA_RXTX_MAP_SMC64, send, recv, pages, 0, 0, 0, 0);
	return ret._regs[0] != FFA_SUCCESS_SMC32;
}

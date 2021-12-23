/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include "ffa_helpers.h"
#include <services/ffa_svc.h>
#include "tsp_private.h"

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
uint32_t ffa_memory_retrieve_request_init(
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

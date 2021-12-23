/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_HELPERS_H
#define FFA_HELPERS_H

#include <stdint.h>

#include "../../services/std_svc/spm/el3_spmc/spmc.h"
#include "../../services/std_svc/spm/el3_spmc/spmc_shared_mem.h"
#include <services/el3_spmc_ffa_memory.h>
#include <services/ffa_svc.h>
#include "tsp_private.h"

static inline uint32_t ffa_func_id(smc_args_t val)
{
	return (uint32_t) val._regs[0];
}

static inline int32_t ffa_error_code(smc_args_t val)
{
	return (uint32_t) val._regs[2];
}

/** The maximum number of recipients a memory region may be sent to. */
#define MAX_MEM_SHARE_RECIPIENTS	2U

/* FFA Memory Management mode flags. */
#define FFA_FLAG_SHARE_MEMORY (1U << 3)
#define FFA_FLAG_LEND_MEMORY (1U << 4)

#define FFA_FLAG_MEMORY_MASK (3U << 3)

#define FFA_MEM_HANDLE_LOW(x) (x & 0xFFFFFFFF)
#define FFA_MEM_HANDLE_HIGH(x) (x >> 32)

#define FFA_MEM_PERM_DATA_OFFSET 0
#define FFA_MEM_PERM_DATA_MASK 0x3

static inline uint32_t ffa_mem_relinquish_init(
	struct ffa_mem_relinquish_descriptor *relinquish_request,
	uint64_t handle, ffa_mtd_flag32_t flags,
	ffa_endpoint_id16_t sender)
{
	relinquish_request->handle = handle;
	relinquish_request->flags = flags;
	relinquish_request->endpoint_count = 1;
	relinquish_request->endpoint_array[0] = sender;

	return sizeof(struct ffa_mem_relinquish_descriptor) + sizeof(ffa_endpoint_id16_t);
}

/**
 * Gets the `ffa_comp_mrd` for the given receiver from an
 * `ffa_mtd`, or NULL if it is not valid.
 */
static inline struct ffa_comp_mrd *
ffa_memory_region_get_composite(struct ffa_mtd *memory_region,
				uint32_t receiver_index)
{
	struct ffa_emad_v1_0 *receivers;
	uint32_t offset;

	receivers = (struct ffa_emad_v1_0 *)
		    ((uint8_t *) memory_region +
		    memory_region->emad_offset +
		    (memory_region->emad_size * receiver_index));
	offset = receivers->comp_mrd_offset;

	if (offset == 0U) {
		return NULL;
	}

	return (struct ffa_comp_mrd *)
	       ((uint8_t *) memory_region + offset);
}

static inline uint32_t ffa_get_data_access_attr(ffa_mem_perm8_t perm)
{
	return ((perm >> FFA_MEM_PERM_DATA_OFFSET) & FFA_MEM_PERM_DATA_MASK);
}

/**
 * Initialises the given `ffa_mtd` to be used for an
 * `FFA_MEM_RETRIEVE_REQ` by the receiver of a memory transaction.
 *
 * Returns the size of the message written.
 */
uint32_t ffa_memory_retrieve_request_init(
	struct ffa_mtd *memory_region, uint64_t handle,
	ffa_endpoint_id16_t sender, ffa_endpoint_id16_t *test_receivers, uint32_t receiver_count,
	uint64_t tag, ffa_mtd_flag32_t flags,
	ffa_mem_perm8_t permissions, ffa_mem_attr16_t attributes);

smc_args_t ffa_mem_frag_rx(uint64_t handle, uint32_t recv_length);
smc_args_t ffa_mem_retrieve_req(uint32_t descriptor_length,
				uint32_t fragment_length);
bool ffa_mem_relinquish(void);
bool ffa_rx_release(void);
bool memory_relinquish(struct ffa_mem_relinquish_descriptor *m, uint64_t handle,
		       ffa_endpoint_id16_t id);
bool ffa_rxtx_map(uintptr_t send, uintptr_t recv, uint32_t pages);

#endif /* FFA_HELPERS_H */

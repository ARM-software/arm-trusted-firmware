/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMC_SHARED_MEM_H
#define SPMC_SHARED_MEM_H

#include <services/el3_spmc_ffa_memory.h>

/**
 * struct ffa_mem_relinquish_descriptor - Relinquish request descriptor.
 * @handle:
 *         Id of shared memory object to relinquish.
 * @flags:
 *         If bit 0 is set clear memory after unmapping from borrower. Must be 0
 *         for share. Bit[1]: Time slicing. Not supported, must be 0. All other
 *         bits are reserved 0.
 * @endpoint_count:
 *         Number of entries in @endpoint_array.
 * @endpoint_array:
 *         Array of endpoint ids.
 */
struct ffa_mem_relinquish_descriptor {
	uint64_t handle;
	uint32_t flags;
	uint32_t endpoint_count;
	ffa_endpoint_id16_t endpoint_array[];
};
CASSERT(sizeof(struct ffa_mem_relinquish_descriptor) == 16,
	assert_ffa_mem_relinquish_descriptor_size_mismatch);

/**
 * struct spmc_shmem_obj_state - Global state.
 * @data:           Backing store for spmc_shmem_obj objects.
 * @data_size:      The size allocated for the backing store.
 * @allocated:      Number of bytes allocated in @data.
 * @next_handle:    Handle used for next allocated object.
 * @lock:           Lock protecting all state in this file.
 */
struct spmc_shmem_obj_state {
	uint8_t *data;
	size_t data_size;
	size_t allocated;
	uint64_t next_handle;
	spinlock_t lock;
};

extern struct spmc_shmem_obj_state spmc_shmem_obj_state;
extern int plat_spmc_shmem_begin(struct ffa_mtd *desc);
extern int plat_spmc_shmem_reclaim(struct ffa_mtd *desc);

long spmc_ffa_mem_send(uint32_t smc_fid,
		       bool secure_origin,
		       uint64_t total_length,
		       uint32_t fragment_length,
		       uint64_t address,
		       uint32_t page_count,
		       void *cookie,
		       void *handle,
		       uint64_t flags);

long spmc_ffa_mem_frag_tx(uint32_t smc_fid,
			  bool secure_origin,
			  uint64_t handle_low,
			  uint64_t handle_high,
			  uint32_t fragment_length,
			  uint32_t sender_id,
			  void *cookie,
			  void *handle,
			  uint64_t flags);

long spmc_ffa_mem_retrieve_req(uint32_t smc_fid,
			       bool secure_origin,
			       uint32_t total_length,
			       uint32_t fragment_length,
			       uint64_t address,
			       uint32_t page_count,
			       void *cookie,
			       void *handle,
			       uint64_t flags);

long spmc_ffa_mem_frag_rx(uint32_t smc_fid,
			  bool secure_origin,
			  uint32_t handle_low,
			  uint32_t handle_high,
			  uint32_t fragment_offset,
			  uint32_t sender_id,
			  void *cookie,
			  void *handle,
			  uint64_t flags);


int spmc_ffa_mem_relinquish(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t handle_low,
			    uint32_t handle_high,
			    uint32_t fragment_offset,
			    uint32_t sender_id,
			    void *cookie,
			    void *handle,
			    uint64_t flags);

int spmc_ffa_mem_reclaim(uint32_t smc_fid,
			 bool secure_origin,
			 uint32_t handle_low,
			 uint32_t handle_high,
			 uint32_t mem_flags,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags);

#endif /* SPMC_SHARED_MEM_H */

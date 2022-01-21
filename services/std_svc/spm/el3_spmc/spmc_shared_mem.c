/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/object_pool.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/ffa_svc.h>
#include "spmc.h"
#include "spmc_shared_mem.h"

#include <platform_def.h>

/**
 * struct spmc_shmem_obj - Shared memory object.
 * @desc_size:      Size of @desc.
 * @desc_filled:    Size of @desc already received.
 * @in_use:         Number of clients that have called ffa_mem_retrieve_req
 *                  without a matching ffa_mem_relinquish call.
 * @desc:           FF-A memory region descriptor passed in ffa_mem_share.
 */
struct spmc_shmem_obj {
	size_t desc_size;
	size_t desc_filled;
	size_t in_use;
	struct ffa_mtd_v1_0 desc;
};

/*
 * Declare our data structure to store the metadata of memory share requests.
 * The main datastore is allocated on a per platform basis to ensure enough
 * storage can be made available.
 * The address of the data store will be populated by the SPMC during its
 * initialization.
 */

struct spmc_shmem_obj_state spmc_shmem_obj_state = {
	/* Set start value for handle so top 32 bits are needed quickly. */
	.next_handle = 0xffffffc0U,
};

/**
 * spmc_shmem_obj_size - Convert from descriptor size to object size.
 * @desc_size:  Size of struct ffa_memory_region_descriptor object.
 *
 * Return: Size of struct spmc_shmem_obj object.
 */
static size_t spmc_shmem_obj_size(size_t desc_size)
{
	return desc_size + offsetof(struct spmc_shmem_obj, desc);
}

/**
 * spmc_shmem_obj_alloc - Allocate struct spmc_shmem_obj.
 * @state:      Global state.
 * @desc_size:  Size of struct ffa_memory_region_descriptor object that
 *              allocated object will hold.
 *
 * Return: Pointer to newly allocated object, or %NULL if there not enough space
 *         left. The returned pointer is only valid while @state is locked, to
 *         used it again after unlocking @state, spmc_shmem_obj_lookup must be
 *         called.
 */
static struct spmc_shmem_obj *
spmc_shmem_obj_alloc(struct spmc_shmem_obj_state *state, size_t desc_size)
{
	struct spmc_shmem_obj *obj;
	size_t free = state->data_size - state->allocated;

	if (state->data == NULL) {
		ERROR("Missing shmem datastore!\n");
		return NULL;
	}

	if (spmc_shmem_obj_size(desc_size) > free) {
		WARN("%s(0x%zx) failed, free 0x%zx\n",
		     __func__, desc_size, free);
		return NULL;
	}
	obj = (struct spmc_shmem_obj *)(state->data + state->allocated);
	obj->desc = (struct ffa_mtd_v1_0) {0};
	obj->desc_size = desc_size;
	obj->desc_filled = 0;
	obj->in_use = 0;
	state->allocated += spmc_shmem_obj_size(desc_size);
	return obj;
}

/**
 * spmc_shmem_obj_free - Free struct spmc_shmem_obj.
 * @state:      Global state.
 * @obj:        Object to free.
 *
 * Release memory used by @obj. Other objects may move, so on return all
 * pointers to struct spmc_shmem_obj object should be considered invalid, not
 * just @obj.
 *
 * The current implementation always compacts the remaining objects to simplify
 * the allocator and to avoid fragmentation.
 */

static void spmc_shmem_obj_free(struct spmc_shmem_obj_state *state,
				  struct spmc_shmem_obj *obj)
{
	size_t free_size = spmc_shmem_obj_size(obj->desc_size);
	uint8_t *shift_dest = (uint8_t *)obj;
	uint8_t *shift_src = shift_dest + free_size;
	size_t shift_size = state->allocated - (shift_src - state->data);

	if (shift_size != 0U) {
		memmove(shift_dest, shift_src, shift_size);
	}
	state->allocated -= free_size;
}

/**
 * spmc_shmem_obj_lookup - Lookup struct spmc_shmem_obj by handle.
 * @state:      Global state.
 * @handle:     Unique handle of object to return.
 *
 * Return: struct spmc_shmem_obj_state object with handle matching @handle.
 *         %NULL, if not object in @state->data has a matching handle.
 */
static struct spmc_shmem_obj *
spmc_shmem_obj_lookup(struct spmc_shmem_obj_state *state, uint64_t handle)
{
	uint8_t *curr = state->data;

	while (curr - state->data < state->allocated) {
		struct spmc_shmem_obj *obj = (struct spmc_shmem_obj *)curr;

		if (obj->desc.handle == handle) {
			return obj;
		}
		curr += spmc_shmem_obj_size(obj->desc_size);
	}
	return NULL;
}

/**
 * spmc_shmem_obj_get_next - Get the next memory object from an offset.
 * @offset:     Offset used to track which objects have previously been
 *              returned.
 *
 * Return: the next struct spmc_shmem_obj_state object from the provided
 *	   offset.
 *	   %NULL, if there are no more objects.
 */
static struct spmc_shmem_obj *
spmc_shmem_obj_get_next(struct spmc_shmem_obj_state *state, size_t *offset)
{
	uint8_t *curr = state->data + *offset;

	if (curr - state->data < state->allocated) {
		struct spmc_shmem_obj *obj = (struct spmc_shmem_obj *)curr;

		*offset += spmc_shmem_obj_size(obj->desc_size);

		return obj;
	}
	return NULL;
}

static struct ffa_comp_mrd *
spmc_shmem_obj_get_comp_mrd(struct spmc_shmem_obj *obj)
{
	return (struct ffa_comp_mrd *)
		((uint8_t *)(&obj->desc) + obj->desc.emad[0].comp_mrd_offset);
}

/**
 * spmc_shmem_obj_ffa_constituent_size - Calculate variable size part of obj.
 * @obj:    Object containing ffa_memory_region_descriptor.
 *
 * Return: Size of ffa_constituent_memory_region_descriptors in @obj.
 */
static size_t
spmc_shmem_obj_ffa_constituent_size(struct spmc_shmem_obj *obj)
{
	return spmc_shmem_obj_get_comp_mrd(obj)->address_range_count *
		sizeof(struct ffa_cons_mrd);
}

/*
 * Compare two memory regions to determine if any range overlaps with another
 * ongoing memory transaction.
 */
static bool
overlapping_memory_regions(struct ffa_comp_mrd *region1,
			   struct ffa_comp_mrd *region2)
{
	uint64_t region1_start;
	uint64_t region1_size;
	uint64_t region1_end;
	uint64_t region2_start;
	uint64_t region2_size;
	uint64_t region2_end;

	assert(region1 != NULL);
	assert(region2 != NULL);

	if (region1 == region2) {
		return true;
	}

	/*
	 * Check each memory region in the request against existing
	 * transactions.
	 */
	for (size_t i = 0; i < region1->address_range_count; i++) {

		region1_start = region1->address_range_array[i].address;
		region1_size =
			region1->address_range_array[i].page_count *
			PAGE_SIZE_4KB;
		region1_end = region1_start + region1_size;

		for (size_t j = 0; j < region2->address_range_count; j++) {

			region2_start = region2->address_range_array[j].address;
			region2_size =
				region2->address_range_array[j].page_count *
				PAGE_SIZE_4KB;
			region2_end = region2_start + region2_size;

			if ((region1_start >= region2_start &&
			     region1_start < region2_end) ||
			    (region1_end >= region2_start
			     && region1_end < region2_end)) {
				WARN("Overlapping mem regions 0x%lx-0x%lx & 0x%lx-0x%lx\n",
				     region1_start, region1_end,
				     region2_start, region2_end);
				return true;
			}
		}
	}
	return false;
}

/**
 * spmc_shmem_check_obj - Check that counts in descriptor match overall size.
 * @obj:    Object containing ffa_memory_region_descriptor.
 *
 * Return: 0 if object is valid, -EINVAL if constituent_memory_region_descriptor
 * offset or count is invalid.
 */
static int spmc_shmem_check_obj(struct spmc_shmem_obj *obj)
{
	uint32_t comp_mrd_offset = 0;

	if (obj->desc.emad_count == 0U) {
		WARN("%s: unsupported attribute desc count %u.\n",
		     __func__, obj->desc.emad_count);
		return -EINVAL;
	}

	/*
	 * Ensure the emad array lies within the bounds of the descriptor by
	 * checking the address of the element past the end of the array.
	 */
	if ((uintptr_t) &obj->desc.emad[obj->desc.emad_count] >
	    (uintptr_t)((uint8_t *) &obj->desc + obj->desc_size)) {
		WARN("Invalid emad access.\n");
		return -EINVAL;
	}

	for (size_t emad_num = 0; emad_num < obj->desc.emad_count; emad_num++) {
		size_t size;
		size_t count;
		size_t expected_size;
		size_t total_page_count;
		struct ffa_comp_mrd *comp;

		uint32_t offset = obj->desc.emad[emad_num].comp_mrd_offset;
		size_t header_emad_size = sizeof(obj->desc) +
			obj->desc.emad_count * sizeof(obj->desc.emad[emad_num]);

		if (offset < header_emad_size) {
			WARN("%s: invalid object, offset %u < header + emad %zu\n",
			     __func__, offset, header_emad_size);
			return -EINVAL;
		}

		size = obj->desc_size;

		if (offset > size) {
			WARN("%s: invalid object, offset %u > total size %zu\n",
			     __func__, offset, obj->desc_size);
			return -EINVAL;
		}
		size -= offset;

		if (size < sizeof(struct ffa_comp_mrd)) {
			WARN("%s: invalid object, offset %u, total size %zu, no header space.\n",
			     __func__, offset, obj->desc_size);
			return -EINVAL;
		}
		size -= sizeof(struct ffa_comp_mrd);

		count = size / sizeof(struct ffa_cons_mrd);

		comp = spmc_shmem_obj_get_comp_mrd(obj);

		if (comp->address_range_count != count) {
			WARN("%s: invalid object, desc count %u != %zu\n",
			     __func__, comp->address_range_count, count);
			return -EINVAL;
		}

		expected_size = offset + sizeof(*comp) +
				spmc_shmem_obj_ffa_constituent_size(obj);
		if (expected_size != obj->desc_size) {
			WARN("%s: invalid object, computed size %zu != size %zu\n",
			       __func__, expected_size, obj->desc_size);
			return -EINVAL;
		}

		if (obj->desc_filled < obj->desc_size) {
			/*
			 * The whole descriptor has not yet been received.
			 * Skip final checks.
			 */
			return 0;
		}

		/*
		 * The offset provided to the composite memory region descriptor
		 * should be consistent across endpoint descriptors. Store the
		 * first entry and compare against subsequent entries.
		 */
		if (comp_mrd_offset == 0) {
			comp_mrd_offset = offset;
		} else {
			if (comp_mrd_offset != offset) {
				ERROR("%s: mismatching offsets provided, %u != %u\n",
				       __func__, offset, comp_mrd_offset);
				return -EINVAL;
			}
		}

		total_page_count = 0;

		for (size_t i = 0; i < count; i++) {
			total_page_count +=
				comp->address_range_array[i].page_count;
		}
		if (comp->total_page_count != total_page_count) {
			WARN("%s: invalid object, desc total_page_count %u != %zu\n",
			     __func__, comp->total_page_count,
			total_page_count);
			return -EINVAL;
		}
	}
	return 0;
}

/**
 * spmc_shmem_check_state_obj - Check if the descriptor describes memory
 *				regions that are currently involved with an
 *				existing memory transactions. This implies that
 *				the memory is not in a valid state for lending.
 * @obj:    Object containing ffa_memory_region_descriptor.
 *
 * Return: 0 if object is valid, -EINVAL if invalid memory state.
 */
static int spmc_shmem_check_state_obj(struct spmc_shmem_obj *obj)
{
	size_t obj_offset = 0;
	struct spmc_shmem_obj *inflight_obj;

	struct ffa_comp_mrd *other_mrd;
	struct ffa_comp_mrd *requested_mrd = spmc_shmem_obj_get_comp_mrd(obj);

	inflight_obj = spmc_shmem_obj_get_next(&spmc_shmem_obj_state,
					       &obj_offset);

	while (inflight_obj != NULL) {
		/*
		 * Don't compare the transaction to itself or to partially
		 * transmitted descriptors.
		 */
		if ((obj->desc.handle != inflight_obj->desc.handle) &&
		    (obj->desc_size == obj->desc_filled)) {
			other_mrd = spmc_shmem_obj_get_comp_mrd(inflight_obj);

			if (overlapping_memory_regions(requested_mrd,
						       other_mrd)) {
				return -EINVAL;
			}
		}

		inflight_obj = spmc_shmem_obj_get_next(&spmc_shmem_obj_state,
						       &obj_offset);
	}
	return 0;
}

static long spmc_ffa_fill_desc(struct mailbox *mbox,
			       struct spmc_shmem_obj *obj,
			       uint32_t fragment_length,
			       ffa_mtd_flag32_t mtd_flag,
			       void *smc_handle)
{
	int ret;
	uint32_t handle_low;
	uint32_t handle_high;

	if (mbox->rxtx_page_count == 0U) {
		WARN("%s: buffer pair not registered.\n", __func__);
		ret = -EINVAL;
		goto err_arg;
	}

	if (fragment_length > mbox->rxtx_page_count * PAGE_SIZE_4KB) {
		WARN("%s: bad fragment size %u > %u buffer size\n", __func__,
		     fragment_length, mbox->rxtx_page_count * PAGE_SIZE_4KB);
		ret = -EINVAL;
		goto err_arg;
	}

	memcpy((uint8_t *)&obj->desc + obj->desc_filled,
	       (uint8_t *) mbox->tx_buffer,
	       fragment_length);

	if (fragment_length > obj->desc_size - obj->desc_filled) {
		WARN("%s: bad fragment size %u > %zu remaining\n", __func__,
		     fragment_length, obj->desc_size - obj->desc_filled);
		ret = -EINVAL;
		goto err_arg;
	}

	/* Ensure that the sender ID resides in the normal world. */
	if (ffa_is_secure_world_id(obj->desc.sender_id)) {
		WARN("%s: Invalid sender ID 0x%x.\n",
		     __func__, obj->desc.sender_id);
		ret = FFA_ERROR_DENIED;
		goto err_arg;
	}

	/*
	 * We don't currently support any optional flags so ensure none are
	 * requested.
	 */
	if (obj->desc.flags != 0U && mtd_flag != 0U &&
	    (obj->desc.flags != mtd_flag)) {
		WARN("%s: invalid memory transaction flags %u != %u\n",
		     __func__, obj->desc.flags, mtd_flag);
		ret = -EINVAL;
		goto err_arg;
	}

	if (obj->desc_filled == 0U) {
		/* First fragment, descriptor header has been copied */
		obj->desc.handle = spmc_shmem_obj_state.next_handle++;
		obj->desc.flags |= mtd_flag;
	}

	obj->desc_filled += fragment_length;

	ret = spmc_shmem_check_obj(obj);
	if (ret != 0) {
		goto err_bad_desc;
	}

	handle_low = (uint32_t)obj->desc.handle;
	handle_high = obj->desc.handle >> 32;

	if (obj->desc_filled != obj->desc_size) {
		SMC_RET8(smc_handle, FFA_MEM_FRAG_RX, handle_low,
			 handle_high, obj->desc_filled,
			 (uint32_t)obj->desc.sender_id << 16, 0, 0, 0);
	}

	/* The full descriptor has been received, perform any final checks. */

	/*
	 * If a partition ID resides in the secure world validate that the
	 * partition ID is for a known partition. Ignore any partition ID
	 * belonging to the normal world as it is assumed the Hypervisor will
	 * have validated these.
	 */
	for (size_t i = 0; i < obj->desc.emad_count; i++) {
		ffa_endpoint_id16_t ep_id = obj->desc.emad[i].mapd.endpoint_id;

		if (ffa_is_secure_world_id(ep_id)) {
			if (spmc_get_sp_ctx(ep_id) == NULL) {
				WARN("%s: Invalid receiver id 0x%x\n",
				     __func__, ep_id);
				ret = FFA_ERROR_INVALID_PARAMETER;
				goto err_bad_desc;
			}
		}
	}

	/* Ensure partition IDs are not duplicated. */
	for (size_t i = 0; i < obj->desc.emad_count; i++) {
		for (size_t j = i + 1; j < obj->desc.emad_count; j++) {
			if (obj->desc.emad[i].mapd.endpoint_id ==
				obj->desc.emad[j].mapd.endpoint_id) {
				ret = FFA_ERROR_INVALID_PARAMETER;
				goto err_bad_desc;
			}
		}
	}

	ret = spmc_shmem_check_state_obj(obj);
	if (ret) {
		ERROR("%s: invalid memory region descriptor.\n", __func__);
		goto err_bad_desc;
	}

	SMC_RET8(smc_handle, FFA_SUCCESS_SMC32, 0, handle_low, handle_high, 0,
		 0, 0, 0);

err_bad_desc:
err_arg:
	spmc_shmem_obj_free(&spmc_shmem_obj_state, obj);
	return spmc_ffa_error_return(smc_handle, FFA_ERROR_INVALID_PARAMETER);
}

/**
 * spmc_ffa_mem_send - FFA_MEM_SHARE/LEND implementation.
 * @client:             Client state.
 * @total_length:       Total length of shared memory descriptor.
 * @fragment_length:    Length of fragment of shared memory descriptor passed in
 *                      this call.
 * @address:            Not supported, must be 0.
 * @page_count:         Not supported, must be 0.
 * @smc_handle:         Handle passed to smc call. Used to return
 *                      FFA_MEM_FRAG_RX or SMC_FC_FFA_SUCCESS.
 *
 * Implements a subset of the FF-A FFA_MEM_SHARE and FFA_MEM_LEND calls needed
 * to share or lend memory from non-secure os to secure os (with no stream
 * endpoints).
 *
 * Return: 0 on success, error code on failure.
 */
long spmc_ffa_mem_send(uint32_t smc_fid,
			bool secure_origin,
			uint64_t total_length,
			uint32_t fragment_length,
			uint64_t address,
			uint32_t page_count,
			void *cookie,
			void *handle,
			uint64_t flags)

{
	long ret;
	struct spmc_shmem_obj *obj;
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);
	ffa_mtd_flag32_t mtd_flag;

	if (address != 0U || page_count != 0U) {
		WARN("%s: custom memory region for message not supported.\n",
		     __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (secure_origin) {
		WARN("%s: unsupported share direction.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (fragment_length < sizeof(obj->desc)) {
		WARN("%s: bad first fragment size %u < %zu\n",
		     __func__, fragment_length, sizeof(obj->desc));
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if ((smc_fid & FUNCID_NUM_MASK) == FFA_FNUM_MEM_SHARE) {
		mtd_flag = FFA_MTD_FLAG_TYPE_SHARE_MEMORY;
	} else if ((smc_fid & FUNCID_NUM_MASK) == FFA_FNUM_MEM_LEND) {
		mtd_flag = FFA_MTD_FLAG_TYPE_LEND_MEMORY;
	} else {
		WARN("%s: invalid memory management operation.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_alloc(&spmc_shmem_obj_state, total_length);
	if (obj == NULL) {
		ret = FFA_ERROR_NO_MEMORY;
		goto err_unlock;
	}

	spin_lock(&mbox->lock);
	ret = spmc_ffa_fill_desc(mbox, obj, fragment_length, mtd_flag, handle);
	spin_unlock(&mbox->lock);

	spin_unlock(&spmc_shmem_obj_state.lock);
	return ret;

err_unlock:
	spin_unlock(&spmc_shmem_obj_state.lock);
	return spmc_ffa_error_return(handle, ret);
}

/**
 * spmc_ffa_mem_frag_tx - FFA_MEM_FRAG_TX implementation.
 * @client:             Client state.
 * @handle_low:         Handle_low value returned from FFA_MEM_FRAG_RX.
 * @handle_high:        Handle_high value returned from FFA_MEM_FRAG_RX.
 * @fragment_length:    Length of fragments transmitted.
 * @sender_id:          Vmid of sender in bits [31:16]
 * @smc_handle:         Handle passed to smc call. Used to return
 *                      FFA_MEM_FRAG_RX or SMC_FC_FFA_SUCCESS.
 *
 * Return: @smc_handle on success, error code on failure.
 */
long spmc_ffa_mem_frag_tx(uint32_t smc_fid,
			  bool secure_origin,
			  uint64_t handle_low,
			  uint64_t handle_high,
			  uint32_t fragment_length,
			  uint32_t sender_id,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	long ret;
	uint32_t desc_sender_id;
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);

	struct spmc_shmem_obj *obj;
	uint64_t mem_handle = handle_low | (((uint64_t)handle_high) << 32);

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_lookup(&spmc_shmem_obj_state, mem_handle);
	if (obj == NULL) {
		WARN("%s: invalid handle, 0x%lx, not a valid handle.\n",
		     __func__, mem_handle);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock;
	}

	desc_sender_id = (uint32_t)obj->desc.sender_id << 16;
	if (sender_id != desc_sender_id) {
		WARN("%s: invalid sender_id 0x%x != 0x%x\n", __func__,
		     sender_id, desc_sender_id);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock;
	}

	if (obj->desc_filled == obj->desc_size) {
		WARN("%s: object desc already filled, %zu\n", __func__,
		     obj->desc_filled);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock;
	}

	spin_lock(&mbox->lock);
	ret = spmc_ffa_fill_desc(mbox, obj, fragment_length, 0, handle);
	spin_unlock(&mbox->lock);

	spin_unlock(&spmc_shmem_obj_state.lock);
	return ret;

err_unlock:
	spin_unlock(&spmc_shmem_obj_state.lock);
	return spmc_ffa_error_return(handle, ret);
}

/**
 * spmc_ffa_mem_retrieve_req - FFA_MEM_RETRIEVE_REQ implementation.
 * @smc_fid:            FID of SMC
 * @total_length:       Total length of retrieve request descriptor if this is
 *                      the first call. Otherwise (unsupported) must be 0.
 * @fragment_length:    Length of fragment of retrieve request descriptor passed
 *                      in this call. Only @fragment_length == @length is
 *                      supported by this implementation.
 * @address:            Not supported, must be 0.
 * @page_count:         Not supported, must be 0.
 * @smc_handle:         Handle passed to smc call. Used to return
 *                      FFA_MEM_RETRIEVE_RESP.
 *
 * Implements a subset of the FF-A FFA_MEM_RETRIEVE_REQ call.
 * Used by secure os to retrieve memory already shared by non-secure os.
 * If the data does not fit in a single FFA_MEM_RETRIEVE_RESP message,
 * the client must call FFA_MEM_FRAG_RX until the full response has been
 * received.
 *
 * Return: @handle on success, error code on failure.
 */
long
spmc_ffa_mem_retrieve_req(uint32_t smc_fid,
			  bool secure_origin,
			  uint32_t total_length,
			  uint32_t fragment_length,
			  uint64_t address,
			  uint32_t page_count,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	int ret;
	size_t buf_size;
	size_t copy_size;
	struct ffa_mtd_v1_0 *resp;
	const struct ffa_mtd_v1_0 *req;
	struct spmc_shmem_obj *obj = NULL;
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);

	if (!secure_origin) {
		WARN("%s: unsupported retrieve req direction.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (address != 0U || page_count != 0U) {
		WARN("%s: custom memory region not supported.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&mbox->lock);

	req = mbox->tx_buffer;
	resp = mbox->rx_buffer;
	buf_size = mbox->rxtx_page_count * FFA_PAGE_SIZE;

	if (mbox->rxtx_page_count == 0U) {
		WARN("%s: buffer pair not registered.\n", __func__);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	if (mbox->state != MAILBOX_STATE_EMPTY) {
		WARN("%s: RX Buffer is full! %d\n", __func__, mbox->state);
		ret = FFA_ERROR_DENIED;
		goto err_unlock_mailbox;
	}

	if (fragment_length != total_length) {
		WARN("%s: fragmented retrieve request not supported.\n",
		     __func__);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	if (req->emad_count == 0U) {
		WARN("%s: unsupported attribute desc count %u.\n",
		     __func__, obj->desc.emad_count);
		return -EINVAL;
	}

	if (total_length < sizeof(*req)) {
		WARN("%s: invalid length %u < %zu\n", __func__, total_length,
		     sizeof(*req));
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_lookup(&spmc_shmem_obj_state, req->handle);
	if (obj == NULL) {
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (obj->desc_filled != obj->desc_size) {
		WARN("%s: incomplete object desc filled %zu < size %zu\n",
		     __func__, obj->desc_filled, obj->desc_size);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (req->emad_count != 0U && req->sender_id != obj->desc.sender_id) {
		WARN("%s: wrong sender id 0x%x != 0x%x\n",
		     __func__, req->sender_id, obj->desc.sender_id);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (req->emad_count != 0U && req->tag != obj->desc.tag) {
		WARN("%s: wrong tag 0x%lx != 0x%lx\n",
		     __func__, req->tag, obj->desc.tag);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (req->emad_count != 0U && req->emad_count != obj->desc.emad_count) {
		WARN("%s: mistmatch of endpoint counts %u != %u\n",
		     __func__, req->emad_count, obj->desc.emad_count);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (req->flags != 0U) {
		if ((req->flags & FFA_MTD_FLAG_TYPE_MASK) !=
		    (obj->desc.flags & FFA_MTD_FLAG_TYPE_MASK)) {
			/*
			 * If the retrieve request specifies the memory
			 * transaction ensure it matches what we expect.
			 */
			WARN("%s: wrong mem transaction flags %x != %x\n",
			__func__, req->flags, obj->desc.flags);
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock_all;
		}

		if (req->flags != FFA_MTD_FLAG_TYPE_SHARE_MEMORY &&
		    req->flags != FFA_MTD_FLAG_TYPE_LEND_MEMORY) {
			/*
			 * Current implementation does not support donate and
			 * it supports no other flags.
			 */
			WARN("%s: invalid flags 0x%x\n", __func__, req->flags);
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock_all;
		}
	}

	/*
	 * Ensure the emad array lies within the bounds of the descriptor by
	 * checking the address of the element past the end of the array.
	 */
	if ((uintptr_t) &req->emad[req->emad_count] >
	    (uintptr_t)((uint8_t *) &req + total_length)) {
		WARN("Invalid emad access.\n");
		return -EINVAL;
	}

	/*
	 * Validate all the endpoints match in the case of multiple
	 * borrowers. We don't mandate that the order of the borrowers
	 * must match in the descriptors therefore check to see if the
	 * endpoints match in any order.
	 */
	for (size_t i = 0; i < req->emad_count; i++) {
		bool found = false;

		for (size_t j = 0; j < obj->desc.emad_count; j++) {
			if (req->emad[i].mapd.endpoint_id ==
			    obj->desc.emad[j].mapd.endpoint_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			WARN("%s: invalid receiver id (0x%x).\n",
			     __func__, req->emad[i].mapd.endpoint_id);
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock_all;
		}
	}

	mbox->state = MAILBOX_STATE_FULL;

	if (req->emad_count != 0U) {
		obj->in_use++;
	}

	copy_size = MIN(obj->desc_size, buf_size);

	memcpy(resp, &obj->desc, copy_size);

	spin_unlock(&spmc_shmem_obj_state.lock);
	spin_unlock(&mbox->lock);

	SMC_RET8(handle, FFA_MEM_RETRIEVE_RESP, obj->desc_size,
		 copy_size, 0, 0, 0, 0, 0);

err_unlock_all:
	spin_unlock(&spmc_shmem_obj_state.lock);
err_unlock_mailbox:
	spin_unlock(&mbox->lock);
	return spmc_ffa_error_return(handle, ret);
}

/**
 * spmc_ffa_mem_frag_rx - FFA_MEM_FRAG_RX implementation.
 * @client:             Client state.
 * @handle_low:         Handle passed to &FFA_MEM_RETRIEVE_REQ. Bit[31:0].
 * @handle_high:        Handle passed to &FFA_MEM_RETRIEVE_REQ. Bit[63:32].
 * @fragment_offset:    Byte offset in descriptor to resume at.
 * @sender_id:          Bit[31:16]: Endpoint id of sender if client is a
 *                      hypervisor. 0 otherwise.
 * @smc_handle:         Handle passed to smc call. Used to return
 *                      FFA_MEM_FRAG_TX.
 *
 * Return: @smc_handle on success, error code on failure.
 */
long spmc_ffa_mem_frag_rx(uint32_t smc_fid,
			  bool secure_origin,
			  uint32_t handle_low,
			  uint32_t handle_high,
			  uint32_t fragment_offset,
			  uint32_t sender_id,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	int ret;
	void *src;
	size_t buf_size;
	size_t copy_size;
	size_t full_copy_size;
	uint32_t desc_sender_id;
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);
	uint64_t mem_handle = handle_low | (((uint64_t)handle_high) << 32);
	struct spmc_shmem_obj *obj;

	if (!secure_origin) {
		WARN("%s: can only be called from swld.\n",
		     __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_lookup(&spmc_shmem_obj_state, mem_handle);
	if (obj == NULL) {
		WARN("%s: invalid handle, 0x%lx, not a valid handle.\n",
		     __func__, mem_handle);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_shmem;
	}

	desc_sender_id = (uint32_t)obj->desc.sender_id << 16;
	if (sender_id != 0U && sender_id != desc_sender_id) {
		WARN("%s: invalid sender_id 0x%x != 0x%x\n", __func__,
		     sender_id, desc_sender_id);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_shmem;
	}

	if (fragment_offset >= obj->desc_size) {
		WARN("%s: invalid fragment_offset 0x%x >= 0x%zx\n",
		     __func__, fragment_offset, obj->desc_size);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_shmem;
	}

	spin_lock(&mbox->lock);

	if (mbox->rxtx_page_count == 0U) {
		WARN("%s: buffer pair not registered.\n", __func__);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (mbox->state != MAILBOX_STATE_EMPTY) {
		WARN("%s: RX Buffer is full!\n", __func__);
		ret = FFA_ERROR_DENIED;
		goto err_unlock_all;
	}

	buf_size = mbox->rxtx_page_count * FFA_PAGE_SIZE;

	mbox->state = MAILBOX_STATE_FULL;

	full_copy_size = obj->desc_size - fragment_offset;
	copy_size = MIN(full_copy_size, buf_size);

	src = &obj->desc;

	memcpy(mbox->rx_buffer, src + fragment_offset, copy_size);

	spin_unlock(&mbox->lock);
	spin_unlock(&spmc_shmem_obj_state.lock);

	SMC_RET8(handle, FFA_MEM_FRAG_TX, handle_low, handle_high,
		 copy_size, sender_id, 0, 0, 0);

err_unlock_all:
	spin_unlock(&mbox->lock);
err_unlock_shmem:
	spin_unlock(&spmc_shmem_obj_state.lock);
	return spmc_ffa_error_return(handle, ret);
}

/**
 * spmc_ffa_mem_relinquish - FFA_MEM_RELINQUISH implementation.
 * @client:             Client state.
 *
 * Implements a subset of the FF-A FFA_MEM_RELINQUISH call.
 * Used by secure os release previously shared memory to non-secure os.
 *
 * The handle to release must be in the client's (secure os's) transmit buffer.
 *
 * Return: 0 on success, error code on failure.
 */
int spmc_ffa_mem_relinquish(uint32_t smc_fid,
			    bool secure_origin,
			    uint32_t handle_low,
			    uint32_t handle_high,
			    uint32_t fragment_offset,
			    uint32_t sender_id,
			    void *cookie,
			    void *handle,
			    uint64_t flags)
{
	int ret;
	struct mailbox *mbox = spmc_get_mbox_desc(secure_origin);
	struct spmc_shmem_obj *obj;
	const struct ffa_mem_relinquish_descriptor *req;

	if (!secure_origin) {
		WARN("%s: unsupported relinquish direction.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&mbox->lock);

	if (mbox->rxtx_page_count == 0U) {
		WARN("%s: buffer pair not registered.\n", __func__);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	req = mbox->tx_buffer;

	if (req->flags != 0U) {
		WARN("%s: unsupported flags 0x%x\n", __func__, req->flags);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	if (req->endpoint_count == 0) {
		WARN("%s: endpoint count cannot be 0.\n", __func__);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_mailbox;
	}

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_lookup(&spmc_shmem_obj_state, req->handle);
	if (obj == NULL) {
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	if (obj->desc.emad_count != req->endpoint_count) {
		WARN("%s: mismatch of endpoint count %u != %u\n", __func__,
		     obj->desc.emad_count, req->endpoint_count);
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}

	/* Validate requested endpoint IDs match descriptor. */
	for (size_t i = 0; i < req->endpoint_count; i++) {
		bool found = false;

		for (unsigned int j = 0; j < obj->desc.emad_count; j++) {
			if (req->endpoint_array[i] ==
			    obj->desc.emad[j].mapd.endpoint_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			WARN("%s: Invalid endpoint ID (0x%x).\n",
			     __func__, req->endpoint_array[i]);
			ret = FFA_ERROR_INVALID_PARAMETER;
			goto err_unlock_all;
		}
	}

	if (obj->in_use == 0U) {
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock_all;
	}
	obj->in_use--;

	spin_unlock(&spmc_shmem_obj_state.lock);
	spin_unlock(&mbox->lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);

err_unlock_all:
	spin_unlock(&spmc_shmem_obj_state.lock);
err_unlock_mailbox:
	spin_unlock(&mbox->lock);
	return spmc_ffa_error_return(handle, ret);
}

/**
 * spmc_ffa_mem_reclaim - FFA_MEM_RECLAIM implementation.
 * @client:         Client state.
 * @handle_low:     Unique handle of shared memory object to reclaim. Bit[31:0].
 * @handle_high:    Unique handle of shared memory object to reclaim.
 *                  Bit[63:32].
 * @flags:          Unsupported, ignored.
 *
 * Implements a subset of the FF-A FFA_MEM_RECLAIM call.
 * Used by non-secure os reclaim memory previously shared with secure os.
 *
 * Return: 0 on success, error code on failure.
 */
int spmc_ffa_mem_reclaim(uint32_t smc_fid,
			 bool secure_origin,
			 uint32_t handle_low,
			 uint32_t handle_high,
			 uint32_t mem_flags,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	int ret;
	struct spmc_shmem_obj *obj;
	uint64_t mem_handle = handle_low | (((uint64_t)handle_high) << 32);

	if (secure_origin) {
		WARN("%s: unsupported reclaim direction.\n", __func__);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	if (mem_flags != 0U) {
		WARN("%s: unsupported flags 0x%x\n", __func__, mem_flags);
		return spmc_ffa_error_return(handle,
					     FFA_ERROR_INVALID_PARAMETER);
	}

	spin_lock(&spmc_shmem_obj_state.lock);

	obj = spmc_shmem_obj_lookup(&spmc_shmem_obj_state, mem_handle);
	if (obj == NULL) {
		ret = FFA_ERROR_INVALID_PARAMETER;
		goto err_unlock;
	}
	if (obj->in_use != 0U) {
		ret = FFA_ERROR_DENIED;
		goto err_unlock;
	}
	spmc_shmem_obj_free(&spmc_shmem_obj_state, obj);
	spin_unlock(&spmc_shmem_obj_state.lock);

	SMC_RET1(handle, FFA_SUCCESS_SMC32);

err_unlock:
	spin_unlock(&spmc_shmem_obj_state.lock);
	return spmc_ffa_error_return(handle, ret);
}

/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <common/debug.h>
#include <drivers/measured_boot/event_log/event_handoff.h>

#include <platform_def.h>

static uint8_t *get_log_ptr(struct transfer_list_entry *te, size_t offset)
{
	uint8_t *base_ptr = transfer_list_entry_data(te);

	if (base_ptr == NULL) {
		return NULL;
	}

	return base_ptr + offset;
}

uint8_t *transfer_list_event_log_extend(struct transfer_list_header *tl,
					size_t req_size, size_t *free)
{
	struct transfer_list_entry *existing_entry;
	struct transfer_list_entry *new_entry;
	uint8_t *old_data;
	size_t existing_offset;
	size_t old_size;

	if (tl == NULL || free == NULL || req_size == 0) {
		ERROR("Invalid arguments to event log extend.\n");
		return NULL;
	}

	existing_entry = transfer_list_find(tl, TL_TAG_TPM_EVLOG);
	existing_offset = EVENT_LOG_RESERVED_BYTES;

	if (existing_entry != NULL) {
		existing_offset = existing_entry->data_size;

		if (transfer_list_set_data_size(tl, existing_entry,
						req_size + existing_offset)) {
			VERBOSE("TPM event log entry resized: new space %zu bytes at offset %zu\n",
				req_size, existing_offset);

			*free = existing_entry->data_size - existing_offset;

			return get_log_ptr(existing_entry, existing_offset);
		}
	}

	/* Add new entry (resize failed or no existing entry) */
	new_entry = transfer_list_add(tl, TL_TAG_TPM_EVLOG,
				      req_size + existing_offset, NULL);

	if (new_entry == NULL) {
		ERROR("Failed to add TPM event log entry to transfer list.\n");
		return NULL;
	}

	VERBOSE("New TPM event log entry added at %p\n",
		transfer_list_entry_data(new_entry));

	if (existing_entry != NULL) {
		old_data = transfer_list_entry_data(existing_entry);
		old_size = existing_offset;

		VERBOSE("Copying existing event log (%zu bytes) to new entry at %p\n",
			old_size, transfer_list_entry_data(new_entry));

		memmove(transfer_list_entry_data(new_entry), old_data,
			old_size);

		transfer_list_rem(tl, existing_entry);
	}

	*free = new_entry->data_size - existing_offset;

	return get_log_ptr(new_entry, existing_offset);
}

uint8_t *transfer_list_event_log_finish(struct transfer_list_header *tl,
					uintptr_t cursor)
{
	uintptr_t entry_data_base;
	size_t final_log_size;
	struct transfer_list_entry *entry;

	entry = transfer_list_find(tl, TL_TAG_TPM_EVLOG);
	entry_data_base = (uintptr_t)transfer_list_entry_data(entry);

	if (cursor < entry_data_base ||
	    cursor >= entry_data_base + entry->data_size) {
		ERROR("Invalid cursor: outside event log bounds.\n");
		return NULL;
	}

	final_log_size = cursor - entry_data_base;

	if (!transfer_list_set_data_size(tl, entry, final_log_size)) {
		ERROR("Unable to resize event log TE.\n");
		return NULL;
	}

	transfer_list_update_checksum(tl);

	VERBOSE("TPM event log finalized: trimmed to %zu bytes",
		final_log_size - EVENT_LOG_RESERVED_BYTES);

	/* Ensure changes are visible to the next stage. */
	flush_dcache_range((uintptr_t)tl, tl->size);

	return get_log_ptr(entry, EVENT_LOG_RESERVED_BYTES);
}

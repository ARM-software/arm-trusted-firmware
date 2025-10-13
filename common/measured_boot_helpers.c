/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/measured_boot.h>

const event_log_metadata_t *
mboot_find_event_log_metadata(const event_log_metadata_t *metadata_table,
			      unsigned int image_id)
{
	const event_log_metadata_t *metadata_ptr;

	assert(metadata_table != NULL);

	metadata_ptr = metadata_table;

	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
	       (metadata_ptr->id != image_id)) {
		metadata_ptr++;
	}

	if (metadata_ptr->id == EVLOG_INVALID_ID) {
		return NULL;
	}

	return metadata_ptr;
}

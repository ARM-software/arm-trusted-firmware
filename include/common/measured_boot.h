/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMMON_MEASURED_BOOT_H
#define COMMON_MEASURED_BOOT_H

#include <stddef.h>

#include <event_measure.h>

const event_log_metadata_t *
mboot_find_event_log_metadata(const event_log_metadata_t *metadata_table,
			      unsigned int image_id);

#endif /* COMMON_MEASURED_BOOT_H */

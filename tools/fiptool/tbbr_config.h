/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_CONFIG_H
#define TBBR_CONFIG_H

#include <stdint.h>

#include <uuid.h>

/* TODO: Update this number as required */
#define TOC_HEADER_SERIAL_NUMBER 0x12345678

typedef struct toc_entry {
	char         *name;
	uuid_t        uuid;
	char         *cmdline_name;
} toc_entry_t;

extern toc_entry_t toc_entries[];

#endif /* TBBR_CONFIG_H */

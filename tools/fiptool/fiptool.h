/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FIPTOOL_H__
#define __FIPTOOL_H__

#include <stddef.h>
#include <stdint.h>

#include <firmware_image_package.h>
#include <uuid.h>

#include "fiptool_platform.h"

#define NELEM(x) (sizeof (x) / sizeof *(x))

enum {
	DO_UNSPEC = 0,
	DO_PACK   = 1,
	DO_UNPACK = 2,
	DO_REMOVE = 3
};

enum {
	LOG_DBG,
	LOG_WARN,
	LOG_ERR
};

typedef struct image_desc {
	uuid_t             uuid;
	char              *name;
	char              *cmdline_name;
	int                action;
	char              *action_arg;
	struct image      *image;
	struct image_desc *next;
} image_desc_t;

typedef struct image {
	struct fip_toc_entry toc_e;
	void                *buffer;
} image_t;

typedef struct cmd {
	char              *name;
	int              (*handler)(int, char **);
	void             (*usage)(void);
} cmd_t;

#endif /* __FIPTOOL_H__ */

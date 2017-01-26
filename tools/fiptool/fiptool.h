/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __FIPTOOL_H__
#define __FIPTOOL_H__

#include <stddef.h>
#include <stdint.h>

#include "uuid.h"

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
	struct image_desc *next;
} image_desc_t;

typedef struct image {
	uuid_t             uuid;
	size_t             size;
	/* offset of this image in FIP (only useful for info command) */
	size_t             offset;
	void              *buffer;
	struct image      *next;
} image_t;

typedef struct cmd {
	char              *name;
	int              (*handler)(int, char **);
	void             (*usage)(void);
} cmd_t;

#endif /* __FIPTOOL_H__ */

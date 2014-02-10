/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __IO_H__
#define __IO_H__

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <stdio.h>	/* For ssize_t */


/* Device type which can be used to enable policy decisions about which device
 * to access */
typedef enum {
	IO_TYPE_INVALID,
	IO_TYPE_SEMIHOSTING,
	IO_TYPE_BLOCK,
	IO_TYPE_MAX
} io_type;


/* Modes used when seeking data on a supported device */
typedef enum {
	IO_SEEK_INVALID,
	IO_SEEK_SET,
	IO_SEEK_END,
	IO_SEEK_CUR,
	IO_SEEK_MAX
} io_seek_mode;


/* Connector type, providing a means of identifying a device to open */
struct io_dev_connector;

/* Device handle, providing a client with access to a specific device */
typedef struct io_dev_info *io_dev_handle;

/* IO handle, providing a client with access to a specific source of data from
 * a device */
typedef struct io_entity *io_handle;


/* File specification - used to refer to data on a device supporting file-like
 * entities */
typedef struct {
	const char *path;
	unsigned int mode;
} io_file_spec;


/* Block specification - used to refer to data on a device supporting
 * block-like entities */
typedef struct {
	unsigned long offset;
	size_t length;
} io_block_spec;


/* Access modes used when accessing data on a device */
#define IO_MODE_INVALID (0)
#define IO_MODE_RO	(1 << 0)
#define IO_MODE_RW	(1 << 1)


/* Return codes reported by 'io_*' APIs */
#define IO_SUCCESS		(0)
#define IO_FAIL			(-1)
#define IO_NOT_SUPPORTED	(-2)
#define IO_RESOURCES_EXHAUSTED	(-3)


/* Open a connection to a device */
int io_dev_open(struct io_dev_connector *dev_con, void *dev_spec,
		io_dev_handle *dev_handle);


/* Initialise a device explicitly - to permit lazy initialisation or
 * re-initialisation */
int io_dev_init(io_dev_handle dev_handle, const void *init_params);

/* TODO: Consider whether an explicit "shutdown" API should be included */

/* Close a connection to a device */
int io_dev_close(io_dev_handle dev_handle);


/* Synchronous operations */
int io_open(io_dev_handle dev_handle, const void *spec, io_handle *handle);

int io_seek(io_handle handle, io_seek_mode mode, ssize_t offset);

int io_size(io_handle handle, size_t *length);

int io_read(io_handle handle, void *buffer, size_t length, size_t *length_read);

int io_write(io_handle handle, const void *buffer, size_t length,
		size_t *length_written);

int io_close(io_handle handle);


#endif /* __ASSEMBLY__ */
#endif /* __IO_H__ */

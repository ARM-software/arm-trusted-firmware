/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_STORAGE_H
#define IO_STORAGE_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h> /* For ssize_t */

#include <tools_share/uuid.h>

/* Device type which can be used to enable policy decisions about which device
 * to access */
typedef enum {
	IO_TYPE_INVALID,
	IO_TYPE_SEMIHOSTING,
	IO_TYPE_MEMMAP,
	IO_TYPE_DUMMY,
	IO_TYPE_FIRMWARE_IMAGE_PACKAGE,
	IO_TYPE_BLOCK,
	IO_TYPE_MTD,
	IO_TYPE_MMC,
	IO_TYPE_STM32IMAGE,
	IO_TYPE_ENCRYPTED,
	IO_TYPE_MAX
} io_type_t;


/* Modes used when seeking data on a supported device */
typedef enum {
	IO_SEEK_INVALID,
	IO_SEEK_SET,
	IO_SEEK_END,
	IO_SEEK_CUR,
	IO_SEEK_MAX
} io_seek_mode_t;


/* Connector type, providing a means of identifying a device to open */
struct io_dev_connector;


/* File specification - used to refer to data on a device supporting file-like
 * entities */
typedef struct io_file_spec {
	const char *path;
	unsigned int mode;
} io_file_spec_t;

/* UUID specification - used to refer to data accessed using UUIDs (i.e. FIP
 * images) */
typedef struct io_uuid_spec {
	uuid_t uuid;
} io_uuid_spec_t;

/* Block specification - used to refer to data on a device supporting
 * block-like entities */
typedef struct io_block_spec {
	size_t offset;
	size_t length;
} io_block_spec_t;


/* Access modes used when accessing data on a device */
#define IO_MODE_INVALID (0)
#define IO_MODE_RO	(1 << 0)
#define IO_MODE_RW	(1 << 1)


/* Open a connection to a device */
int io_dev_open(const struct io_dev_connector *dev_con,
		const uintptr_t dev_spec,
		uintptr_t *handle);


/* Initialise a device explicitly - to permit lazy initialisation or
 * re-initialisation */
int io_dev_init(uintptr_t dev_handle, const uintptr_t init_params);

/* Close a connection to a device */
int io_dev_close(uintptr_t dev_handle);


/* Synchronous operations */
int io_open(uintptr_t dev_handle, const uintptr_t spec, uintptr_t *handle);

int io_seek(uintptr_t handle, io_seek_mode_t mode, signed long long offset);

int io_size(uintptr_t handle, size_t *length);

int io_read(uintptr_t handle, uintptr_t buffer, size_t length,
		size_t *length_read);

int io_write(uintptr_t handle, const uintptr_t buffer, size_t length,
		size_t *length_written);

int io_close(uintptr_t handle);


#endif /* IO_STORAGE_H */

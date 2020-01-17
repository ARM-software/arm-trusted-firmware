/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stdint.h>

#include <drivers/io/io_storage.h>

/* Generic IO entity structure,representing an accessible IO construct on the
 * device, such as a file */
typedef struct io_entity {
	struct io_dev_info *dev_handle;
	uintptr_t info;
} io_entity_t;


/* Device info structure, providing device-specific functions and a means of
 * adding driver-specific state */
typedef struct io_dev_info {
	const struct io_dev_funcs *funcs;
	uintptr_t info;
} io_dev_info_t;


/* Structure used to create a connection to a type of device */
typedef struct io_dev_connector {
	/* dev_open opens a connection to a particular device driver */
	int (*dev_open)(const uintptr_t dev_spec, io_dev_info_t **dev_info);
} io_dev_connector_t;


/* Structure to hold device driver function pointers */
typedef struct io_dev_funcs {
	io_type_t (*type)(void);
	int (*open)(io_dev_info_t *dev_info, const uintptr_t spec,
			io_entity_t *entity);
	int (*seek)(io_entity_t *entity, int mode, signed long long offset);
	int (*size)(io_entity_t *entity, size_t *length);
	int (*read)(io_entity_t *entity, uintptr_t buffer, size_t length,
			size_t *length_read);
	int (*write)(io_entity_t *entity, const uintptr_t buffer,
			size_t length, size_t *length_written);
	int (*close)(io_entity_t *entity);
	int (*dev_init)(io_dev_info_t *dev_info, const uintptr_t init_params);
	int (*dev_close)(io_dev_info_t *dev_info);
} io_dev_funcs_t;


/* Operations intended to be performed during platform initialisation */

/* Register an IO device */
int io_register_device(const io_dev_info_t *dev_info);

#endif /* IO_DRIVER_H */

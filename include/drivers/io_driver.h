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

#ifndef __IO_DRIVER_H__
#define __IO_DRIVER_H__

#include <io_storage.h>
#include <platform_def.h> /* For MAX_IO_DEVICES */
#include <stdint.h>


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
	int (*seek)(io_entity_t *entity, int mode, ssize_t offset);
	int (*size)(io_entity_t *entity, size_t *length);
	int (*read)(io_entity_t *entity, uintptr_t buffer, size_t length,
			size_t *length_read);
	int (*write)(io_entity_t *entity, const uintptr_t buffer,
			size_t length, size_t *length_written);
	int (*close)(io_entity_t *entity);
	int (*dev_init)(io_dev_info_t *dev_info, const uintptr_t init_params);
	int (*dev_close)(io_dev_info_t *dev_info);
} io_dev_funcs_t;


/* IO platform data - used to track devices registered for a specific
 * platform */
typedef struct io_plat_data {
	const io_dev_info_t *devices[MAX_IO_DEVICES];
	unsigned int dev_count;
} io_plat_data_t;


/* Operations intended to be performed during platform initialisation */

/* Initialise the IO layer */
void io_init(io_plat_data_t *data);

/* Register a device driver */
int io_register_device(const io_dev_info_t *dev_info);

#endif  /* __IO_DRIVER_H__ */

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

#include <platform.h>   /* For MAX_IO_DEVICES */


/* Generic IO entity structure,representing an accessible IO construct on the
 * device, such as a file */
struct io_entity {
	io_dev_handle dev_handle;
	uintptr_t info;
};


/* Device info structure, providing device-specific functions and a means of
 * adding driver-specific state */
struct io_dev_info {
	struct io_dev_funcs *funcs;
	uintptr_t info;
};


/* Structure used to create a connection to a type of device */
struct io_dev_connector {
	/* dev_open opens a connection to a particular device driver */
	int (*dev_open)(void *spec, struct io_dev_info **dev_info);
};


/* Structure to hold device driver function pointers */
struct io_dev_funcs {
	io_type (*type)(void);
	int (*open)(struct io_dev_info *dev_info, const void *spec,
			struct io_entity *entity);
	int (*seek)(struct io_entity *entity, int mode, ssize_t offset);
	int (*size)(struct io_entity *entity, size_t *length);
	int (*read)(struct io_entity *entity, void *buffer, size_t length,
			size_t *length_read);
	int (*write)(struct io_entity *entity, const void *buffer,
			size_t length, size_t *length_written);
	int (*close)(struct io_entity *entity);
	int (*dev_init)(struct io_dev_info *dev_info, const void *init_params);
	int (*dev_close)(struct io_dev_info *dev_info);
};


/* IO platform data - used to track devices registered for a specific
 * platform */
struct io_plat_data {
	struct io_dev_info *devices[MAX_IO_DEVICES];
	unsigned int dev_count;
};


/* Operations intended to be performed during platform initialisation */

/* Initialise the IO layer */
void io_init(struct io_plat_data *data);

/* Register a device driver */
int io_register_device(struct io_dev_info *dev_info);

#endif  /* __IO_DRIVER_H__ */

/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_H
#define FCONF_H

#include <stdint.h>

/* Public API */
#define FCONF_GET_PROPERTY(a, b, c)	a##__##b##_getter(c)

/*
 * This macro takes three arguments:
 *   config:	Configuration identifier
 *   name:	property namespace
 *   callback:	populate() function
 */
#define FCONF_REGISTER_POPULATOR(config, name, callback)			\
	__attribute__((used, section(".fconf_populator")))			\
	const struct fconf_populator (name##__populator) = {			\
		.config_type = (#config),					\
		.info = (#name),						\
		.populate = (callback)						\
	};

/*
 * Populator callback
 *
 * This structure are used by the fconf_populate function and should only be
 * defined by the FCONF_REGISTER_POPULATOR macro.
 */
struct fconf_populator {
	/* Description of the data loaded by the callback */
	const char *config_type;
	const char *info;

	/* Callback used by fconf_populate function with a provided config dtb.
	 * Return 0 on success, err_code < 0 otherwise.
	 */
	int (*populate)(uintptr_t config);
};

/* This function supports to load tb_fw_config and fw_config dtb */
int fconf_load_config(unsigned int image_id);

/* Top level populate function
 *
 * This function takes a configuration dtb and calls all the registered
 * populator callback with it.
 *
 *  Panic on error.
 */
void fconf_populate(const char *config_type, uintptr_t config);

/* FCONF specific getter */
#define fconf__dtb_getter(prop)	fconf_dtb_info.prop

/* Structure used to locally keep a reference to the config dtb. */
struct fconf_dtb_info_t {
	uintptr_t base_addr;
	size_t size;
};

extern struct fconf_dtb_info_t fconf_dtb_info;

#endif /* FCONF_H */

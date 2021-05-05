/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_MPMM_GETTER_H
#define FCONF_MPMM_GETTER_H

#include <lib/mpmm/mpmm.h>

#define mpmm__config_getter(id)	fconf_mpmm_config.id

struct fconf_mpmm_config {
	const struct mpmm_topology *topology;
};

extern struct fconf_mpmm_config fconf_mpmm_config;

#endif /* FCONF_MPMM_GETTER_H */

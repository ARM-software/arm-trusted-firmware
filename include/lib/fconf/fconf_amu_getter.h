/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_AMU_GETTER_H
#define FCONF_AMU_GETTER_H

#include <lib/extensions/amu.h>

#define amu__config_getter(id)	fconf_amu_config.id

struct fconf_amu_config {
	const struct amu_topology *topology;
};

extern struct fconf_amu_config fconf_amu_config;

#endif /* FCONF_AMU_GETTER_H */

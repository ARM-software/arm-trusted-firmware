/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_ARM_SP_GETTER_H
#define FCONF_ARM_SP_GETTER_H

#include <lib/fconf/fconf.h>
#include <tools_share/uuid.h>

/* arm_sp getter */
#define arm__sp_getter(prop)	arm_sp.prop

struct arm_sp_t {
	unsigned int		number_of_sp;
	union uuid_helper_t	uuids[MAX_SP_IDS];
	uintptr_t		load_addr[MAX_SP_IDS];
};

int fconf_populate_arm_sp(uintptr_t config);

extern struct arm_sp_t arm_sp;

#endif /* FCONF_ARM_SP_GETTER_H */

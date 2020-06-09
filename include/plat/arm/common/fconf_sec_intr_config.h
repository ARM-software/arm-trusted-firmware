/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_SEC_INTR_CONFIG_H
#define FCONF_SEC_INTR_CONFIG_H

#include <lib/fconf/fconf.h>

#include <platform_def.h>

#define hw_config__sec_intr_prop_getter(id)	sec_intr_prop.id

#define SEC_INT_COUNT_MAX U(15)

struct sec_intr_prop_t {
	interrupt_prop_t descriptor[SEC_INT_COUNT_MAX];
	uint32_t count;
};

int fconf_populate_sec_intr_config(uintptr_t config);

extern struct sec_intr_prop_t sec_intr_prop;

#endif /* FCONF_SEC_INTR_CONFIG_H */

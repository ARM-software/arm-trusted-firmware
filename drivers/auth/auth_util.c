/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <drivers/auth/auth_util.h>

static const char *current_pk_oid;

const char *get_current_pk_oid(void)
{
	return current_pk_oid;
}

void set_current_pk_oid(const char *pk_oid)
{
	current_pk_oid = pk_oid;
}

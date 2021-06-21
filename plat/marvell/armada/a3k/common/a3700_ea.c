/*
 * Copyright (C) 2019 Repk repk@triplefau.lt
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <common/bl_common.h>
#include <common/debug.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>

#define ADVK_SERROR_SYNDROME 0xbf000002

void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
	if (syndrome == ADVK_SERROR_SYNDROME)
		return;

	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
}

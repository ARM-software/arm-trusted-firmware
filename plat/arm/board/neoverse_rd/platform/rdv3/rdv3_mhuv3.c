/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>

#include <nrd_css_def3.h>
#include <nrd_plat.h>
#include <rdv3_mhuv3.h>

void mhu_v3_get_secure_device_base(uintptr_t *base, bool sender)
{
	if (sender) {
		*base = AP_RSE_ROOT_MHU_V3_PBX;
	} else {
		*base = AP_RSE_ROOT_MHU_V3_MBX;
	}
}

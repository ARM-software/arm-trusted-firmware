/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_params.h>
#include <thunder_private.h>

void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *bl31_param;
	struct bl31_fdt_param *fdt_param;
#if COREBOOT
	struct bl31_u64_param *u64_param;
#endif

	/* keep plat parameters for later processing if need */
	bl31_param = (struct bl31_plat_param *)plat_param_from_bl2;

	while (bl31_param) {
		switch (bl31_param->type) {
		case PARAM_FDT:
			fdt_param = (struct bl31_fdt_param *)bl31_param;
			fdt_ptr = (void *)fdt_param->fdt_ptr;
			break;
#if COREBOOT
		case PARAM_COREBOOT_TABLE:
			u64_param = (struct bl31_u64_param *)bl31_param;
			coreboot_table_setup((void *)u64_param->value);
			break;
#endif
		default:
			ERROR("not expected type found %lld\n", bl31_param->type);
			break;
		}
		bl31_param = bl31_param->next;
	}
}

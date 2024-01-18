/*
 * Copyright (c) 2024, Pengutronix, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

#include <stdint.h>
#include <common/bl_common.h>

int imx_bl31_params_parse(uintptr_t arg0, uintptr_t ocram_base,
			  uintptr_t ocram_size,
			  entry_point_info_t *bl32_info,
			  entry_point_info_t *bl33_info);

#endif /* PLAT_COMMON_H */

/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef LIB_BL_AUX_PARAMS_H
#define LIB_BL_AUX_PARAMS_H

#include <stdbool.h>
#include <stdint.h>

#include <export/lib/bl_aux_params/bl_aux_params_exp.h>

/*
 * Handler function that handles an individual aux parameter. Return true if
 * the parameter was handled, and flase if bl_aux_params_parse() should make its
 * own attempt at handling it (for generic parameters).
 */
typedef bool (*bl_aux_param_handler_t)(struct bl_aux_param_header *param);

/*
 * Interprets head as the start of an aux parameter list, and passes the
 * parameters individually to handler(). Handles generic parameters directly if
 * handler() hasn't already done so. If only generic parameters are expected,
 * handler() can be NULL.
 */
void bl_aux_params_parse(u_register_t head,
			 bl_aux_param_handler_t handler);

#endif /* LIB_BL_AUX_PARAMS_H */

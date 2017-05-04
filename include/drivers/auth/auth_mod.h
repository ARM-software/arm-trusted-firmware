/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUTH_MOD_H__
#define __AUTH_MOD_H__

#if TRUSTED_BOARD_BOOT

#include <auth_common.h>
#include <cot_def.h>
#include <img_parser_mod.h>

/*
 * Image flags
 */
#define IMG_FLAG_AUTHENTICATED		(1 << 0)


/*
 * Authentication image descriptor
 */
typedef struct auth_img_desc_s {
	unsigned int img_id;
	img_type_t img_type;
	const struct auth_img_desc_s *parent;
	auth_method_desc_t img_auth_methods[AUTH_METHOD_NUM];
	auth_param_desc_t authenticated_data[COT_MAX_VERIFIED_PARAMS];
} auth_img_desc_t;

/* Public functions */
void auth_mod_init(void);
int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id);
int auth_mod_verify_img(unsigned int img_id,
			void *img_ptr,
			unsigned int img_len);

/* Macro to register a CoT defined as an array of auth_img_desc_t */
#define REGISTER_COT(_cot) \
	const auth_img_desc_t *const cot_desc_ptr = \
			(const auth_img_desc_t *const)&_cot[0]; \
	unsigned int auth_img_flags[sizeof(_cot)/sizeof(_cot[0])]

#endif /* TRUSTED_BOARD_BOOT */

#endif /* __AUTH_MOD_H__ */

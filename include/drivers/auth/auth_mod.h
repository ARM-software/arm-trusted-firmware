/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AUTH_MOD_H
#define AUTH_MOD_H

#if TRUSTED_BOARD_BOOT

#include <common/tbbr/cot_def.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/auth/auth_common.h>
#include <drivers/auth/img_parser_mod.h>

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
	const auth_method_desc_t *const img_auth_methods;
	const auth_param_desc_t *const authenticated_data;
} auth_img_desc_t;

/* Public functions */
void auth_mod_init(void);
int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id);
int auth_mod_verify_img(unsigned int img_id,
			void *img_ptr,
			unsigned int img_len);

/* Macro to register a CoT defined as an array of auth_img_desc_t pointers */
#define REGISTER_COT(_cot) \
	const auth_img_desc_t *const *const cot_desc_ptr = (_cot); \
	unsigned int auth_img_flags[MAX_NUMBER_IDS]

extern const auth_img_desc_t *const *const cot_desc_ptr;
extern unsigned int auth_img_flags[MAX_NUMBER_IDS];

#endif /* TRUSTED_BOARD_BOOT */

#endif /* AUTH_MOD_H */

/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AUTH_MOD_H
#define AUTH_MOD_H

#include <common/tbbr/cot_def.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/auth/auth_common.h>
#include <drivers/auth/img_parser_mod.h>

#include <lib/utils_def.h>

/*
 * Image flags
 */
#define IMG_FLAG_AUTHENTICATED		(1 << 0)

#if COT_DESC_IN_DTB && !IMAGE_BL1
/*
 * Authentication image descriptor
 */
typedef struct auth_img_desc_s {
	unsigned int img_id;
	img_type_t img_type;
	const struct auth_img_desc_s *parent;
	auth_method_desc_t *img_auth_methods;
	auth_param_desc_t *authenticated_data;
} auth_img_desc_t;
#else
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
#endif /* COT_DESC_IN_DTB && !IMAGE_BL1 */

/* Public functions */
#if TRUSTED_BOARD_BOOT
void auth_mod_init(void);
#else
static inline void auth_mod_init(void)
{
}
#endif /* TRUSTED_BOARD_BOOT */
int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id);
int auth_mod_verify_img(unsigned int img_id,
			void *img_ptr,
			unsigned int img_len);

/* Macro to register a CoT defined as an array of auth_img_desc_t pointers */
#define REGISTER_COT(_cot) \
	const auth_img_desc_t *const *const cot_desc_ptr = (_cot); \
	const size_t cot_desc_size = ARRAY_SIZE(_cot);		   \
	unsigned int auth_img_flags[MAX_NUMBER_IDS]

extern const auth_img_desc_t *const *const cot_desc_ptr;
extern const size_t cot_desc_size;
extern unsigned int auth_img_flags[MAX_NUMBER_IDS];

#if defined(SPD_spmd)

#define DEFINE_SIP_SP_PKG(n)		DEFINE_SP_PKG(n, sip_sp_content_cert)
#define DEFINE_PLAT_SP_PKG(n)		DEFINE_SP_PKG(n, plat_sp_content_cert)

#define DEFINE_SP_PKG(n, cert) \
	static const auth_img_desc_t sp_pkg##n = { \
		.img_id = SP_PKG##n##_ID, \
		.img_type = IMG_RAW, \
		.parent = &cert, \
		.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) { \
			[0] = { \
				.type = AUTH_METHOD_HASH, \
				.param.hash = { \
					.data = &raw_data, \
					.hash = &sp_pkg##n##_hash \
				} \
			} \
		} \
	}

#endif

#endif /* AUTH_MOD_H */

/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

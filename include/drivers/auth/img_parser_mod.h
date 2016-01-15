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

#ifndef __IMG_PARSER_MOD_H__
#define __IMG_PARSER_MOD_H__

#include <auth_common.h>

/*
 * Return values
 */
enum img_parser_ret_value {
	IMG_PARSER_OK,
	IMG_PARSER_ERR,			/* Parser internal error */
	IMG_PARSER_ERR_FORMAT,		/* Malformed image */
	IMG_PARSER_ERR_NOT_FOUND	/* Authentication data not found */
};

/*
 * Image types. A parser should be instantiated and registered for each type
 */
typedef enum img_type_enum {
	IMG_RAW,			/* Binary image */
	IMG_PLAT,			/* Platform specific format */
	IMG_CERT,			/* X509v3 certificate */
	IMG_MAX_TYPES,
} img_type_t;

/* Image parser library structure */
typedef struct img_parser_lib_desc_s {
	img_type_t img_type;
	const char *name;

	void (*init)(void);
	int (*check_integrity)(void *img, unsigned int img_len);
	int (*get_auth_param)(const auth_param_type_desc_t *type_desc,
			void *img, unsigned int img_len,
			void **param, unsigned int *param_len);
} img_parser_lib_desc_t;

/* Exported functions */
void img_parser_init(void);
int img_parser_check_integrity(img_type_t img_type,
		void *img, unsigned int img_len);
int img_parser_get_auth_param(img_type_t img_type,
		const auth_param_type_desc_t *type_desc,
		void *img, unsigned int img_len,
		void **param_ptr, unsigned int *param_len);

/* Macro to register an image parser library */
#define REGISTER_IMG_PARSER_LIB(_type, _name, _init, _check_int, _get_param) \
	static const img_parser_lib_desc_t __img_parser_lib_desc_##_type \
	__section(".img_parser_lib_descs") __used = { \
		.img_type = _type, \
		.name = _name, \
		.init = _init, \
		.check_integrity = _check_int, \
		.get_auth_param = _get_param \
	}

#endif /* __IMG_PARSER_MOD_H__ */

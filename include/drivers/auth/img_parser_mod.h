/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

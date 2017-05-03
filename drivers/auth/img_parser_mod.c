/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <auth_common.h>
#include <debug.h>
#include <errno.h>
#include <img_parser_mod.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

extern uintptr_t __PARSER_LIB_DESCS_START__;
extern uintptr_t __PARSER_LIB_DESCS_END__;
#define PARSER_LIB_DESCS_START	((uintptr_t) (&__PARSER_LIB_DESCS_START__))
#define PARSER_LIB_DESCS_END	((uintptr_t) (&__PARSER_LIB_DESCS_END__))
static unsigned int parser_lib_indices[IMG_MAX_TYPES];
static img_parser_lib_desc_t *parser_lib_descs;

#define INVALID_IDX		UINT_MAX

static void validate_desc(img_parser_lib_desc_t *desc)
{
	assert(desc != NULL);
	assert(desc->init != NULL);
	assert(desc->name != NULL);
	assert(desc->check_integrity != NULL);
	assert(desc->get_auth_param != NULL);
}

void img_parser_init(void)
{
	unsigned int index, mod_num;

	/* Initialise internal variables to invalid state */
	for (index = 0; index < IMG_MAX_TYPES; index++) {
		parser_lib_indices[index] = INVALID_IDX;
	}

	/* Calculate how many image parsers are registered. At least one parser
	 * must be present */
	mod_num = PARSER_LIB_DESCS_END - PARSER_LIB_DESCS_START;
	mod_num /= sizeof(img_parser_lib_desc_t);
	assert(mod_num > 0);

	parser_lib_descs = (img_parser_lib_desc_t *) PARSER_LIB_DESCS_START;
	for (index = 0; index < mod_num; index++) {

		/* Check that the image parser library descriptor is valid */
		validate_desc(&parser_lib_descs[index]);

		/* Initialize image parser */
		parser_lib_descs[index].init();

		/* Ensure only one parser is registered for each image type */
		assert(parser_lib_indices[parser_lib_descs[index].img_type] ==
				INVALID_IDX);

		/* Keep the index of this hash calculator */
		parser_lib_indices[parser_lib_descs[index].img_type] = index;
	}
}

int img_parser_check_integrity(img_type_t img_type,
			       void *img_ptr, unsigned int img_len)
{
	unsigned int idx;

	assert(img_ptr != NULL);
	assert(img_len != 0);

	/* No integrity checks on raw images */
	if (img_type == IMG_RAW) {
		return IMG_PARSER_OK;
	}

	/* Find the index of the required image parser */
	idx = parser_lib_indices[img_type];
	assert(idx != INVALID_IDX);

	/* Call the function to check the image integrity */
	return parser_lib_descs[idx].check_integrity(img_ptr, img_len);
}

/*
 * Extract an authentication parameter from an image
 *
 * Parameters:
 *   img_type: image type (certificate, raw image, etc)
 *   type_desc: provides info to obtain the parameter
 *   img_ptr: pointer to image data
 *   img_len: image length
 *   param_ptr: [out] stores a pointer to the parameter
 *   param_len: [out] stores the length of the parameter
 */
int img_parser_get_auth_param(img_type_t img_type,
			      const auth_param_type_desc_t *type_desc,
			      void *img_ptr, unsigned int img_len,
			      void **param_ptr, unsigned int *param_len)
{
	unsigned int idx;

	assert(type_desc != NULL);
	assert(img_ptr != NULL);
	assert(img_len != 0);
	assert(param_ptr != NULL);
	assert(param_len != NULL);

	/* In a raw image we can only get the data itself */
	if (img_type == IMG_RAW) {
		assert(type_desc->type == AUTH_PARAM_RAW_DATA);
		*param_ptr = img_ptr;
		*param_len = img_len;
		return IMG_PARSER_OK;
	}

	/* Find the index of the required image parser library */
	idx = parser_lib_indices[img_type];
	assert(idx != INVALID_IDX);

	/* Call the function to obtain the parameter */
	return parser_lib_descs[idx].get_auth_param(type_desc, img_ptr, img_len,
			param_ptr, param_len);
}

/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __DESC_IMAGE_LOAD_H__
#define __DESC_IMAGE_LOAD_H__

#include <bl_common.h>

#if LOAD_IMAGE_V2
/* Following structure is used to store BL ep/image info. */
typedef struct bl_mem_params_node {
	unsigned int image_id;
	image_info_t image_info;
	entry_point_info_t ep_info;
	unsigned int next_handoff_image_id;
	bl_load_info_node_t load_node_mem;
	bl_params_node_t params_node_mem;
} bl_mem_params_node_t;

/*
 * Macro to register list of BL image descriptors,
 * defined as an array of bl_mem_params_node_t.
 */
#define REGISTER_BL_IMAGE_DESCS(_img_desc)				\
	bl_mem_params_node_t *bl_mem_params_desc_ptr = &_img_desc[0];	\
	unsigned int bl_mem_params_desc_num = ARRAY_SIZE(_img_desc);

/* BL image loading utility functions */
void flush_bl_params_desc(void);
int get_bl_params_node_index(unsigned int image_id);
bl_mem_params_node_t *get_bl_mem_params_node(unsigned int image_id);
bl_load_info_t *get_bl_load_info_from_mem_params_desc(void);
bl_params_t *get_next_bl_params_from_mem_params_desc(void);


#endif /* LOAD_IMAGE_V2 */
#endif /* __DESC_IMAGE_LOAD_H__ */

/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#include <ep_info.h>
#include <param_header.h>

#define UP	1
#define DOWN	0

/*******************************************************************************
 * Constants to identify the location of a memory region in a given memory
 * layout.
******************************************************************************/
#define TOP	0x1
#define BOTTOM	!TOP

/*
 * The following are used for image state attributes.
 * Image can only be in one of the following state.
 */
#define IMAGE_STATE_RESET			0
#define IMAGE_STATE_COPIED			1
#define IMAGE_STATE_COPYING			2
#define IMAGE_STATE_AUTHENTICATED		3
#define IMAGE_STATE_EXECUTED			4
#define IMAGE_STATE_INTERRUPTED			5

#define IMAGE_ATTRIB_SKIP_LOADING	0x02
#define IMAGE_ATTRIB_PLAT_SETUP		0x04

#define INVALID_IMAGE_ID		(0xFFFFFFFF)

/*******************************************************************************
 * Constants to indicate type of exception to the common exception handler.
 ******************************************************************************/
#define SYNC_EXCEPTION_SP_EL0		0x0
#define IRQ_SP_EL0			0x1
#define FIQ_SP_EL0			0x2
#define SERROR_SP_EL0			0x3
#define SYNC_EXCEPTION_SP_ELX		0x4
#define IRQ_SP_ELX			0x5
#define FIQ_SP_ELX			0x6
#define SERROR_SP_ELX			0x7
#define SYNC_EXCEPTION_AARCH64		0x8
#define IRQ_AARCH64			0x9
#define FIQ_AARCH64			0xa
#define SERROR_AARCH64			0xb
#define SYNC_EXCEPTION_AARCH32		0xc
#define IRQ_AARCH32			0xd
#define FIQ_AARCH32			0xe
#define SERROR_AARCH32			0xf

#ifndef __ASSEMBLY__
#include <cassert.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <utils_def.h> /* To retain compatibility */

/*
 * Declarations of linker defined symbols to help determine memory layout of
 * BL images
 */
#if SEPARATE_CODE_AND_RODATA
extern uintptr_t __TEXT_START__;
extern uintptr_t __TEXT_END__;
extern uintptr_t __RODATA_START__;
extern uintptr_t __RODATA_END__;
#else
extern uintptr_t __RO_START__;
extern uintptr_t __RO_END__;
#endif

#if defined(IMAGE_BL2)
extern uintptr_t __BL2_END__;
#elif defined(IMAGE_BL2U)
extern uintptr_t __BL2U_END__;
#elif defined(IMAGE_BL31)
extern uintptr_t __BL31_END__;
#elif defined(IMAGE_BL32)
extern uintptr_t __BL32_END__;
#endif /* IMAGE_BLX */

#if USE_COHERENT_MEM
extern uintptr_t __COHERENT_RAM_START__;
extern uintptr_t __COHERENT_RAM_END__;
#endif

/*******************************************************************************
 * Structure used for telling the next BL how much of a particular type of
 * memory is available for its use and how much is already used.
 ******************************************************************************/
typedef struct meminfo {
	uintptr_t total_base;
	size_t total_size;
#if !LOAD_IMAGE_V2
	uintptr_t free_base;
	size_t free_size;
#endif
} meminfo_t;

/*****************************************************************************
 * Image info binary provides information from the image loader that
 * can be used by the firmware to manage available trusted RAM.
 * More advanced firmware image formats can provide additional
 * information that enables optimization or greater flexibility in the
 * common firmware code
 *****************************************************************************/
typedef struct image_info {
	param_header_t h;
	uintptr_t image_base;   /* physical address of base of image */
	uint32_t image_size;    /* bytes read from image file */
#if LOAD_IMAGE_V2
	uint32_t image_max_size;
#endif
} image_info_t;

/*****************************************************************************
 * The image descriptor struct definition.
 *****************************************************************************/
typedef struct image_desc {
	/* Contains unique image id for the image. */
	unsigned int image_id;
	/*
	 * This member contains Image state information.
	 * Refer IMAGE_STATE_XXX defined above.
	 */
	unsigned int state;
	uint32_t copied_size;	/* image size copied in blocks */
	image_info_t image_info;
	entry_point_info_t ep_info;
} image_desc_t;

#if LOAD_IMAGE_V2
/* BL image node in the BL image loading sequence */
typedef struct bl_load_info_node {
	unsigned int image_id;
	image_info_t *image_info;
	struct bl_load_info_node *next_load_info;
} bl_load_info_node_t;

/* BL image head node in the BL image loading sequence */
typedef struct bl_load_info {
	param_header_t h;
	bl_load_info_node_t *head;
} bl_load_info_t;

/* BL image node in the BL image execution sequence */
typedef struct bl_params_node {
	unsigned int image_id;
	image_info_t *image_info;
	entry_point_info_t *ep_info;
	struct bl_params_node *next_params_info;
} bl_params_node_t;

/*
 * BL image head node in the BL image execution sequence
 * It is also used to pass information to next BL image.
 */
typedef struct bl_params {
	param_header_t h;
	bl_params_node_t *head;
} bl_params_t;

#else /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence. A pointer to a structure of this
 * type should be passed in X0 to BL31's cold boot entrypoint.
 *
 * Use of this structure and the X0 parameter is not mandatory: the BL31
 * platform code can use other mechanisms to provide the necessary information
 * about BL32 and BL33 to the common and SPD code.
 *
 * BL31 image information is mandatory if this structure is used. If either of
 * the optional BL32 and BL33 image information is not provided, this is
 * indicated by the respective image_info pointers being zero.
 ******************************************************************************/
typedef struct bl31_params {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
} bl31_params_t;

#endif /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
size_t image_size(unsigned int image_id);

int is_mem_free(uintptr_t free_base, size_t free_size,
		uintptr_t addr, size_t size);

#if LOAD_IMAGE_V2

int load_auth_image(unsigned int image_id, image_info_t *image_data);

#else

uintptr_t page_align(uintptr_t, unsigned);
int load_image(meminfo_t *mem_layout,
	       unsigned int image_id,
	       uintptr_t image_base,
	       image_info_t *image_data,
	       entry_point_info_t *entry_point_info);
int load_auth_image(meminfo_t *mem_layout,
		    unsigned int image_id,
		    uintptr_t image_base,
		    image_info_t *image_data,
		    entry_point_info_t *entry_point_info);
void reserve_mem(uintptr_t *free_base, size_t *free_size,
		uintptr_t addr, size_t size);

#endif /* LOAD_IMAGE_V2 */

extern const char build_message[];
extern const char version_string[];

void print_entry_point_info(const entry_point_info_t *ep_info);

#endif /*__ASSEMBLY__*/

#endif /* __BL_COMMON_H__ */

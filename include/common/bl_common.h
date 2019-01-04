/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL_COMMON_H
#define BL_COMMON_H

#include <common/ep_info.h>
#include <common/param_header.h>
#include <lib/utils_def.h>

#define UP	U(1)
#define DOWN	U(0)

/*******************************************************************************
 * Constants to identify the location of a memory region in a given memory
 * layout.
******************************************************************************/
#define TOP	U(0x1)
#define BOTTOM	U(0x0)

/*
 * The following are used for image state attributes.
 * Image can only be in one of the following state.
 */
#define IMAGE_STATE_RESET		U(0)
#define IMAGE_STATE_COPIED		U(1)
#define IMAGE_STATE_COPYING		U(2)
#define IMAGE_STATE_AUTHENTICATED	U(3)
#define IMAGE_STATE_EXECUTED		U(4)
#define IMAGE_STATE_INTERRUPTED		U(5)

#define IMAGE_ATTRIB_SKIP_LOADING	U(0x02)
#define IMAGE_ATTRIB_PLAT_SETUP		U(0x04)

#define INVALID_IMAGE_ID		U(0xFFFFFFFF)

/*******************************************************************************
 * Constants to indicate type of exception to the common exception handler.
 ******************************************************************************/
#define SYNC_EXCEPTION_SP_EL0		U(0x0)
#define IRQ_SP_EL0			U(0x1)
#define FIQ_SP_EL0			U(0x2)
#define SERROR_SP_EL0			U(0x3)
#define SYNC_EXCEPTION_SP_ELX		U(0x4)
#define IRQ_SP_ELX			U(0x5)
#define FIQ_SP_ELX			U(0x6)
#define SERROR_SP_ELX			U(0x7)
#define SYNC_EXCEPTION_AARCH64		U(0x8)
#define IRQ_AARCH64			U(0x9)
#define FIQ_AARCH64			U(0xa)
#define SERROR_AARCH64			U(0xb)
#define SYNC_EXCEPTION_AARCH32		U(0xc)
#define IRQ_AARCH32			U(0xd)
#define FIQ_AARCH32			U(0xe)
#define SERROR_AARCH32			U(0xf)

#ifndef __ASSEMBLY__

#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>

/*
 * Declarations of linker defined symbols to help determine memory layout of
 * BL images
 */
#if SEPARATE_CODE_AND_RODATA
IMPORT_SYM(unsigned long, __TEXT_START__,	BL_CODE_BASE);
IMPORT_SYM(unsigned long, __TEXT_END__,		BL_CODE_END);
IMPORT_SYM(unsigned long, __RODATA_START__,	BL_RO_DATA_BASE);
IMPORT_SYM(unsigned long, __RODATA_END__,	BL_RO_DATA_END);
#else
IMPORT_SYM(unsigned long, __RO_START__,		BL_CODE_BASE);
IMPORT_SYM(unsigned long, __RO_END__,		BL_CODE_END);
#endif

#if defined(IMAGE_BL2)
IMPORT_SYM(unsigned long, __BL2_END__,		BL2_END);
#elif defined(IMAGE_BL2U)
IMPORT_SYM(unsigned long, __BL2U_END__,		BL2U_END);
#elif defined(IMAGE_BL31)
IMPORT_SYM(unsigned long, __BL31_START__,	BL31_START);
IMPORT_SYM(unsigned long, __BL31_END__,		BL31_END);
#elif defined(IMAGE_BL32)
IMPORT_SYM(unsigned long, __BL32_END__,		BL32_END);
#endif /* IMAGE_BLX */

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#if USE_COHERENT_MEM
IMPORT_SYM(unsigned long, __COHERENT_RAM_START__,	BL_COHERENT_RAM_BASE);
IMPORT_SYM(unsigned long, __COHERENT_RAM_END__,		BL_COHERENT_RAM_END);
#endif

/*******************************************************************************
 * Structure used for telling the next BL how much of a particular type of
 * memory is available for its use and how much is already used.
 ******************************************************************************/
typedef struct meminfo {
	uintptr_t total_base;
	size_t total_size;
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
	uint32_t image_max_size;
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

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
size_t get_image_size(unsigned int image_id);

int is_mem_free(uintptr_t free_base, size_t free_size,
		uintptr_t addr, size_t size);

int load_auth_image(unsigned int image_id, image_info_t *image_data);

#if TRUSTED_BOARD_BOOT && defined(DYN_DISABLE_AUTH)
/*
 * API to dynamically disable authentication. Only meant for development
 * systems.
 */
void dyn_disable_auth(void);
#endif

extern const char build_message[];
extern const char version_string[];

void print_entry_point_info(const entry_point_info_t *ep_info);
uintptr_t page_align(uintptr_t value, unsigned dir);

struct mmap_region;

void setup_page_tables(const struct mmap_region *bl_regions,
			   const struct mmap_region *plat_regions);

#endif /*__ASSEMBLY__*/

#endif /* BL_COMMON_H */

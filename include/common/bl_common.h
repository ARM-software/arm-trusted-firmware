/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#define SECURE		0x0
#define NON_SECURE	0x1
#define sec_state_is_valid(s) (((s) == SECURE) || ((s) == NON_SECURE))

#define UP	1
#define DOWN	0

/*******************************************************************************
 * Constants to identify the location of a memory region in a given memory
 * layout.
******************************************************************************/
#define TOP	0x1
#define BOTTOM	!TOP

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'entry_point_info' structure at their correct offsets.
 ******************************************************************************/
#define ENTRY_POINT_INFO_PC_OFFSET	0x08
#ifdef AARCH32
#define ENTRY_POINT_INFO_ARGS_OFFSET	0x10
#else
#define ENTRY_POINT_INFO_ARGS_OFFSET	0x18
#endif

/* The following are used to set/get image attributes. */
#define PARAM_EP_SECURITY_MASK		(0x1)

#define GET_SECURITY_STATE(x) (x & PARAM_EP_SECURITY_MASK)
#define SET_SECURITY_STATE(x, security) \
			((x) = ((x) & ~PARAM_EP_SECURITY_MASK) | (security))


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

#define EP_EE_MASK	0x2
#define EP_EE_LITTLE	0x0
#define EP_EE_BIG	0x2
#define EP_GET_EE(x) (x & EP_EE_MASK)
#define EP_SET_EE(x, ee) ((x) = ((x) & ~EP_EE_MASK) | (ee))

#define EP_ST_MASK	0x4
#define EP_ST_DISABLE	0x0
#define EP_ST_ENABLE	0x4
#define EP_GET_ST(x) (x & EP_ST_MASK)
#define EP_SET_ST(x, ee) ((x) = ((x) & ~EP_ST_MASK) | (ee))

#define EP_EXE_MASK	0x8
#define NON_EXECUTABLE	0x0
#define EXECUTABLE	0x8
#define EP_GET_EXE(x) (x & EP_EXE_MASK)
#define EP_SET_EXE(x, ee) ((x) = ((x) & ~EP_EXE_MASK) | (ee))

#define EP_FIRST_EXE_MASK	0x10
#define EP_FIRST_EXE		0x10
#define EP_GET_FIRST_EXE(x) ((x) & EP_FIRST_EXE_MASK)
#define EP_SET_FIRST_EXE(x, ee) ((x) = ((x) & ~EP_FIRST_EXE_MASK) | (ee))

#define PARAM_EP		0x01
#define PARAM_IMAGE_BINARY	0x02
#define PARAM_BL31		0x03
#define PARAM_BL_LOAD_INFO	0x04
#define PARAM_BL_PARAMS		0x05
#define PARAM_PSCI_LIB_ARGS	0x06

#define IMAGE_ATTRIB_SKIP_LOADING	0x02
#define IMAGE_ATTRIB_PLAT_SETUP		0x04

#define VERSION_1	0x01
#define VERSION_2	0x02

#define INVALID_IMAGE_ID		(0xFFFFFFFF)

#define SET_PARAM_HEAD(_p, _type, _ver, _attr) do { \
	(_p)->h.type = (uint8_t)(_type); \
	(_p)->h.version = (uint8_t)(_ver); \
	(_p)->h.size = (uint16_t)sizeof(*_p); \
	(_p)->h.attr = (uint32_t)(_attr) ; \
	} while (0)

/* Following is used for populating structure members statically. */
#define SET_STATIC_PARAM_HEAD(_p, _type, _ver, _p_type, _attr)	\
	._p.h.type = (uint8_t)(_type), \
	._p.h.version = (uint8_t)(_ver), \
	._p.h.size = (uint16_t)sizeof(_p_type), \
	._p.h.attr = (uint32_t)(_attr)


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
#include <cdefs.h> /* For __dead2 */
#include <cassert.h>
#include <stdint.h>
#include <stddef.h>
#include <types.h>
#include <utils.h> /* To retain compatibility */

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

#if IMAGE_BL2
extern uintptr_t __BL2_END__;
#elif IMAGE_BL2U
extern uintptr_t __BL2U_END__;
#elif IMAGE_BL31
extern uintptr_t __BL31_END__;
#elif IMAGE_BL32
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

typedef struct aapcs64_params {
	u_register_t arg0;
	u_register_t arg1;
	u_register_t arg2;
	u_register_t arg3;
	u_register_t arg4;
	u_register_t arg5;
	u_register_t arg6;
	u_register_t arg7;
} aapcs64_params_t;

typedef struct aapcs32_params {
	u_register_t arg0;
	u_register_t arg1;
	u_register_t arg2;
	u_register_t arg3;
} aapcs32_params_t;

/***************************************************************************
 * This structure provides version information and the size of the
 * structure, attributes for the structure it represents
 ***************************************************************************/
typedef struct param_header {
	uint8_t type;		/* type of the structure */
	uint8_t version;    /* version of this structure */
	uint16_t size;      /* size of this structure in bytes */
	uint32_t attr;      /* attributes: unused bits SBZ */
} param_header_t;

/*****************************************************************************
 * This structure represents the superset of information needed while
 * switching exception levels. The only two mechanisms to do so are
 * ERET & SMC. Security state is indicated using bit zero of header
 * attribute
 * NOTE: BL1 expects entrypoint followed by spsr at an offset from the start
 * of this structure defined by the macro `ENTRY_POINT_INFO_PC_OFFSET` while
 * processing SMC to jump to BL31.
 *****************************************************************************/
typedef struct entry_point_info {
	param_header_t h;
	uintptr_t pc;
	uint32_t spsr;
#ifdef AARCH32
	aapcs32_params_t args;
#else
	aapcs64_params_t args;
#endif
} entry_point_info_t;

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

/*
 * Compile time assertions related to the 'entry_point_info' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(ENTRY_POINT_INFO_PC_OFFSET ==
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_BL31_pc_offset_mismatch);

CASSERT(ENTRY_POINT_INFO_ARGS_OFFSET == \
		__builtin_offsetof(entry_point_info_t, args), \
		assert_BL31_args_offset_mismatch);

CASSERT(sizeof(uintptr_t) ==
		__builtin_offsetof(entry_point_info_t, spsr) - \
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_entrypoint_and_spsr_should_be_adjacent);

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
size_t image_size(unsigned int image_id);

#if LOAD_IMAGE_V2

int load_image(unsigned int image_id, image_info_t *image_data);
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

/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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
#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <bl_common.h>
#include <platform_def.h>

/******************************************************************************
 * Required platform porting definitions that are expected to be common to
 * all platforms
 *****************************************************************************/

/*
 * Platform binary types for linking
 */
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64


/*
 * Generic platform constants
 */
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#if LOAD_IMAGE_V2
#define BL2_IMAGE_DESC {				\
	.image_id = BL2_IMAGE_ID,			\
	SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,	\
		VERSION_2, image_info_t, 0),		\
	.image_info.image_base = BL2_BASE,		\
	.image_info.image_max_size = BL2_LIMIT - BL2_BASE,\
	SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,	\
		VERSION_2, entry_point_info_t, SECURE | EXECUTABLE),\
	.ep_info.pc = BL2_BASE,				\
}
#else /* LOAD_IMAGE_V2 */
#define BL2_IMAGE_DESC {				\
	.image_id = BL2_IMAGE_ID,			\
	SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,	\
		VERSION_1, image_info_t, 0),		\
	.image_info.image_base = BL2_BASE,		\
	SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,	\
		VERSION_1, entry_point_info_t, SECURE | EXECUTABLE),\
	.ep_info.pc = BL2_BASE,				\
}
#endif

#endif /* __COMMON_DEF_H__ */


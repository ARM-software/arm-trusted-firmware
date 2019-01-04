/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PARAM_HEADER_H
#define PARAM_HEADER_H

#include <stdbool.h>

#include <lib/utils_def.h>

/* Param header types */
#define PARAM_EP			U(0x01)
#define PARAM_IMAGE_BINARY		U(0x02)
#define PARAM_BL31			U(0x03)
#define PARAM_BL_LOAD_INFO		U(0x04)
#define PARAM_BL_PARAMS			U(0x05)
#define PARAM_PSCI_LIB_ARGS		U(0x06)
#define PARAM_SP_IMAGE_BOOT_INFO	U(0x07)

/* Param header version */
#define VERSION_1	U(0x01)
#define VERSION_2	U(0x02)

#define SET_PARAM_HEAD(_p, _type, _ver, _attr) do { \
	(_p)->h.type = (uint8_t)(_type); \
	(_p)->h.version = (uint8_t)(_ver); \
	(_p)->h.size = (uint16_t)sizeof(*(_p)); \
	(_p)->h.attr = (uint32_t)(_attr) ; \
	} while (false)

/* Following is used for populating structure members statically. */
#define SET_STATIC_PARAM_HEAD(_p, _type, _ver, _p_type, _attr)	\
	._p.h.type = (uint8_t)(_type), \
	._p.h.version = (uint8_t)(_ver), \
	._p.h.size = (uint16_t)sizeof(_p_type), \
	._p.h.attr = (uint32_t)(_attr)

#ifndef __ASSEMBLY__

#include <stdint.h>

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

#endif /*__ASSEMBLY__*/

#endif /* PARAM_HEADER_H */

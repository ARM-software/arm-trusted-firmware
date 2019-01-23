/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SDS_PRIVATE_H
#define SDS_PRIVATE_H

/* SDS Header defines */
#define SDS_HEADER_ID_SHIFT			0
#define SDS_HEADER_ID_WIDTH			16
#define SDS_HEADER_ID_MASK			((1 << SDS_HEADER_ID_WIDTH) - 1)

#define SDS_HEADER_MINOR_VERSION_WIDTH		8
#define SDS_HEADER_MINOR_VERSION_SHIFT		16
#define SDS_HEADER_MAJOR_VERSION_WIDTH		8

#define MAKE_SDS_HEADER_VERSION(major, minor)	\
	(((((major) & 0xff) << SDS_HEADER_MINOR_VERSION_WIDTH) | ((minor) & 0xff)))
#define SDS_HEADER_VERSION_MASK			\
	((1 << (SDS_HEADER_MINOR_VERSION_WIDTH + SDS_HEADER_MAJOR_VERSION_WIDTH)) - 1)

#define SDS_HEADER_VERSION			MAKE_SDS_HEADER_VERSION(1, 0)
#define SDS_HEADER_STRUCT_SIZE_WIDTH		23
#define SDS_HEADER_STRUCT_SIZE_SHIFT		1
#define SDS_HEADER_STRUCT_SIZE_MASK		((1 << SDS_HEADER_STRUCT_SIZE_WIDTH) - 1)
#define SDS_HEADER_VALID_MASK			0x1
#define SDS_HEADER_VALID_SHIFT			0
#define SDS_HEADER_SIZE				0x8

/* Arbitrary, 16 bit value that indicates a valid SDS Memory Region */
#define SDS_REGION_SIGNATURE			0xAA7A
#define SDS_REGION_SIGNATURE_WIDTH		16
#define SDS_REGION_SIGNATURE_SHIFT		0
#define SDS_REGION_SIGNATURE_MASK		((1 << SDS_REGION_SIGNATURE_WIDTH) - 1)

#define SDS_REGION_STRUCT_COUNT_SHIFT		16
#define SDS_REGION_STRUCT_COUNT_WIDTH		8
#define SDS_REGION_STRUCT_COUNT_MASK		((1 << SDS_REGION_STRUCT_COUNT_WIDTH) - 1)

#define SDS_REGION_SCH_MINOR_SHIFT		24
#define SDS_REGION_SCH_MINOR_WIDTH		4
#define SDS_REGION_SCH_MINOR_MASK		((1 << SDS_REGION_SCH_MINOR_WIDTH) - 1)

#define SDS_REGION_SCH_MAJOR_SHIFT		28
#define SDS_REGION_SCH_MAJOR_WIDTH		4
#define SDS_REGION_SCH_MAJOR_MASK		((1 << SDS_REGION_SCH_MAJOR_WIDTH) - 1)

#define SDS_REGION_SCH_VERSION_MASK		\
	((1 << (SDS_REGION_SCH_MINOR_WIDTH + SDS_REGION_SCH_MAJOR_WIDTH)) - 1)

#define MAKE_SDS_REGION_SCH_VERSION(maj, min)	\
	((((maj) & SDS_REGION_SCH_MAJOR_MASK) << SDS_REGION_SCH_MINOR_WIDTH) |	\
	((min) & SDS_REGION_SCH_MINOR_MASK))

#define SDS_REGION_SCH_VERSION			MAKE_SDS_REGION_SCH_VERSION(1, 0)
#define SDS_REGION_REGIONSIZE_OFFSET		0x4
#define SDS_REGION_DESC_SIZE			0x8

#ifndef __ASSEMBLY__
#include <stddef.h>
#include <stdint.h>

/* Header containing Shared Data Structure metadata */
typedef struct structure_header {
	uint32_t reg[2];
} struct_header_t;

#define GET_SDS_HEADER_ID(_header)			\
	((((struct_header_t *)(_header))->reg[0]) & SDS_HEADER_ID_MASK)
#define GET_SDS_HEADER_VERSION(_header)			\
	(((((struct_header_t *)(_header))->reg[0]) >> SDS_HEADER_MINOR_VERSION_SHIFT)\
	& SDS_HEADER_VERSION_MASK)
#define GET_SDS_HEADER_STRUCT_SIZE(_header)		\
	(((((struct_header_t *)(_header))->reg[1]) >> SDS_HEADER_STRUCT_SIZE_SHIFT)\
	& SDS_HEADER_STRUCT_SIZE_MASK)
#define IS_SDS_HEADER_VALID(_header)			\
	((((struct_header_t *)(_header))->reg[1]) & SDS_HEADER_VALID_MASK)
#define GET_SDS_STRUCT_FIELD(_header, _field_offset)	\
	((((uint8_t *)(_header)) + sizeof(struct_header_t)) + (_field_offset))

/* Region Descriptor describing the SDS Memory Region */
typedef struct region_descriptor {
	uint32_t reg[2];
} region_desc_t;

#define IS_SDS_REGION_VALID(region)			\
	(((((region_desc_t *)(region))->reg[0]) & SDS_REGION_SIGNATURE_MASK) == SDS_REGION_SIGNATURE)
#define GET_SDS_REGION_STRUCTURE_COUNT(region)		\
	(((((region_desc_t *)(region))->reg[0]) >> SDS_REGION_STRUCT_COUNT_SHIFT)\
	& SDS_REGION_STRUCT_COUNT_MASK)
#define GET_SDS_REGION_SCHEMA_VERSION(region)		\
	(((((region_desc_t *)(region))->reg[0]) >> SDS_REGION_SCH_MINOR_SHIFT)\
	& SDS_REGION_SCH_VERSION_MASK)
#define GET_SDS_REGION_SIZE(region)		((((region_desc_t *)(region))->reg[1]))

#endif /* __ASSEMBLY__ */

#endif /* SDS_PRIVATE_H */

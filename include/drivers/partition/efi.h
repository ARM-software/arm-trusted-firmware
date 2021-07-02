/*
 * Copyright (c) 2021, Linaro Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DRIVERS_PARTITION_EFI_H
#define DRIVERS_PARTITION_EFI_H

#include <string.h>

#include <tools_share/uuid.h>

#define EFI_NAMELEN		36

static inline int guidcmp(const void *g1, const void *g2)
{
	return memcmp(g1, g2, sizeof(struct efi_guid));
}

static inline void *guidcpy(void *dst, const void *src)
{
	return memcpy(dst, src, sizeof(struct efi_guid));
}

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
	{ (a) & 0xffffffff,		\
	  (b) & 0xffff,			\
	  (c) & 0xffff,			\
	  { (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } }

#define NULL_GUID \
	EFI_GUID(0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#endif /* DRIVERS_PARTITION_EFI_H */

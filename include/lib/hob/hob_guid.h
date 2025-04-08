/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOB_GUID_H
#define HOB_GUID_H

#include <lib/hob/efi_types.h>

/**
 * Guid used for creating StandaloneMm related information.
 */

#define MM_PEI_MMRAM_MEMORY_RESERVE_GUID                                        \
{                                                                               \
	0x0703f912, 0xbf8d, 0x4e2a, {0xbe, 0x07, 0xab, 0x27, 0x25, 0x25, 0xc5, 0x92 } \
}

#define MM_NS_BUFFER_GUID                                                       \
{                                                                               \
	0xf00497e3, 0xbfa2, 0x41a1, {0x9d, 0x29, 0x54, 0xc2, 0xe9, 0x37, 0x21, 0xc5 } \
}

#define MM_MP_INFORMATION_GUID                                                  \
{                                                                               \
	0xba33f15d, 0x4000, 0x45c1, {0x8e, 0x88, 0xf9, 0x16, 0x92, 0xd4, 0x57, 0xe3}  \
}

#define MM_TPM_EVENT_LOG_GUID                                                   \
{                                                                               \
	0x360c4a06, 0x146c, 0x11f0, {0xb1, 0x73, 0x6b, 0xd5, 0x98, 0xff, 0x54, 0xd6}  \
}

#endif /* HOB_GUID_H */

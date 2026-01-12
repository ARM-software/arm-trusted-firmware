/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 */

#ifndef STM32MP_EFI_H
#define STM32MP_EFI_H

#include <drivers/partition/efi.h>

#define FWU_METADATA1_PART_GUID \
	EFI_GUID(0x5F91C128U, 0xE120U, 0x48D7U, \
		 0xBBU, 0x64U, 0x9FU, 0xA9U, 0xAEU, 0xD4U, 0xC7U, 0xE3U)

#define FWU_METADATA2_PART_GUID \
	EFI_GUID(0xF469F981U, 0x5985U, 0x4206U, \
		 0x8FU, 0xB3U, 0x83U, 0x99U, 0x56U, 0x03U, 0x5AU, 0x65U)

#define STM32MP_FIP_GUID \
	EFI_GUID(0x19d5df83U, 0x11b0U, 0x457bU, \
		 0xbeU, 0x2cU, 0x75U, 0x59U, 0xc1U, 0x31U, 0x42U, 0xa5U)


#endif /* STM32MP_EFI_H */

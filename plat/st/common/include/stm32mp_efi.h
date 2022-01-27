/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 */

#ifndef STM32MP_EFI_H
#define STM32MP_EFI_H

#include <drivers/partition/efi.h>

#define STM32MP_FIP_GUID \
	EFI_GUID(0x19d5df83, 0x11b0, 0x457b, \
		 0xbe, 0x2c, 0x75, 0x59, 0xc1, 0x31, 0x42, 0xa5)

#endif /* STM32MP_EFI_H */

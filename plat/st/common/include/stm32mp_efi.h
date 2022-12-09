/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 */

#ifndef STM32MP_EFI_H
#define STM32MP_EFI_H

#include <drivers/partition/efi.h>

#define STM32MP_FIP_GUID \
	EFI_GUID(0x19d5df83U, 0x11b0U, 0x457bU, \
		 0xbeU, 0x2cU, 0x75U, 0x59U, 0xc1U, 0x31U, 0x42U, 0xa5U)

#endif /* STM32MP_EFI_H */

/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <drivers/usb_device.h>

#include <usb_dfu.h>

struct usb_handle *usb_dfu_plat_init(void)
{
	return NULL;
}

uint8_t usb_dfu_get_phase(uint8_t alt)
{
	return 0;
}

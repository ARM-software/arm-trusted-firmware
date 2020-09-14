/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_USB_H
#define STM32MP1_USB_H

#include <drivers/usb_device.h>

void stm32mp1_usb_init_driver(struct usb_handle *usb_core_handle,
			      struct pcd_handle *pcd_handle,
			      void *base_register);

#endif /* STM32MP1_USB_H */

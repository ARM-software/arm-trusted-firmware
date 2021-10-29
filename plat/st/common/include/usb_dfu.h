/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_DFU_H
#define USB_DFU_H

#include <stdint.h>

#include <drivers/usb_device.h>

#define DFU_DESCRIPTOR_TYPE		0x21U

/* Max DFU Packet Size = 1024 bytes */
#define USBD_DFU_XFER_SIZE		1024U

#define TRANSFER_SIZE_BYTES(size) \
	((uint8_t)((size) & 0xFF)), /* XFERSIZEB0 */\
	((uint8_t)((size) >> 8))    /* XFERSIZEB1 */

/*
 * helper for descriptor of DFU interface 0 Alternate setting n
 * with iInterface = index of string descriptor, assumed Nth user string
 */
#define USBD_DFU_IF_DESC(n)	0x09U, /* Interface Descriptor size */\
				USB_DESC_TYPE_INTERFACE, /* descriptor type */\
				0x00U, /* Number of Interface */\
				(n), /* Alternate setting */\
				0x00U, /* bNumEndpoints*/\
				0xFEU, /* Application Specific Class Code */\
				0x01U, /* Device Firmware Upgrade Code */\
				0x02U, /* DFU mode protocol */ \
				USBD_IDX_USER0_STR + (n) /* iInterface */

/* DFU1.1 Standard */
#define USB_DFU_VERSION			0x0110U
#define USB_DFU_ITF_SIZ			9U
#define USB_DFU_DESC_SIZ(itf)		(USB_DFU_ITF_SIZ * ((itf) + 2U))

/*
 * bmAttribute value for DFU:
 * bitCanDnload = 1(bit 0)
 * bitCanUpload = 1(bit 1)
 * bitManifestationTolerant = 1 (bit 2)
 * bitWillDetach = 1(bit 3)
 * Reserved (bit4-6)
 * bitAcceleratedST = 0(bit 7)
 */
#define DFU_BM_ATTRIBUTE		0x0FU

#define DFU_STATUS_SIZE			6U

/* Callback for media access */
struct usb_dfu_media {
	int (*upload)(uint8_t alt, uintptr_t *buffer, uint32_t *len,
		      void *user_data);
	int (*download)(uint8_t alt, uintptr_t *buffer, uint32_t *len,
			void *user_data);
	int (*manifestation)(uint8_t alt, void *user_data);
};

/* Internal DFU handle */
struct usb_dfu_handle {
	uint8_t status[DFU_STATUS_SIZE];
	uint8_t dev_state;
	uint8_t dev_status;
	uint8_t alt_setting;
	const struct usb_dfu_media *callback;
};

void usb_dfu_register(struct usb_handle *pdev, struct usb_dfu_handle *phandle);

int usb_dfu_loop(struct usb_handle *pdev, const struct usb_dfu_media *pmedia);

/* Function provided by plat */
struct usb_handle *usb_dfu_plat_init(void);

#endif /* USB_DFU_H */

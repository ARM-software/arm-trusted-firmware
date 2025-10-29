/*
 * Copyright (c) 2024-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/st/usb_dwc3.h>
#include <drivers/usb_device.h>

#include <platform_def.h>
#include <stm32cubeprogrammer.h>
#include <stm32mp_common.h>
#include <usb_dfu.h>

/*  String size (1 byte) + type (1 byte) + 24 UTF16 characters: 2 bytes each */
#define SIZ_STRING_SERIAL U(24)
#define USB_SIZ_STRING_SERIAL (1U + 1U + (SIZ_STRING_SERIAL * 2U))
#define USBD_MAX_STR_DESC_SIZ U(0x100)
#define USBD_VID U(0x0483)
#define USBD_PID U(0xDF11)
#define USBD_LANGID_STRING U(0x409)
#define USBD_MANUFACTURER_STRING "STMicroelectronics"
#define USBD_CONFIGURATION_STRING "DFU Config"
#define USBD_INTERFACE_STRING "DFU Interface"

#define USB_DFU_ITF_NUM U(3)

#define USB_DFU_CONFIG_DESC_SIZ USB_DFU_DESC_SIZ(USB_DFU_ITF_NUM)

/* DFU devices */
static struct usb_dfu_handle usb_dfu_handle;

/* USB Standard Device Descriptor */
static const uint8_t usb_stm32mp2_desc[USB_LEN_DEV_DESC] = {
	USB_LEN_DEV_DESC, /* bLength */
	USB_DESC_TYPE_DEVICE, /* bDescriptorType */
	0x00, /* bcdUSB */
	0x02, /* version */
	0x00, /* bDeviceClass */
	0x00, /* bDeviceSubClass */
	0x00, /* bDeviceProtocol */
	USB_MAX_EP0_SIZE, /* bMaxPacketSize */
	LOBYTE(USBD_VID), /* idVendor */
	HIBYTE(USBD_VID), /* idVendor */
	LOBYTE(USBD_PID), /* idVendor */
	HIBYTE(USBD_PID), /* idVendor */
	0x00, /* bcdDevice rel. 2.00 */
	0x02,
	USBD_IDX_MFC_STR, /* Index of manufacturer string */
	USBD_IDX_PRODUCT_STR, /* Index of product string */
	USBD_IDX_SERIAL_STR, /* Index of serial number string */
	USBD_MAX_NUM_CONFIGURATION /* bNumConfigurations */
}; /* USB_DeviceDescriptor */

/* USB Standard String Descriptor */
static const uint8_t usb_stm32mp2_lang_id_desc[USB_LEN_LANGID_STR_DESC] = {
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING),
};

/* USB Standard Device Descriptor */
static const uint8_t usbd_stm32mp2_qualifier_desc[USB_LEN_DEV_QUALIFIER_DESC] = {
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};

/* USB serial number: build dynamically */
static uint8_t usb_stm32mp2_serial[USB_SIZ_STRING_SERIAL + 1];

/* USB DFU device Configuration Descriptor */
static const uint8_t usb_stm32mp2_config_desc[USB_DFU_CONFIG_DESC_SIZ] = {
	0x09, /* bLength: Configuration Descriptor size */
	USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
	USB_DFU_CONFIG_DESC_SIZ, /* wTotalLength: Bytes returned */
	0x00, 0x01, /* bNumInterfaces: 1 interface */
	0x01, /* bConfigurationValue: Configuration value */
	0x02, /* iConfiguration: Index of string descriptor for configuration */
	0xC0, /* bmAttributes: bus powered and Supprts Remote Wakeup */
	0x32, /* MaxPower 100 mA: this current is used for detecting Vbus */

	/* Descriptor of DFU interface 0 Alternate setting 0..N */
	USBD_DFU_IF_DESC(0), USBD_DFU_IF_DESC(1), USBD_DFU_IF_DESC(2),
	/* DFU Functional Descriptor */
	0x09, /* blength = 9 Bytes */
	DFU_DESCRIPTOR_TYPE, /* DFU Functional Descriptor */
	DFU_BM_ATTRIBUTE, /* bmAttribute for DFU */
	0xFF, /* DetachTimeOut = 255 ms */
	0x00,
	/* WARNING: In DMA mode the multiple MPS packets feature
	 *  is still not supported ==> In this case,
	 *  when using DMA USBD_DFU_XFER_SIZE should be set
	 *  to 64 in usbd_conf.h
	 */
	TRANSFER_SIZE_BYTES(USBD_DFU_XFER_SIZE), /* TransferSize = 1024 Byte */
	((USB_DFU_VERSION >> 0) & 0xFF), /* bcdDFUVersion */
	((USB_DFU_VERSION >> 8) & 0xFF)
};

const char *const if_desc_string[USB_DFU_ITF_NUM] = {
	"@DDR FIP /0x02/1*32Ke", "@FIP /0x03/1*16Me", "@virtual /0xF1/1*512Ba"
};

/* Buffer to build the unicode string provided to USB device stack */
static uint8_t usb_str_dec[USBD_MAX_STR_DESC_SIZ];

/*
 * Convert Ascii string into unicode one
 * desc : descriptor buffer
 * unicode : Formatted string buffer (unicode)
 * len : descriptor length
 */
static void stm32mp2_get_string(const char *desc, uint8_t *unicode,
				uint16_t *len)
{
	uint8_t idx = 0U;

	if (desc == NULL) {
		return;
	}

	*len = strlen(desc) * 2U + 2U;
	unicode[idx++] = *len;
	unicode[idx++] = USB_DESC_TYPE_STRING;

	while (*desc != '\0') {
		unicode[idx++] = *desc++;
		unicode[idx++] = 0x00U;
	}
}

/*
 * Create the serial number string descriptor
 */
static void update_serial_num_string(void)
{
	char serial_string[SIZ_STRING_SERIAL + 2U];
	/* serial number is set to 0 */
	uint32_t deviceserial[UID_WORD_NB] = { 0U, 0U, 0U };
	uint16_t length;

	if (stm32_get_uid_otp(deviceserial) != 0) {
		return;
	}

	/* build serial number with OTP value as in ROM code */
	snprintf(serial_string, sizeof(serial_string), "%08X%08X%08X",
		 deviceserial[0], deviceserial[1], deviceserial[2]);

	length = USB_SIZ_STRING_SERIAL;
	stm32mp2_get_string(serial_string, usb_stm32mp2_serial, &length);
}

/*
 * Return Device Qualifier descriptor
 * length : pointer data length
 * return : pointer to descriptor buffer
 */
static uint8_t *stm32mp2_get_qualifier_desc(uint16_t *length)
{
	*length = sizeof(usbd_stm32mp2_qualifier_desc);

	return (uint8_t *)usbd_stm32mp2_qualifier_desc;
}

/*
 * Return configuration descriptor
 * length : pointer data length
 * return : pointer to descriptor buffer
 */
static uint8_t *stm32mp2_get_config_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp2_config_desc);

	return (uint8_t *)usb_stm32mp2_config_desc;
}

/*
 * Returns the device descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_device_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp2_desc);

	return (uint8_t *)usb_stm32mp2_desc;
}

/*
 * Returns the LangID string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_lang_id_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp2_lang_id_desc);

	return (uint8_t *)usb_stm32mp2_lang_id_desc;
}

/*
 *  Returns the product string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_product_desc(uint16_t *length)
{
	char name[STM32_SOC_NAME_SIZE];
	char product[128];
	uint32_t chip_id;
	uint32_t chip_version;

	stm32mp_get_soc_name(name);
	chip_id = stm32mp_get_chip_dev_id();
	chip_version = stm32mp_get_chip_version();

	snprintf(product, sizeof(product),
		 "DFU @Device ID /0x%03X, @Revision ID /0x%04X, @Name /%s,",
		 chip_id, chip_version, name);

	stm32mp2_get_string(product, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the manufacturer string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_manufacturer_desc(uint16_t *length)
{
	stm32mp2_get_string(USBD_MANUFACTURER_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the serial number string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_serial_desc(uint16_t *length)
{
	*length = USB_SIZ_STRING_SERIAL;

	return (uint8_t *)usb_stm32mp2_serial;
}

/*
 * Returns the configuration string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_config_desc(uint16_t *length)
{
	stm32mp2_get_string(USBD_CONFIGURATION_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the interface string descriptor.
 * length : Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp2_interface_desc(uint16_t *length)
{
	stm32mp2_get_string(USBD_INTERFACE_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Manages the transfer of memory interfaces string descriptors.
 * index: descriptor index
 * length : pointer data length
 * return : pointer to the descriptor table or NULL if the descriptor
 *          is not supported.
 */
static uint8_t *stm32mp2_get_usr_desc(uint8_t index, uint16_t *length)
{
	if (index >= ARRAY_SIZE(if_desc_string)) {
		return NULL;
	}

	stm32mp2_get_string(if_desc_string[index], usb_str_dec, length);

	return usb_str_dec;
}

static const struct usb_desc dfu_desc = {
	.get_device_desc = stm32mp2_device_desc,
	.get_lang_id_desc = stm32mp2_lang_id_desc,
	.get_manufacturer_desc = stm32mp2_manufacturer_desc,
	.get_product_desc = stm32mp2_product_desc,
	.get_configuration_desc = stm32mp2_config_desc,
	.get_serial_desc = stm32mp2_serial_desc,
	.get_interface_desc = stm32mp2_interface_desc,
	.get_usr_desc = stm32mp2_get_usr_desc,
	.get_config_desc = stm32mp2_get_config_desc,
	.get_device_qualifier_desc = stm32mp2_get_qualifier_desc,
	/* only HS is supported, as ROM code */
	.get_other_speed_config_desc = NULL,
};

static struct usb_handle usb_core_handle;
static struct pcd_handle pcd_handle;
static dwc3_handle_t dwc3_handle;

struct usb_handle *usb_dfu_plat_init(void)
{
	/* prepare USB Driver */
	pcd_handle.in_ep[0].maxpacket = USB_MAX_EP0_SIZE;
	pcd_handle.out_ep[0].maxpacket = USB_MAX_EP0_SIZE;
	usb_dwc3_init_driver(&usb_core_handle, &pcd_handle, &dwc3_handle,
			     (void *)USB_DWC3_BASE);

	/* keep the configuration from ROM code */
	usb_core_handle.ep0_state = USBD_EP0_DATA_IN;
	usb_core_handle.dev_state = USBD_STATE_CONFIGURED;

	/* Update the serial number string descriptor from the unique ID */
	update_serial_num_string();

	/* Prepare USB DFU stack */
	usb_dfu_register(&usb_core_handle, &usb_dfu_handle);

	/* Register DFU descriptor in USB stack */
	register_platform(&usb_core_handle, &dfu_desc);

	return &usb_core_handle;
}

/* Link between USB alternate and STM32CubeProgramer phase */
uint8_t usb_dfu_get_phase(uint8_t alt)
{
	uint8_t ret;

	switch (alt) {
	case 0:
		ret = PHASE_DDR_FW;
		break;
	case 1:
		ret = PHASE_SSBL;
		break;
	case 2:
		ret = PHASE_CMD;
		break;
	default:
		ret = PHASE_RESET;
		break;
	}

	return ret;
}

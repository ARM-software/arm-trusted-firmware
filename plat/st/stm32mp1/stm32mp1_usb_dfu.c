/*
 * Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <limits.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <drivers/st/stm32mp1_usb.h>
#include <drivers/usb_device.h>

#include <platform_def.h>
#include <stm32cubeprogrammer.h>
#include <stm32mp_common.h>
#include <usb_dfu.h>

/*  String size (1 byte) + type (1 byte) + 24 UTF16 characters: 2 bytes each */
#define SIZ_STRING_SERIAL		U(24)
#define USB_SIZ_STRING_SERIAL		(1U + 1U + (SIZ_STRING_SERIAL * 2U))
#define USBD_MAX_STR_DESC_SIZ		0x100
#define USBD_VID			0x0483
#define USBD_PID			0xDF11
#define USBD_LANGID_STRING		0x409
#define USBD_MANUFACTURER_STRING	"STMicroelectronics"
#define USBD_CONFIGURATION_STRING	"DFU Config"
#define USBD_INTERFACE_STRING		"DFU Interface"

#if STM32MP13
#define USB_DFU_ITF_NUM			2
#endif
#if STM32MP15
#define USB_DFU_ITF_NUM			6
#endif

#define USB_DFU_CONFIG_DESC_SIZ		USB_DFU_DESC_SIZ(USB_DFU_ITF_NUM)

/* DFU devices */
static struct usb_dfu_handle usb_dfu_handle;

/* USB Standard Device Descriptor */
static const uint8_t usb_stm32mp1_desc[USB_LEN_DEV_DESC] = {
	USB_LEN_DEV_DESC,           /* bLength */
	USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
	0x00,                       /* bcdUSB */
	0x02,                       /* version */
	0x00,                       /* bDeviceClass */
	0x00,                       /* bDeviceSubClass */
	0x00,                       /* bDeviceProtocol */
	USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
	LOBYTE(USBD_VID),           /* idVendor */
	HIBYTE(USBD_VID),           /* idVendor */
	LOBYTE(USBD_PID),           /* idVendor */
	HIBYTE(USBD_PID),           /* idVendor */
	0x00,                       /* bcdDevice rel. 2.00 */
	0x02,
	USBD_IDX_MFC_STR,           /* Index of manufacturer string */
	USBD_IDX_PRODUCT_STR,       /* Index of product string */
	USBD_IDX_SERIAL_STR,        /* Index of serial number string */
	USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
}; /* USB_DeviceDescriptor */

/* USB Standard String Descriptor */
static const uint8_t usb_stm32mp1_lang_id_desc[USB_LEN_LANGID_STR_DESC] = {
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING),
};

/* USB Standard Device Descriptor */
static const uint8_t
usbd_stm32mp1_qualifier_desc[USB_LEN_DEV_QUALIFIER_DESC] = {
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
static uint8_t usb_stm32mp1_serial[USB_SIZ_STRING_SERIAL + 1];

/* USB DFU device Configuration Descriptor */
static const uint8_t usb_stm32mp1_config_desc[USB_DFU_CONFIG_DESC_SIZ] = {
	0x09, /* bLength: Configuration Descriptor size */
	USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
	USB_DFU_CONFIG_DESC_SIZ, /* wTotalLength: Bytes returned */
	0x00,
	0x01, /* bNumInterfaces: 1 interface */
	0x01, /* bConfigurationValue: Configuration value */
	0x02, /* iConfiguration: Index of string descriptor for configuration */
	0xC0, /* bmAttributes: bus powered and Supprts Remote Wakeup */
	0x32, /* MaxPower 100 mA: this current is used for detecting Vbus */

	/* Descriptor of DFU interface 0 Alternate setting 0..N */
	USBD_DFU_IF_DESC(0),
	USBD_DFU_IF_DESC(1),
#if USB_DFU_ITF_NUM > 2
	USBD_DFU_IF_DESC(2),
#endif
#if USB_DFU_ITF_NUM > 3
	USBD_DFU_IF_DESC(3),
#endif
#if USB_DFU_ITF_NUM > 4
	USBD_DFU_IF_DESC(4),
#endif
#if USB_DFU_ITF_NUM > 5
	USBD_DFU_IF_DESC(5),
#endif
	/* DFU Functional Descriptor */
	0x09, /* blength = 9 Bytes */
	DFU_DESCRIPTOR_TYPE, /* DFU Functional Descriptor */
	DFU_BM_ATTRIBUTE, /* bmAttribute for DFU */
	0xFF, /* DetachTimeOut = 255 ms */
	0x00,
	TRANSFER_SIZE_BYTES(USBD_DFU_XFER_SIZE), /* TransferSize = 1024 Byte */
	((USB_DFU_VERSION >> 0) & 0xFF), /* bcdDFUVersion */
	((USB_DFU_VERSION >> 8) & 0xFF)
};

/* The user strings: one by alternate as defined in USBD_DFU_IF_DESC */
#if STM32MP13
const char *const if_desc_string[USB_DFU_ITF_NUM] = {
	"@SSBL /0x03/1*16Me",
	"@virtual /0xF1/1*512Ba"
};
#endif
#if STM32MP15
const char *const if_desc_string[USB_DFU_ITF_NUM] = {
	"@Partition0 /0x00/1*256Ke",
	"@FSBL /0x01/1*1Me",
	"@Partition2 /0x02/1*1Me",
	"@Partition3 /0x03/1*16Me",
	"@Partition4 /0x04/1*16Me",
	"@virtual /0xF1/1*512Ba"
};
#endif

/* Buffer to build the unicode string provided to USB device stack */
static uint8_t usb_str_dec[USBD_MAX_STR_DESC_SIZ];

/*
 * Convert Ascii string into unicode one
 * desc : descriptor buffer
 * unicode : Formatted string buffer (unicode)
 * len : descriptor length
 */
static void stm32mp1_get_string(const char *desc, uint8_t *unicode, uint16_t *len)
{
	uint8_t idx = 0U;

	if (desc == NULL) {
		return;
	}

	*len =  strlen(desc) * 2U + 2U;
	unicode[idx++] = *len;
	unicode[idx++] =  USB_DESC_TYPE_STRING;

	while (*desc != '\0') {
		unicode[idx++] = *desc++;
		unicode[idx++] =  0x00U;
	}
}

/*
 * Create the serial number string descriptor
 */
static void update_serial_num_string(void)
{
	uint8_t i;
	char serial_string[SIZ_STRING_SERIAL + 2U];
	/* serial number is set to 0 */
	uint32_t deviceserial[UID_WORD_NB] = {0U, 0U, 0U};
	uint32_t otp;
	uint32_t len;
	uint16_t length;

	if (stm32_get_otp_index(UID_OTP, &otp, &len) != 0) {
		ERROR("BSEC: Get UID_OTP number Error\n");
		return;
	}

	if ((len / __WORD_BIT) != UID_WORD_NB) {
		ERROR("BSEC: Get UID_OTP length Error\n");
		return;
	}

	for (i = 0; i < UID_WORD_NB; i++) {
		if (bsec_shadow_read_otp(&deviceserial[i], i + otp) !=
		    BSEC_OK) {
			ERROR("BSEC: UID%d Error\n", i);
			return;
		}
	}
	/* build serial number with OTP value as in ROM code */
	snprintf(serial_string, sizeof(serial_string), "%08X%08X%08X",
		 deviceserial[0], deviceserial[1], deviceserial[2]);

	length = USB_SIZ_STRING_SERIAL;
	stm32mp1_get_string(serial_string, usb_stm32mp1_serial, &length);
}

/*
 * Return Device Qualifier descriptor
 * length : pointer data length
 * return : pointer to descriptor buffer
 */
static uint8_t *stm32mp1_get_qualifier_desc(uint16_t *length)
{
	*length = sizeof(usbd_stm32mp1_qualifier_desc);

	return (uint8_t *)usbd_stm32mp1_qualifier_desc;
}

/*
 * Return configuration descriptor
 * length : pointer data length
 * return : pointer to descriptor buffer
 */
static uint8_t *stm32mp1_get_config_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp1_config_desc);

	return (uint8_t *)usb_stm32mp1_config_desc;
}

/*
 * Returns the device descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_device_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp1_desc);

	return (uint8_t *)usb_stm32mp1_desc;
}

/*
 * Returns the LangID string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_lang_id_desc(uint16_t *length)
{
	*length = sizeof(usb_stm32mp1_lang_id_desc);

	return (uint8_t *)usb_stm32mp1_lang_id_desc;
}

/*
 *  Returns the product string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_product_desc(uint16_t *length)
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

	stm32mp1_get_string(product, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the manufacturer string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_manufacturer_desc(uint16_t *length)
{
	stm32mp1_get_string(USBD_MANUFACTURER_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the serial number string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_serial_desc(uint16_t *length)
{
	*length = USB_SIZ_STRING_SERIAL;

	return (uint8_t *)usb_stm32mp1_serial;
}

/*
 * Returns the configuration string descriptor.
 * length: Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_config_desc(uint16_t *length)
{
	stm32mp1_get_string(USBD_CONFIGURATION_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Returns the interface string descriptor.
 * length : Pointer to data length variable
 * return : Pointer to descriptor buffer
 */
static uint8_t *stm32mp1_interface_desc(uint16_t *length)
{
	stm32mp1_get_string(USBD_INTERFACE_STRING, usb_str_dec, length);

	return usb_str_dec;
}

/*
 * Manages the transfer of memory interfaces string descriptors.
 * index: descriptor index
 * length : pointer data length
 * return : pointer to the descriptor table or NULL if the descriptor
 *          is not supported.
 */
static uint8_t *stm32mp1_get_usr_desc(uint8_t index, uint16_t *length)
{
	if (index >= ARRAY_SIZE(if_desc_string)) {
		return NULL;
	}

	stm32mp1_get_string(if_desc_string[index], usb_str_dec, length);

	return usb_str_dec;
}

static const struct usb_desc dfu_desc = {
	.get_device_desc = stm32mp1_device_desc,
	.get_lang_id_desc = stm32mp1_lang_id_desc,
	.get_manufacturer_desc = stm32mp1_manufacturer_desc,
	.get_product_desc = stm32mp1_product_desc,
	.get_configuration_desc = stm32mp1_config_desc,
	.get_serial_desc = stm32mp1_serial_desc,
	.get_interface_desc = stm32mp1_interface_desc,
	.get_usr_desc = stm32mp1_get_usr_desc,
	.get_config_desc = stm32mp1_get_config_desc,
	.get_device_qualifier_desc = stm32mp1_get_qualifier_desc,
	/* only HS is supported, as ROM code */
	.get_other_speed_config_desc = NULL,
};

static struct usb_handle usb_core_handle;
static struct pcd_handle pcd_handle;

struct usb_handle *usb_dfu_plat_init(void)
{
	/* Prepare USB Driver */
	pcd_handle.in_ep[0].maxpacket = USB_MAX_EP0_SIZE;
	pcd_handle.out_ep[0].maxpacket = USB_MAX_EP0_SIZE;
	stm32mp1_usb_init_driver(&usb_core_handle, &pcd_handle,
				 (uint32_t *)USB_OTG_BASE);

#if STM32MP15
	/* STM32MP15 = keep the configuration from ROM code */
	usb_core_handle.ep0_state = USBD_EP0_DATA_IN;
	usb_core_handle.dev_state = USBD_STATE_CONFIGURED;
#endif

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
#if STM32MP13
	case 0:
		ret = PHASE_SSBL;
		break;
	case 1:
		ret = PHASE_CMD;
		break;
#endif
#if STM32MP15
	case 3:
		ret = PHASE_SSBL;
		break;
	case 5:
		ret = PHASE_CMD;
		break;
#endif
	default:
		ret = PHASE_RESET;
		break;
	}

	return ret;
}

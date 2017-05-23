/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __USB_H__
#define __USB_H__

#define USB_BLOCK_HIGH_SPEED_SIZE	512

#define EP0				0
#define EP1				1

#define RX_REQ_LEN			512
#define MAX_PACKET_LEN			512

#define DATA_FIFO_CONFIG		(0x780 << GDFIFOCFG_EPINFOBASE_SHIFT |\
					 0x800 << GDFIFOCFG_GDFIFOCFG_SHIFT)
/* RX FIFO: 2048 bytes */
#define RX_SIZE				0x00000200
/* Non-periodic TX FIFO: 128 bytes. start address: 0x200 * 4. */
#define ENDPOINT_TX_SIZE		0x00200200

/* EP1  TX FIFO: 1024 bytes. start address: 0x220 * 4. */
#define DATA_IN_ENDPOINT_TX_FIFO1	0x01000220

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT                     0               /* to device */
#define USB_DIR_IN                      0x80            /* to host */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07
#define USB_DT_INTERFACE_POWER          0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG                      0x09
#define USB_DT_DEBUG                    0x0a
#define USB_DT_INTERFACE_ASSOCIATION    0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY                 0x0c
#define USB_DT_KEY                      0x0d
#define USB_DT_ENCRYPTION_TYPE          0x0e
#define USB_DT_BOS                      0x0f
#define USB_DT_DEVICE_CAPABILITY        0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP   0x11
#define USB_DT_WIRE_ADAPTER             0x21
#define USB_DT_RPIPE                    0x22
#define USB_DT_CS_RADIO_CONTROL         0x23

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK                  0x1f
#define USB_RECIP_DEVICE                0x00
#define USB_RECIP_INTERFACE             0x01
#define USB_RECIP_ENDPOINT              0x02
#define USB_RECIP_OTHER                 0x03

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK        0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK           0x80

#define USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL       0
#define USB_ENDPOINT_XFER_ISOC          1
#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_XFER_INT           3
#define USB_ENDPOINT_MAX_ADJUSTABLE     0x80

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS              0x00
#define USB_REQ_CLEAR_FEATURE           0x01
#define USB_REQ_SET_FEATURE             0x03
#define USB_REQ_SET_ADDRESS             0x05
#define USB_REQ_GET_DESCRIPTOR          0x06
#define USB_REQ_SET_DESCRIPTOR          0x07
#define USB_REQ_GET_CONFIGURATION       0x08
#define USB_REQ_SET_CONFIGURATION       0x09
#define USB_REQ_GET_INTERFACE           0x0A
#define USB_REQ_SET_INTERFACE           0x0B
#define USB_REQ_SYNCH_FRAME             0x0C

#define USB_DT_DEVICE_SIZE              18

#define USB_CLASS_VENDOR_SPEC           0xff

/* from config descriptor bmAttributes */
#define USB_CONFIG_ATT_ONE              (1 << 7)        /* must be set */
#define USB_CONFIG_ATT_SELFPOWER        (1 << 6)        /* self powered */
#define USB_CONFIG_ATT_WAKEUP           (1 << 5)        /* can wakeup */
#define USB_CONFIG_ATT_BATTERY          (1 << 4)        /* battery powered */

typedef struct setup_packet {
	unsigned char		type;
	unsigned char		request;
	unsigned short		value;
	unsigned short		index;
	unsigned short		length;
} setup_packet_t;

typedef enum usb_interrupt {
	USB_INT_EP0_SETUP,
	USB_INT_EP0_OUT,
	USB_INT_EPX_OUT,
	USB_INT_EP0_IN,
	USB_INT_EPX_IN,
	USB_INT_ENUM_DONE,
	USB_INT_RESET,
	USB_INT_INVALID
} usb_interrupt_t;

typedef struct usb_ops {
	setup_packet_t	*(*handle_setup)(void);
	int		(*init)(void);
	int		(*poll)(usb_interrupt_t *event, size_t *size);
	int		(*prepare_setup)(void);
	int		(*receive_epx)(uintptr_t buf, size_t size);
	int		(*submit_packet)(uintptr_t buf, size_t size);
} usb_ops_t;

struct usb_device_descriptor {
        unsigned char  bLength;
        unsigned char  bDescriptorType;

        unsigned short bcdUSB;
        unsigned char  bDeviceClass;
        unsigned char  bDeviceSubClass;
        unsigned char  bDeviceProtocol;
        unsigned char  bMaxPacketSize0;
        unsigned short idVendor;
        unsigned short idProduct;
        unsigned short bcdDevice;
        unsigned char  iManufacturer;
        unsigned char  iProduct;
        unsigned char  iSerialNumber;
        unsigned char  bNumConfigurations;
} __attribute__ ((packed));

struct usb_config_descriptor {
       unsigned char  bLength;
       unsigned char  bDescriptorType;

       unsigned short wTotalLength;
       unsigned char  bNumInterfaces;
       unsigned char  bConfigurationValue;
       unsigned char  iConfiguration;
       unsigned char  bmAttributes;
       unsigned char  bMaxPower;
} __attribute__((packed));

struct usb_string_descriptor {
        unsigned char  bLength;
        unsigned char  bDescriptorType;

        unsigned short wString[16];             /* UTF-16LE encoded */
} __attribute__((packed));

struct usb_interface_descriptor {
        unsigned char  bLength;
        unsigned char  bDescriptorType;

        unsigned char  bInterfaceNumber;
        unsigned char  bAlternateSetting;
        unsigned char  bNumEndpoints;
        unsigned char  bInterfaceClass;
        unsigned char  bInterfaceSubClass;
        unsigned char  bInterfaceProtocol;
        unsigned char  iInterface;
};

struct usb_endpoint_descriptor {
        unsigned char  bLength;
        unsigned char  bDescriptorType;

        unsigned char  bEndpointAddress;
        unsigned char  bmAttributes;
        unsigned short wMaxPacketSize;
        unsigned char  bInterval;
} __attribute__ ((packed));

void usb_init(const usb_ops_t *ops_ptr);

#endif	/* __USB_H__ */

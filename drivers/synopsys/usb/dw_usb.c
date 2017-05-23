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

#include <arch_helpers.h>
#include <assert.h>
#include <ctype.h>
#include <debug.h>
#include <dw_usb.h>
#include <fastboot/fastboot.h>
#include <fastboot/usb.h>
#include <gpio.h>
#include <hi6220.h>
#include <mmio.h>
#include <platform_def.h>
#include <sp804_delay_timer.h>
#include <string.h>

static int dw_usb_get_descriptor(setup_packet_t *setup, uintptr_t out_buf,
				 size_t *size);
static setup_packet_t *dw_usb_handle_setup(void);
static int dw_usb_core_init(void);
static int dw_usb_poll(usb_interrupt_t *intr, size_t *size);
static int dw_usb_prepare_setup(void);
static int dw_usb_receive_epx(uintptr_t buf, size_t size);
static int dw_usb_set_addr(int addr);
static int dw_usb_submit_packet(uintptr_t buf, size_t size);
static void dw_usb_start_dma(uintptr_t buf, int in, int ep, size_t size);

static const struct usb_string_descriptor string_devicename = {
	24,
	USB_DT_STRING,
	{'A', 'n', 'd', 'r', 'o', 'i', 'd', ' ', '2', '.', '0'}
};

static const struct usb_string_descriptor lang_descriptor = {
	4,
	USB_DT_STRING,
	{0x0409}	/* en-US */
};

unsigned int rx_desc_bytes = 0;
static dw_usb_params_t dw_params;
static setup_packet_t dw_setup;

static const usb_ops_t dw_usb_ops = {
	.handle_setup		= dw_usb_handle_setup,
	.init			= dw_usb_core_init,
	.prepare_setup		= dw_usb_prepare_setup,
	.poll			= dw_usb_poll,
	.receive_epx		= dw_usb_receive_epx,
	.submit_packet		= dw_usb_submit_packet,
};

/* check high speed */
static int dw_usb_is_hs(void)
{
	/* 2'b00 High speed (PHY clock is at 30MHz or 60MHz) */
	return (mmio_read_32(dw_params.reg_base + DSTS) & 2) == 0 ? 1 : 0;
}

static void dw_usb_start_dma(uintptr_t buf, int in, int ep, size_t size)
{
	dw_usb_dma_desc_t  *descriptor = NULL;
	unsigned int data;
	int timeout = DW_USB_TIMEOUT;
	int quarter_size;

	data = DWUSB_DMAC_LAST | DWUSB_DMAC_IOC |
	       DWUSB_DMAC_BYTES((unsigned int)size);
	if (size == 0)
		buf = 0;

	/*
	 * IN uses 1/4 descriptor region
	 * EP0 OUT uses 1/4 descriptor region
	 * EPX OUT uese 1/2 descriptor region
	 */
	quarter_size = dw_params.desc.size / 4;
	if (in) {
		descriptor = (dw_usb_dma_desc_t *)dw_params.desc.base;
		descriptor->status = data;
		descriptor->buf = (unsigned int)buf;

		mmio_write_32(dw_params.reg_base + DIEPDMA(ep),
			      (unsigned int)((uintptr_t)descriptor));
		data = mmio_read_32(dw_params.reg_base + DIEPCTL(ep));
		data |= DXEPCTL_EPENA | DXEPCTL_CNAK;
		mmio_write_32(dw_params.reg_base + DIEPCTL(ep), data);
		/* polling for INT */
		do {
			data = mmio_read_32(dw_params.reg_base + DIEPINT(ep));
			if (--timeout == 0) {
				INFO("IN Timeout\n");
				break;
			}
			udelay(10);
		} while (data == 0);
	} else {
		if (ep)
			descriptor = (dw_usb_dma_desc_t *)(dw_params.desc.base +
							   quarter_size * 2);
		else
			descriptor = (dw_usb_dma_desc_t *)(dw_params.desc.base +
							   quarter_size);
		descriptor->status = data;
		descriptor->buf = (unsigned int)buf;

		mmio_write_32(dw_params.reg_base + DOEPDMA(ep),
			      (unsigned int)((uintptr_t)descriptor));
		data = mmio_read_32(dw_params.reg_base + DOEPCTL(ep));
		data |= DXEPCTL_EPENA | DXEPCTL_CNAK;
		mmio_write_32(dw_params.reg_base + DOEPCTL(ep), data);
	}
}

static void ep_send(int ep, const void *ptr, int len)
{
	unsigned int data;
	int tx_size, packets;

	if (ep == 0) {
		tx_size = 64;
	} else {
		if (dw_usb_is_hs()) {
			tx_size = USB_BLOCK_HIGH_SPEED_SIZE;
		} else {
			tx_size = 64;
		}
	}
	packets = (len + tx_size - 1) / tx_size;

	/* EPx OUT ACTIVE */
	data = mmio_read_32(dw_params.reg_base + DIEPCTL(ep)) |
	       DXEPCTL_USBACTEP;
	mmio_write_32(dw_params.reg_base + DIEPCTL(ep), data);

	if (!len) {
		mmio_write_32(dw_params.reg_base + DIEPTSIZ(ep),
			      DXEPTSIZ_PKTCNT(1));
		dw_usb_start_dma(0, 1, ep, len);
	} else {
		mmio_write_32(dw_params.reg_base + DIEPTSIZ(ep),
			      len | DXEPTSIZ_PKTCNT(packets));
		dw_usb_start_dma((uintptr_t)ptr, 1, ep, len);
	}
}

void dw_usb_stall(int endpoint, char stall, int dir)
{
	unsigned int data;

	data = mmio_read_32(dw_params.reg_base + DIEPCTL(endpoint));
	if (stall)
		data |= DXEPCTL_STALL;
	else
		data &= ~DXEPCTL_STALL;
	if (dir)
		mmio_write_32(dw_params.reg_base + DIEPCTL(endpoint), data);
	else
		mmio_write_32(dw_params.reg_base + DOEPCTL(endpoint), data);
}

static inline int ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

static setup_packet_t *dw_usb_handle_setup(void)
{
	uint8_t *resp;
	size_t size = 0;
	int result;
	unsigned int data;
	uintptr_t in_buf, offset;
	setup_packet_t *setup;

	/* OUT buffer */
	memcpy(&dw_setup, (void *)dw_params.buffer.base,
	       sizeof(setup_packet_t));
	setup = &dw_setup;
	/* IN buffer */
	offset = dw_params.buffer.size - (dw_params.buffer.size / 8);
	in_buf = dw_params.buffer.base + offset;
	switch (setup->request) {
	case USB_REQ_GET_STATUS:
		resp = (uint8_t *)in_buf;
		if (setup->type == USB_DIR_IN)
			resp[0] = 1;
		else
			resp[0] = 0;
		resp[1] = 0;
		size = 2;
		break;
	case USB_REQ_SET_ADDRESS:
		result = dw_usb_set_addr(setup->value);
		assert(result == 0);
		size = 0;
		break;
	case USB_REQ_GET_DESCRIPTOR:
		result = dw_usb_get_descriptor(setup, in_buf, &size);
		assert((result == 0) && (size <= 64) && (size > 0));
		break;
	case USB_REQ_SET_CONFIGURATION:
		data = mmio_read_32(dw_params.reg_base + DIEPCTL(EP1));
		data &= ~DXEPCTL_EPTYPE_MASK;
		data |= DXEPCTL_EPTYPE(USB_ENDPOINT_XFER_BULK) |
			DXEPCTL_SETD0PID | DXEPCTL_TXFNUM(EP1) |
			DXEPCTL_NAKSTS | DXEPCTL_NEXTEP(EP1) |
			DXEPCTL_USBACTEP;
		mmio_write_32(dw_params.reg_base + DIEPCTL(EP1), data);
		data = mmio_read_32(dw_params.reg_base + DOEPCTL(EP1));
		data &= ~DXEPCTL_EPTYPE_MASK;
		data |= DXEPCTL_EPTYPE(USB_ENDPOINT_XFER_BULK) |
			DXEPCTL_SETD0PID |DXEPCTL_NEXTEP(EP1) |
			DXEPCTL_USBACTEP;
		mmio_write_32(dw_params.reg_base + DOEPCTL(EP1), data);

		data = mmio_read_32(dw_params.reg_base + DIEPCTL(EP1));
		data |= DXEPCTL_SNAK;
		mmio_write_32(dw_params.reg_base + DIEPCTL(EP1), data);

		/* Enable interrupts on all endpoints */
		mmio_write_32(dw_params.reg_base + DAINTMSK, ~0);

		size = 0;
		break;
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
	case USB_REQ_GET_CONFIGURATION:
	case USB_REQ_GET_INTERFACE:
	case USB_REQ_SET_INTERFACE:
		break;
	default:
		NOTICE("not matched. request:0x%x\n", setup->request);
		break;
	}

	if (size > 0) {
		/* clear STALL to send out response back */
		dw_usb_stall(EP0, 0, USB_DIR_IN);
		dw_usb_stall(EP0, 0, USB_DIR_OUT);
		if (size > setup->length)
			size = setup->length;
		ep_send(0, (void *)in_buf, size);
	} else if (size == 0) {
		/* send an empty packet */
		ep_send(0, NULL, 0);
	}
	return setup;
}

static void dw_usb_core_reset(void)
{
	unsigned int	data;

	/* wait for OTG AHB master idle */
	do {
		data = mmio_read_32(dw_params.reg_base + GRSTCTL);
	} while ((data & GRSTCTL_AHBIDLE) == 0);

	/* OTG: assert software reset */
	mmio_write_32(dw_params.reg_base + GRSTCTL, GRSTCTL_CSFTRST);

	/* wait for OTG to ack reset */
	while (mmio_read_32(dw_params.reg_base + GRSTCTL) & GRSTCTL_CSFTRST);

	/* wait for OTG AHB master idle */
	do {
		data = mmio_read_32(dw_params.reg_base + GRSTCTL);
	} while ((data & GRSTCTL_AHBIDLE) == 0);
}

static void dw_usb_reset_endpoints(void)
{
	/* EP0 IN ACTIVE NEXT=1 */
	mmio_write_32(dw_params.reg_base + DIEPCTL(0), 0x8800);

	/* EP0 OUT ACTIVE */
	mmio_write_32(dw_params.reg_base + DOEPCTL(0), 0x8000);

	/* Clear any pending OTG Interrupts */
	mmio_write_32(dw_params.reg_base + GOTGINT, ~0);

	/* Clear any pending interrupts */
	mmio_write_32(dw_params.reg_base + GINTSTS, ~0);
	mmio_write_32(dw_params.reg_base + DIEPINT(0), ~0);
	mmio_write_32(dw_params.reg_base + DOEPINT(0), ~0);
	mmio_write_32(dw_params.reg_base + DIEPINT(1), ~0);
	mmio_write_32(dw_params.reg_base + DOEPINT(1), ~0);

	/* IN EP interrupt mask */
	mmio_write_32(dw_params.reg_base + DIEPMSK, 0x0D);
	/* OUT EP interrupt mask */
	mmio_write_32(dw_params.reg_base + DOEPMSK, 0x0D);
	/* Enable interrupts on Ep0 */
	mmio_write_32(dw_params.reg_base + DAINTMSK, 0x00010001);
}

static int dw_usb_core_init(void)
{
	unsigned int data;
	int timeout;

	dw_usb_core_reset();

	mmio_write_32(dw_params.reg_base + GDFIFOCFG, DATA_FIFO_CONFIG);
	mmio_write_32(dw_params.reg_base + GRXFSIZ, RX_SIZE);
	mmio_write_32(dw_params.reg_base + GNPTXFSIZ, ENDPOINT_TX_SIZE);
	mmio_write_32(dw_params.reg_base + DIEPTXF(1),
		      DATA_IN_ENDPOINT_TX_FIFO1);

	/*
	 * set Periodic TxFIFO Empty Level,
	 * Non-Periodic TxFIFO Empty Level,
	 * Enable DMA, Unmask Global Intr
	 */
	mmio_write_32(dw_params.reg_base + GAHBCFG, GAHBCFG_CTRL_MASK);

	/*select 8bit UTMI+, ULPI Inerface*/
	mmio_write_32(dw_params.reg_base + GUSBCFG, 0x2400);

	/* Detect usb work mode,host or device? */
	timeout = DW_USB_TIMEOUT;
	for (timeout = DW_USB_TIMEOUT; timeout > 0; timeout--) {
		data = mmio_read_32(dw_params.reg_base + GINTSTS);
		if ((data & GINTSTS_CURMODE_HOST) == 0)
			break;
	}
	if (timeout == 0) {
		WARN("dwusb is in host mode\n");
		return -EINVAL;
	}
	do {
		data = mmio_read_32(dw_params.reg_base + GINTSTS);
	} while (data & GINTSTS_CURMODE_HOST);

	/*set Non-Zero-Length status out handshake */
	data = (0x20 << DCFG_EPMISCNT_SHIFT) | DCFG_NZ_STS_OUT_HSHK;
	mmio_write_32(dw_params.reg_base + DCFG, data);

	/* Interrupt unmask: IN event, OUT event, ENUMDONE, bus reset */
	data = GINTSTS_OEPINT | GINTSTS_IEPINT | GINTSTS_ENUMDONE |
	       GINTSTS_USBRST;
	mmio_write_32(dw_params.reg_base + GINTMSK, data);

	do {
		data = mmio_read_32(dw_params.reg_base + GINTSTS);
	} while (data & GINTSTS_ENUMDONE);

	/* Clear any pending interrupts */
	mmio_write_32(dw_params.reg_base + GOTGINT, ~0);
	mmio_write_32(dw_params.reg_base + GINTSTS, ~0);
	/*endpoint settings cfg*/
	dw_usb_reset_endpoints();

	udelay(1);

	/* Soft Disconnect */
	mmio_write_32(dw_params.reg_base + DCTL, 0x802);
	udelay(10000);

	/* Soft Reconnect */
	mmio_write_32(dw_params.reg_base + DCTL, 0x800);

	VERBOSE("exit usbloader_init\n");
	return 0;
}

static int dw_usb_poll(usb_interrupt_t *event, size_t *size)
{
	unsigned int ints, maxpacket;
	unsigned int epints, data, ep;
	uintptr_t epx_desc, reg;
	dw_usb_dma_desc_t *desc;
	int result = -EAGAIN;

	ints = mmio_read_32(dw_params.reg_base + GINTSTS);

	*event = USB_INT_INVALID;
	if (ints & GINTSTS_USBRST) {
		*event = USB_INT_RESET;
		INFO("usb bus reset\n");
		goto exit;
	}
	if (ints & GINTSTS_ENUMDONE) {
		/* Set up the maximum packet sizes accordingly */
		maxpacket = dw_usb_is_hs() ? USB_BLOCK_HIGH_SPEED_SIZE : 64;
		data = mmio_read_32(dw_params.reg_base + DIEPCTL(1)) &
				    ~DXEPCTL_MPS_MASK;
		mmio_write_32(dw_params.reg_base + DIEPCTL(1),
			      data | maxpacket);
		data = mmio_read_32(dw_params.reg_base + DOEPCTL(1)) &
				    ~DXEPCTL_MPS_MASK;
		mmio_write_32(dw_params.reg_base + DOEPCTL(1),
			      data | maxpacket);
		*event = USB_INT_ENUM_DONE;
		goto exit;
	}


	if (ints & GINTSTS_OEPINT) {
		epints = mmio_read_32(dw_params.reg_base + DOEPINT(0));
		if (epints) {
			mmio_write_32(dw_params.reg_base + DOEPINT(0), epints);
			/* Transfer completed */
			if (epints & DXEPINT_AHBERR) {
				WARN("AHB error on OUT EP0.\n");
			}

			if (epints & DXEPINT_SETUP) {
				reg = dw_params.reg_base + DIEPCTL(0);
				data = mmio_read_32(reg) | DXEPCTL_SNAK;
				mmio_write_32(reg, data);
				reg = dw_params.reg_base + DOEPCTL(0);
				data = mmio_read_32(reg) | DXEPCTL_SNAK;
				mmio_write_32(reg, data);
				/*clear IN EP intr*/
				mmio_write_32(dw_params.reg_base + DIEPINT(0),
					      ~0);
				if (*event == USB_INT_INVALID)
					*event = USB_INT_EP0_SETUP;
			}
			if (*event == USB_INT_INVALID)
				*event = USB_INT_EP0_OUT;
			result = 0;
			goto exit;
		}

		epints = mmio_read_32(dw_params.reg_base + DOEPINT(1));
		if(epints) {
			mmio_write_32(dw_params.reg_base + DOEPINT(1), epints);
			/* Transfer Completed Interrupt (XferCompl) */
			if (epints & DXEPINT_XFERCOMPL) {
				if (*event == USB_INT_INVALID) {
					*event = USB_INT_EPX_OUT;
					assert(size != NULL);
					epx_desc = dw_params.desc.base +
						   dw_params.desc.size / 2;
					desc = (dw_usb_dma_desc_t *)epx_desc;
					*size = rx_desc_bytes -
						DWUSB_DMAC_BYTES(desc->status);
				}
			}

			if (epints & DXEPINT_AHBERR) {
				WARN("AHB error on OUT EP1.\n");
			}
			if (epints & DXEPINT_SETUP) {
				WARN("SETUP phase done  on OUT EP1.\n");
			}
			result = 0;
			goto exit;
		}
	}

	if (ints & GINTSTS_IEPINT) {
		ep = mmio_read_32(dw_params.reg_base + DAINT) & DAINT_IN_MASK;
		ep = ffs(ep) - 1;
		epints = mmio_read_32(dw_params.reg_base + DIEPINT(ep));
		mmio_write_32(dw_params.reg_base + DIEPINT(ep), epints);
		if (epints & DXEPINT_XFERCOMPL) {
			if (*event == USB_INT_INVALID) {
				if (ep == 0)
					*event = USB_INT_EP0_IN;
				else
					*event = USB_INT_EPX_IN;
			}
			result = 0;
			goto exit;
		}
	}
exit:
	/* write to clear interrupts */
	mmio_write_32(dw_params.reg_base + GINTSTS, ints);
	return result;
}

static int dw_usb_prepare_setup(void)
{
	unsigned int data;

	/* prepare to accept next setup packet */
	data = DOEPTSIZ0_SUPCNT(1) | DOEPTSIZ0_PKTCNT |
		(64 << DOEPTSIZ0_XFERSIZE_SHIFT);
	mmio_write_32(dw_params.reg_base + DOEPTSIZ0, data);
	dw_usb_start_dma(dw_params.buffer.base, 0, 0, 64);
	return 0;
}

static int dw_usb_set_addr(int addr)
{
	unsigned int data;

	data = mmio_read_32(dw_params.reg_base + DCFG);
	data &= ~DCFG_DEVADDR_MASK;
	mmio_write_32(dw_params.reg_base + DCFG, data | DCFG_DEVADDR(addr));
	return 0;
}

static int dw_usb_get_descriptor(setup_packet_t *setup,
				 uintptr_t out_buf, size_t *size)
{
	unsigned int type;
	struct usb_device_descriptor *device_desc;
	struct usb_config_descriptor *cfg_desc;
	struct usb_interface_descriptor *intf_desc;
	struct usb_endpoint_descriptor *ep0_desc, *ep1_desc;
	uintptr_t buf;

	assert((setup != NULL) && (out_buf != 0) && (size != NULL));
	type = setup->value >> 8;
	switch (type) {
	case USB_DT_DEVICE:
		device_desc = (struct usb_device_descriptor *)out_buf;
		device_desc->bLength = sizeof(struct usb_device_descriptor);
		device_desc->bDescriptorType = USB_DT_DEVICE;
		device_desc->bcdUSB = 0x0200;		/* USB 2.0 */
		device_desc->bDeviceClass = 0;
		device_desc->bDeviceSubClass = 0;
		device_desc->bDeviceProtocol = 0;
		device_desc->bMaxPacketSize0 = 64;
		device_desc->idVendor = 0x18d1;
		device_desc->idProduct = 0xd00d;
		device_desc->bcdDevice = 0x0100;
		device_desc->iManufacturer = 1;
		device_desc->iProduct = 2;
		device_desc->iSerialNumber = 3;
		device_desc->bNumConfigurations = 1;
		*size = device_desc->bLength;
		break;
	case USB_DT_CONFIG:
	case USB_DT_OTHER_SPEED_CONFIG:
		/* 1 config, 1 interface, 2 ep */
		buf = out_buf;
		*size = sizeof(struct usb_config_descriptor) +		\
			sizeof(struct usb_interface_descriptor) +	\
			sizeof(struct usb_endpoint_descriptor) * 2;
		cfg_desc = (struct usb_config_descriptor *)buf;
		cfg_desc->wTotalLength = *size;
		cfg_desc->bLength = sizeof(struct usb_config_descriptor);
		if (type == USB_DT_CONFIG)
			cfg_desc->bDescriptorType = USB_DT_CONFIG;
		else
			cfg_desc->bDescriptorType = USB_DT_OTHER_SPEED_CONFIG;
		cfg_desc->bNumInterfaces = 1;
		cfg_desc->bConfigurationValue = 1;
		cfg_desc->iConfiguration = 0;
		cfg_desc->bmAttributes = USB_CONFIG_ATT_ONE;
		cfg_desc->bMaxPower = 0x80;
		buf += cfg_desc->bLength;

		intf_desc = (struct usb_interface_descriptor *)buf;
		intf_desc->bLength = sizeof(struct usb_interface_descriptor);
		intf_desc->bDescriptorType = USB_DT_INTERFACE;
		intf_desc->bInterfaceNumber = 0;
		intf_desc->bAlternateSetting = 0;
		intf_desc->bNumEndpoints = 2;
		intf_desc->bInterfaceClass = USB_CLASS_VENDOR_SPEC;
		intf_desc->bInterfaceSubClass = 0x42;
		intf_desc->bInterfaceProtocol = 0x03;
		intf_desc->iInterface = 0;
		buf += intf_desc->bLength;

		ep0_desc = (struct usb_endpoint_descriptor *)buf;
		ep0_desc->bLength = sizeof(struct usb_endpoint_descriptor);
		ep0_desc->bDescriptorType = USB_DT_ENDPOINT;
		ep0_desc->bEndpointAddress = 0x81;
		ep0_desc->bmAttributes = USB_ENDPOINT_XFER_BULK;
		ep0_desc->bInterval = 0;
		buf += sizeof(struct usb_endpoint_descriptor);

		ep1_desc = (struct usb_endpoint_descriptor *)buf;
		ep1_desc->bLength = sizeof(struct usb_endpoint_descriptor);
		ep1_desc->bDescriptorType = USB_DT_ENDPOINT;
		ep1_desc->bEndpointAddress = 0x1;
		ep1_desc->bmAttributes = USB_ENDPOINT_XFER_BULK;
		ep1_desc->bInterval = 0;
		if (dw_usb_is_hs()) {
			ep0_desc->wMaxPacketSize = USB_BLOCK_HIGH_SPEED_SIZE;
			ep1_desc->wMaxPacketSize = USB_BLOCK_HIGH_SPEED_SIZE;
		} else {
			ep0_desc->wMaxPacketSize = 64;
			ep1_desc->wMaxPacketSize = 64;
		}
		break;
	case USB_DT_STRING:
		buf = out_buf;
		switch (setup->value & 0xff) {
		case 0:
			memcpy((void *)buf, &lang_descriptor,
			       sizeof(lang_descriptor));
			*size = lang_descriptor.bLength;
			break;
		case 1:
			memcpy((void *)buf, &string_devicename,
			       sizeof(string_devicename));
			*size = 14;
			break;
		case 2:
			memcpy((void *)buf, &string_devicename,
			       sizeof(string_devicename));
			*size = string_devicename.bLength;
			break;
		case 3:
			*size = 16;
			memcpy((void *)buf, dw_params.serialno, *size);
			break;
		}
		break;
	default:
		assert(0);
		break;
	}
	return 0;
}

int dw_usb_receive_epx(uintptr_t buf, size_t len)
{
	int rx_size;
	unsigned int data;

	dw_usb_stall(EP1, 0, USB_DIR_OUT);
	data = mmio_read_32(dw_params.reg_base + DOEPCTL(EP1)) |
			    DOEPCTL_USBACTEP;
	mmio_write_32(dw_params.reg_base + DOEPCTL(EP1), data);

	if (dw_usb_is_hs()) {
		rx_size = USB_BLOCK_HIGH_SPEED_SIZE;
	} else {
		rx_size = 64;
	}

	/* FIXME: what's this? */
	if (len >= rx_size * 64) {
		rx_desc_bytes = rx_size * 64;
	} else {
		rx_desc_bytes = len;
	}
	if (buf == 0)
		buf = dw_params.buffer.base;
	dw_usb_start_dma(buf, 0, EP1, rx_desc_bytes);
	return 0;
}

void dw_udc_epx_tx(int ep, void *buf, int len)
{
	int tx_size, timeout, packets;
	unsigned int data;

	/* clear EPx IN NAK status */
	while (mmio_read_32(dw_params.reg_base + GINTSTS) & GINTSTS_GINNAKEFF) {
		data = mmio_read_32(dw_params.reg_base + DCTL) | DCTL_CGNPINNAK;
		mmio_write_32(dw_params.reg_base + DCTL, data);
	}

	/* enable NAK for setup packet */
	data = mmio_read_32(dw_params.reg_base + DIEPCTL(ep)) | DXEPCTL_SNAK;
	mmio_write_32(dw_params.reg_base + DIEPCTL(ep), data);

	if (ep == 0) {
		tx_size = 64;
	} else {
		if (dw_usb_is_hs()) {
			tx_size = USB_BLOCK_HIGH_SPEED_SIZE;
		} else {
			tx_size = 64;
		}
	}
	packets = (len + tx_size - 1) / tx_size;

	/* wait for NAK interrupt */
	timeout = DW_USB_TIMEOUT;
	while (timeout--) {
		data = mmio_read_32(dw_params.reg_base + DIEPINT(ep));
		if (data & DXEPINT_NAKINTRPT)
			break;
		udelay(10);
	}
	if (timeout == 0) {
		WARN("DIEPCTL(%d):0x%x, DTXFSTS(%d):0x%x, "
		     "DIEPINT(%d):0x%x, DIEPTSIZ(%d):0x%x, "
		     "GINTSTS:0x%x\n",
		     ep, mmio_read_32(dw_params.reg_base + DIEPCTL(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DTXFSTS(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DIEPINT(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DIEPTSIZ(ep)),
		     mmio_read_32(dw_params.reg_base + GINTSTS));
	}
	if (len == 0) {
		mmio_write_32(dw_params.reg_base + DIEPTSIZ(ep),
			      DXEPTSIZ_PKTCNT(1));
		dw_usb_start_dma(0, 1, ep, len);
	} else {
		mmio_write_32(dw_params.reg_base + DIEPTSIZ(ep),
			      len | DXEPTSIZ_PKTCNT(packets));
		dw_usb_start_dma((uintptr_t)buf, 1, ep, len);
	}

	/* wait for EP transmission completed */
	timeout = DW_USB_TIMEOUT;
	while (timeout--) {
		if (!(mmio_read_32(dw_params.reg_base + GINTSTS) &
		      GINTSTS_IEPINT))
			continue;
		data = mmio_read_32(dw_params.reg_base + DIEPINT(ep));
		if (data & DXEPINT_XFERCOMPL)
			break;
	}
	if (timeout == 0) {
		WARN("DIEPCTL(%d):0x%x, DTXFSTS(%d):0x%x, "
		     "DIEPINT(%d):0x%x, DIEPTSIZ(%d):0x%x, "
		     "GINTSTS:0x%x\n",
		     ep, mmio_read_32(dw_params.reg_base + DIEPCTL(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DTXFSTS(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DIEPINT(ep)),
		     ep, mmio_read_32(dw_params.reg_base + DIEPTSIZ(ep)),
		     mmio_read_32(dw_params.reg_base + GINTSTS));
	}
}

static int dw_usb_submit_packet(uintptr_t buf, size_t size)
{
	uintptr_t offset, in_base;

	offset = dw_params.buffer.size - (dw_params.buffer.size / 8);
	in_base = dw_params.buffer.base + offset;
	memcpy((void *)in_base, (void *)buf, size + 1);
	dw_udc_epx_tx(EP1, (void *)in_base, size);
	return 0;
}

void dw_usb_init(const dw_usb_params_t *params)
{
	assert((params != NULL) && (params->serialno != NULL));
	memcpy(&dw_params, params, sizeof(dw_usb_params_t));
	usb_init(&dw_usb_ops);
}

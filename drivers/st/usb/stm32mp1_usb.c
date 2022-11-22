/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp1_usb.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define USB_OTG_MODE_DEVICE			0U
#define USB_OTG_MODE_HOST			1U
#define USB_OTG_MODE_DRD			2U

#define EP_TYPE_CTRL				0U
#define EP_TYPE_ISOC				1U
#define EP_TYPE_BULK				2U
#define EP_TYPE_INTR				3U

#define USBD_FIFO_FLUSH_TIMEOUT_US		1000U
#define EP0_FIFO_SIZE				64U

/* OTG registers offsets */
#define OTG_GOTGINT				0x004U
#define OTG_GAHBCFG				0x008U
#define OTG_GUSBCFG				0x00CU
#define OTG_GRSTCTL				0x010U
#define OTG_GINTSTS				0x014U
#define OTG_GINTMSK				0x018U
#define OTG_GRXSTSP				0x020U
#define OTG_GLPMCFG				0x054U
#define OTG_DCFG				0x800U
#define OTG_DCTL				0x804U
#define OTG_DSTS				0x808U
#define OTG_DIEPMSK				0x810U
#define OTG_DOEPMSK				0x814U
#define OTG_DAINT				0x818U
#define OTG_DAINTMSK				0x81CU
#define OTG_DIEPEMPMSK				0x834U

/* Definitions for OTG_DIEPx registers */
#define OTG_DIEP_BASE				0x900U
#define OTG_DIEP_SIZE				0x20U
#define OTG_DIEPCTL				0x00U
#define OTG_DIEPINT				0x08U
#define OTG_DIEPTSIZ				0x10U
#define OTG_DIEPDMA				0x14U
#define OTG_DTXFSTS				0x18U
#define OTG_DIEP_MAX_NB				9U

/* Definitions for OTG_DOEPx registers */
#define OTG_DOEP_BASE				0xB00U
#define OTG_DOEP_SIZE				0x20U
#define OTG_DOEPCTL				0x00U
#define OTG_DOEPINT				0x08U
#define OTG_DOEPTSIZ				0x10U
#define OTG_DOEPDMA				0x14U
#define OTG_D0EP_MAX_NB				9U

/* Definitions for OTG_DAINT registers */
#define OTG_DAINT_OUT_MASK			GENMASK(31, 16)
#define OTG_DAINT_OUT_SHIFT			16U
#define OTG_DAINT_IN_MASK			GENMASK(15, 0)
#define OTG_DAINT_IN_SHIFT			0U

#define OTG_DAINT_EP0_IN			BIT(16)
#define OTG_DAINT_EP0_OUT			BIT(0)

/* Definitions for FIFOs */
#define OTG_FIFO_BASE				0x1000U
#define OTG_FIFO_SIZE				0x1000U

/* Bit definitions for OTG_GOTGINT register */
#define OTG_GOTGINT_SEDET			BIT(2)

/* Bit definitions for OTG_GAHBCFG register */
#define OTG_GAHBCFG_GINT			BIT(0)

/* Bit definitions for OTG_GUSBCFG register */
#define OTG_GUSBCFG_TRDT			GENMASK(13, 10)
#define OTG_GUSBCFG_TRDT_SHIFT			10U

#define USBD_HS_TRDT_VALUE			9U

/* Bit definitions for OTG_GRSTCTL register */
#define OTG_GRSTCTL_RXFFLSH			BIT(4)
#define OTG_GRSTCTL_TXFFLSH			BIT(5)
#define OTG_GRSTCTL_TXFNUM_SHIFT		6U

/* Bit definitions for OTG_GINTSTS register */
#define OTG_GINTSTS_CMOD			BIT(0)
#define OTG_GINTSTS_MMIS			BIT(1)
#define OTG_GINTSTS_OTGINT			BIT(2)
#define OTG_GINTSTS_SOF				BIT(3)
#define OTG_GINTSTS_RXFLVL			BIT(4)
#define OTG_GINTSTS_USBSUSP			BIT(11)
#define OTG_GINTSTS_USBRST			BIT(12)
#define OTG_GINTSTS_ENUMDNE			BIT(13)
#define OTG_GINTSTS_IEPINT			BIT(18)
#define OTG_GINTSTS_OEPINT			BIT(19)
#define OTG_GINTSTS_IISOIXFR			BIT(20)
#define OTG_GINTSTS_IPXFR_INCOMPISOOUT		BIT(21)
#define OTG_GINTSTS_LPMINT			BIT(27)
#define OTG_GINTSTS_SRQINT			BIT(30)
#define OTG_GINTSTS_WKUPINT			BIT(31)

/* Bit definitions for OTG_GRXSTSP register */
#define OTG_GRXSTSP_EPNUM			GENMASK(3, 0)
#define OTG_GRXSTSP_BCNT			GENMASK(14, 4)
#define OTG_GRXSTSP_BCNT_SHIFT			4U
#define OTG_GRXSTSP_PKTSTS			GENMASK(20, 17)
#define OTG_GRXSTSP_PKTSTS_SHIFT		17U

#define STS_GOUT_NAK				1U
#define STS_DATA_UPDT				2U
#define STS_XFER_COMP				3U
#define STS_SETUP_COMP				4U
#define STS_SETUP_UPDT				6U

/* Bit definitions for OTG_GLPMCFG register */
#define OTG_GLPMCFG_BESL			GENMASK(5, 2)

/* Bit definitions for OTG_DCFG register */
#define OTG_DCFG_DAD				GENMASK(10, 4)
#define OTG_DCFG_DAD_SHIFT			4U

/* Bit definitions for OTG_DCTL register */
#define OTG_DCTL_RWUSIG				BIT(0)
#define OTG_DCTL_SDIS				BIT(1)
#define OTG_DCTL_CGINAK				BIT(8)

/* Bit definitions for OTG_DSTS register */
#define OTG_DSTS_SUSPSTS			BIT(0)
#define OTG_DSTS_ENUMSPD_MASK			GENMASK(2, 1)
#define OTG_DSTS_FNSOF0				BIT(8)

#define OTG_DSTS_ENUMSPD(val)			((val) << 1)
#define OTG_DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ	OTG_DSTS_ENUMSPD(0U)
#define OTG_DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ	OTG_DSTS_ENUMSPD(1U)
#define OTG_DSTS_ENUMSPD_LS_PHY_6MHZ		OTG_DSTS_ENUMSPD(2U)
#define OTG_DSTS_ENUMSPD_FS_PHY_48MHZ		OTG_DSTS_ENUMSPD(3U)

/* Bit definitions for OTG_DIEPMSK register */
#define OTG_DIEPMSK_XFRCM			BIT(0)
#define OTG_DIEPMSK_EPDM			BIT(1)
#define OTG_DIEPMSK_TOM				BIT(3)

/* Bit definitions for OTG_DOEPMSK register */
#define OTG_DOEPMSK_XFRCM			BIT(0)
#define OTG_DOEPMSK_EPDM			BIT(1)
#define OTG_DOEPMSK_STUPM			BIT(3)

/* Bit definitions for OTG_DIEPCTLx registers */
#define OTG_DIEPCTL_MPSIZ			GENMASK(10, 0)
#define OTG_DIEPCTL_STALL			BIT(21)
#define OTG_DIEPCTL_CNAK			BIT(26)
#define OTG_DIEPCTL_SD0PID_SEVNFRM		BIT(28)
#define OTG_DIEPCTL_SODDFRM			BIT(29)
#define OTG_DIEPCTL_EPDIS			BIT(30)
#define OTG_DIEPCTL_EPENA			BIT(31)

/* Bit definitions for OTG_DIEPINTx registers */
#define OTG_DIEPINT_XFRC			BIT(0)
#define OTG_DIEPINT_EPDISD			BIT(1)
#define OTG_DIEPINT_TOC				BIT(3)
#define OTG_DIEPINT_ITTXFE			BIT(4)
#define OTG_DIEPINT_INEPNE			BIT(6)
#define OTG_DIEPINT_TXFE			BIT(7)
#define OTG_DIEPINT_TXFE_SHIFT			7U

#define OTG_DIEPINT_MASK			(BIT(13) | BIT(11) | GENMASK(9, 0))

/* Bit definitions for OTG_DIEPTSIZx registers */
#define OTG_DIEPTSIZ_XFRSIZ			GENMASK(18, 0)
#define OTG_DIEPTSIZ_PKTCNT			GENMASK(28, 19)
#define OTG_DIEPTSIZ_PKTCNT_SHIFT		19U
#define OTG_DIEPTSIZ_MCNT_MASK			GENMASK(30, 29)
#define OTG_DIEPTSIZ_MCNT_DATA0			BIT(29)

#define OTG_DIEPTSIZ_PKTCNT_1			BIT(19)

/* Bit definitions for OTG_DTXFSTSx registers */
#define OTG_DTXFSTS_INEPTFSAV			GENMASK(15, 0)

/* Bit definitions for OTG_DOEPCTLx registers */
#define OTG_DOEPCTL_STALL			BIT(21)
#define OTG_DOEPCTL_CNAK			BIT(26)
#define OTG_DOEPCTL_SD0PID_SEVNFRM		BIT(28) /* other than endpoint 0 */
#define OTG_DOEPCTL_SD1PID_SODDFRM		BIT(29) /* other than endpoint 0 */
#define OTG_DOEPCTL_EPDIS			BIT(30)
#define OTG_DOEPCTL_EPENA			BIT(31)

/* Bit definitions for OTG_DOEPTSIZx registers */
#define OTG_DOEPTSIZ_XFRSIZ			GENMASK(18, 0)
#define OTG_DOEPTSIZ_PKTCNT			GENMASK(28, 19)
#define OTG_DOEPTSIZ_RXDPID_STUPCNT		GENMASK(30, 29)

/* Bit definitions for OTG_DOEPINTx registers */
#define OTG_DOEPINT_XFRC			BIT(0)
#define OTG_DOEPINT_STUP			BIT(3)
#define OTG_DOEPINT_OTEPDIS			BIT(4)

#define OTG_DOEPINT_MASK			(GENMASK(15, 12) | GENMASK(9, 8) | GENMASK(6, 0))

#define EP_NB					15U
#define EP_ALL					0x10U

/*
 * Flush TX FIFO.
 * handle: PCD handle.
 * num: FIFO number.
 *	   This parameter can be a value from 1 to 15 or EP_ALL.
 *	   EP_ALL= 0x10 means Flush all TX FIFOs
 * return: USB status.
 */
static enum usb_status usb_dwc2_flush_tx_fifo(void *handle, uint32_t num)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint64_t timeout = timeout_init_us(USBD_FIFO_FLUSH_TIMEOUT_US);

	mmio_write_32(usb_base_addr + OTG_GRSTCTL,
		      OTG_GRSTCTL_TXFFLSH | (uint32_t)(num << OTG_GRSTCTL_TXFNUM_SHIFT));

	while ((mmio_read_32(usb_base_addr + OTG_GRSTCTL) &
		OTG_GRSTCTL_TXFFLSH) == OTG_GRSTCTL_TXFFLSH) {
		if (timeout_elapsed(timeout)) {
			return USBD_TIMEOUT;
		}
	}

	return USBD_OK;
}

/*
 * Flush RX FIFO.
 * handle: PCD handle.
 * return: USB status.
 */
static enum usb_status usb_dwc2_flush_rx_fifo(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint64_t timeout = timeout_init_us(USBD_FIFO_FLUSH_TIMEOUT_US);

	mmio_write_32(usb_base_addr + OTG_GRSTCTL, OTG_GRSTCTL_RXFFLSH);

	while ((mmio_read_32(usb_base_addr + OTG_GRSTCTL) &
		 OTG_GRSTCTL_RXFFLSH) == OTG_GRSTCTL_RXFFLSH) {
		if (timeout_elapsed(timeout)) {
			return USBD_TIMEOUT;
		}
	}

	return USBD_OK;
}

/*
 * Return the global USB interrupt status.
 * handle: PCD handle.
 * return: Interrupt register value.
 */
static uint32_t usb_dwc2_read_int(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	return mmio_read_32(usb_base_addr + OTG_GINTSTS) &
	       mmio_read_32(usb_base_addr + OTG_GINTMSK);
}

/*
 * Return the USB device OUT endpoints interrupt.
 * handle: PCD handle.
 * return: Device OUT endpoint interrupts.
 */
static uint32_t usb_dwc2_all_out_ep_int(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	return ((mmio_read_32(usb_base_addr + OTG_DAINT) &
		 mmio_read_32(usb_base_addr + OTG_DAINTMSK)) &
		OTG_DAINT_OUT_MASK) >> OTG_DAINT_OUT_SHIFT;
}

/*
 * Return the USB device IN endpoints interrupt.
 * handle: PCD handle.
 * return: Device IN endpoint interrupts.
 */
static uint32_t usb_dwc2_all_in_ep_int(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	return ((mmio_read_32(usb_base_addr + OTG_DAINT) &
		 mmio_read_32(usb_base_addr + OTG_DAINTMSK)) &
		OTG_DAINT_IN_MASK) >> OTG_DAINT_IN_SHIFT;
}

/*
 * Return Device OUT EP interrupt register.
 * handle: PCD handle.
 * epnum: Endpoint number.
 *         This parameter can be a value from 0 to 15.
 * return: Device OUT EP Interrupt register.
 */
static uint32_t usb_dwc2_out_ep_int(void *handle, uint8_t epnum)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	return mmio_read_32(usb_base_addr + OTG_DOEP_BASE +
			    (epnum * OTG_DOEP_SIZE) + OTG_DOEPINT) &
	       mmio_read_32(usb_base_addr + OTG_DOEPMSK);
}

/*
 * Return Device IN EP interrupt register.
 * handle: PCD handle.
 * epnum: Endpoint number.
 *         This parameter can be a value from 0 to 15.
 * return: Device IN EP Interrupt register.
 */
static uint32_t usb_dwc2_in_ep_int(void *handle, uint8_t epnum)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t msk;
	uint32_t emp;

	msk = mmio_read_32(usb_base_addr + OTG_DIEPMSK);
	emp = mmio_read_32(usb_base_addr + OTG_DIEPEMPMSK);
	msk |= ((emp >> epnum) << OTG_DIEPINT_TXFE_SHIFT) & OTG_DIEPINT_TXFE;

	return mmio_read_32(usb_base_addr + OTG_DIEP_BASE +
			    (epnum * OTG_DIEP_SIZE) + OTG_DIEPINT) & msk;
}

/*
 * Return USB core mode.
 * handle: PCD handle.
 * return: Core mode.
 *         This parameter can be 0 (host) or 1 (device).
 */
static uint32_t usb_dwc2_get_mode(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	return mmio_read_32(usb_base_addr + OTG_GINTSTS) & OTG_GINTSTS_CMOD;
}

/*
 * Activate EP0 for detup transactions.
 * handle: PCD handle.
 * return: USB status.
 */
static enum usb_status usb_dwc2_activate_setup(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uintptr_t reg_offset = usb_base_addr + OTG_DIEP_BASE;

	/* Set the MPS of the IN EP based on the enumeration speed */
	mmio_clrbits_32(reg_offset + OTG_DIEPCTL, OTG_DIEPCTL_MPSIZ);

	if ((mmio_read_32(usb_base_addr + OTG_DSTS) & OTG_DSTS_ENUMSPD_MASK) ==
	    OTG_DSTS_ENUMSPD_LS_PHY_6MHZ) {
		mmio_setbits_32(reg_offset + OTG_DIEPCTL, 3U);
	}

	mmio_setbits_32(usb_base_addr + OTG_DCTL, OTG_DCTL_CGINAK);

	return USBD_OK;
}

/*
 * Prepare the EP0 to start the first control setup.
 * handle: Selected device.
 * return: USB status.
 */
static enum usb_status usb_dwc2_ep0_out_start(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uintptr_t reg_offset = usb_base_addr + OTG_DIEP_BASE + OTG_DIEPTSIZ;
	uint32_t reg_value = 0U;

	/* PKTCNT = 1 and XFRSIZ = 24 bytes for endpoint 0 */
	reg_value |= OTG_DIEPTSIZ_PKTCNT_1;
	reg_value |= (EP0_FIFO_SIZE & OTG_DIEPTSIZ_XFRSIZ);
	reg_value |= OTG_DOEPTSIZ_RXDPID_STUPCNT;

	mmio_write_32(reg_offset, reg_value);

	return USBD_OK;
}

/*
 * Write a packet into the TX FIFO associated with the EP/channel.
 * handle: Selected device.
 * src: Pointer to source buffer.
 * ch_ep_num: Endpoint or host channel number.
 * len: Number of bytes to write.
 * return: USB status.
 */
static enum usb_status usb_dwc2_write_packet(void *handle, uint8_t *src,
					  uint8_t ch_ep_num, uint16_t len)
{
	uint32_t reg_offset;
	uint32_t count32b = (len + 3U) / 4U;
	uint32_t i;

	reg_offset = (uintptr_t)handle + OTG_FIFO_BASE +
		     (ch_ep_num * OTG_FIFO_SIZE);

	for (i = 0U; i < count32b; i++) {
		uint32_t src_copy = 0U;
		uint32_t j;

		/* Data written to FIFO need to be 4 bytes aligned */
		for (j = 0U; j < 4U; j++) {
			src_copy += (*(src + j)) << (8U * j);
		}

		mmio_write_32(reg_offset, src_copy);
		src += 4U;
	}

	return USBD_OK;
}

/*
 * Read a packet from the RX FIFO associated with the EP/channel.
 * handle: Selected device.
 * dst: Destination pointer.
 * len: Number of bytes to read.
 * return: Pointer to destination buffer.
 */
static void *usb_dwc2_read_packet(void *handle, uint8_t *dest, uint16_t len)
{
	uint32_t reg_offset;
	uint32_t count32b = (len + 3U) / 4U;
	uint32_t i;

	VERBOSE("read packet length %i to 0x%lx\n", len, (uintptr_t)dest);

	reg_offset = (uintptr_t)handle + OTG_FIFO_BASE;

	for (i = 0U; i < count32b; i++) {
		*(uint32_t *)dest = mmio_read_32(reg_offset);
		dest += 4U;
		dsb();
	}

	return (void *)dest;
}

/*
 * Setup and start a transfer over an EP.
 * handle: Selected device
 * ep: Pointer to endpoint structure.
 * return: USB status.
 */
static enum usb_status usb_dwc2_ep_start_xfer(void *handle, struct usbd_ep *ep)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t reg_offset;
	uint32_t reg_value;
	uint32_t clear_value;

	if (ep->is_in) {
		reg_offset = usb_base_addr + OTG_DIEP_BASE + (ep->num * OTG_DIEP_SIZE);
		clear_value = OTG_DIEPTSIZ_PKTCNT | OTG_DIEPTSIZ_XFRSIZ;
		if (ep->xfer_len == 0U) {
			reg_value = OTG_DIEPTSIZ_PKTCNT_1;
		} else {
			/*
			 * Program the transfer size and packet count
			 * as follows:
			 * xfersize = N * maxpacket + short_packet
			 * pktcnt = N + (short_packet exist ? 1 : 0)
			 */
			reg_value = (OTG_DIEPTSIZ_PKTCNT &
				     (((ep->xfer_len + ep->maxpacket - 1U) /
				       ep->maxpacket) << OTG_DIEPTSIZ_PKTCNT_SHIFT))
				    | ep->xfer_len;

			if (ep->type == EP_TYPE_ISOC) {
				clear_value |= OTG_DIEPTSIZ_MCNT_MASK;
				reg_value |= OTG_DIEPTSIZ_MCNT_DATA0;
			}
		}

		mmio_clrsetbits_32(reg_offset + OTG_DIEPTSIZ, clear_value, reg_value);

		if ((ep->type != EP_TYPE_ISOC) && (ep->xfer_len > 0U)) {
			/* Enable the TX FIFO empty interrupt for this EP */
			mmio_setbits_32(usb_base_addr + OTG_DIEPEMPMSK, BIT(ep->num));
		}

		/* EP enable, IN data in FIFO */
		reg_value = OTG_DIEPCTL_CNAK | OTG_DIEPCTL_EPENA;

		if (ep->type == EP_TYPE_ISOC) {
			if ((mmio_read_32(usb_base_addr + OTG_DSTS) & OTG_DSTS_FNSOF0) == 0U) {
				reg_value |= OTG_DIEPCTL_SODDFRM;
			} else {
				reg_value |= OTG_DIEPCTL_SD0PID_SEVNFRM;
			}
		}

		mmio_setbits_32(reg_offset + OTG_DIEPCTL, reg_value);

		if (ep->type == EP_TYPE_ISOC) {
			usb_dwc2_write_packet(handle, ep->xfer_buff, ep->num, ep->xfer_len);
		}
	} else {
		reg_offset = usb_base_addr + OTG_DOEP_BASE + (ep->num * OTG_DOEP_SIZE);
		/*
		 * Program the transfer size and packet count as follows:
		 * pktcnt = N
		 * xfersize = N * maxpacket
		 */
		if (ep->xfer_len == 0U) {
			reg_value = ep->maxpacket | OTG_DIEPTSIZ_PKTCNT_1;
		} else {
			uint16_t pktcnt = (ep->xfer_len + ep->maxpacket - 1U) / ep->maxpacket;

			reg_value = (pktcnt << OTG_DIEPTSIZ_PKTCNT_SHIFT) |
				    (ep->maxpacket * pktcnt);
		}

		mmio_clrsetbits_32(reg_offset + OTG_DOEPTSIZ,
				   OTG_DOEPTSIZ_XFRSIZ & OTG_DOEPTSIZ_PKTCNT,
				   reg_value);

		/* EP enable */
		reg_value = OTG_DOEPCTL_CNAK | OTG_DOEPCTL_EPENA;

		if (ep->type == EP_TYPE_ISOC) {
			if ((mmio_read_32(usb_base_addr + OTG_DSTS) & OTG_DSTS_FNSOF0) == 0U) {
				reg_value |= OTG_DOEPCTL_SD1PID_SODDFRM;
			} else {
				reg_value |= OTG_DOEPCTL_SD0PID_SEVNFRM;
			}
		}

		mmio_setbits_32(reg_offset + OTG_DOEPCTL, reg_value);
	}

	return USBD_OK;
}

/*
 * Setup and start a transfer over the EP0.
 * handle: Selected device.
 * ep: Pointer to endpoint structure.
 * return: USB status.
 */
static enum usb_status usb_dwc2_ep0_start_xfer(void *handle, struct usbd_ep *ep)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t reg_offset;
	uint32_t reg_value;

	if (ep->is_in) {
		reg_offset = usb_base_addr + OTG_DIEP_BASE +
			     (ep->num * OTG_DIEP_SIZE);

		if (ep->xfer_len == 0U) {
			reg_value = OTG_DIEPTSIZ_PKTCNT_1;
		} else {
			/*
			 * Program the transfer size and packet count
			 * as follows:
			 * xfersize = N * maxpacket + short_packet
			 * pktcnt = N + (short_packet exist ? 1 : 0)
			 */

			if (ep->xfer_len > ep->maxpacket) {
				ep->xfer_len = ep->maxpacket;
			}

			reg_value = OTG_DIEPTSIZ_PKTCNT_1 | ep->xfer_len;
		}

		mmio_clrsetbits_32(reg_offset + OTG_DIEPTSIZ,
				   OTG_DIEPTSIZ_XFRSIZ | OTG_DIEPTSIZ_PKTCNT,
				   reg_value);

		/* Enable the TX FIFO empty interrupt for this EP */
		if (ep->xfer_len > 0U) {
			mmio_setbits_32(usb_base_addr +	OTG_DIEPEMPMSK,
					BIT(ep->num));
		}

		/* EP enable, IN data in FIFO */
		mmio_setbits_32(reg_offset + OTG_DIEPCTL,
				OTG_DIEPCTL_CNAK | OTG_DIEPCTL_EPENA);
	} else {
		reg_offset = usb_base_addr + OTG_DOEP_BASE +
			     (ep->num * OTG_DOEP_SIZE);

		/*
		 * Program the transfer size and packet count as follows:
		 * pktcnt = N
		 * xfersize = N * maxpacket
		 */
		if (ep->xfer_len > 0U) {
			ep->xfer_len = ep->maxpacket;
		}

		reg_value = OTG_DIEPTSIZ_PKTCNT_1 | ep->maxpacket;

		mmio_clrsetbits_32(reg_offset + OTG_DIEPTSIZ,
				   OTG_DIEPTSIZ_XFRSIZ | OTG_DIEPTSIZ_PKTCNT,
				   reg_value);

		/* EP enable */
		mmio_setbits_32(reg_offset + OTG_DOEPCTL,
				OTG_DOEPCTL_CNAK | OTG_DOEPCTL_EPENA);
	}

	return USBD_OK;
}

/*
 * Set a stall condition over an EP.
 * handle: Selected device.
 * ep: Pointer to endpoint structure.
 * return: USB status.
 */
static enum usb_status usb_dwc2_ep_set_stall(void *handle, struct usbd_ep *ep)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t reg_offset;
	uint32_t reg_value;

	if (ep->is_in) {
		reg_offset = usb_base_addr + OTG_DIEP_BASE +
			     (ep->num * OTG_DIEP_SIZE);
		reg_value = mmio_read_32(reg_offset + OTG_DIEPCTL);

		if ((reg_value & OTG_DIEPCTL_EPENA) == 0U) {
			reg_value &= ~OTG_DIEPCTL_EPDIS;
		}

		reg_value |= OTG_DIEPCTL_STALL;

		mmio_write_32(reg_offset + OTG_DIEPCTL, reg_value);
	} else {
		reg_offset = usb_base_addr + OTG_DOEP_BASE +
			     (ep->num * OTG_DOEP_SIZE);
		reg_value = mmio_read_32(reg_offset + OTG_DOEPCTL);

		if ((reg_value & OTG_DOEPCTL_EPENA) == 0U) {
			reg_value &= ~OTG_DOEPCTL_EPDIS;
		}

		reg_value |= OTG_DOEPCTL_STALL;

		mmio_write_32(reg_offset + OTG_DOEPCTL, reg_value);
	}

	return USBD_OK;
}

/*
 * Stop the USB device mode.
 * handle: Selected device.
 * return: USB status.
 */
static enum usb_status usb_dwc2_stop_device(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t i;

	/* Disable Int */
	mmio_clrbits_32(usb_base_addr + OTG_GAHBCFG, OTG_GAHBCFG_GINT);

	/* Clear pending interrupts */
	for (i = 0U; i < EP_NB; i++) {
		mmio_write_32(usb_base_addr + OTG_DIEP_BASE + (i * OTG_DIEP_SIZE) + OTG_DIEPINT,
			      OTG_DIEPINT_MASK);
		mmio_write_32(usb_base_addr + OTG_DOEP_BASE + (i * OTG_DOEP_SIZE) + OTG_DOEPINT,
			      OTG_DOEPINT_MASK);
	}

	mmio_write_32(usb_base_addr + OTG_DAINT, OTG_DAINT_IN_MASK | OTG_DAINT_OUT_MASK);

	/* Clear interrupt masks */
	mmio_write_32(usb_base_addr + OTG_DIEPMSK, 0U);
	mmio_write_32(usb_base_addr + OTG_DOEPMSK, 0U);
	mmio_write_32(usb_base_addr + OTG_DAINTMSK, 0U);

	/* Flush the FIFO */
	usb_dwc2_flush_rx_fifo(handle);
	usb_dwc2_flush_tx_fifo(handle, EP_ALL);

	/* Disconnect the USB device by disabling the pull-up/pull-down */
	mmio_setbits_32((uintptr_t)handle + OTG_DCTL, OTG_DCTL_SDIS);

	return USBD_OK;
}

/*
 * Stop the USB device mode.
 * handle: Selected device.
 * address: New device address to be assigned.
 *         This parameter can be a value from 0 to 255.
 * return: USB status.
 */
static enum usb_status usb_dwc2_set_address(void *handle, uint8_t address)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	mmio_clrsetbits_32(usb_base_addr + OTG_DCFG,
			   OTG_DCFG_DAD,
			   address << OTG_DCFG_DAD_SHIFT);

	return USBD_OK;
}

/*
 * Check FIFO for the next packet to be loaded.
 * handle: Selected device.
 * epnum : Endpoint number.
 * xfer_len: Block length.
 * xfer_count: Number of blocks.
 * maxpacket: Max packet length.
 * xfer_buff: Buffer pointer.
 * return: USB status.
 */
static enum usb_status usb_dwc2_write_empty_tx_fifo(void *handle,
						    uint32_t epnum,
						    uint32_t xfer_len,
						    uint32_t *xfer_count,
						    uint32_t maxpacket,
						    uint8_t **xfer_buff)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t reg_offset;
	int32_t len;
	uint32_t len32b;
	enum usb_status ret;

	len = xfer_len - *xfer_count;

	if ((len > 0) && ((uint32_t)len > maxpacket)) {
		len = maxpacket;
	}

	len32b = (len + 3U) / 4U;

	reg_offset = usb_base_addr + OTG_DIEP_BASE + (epnum * OTG_DIEP_SIZE);

	while (((mmio_read_32(reg_offset + OTG_DTXFSTS) &
		OTG_DTXFSTS_INEPTFSAV) > len32b) &&
	       (*xfer_count < xfer_len) && (xfer_len != 0U)) {
		/* Write the FIFO */
		len = xfer_len - *xfer_count;

		if ((len > 0) && ((uint32_t)len > maxpacket)) {
			len = maxpacket;
		}

		len32b = (len + 3U) / 4U;

		ret = usb_dwc2_write_packet(handle, *xfer_buff, epnum, len);
		if (ret != USBD_OK) {
			return ret;
		}

		*xfer_buff  += len;
		*xfer_count += len;
	}

	if (len <= 0) {
		mmio_clrbits_32(usb_base_addr + OTG_DIEPEMPMSK, BIT(epnum));
	}

	return USBD_OK;
}

/*
 * Handle PCD interrupt request.
 * handle: PCD handle.
 * param: Pointer to information updated by the IT handling.
 * return: Action to do after IT handling.
 */
static enum usb_action usb_dwc2_it_handler(void *handle, uint32_t *param)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;
	uint32_t ep_intr;
	uint32_t epint;
	uint32_t epnum;
	uint32_t temp;
	enum usb_status __unused ret;

	if (usb_dwc2_get_mode(handle) != USB_OTG_MODE_DEVICE) {
		return USB_NOTHING;
	}

	/* Avoid spurious interrupt */
	if (usb_dwc2_read_int(handle) == 0U) {
		return USB_NOTHING;
	}

	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_MMIS) != 0U) {
		/* Incorrect mode, acknowledge the interrupt */
		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_MMIS);
	}

	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_OEPINT) != 0U) {
		uint32_t reg_offset;

		/* Read in the device interrupt bits */
		ep_intr = usb_dwc2_all_out_ep_int(handle);
		epnum = 0U;
		while ((ep_intr & BIT(0)) != BIT(0)) {
			epnum++;
			ep_intr >>= 1;
		}

		reg_offset = usb_base_addr + OTG_DOEP_BASE + (epnum * OTG_DOEP_SIZE) + OTG_DOEPINT;

		epint = usb_dwc2_out_ep_int(handle, epnum);

		if ((epint & OTG_DOEPINT_XFRC) == OTG_DOEPINT_XFRC) {
			mmio_write_32(reg_offset, OTG_DOEPINT_XFRC);
			*param = epnum;

			return USB_DATA_OUT;
		}

		if ((epint & OTG_DOEPINT_STUP) == OTG_DOEPINT_STUP) {
			/* Inform  that a setup packet is available */
			mmio_write_32(reg_offset, OTG_DOEPINT_STUP);

			return USB_SETUP;
		}

		if ((epint & OTG_DOEPINT_OTEPDIS) == OTG_DOEPINT_OTEPDIS) {
			mmio_write_32(reg_offset, OTG_DOEPINT_OTEPDIS);
		}
	}

	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_IEPINT) != 0U) {
		uint32_t reg_offset;

		/* Read in the device interrupt bits */
		ep_intr = usb_dwc2_all_in_ep_int(handle);
		epnum = 0U;
		while ((ep_intr & BIT(0)) != BIT(0)) {
			epnum++;
			ep_intr >>= 1;
		}

		reg_offset = usb_base_addr + OTG_DIEP_BASE + (epnum * OTG_DIEP_SIZE) + OTG_DIEPINT;

		epint = usb_dwc2_in_ep_int(handle, epnum);

		if ((epint & OTG_DIEPINT_XFRC) == OTG_DIEPINT_XFRC) {
			mmio_clrbits_32(usb_base_addr + OTG_DIEPEMPMSK, BIT(epnum));
			mmio_write_32(reg_offset, OTG_DIEPINT_XFRC);
			*param = epnum;

			return USB_DATA_IN;
		}

		if ((epint & OTG_DIEPINT_TOC) == OTG_DIEPINT_TOC) {
			mmio_write_32(reg_offset, OTG_DIEPINT_TOC);
		}

		if ((epint & OTG_DIEPINT_ITTXFE) == OTG_DIEPINT_ITTXFE) {
			mmio_write_32(reg_offset, OTG_DIEPINT_ITTXFE);
		}

		if ((epint & OTG_DIEPINT_INEPNE) == OTG_DIEPINT_INEPNE) {
			mmio_write_32(reg_offset, OTG_DIEPINT_INEPNE);
		}

		if ((epint & OTG_DIEPINT_EPDISD) == OTG_DIEPINT_EPDISD) {
			mmio_write_32(reg_offset, OTG_DIEPINT_EPDISD);
		}

		if ((epint & OTG_DIEPINT_TXFE) == OTG_DIEPINT_TXFE) {
			*param = epnum;

			return USB_WRITE_EMPTY;
		}
	}

	/* Handle resume interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_WKUPINT) != 0U) {
		INFO("handle USB : Resume\n");

		/* Clear the remote wake-up signaling */
		mmio_clrbits_32(usb_base_addr + OTG_DCTL, OTG_DCTL_RWUSIG);
		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_WKUPINT);

		return USB_RESUME;
	}

	/* Handle suspend interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_USBSUSP) != 0U) {
		INFO("handle USB : Suspend int\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_USBSUSP);

		if ((mmio_read_32(usb_base_addr + OTG_DSTS) &
		     OTG_DSTS_SUSPSTS) == OTG_DSTS_SUSPSTS) {
			return USB_SUSPEND;
		}
	}

	/* Handle LPM interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_LPMINT) != 0U) {
		INFO("handle USB : LPM int enter in suspend\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_LPMINT);
		*param = (mmio_read_32(usb_base_addr + OTG_GLPMCFG) &
			  OTG_GLPMCFG_BESL) >> 2;

		return USB_LPM;
	}

	/* Handle reset interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_USBRST) != 0U) {
		INFO("handle USB : Reset\n");

		mmio_clrbits_32(usb_base_addr + OTG_DCTL, OTG_DCTL_RWUSIG);

		usb_dwc2_flush_tx_fifo(handle, 0U);

		mmio_write_32(usb_base_addr + OTG_DAINT, OTG_DAINT_IN_MASK | OTG_DAINT_OUT_MASK);
		mmio_setbits_32(usb_base_addr + OTG_DAINTMSK, OTG_DAINT_EP0_IN | OTG_DAINT_EP0_OUT);

		mmio_setbits_32(usb_base_addr + OTG_DOEPMSK, OTG_DOEPMSK_STUPM |
							     OTG_DOEPMSK_XFRCM |
							     OTG_DOEPMSK_EPDM);
		mmio_setbits_32(usb_base_addr + OTG_DIEPMSK, OTG_DIEPMSK_TOM |
							     OTG_DIEPMSK_XFRCM |
							     OTG_DIEPMSK_EPDM);

		/* Set default address to 0 */
		mmio_clrbits_32(usb_base_addr + OTG_DCFG, OTG_DCFG_DAD);

		/* Setup EP0 to receive SETUP packets */
		ret = usb_dwc2_ep0_out_start(handle);
		assert(ret == USBD_OK);

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_USBRST);

		return USB_RESET;
	}

	/* Handle enumeration done interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_ENUMDNE) != 0U) {
		ret = usb_dwc2_activate_setup(handle);
		assert(ret == USBD_OK);

		mmio_clrbits_32(usb_base_addr + OTG_GUSBCFG, OTG_GUSBCFG_TRDT);

		mmio_setbits_32(usb_base_addr + OTG_GUSBCFG,
				(USBD_HS_TRDT_VALUE << OTG_GUSBCFG_TRDT_SHIFT) & OTG_GUSBCFG_TRDT);

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_ENUMDNE);

		return USB_ENUM_DONE;
	}

	/* Handle RXQLevel interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_RXFLVL) != 0U) {
		mmio_clrbits_32(usb_base_addr + OTG_GINTMSK,
				OTG_GINTSTS_RXFLVL);

		temp = mmio_read_32(usb_base_addr + OTG_GRXSTSP);

		*param = temp & OTG_GRXSTSP_EPNUM;
		*param |= (temp & OTG_GRXSTSP_BCNT) << (USBD_OUT_COUNT_SHIFT -
							OTG_GRXSTSP_BCNT_SHIFT);

		if (((temp & OTG_GRXSTSP_PKTSTS) >> OTG_GRXSTSP_PKTSTS_SHIFT) == STS_DATA_UPDT) {
			if ((temp & OTG_GRXSTSP_BCNT) != 0U) {
				mmio_setbits_32(usb_base_addr + OTG_GINTMSK, OTG_GINTSTS_RXFLVL);

				return USB_READ_DATA_PACKET;
			}
		} else if (((temp & OTG_GRXSTSP_PKTSTS) >> OTG_GRXSTSP_PKTSTS_SHIFT) ==
			    STS_SETUP_UPDT) {
			mmio_setbits_32(usb_base_addr + OTG_GINTMSK, OTG_GINTSTS_RXFLVL);

			return USB_READ_SETUP_PACKET;
		}

		mmio_setbits_32(usb_base_addr + OTG_GINTMSK, OTG_GINTSTS_RXFLVL);
	}

	/* Handle SOF interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_SOF) != 0U) {
		INFO("handle USB : SOF\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_SOF);

		return USB_SOF;
	}

	/* Handle incomplete ISO IN interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_IISOIXFR) != 0U) {
		INFO("handle USB : ISO IN\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS,
			      OTG_GINTSTS_IISOIXFR);
	}

	/* Handle incomplete ISO OUT interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_IPXFR_INCOMPISOOUT) !=
	    0U) {
		INFO("handle USB : ISO OUT\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS,
			      OTG_GINTSTS_IPXFR_INCOMPISOOUT);
	}

	/* Handle connection event interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_SRQINT) != 0U) {
		INFO("handle USB : Connect\n");

		mmio_write_32(usb_base_addr + OTG_GINTSTS, OTG_GINTSTS_SRQINT);
	}

	/* Handle disconnection event interrupt */
	if ((usb_dwc2_read_int(handle) & OTG_GINTSTS_OTGINT) != 0U) {
		INFO("handle USB : Disconnect\n");

		temp = mmio_read_32(usb_base_addr + OTG_GOTGINT);

		if ((temp & OTG_GOTGINT_SEDET) == OTG_GOTGINT_SEDET) {
			return USB_DISCONNECT;
		}
	}

	return USB_NOTHING;
}

/*
 * Start the usb device mode
 * usb_core_handle: USB core driver handle.
 * return  USB status.
 */
static enum usb_status usb_dwc2_start_device(void *handle)
{
	uintptr_t usb_base_addr = (uintptr_t)handle;

	mmio_clrbits_32(usb_base_addr + OTG_DCTL, OTG_DCTL_SDIS);
	mmio_setbits_32(usb_base_addr + OTG_GAHBCFG, OTG_GAHBCFG_GINT);

	return USBD_OK;
}

static const struct usb_driver usb_dwc2driver = {
	.ep0_out_start = usb_dwc2_ep0_out_start,
	.ep_start_xfer = usb_dwc2_ep_start_xfer,
	.ep0_start_xfer = usb_dwc2_ep0_start_xfer,
	.write_packet = usb_dwc2_write_packet,
	.read_packet = usb_dwc2_read_packet,
	.ep_set_stall = usb_dwc2_ep_set_stall,
	.start_device = usb_dwc2_start_device,
	.stop_device = usb_dwc2_stop_device,
	.set_address = usb_dwc2_set_address,
	.write_empty_tx_fifo = usb_dwc2_write_empty_tx_fifo,
	.it_handler = usb_dwc2_it_handler
};

/*
 * Initialize USB DWC2 driver.
 * usb_core_handle: USB core driver handle.
 * pcd_handle: PCD handle.
 * base_register: USB global register base address.
 */
void stm32mp1_usb_init_driver(struct usb_handle *usb_core_handle,
			      struct pcd_handle *pcd_handle,
			      void *base_register)
{
	register_usb_driver(usb_core_handle, pcd_handle, &usb_dwc2driver,
			    base_register);
}

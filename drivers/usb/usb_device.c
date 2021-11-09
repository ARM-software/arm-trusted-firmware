/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/usb_device.h>

/* Define for EP address */
#define EP_DIR_MASK		BIT(7)
#define EP_DIR_IN		BIT(7)
#define EP_NUM_MASK		GENMASK(3, 0)

#define EP0_IN			(0U | EP_DIR_IN)
#define EP0_OUT			0U

/* USB address between 1 through 127 = 0x7F mask */
#define ADDRESS_MASK		GENMASK(6, 0)

/*
 * Set a STALL condition over an endpoint
 * pdev: USB handle
 * ep_addr: endpoint address
 * return : status
 */
static enum usb_status usb_core_set_stall(struct usb_handle *pdev, uint8_t ep_addr)
{
	struct usbd_ep *ep;
	struct pcd_handle *hpcd = (struct pcd_handle *)pdev->data;
	uint8_t num;

	num = ep_addr & EP_NUM_MASK;
	if (num >= USBD_EP_NB) {
		return USBD_FAIL;
	}
	if ((EP_DIR_MASK & ep_addr) == EP_DIR_IN) {
		ep = &hpcd->in_ep[num];
		ep->is_in = true;
	} else {
		ep = &hpcd->out_ep[num];
		ep->is_in = false;
	}
	ep->num = num;

	pdev->driver->ep_set_stall(hpcd->instance, ep);
	if (num == 0U) {
		pdev->driver->ep0_out_start(hpcd->instance);
	}

	return USBD_OK;
}

/*
 * usb_core_get_desc
 *         Handle Get Descriptor requests
 * pdev : device instance
 * req : usb request
 */
static void usb_core_get_desc(struct usb_handle *pdev, struct usb_setup_req *req)
{
	uint16_t len;
	uint8_t *pbuf;
	uint8_t desc_type = HIBYTE(req->value);
	uint8_t desc_idx = LOBYTE(req->value);

	switch (desc_type) {
	case USB_DESC_TYPE_DEVICE:
		pbuf = pdev->desc->get_device_desc(&len);
		break;

	case USB_DESC_TYPE_CONFIGURATION:
		pbuf = pdev->desc->get_config_desc(&len);
		break;

	case USB_DESC_TYPE_STRING:
		switch (desc_idx) {
		case USBD_IDX_LANGID_STR:
			pbuf = pdev->desc->get_lang_id_desc(&len);
			break;

		case USBD_IDX_MFC_STR:
			pbuf = pdev->desc->get_manufacturer_desc(&len);
			break;

		case USBD_IDX_PRODUCT_STR:
			pbuf = pdev->desc->get_product_desc(&len);
			break;

		case USBD_IDX_SERIAL_STR:
			pbuf = pdev->desc->get_serial_desc(&len);
			break;

		case USBD_IDX_CONFIG_STR:
			pbuf = pdev->desc->get_configuration_desc(&len);
			break;

		case USBD_IDX_INTERFACE_STR:
			pbuf = pdev->desc->get_interface_desc(&len);
			break;

		/* For all USER string */
		case USBD_IDX_USER0_STR:
		default:
			pbuf = pdev->desc->get_usr_desc(desc_idx - USBD_IDX_USER0_STR, &len);
			break;
		}
		break;

	case USB_DESC_TYPE_DEVICE_QUALIFIER:
		pbuf = pdev->desc->get_device_qualifier_desc(&len);
		break;

	case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
		if (pdev->desc->get_other_speed_config_desc == NULL) {
			usb_core_ctl_error(pdev);
			return;
		}
		pbuf = pdev->desc->get_other_speed_config_desc(&len);
		break;

	default:
		ERROR("Unknown request %i\n", desc_type);
		usb_core_ctl_error(pdev);
		return;
	}

	if ((len != 0U) && (req->length != 0U)) {
		len = MIN(len, req->length);

		/* Start the transfer */
		usb_core_transmit_ep0(pdev, pbuf, len);
	}
}

/*
 * usb_core_set_config
 *         Handle Set device configuration request
 * pdev : device instance
 * req : usb request
 */
static void usb_core_set_config(struct usb_handle *pdev, struct usb_setup_req *req)
{
	static uint8_t cfgidx;

	cfgidx = LOBYTE(req->value);

	if (cfgidx > USBD_MAX_NUM_CONFIGURATION) {
		usb_core_ctl_error(pdev);
		return;
	}

	switch (pdev->dev_state) {
	case USBD_STATE_ADDRESSED:
		if (cfgidx != 0U) {
			pdev->dev_config = cfgidx;
			pdev->dev_state = USBD_STATE_CONFIGURED;
			if (!pdev->class) {
				usb_core_ctl_error(pdev);
				return;
			}
			/* Set configuration and Start the Class */
			if (pdev->class->init(pdev, cfgidx) != 0U) {
				usb_core_ctl_error(pdev);
				return;
			}
		}
		break;

	case USBD_STATE_CONFIGURED:
		if (cfgidx == 0U) {
			pdev->dev_state = USBD_STATE_ADDRESSED;
			pdev->dev_config = cfgidx;
			pdev->class->de_init(pdev, cfgidx);
		} else if (cfgidx != pdev->dev_config) {
			if (pdev->class == NULL) {
				usb_core_ctl_error(pdev);
				return;
			}
			/* Clear old configuration */
			pdev->class->de_init(pdev, pdev->dev_config);
			/* Set new configuration */
			pdev->dev_config = cfgidx;
			/* Set configuration and start the USB class */
			if (pdev->class->init(pdev, cfgidx) != 0U) {
				usb_core_ctl_error(pdev);
				return;
			}
		}
		break;

	default:
		usb_core_ctl_error(pdev);
		return;
	}

	/* Send status */
	usb_core_transmit_ep0(pdev, NULL, 0U);
}

/*
 * usb_core_get_status
 *         Handle Get Status request
 * pdev : device instance
 * req : usb request
 */
static void usb_core_get_status(struct usb_handle *pdev,
				struct usb_setup_req *req)
{
	if ((pdev->dev_state != USBD_STATE_ADDRESSED) &&
	    (pdev->dev_state != USBD_STATE_CONFIGURED)) {
		usb_core_ctl_error(pdev);
		return;
	}

	pdev->dev_config_status = USB_CONFIG_SELF_POWERED;

	if (pdev->dev_remote_wakeup != 0U) {
		pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;
	}

	/* Start the transfer */
	usb_core_transmit_ep0(pdev, (uint8_t *)&pdev->dev_config_status, 2U);
}

/*
 * usb_core_set_address
 *         Set device address
 * pdev : device instance
 * req : usb request
 */
static void usb_core_set_address(struct usb_handle *pdev,
				 struct usb_setup_req *req)
{
	uint8_t dev_addr;

	if ((req->index != 0U) || (req->length != 0U)) {
		usb_core_ctl_error(pdev);
		return;
	}

	dev_addr = req->value & ADDRESS_MASK;
	if (pdev->dev_state != USBD_STATE_DEFAULT) {
		usb_core_ctl_error(pdev);
		return;
	}

	pdev->dev_address = dev_addr;
	pdev->driver->set_address(((struct pcd_handle *)(pdev->data))->instance, dev_addr);

	/* Send status */
	usb_core_transmit_ep0(pdev, NULL, 0U);

	if (dev_addr != 0U) {
		pdev->dev_state  = USBD_STATE_ADDRESSED;
	} else {
		pdev->dev_state  = USBD_STATE_DEFAULT;
	}
}

/*
 * usb_core_dev_req
 *         Handle standard usb device requests
 * pdev : device instance
 * req : usb request
 * return : status
 */
static enum usb_status usb_core_dev_req(struct usb_handle *pdev,
					struct usb_setup_req *req)
{
	VERBOSE("receive request %i\n", req->b_request);
	switch (req->b_request) {
	case USB_REQ_GET_DESCRIPTOR:
		usb_core_get_desc(pdev, req);
		break;

	case USB_REQ_SET_CONFIGURATION:
		usb_core_set_config(pdev, req);
		break;

	case USB_REQ_GET_STATUS:
		usb_core_get_status(pdev, req);
		break;

	case USB_REQ_SET_ADDRESS:
		usb_core_set_address(pdev, req);
		break;

	case USB_REQ_GET_CONFIGURATION:
	case USB_REQ_SET_FEATURE:
	case USB_REQ_CLEAR_FEATURE:
	default:
		ERROR("NOT SUPPORTED %i\n", req->b_request);
		usb_core_ctl_error(pdev);
		break;
	}

	return USBD_OK;
}

/*
 * usb_core_itf_req
 *         Handle standard usb interface requests
 * pdev : device instance
 * req : usb request
 * return : status
 */
static enum usb_status usb_core_itf_req(struct usb_handle *pdev,
					struct usb_setup_req *req)
{
	if (pdev->dev_state != USBD_STATE_CONFIGURED) {
		usb_core_ctl_error(pdev);
		return USBD_OK;
	}

	if (LOBYTE(req->index) <= USBD_MAX_NUM_INTERFACES) {
		pdev->class->setup(pdev, req);

		if (req->length == 0U) {
			usb_core_transmit_ep0(pdev, NULL, 0U);
		}
	} else {
		usb_core_ctl_error(pdev);
	}

	return USBD_OK;
}

/*
 * usb_core_setup_stage
 *         Handle the setup stage
 * pdev: device instance
 * psetup : setup buffer
 * return : status
 */
static enum usb_status usb_core_setup_stage(struct usb_handle *pdev,
					    uint8_t *psetup)
{
	struct usb_setup_req *req = &pdev->request;

	/* Copy setup buffer into req structure */
	req->bm_request = psetup[0];
	req->b_request = psetup[1];
	req->value = psetup[2] + (psetup[3] << 8);
	req->index = psetup[4] + (psetup[5] << 8);
	req->length = psetup[6] + (psetup[7] << 8);

	pdev->ep0_state = USBD_EP0_SETUP;
	pdev->ep0_data_len = pdev->request.length;

	switch (pdev->request.bm_request & USB_REQ_RECIPIENT_MASK) {
	case USB_REQ_RECIPIENT_DEVICE:
		usb_core_dev_req(pdev, &pdev->request);
		break;

	case USB_REQ_RECIPIENT_INTERFACE:
		usb_core_itf_req(pdev, &pdev->request);
		break;

	case USB_REQ_RECIPIENT_ENDPOINT:
	default:
		ERROR("receive unsupported request %i",
		      pdev->request.bm_request & USB_REQ_RECIPIENT_MASK);
		usb_core_set_stall(pdev, pdev->request.bm_request & USB_REQ_DIRECTION);
		return USBD_FAIL;
	}

	return USBD_OK;
}

/*
 * usb_core_data_out
 *         Handle data OUT stage
 * pdev: device instance
 * epnum: endpoint index
 * pdata: buffer to sent
 * return : status
 */
static enum usb_status usb_core_data_out(struct usb_handle *pdev, uint8_t epnum,
					 uint8_t *pdata)
{
	struct usb_endpoint *pep;

	if (epnum == 0U) {
		pep = &pdev->ep_out[0];
		if (pdev->ep0_state == USBD_EP0_DATA_OUT) {
			if (pep->rem_length > pep->maxpacket) {
				pep->rem_length -= pep->maxpacket;

				usb_core_receive(pdev, 0U, pdata,
						 MIN(pep->rem_length,
						     pep->maxpacket));
			} else {
				if (pdev->class->ep0_rx_ready &&
				    (pdev->dev_state == USBD_STATE_CONFIGURED)) {
					pdev->class->ep0_rx_ready(pdev);
				}

				usb_core_transmit_ep0(pdev, NULL, 0U);
			}
		}
	} else if (pdev->class->data_out != NULL &&
		   (pdev->dev_state == USBD_STATE_CONFIGURED)) {
		pdev->class->data_out(pdev, epnum);
	}

	return USBD_OK;
}

/*
 * usb_core_data_in
 *         Handle data in stage
 * pdev: device instance
 * epnum: endpoint index
 * pdata: buffer to fill
 * return : status
 */
static enum usb_status usb_core_data_in(struct usb_handle *pdev, uint8_t epnum,
					uint8_t *pdata)
{
	if (epnum == 0U) {
		struct usb_endpoint *pep = &pdev->ep_in[0];

		if (pdev->ep0_state == USBD_EP0_DATA_IN) {
			if (pep->rem_length > pep->maxpacket) {
				pep->rem_length -= pep->maxpacket;

				usb_core_transmit(pdev, 0U, pdata,
						  pep->rem_length);

				/* Prepare EP for premature end of transfer */
				usb_core_receive(pdev, 0U, NULL, 0U);
			} else {
				/* Last packet is MPS multiple, send ZLP packet */
				if ((pep->total_length % pep->maxpacket == 0U) &&
				    (pep->total_length >= pep->maxpacket) &&
				    (pep->total_length < pdev->ep0_data_len)) {
					usb_core_transmit(pdev, 0U, NULL, 0U);

					pdev->ep0_data_len = 0U;

					/* Prepare endpoint for premature end of transfer */
					usb_core_receive(pdev, 0U, NULL, 0U);
				} else {
					if (pdev->class->ep0_tx_sent != NULL &&
					    (pdev->dev_state ==
					     USBD_STATE_CONFIGURED)) {
						pdev->class->ep0_tx_sent(pdev);
					}
					/* Start the transfer */
					usb_core_receive_ep0(pdev, NULL, 0U);
				}
			}
		}
	} else if ((pdev->class->data_in != NULL) &&
		  (pdev->dev_state == USBD_STATE_CONFIGURED)) {
		pdev->class->data_in(pdev, epnum);
	}

	return USBD_OK;
}

/*
 * usb_core_suspend
 *         Handle suspend event
 * pdev : device instance
 * return : status
 */
static enum usb_status usb_core_suspend(struct usb_handle  *pdev)
{
	INFO("USB Suspend mode\n");
	pdev->dev_old_state =  pdev->dev_state;
	pdev->dev_state  = USBD_STATE_SUSPENDED;

	return USBD_OK;
}

/*
 * usb_core_resume
 *         Handle resume event
 * pdev : device instance
 * return : status
 */
static enum usb_status usb_core_resume(struct usb_handle *pdev)
{
	INFO("USB Resume\n");
	pdev->dev_state = pdev->dev_old_state;

	return USBD_OK;
}

/*
 * usb_core_sof
 *         Handle SOF event
 * pdev : device instance
 * return : status
 */
static enum usb_status usb_core_sof(struct usb_handle *pdev)
{
	if (pdev->dev_state == USBD_STATE_CONFIGURED) {
		if (pdev->class->sof != NULL) {
			pdev->class->sof(pdev);
		}
	}

	return USBD_OK;
}

/*
 * usb_core_disconnect
 *         Handle device disconnection event
 * pdev : device instance
 * return : status
 */
static enum usb_status usb_core_disconnect(struct usb_handle *pdev)
{
	/* Free class resources */
	pdev->dev_state = USBD_STATE_DEFAULT;
	pdev->class->de_init(pdev, pdev->dev_config);

	return USBD_OK;
}

enum usb_status usb_core_handle_it(struct usb_handle *pdev)
{
	uint32_t param = 0U;
	uint32_t len = 0U;
	struct usbd_ep *ep;

	switch (pdev->driver->it_handler(pdev->data->instance, &param)) {
	case USB_DATA_OUT:
		usb_core_data_out(pdev, param,
				  pdev->data->out_ep[param].xfer_buff);
		break;

	case USB_DATA_IN:
		usb_core_data_in(pdev, param,
				 pdev->data->in_ep[param].xfer_buff);
		break;

	case USB_SETUP:
		usb_core_setup_stage(pdev, (uint8_t *)pdev->data->setup);
		break;

	case USB_ENUM_DONE:
		break;

	case USB_READ_DATA_PACKET:
		ep = &pdev->data->out_ep[param &  USBD_OUT_EPNUM_MASK];
		len = (param &  USBD_OUT_COUNT_MASK) >> USBD_OUT_COUNT_SHIFT;
		pdev->driver->read_packet(pdev->data->instance,
					  ep->xfer_buff, len);
		ep->xfer_buff += len;
		ep->xfer_count += len;
		break;

	case USB_READ_SETUP_PACKET:
		ep = &pdev->data->out_ep[param &  USBD_OUT_EPNUM_MASK];
		len = (param &  USBD_OUT_COUNT_MASK) >> 0x10;
		pdev->driver->read_packet(pdev->data->instance,
					  (uint8_t *)pdev->data->setup, 8);
		ep->xfer_count += len;
		break;

	case USB_RESET:
		pdev->dev_state = USBD_STATE_DEFAULT;
		break;

	case USB_RESUME:
		if (pdev->data->lpm_state == LPM_L1) {
			pdev->data->lpm_state = LPM_L0;
		} else {
			usb_core_resume(pdev);
		}
		break;

	case USB_SUSPEND:
		usb_core_suspend(pdev);
		break;

	case USB_LPM:
		if (pdev->data->lpm_state == LPM_L0) {
			pdev->data->lpm_state = LPM_L1;
		} else {
			usb_core_suspend(pdev);
		}
		break;

	case USB_SOF:
		usb_core_sof(pdev);
		break;

	case USB_DISCONNECT:
		usb_core_disconnect(pdev);
		break;

	case USB_WRITE_EMPTY:
		pdev->driver->write_empty_tx_fifo(pdev->data->instance, param,
				     pdev->data->in_ep[param].xfer_len,
				     (uint32_t *)&pdev->data->in_ep[param].xfer_count,
				     pdev->data->in_ep[param].maxpacket,
				     &pdev->data->in_ep[param].xfer_buff);
		break;

	case USB_NOTHING:
	default:
		break;
	}

	return USBD_OK;
}

static void usb_core_start_xfer(struct usb_handle *pdev,
				void *handle,
				struct usbd_ep *ep)
{
	if (ep->num == 0U) {
		pdev->driver->ep0_start_xfer(handle, ep);
	} else {
		pdev->driver->ep_start_xfer(handle, ep);
	}
}

/*
 * usb_core_receive
 *          Receive an amount of data
 * pdev: USB handle
 * ep_addr: endpoint address
 * buf: pointer to the reception buffer
 * len: amount of data to be received
 * return : status
 */
enum usb_status usb_core_receive(struct usb_handle *pdev, uint8_t ep_addr,
				 uint8_t *buf, uint32_t len)
{
	struct usbd_ep *ep;
	struct pcd_handle *hpcd = (struct pcd_handle *)pdev->data;
	uint8_t num;

	num = ep_addr & EP_NUM_MASK;
	if (num >= USBD_EP_NB) {
		return USBD_FAIL;
	}
	ep = &hpcd->out_ep[num];

	/* Setup and start the Xfer */
	ep->xfer_buff = buf;
	ep->xfer_len = len;
	ep->xfer_count = 0U;
	ep->is_in = false;
	ep->num = num;

	usb_core_start_xfer(pdev, hpcd->instance, ep);

	return USBD_OK;
}

/*
 * usb_core_transmit
 *          Send an amount of data
 * pdev: USB handle
 * ep_addr: endpoint address
 * buf: pointer to the transmission buffer
 * len: amount of data to be sent
 * return : status
 */
enum usb_status usb_core_transmit(struct usb_handle *pdev, uint8_t ep_addr,
				  uint8_t *buf, uint32_t len)
{
	struct usbd_ep *ep;
	struct pcd_handle *hpcd = (struct pcd_handle *)pdev->data;
	uint8_t num;

	num = ep_addr & EP_NUM_MASK;
	if (num >= USBD_EP_NB) {
		return USBD_FAIL;
	}
	ep = &hpcd->in_ep[num];

	/* Setup and start the Xfer */
	ep->xfer_buff = buf;
	ep->xfer_len = len;
	ep->xfer_count = 0U;
	ep->is_in = true;
	ep->num = num;

	usb_core_start_xfer(pdev, hpcd->instance, ep);

	return USBD_OK;
}

/*
 * usb_core_receive_ep0
 *          Receive an amount of data on ep0
 * pdev: USB handle
 * buf: pointer to the reception buffer
 * len: amount of data to be received
 * return : status
 */
enum usb_status usb_core_receive_ep0(struct usb_handle *pdev, uint8_t *buf,
				     uint32_t len)
{
	/* Prepare the reception of the buffer over EP0 */
	if (len != 0U) {
		pdev->ep0_state = USBD_EP0_DATA_OUT;
	} else {
		pdev->ep0_state = USBD_EP0_STATUS_OUT;
	}

	pdev->ep_out[0].total_length = len;
	pdev->ep_out[0].rem_length = len;

	/* Start the transfer */
	return usb_core_receive(pdev, 0U, buf, len);
}

/*
 * usb_core_transmit_ep0
 *          Send an amount of data on ep0
 * pdev: USB handle
 * buf: pointer to the transmission buffer
 * len: amount of data to be sent
 * return : status
 */
enum usb_status usb_core_transmit_ep0(struct usb_handle *pdev, uint8_t *buf,
				      uint32_t len)
{
	/* Set EP0 State */
	if (len != 0U) {
		pdev->ep0_state = USBD_EP0_DATA_IN;
	} else {
		pdev->ep0_state = USBD_EP0_STATUS_IN;
	}

	pdev->ep_in[0].total_length = len;
	pdev->ep_in[0].rem_length = len;

	/* Start the transfer */
	return usb_core_transmit(pdev, 0U, buf, len);
}

/*
 * usb_core_ctl_error
 *         Handle USB low level error
 * pdev: device instance
 * req: usb request
 * return : None
 */

void usb_core_ctl_error(struct usb_handle *pdev)
{
	ERROR("%s : Send an ERROR\n", __func__);
	usb_core_set_stall(pdev, EP0_IN);
	usb_core_set_stall(pdev, EP0_OUT);
}

/*
 * usb_core_start
 *         Start the USB device core.
 * pdev: Device Handle
 * return : USBD Status
 */
enum usb_status usb_core_start(struct usb_handle *pdev)
{
	/* Start the low level driver */
	pdev->driver->start_device(pdev->data->instance);

	return USBD_OK;
}

/*
 * usb_core_stop
 *         Stop the USB device core.
 * pdev: Device Handle
 * return : USBD Status
 */
enum usb_status usb_core_stop(struct usb_handle *pdev)
{
	/* Free class resources */
	pdev->class->de_init(pdev, pdev->dev_config);

	/* Stop the low level driver */
	pdev->driver->stop_device(pdev->data->instance);

	return USBD_OK;
}

/*
 * register_usb_driver
 *         Stop the USB device core.
 * pdev: Device Handle
 * pcd_handle: PCD handle
 * driver: USB driver
 * driver_handle: USB driver handle
 * return : USBD Status
 */
enum usb_status register_usb_driver(struct usb_handle *pdev,
				    struct pcd_handle *pcd_handle,
				    const struct usb_driver *driver,
				    void *driver_handle)
{
	uint8_t i;

	assert(pdev != NULL);
	assert(pcd_handle != NULL);
	assert(driver != NULL);
	assert(driver_handle != NULL);

	/* Free class resources */
	pdev->driver = driver;
	pdev->data = pcd_handle;
	pdev->data->instance = driver_handle;
	pdev->dev_state = USBD_STATE_DEFAULT;
	pdev->ep0_state = USBD_EP0_IDLE;

	/* Copy endpoint information */
	for (i = 0U; i < USBD_EP_NB; i++) {
		pdev->ep_in[i].maxpacket = pdev->data->in_ep[i].maxpacket;
		pdev->ep_out[i].maxpacket = pdev->data->out_ep[i].maxpacket;
	}

	return USBD_OK;
}

/*
 * register_platform
 *         Register the USB device core.
 * pdev: Device Handle
 * plat_call_back: callback
 * return : USBD Status
 */
enum usb_status register_platform(struct usb_handle *pdev,
			       const struct usb_desc *plat_call_back)
{
	assert(pdev != NULL);
	assert(plat_call_back != NULL);

	/* Save platform info in class resources */
	pdev->desc = plat_call_back;

	return USBD_OK;
}

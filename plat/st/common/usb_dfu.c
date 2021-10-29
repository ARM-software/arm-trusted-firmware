/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>

#include <platform_def.h>
#include <usb_dfu.h>

/* Device states as defined in DFU spec */
#define STATE_APP_IDLE			0
#define STATE_APP_DETACH		1
#define STATE_DFU_IDLE			2
#define STATE_DFU_DNLOAD_SYNC		3
#define STATE_DFU_DNLOAD_BUSY		4
#define STATE_DFU_DNLOAD_IDLE		5
#define STATE_DFU_MANIFEST_SYNC		6
#define STATE_DFU_MANIFEST		7
#define STATE_DFU_MANIFEST_WAIT_RESET	8
#define STATE_DFU_UPLOAD_IDLE		9
#define STATE_DFU_ERROR			10

/* DFU errors */
#define DFU_ERROR_NONE			0x00
#define DFU_ERROR_TARGET		0x01
#define DFU_ERROR_FILE			0x02
#define DFU_ERROR_WRITE			0x03
#define DFU_ERROR_ERASE			0x04
#define DFU_ERROR_CHECK_ERASED		0x05
#define DFU_ERROR_PROG			0x06
#define DFU_ERROR_VERIFY		0x07
#define DFU_ERROR_ADDRESS		0x08
#define DFU_ERROR_NOTDONE		0x09
#define DFU_ERROR_FIRMWARE		0x0A
#define DFU_ERROR_VENDOR		0x0B
#define DFU_ERROR_USB			0x0C
#define DFU_ERROR_POR			0x0D
#define DFU_ERROR_UNKNOWN		0x0E
#define DFU_ERROR_STALLEDPKT		0x0F

/* DFU request */
#define DFU_DETACH			0
#define DFU_DNLOAD			1
#define DFU_UPLOAD			2
#define DFU_GETSTATUS			3
#define DFU_CLRSTATUS			4
#define DFU_GETSTATE			5
#define DFU_ABORT			6

static bool usb_dfu_detach_req;

/*
 * usb_dfu_init
 *         Initialize the DFU interface
 * pdev: device instance
 * cfgidx: Configuration index
 * return: status
 */
static uint8_t usb_dfu_init(struct usb_handle *pdev, uint8_t cfgidx)
{
	(void)pdev;
	(void)cfgidx;

	/* Nothing to do in this stage */
	return USBD_OK;
}

/*
 * usb_dfu_de_init
 *         De-Initialize the DFU layer
 * pdev: device instance
 * cfgidx: Configuration index
 * return: status
 */
static uint8_t usb_dfu_de_init(struct usb_handle *pdev, uint8_t cfgidx)
{
	(void)pdev;
	(void)cfgidx;

	/* Nothing to do in this stage */
	return USBD_OK;
}

/*
 * usb_dfu_data_in
 *         handle data IN Stage
 * pdev: device instance
 * epnum: endpoint index
 * return: status
 */
static uint8_t usb_dfu_data_in(struct usb_handle *pdev, uint8_t epnum)
{
	(void)pdev;
	(void)epnum;

	return USBD_OK;
}

/*
 * usb_dfu_ep0_rx_ready
 *         handle EP0 Rx Ready event
 * pdev: device
 * return: status
 */
static uint8_t usb_dfu_ep0_rx_ready(struct usb_handle *pdev)
{
	(void)pdev;

	return USBD_OK;
}

/*
 * usb_dfu_ep0_tx_ready
 *         handle EP0 TRx Ready event
 * pdev: device instance
 * return: status
 */
static uint8_t usb_dfu_ep0_tx_ready(struct usb_handle *pdev)
{
	(void)pdev;

	return USBD_OK;
}

/*
 * usb_dfu_sof
 *         handle SOF event
 * pdev: device instance
 * return: status
 */
static uint8_t usb_dfu_sof(struct usb_handle *pdev)
{
	(void)pdev;

	return USBD_OK;
}

/*
 * usb_dfu_iso_in_incomplete
 *         handle data ISO IN Incomplete event
 * pdev: device instance
 * epnum: endpoint index
 * return: status
 */
static uint8_t usb_dfu_iso_in_incomplete(struct usb_handle *pdev, uint8_t epnum)
{
	(void)pdev;
	(void)epnum;

	return USBD_OK;
}

/*
 * usb_dfu_iso_out_incomplete
 *         handle data ISO OUT Incomplete event
 * pdev: device instance
 * epnum: endpoint index
 * return: status
 */
static uint8_t usb_dfu_iso_out_incomplete(struct usb_handle *pdev,
					  uint8_t epnum)
{
	(void)pdev;
	(void)epnum;

	return USBD_OK;
}

/*
 * usb_dfu_data_out
 *         handle data OUT Stage
 * pdev: device instance
 * epnum: endpoint index
 * return: status
 */
static uint8_t usb_dfu_data_out(struct usb_handle *pdev, uint8_t epnum)
{
	(void)pdev;
	(void)epnum;

	return USBD_OK;
}

/*
 * usb_dfu_detach
 *         Handles the DFU DETACH request.
 * pdev: device instance
 * req: pointer to the request structure.
 */
static void usb_dfu_detach(struct usb_handle *pdev, struct usb_setup_req *req)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	INFO("Receive DFU Detach\n");

	if ((hdfu->dev_state == STATE_DFU_IDLE) ||
	    (hdfu->dev_state == STATE_DFU_DNLOAD_SYNC) ||
	    (hdfu->dev_state == STATE_DFU_DNLOAD_IDLE) ||
	    (hdfu->dev_state == STATE_DFU_MANIFEST_SYNC) ||
	    (hdfu->dev_state == STATE_DFU_UPLOAD_IDLE)) {
		/* Update the state machine */
		hdfu->dev_state = STATE_DFU_IDLE;
		hdfu->dev_status = DFU_ERROR_NONE;
	}

	usb_dfu_detach_req = true;
}

/*
 * usb_dfu_download
 *         Handles the DFU DNLOAD request.
 * pdev: device instance
 * req: pointer to the request structure
 */
static void usb_dfu_download(struct usb_handle *pdev, struct usb_setup_req *req)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;
	uintptr_t data_ptr;
	uint32_t length;
	int ret;

	/* Data setup request */
	if (req->length > 0) {
		/* Unsupported state */
		if ((hdfu->dev_state != STATE_DFU_IDLE) &&
		    (hdfu->dev_state != STATE_DFU_DNLOAD_IDLE)) {
			/* Call the error management function (command will be nacked) */
			usb_core_ctl_error(pdev);
			return;
		}

		/* Get the data address */
		length = req->length;
		ret = hdfu->callback->download(hdfu->alt_setting, &data_ptr,
					       &length, pdev->user_data);
		if (ret == 0U) {
			/* Update the state machine */
			hdfu->dev_state = STATE_DFU_DNLOAD_SYNC;
			/* Start the transfer */
			usb_core_receive_ep0(pdev, (uint8_t *)data_ptr, length);
		} else {
			usb_core_ctl_error(pdev);
		}
	} else {
		/* End of DNLOAD operation*/
		if (hdfu->dev_state != STATE_DFU_DNLOAD_IDLE) {
			/* Call the error management function (command will be nacked) */
			usb_core_ctl_error(pdev);
			return;
		}
		/* End of DNLOAD operation*/
		hdfu->dev_state = STATE_DFU_MANIFEST_SYNC;
		ret = hdfu->callback->manifestation(hdfu->alt_setting, pdev->user_data);
		if (ret == 0U) {
			hdfu->dev_state = STATE_DFU_MANIFEST_SYNC;
		} else {
			usb_core_ctl_error(pdev);
		}
	}
}

/*
 * usb_dfu_upload
 *         Handles the DFU UPLOAD request.
 * pdev: instance
 * req: pointer to the request structure
 */
static void usb_dfu_upload(struct usb_handle *pdev, struct usb_setup_req *req)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;
	uintptr_t data_ptr;
	uint32_t length;
	int ret;

	/* Data setup request */
	if (req->length == 0) {
		/* No Data setup request */
		hdfu->dev_state = STATE_DFU_IDLE;
		return;
	}

	/* Unsupported state */
	if ((hdfu->dev_state != STATE_DFU_IDLE) && (hdfu->dev_state != STATE_DFU_UPLOAD_IDLE)) {
		ERROR("UPLOAD : Unsupported State\n");
		/* Call the error management function (command will be nacked) */
		usb_core_ctl_error(pdev);
		return;
	}

	/* Update the data address */
	length = req->length;
	ret = hdfu->callback->upload(hdfu->alt_setting, &data_ptr, &length, pdev->user_data);
	if (ret == 0U) {
		/* Short frame */
		hdfu->dev_state = (req->length > length) ? STATE_DFU_IDLE : STATE_DFU_UPLOAD_IDLE;

		/* Start the transfer */
		usb_core_transmit_ep0(pdev, (uint8_t *)data_ptr, length);
	} else {
		ERROR("UPLOAD : bad block %i on alt %i\n", req->value, req->index);
		hdfu->dev_state = STATE_DFU_ERROR;
		hdfu->dev_status = DFU_ERROR_STALLEDPKT;

		/* Call the error management function (command will be nacked) */
		usb_core_ctl_error(pdev);
	}
}

/*
 * usb_dfu_get_status
 *         Handles the DFU GETSTATUS request.
 * pdev: instance
 */
static void usb_dfu_get_status(struct usb_handle *pdev)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	hdfu->status[0] = hdfu->dev_status;	/* bStatus */
	hdfu->status[1] = 0;			/* bwPollTimeout[3] */
	hdfu->status[2] = 0;
	hdfu->status[3] = 0;
	hdfu->status[4] = hdfu->dev_state;	/* bState */
	hdfu->status[5] = 0;			/* iString */

	/* next step */
	switch (hdfu->dev_state) {
	case STATE_DFU_DNLOAD_SYNC:
		hdfu->dev_state = STATE_DFU_DNLOAD_IDLE;
		break;
	case STATE_DFU_MANIFEST_SYNC:
		/* the device is 'ManifestationTolerant' */
		hdfu->status[4] = STATE_DFU_MANIFEST;
		hdfu->status[1] = 1U; /* bwPollTimeout = 1ms */
		hdfu->dev_state = STATE_DFU_IDLE;
		break;

	default:
		break;
	}

	/* Start the transfer */
	usb_core_transmit_ep0(pdev, (uint8_t *)&hdfu->status[0], sizeof(hdfu->status));
}

/*
 * usb_dfu_clear_status
 *         Handles the DFU CLRSTATUS request.
 * pdev: device instance
 */
static void usb_dfu_clear_status(struct usb_handle *pdev)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	if (hdfu->dev_state == STATE_DFU_ERROR) {
		hdfu->dev_state = STATE_DFU_IDLE;
		hdfu->dev_status = DFU_ERROR_NONE;
	} else {
		/* State Error */
		hdfu->dev_state = STATE_DFU_ERROR;
		hdfu->dev_status = DFU_ERROR_UNKNOWN;
	}
}

/*
 * usb_dfu_get_state
 *         Handles the DFU GETSTATE request.
 * pdev: device instance
 */
static void usb_dfu_get_state(struct usb_handle *pdev)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	/* Return the current state of the DFU interface */
	usb_core_transmit_ep0(pdev, &hdfu->dev_state, 1);
}

/*
 * usb_dfu_abort
 *         Handles the DFU ABORT request.
 * pdev: device instance
 */
static void usb_dfu_abort(struct usb_handle *pdev)
{
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	if ((hdfu->dev_state == STATE_DFU_IDLE) ||
	    (hdfu->dev_state == STATE_DFU_DNLOAD_SYNC) ||
	    (hdfu->dev_state == STATE_DFU_DNLOAD_IDLE) ||
	    (hdfu->dev_state == STATE_DFU_MANIFEST_SYNC) ||
	    (hdfu->dev_state == STATE_DFU_UPLOAD_IDLE)) {
		hdfu->dev_state = STATE_DFU_IDLE;
		hdfu->dev_status = DFU_ERROR_NONE;
	}
}

/*
 * usb_dfu_setup
 *         Handle the DFU specific requests
 * pdev: instance
 * req: usb requests
 * return: status
 */
static uint8_t usb_dfu_setup(struct usb_handle *pdev, struct usb_setup_req *req)
{
	uint8_t *pbuf = NULL;
	uint16_t len = 0U;
	uint8_t ret = USBD_OK;
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	switch (req->bm_request & USB_REQ_TYPE_MASK) {
	case USB_REQ_TYPE_CLASS:
		switch (req->b_request) {
		case DFU_DNLOAD:
			usb_dfu_download(pdev, req);
			break;

		case DFU_UPLOAD:
			usb_dfu_upload(pdev, req);
			break;

		case DFU_GETSTATUS:
			usb_dfu_get_status(pdev);
			break;

		case DFU_CLRSTATUS:
			usb_dfu_clear_status(pdev);
			break;

		case DFU_GETSTATE:
			usb_dfu_get_state(pdev);
			break;

		case DFU_ABORT:
			usb_dfu_abort(pdev);
			break;

		case DFU_DETACH:
			usb_dfu_detach(pdev, req);
			break;

		default:
			ERROR("unknown request %x on alternate %i\n",
			      req->b_request, hdfu->alt_setting);
			usb_core_ctl_error(pdev);
			ret = USBD_FAIL;
			break;
		}
		break;
	case USB_REQ_TYPE_STANDARD:
		switch (req->b_request) {
		case USB_REQ_GET_DESCRIPTOR:
			if (HIBYTE(req->value) == DFU_DESCRIPTOR_TYPE) {
				pbuf = pdev->desc->get_config_desc(&len);
				/* DFU descriptor at the end of the USB */
				pbuf += len - 9U;
				len = 9U;
				len = MIN(len, req->length);
			}

			/* Start the transfer */
			usb_core_transmit_ep0(pdev, pbuf, len);

			break;

		case USB_REQ_GET_INTERFACE:
			/* Start the transfer */
			usb_core_transmit_ep0(pdev, (uint8_t *)&hdfu->alt_setting, 1U);
			break;

		case USB_REQ_SET_INTERFACE:
			hdfu->alt_setting = LOBYTE(req->value);
			break;

		default:
			usb_core_ctl_error(pdev);
			ret = USBD_FAIL;
			break;
		}
	default:
		break;
	}

	return ret;
}

static const struct usb_class usb_dfu = {
	.init = usb_dfu_init,
	.de_init = usb_dfu_de_init,
	.setup = usb_dfu_setup,
	.ep0_tx_sent = usb_dfu_ep0_tx_ready,
	.ep0_rx_ready = usb_dfu_ep0_rx_ready,
	.data_in = usb_dfu_data_in,
	.data_out = usb_dfu_data_out,
	.sof = usb_dfu_sof,
	.iso_in_incomplete = usb_dfu_iso_in_incomplete,
	.iso_out_incomplete = usb_dfu_iso_out_incomplete,
};

void usb_dfu_register(struct usb_handle *pdev, struct usb_dfu_handle *phandle)
{
	pdev->class = (struct usb_class *)&usb_dfu;
	pdev->class_data = phandle;

	phandle->dev_state = STATE_DFU_IDLE;
	phandle->dev_status = DFU_ERROR_NONE;
}

int usb_dfu_loop(struct usb_handle *pdev, const struct usb_dfu_media *pmedia)
{
	uint32_t it_count;
	enum usb_status ret;
	struct usb_dfu_handle *hdfu = (struct usb_dfu_handle *)pdev->class_data;

	hdfu->callback = pmedia;
	usb_dfu_detach_req = false;
	/* Continue to handle USB core IT to assure complete data transmission */
	it_count = 100U;

	/* DFU infinite loop until DETACH_REQ */
	while (it_count != 0U) {
		ret = usb_core_handle_it(pdev);
		if (ret != USBD_OK) {
			return -EIO;
		}

		/* Detach request received */
		if (usb_dfu_detach_req) {
			it_count--;
		}
	}

	return 0;
}
